/**
 * @file main.cpp
 * @author lzx0626 (2065666169@qq.com)
 * @brief 测试程序，测试图片到 base64 编码再解码的过程
 * @version 1.0
 * @date 2024-02-29
 *
 * Copyright (c) 2023 电子科技大学 刘治学
 *
 */

#include <iostream>
#include <fstream>
#include <string>
#include <cstring>

#include "base64.h"

#define readBufSize 1024

/**
 * @brief 测试读取图片文件，对二进制流进行 base64 编码，然后传递给另一个文件解码再输出成拷贝文件的过程
 * @note 测试 shell 命令：
 * ./test LBJ.jpg LBJ_copy.jpg
 * ./test 基泥胎煤.png 基泥胎煤_copy.png
 */
int main(int argc, char const *argv[])
{
    if (argc < 3)
    {
        std::cerr << "usage:  ./test  <inputFileName>  <outputFileName>" << '\n';
        return -1;
    }

    std::string inputFileName = "../res/" + std::string(argv[1]);
    std::string outputFileName = "../res/" + std::string(argv[2]);

    std::string encode;
    std::string decode;

    std::ifstream inputFile(inputFileName, std::ios::binary);
    if (!inputFile)
    {
        std::cerr << "无法打开输入文件。" << std::endl;
        return -1;
    }

    std::ofstream outputFile(outputFileName, std::ios::binary);
    if (!outputFile)
    {
        std::cerr << "无法创建输出文件。" << std::endl;
        return -1;
    }

    char readBuf[readBufSize] = {0};
    while (!inputFile.eof())
    {
        bzero(readBuf, sizeof(readBuf));
        inputFile.read(readBuf, readBufSize);
        std::streamsize bytesRead = inputFile.gcount();

        // 这里采取的策略是将文件分块进行传输，而不是整体传输，这样可以避免文件过大而带来的种种问题
        if (bytesRead > 0)
        {
            std::string encode = base64::base64Encode(readBuf, (size_t)bytesRead);
            std::cout << encode; // 输出一下编码后的结果

            std::string decode = base64::base64Decode(encode);
            outputFile.write(decode.c_str(), decode.size());
        }
    }
    std::cout << '\n'
              << "文件拷贝完成。" << '\n';

    inputFile.close();
    outputFile.close();

    return 0;
}
