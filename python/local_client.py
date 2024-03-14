import threading

import torch
from torch.utils.data import DataLoader
from tqdm import tqdm

from aggregate_config import parse_arguments
from my_socket import Client
from src.tool import get_model, get_dataset


class LocalClient(Client):
    def __init__(self, client_id, server_host, server_port, config: dict):
        super().__init__(client_id, server_host, server_port)
        self.config = config
        self.local_model = get_model(config)
        self.local_dataset = get_dataset(config,True)
        self.local_dataloader = DataLoader(self.local_dataset, batch_size=self.config['batch_size'], shuffle=True)
        self.optimizer = torch.optim.SGD(self.local_model.parameters(), lr=0.1)
        self.criterion = torch.nn.CrossEntropyLoss()

    # This is for FedAvg
    def local_train(self):
        total_e = config['epoch']
        while total_e > 0:
            total_e -= 1
            # 接收开始信号
            start = self.receive_json(self.client_socket)
            # 在本地训练一个轮次

            for epoch in range(1):
                print(f"Client {self.client_id} Local Train Epoch{epoch}")
                running_loss = 0.0
                dataloader = tqdm(self.local_dataloader) if self.client_id == 0 else self.local_dataloader
                for inputs, labels in dataloader:
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
            gradients = [param.grad.numpy() for param in self.local_model.parameters()]
            self.send_json(self.client_socket, gradients)


class LocalClientThread(threading.Thread):
    def __init__(self, client):
        super().__init__()
        self.client = client

    def run(self):
        self.client.local_train()


if __name__ == '__main__':
    args = parse_arguments()
    config = vars(args)
    client_list = []
    threads = []

    for i in range(1):
        client = LocalClient(i, '127.0.0.1', int('8010'), config)
        client_list.append(client)
        thread = LocalClientThread(client)
        threads.append(thread)
        thread.start()

    for thread in threads:
        thread.join()
