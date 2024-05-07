import serial
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D


class RealTime1DPlot:
    def __init__(self):
        # 创建一个坐标系
        self.fig, self.ax = plt.subplots()

        # 设置坐标轴标签
        self.ax.set_xlabel('Time')
        self.ax.set_ylabel('Value')

        # 初始化坐标点
        self.x = 0
        self.x_data = []
        self.y_data = []
        self.scatter = self.ax.scatter([], [])

    def update_plot(self, new_points, ax):
        for point in new_points:
            # 添加新的点
            self.x_data.append(self.x)
            self.y_data.append(point[ax])
            self.x += 1


        # 更新散点图
        self.scatter.remove()
        self.scatter = self.ax.scatter(self.x_data, self.y_data, 1)

        # 重新绘制
        self.fig.canvas.draw()
        plt.pause(0.1)  # 添加短暂的延迟，以确保图形更新

        # # 设置坐标轴范围，使其一致
        # self.ax.set_xlim(0, self.x)
        # self.ax.set_ylim(min(self.y_data), min(self.y_data))

class RealTime2DPlot:
    def __init__(self):
        # 创建一个坐标系
        self.fig, self.ax = plt.subplots()

        # 设置坐标轴标签
        self.ax.set_xlabel('X')
        self.ax.set_ylabel('Y')

        # 初始化坐标点
        self.x_data = []
        self.y_data = []
        self.scatter = self.ax.scatter([], [])

    def update_plot(self, new_points, x, y):
        for point in new_points:
            # 添加新的点
            self.x_data.append(point[x])
            self.y_data.append(point[y])

        # 更新散点图
        self.scatter.remove()
        self.scatter = self.ax.scatter(self.x_data, self.y_data, 1)

        # 重新绘制
        self.fig.canvas.draw()
        plt.pause(0.1)  # 添加短暂的延迟，以确保图形更新

        # # 设置坐标轴范围，使其一致
        # self.ax.set_xlim(0, self.x)
        # self.ax.set_ylim(min(self.y_data), min(self.y_data))

class RealTime3DPlot:
    def __init__(self):
        # 创建一个3D坐标系
        self.fig = plt.figure()
        self.ax = self.fig.add_subplot(111, projection='3d')

        # 设置坐标轴标签
        self.ax.set_xlabel('X')
        self.ax.set_ylabel('Y')
        self.ax.set_zlabel('Z')

        # 初始化坐标点
        self.x_data = []
        self.y_data = []
        self.z_data = []
        self.scatter = self.ax.scatter([], [], [])

    def update_plot(self, new_points):
        for point in new_points:
            # 添加新的点
            self.x_data.append(point[0])
            self.y_data.append(point[1])
            self.z_data.append(point[2])


        # 更新散点图
        self.scatter.remove()
        self.scatter = self.ax.scatter(self.x_data, self.y_data, self.z_data)

        # 重新绘制
        self.fig.canvas.draw()
        plt.pause(0.1)  # 添加短暂的延迟，以确保图形更新

        # 设置坐标轴范围，使其一致
        ax_len = max(max(self.x_data)-min(self.x_data), max(self.y_data)-min(self.y_data), max(self.z_data)-min(self.z_data))
        self.ax.set_xlim(min(self.x_data), min(self.x_data)+ax_len)
        self.ax.set_ylim(min(self.y_data), min(self.y_data)+ax_len)
        self.ax.set_zlim(min(self.z_data), min(self.z_data)+ax_len)


import socket
import struct

# 每次绘制的点数
drawpointsnum = 300
# 绘制的图像
drawxD = 1
drawAx = 0
drawAy = 1
# 要绘制的类型
drawType = 1 # 0:加速度、1:速度、2:位置

# 服务器地址和端口
server_address = ('192.168.1.245', 56050)
# 创建TCP套接字
server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
# 绑定套接字到指定地址和端口
server_socket.bind(server_address)
# 监听连接，最大连接数为1
server_socket.listen(1)
print(f"等待ESP32客户端连接在 {server_address}")
# 等待客户端连接
client_socket, client_address = server_socket.accept()
print(f"接受来自 {client_address} 的连接")

if 1 == drawxD:
    # 创建RealTime1DPlot对象
    real_time_1d_plot = RealTime1DPlot()
elif 2== drawxD:
    # 创建RealTime2DPlot对象
    real_time_2d_plot = RealTime2DPlot()
elif 3== drawxD:
    # 创建RealTime3DPlot对象
    real_time_3d_plot = RealTime3DPlot()


try:
    last_data = b''  # 使用bytes类型作为缓冲区
    pointnum = 0 # 数据计数
    thisPos = [0.0,0.0,0.0]
    thisVel = [0.0,0.0,0.0]
    thisAcc = [0.0,0.0,0.0]
    lastPos = [0.0,0.0,0.0]
    lastVel = [0.0,0.0,0.0]
    lastAcc = [0.0,0.0,0.0]
    points = [[0.0, 0.0, 0.0] for _ in range(drawpointsnum)]
    while True:
        # 接收客户端发送的数据
        data = client_socket.recv(1024)
        if not data:
            break  # 没有数据表示连接已关闭
        # 将接收到的字节数据添加到缓冲区
        complete_data = last_data + data
        # 将缓冲区的数据解码为字符串
        received_data = complete_data.decode('utf-8')
        # 使用逗号和分号分隔数据
        components = received_data.split(';')
        # 处理完整的数据段，最后一个可能是不完整的
        for i in range(len(components) - 1):
            component = components[i]
            if component:
                values = component.split(',')
                if len(values) == 3:
                    try:
                        x, y, z = map(float, values)
                        # print(f"接收到数据: X={x}, Y={y}, Z={z}")
                        thisAcc = [x,y,z]
                        thisVel = [lastVel[0]+thisAcc[0],lastVel[1]+thisAcc[1],lastVel[2]+thisAcc[2]]
                        thisPos = [lastPos[0]+thisVel[0],lastPos[1]+thisVel[1],lastPos[2]+thisVel[2]]
                        lastAcc = thisAcc
                        lastVel = thisVel
                        lastPos = thisPos
                        if 0 == drawType: # 绘制加速度数据
                            points[pointnum] = thisAcc
                        elif 1 == drawType: # 绘制速度数据
                            points[pointnum] = thisVel 
                        elif 2 == drawType: # 绘制速度数据
                            points[pointnum] = thisPos
                        pointnum+=1
                        # print(pointnum)
                    except:
                        print(f"无效的数据格式: {component}")
                else:
                    print(f"无效的数据格式: {component}")
            if(drawpointsnum == pointnum):
                print(pointnum)
                # 每100个点绘制一次图像
                if 1 == drawxD:
                    real_time_1d_plot.update_plot(points,drawAx)
                elif 2== drawxD:
                    real_time_2d_plot.update_plot(points,drawAx,drawAy)
                elif 3== drawxD:
                    real_time_3d_plot.update_plot(points)
                    
                pointnum = 0
                points = [[0.0, 0.0, 0.0] for _ in range(drawpointsnum)]
        # 更新缓冲区，将最后一个可能不完整的数据保存下来
        last_data = components[-1].encode('utf-8')
        # # 向客户端发送响应数据
        # response = "Hello from Python!"
        # client_socket.sendall(response.encode('utf-8'))


finally:
    # 关闭连接
    print("关闭连接")
    client_socket.close()
    server_socket.close()
