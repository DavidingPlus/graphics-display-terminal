/**
 * @file base64.cpp
 * @author lzx0626 (2065666169@qq.com)
 * @brief 提供一种 base64 编码算法的源文件
 * @version 1.0
 * @date 2024-02-29
 *
 * Copyright (c) 2023 电子科技大学 刘治学
 *
 */

#include "base64.h"

namespace base64
{

    std::string base64Encode(const char *buf, size_t len)
    {
        const std::string base64_chars =
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz"
            "0123456789+/";

        std::string res;
        int i = 0, j = 0;
        unsigned char char_array_3[3] = {0}, char_array_4[4] = {0};

        while (len--)
        {
            char_array_3[i++] = *(buf++);
            if (3 == i)
            {
                char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
                char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
                char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
                char_array_4[3] = char_array_3[2] & 0x3f;

                for (i = 0; i < 4; ++i)
                    res += base64_chars[char_array_4[i]];
                i = 0;
            }
        }

        if (i)
        {
            for (j = i; j < 3; ++j)
                char_array_3[j] = '\0';

            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;

            for (j = 0; j < i + 1; ++j)
                res += base64_chars[char_array_4[j]];

            while (i++ < 3)
                res += '=';
        }

        return res;
    }

    std::string base64Decode(const std::string &encodedStr)
    {
        const std::string base64_chars =
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz"
            "0123456789+/";

        int len = (int)encodedStr.size();
        int i = 0, j = 0;
        int in_ = 0;
        unsigned char char_array_4[4] = {0}, char_array_3[3] = {0};
        std::string res;

        while (len-- and (encodedStr[in_] != '=') and is_base64(encodedStr[in_]))
        {
            char_array_4[i++] = encodedStr[in_];
            ++in_;
            if (4 == i)
            {
                for (i = 0; i < 4; ++i)
                    char_array_4[i] = base64_chars.find(char_array_4[i]);

                char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
                char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
                char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

                for (i = 0; i < 3; ++i)
                    res += char_array_3[i];
                i = 0;
            }
        }

        if (i)
        {
            for (j = i; j < 4; ++j)
                char_array_4[j] = 0;

            for (j = 0; j < 4; ++j)
                char_array_4[j] = base64_chars.find(char_array_4[j]);

            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

            for (j = 0; j < i - 1; ++j)
                res += char_array_3[j];
        }

        return res;
    }
} // namespace base64
