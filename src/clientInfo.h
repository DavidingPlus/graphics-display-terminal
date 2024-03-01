/**
 * @file clientInfo.h
 * @author lzx0626 (2065666169@qq.com)
 * @brief 存储客户端 IP 和端口信息的头文件
 * @version 1.0
 * @date 2024-03-01
 *
 * Copyright (c) 2023 电子科技大学 刘治学
 *
 */

#ifndef _CLIENTINFO_H_
#define _CLIENTINFO_H_

#include <iostream>
#include <cstring>

#define maxIpv4Len 16

/**
 * @brief 存储客户端的 IP 和端口信息
 * @class clientInfo
 */
struct clientInfo
{
    /**
     * @brief 默认构造函数
     *
     */
    clientInfo() = default;

    /**
     * @brief 析构函数
     *
     */
    ~clientInfo() { clear(); }

    /**
     * @brief 清理函数，将内部资源重置
     *
     */
    void clear();

    /**
     * @brief 端口号
     */
    int port = -1;

    /**
     * @brief IP地址
     */
    char ip[maxIpv4Len] = {0};
};

#endif
