import json

import numpy as np
from torch import optim

from aggregate_config import parse_arguments
from my_socket import Server
from src.tool import get_model


class ParameterServer(Server):
    def __init__(self, config):
        super().__init__(config['server_ip'], int(config['server_port']))
        self.config = config
        self.global_model = get_model(config)
        self.num_clients = config['num_clients']
        self.epoch = config['epoch']
        self.optimizer = optim.SGD(self.global_model.parameters(), lr=0.1)

    def FedAvg(self):
        # # 分发初始模型
        # init_global_model_dict = self.global_model.state_dict()
        #
        # # 将 Tensor 对象转换成 NumPy 数组
        # init_global_model_dict_numpy = {}
        # for key, value in init_global_model_dict.items():
        #     init_global_model_dict_numpy[key] = value.detach().numpy().tolist()

        # 将基于 NumPy 数组的字典序列化成 JSON 字符串
        json_str = json.dumps("{}")

        # 对于所有可用的客户端
        print("Sending initial weight")
        for i in range(len(self.connected_socket)):
            self.send_json(self.connected_socket[i], json_str)

        for e in range(self.epoch):
            print(f"=====Epoch{e}=====")
            client_updates = []
            for i in range(len(self.connected_socket)):
                print(f"Receive Data From Client{i}")
                client_updates.append(self.receive_json(self.connected_socket[i]))
            print(f"Aggregate")
            self.aggregate_model_updates(client_updates)

    def aggregate_model_updates(self, client_updates):
        # 初始化全局模型参数梯度
        self.optimizer.zero_grad()

        # 聚合客户端模型更新
        for client_update in client_updates:
            client_update.backward()  # 累积客户端模型梯度

        # 平均模型参数梯度
        for param in self.global_model.parameters():
            param.grad /= self.num_clients

        # 更新全局模型
        self.optimizer.step()


if __name__ == '__main__':
    args = parse_arguments()
    config = vars(args)

    server = ParameterServer(config)
    server.start()
    while len(server.connected_socket) < 1:
        pass
    print("start fedavg")
    server.FedAvg()
