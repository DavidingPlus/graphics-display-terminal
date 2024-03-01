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

#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>

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
    // TODO
    while (1)
    {
    }

    // 4. 关闭套接字
    close(connectFd);

    return 0;
}
