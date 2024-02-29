/**
 * @file main.cpp
 * @author lzx0626 (2065666169@qq.com)
 * @brief 服务端主程序
 * @version 1.0
 * @date 2024-02-29
 *
 * Copyright (c) 2023 电子科技大学 刘治学
 *
 */

#include <iostream>
#include <unistd.h>
#include <fstream>

#include "base64.h"

int main()
{
    // 先删除拷贝文件
    if (0 == access("../res/LBJ_copy.jpg", F_OK))
        unlink("../resLBJ_copy.jpg");

    std::cout << "Waiting to remove old copy files." << std::endl;
    sleep(1);

    std::fstream f;
    f.open("../res/LBJ.jpg", std::ios::in | std::ios::binary);
    f.seekg(0, std::ios_base::end); // 设置偏移量至文件结尾
    std::streampos sp = f.tellg();  // 获取文件大小
    int size = sp;

    char *buffer = (char *)malloc(sizeof(char) * size);
    f.seekg(0, std::ios_base::beg); // 设置偏移量至文件开头
    f.read(buffer, size);           // 将文件内容读入buffer
    std::cout << "file size:" << size << '\n'
              << std::endl;

    std::string imgBase64 = base64::base64_encode(buffer, size); // 编码
    std::cout << "img base64 encode size: " << imgBase64.size() << std::endl;
    std::cout << "img base64 encode: " << imgBase64 << '\n'
              << std::endl;

    std::string imgdecode64 = base64::base64_decode(imgBase64); // 解码
    std::cout << "img base64 decode size: " << imgdecode64.size() << std::endl;
    // std::cout << "img base64 decode: " << imgdecode64 << std::endl; // 乱码不打印

    const char *p = imgdecode64.c_str();
    std::ofstream fout("../res/LBJ_copy.jpg", std::ios::out | std::ios::binary);
    if (!fout)
    {
        std::cout << "error" << std::endl;
    }
    else
    {
        std::cout << "Success!" << std::endl;
        fout.write(p, size);
    }

    fout.close();

    return 0;
}
