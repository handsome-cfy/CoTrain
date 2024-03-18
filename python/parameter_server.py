import copy
import json
import random

import numpy as np
import torch
from torch import optim
from torch.utils.data import DataLoader
from torch.utils.data.dataset import Subset
from tqdm import tqdm

from aggregate_config import parse_arguments
from local_client import LocalClient, LocalClientThread, DPLocalClient, CoTrainLocalClient, CoTrainClientThread
from my_socket import Server
from src.tool import get_model, get_dataset
from utils import set_seed


class ParameterServer(Server):
    def __init__(self, config):
        super().__init__(config['server_ip'], int(config['server_port']))
        self.learning_rate = 0.001
        self.config = config
        self.device = torch.device('cuda' if torch.cuda.is_available() else 'cpu')

        self.global_model = get_model(config).to(self.device)
        self.num_clients = config['num_clients']
        self.epoch = config['epoch']
        self.optimizer = optim.SGD(self.global_model.parameters(), lr=0.1)

        self.test_datset = None
        self.test_dataloader = None
        self.test_loss = []
        self.best_test_acc = 0

        self.client_list = []
        self.threads = []

        self.num_select = config['num_select'] if config['num_select'] <= self.num_clients else self.num_clients

    def init_client(self):

        trainset, test_set = get_dataset(config, True)

        num_client = config['num_clients']

        trainset_config = {'users': [],
                           'user_data': {},
                           'num_samples': []}

        config_division = {}  # Count of the classes for division
        config_class = {}  # Configuration of class distribution in clients
        config_data = {}  # Configuration of data indexes for each class : Config_data[cls] = [0, []] | pointer and indexes

        for i in range(num_client):
            config_class['f_{0:05d}'.format(i)] = []
            for j in range(config['num_classes']):
                cls = (i + j) % config['num_classes']
                if cls not in config_division:
                    config_division[cls] = 1
                    config_data[cls] = [0, []]

                else:
                    config_division[cls] += 1
                config_class['f_{0:05d}'.format(i)].append(cls)

        for cls in config_division.keys():
            indexes = torch.nonzero(torch.tensor(trainset.dataset.targets) == cls)
            num_datapoint = indexes.shape[0]
            indexes = indexes[torch.randperm(num_datapoint)]
            num_partition = num_datapoint // config_division[cls]
            for i_partition in range(config_division[cls]):
                if i_partition == config_division[cls] - 1:
                    config_data[cls][1].append(indexes[i_partition * num_partition:])
                else:
                    config_data[cls][1].append(indexes[i_partition * num_partition: (i_partition + 1) * num_partition])

        for user in config_class.keys():
            user_data_indexes = torch.tensor([])
            for cls in config_class[user]:
                user_data_index = config_data[cls][1][config_data[cls][0]]
                user_data_indexes = torch.cat((user_data_indexes, user_data_index))
                config_data[cls][0] += 1
            user_data_indexes = user_data_indexes.squeeze().int().tolist()
            user_data = Subset(trainset, user_data_indexes)
            # user_targets = trainset.target[user_data_indexes.tolist()]
            trainset_config['users'].append(user)
            trainset_config['user_data'][user] = user_data
            trainset_config['num_samples'] = len(user_data)

        for client_id in trainset_config['users']:
            if self.config['Algorithm'] == 'FedAvg':
                client = LocalClient(client_id, '127.0.0.1', int('8010'), config,
                                     trainset_config['user_data'][client_id])
            elif self.config['Algorithm'] == 'CoTrain':
                client = CoTrainLocalClient(client_id, '127.0.0.1', int('8010'), config,
                                            trainset_config['user_data'][client_id])
            self.client_list.append(client)
        self.test_datset = test_set
        self.test_dataloader = DataLoader(self.test_datset, batch_size=self.config['batch_size'], shuffle=True)

    def FedAvg(self):

        # 对于所有可用的客户端
        print("Sending initial weight")

        for e in range(self.epoch):
            print(f"=====Epoch{e}=====")
            client_updates = []
            client_weight = []
            thread_list = []
            for i in range(len(self.client_list)):
                thread = LocalClientThread(self.client_list[i], copy.deepcopy(self.global_model.state_dict()))
                thread_list.append(thread)
                thread.start()

            for thread in self.select_clients(thread_list):
                thread.join()
                client_updates.append(copy.deepcopy(thread.output))
                client_weight.append(thread.weight)

            self.agg(client_updates, client_weight)

            self.test_global_model()

        print("=====Finish=====")
        print(f"Best ACC{self.best_test_acc}")
        print(f"Loss:{self.test_loss}")

    def aggregate_model_updates(self, client_updates):
        # 初始化全局模型参数梯度
        # self.optimizer.zero_grad()

        def aggregate_gradients(gradients_list):
            # 初始化聚合后的梯度列表
            aggregated_gradients = []

            # 确定梯度张量的数量
            num_gradients = len(gradients_list[0])

            # 对每个梯度张量进行聚合
            for i in range(num_gradients):
                # 初始化聚合后的梯度张量
                aggregated_gradient = torch.zeros_like(gradients_list[0][i])

                # 对每个客户端的梯度张量进行累加
                for gradients in gradients_list:
                    aggregated_gradient += gradients[i]

                # 计算平均梯度
                aggregated_gradient = round(aggregated_gradient / len(gradients_list))

                # 将聚合后的梯度张量添加到聚合列表中
                aggregated_gradients.append(aggregated_gradient)

            return aggregated_gradients

        aggregated_gradients = aggregate_gradients(client_updates)

        # 更新全局模型参数的梯度
        for param, gradient in zip(self.global_model.parameters(), aggregated_gradients):
            # param.requires_grad = True
            param.grad = gradient

        # 使用优化器进行全局模型参数更新
        self.optimizer.step()

    def test_global_model(self):
        self.global_model.eval()  # 设置模型为评估模式
        correct = 0
        total = 0
        dataloader = tqdm(self.test_dataloader)
        loss = 0
        with torch.no_grad():
            for data in dataloader:
                inputs, labels = data[0].to(self.device), data[1].to(self.device)
                outputs = self.global_model(inputs)
                # print(outputs)
                # loss += torch.nn.CrossEntropyLoss(outputs,labels)
                _, predicted = torch.max(outputs.data, 1)
                total += labels.size(0)
                correct += (predicted == labels).sum().item()

        accuracy = 100 * correct / total
        if accuracy > self.best_test_acc:
            self.best_test_acc = accuracy
        self.test_loss.append(accuracy)

        print(f"Global Model Accuracy: {accuracy}%")
        print(f"Global Model Loss:{loss / total}")

    def agg(self, client_updates, client_weight):
        model_state = self.global_model.state_dict()
        w_sum = 0
        for w in client_weight:
            w_sum += w

        # 创建一个全为0的初始模型
        initial_model_state = {}
        for key in model_state:
            initial_model_state[key] = torch.zeros_like(model_state[key])

        for idx, client_update in enumerate(client_updates):
            for key in client_update:
                # print(model_state[key].dtype)
                if idx == 0:
                    initial_model_state[key] = torch.multiply(client_update[key], (client_weight[idx] / w_sum))
                else:
                    initial_model_state[key] += torch.multiply(client_update[key], (client_weight[idx] / w_sum))

        self.global_model.load_state_dict(initial_model_state)

    def select_clients(self,threadlist):

        selected_clients = random.sample(threadlist, self.num_select)
        return selected_clients


