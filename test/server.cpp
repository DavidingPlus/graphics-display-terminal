/**
 * @file server.cpp
 * @author lzx0626 (2065666169@qq.com)
 * @brief 服务端主程序
 * @version 1.0
 * @date 2024-03-01
 *
 * Copyright (c) 2023 电子科技大学 刘治学
 *
 */

#include <iostream>

#include <sys/epoll.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>

#define maxEvents 1000
#define maxIpv4Len 16

/**
 * @brief 服务端主程序，接受客户端连接，并且作信息交互
 */
int main(int argc, char *const argv[])
{
    if (argc < 2)
    {
        std::cerr << "usage:  ./server  <port>\n";
        return -1;
    }

    // 1. 创建socket套接字
    int listenFd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (-1 == listenFd)
    {
        perror("socket");
        return -1;
    }

    // 设置端口复用
    int optval = 1;
    int res = setsockopt(listenFd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));
    if (-1 == res)
    {
        perror("setsockopt");
        return -1;
    }

    // 2. 绑定端口
    struct sockaddr_in serverAddr;
    // 地址族
    serverAddr.sin_family = AF_INET;
    // IP
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    // 端口
    serverAddr.sin_port = htons(atoi(argv[1]));

    res = bind(listenFd, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
    if (-1 == res)
    {
        perror("bind");
        return -1;
    }

    std::cout << "server has successfully initialized." << std::endl;

    // 3. 开始监听
    res = listen(listenFd, 5);
    if (-1 == res)
    {
        perror("listen");
        return -1;
    }

    // 4. 创建 epoll 实例
    int epollFd = epoll_create(1);
    if (-1 == epollFd)
    {
        perror("epoll_create");
        return -1;
    }

    // 将 listenFd 加入到 epoll 的监听事件中
    struct epoll_event listenEvent;
    listenEvent.data.fd = listenFd;
    listenEvent.events = EPOLLIN;

    res = epoll_ctl(epollFd, EPOLL_CTL_ADD, listenFd, &listenEvent);
    if (-1 == res)
    {
        perror("epoll_ctl");
        return -1;
    }

    // 5. 开始检测
    while (1)
    {
        struct epoll_event resEvents[maxEvents] = {0};
        int count = epoll_wait(epollFd, resEvents, maxEvents, -1); //-1表示阻塞
        if (-1 == count)
        {
            perror("epoll_wait");
            return -1;
        }

        for (int i = 0; i < count; ++i)
        {
            // 新客户端加入
            if (listenFd == resEvents[i].data.fd)
            {
                // 接受请求
                struct sockaddr_in clientAddr;
                socklen_t clientAddrLen = sizeof(clientAddr);

                int connectFd = accept(listenFd, (struct sockaddr *)&clientAddr, &clientAddrLen);
                if (-1 == connectFd)
                {
                    perror("accept");
                    return -1;
                }

                // 输出客户端信息
                char clientIp[maxIpv4Len] = {0};
                inet_ntop(AF_INET, &clientAddr.sin_addr.s_addr, clientIp, sizeof(clientIp));

                std::cout << "client (ip: " << clientIp << " , "
                          << "port: " << ntohs(clientAddr.sin_port) << ") has connected." << std::endl;

                // 设置读取为非阻塞，因为这是 NIO 模型
                fcntl(connectFd, F_SETFL, O_NONBLOCK | fcntl(connectFd, F_GETFL));

                // 加入到监听事件
                struct epoll_event clientEvent;
                clientEvent.data.fd = connectFd;
                clientEvent.events = EPOLLIN;

                res = epoll_ctl(epollFd, EPOLL_CTL_ADD, connectFd, &clientEvent);
                if (-1 == res)
                {
                    perror("epoll_ctl");
                    return -1;
                }
            }
            // 旧客户端通信
            else
            {
                // TODO
            }
        }
    }

    // 6. 关闭套接字
    close(listenFd);

    return 0;
}
