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
    dir.mkdir(QString("./res"));

    // 新开文件
    file = new QFile(QString("./res/鸡你太美_copy.png"), this);

    //绑定信号槽，服务端发送信息过来之后自动打印（前提是客户端发送有效信息）
    connect(clientSock, &QTcpSocket::readyRead, this, [ = ]()
    {
        if(false == file->isOpen())
            file->open(QFile::ReadWrite);

        QByteArray readBuf = clientSock->readAll(); // 这里 Qt 提供了一次性读完的功能
        qDebug() << readBuf;

        // 做退出的判断
        if( QString("exit success\n") == QString(readBuf))
        {
            clientSock->disconnectFromHost();
            qDebug() << "disconnect from server successfully.";

            return;
        }

        // 做结束读取的判断
        if( QString("send over\n") == QString(readBuf))
        {
            decode.clear();
            file->close();

            // 图片已经写入文件完毕
            emit recvAndWriteOver();

            return;
        }

        // 写入字节数组
        decode = QByteArray::fromStdString(base64Decode(readBuf.toStdString()));
        file->write(decode);
        decode.clear();
    });

    // 绑定信号槽，图片写入完毕之后准备绘图
    connect(this, &Widget::recvAndWriteOver, [ = ]()
    {
        QImage image;
        image.load("./res/鸡你太美_copy.png");
        ui->picLabel->setPixmap(QPixmap::fromImage(image));
        ui->picLabel->setGeometry(image.rect());
        ui->picLabel->show();
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

    // 修改 isConnectedLabel 内容
    ui->isConnectedLabel->setText("连接状态：已连接");

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

    // 修改 isConnectedLabel 内容
    ui->isConnectedLabel->setText("连接状态：未连接");

    // note： 与服务端断开连接已在接收信息的信号槽中处理，当然这么处理的前提是服务端与客户端协商好通信规则
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