class DPParameterServer(ParameterServer):
    def __init__(self, config):
        super().__init__(config)
        self.epsilon = config["epsilon"]

    def init_client(self):

        trainset, test_set = get_dataset(config, True)

        num_client = config['num_clients']

        trainset_config = {'users': [],
                           'user_data': {},
                           'num_samples': []}

        config_division = {}  # Count of the classes for division
        config_class = {}  # Configuration of class distribution in clients
        config_data = {}  # Configuration of data indexes for each class : Config_data[cls] = [0, []] | pointer and indexes

        for i in range(num_client):
            config_class['f_{0:05d}'.format(i)] = []
            for j in range(config['num_classes']):
                cls = (i + j) % config['num_classes']
                if cls not in config_division:
                    config_division[cls] = 1
                    config_data[cls] = [0, []]

                else:
                    config_division[cls] += 1
                config_class['f_{0:05d}'.format(i)].append(cls)

        for cls in config_division.keys():
            indexes = torch.nonzero(torch.tensor(trainset.dataset.targets) == cls)
            num_datapoint = indexes.shape[0]
            indexes = indexes[torch.randperm(num_datapoint)]
            num_partition = num_datapoint // config_division[cls]
            for i_partition in range(config_division[cls]):
                if i_partition == config_division[cls] - 1:
                    config_data[cls][1].append(indexes[i_partition * num_partition:])
                else:
                    config_data[cls][1].append(indexes[i_partition * num_partition: (i_partition + 1) * num_partition])

        for user in config_class.keys():
            user_data_indexes = torch.tensor([])
            for cls in config_class[user]:
                user_data_index = config_data[cls][1][config_data[cls][0]]
                user_data_indexes = torch.cat((user_data_indexes, user_data_index))
                config_data[cls][0] += 1
            user_data_indexes = user_data_indexes.squeeze().int().tolist()
            user_data = Subset(trainset, user_data_indexes)
            # user_targets = trainset.target[user_data_indexes.tolist()]
            trainset_config['users'].append(user)
            trainset_config['user_data'][user] = user_data
            trainset_config['num_samples'] = len(user_data)

        for client_id in trainset_config['users']:
            client = DPLocalClient(client_id, '127.0.0.1', int('8010'), config, trainset_config['user_data'][client_id],
                                   self.epsilon)
            self.client_list.append(client)
        self.test_datset = test_set
        self.test_dataloader = DataLoader(self.test_datset, batch_size=self.config['batch_size'], shuffle=True)


