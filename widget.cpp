#include "widget.h"
#include "ui_widget.h"

Widget::Widget(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);

    // note： Qt 中的服务端和客户端的正确方法，和自己用过的 Linux 平台接口不同，用的时候记得查资料
    clientSock = new QTcpSocket(this);
    decode.clear();

    // 先创建存储图片的目录
    QDir dir;
    if(dir.exists("./res"))
        dir.rmdir(QString("./res"));
    Sleep(3000);
    dir.mkdir(QString("./res"));

    //绑定信号槽，服务端发送信息过来之后自动打印（前提是客户端发送有效信息）
    connect(clientSock, &QTcpSocket::readyRead, [ = ]()
    {
        QByteArray readBuf = clientSock->readAll(); // 这里 Qt 提供了一次性读完的功能
        qDebug() << readBuf;

        // 做退出的判断
        if( QString("exit success\n") == QString(readBuf))
        {
            clientSock->disconnectFromHost();
            qDebug() << "disconnect from server successfully.";
        }

        // 做结束读取的判断
        if( QString("send over\n") == QString(readBuf))
        {
            Sleep(2000);
            qDebug() << decode;

            // TODO：由于网络传输过程中可能出现各种各样的丢失，为了保证图片正常读取，考虑加一个校验文件大小的判断，失败则重传（本地传不会有这个问题）

            // 发送结束读取的信号
            emit recvOver();
            return;
        }

        // 写入字节数组
        decode += QByteArray::fromStdString(base64Decode(readBuf.toStdString()));
    });

    // 绑定信号槽，图片数据读取完毕之后写入文件
    connect(this, &Widget::recvOver, [ = ]()
    {
        qDebug() << "start to writing into file.";

        // 将字节数组写入文件
        // TODO：想改成边读取边写入
        QFile file(QString("./res/圆形_copy.png"), this);
        file.open(QFile::ReadWrite);
        file.write(decode);

        qDebug() << "write all data successfully.";

        file.close();

        // 写入完毕清空 decode
        decode.clear();
    });
}

Widget::~Widget()
{
    delete ui;
}

void Widget::on_connectBtn_clicked()
{
    // 已连接不可再连接
    if(QAbstractSocket::ConnectedState == clientSock->state())
    {
        qDebug() << "already connected to server, can not connect again!";
        return;
    }

    // 连接服务端
    // 通过读取 lineEdit 中的信息来连接
    QString serverIP = ui->IPEdit->text();
    quint16 serverPort =  quint16(ui->portEdit->text().toInt());
    clientSock->connectToHost(serverIP, serverPort);

    // 等待连接成功，等待时间5秒，太长经测试会让程序卡死
    bool res = clientSock->waitForConnected(5000);// 单位是毫秒
    qDebug() << res;
    if(false == res)
    {
        qDebug() << "fail to connect to server, please check your IP or port.";
        return;
    }

    // 等待状态转换完成
    while(clientSock->state() != QAbstractSocket::ConnectedState)
        QCoreApplication::processEvents();

    qDebug() << "connect to server successfully.";
}

void Widget::on_recvBtn_clicked()
{
    // 未连接不可发送信息
    if(QAbstractSocket::UnconnectedState == clientSock->state())
    {
        qDebug() << "not connected to server, can not recv any message from server!";
        return;
    }

    // 发送信息
    QString sendMessage("send\n");
    clientSock->write(sendMessage.toUtf8());
    //不缓存，直接发送，否则不会立即发出去
    clientSock->flush();
}

void Widget::on_disconnectBtn_clicked()
{
    // 为连接不可断开连接
    if(QAbstractSocket::UnconnectedState == clientSock->state())
    {
        qDebug() << "not connected to server, can not disconnect from server!";
        return;
    }

    // 发送断开连接信息
    QString sendMessage("exit\n");
    clientSock->write(sendMessage.toUtf8());
    clientSock->flush();

    // note： 与服务端断开连接已在接收信息的信号槽中处理，当然也只在我这么写的服务端中可以这么处理
}

bool Widget::isBase64(const char& c)
{
    return isalnum(c) or ('+' == c) or ('/' == c);
}

std::string Widget::base64Decode(const std::string& encodedStr)
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
