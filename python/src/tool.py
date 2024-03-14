import torch

from src.model import *
from src.dataset import *


def get_dataset(config: dict, train:bool, data_root="/tmp/public_dataset/pytorch") -> torch.utils.data.Dataset:
    name = config['dataset']
    # data_root = config['data_root']
    # train = config['train']
    download = True
    data_root = data_root

    if name == 'SimpleDataset':
        # 构建 SimpleDataset 数据集
        data = config['data']
        targets = config['targets']
        dataset = SimpleDataset(data, targets)
    elif name == 'CIFAR10':
        # 构建 CIFAR10 数据集
        dataset = Cifar10(data_root, train=train, download=download)
    elif name == 'SVHN':
        # 构建 SVHN 数据集
        dataset = SVHN(data_root, train=train, download=download)
    elif name == 'MNIST':
        # 构建 MNIST 数据集
        dataset = MNIST(data_root, train=train, download=download)
    else:
        raise ValueError(f"Unsupported dataset: {name}")
    return dataset

def get_model(config: dict) -> nn.Module:
    name = config['model_name']
    num_classes = config['num_classes']
    if name == 'ResNet18':
        model = ResNet18(num_classes)
    elif name == 'VGG16':
        model = VGG16(num_classes)
    elif name == 'DenseNet121':
        model = DenseNet121(num_classes)
    elif name == 'MobileNetV2':
        model = MobileNetV2(num_classes)
    else:
        raise ValueError(f"Unsupported model: {name}")

    return model
