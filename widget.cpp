#include "widget.h"
#include "ui_widget.h"

#include <synchapi.h>
#include <QtNetwork>
#include <QDebug>

#include <iostream>
#include <string>
#include <cstring>

#define maxBufferSize 1024

std::string base64Decode(const std::string& encodedStr);
bool isBase64(const char& c);

Widget::Widget(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);

    // note： 以下的示例是 Qt 的服务端和客户端的正确方法，和自己用过的 Linux 平台接口不同，用的时候查资料即可

    // 1. 连接服务端
    QTcpSocket* clientSock = new QTcpSocket(this);
    clientSock->connectToHost(QString("106.55.60.140"), uint16_t(8080));

    // 2. 等待连接成功
    clientSock->waitForConnected();

    // 3. 等待状态转换完成
    while(clientSock->state() != QAbstractSocket::ConnectedState)
        QCoreApplication::processEvents();

    // 4. 发送信息
    QString sendMessage("send\n");
    clientSock->write(sendMessage.toUtf8());
    //不缓存，直接发送，否则不会立即发出去
    clientSock->flush();

    // 5. Qt 中接收信息，直接接收我也不知道为啥接收不到，推荐使用信号槽的方式
    connect(clientSock, &QTcpSocket::readyRead, this, [ = ]()
    {
        QByteArray readBuf = clientSock->readAll();
        qDebug() << readBuf;
//        QString decode = QString::fromStdString(base64Decode(readBuf.toStdString()));
//        qDebug() << decode;
    });
}

Widget::~Widget()
{
    delete ui;
}

bool isBase64(const char& c)
{
    return isalnum(c) or ('+' == c) or ('/' == c);
}

std::string base64Decode(const std::string& encodedStr)
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
