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
    def __init__(self, client_id, server_host, server_port, config: dict, dataset=None):
        super().__init__(client_id, server_host, server_port)
        self.config = config
        self.device = torch.device('cuda' if torch.cuda.is_available() else 'cpu')
        self.local_model = get_model(config).to(self.device)

        if dataset is None:
            self.local_dataset = get_dataset(config, True)
        else:
            self.local_dataset = dataset
        self.local_dataloader = DataLoader(self.local_dataset, batch_size=self.config['batch_size'], shuffle=True)
        self.optimizer = torch.optim.SGD(self.local_model.parameters(), lr=0.1)
        self.criterion = torch.nn.CrossEntropyLoss()

        # 隐私预算
        self.epsilon = 0.2

    # This is for FedAvg
    def local_train(self, global_model=None, bHasDP=False):
        # total_e = config['epoch']
        # while total_e > 0:
        # # 接收开始信号
        # start = self.receive_json()
        # # 在本地训练一个轮次
        # if start is None:
        #     continue

        # total_e -= 1
        if global_model is not None:
            # 使用全局模型更新本地模型
            self.local_model.load_state_dict(global_model.state_dict())

        for epoch in range(1):
            # print(f"Client {self.client_id} Local Train Epoch{epoch}")
            running_loss = 0.0
            dataloader = tqdm(self.local_dataloader) if self.client_id == 0 else self.local_dataloader
            for inputs, labels in dataloader:
                inputs = inputs.to(self.device)
                labels = labels.to(self.device)

                self.optimizer.zero_grad()
                outputs = self.local_model(inputs)
                loss = self.criterion(outputs, labels)
                loss.backward()
                self.optimizer.step()
                running_loss += loss.item()

            # 计算平均损失并打印
            epoch_loss = running_loss / len(self.local_dataset)
            print(f"Client {self.client_id} - Epoch {epoch + 1} Loss: {epoch_loss}")

        # 将模型参数的梯度上传给服务器
        # gradients = [param.grad for param in self.local_model.parameters()]
        #
        # if bHasDP:
        #     return add_laplace_noise(gradients,self.epsilon)

        return self.local_model.state_dict()
        # self.send_json(gradients)


class LocalClientThread(threading.Thread):
    def __init__(self, client, global_model):
        super().__init__()
        self.client = client
        self.global_model = global_model
        self.output = None

    def run(self):
        self.output = self.client.local_train(self.global_model)


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
