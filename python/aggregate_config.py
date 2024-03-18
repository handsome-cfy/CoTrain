import argparse


def parse_arguments():
    # 创建参数解析器
    parser = argparse.ArgumentParser(description='示例脚本')

    # 添加命令行参数
    parser.add_argument('--gradient_path', type=str, help='梯度路径', default='')
    parser.add_argument('--model_name', type=str, help='模型名称', default='ResNet18')
    parser.add_argument('--dataset', type=str, help='数据集', default='CIFAR10')
    parser.add_argument('--Algorithm', type=str, help='聚合算法', default='FedAvg')
    parser.add_argument('--num_classes', type=int, help='类数量', default='10')
    parser.add_argument('--server_ip', type=str, help='ip', default='127.0.0.1')
    parser.add_argument('--server_port', type=str, help='port', default='8010')
    parser.add_argument('--num_clients', type=int, help='the number of the client', default=5)
    parser.add_argument('--epoch', type=int, default=100)
    parser.add_argument('--save_path', type=str, default="")
    parser.add_argument('--batch_size', type=int, default=64)
    parser.add_argument('--random_seed', type=int, default=3047)
    parser.add_argument('--epsilon', type=float, help="隐私预算", default=0.8)
    parser.add_argument('--num_select',type=int,help="每轮选择的设备数量",default=4)
    parser.add_argument('--device',type=int,default=0)

    # 解析命令行参数
    args = parser.parse_args()

    # 返回解析结果
    return args


if __name__ == '__main__':
    pass
    # 解析命令行参数
    # args = parser.parse_args()

    # 打印命令行参数
    # if args.name:
    #     print('姓名:', args.name)
    # if args.age:
    #     print('年龄:', args.age)
