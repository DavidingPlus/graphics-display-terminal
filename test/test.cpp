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

#include "tools.h"

/**
 * @brief 测试读取图片文件，对二进制流进行 base64 编码，然后传递给另一个文件解码再输出成拷贝文件的过程
 * @note 测试 shell 命令：
 * ./test 圆形.png 圆形_copy.png
 */
int main(int argc, char const *argv[])
{
    if (argc < 3)
    {
        std::cerr << "usage:  ./test  <inputFileName>  <outputFileName>" << '\n';
        return -1;
    }

    std::string inputFilePath = "../res/" + std::string(argv[1]);
    std::string outputFilePath = "../res/" + std::string(argv[2]);

    // 读物文件编码
    auto encodedList = tools::readFileAndEncode(inputFilePath.c_str());
    // 解码输出到文件
    tools::decodeAndOutputToFile(outputFilePath.c_str(), encodedList);

    std::cout << '\n'
              << "文件拷贝完成。" << '\n';

    return 0;
}
