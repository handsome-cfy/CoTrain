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

            # client_socket.settimeout(1)
            # 存储连接的套接字和地址
            self.connected_socket.append(client_socket)
            self.connected_address.append(client_address)

            # 创建新线程来处理客户端请求
            # thread = threading.Thread(target=self.handle_client, args=(client_socket, client_address))
            # thread.start()

    def handle_client(self, client_socket, client_address):
        try:
            while True:
                pass
                # # 接收客户端发送的数据
                # data = client_socket.recv(1024).decode()
                # print(f"接收到来自客户端 {client_address} 的数据：{data}")
                #
                # if data == "quit":
                #     break
                #
                # # 进行计算，这里只是简单地将数据加倍并返回给客户端
                # result = int(data) * 2
                #
                # # 发送计算结果给客户端
                # response = str(result).encode()
                # client_socket.send(response)
                # print(f"向客户端 {client_address} 发送计算结果：{result}")
                # receive_json(client_socket, r'C:\Users\13391\Downloads\CoTrain-main\CoTrain-main\python\receive.json')

        except Exception as e:
            print(f"处理客户端请求时发生错误：{e}")

        finally:
            client_socket.close()
            print(f"与客户端 {client_address} 的连接已关闭。")

            # 从连接列表中删除已关闭的套接字和地址
            self.connected_socket.remove(client_socket)
            self.connected_address.remove(client_address)

    def stop(self):
        # 关闭服务器套接字连接
        self.server_socket.close()
        print("服务器已关闭。")

    def receive_json(self, client_socket, buffer_size=2^64, end_marker=b"<END>"):
        try:
            # 接收数据并解码为JSON
            data = b""
            while True:
                chunk = client_socket.recv(buffer_size)
                if not chunk:
                    break
                data += chunk
                if data.endswith(end_marker):
                    data = data[:-len(end_marker)]
                    break
            json_data = json.loads(data.decode())
            return json_data

        except Exception as e:
            print(f"接收JSON数据时发生错误：{e}")

    def send_json(self, client_socket, json_data, buffer_size=2^64, end_marker=b"<END>"):
        try:
            # 将JSON数据编码并分块发送给客户端
            data = json.dumps(json_data).encode() + end_marker
            total_size = len(data)
            sent_size = 0
            while sent_size < total_size:
                chunk = data[sent_size:sent_size + buffer_size]
                client_socket.send(chunk)
                sent_size += len(chunk)
            print("JSON数据发送完成。")

        except Exception as e:
            print(f"发送JSON数据时发生错误：{e}")


# def receive_json(client_socket, file_path, buffer_size=2^64):
#     try:
#         # 接收数据并解码为JSON
#         data = b""
#         while True:
#             chunk = client_socket.recv(buffer_size)
#             if not chunk:
#                 break
#             data += chunk
#         json_data = json.loads(data.decode())
#
#         # 将JSON数据写入文件
#         with open(file_path, 'w') as file:
#             json.dump(json_data, file)
#     except Exception as e:
#         print(f"接收JSON文件时发生错误：{e}")
#     finally:
#         # 关闭客户端套接字连接
#         # client_socket.close()
#         print("JSON文件接收完成。")
#
#
# def send_json(client_socket, file_path, buffer_size=2^64):
#     try:
#         # 读取JSON文件数据
#         with open(file_path, 'r') as file:
#             json_data = json.load(file)
#
#         # 将JSON数据编码并分块发送给客户端
#         data = json.dumps(json_data).encode()
#         total_size = len(data)
#         sent_size = 0
#         while sent_size < total_size:
#             chunk = data[sent_size:sent_size + buffer_size]
#             client_socket.send(chunk)
#             sent_size += len(chunk)
#     except Exception as e:
#         print(f"发送JSON文件时发生错误：{e}")
#     finally:
#         # 关闭客户端套接字连接
#         client_socket.close()
#         print("JSON文件发送完成。")


class Client:
    def __init__(self, client_id, server_host, server_port):
        self.client_id = client_id
        self.server_host = server_host
        self.server_port = server_port
        self.client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        # self.client_socket.connect((self.server_host, self.server_port))
        # self.client_socket.settimeout(1)

    def send_file(self, filename):
        try:
            # 发送文件名给服务器
            self.client_socket.send(filename.encode())

            # 读取文件内容并发送给服务器
            with open(filename, 'rb') as file:
                data = file.read()
                self.client_socket.sendall(data)

            print(f"文件 {filename} 已成功发送至服务器。")

        except Exception as e:
            print(f"发送文件时发生错误：{e}")

    def send_calculation_request(self, data):
        try:
            # 发送计算请求给服务器
            self.client_socket.send(data.encode())

            # 接收计算结果
            result = self.client_socket.recv(1024).decode()
            print(f"接收到计算结果：{result}")

        except Exception as e:
            print(f"发送计算请求时发生错误：{e}")

    def receive_json(self, buffer_size=2^64, end_marker=b"<END>"):
        try:
            # Receive data and decode it as JSON
            data = b""
            while True:
                chunk = self.client_socket.recv(buffer_size)
                if not chunk:
                    break
                data += chunk
                if data.endswith(end_marker):
                    data = data[:-len(end_marker)]
                    break
            json_data = json.loads(data.decode())
            return json_data

        except Exception as e:
            print(f"An error occurred while receiving JSON data: {e}")

    def send_json(self, json_data, buffer_size=2^64, end_marker=b"<END>"):
        try:
            # Encode the JSON data and send it to the client in chunks
            data = json.dumps(json_data).encode() + end_marker
            total_size = len(data)
            sent_size = 0
            while sent_size < total_size:
                chunk = data[sent_size:sent_size + buffer_size]
                sent_bytes = self.client_socket.send(chunk)
                sent_size += sent_bytes
            print("JSON data has been sent.")

        except Exception as e:
            print(f"An error occurred while sending JSON data: {e}")

    def close(self):
        # 关闭客户端套接字连接
        self.client_socket.close()
        print("客户端已关闭。")
