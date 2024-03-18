import json
import random
from typing import List

import numpy as np
import torch
import torch.nn as nn
from torch import Tensor


def train_one_epoch(model: nn.Module, dataloader, gradients: List[torch.Tensor] = None,
                    optimizer: torch.optim.Optimizer = None, loss_fn: nn.Module = None) -> List[Tensor]:
    '''用来训练一个epoch、传入中央服务器给的梯度、如果为空那么就使用上一轮保存的梯度
        返回值是这一轮最后的模型梯度
    '''
    # 迭代数据加载器，计算损失并进行反向传播
    if gradients is not None:
        for param, gradient in zip(model.parameters(), gradients):
            param.grad = gradient

    for inputs, targets in dataloader:
        # 前向传播
        outputs = model(inputs)
        if loss_fn is not None:
            loss = loss_fn(outputs, targets)  # 使用传入的损失函数计算损失

            # 反向传播及优化
            loss.backward()

        if optimizer is not None:
            optimizer.step()

            # 清空梯度
            # optimizer.zero_grad()
    # 返回本轮最后的模型梯度
    return [param.grad.clone() for param in model.parameters()]


def tensor2json(gradient: List[Tensor]):
    # 将梯度张量转换为可序列化的列表
    serialized_gradient = [tensor.tolist() for tensor in gradient]

    # 转换为JSON格式的字符串
    json_gradient = json.dumps(serialized_gradient)

    return json_gradient


def json2tensor(json_gradient: str) -> List[Tensor]:
    # 从JSON字符串解析为Python对象
    serialized_gradient = json.loads(json_gradient)

    # 将列表中的每个元素转换为张量
    gradient = [torch.tensor(tensor) for tensor in serialized_gradient]

    return gradient


def add_laplace_noise(gradient: List[Tensor], epsilon: float) -> List[Tensor]:
    noisy_gradient = []
    for tensor in gradient:
        # 计算噪声的尺度
        sensitivity = tensor.norm(1)  # 计算张量的L1范数作为敏感性
        scale = sensitivity / epsilon

        # 生成噪声并添加到梯度中
        noise = torch.randn_like(tensor) * scale
        noisy_tensor = tensor + noise

        noisy_gradient.append(noisy_tensor)

    return noisy_gradient


def save_gradients_to_file(gradients, file_path):
    json_data = json.dumps([grad.tolist() for grad in gradients])

    with open(file_path, 'w') as file:
        file.write(json_data)


def generate_gradient_with_noise(model, dataloader, epsilon):
    gradients = train_one_epoch(model, dataloader)

    noisy_gradients = add_laplace_noise(gradients, epsilon)

    return noisy_gradients


def read_config_file(file_path="../setting/ClientNode.json"):
    with open(file_path, 'r') as file:
        config = json.load(file)
    return config


import time


def generate_file_name():
    # 获取当前时间
    current_time = time.localtime()

    # 构造文件名
    file_name = f"file_{current_time.tm_year}-{current_time.tm_mon + 1:02d}-{current_time.tm_mday:02d}_{current_time.tm_hour:02d}-{current_time.tm_min:02d}-{current_time.tm_sec:02d}.json"

    return file_name


def save_json_to_file(data, file_path):
    with open(file_path, 'w') as file:
        json.dump(data, file)

def set_seed(seed):
    random.seed(seed)
    np.random.seed(seed)
    torch.manual_seed(seed)
    torch.cuda.manual_seed_all(seed)  # 如果你在使用GPU，还需要设置CUDA的随机种子
    torch.backends.cudnn.deterministic = True
    torch.backends.cudnn.benchmark = False