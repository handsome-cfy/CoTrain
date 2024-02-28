import torch
import torch.nn as nn
from torch.utils.data import DataLoader
from utils import train_one_epoch, tensor2json, json2tensor, add_laplace_noise


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


# 测试 train_one_epoch 函数
def test_train_one_epoch():
    # 进行训练一个 epoch
    gradients = [torch.randn_like(param) for param in model.parameters()]
    gradients = None
    model.train()
    model_grads = train_one_epoch(model, dataloader, gradients, optimizer, loss_fn)

    # 打印模型参数的梯度
    print("模型参数的梯度：")
    for param, grad in zip(model.parameters(), model_grads):
        print(param, grad)


def test_tensor2json():
    gradients = [torch.randn_like(param) for param in model.parameters()]
    model.train()
    model_grads = train_one_epoch(model, dataloader, gradients, optimizer, loss_fn)
    json_gradient = tensor2json(model_grads)
    print("梯度的JSON表示：")
    print(json_gradient)


def test_json2tensor():
    gradients = [torch.randn_like(param) for param in model.parameters()]
    model.train()
    model_grads = train_one_epoch(model, dataloader, gradients, optimizer, loss_fn)
    json_gradient = tensor2json(model_grads)
    gradient = json2tensor(json_gradient)
    print("从JSON恢复的梯度：")
    for grad in gradient:
        print(grad)


def test_add_laplace_noise():
    gradients = [torch.randn_like(param) for param in model.parameters()]
    epsilon = 1.0
    noisy_gradient = add_laplace_noise(gradients, epsilon)
    print("添加拉普拉斯噪声后的梯度：")
    for noisy_grad in noisy_gradient:
        print(noisy_grad)


test_train_one_epoch()
test_tensor2json()
test_json2tensor()
test_add_laplace_noise()
