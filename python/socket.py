import json

import socket
import threading


class Server:
    def __init__(self, host, port):
        self.host = host
        self.port = port
        self.server_socket = None
        self.connected_socket = []
        self.connected_address = []

    def start(self):
        # 创建套接字对象
        self.server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        # 绑定地址和端口
        self.server_socket.bind((self.host, self.port))
        # 开始监听连接请求
        self.server_socket.listen(1)
        print(f"服务器正在监听端口 {self.port}...")

        # 创建新线程来处理客户端连接
        thread = threading.Thread(target=self.handle_client_connections)
        thread.start()

    def handle_client_connections(self):
        while True:
            # 接受客户端的连接请求
            client_socket, client_address = self.server_socket.accept()
            print(f"与客户端 {client_address} 建立连接！")

            # 存储连接的套接字和地址
            self.connected_socket.append(client_socket)
            self.connected_address.append(client_address)

            # # 创建新线程来处理客户端请求
            # thread = threading.Thread(target=self.handle_client, args=(client_socket, client_address))
            # thread.start()

    def handle_client(self, client_socket, client_address):
        # 接收客户端发送的数据
        data = client_socket.recv(1024).decode()
        print(f"接收到来自客户端 {client_address} 的数据：{data}")

        # 做一些处理，这里只是简单地将数据原样返回给客户端
        response = data.encode()

        # 发送响应给客户端
        client_socket.send(response)
        print(f"向客户端 {client_address} 发送响应：{response.decode()}")

        # 关闭客户端套接字连接
        client_socket.close()

    def stop(self):
        # 关闭服务器套接字连接
        self.server_socket.close()
        print("服务器已关闭。")


def receive_json(client_socket, file_path, buffer_size=1024):
    try:
        # 接收数据并解码为JSON
        data = b""
        while True:
            chunk = client_socket.recv(buffer_size)
            if not chunk:
                break
            data += chunk
        json_data = json.loads(data.decode())

        # 将JSON数据写入文件
        with open(file_path, 'w') as file:
            json.dump(json_data, file)
    except Exception as e:
        print(f"接收JSON文件时发生错误：{e}")
    finally:
        # 关闭客户端套接字连接
        client_socket.close()
        print("JSON文件接收完成。")


def send_json(client_socket, file_path, buffer_size=1024):
    try:
        # 读取JSON文件数据
        with open(file_path, 'r') as file:
            json_data = json.load(file)

        # 将JSON数据编码并分块发送给客户端
        data = json.dumps(json_data).encode()
        total_size = len(data)
        sent_size = 0
        while sent_size < total_size:
            chunk = data[sent_size:sent_size + buffer_size]
            client_socket.send(chunk)
            sent_size += len(chunk)
    except Exception as e:
        print(f"发送JSON文件时发生错误：{e}")
    finally:
        # 关闭客户端套接字连接
        client_socket.close()
        print("JSON文件发送完成。")


