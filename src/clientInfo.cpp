/**
 * @file clientInfo.cpp
 * @author lzx0626 (2065666169@qq.com)
 * @brief 存储客户端 IP 和端口信息的源文件
 * @version 1.0
 * @date 2024-03-01
 *
 * Copyright (c) 2023 电子科技大学 刘治学
 *
 */

#include "clientInfo.h"

void clientInfo::clear()
{
    port = -1;
    bzero(ip, sizeof(ip));
}
