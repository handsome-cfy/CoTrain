import torch
import torchvision


class SimpleDataset(torch.utils.data.Dataset):
    def __init__(self, data, targets):
        self.data = data
        self.targets = targets

    def __len__(self):
        return len(self.data)

    def __getitem__(self, index):
        return self.data[index], self.targets[index]


class Cifar10(torch.utils.data.Dataset):
    def __init__(self, data_root, train=True, download=True):
        self.dataset = torchvision.datasets.CIFAR10(
            root=data_root, train=train, download=download,
        )
        # self.OOD_dataset = torchvision.datasets.SVHN(
        #     root=data_root, split="train" if train else "test", download=download,
        # )

        self.len = len(self.dataset)

        self.transform = torchvision.transforms.Compose([
            torchvision.transforms.ToTensor()
            , torchvision.transforms.RandomCrop(32, padding=4)
            , torchvision.transforms.RandomHorizontalFlip(p=0.5)
            , torchvision.transforms.Normalize([0.485, 0.456, 0.406], [0.229, 0.224, 0.225])
        ])

    def __getitem__(self, item):
        img, label = self.dataset[item]

        return self.transform(img), label

    def __len__(self):
        return self.len


class SVHN(torch.utils.data.Dataset):
    def __init__(self, data_root, train=True, download=True):
        self.dataset = torchvision.datasets.SVHN(
            root=data_root, split="train" if train else "test", download=download,
        )

        self.len = len(self.dataset)

        self.transform = torchvision.transforms.Compose([
            torchvision.transforms.ToTensor()
        ])

    def __getitem__(self, item):
        img, label = self.dataset[item]

        return self.transform(img), label

    def __len__(self):
        return self.len


class MNIST(torch.utils.data.Dataset):
    def __init__(self, data_root, train=True, download=True):
        self.dataset = torchvision.datasets.MNIST(
            root=data_root, train=train, download=download,
        )

        self.len = len(self.dataset)

        self.transform = torchvision.transforms.Compose([
            torchvision.transforms.ToTensor()
        ])

    def __getitem__(self, item):
        img, label = self.dataset[item]

        return self.transform(img), label

    def __len__(self):
        return self.len
