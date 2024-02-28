import torch
import torch.nn as nn
from torch.utils.data import DataLoader
from utils import train_one_epoch, tensor2json, json2tensor, add_laplace_noise, generate_gradient_with_noise, \
    generate_file_name, save_gradients_to_file, read_config_file
from train_config import *


# 假设有一个简单的模型类
class SimpleModel(nn.Module):
    def __init__(self):
        super(SimpleModel, self).__init__()
        self.fc = nn.Linear(10, 1)

    def forward(self, x):
        return self.fc(x)


# 假设有一个简单的数据集类
class SimpleDataset(torch.utils.data.Dataset):
    def __init__(self, data, targets):
        self.data = data
        self.targets = targets

    def __len__(self):
        return len(self.data)

    def __getitem__(self, index):
        return self.data[index], self.targets[index]


args = parse_arguments()

# 创建一个简单的数据集
data = torch.randn(100, 10)
targets = torch.randn(100, 1)
dataset = SimpleDataset(data, targets)

# 创建一个简单的数据加载器
dataloader = DataLoader(dataset, batch_size=10, shuffle=True)

# 创建一个简单的损失函数
loss_fn = nn.MSELoss()

# 创建一个简单的优化器
model = SimpleModel()
optimizer = torch.optim.SGD(model.parameters(), lr=0.01)

# 设置隐私参数
epsilon = 1.0
gradients = None
# 生成带噪声的梯度
model_grads = train_one_epoch(model, dataloader, gradients, optimizer, loss_fn)
noisy_gradients = add_laplace_noise(model_grads, epsilon)

config_path = "/Users/chenfeiyang/Documents/vscode/cfyserver/setting/ClientNode.json"
config = read_config_file(config_path)

gradient_path = config["GradientFilePath"]

# 获取当前时间作为文件名
file_name = generate_file_name()

file_path = gradient_path + "/" + file_name
# 将梯度存储到文件中
save_gradients_to_file(noisy_gradients, file_path)

print(train_one_epoch(model, dataloader, model_grads, optimizer, loss_fn))

print(1)
