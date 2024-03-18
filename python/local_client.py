import copy
import os
import random
import re
import shutil
import threading

import torch
from torch.utils.data import DataLoader
from torch.utils.data.dataset import Subset
from tqdm import tqdm

from aggregate_config import parse_arguments
from my_socket import Client
from src.tool import get_model, get_dataset
from utils import add_laplace_noise


class LocalClient(Client):
    def __init__(self, client_id, server_host, server_port, config: dict, dataset=None, weight=1):
        super().__init__(client_id, server_host, server_port)
        self.config = config
        self.device = torch.device('cuda' if torch.cuda.is_available() else 'cpu')
        self.local_model = get_model(config).to(self.device)

        if dataset is None:
            self.local_dataset = get_dataset(config, True)
        else:
            self.local_dataset = dataset

        if weight != 1:
            self.weight = weight
        else:
            self.weight = len(dataset)

        self.local_dataloader = DataLoader(self.local_dataset, batch_size=self.config['batch_size'], shuffle=True)
        self.optimizer = torch.optim.SGD(self.local_model.parameters(), lr=0.1)
        self.criterion = torch.nn.CrossEntropyLoss()

    # This is for FedAvg
    def local_train(self, global_model=None, bHasDP=False):

        # total_e -= 1
        if global_model is not None:
            # 使用全局模型更新本地模型
            self.local_model.load_state_dict(global_model)

        for epoch in range(1):
            # print(f"Client {self.client_id} Local Train Epoch{epoch}")
            running_loss = 0.0
            dataloader = tqdm(self.local_dataloader) if self.client_id == "f_00000" else self.local_dataloader
            for inputs, labels in dataloader:
                inputs = inputs.to(self.device)
                labels = labels.to(self.device)

                outputs = self.local_model(inputs)
                loss = self.criterion(outputs, labels)
                self.optimizer.zero_grad()
                loss.backward()
                self.optimizer.step()
                running_loss += loss.item()
                # break

            # 计算平均损失并打印
            epoch_loss = running_loss / len(self.local_dataset)
            print(f"Client {self.client_id} - Epoch {epoch + 1} Loss: {epoch_loss}")

        # 将模型参数的梯度上传给服务器
        # gradients = [param.grad for param in self.local_model.parameters()]
        #
        # if bHasDP:
        #     return add_laplace_noise(gradients,self.epsilon)

        return self.local_model.state_dict(), self.weight
        # self.send_json(gradients)


class LocalClientThread(threading.Thread):
    def __init__(self, client, global_model):
        super().__init__()
        self.client = client
        self.global_model = global_model
        self.output = None
        self.weight = None

    def run(self):
        self.output, self.weight = self.client.local_train(copy.deepcopy(self.global_model))


class DPLocalClient(LocalClient):
    def __init__(self, client_id, server_host, server_port, config: dict, dataset=None, weight=1, epsilon=-1):
        super().__init__(client_id, server_host, server_port, config, dataset, weight)
        # 隐私预算
        if epsilon == -1:
            self.epsilon = config['epsilon']
        else:
            self.epsilon = epsilon

    def add_laplace_noise_to_state_dict(self, state_dict, epsilon):
        parameter = self.local_model.parameters()
        norms = [float(p.data.norm(2)) for p in parameter]
        max_norm = torch.median(torch.tensor(norms))

        for name, param in state_dict.items():
            if random.random() < 0.2:
                laplace_dist = torch.distributions.Laplace(0, epsilon)
                noise = laplace_dist.sample(param.size()).to(param.device)
                max_norm_clip = max(1, param.data.float().norm(2) / max_norm)
                x = state_dict[name]
                noisy_param = state_dict[name] + noise * 0.005
                # print("1")
                # noisy_param = state_dict[name] + 0

                # print(state_dict[name])
                # print(noisy_param)
                # print(f"Differ for self:{torch.abs(state_dict[name]-state_dict[name])}")
                # print(f"Differ for noise:{torch.abs(state_dict[name]-noisy_param)}")
                # print(f"Cosine Similarity:{torch.cosine_similarity(state_dict[name].unsqueeze(0), noisy_param.unsqueeze(0))}")
                state_dict[name] = noisy_param

        return state_dict

    def local_train(self, global_model=None, bHasDP=True):

        if global_model is not None:
            # 使用全局模型更新本地模型
            self.local_model.load_state_dict(global_model)

        for epoch in range(1):
            running_loss = 0.0
            dataloader = tqdm(self.local_dataloader) if self.client_id == "f_00000" else self.local_dataloader
            for inputs, labels in dataloader:
                inputs = inputs.to(self.device)
                labels = labels.to(self.device)

                outputs = self.local_model(inputs)
                loss = self.criterion(outputs, labels)
                self.optimizer.zero_grad()
                loss.backward()
                self.optimizer.step()
                running_loss += loss.item()

            # 计算平均损失并打印
            epoch_loss = running_loss / len(self.local_dataset)
            print(f"Client {self.client_id} - Epoch {epoch + 1} Loss: {epoch_loss}")

        return self.add_laplace_noise_to_state_dict(self.local_model.state_dict(), self.epsilon), self.weight


