/**
 * @file tools.cpp
 * @author lzx0626 (2065666169@qq.com)
 * @brief 工具函数存放的头文件，包括读取图片文件，进行 base64 编码解码等
 * @version 1.0
 * @date 2024-02-29
 *
 * Copyright (c) 2023 电子科技大学 刘治学
 *
 */

#include "tools.h"

bool tools::isBase64(const char &c) { return isalnum(c) or ('+' == c) or ('/' == c); }

std::string tools::base64Encode(const char *buf, size_t len)
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

std::string tools::base64Decode(const std::string &encodedStr)
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

    while (len-- and (encodedStr[in_] != '=') and isBase64(encodedStr[in_]))
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

std::vector<std::string> tools::readFileAndEncode(const char *inputFilePath)
{
    std::vector<std::string> res;

    // 打开文件
    std::ifstream file(inputFilePath, std::ios::binary);
    if (!file)
    {
        std::cerr << "readFileAndEncode: can not open target file.\n";
        exit(-1);
    }

    // 读取
    char readBuf[maxBufferSize + 1] = {0}; // 以后开buf，这里为了保险都多开一个字节
    while (!file.eof())
    {
        bzero(readBuf, sizeof(readBuf));
        file.read(readBuf, sizeof(readBuf) - 1);
        std::streamsize bytesRead = file.gcount();

        if (bytesRead > 0)
        {
            // 原字符串小于 maxBufferSize ，编码出来的长度不一定就一定小于 maxBufferSize ，因此这个地方做一个处理，为了后面每次包发送的时候能够正确发送，保证该长度小于 maxBufferSize ，若大于或等于 maxBufferSize （有一个'\0'符号，所以这里设置为减一），则再拆包
            std::string encode = tools::base64Encode(readBuf, (size_t)bytesRead);
            while (encode.size() >= maxBufferSize)
            {
                res.push_back(std::string{encode.begin(), encode.begin() + maxBufferSize});
                encode.erase(encode.begin(), encode.begin() + maxBufferSize);
            }
            res.push_back(encode);
        }
    }

    // 关闭文件
    file.close();

    return res;
}

void tools::decodeAndOutputToFile(const char *outputFilePath, const std::vector<std::string> &encodedList)
{
    // 打开文件
    std::ofstream file(outputFilePath, std::ios::binary);
    if (!file)
    {
        std::cerr << "decodeAndOutputToFile: can not open target file.\n";
        exit(-1);
    }

    for (auto &encode : encodedList)
    {
        std::string decode = tools::base64Decode(encode);
        file.write(decode.c_str(), decode.size());
    }

    // 关闭文件
    file.close();
}

std::vector<std::string> tools::getAllFileName(const char *inputDirPath)
{
    std::vector<std::string> res;

    // 打开目录
    DIR *dir = opendir(inputDirPath);
    if (nullptr == dir)
    {
        perror("opendir");
        exit(-1);
    }

    // 循环读取目录中的文件
    // note： 不做读到子目录的处理，因为这个内部工具函数处理的目录保证只有文件
    struct dirent *ptr = nullptr;
    while (1)
    {
        ptr = readdir(dir);
        if (nullptr == ptr)
            break;

        std::string fileName = ptr->d_name;
        if ("." == fileName or ".." == fileName)
            continue;

        res.push_back(fileName);
    }

    // 关闭目录
    closedir(dir);

    return res;
}

long tools::getFileSize(const char *inputFilePath)
{
    FILE *file = fopen(inputFilePath, "r");
    if (nullptr == file)
    {
        perror("fopen");
        return -1;
    }

    fseek(file, 0, SEEK_END);
    long res = ftell(file);
    fclose(file);
    return res;
}
