/**
 * @file base64.h
 * @author lzx0626 (2065666169@qq.com)
 * @brief 提供一种 base64 编码算法的头文件
 * @version 1.0
 * @date 2024-02-29
 *
 * Copyright (c) 2023 电子科技大学 刘治学
 *
 */

#ifndef _BASE64_H_
#define _BASE64_H_

#include <string>
#include <iostream>

/**
 * @brief 由于都是一些函数，所以就封一个名字空间得了，不封成类
 * @namespace base64
 */
namespace base64
{

    /**
     * @brief 判断是否为 base64 编码
     * @param c 一个字符
     * @return bool 值真假
     */
    static bool is_base64(const char &c) { return isalnum(c) or ('+' == c) or ('/' == c); }

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

}; // namespace base64

#endif
