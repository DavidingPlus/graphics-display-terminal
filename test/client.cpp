/**
 * @file client.cpp
 * @author lzx0626 (2065666169@qq.com)
 * @brief 客户端主程序（仅用作服务端功能测试，真正的客户端使用 Qt 编写）
 * @version 1.0
 * @date 2024-03-01
 *
 * Copyright (c) 2023 电子科技大学 刘治学
 *
 */

#include <iostream>
#include <cstring>

#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>

#include "tools.h"

/**
 * @brief 客户端程序，仅用作服务端功能测试，连接服务端，并且作信息交互
 */
int main(int argc, char *const argv[])
{
    if (argc < 3)
    {
        std::cerr << "usage: ./client  <serverIp>  <serverPort>\n";
        return -1;
    }

    // 1. 创建 socket 套接字
    int connectFd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (-1 == connectFd)
    {
        perror("socket");
        return -1;
    }

    // 为了更快的读取图片文件，修改一下内核中接收缓冲区的大小，改为 0.5 MB
    // int recvBuf = 1024 * 512;
    // int res = setsockopt(connectFd, SOL_SOCKET, SO_RCVBUF, &recvBuf, sizeof(recvBuf));
    // if (-1 == res)
    // {
    //     perror("sersockopt");
    //     return -1;
    // }

    // 2. 连接服务端
    struct sockaddr_in serverAddr;
    // 地址族
    serverAddr.sin_family = AF_INET;
    // IP
    inet_pton(AF_INET, argv[1], &serverAddr.sin_addr.s_addr);
    // 端口
    serverAddr.sin_port = htons(atoi(argv[2]));

    int res = connect(connectFd, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
    if (-1 == res)
    {
        perror("connect");
        return -1;
    }

    std::cout << "connect to server successfully." << std::endl;

    // 3. 通信
    char buf[maxBufferSize + 1] = {0};

    while (1)
    {
        bzero(buf, sizeof(buf));
        fgets(buf, sizeof(buf) - 1, stdin);

        // 写
        // 我们自定义客户端发送的信息，一次性能全部发完，所以这里就不做循环发送了
        send(connectFd, buf, strlen(buf), 0);
        std::cout << "send: " << buf;

        // 读
        std::vector<std::string> encodedList;
        int len = -1;
        while (1)
        {
            bzero(buf, sizeof(buf));
            len = recv(connectFd, buf, sizeof(buf) - 1, 0);
            if (-1 == len)
            {
                perror("recv");
                return -1;
            }

            // 服务端关闭
            if (0 == len)
            {
                std::cout << "server has closed." << std::endl;
                close(connectFd);
                exit(-1);
            }
            // 正常通信
            else
            {
                // 自动手动退出，服务端发送的回信
                if (0 == strcmp("exit success\n", buf))
                {
                    close(connectFd);
                    exit(-1);
                }

                // 传输结束
                if (0 == strcmp("send over\n", buf))
                {
                    std::cout << "\nrecv over\n";
                    break;
                }

                // 我们默认数据传输过程中不会出现问题
                // 经过 tools 中的处理，这里的 buf 每一个最多都只能是 1024 字节，不会溢出
                std::cout << buf;
                encodedList.push_back(buf);
            }
        }
        tools::decodeAndOutputToFile("../res/圆形_copy.png", encodedList);
    }

    // 4. 关闭套接字
    close(connectFd);

    return 0;
}
