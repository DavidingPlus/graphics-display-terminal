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
#include <cstring>

#include <sys/epoll.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>

#include "clientInfo.h"
#include "tools.h"

/**
 * @brief 定义 epoll 能检测的最大事件个数max_events
 */
#define maxEvents 1000

/**
 * @brief 定义退出成功时候服务端返回的信息
 */
static const char *exitSuccess = "exit success\n";

/**
 * @brief 定义图片发送完毕时候返回的信息
 *
 */
static const char *sendOver = "send over\n";

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

    // 创建存储客户端信息的结构体
    struct clientInfo cliInfos[10 + maxEvents];

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

                // 处理客户端信息，存入结构体，下标为 connectFd
                cliInfos[connectFd].port = ntohs(clientAddr.sin_port);
                inet_ntop(AF_INET, &clientAddr.sin_addr.s_addr, cliInfos[connectFd].ip, maxIpv4Len);

                // 打印信息
                std::cout << "client (ip: " << cliInfos[connectFd].ip << " , "
                          << "port: " << cliInfos[connectFd].port << ") has connected." << std::endl;

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
                int connectFd = resEvents[i].data.fd;
                char readBuf[maxBufferSize + 1] = {0};

                // 读
                int len = recv(connectFd, readBuf, sizeof(readBuf) - 1, 0);
                if (-1 == len)
                {
                    // 由于读取是非阻塞的，这里有两种情况需要特殊判断，但是这里遇不到
                    // errno==EINTR，收到信号并从信号处理函数返回时，慢系统调用会返回并设置errno为EINTR，应该重新调用read。
                    // errno==EAGAIN，表示当前暂时没有数据可读，应该稍后读取。
                    perror("recv");
                    return -1;
                }
                // 客户端关闭，包括客户端强制断开连接和输入 exit 退出
                if (0 == len or 0 == strcmp("exit\n", readBuf))
                {
                    if (0 == strcmp("exit\n", readBuf))
                    {
                        std::cout << "client (ip: " << cliInfos[connectFd].ip << " , "
                                  << "port: " << cliInfos[connectFd].port << ") send: " << readBuf;

                        // 发送规定回信
                        send(connectFd, exitSuccess, strlen(exitSuccess), 0);
                    }

                    // 从监听事件中删除
                    res = epoll_ctl(epollFd, EPOLL_CTL_DEL, connectFd, nullptr);
                    if (-1 == res)
                    {
                        perror("epoll_ctl");
                        return -1;
                    }

                    std::cout << "client (ip: " << cliInfos[connectFd].ip << " , "
                              << "port: " << cliInfos[connectFd].port << ") has closed." << std::endl;
                    cliInfos[connectFd].clear();

                    // 关闭文件描述符
                    close(connectFd);

                    continue;
                }
                else
                    std::cout << "client (ip: " << cliInfos[connectFd].ip << " , "
                              << "port: " << cliInfos[connectFd].port << ") send: " << readBuf;

                // 写
                // 读取图片并且发送图片
                // 我在工具函数里面保证了编码之后每个子包的长度都是小于1024的
                auto encodedList = tools::readFileAndEncode("../res/LBJ.jpg");

                // 服务端发送到客户端的接收缓冲区，由于客户端接受的性能问题，显然不是同步接受，因此需要间隔调整服务端发送图片的时间；如果服务端发送的太快，导致客户端没来得及接受，会导致缓冲区被撑爆，数据丢失
                for (auto &encode : encodedList)
                {
                    send(connectFd, encode.c_str(), encode.size(), 0);
                    usleep(1000); // 延迟1毫秒继续发送
                    std::cout << encode;
                }
                std::cout << std::endl;
                send(connectFd, sendOver, strlen(sendOver), 0);
            }
        }
    }

    // 6. 关闭套接字
    close(listenFd);

    return 0;
}
