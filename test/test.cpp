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
 * @brief 测试读取 res 目录下的所有图片文件，对二进制流进行 base64 编码，然后传递给另一个文件解码再输出成拷贝文件的过程
 */
int main()
{
    auto fileNameList = tools::getAllFileName("../res");
    for (auto &fileName : fileNameList)
    {
        std::cout << fileName << '\n';

        std::string inputFilePath = "../res/" + fileName;

        auto outputFileName = fileName;
        for (int i = 0; i < 4; ++i) // 弹掉 .png
            outputFileName.pop_back();
        outputFileName += "_copy.png";

        std::string outputFilePath = "../res/" + outputFileName;

        // 读物文件编码
        auto encodedList = tools::readFileAndEncode(inputFilePath.c_str());
        // 解码输出到文件
        tools::decodeAndOutputToFile(outputFilePath.c_str(), encodedList);

        std::cout << outputFileName << " 文件拷贝完成。" << '\n'
                  << '\n';
    }

    return 0;
}
