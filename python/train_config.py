import argparse


def parse_arguments():
    # 创建参数解析器
    parser = argparse.ArgumentParser(description='示例脚本')

    # 添加命令行参数
    parser.add_argument('--gradient_path', type=str, help='梯度路径')
    parser.add_argument('--model_name', type=str, help='模型名称')

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
