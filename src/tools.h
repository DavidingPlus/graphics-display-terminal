/**
 * @file tools.h
 * @author lzx0626 (2065666169@qq.com)
 * @brief 工具函数存放的头文件，包括读取图片文件，进行 base64 编码解码等
 * @version 1.0
 * @date 2024-02-29
 *
 * Copyright (c) 2023 电子科技大学 刘治学
 *
 */

#ifndef _TOOLS_H_
#define _TOOLS_H_

#include <string>
#include <fstream>
#include <iostream>
#include <vector>
#include <cstring>

#include <unistd.h>
#include <dirent.h>

/**
 * @brief 定义一次最多读取的数组大小，见 client.cpp 中，定义为 1KB 即可，不用开太多，因为素材图片没那么大
 */
#define maxBufferSize 1024

/**
 * @brief 由于都是一些函数，所以就封一个名字空间得了，不封成类
 * @note 这些函数的定义均必须在 cpp 文件中，否则会出现链接的问题；在 h 中定义需加上静态关键字
 * @namespace tools
 */
namespace tools
{

    /**
     * @brief 判断是否为 base64 编码
     * @param c 一个字符
     * @return bool 值真假
     */
    bool isBase64(const char &c);

    /**
     * @brief 对输入的字符串进行 base64 编码
     * @param buf 字符串首地址
     * @param len 编码的字节长度
     * @return 编码后的 std::string 格式字符串
     */
    std::string base64Encode(const char *buf, size_t len);

    /**
     * @brief 对输入的已编码 base64 字符串进行解码
     * @param encodedStr 已编码的字符串
     * @return 解码后的 std::string 格式字符串
     */
    std::string base64Decode(const std::string &encodedStr);

    /**
     * @brief 通过二进制流读取指定文件，并且分块编码输出
     * @param FilePath 输入文件路径
     * @return 编码过后的字符串列表
     */
    std::vector<std::string> readFileAndEncode(const char *inputFilePath);

    /**
     * @brief 接受编码后的字符串，然后解码输出到指定文件
     * @param outputFilePath 输入文件路径
     * @param encodedList 编码过后的字符串列表
     */
    void decodeAndOutputToFile(const char *outputFilePath, const std::vector<std::string> &encodedList);

    /**
     * @brief 获取制定目录下所有文件的名称，不包括目录（内部函数，我保证目录内部不存在子目录）
     * @param inputDirPath 输入目录路径
     * @return 文件名列表
     */
    std::vector<std::string> getAllFileName(const char *inputDirPath);

}; // namespace tools

#endif