class CoTrainParameterServer(DPParameterServer):
    def __init__(self, config):
        super().__init__(config)

    def select_clients(self, threadlist):
        # 创建一个列表，其中每个线程的权重与其resource_abundance值成正比
        weights = [thread.resource_abundance for thread in threadlist]

        # 使用random.choices()函数根据权重随机选择n个线程
        selected_clients = random.choices(threadlist, weights=weights, k=self.num_select)
        return selected_clients

    def CoTrain(self):

        # 对于所有可用的客户端
        print("Sending initial weight")

        for e in range(self.epoch):
            print(f"=====Epoch{e}=====")
            client_updates = []
            client_weight = []
            thread_list = []
            for i in range(len(self.client_list)):
                thread = CoTrainClientThread(self.client_list[i], copy.deepcopy(self.global_model.state_dict()))
                thread_list.append(thread)
                thread.start()

            for thread in self.select_clients(thread_list):
                thread.join()
                client_updates.append(copy.deepcopy(thread.output))
                client_weight.append(thread.weight)

            self.agg(client_updates, client_weight)

            self.test_global_model()

        print("=====Finish=====")
        print(f"Best ACC{self.best_test_acc}")
        print(f"Loss:{self.test_loss}")

if __name__ == '__main__':
    args = parse_arguments()
    config = vars(args)

    # torch.manual_seed(3047)
    set_seed(3047)
    if config["Algorithm"] == "FedAvg":
        server = ParameterServer(config)

    elif config["Algorithm"] == "DPFedAvg":
        server = DPParameterServer(config)

    elif config["Algorithm"] == "CoTrain":
        server = CoTrainParameterServer(config)
    else:
        assert "Alogorithm does not support"
    print(f"start {config['Algorithm']}")

    server.init_client()
    server.FedAvg()
