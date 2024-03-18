import torch

from src.model import *
from src.dataset import *


def get_dataset(config: dict, train:bool, data_root=r"C:\tmp\public_dataset\pytorch") -> torch.utils.data.Dataset:
    name = config['dataset']
    # data_root = config['data_root']
    # train = config['train']
    download = True
    data_root = data_root

    if name == 'SimpleDataset':
        # 构建 SimpleDataset 数据集
        data = config['data']
        targets = config['targets']
        trainset = SimpleDataset(data, targets)
    elif name == 'CIFAR10':
        # 构建 CIFAR10 数据集
        trainset = Cifar10(data_root, train=True, download=download)
        testset = Cifar10(data_root, train=False, download=download)
    elif name == 'SVHN':
        # 构建 SVHN 数据集
        trainset = SVHN(data_root, train=True, download=download)
        testset = SVHN(data_root, train=False, download=download)
    elif name == 'MNIST':
        # 构建 MNIST 数据集
        trainset = MNIST(data_root, train=True, download=download)
        testset = MNIST(data_root, train=False, download=download)
    else:
        raise ValueError(f"Unsupported dataset: {name}")
    return trainset,testset

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
