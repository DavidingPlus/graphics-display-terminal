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

    //绑定信号槽，服务端发送信息过来之后自动打印（前提是客户端发送有效信息）
    connect(clientSock, &QTcpSocket::readyRead, this, [ = ]()
    {
        QByteArray readBuf = clientSock->readAll(); // 这里 Qt 提供了一次性读完的功能
        qDebug() << readBuf;

        // 做退出的判断
        if( QString("exit success\n") == QString(readBuf))
        {
            clientSock->disconnectFromHost();
            qDebug() << "disconnect from server successfully.";
            QMessageBox::information(this, "断开连接", "与服务端断开连接成功！");

            return;
        }

        // 图片个数，不做任何处理，仅作过滤作用
        if( 0 == picNum)
        {
            picNum = readBuf.toInt();
            return;
        }

        // 图片名字
        if(isPicName)
        {
            // 打开文件
            QString filePath = QString("./res/") + QString(readBuf);
            file = new QFile(filePath, this);
            file->open(QFile::ReadWrite);

            isPicName = false;
            return;
        }

        // 做结束读取的判断
        if( QString("send over\n") == QString(readBuf))
        {
            decode.clear();

            // 图片已经写入文件完毕
            emit recvAndWriteOver();

            // 关闭文件
            file->close();

            // 修改图片名字标志位，下一次可读取
            isPicName = true;

            // 如果全部接收完毕，弹出一个提示窗口
            if( picNum == ++picIndex)
            {
                qDebug() << "recv pic successfully.";
                QMessageBox::information(this, "接收图片", "图片数据接收完毕！");

                isRecvPic = false;
            }

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

        qDebug() << file->fileName();
        qDebug() << file->exists();
        // TODO： 画图片的逻辑有问题，图片能正确接收但是绘图的时候出问题
//        ui->picLabel->setScaledContents(true); // 这条是让图片自适应 Label ，不然显示不完整
//        ui->picLabel->setPixmap(file->fileName());
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
        QMessageBox::warning(this, "连接", "您已经连接上了服务端，不可再次连接！");

        return;
    }

    // 正在传输不可点击
    if(isRecvPic)
    {
        qDebug() << "recving pic right now!";
        QMessageBox::warning(this, "连接", "您正在进行传输，不可进行连接！");

        return;
    }

    // 连接服务端
    // 通过读取 lineEdit 中的信息来连接
    QString serverIP = ui->IPEdit->text();
    quint16 serverPort =  quint16(ui->portEdit->text().toInt());
    clientSock->connectToHost(serverIP, serverPort);

    // 等待连接成功，等待时间3秒，太长经测试会让程序卡死
    bool res = clientSock->waitForConnected(3000);// 单位是毫秒
    if(false == res)
    {
        qDebug() << "fail to connect to server, please check your IP or port.";
        QMessageBox::warning(this, "连接", "服务端 IP 或端口错误，请您检查后重试！");

        return;
    }

    // 等待状态转换完成
    while(clientSock->state() != QAbstractSocket::ConnectedState)
        QCoreApplication::processEvents();

    // 修改 isConnectedLabel 内容
    ui->isConnectedLabel->setText("连接状态：已连接");

    qDebug() << "connect to server successfully.";
    QMessageBox::information(this, "连接", "连接服务端成功！");
}

void Widget::on_recvPicBtn_clicked()
{
    // 未连接不可发送信息
    if(QAbstractSocket::UnconnectedState == clientSock->state())
    {
        qDebug() << "not connected to server, can not recv any message from server!";
        QMessageBox::warning(this, "接收图片", "您尚未连接，不可接收图片！");

        return;
    }

    // 正在传输不可再次点击
    if(isRecvPic)
    {
        qDebug() << "recving pic right now!";
        QMessageBox::warning(this, "接收图片", "您正在进行传输，不可再次发送接收图片请求！");

        return;
    }

    // 修改发送图片个数的标志位
    picNum = 0;

    // 修改图片发送到第几张
    picIndex = 0;

    // 修改发送图片名字的标志位
    isPicName = true;

    // 修改是否正在传输标志位
    isRecvPic = true;

    // 处理存储图片的目录，先删除在创建
    QDir dir("./"),
         subDir("./res/");
    subDir.removeRecursively();
    dir.mkdir("./res/");

    // 发送信息
    QString sendMessage("send\n");
    clientSock->write(sendMessage.toUtf8());
    //不缓存，直接发送，否则不会立即发出去
    clientSock->flush();
}

void Widget::on_disconnectBtn_clicked()
{
    // 未连接不可断开连接
    if(QAbstractSocket::UnconnectedState == clientSock->state())
    {
        qDebug() << "not connected to server, can not disconnect from server!";
        QMessageBox::warning(this, "断开连接", "您尚未连接，不可断开连接！");

        return;
    }

    // 正在传输不可点击
    if(isRecvPic)
    {
        qDebug() << "recving pic right now!";
        QMessageBox::warning(this, "断开连接", "您正在进行传输，不可断开连接！");

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
