import torch
from torch import nn
from torchvision import models


class ResNet18(nn.Module):
    def __init__(self, num_classes):
        super(ResNet18, self).__init__()
        self.resnet = models.resnet18(pretrained=True)
        self.resnet.fc = nn.Linear(512, num_classes)  # 替换最后一层全连接层

    def forward(self, x):
        return self.resnet(x)


class VGG16(nn.Module):
    def __init__(self, num_classes):
        super(VGG16, self).__init__()
        self.vgg = models.vgg16(pretrained=True)
        self.vgg.classifier[6] = nn.Linear(4096, num_classes)  # 替换最后一层全连接层

    def forward(self, x):
        return self.vgg(x)


class DenseNet121(nn.Module):
    def __init__(self, num_classes):
        super(DenseNet121, self).__init__()
        self.densenet = models.densenet121(pretrained=True)
        self.densenet.classifier = nn.Linear(1024, num_classes)  # 替换最后一层全连接层

    def forward(self, x):
        return self.densenet(x)


class MobileNetV2(nn.Module):
    def __init__(self, num_classes):
        super(MobileNetV2, self).__init__()
        self.mobilenet = models.mobilenet_v2(pretrained=True)
        self.mobilenet.classifier[1] = nn.Linear(1280, num_classes)  # 替换最后一层全连接层

    def forward(self, x):
        return self.mobilenet(x)
