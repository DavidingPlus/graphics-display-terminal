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

namespace base64
{

    static bool is_base64(const char c) { return (isalnum(c) || (c == '+') || (c == '/')); }

    std::string base64_encode(const char *bytes_to_encode, unsigned int in_len);

    std::string base64_decode(const std::string &encoded_string);

}; // namespace base64

#endif
