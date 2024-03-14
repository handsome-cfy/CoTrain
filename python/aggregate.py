from aggregate_config import parse_arguments

if __name__ == '__main__':
    args = parse_arguments()
    args = vars(args)
    print(args['dataset'])