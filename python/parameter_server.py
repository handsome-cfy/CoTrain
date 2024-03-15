import json

import numpy as np
import torch
from torch import optim
from torch.utils.data import DataLoader
from torch.utils.data.dataset import Subset
from tqdm import tqdm

from aggregate_config import parse_arguments
from local_client import LocalClient, LocalClientThread
from my_socket import Server
from src.tool import get_model, get_dataset



class ParameterServer(Server):
    def __init__(self, config):
        super().__init__(config['server_ip'], int(config['server_port']))
        self.learning_rate = 0.1
        self.config = config
        self.device = torch.device('cuda' if torch.cuda.is_available() else 'cpu')

        self.global_model = get_model(config).to(self.device)
        self.num_clients = config['num_clients']
        self.epoch = config['epoch']
        self.optimizer = optim.SGD(self.global_model.parameters(), lr=0.1)

        self.test_datset = get_dataset(self.config,False)
        self.test_dataloader = DataLoader(self.test_datset, batch_size=self.config['batch_size'], shuffle=True)
        self.test_loss = []
        self.best_test_acc = 0


        self.client_list = []
        self.threads = []

    def init_client(self):

        dataset = get_dataset(config, True)

        num_clients = config['num_clients']

        # 计算每个客户端的样本数量
        num_samples = len(dataset)
        samples_per_client = [num_samples // num_clients] * num_clients

        # 对于剩余的样本数量，将其均匀分配给每个客户端
        remaining_samples = num_samples % num_clients
        for i in range(remaining_samples):
            samples_per_client[i] += 1

        for i in range(config['num_clients']):
            start_index = sum(samples_per_client[:i])
            end_index = start_index + samples_per_client[i]
            subset_indices = range(start_index, end_index)
            client_dataset = Subset(dataset, subset_indices)

            client = LocalClient(i, '127.0.0.1', int('8010'), config, client_dataset)
            self.client_list.append(client)
        #     thread = LocalClientThread(client)
        #     self.threads.append(thread)
        #     thread.start()
        #
        # for thread in self.threads:
        #     thread.join()

    def FedAvg(self):
        # # 分发初始模型
        # init_global_model_dict = self.global_model.state_dict()
        #
        # # 将 Tensor 对象转换成 NumPy 数组
        # init_global_model_dict_numpy = {}
        # for key, value in init_global_model_dict.items():
        #     init_global_model_dict_numpy[key] = value.detach().numpy().tolist()

        # 对于所有可用的客户端
        print("Sending initial weight")
        # for i in range(len(self.connected_socket)):
        #     self.send_json(self.connected_socket[i], json_str)

        for e in range(self.epoch):
            print(f"=====Epoch{e}=====")
            client_updates = []
            thread_list = []
            for i in range(len(self.client_list)):
                # print(f"Receive Data From Client{i}")
                thread = LocalClientThread(self.client_list[i], self.global_model)

                thread_list.append(thread)
                thread.start()

            for thread in thread_list:
                thread.join()
                client_updates.append(thread.output)

            self.aggregate_model_updates(client_updates)
            self.test_global_model()

        print("=====Finish=====")
        print(f"Best ACC{self.best_test_acc}")
        print(f"Loss:{self.test_loss}")

    def DP_FedAvg(self):
        for e in range(self.epoch):
            print(f"=====Epoch{e}=====")
            client_updates = []
            thread_list = []
            for i in range(len(self.client_list)):
                # print(f"Receive Data From Client{i}")
                thread = LocalClientThread(self.client_list[i], self.global_model)

                thread_list.append(thread)
                thread.start()

            for thread in thread_list:
                thread.join()
                client_updates.append(thread.output)

            self.aggregate_model_updates(client_updates)
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
                aggregated_gradient /= len(gradients_list)

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
        with torch.no_grad():
            for data in dataloader:
                inputs, labels = data[0].to(self.device), data[1].to(self.device)
                outputs = self.global_model(inputs)
                _, predicted = torch.max(outputs.data, 1)
                total += labels.size(0)
                correct += (predicted == labels).sum().item()

        accuracy = 100 * correct / total
        if accuracy > self.best_test_acc:
            self.best_test_acc = accuracy
        self.test_loss.append(accuracy)
        print(f"Global Model Accuracy: {accuracy}%")

if __name__ == '__main__':
    args = parse_arguments()
    config = vars(args)

    torch.manual_seed(config["random_seed"])
    server = ParameterServer(config)
    # server.start()
    # while len(server.connected_socket) < config['num_clients']:
    #     pass
    print("start fedavg")
    server.init_client()
    server.FedAvg()