class CoTrainLocalClient(DPLocalClient):
    def __init__(self, client_id, server_host, server_port, config: dict, dataset=None, weight=1, storage_capacity=0,
                 computation_capacity=0, bandwidth_capacity=0, data_capacity=0):
        super().__init__(client_id, server_host, server_port, config, dataset, weight)

        if storage_capacity == 0:
            # 添加资源丰富度的属性
            self.storage_capacity = storage_capacity  # 存储容量
            self.computation_capacity = computation_capacity  # 计算容量
            self.bandwidth_capacity = bandwidth_capacity  # 带宽容量
            self.data_capacity = data_capacity  # 数据容量
        else:
            self.storage_capacity = self.measure_storage_capacity()  # 存储容量
            self.computation_capacity = self.measure_computation_capacity()  # 计算容量
            self.bandwidth_capacity = self.measure_bandwidth_capacity()  # 带宽容量
            self.data_capacity = self.measure_data_capacity()  # 数据容量

        # 计算归一化后的资源丰富度
        self.resource_abundance = self.calculate_resource_abundance()

    def measure_storage_capacity(self):
        # 假设节点的存储容量是磁盘空间可用容量的大小，单位为字节
        total, used, free = shutil.disk_usage("/")
        storage_capacity = free
        return storage_capacity

    def measure_computation_capacity(self):
        # 通过获取处理器型号和速度来确定计算能力
        # 假设计算容量是处理器速度的值，单位为GHz
        cpu_info = os.popen('lscpu').read()
        cpu_speed = re.search(r"CPU MHz:\s+(.*)", cpu_info)
        computation_capacity = float(cpu_speed.group(1)) / 1000.0
        return computation_capacity

    def measure_bandwidth_capacity(self):
        # 测量带宽容量的方法是通过进行网络速度测试来确定带宽能力
        # 假设带宽容量是网络速度的值，单位为Mbps
        speedtest_result = os.popen('speedtest-cli --simple').read()
        download_speed = re.search(r"Download:\s+(.*)", speedtest_result)
        upload_speed = re.search(r"Upload:\s+(.*)", speedtest_result)
        bandwidth_capacity = min(float(download_speed.group(1)), float(upload_speed.group(1)))
        return bandwidth_capacity

    def measure_data_capacity(self):
        # 数据集大小来确定数据容量
        if self.dataset:
            data_capacity = len(self.dataset)
        else:
            data_capacity = 0
        return data_capacity

    def calculate_resource_abundance(self):
        # 对四个量进行归一化处理
        max_capacity = max(self.storage_capacity, self.computation_capacity, self.bandwidth_capacity,
                           self.data_capacity)
        min_capacity = min(self.storage_capacity, self.computation_capacity, self.bandwidth_capacity,
                           self.data_capacity)

        normalized_storage_capacity = (self.storage_capacity - min_capacity) / (max_capacity - min_capacity)
        normalized_computation_capacity = (self.computation_capacity - min_capacity) / (max_capacity - min_capacity)
        normalized_bandwidth_capacity = (self.bandwidth_capacity - min_capacity) / (max_capacity - min_capacity)
        normalized_data_capacity = (self.data_capacity - min_capacity) / (max_capacity - min_capacity)

        # 计算资源丰富度，四个量的平均值
        resource_abundance = (
                                     normalized_storage_capacity + normalized_computation_capacity + normalized_bandwidth_capacity + normalized_data_capacity) / 4
        return resource_abundance


class CoTrainClientThread(LocalClientThread):
    def __init__(self, client, global_model, resource_abundance):
        super().__init__(client, global_model)
        self.resource_abundance = resource_abundance


if __name__ == '__main__':
    args = parse_arguments()
    config = vars(args)
    client_list = []
    threads = []

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
        client_list.append(client)
        thread = LocalClientThread(client)
        threads.append(thread)
        thread.start()

    for thread in threads:
        thread.join()
