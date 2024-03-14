import torch

from model import *
from dataset import *


def get_dataset(config:dict)->torch.utils.data.Dataset:
    name = config['dataset']
    if name == 'CIFAR10':
        return Cifar10(''),Cifar10('',False)