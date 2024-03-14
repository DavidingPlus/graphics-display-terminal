#include "widget.h"
#include "ui_widget.h"

// 静态成员在类外初始化，在演示 Demo 中就定死为 150 了，后续不要动
int Widget::picSize = 150;

Widget::Widget(QWidget *parent)
    : QWidget(parent),
      ui(new Ui::Widget)
{
    ui->setupUi(this);

    // 设置大小为固定，具体大小 ui 中给出
    setFixedSize(width(), height());

    // 初始化定时器对象
    timer = new QTimer(this);
    // 绑定信号槽，每隔一秒超时
    // 启动一次就行了，会每隔这么久时间超时，若想停止定时器就用 stop 函数
    connect(timer, &QTimer::timeout, this, [=]()
            {
        ++runSeconds;
        // 修改在线时间内容
        ui->timeLabel->setText(QString("在线时长: %1").arg(secToString(runSeconds))); });
    // 启动定时器
    timer->start(1000); // 单位毫秒

    // note： Qt 中的 res.qrc 中的资源文件不能使用中文名称，否则会出问题（亲自试验过）
    // note： Qt 中的服务端和客户端的正确方法，和自己用过的 Linux 平台接口不同，用的时候记得查资料
    clientSock = new QTcpSocket(this);
    decode.clear();

    // 初始化 gap
    // note： 我的设计中，一行放三个，每个大小 150 * 150
    gap = (ui->frameMidLeft->width() - 3 * picSize) / 4;

    // 绑定信号槽，服务端发送信息过来之后自动打印（前提是客户端发送有效信息）
    connect(clientSock, &QTcpSocket::readyRead, this, [=]()
            {
        // TODO： 从我自己的服务器上读取数据，由于传输原因可能导致粘包，但现在不做处理，因为我现在演示在本地起服务端，仅用作演示就不管了
        QByteArray readBuf = clientSock->read(maxBufferSize); // 与服务端发送的包大小对应，即 1024
        qDebug() << readBuf;

        // 做退出的判断，什么也不做，断开连接在槽函数中处理
        if( QString("exit success\n") == QString(readBuf))
            return;

        // 图片个数
        if( 0 == picNum)
        {
            picNum = readBuf.toInt();

            // 修改正在接受图形组件 processLabel
            ui->processLabel->setText(QString("正在接收图形组件: %1/%2").arg(picIndex).arg(picNum));

            return;
        }

        // 图片名字和大小
        if(isPicNameSize)
        {
            QString fileNameSize = QString(readBuf);
            auto nameSizeList = fileNameSize.split(QChar('\n'));

            // 处理文件大小和当前接受字节数
            fileSize = nameSizeList[1].toInt();
            fileBytesRecv = 0;

            // 进度条清空
            ui->progressBar->setValue(0);

            // 打开文件
            QString filePath = QString("./res/") + nameSizeList[0];
            file = new QFile(filePath, this);
            file->open(QFile::ReadWrite);

            isPicNameSize = false;

            // 修改 picNameLabel
            QString name = file->fileName();// 这里返回的其实是路径，所以我命名才没用 fileName ，就是怕混淆，例如 ./res/rect.png
            //弹掉前面的 ./res/ 和后面的 .png
            name.remove(0, 6);
            name.remove(name.size() - 4, 4);
            ui->picNameLabel->setText(QString("正在接收:\n%1").arg(nameFromChtoEn(name)));

            // 修改当前接收的图片 picRecvingLabel ，绘图
            ui->picRecvingLabel->setScaledContents(true); // 这条是让图片自适应 Label ，不然显示不完整
            ui->picRecvingLabel->setPixmap(QString(":/res/%1.png").arg(name));
            ui->picRecvingLabel->show(); //将其 show 出来，否则不会展示

            // 修改正在接受图形组件 processLabel
            ui->processLabel->setText(QString("正在接收图形组件: %1/%2").arg(1 + picIndex).arg(picNum));

            return;
        }

        // 做结束读取的判断
        if( QString("send over\n") == QString(readBuf))
        {
            decode.clear();

            // 关闭文件
            file->close();

            // 图片已经写入文件完毕，绘图
            // note： 必须先把文件指针关闭，否则由于文件打开，其他操作可能受限，图片可能加载不出来（之前就是这样）
            QLabel* newLabel = new QLabel(ui->frameMidLeft);

            // 修改 Label 位置和大小
            int x = ui->frameMidLeft->x() + gap + (picIndex % 3) * (picSize + gap),
                y = ui->frameMidLeft->y() + gap + (picIndex / 3) * (picSize + gap);

            // 如果计算出的这个 y 加上图片 size 加上一个 gap 超 frameMidLeft 的 高了，那么需要扩高
            if( y + picSize + gap > ui->frameMidLeft->height())
                // 这里直接改 size 是没用的，需要改 minimum 才行
                ui->frameMidLeft->setMinimumHeight(y + picSize + gap);

            newLabel->setGeometry(x, y, picSize, picSize);

            // 绘图
            newLabel->setScaledContents(true); // 这条是让图片自适应 Label ，不然显示不完整
            newLabel->setPixmap(file->fileName());
            newLabel->show(); //将其 show 出来，否则不会展示

            // 加入数组，方便后续销毁
            picLabels.push_back(newLabel);

            // 修改图片名字标志位，下一次可读取
            isPicNameSize = true;

            // 如果全部接收完毕，弹出一个提示窗口
            if( picNum == ++picIndex)
            {
                qDebug() << "recv pic successfully.";
                QMessageBox::information(this, "接收图片", "图片数据接收完毕！");

                isRecvingPic = false;
            }

            return;
        }

        // 写入字节数组
        decode = QByteArray::fromStdString(base64Decode(readBuf.toStdString()));
        file->write(decode);

        // 修改当前接收字节数，注意，需要的是解码之后的大小
        fileBytesRecv += decode.size();
        // 修改进度条
        ui->progressBar->setValue(float(100) * float(fileBytesRecv) / float(fileSize)); });
}

Widget::~Widget()
{
    delete ui;
}

void Widget::on_connectBtn_clicked()
{
    // 已连接不可再连接
    if (QAbstractSocket::ConnectedState == clientSock->state())
    {
        qDebug() << "already connected to server, can not connect again!";
        QMessageBox::warning(this, "连接服务", "您已经连接上了服务端，不可再次连接！");

        return;
    }

    // 正在传输不可点击
    if (isRecvingPic)
    {
        qDebug() << "recving pic right now!";
        QMessageBox::warning(this, "连接服务", "您正在进行传输，不可进行连接！");

        return;
    }

    // 连接服务端
    // 通过读取 lineEdit 中的信息来连接
    QString serverIP = ui->IPEdit->text();
    quint16 serverPort = quint16(ui->portEdit->text().toInt());
    clientSock->connectToHost(serverIP, serverPort);

    // 等待连接成功，等待时间3秒，太长经测试会让程序卡死
    bool res = clientSock->waitForConnected(3000); // 单位是毫秒
    if (false == res)
    {
        qDebug() << "fail to connect to server, please check your IP or port.";
        QMessageBox::warning(this, "连接服务", "服务端 IP 或端口错误，请您检查后重试！");

        return;
    }

    // 等待状态转换完成
    while (clientSock->state() != QAbstractSocket::ConnectedState)
        QCoreApplication::processEvents();

    // 修改 isConnected 标签相关内容
    ui->isConnectedLabel->setText("已连接");
    ui->isConnectedPicLabel->setPixmap(QPixmap(":/res/connected.png"));

    qDebug() << "connect to server successfully.";
    QMessageBox::information(this, "连接服务", "连接服务端成功！");
}

void Widget::on_recvPicBtn_clicked()
{
    // 未连接不可发送信息
    if (QAbstractSocket::UnconnectedState == clientSock->state())
    {
        qDebug() << "not connected to server, can not recv any message from server!";
        QMessageBox::warning(this, "接收图片", "您尚未连接，不可接收图片！");

        return;
    }

    // 正在传输不可再次点击
    if (isRecvingPic)
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
    isPicNameSize = true;

    // 修改是否正在传输标志位
    isRecvingPic = true;

    // 重置文件大小和当前接收字节数
    fileSize = 0;
    fileBytesRecv = 0;

    // 进度条清空
    ui->progressBar->setValue(0);

    // 处理存储图片的目录，先删除在创建
    QDir dir("./"),
        subDir("./res/");
    subDir.removeRecursively();
    dir.mkdir("./res/");

    // 重置图片显示名字和内容，以及 frameMidLeft 的最小高
    resetPicDisplay();

    // 发送信息
    QString sendMessage("send\n");
    clientSock->write(sendMessage.toUtf8());
    // 不缓存，直接发送，否则不会立即发出去
    clientSock->flush();
}

void Widget::on_disconnectBtn_clicked()
{
    // 未连接不可断开连接
    if (QAbstractSocket::UnconnectedState == clientSock->state())
    {
        qDebug() << "not connected to server, can not disconnect from server!";
        QMessageBox::warning(this, "断开连接", "您尚未连接，不可断开连接！");

        return;
    }

    // 正在传输不可点击
    if (isRecvingPic)
    {
        qDebug() << "recving pic right now!";
        QMessageBox::warning(this, "断开连接", "您正在进行传输，不可断开连接！");

        return;
    }

    // 发送断开连接信息
    QString sendMessage("exit\n");
    clientSock->write(sendMessage.toUtf8());
    clientSock->flush();

    // 断开连接
    clientSock->disconnectFromHost();
    qDebug() << "disconnect from server successfully.";

    // 修改 isConnected 标签相关内容
    ui->isConnectedLabel->setText("未连接");
    ui->isConnectedPicLabel->setPixmap(QPixmap(":/res/unConnected.png"));

    // 弹出 MessageBox ，个人感觉在未连接文字和图标状态修改后弹出比较合适
    QMessageBox::information(this, "断开连接", "与服务端断开连接成功！");
}

void Widget::on_clearBtn_clicked()
{
    // 正在传输不可点击
    if (isRecvingPic)
    {
        qDebug() << "recving pic right now!";
        QMessageBox::warning(this, "清空显示", "您正在进行传输，不可清空显示！");

        return;
    }

    // 进度条清空
    ui->progressBar->setValue(0);

    // 重置图片显示名字和内容，以及 frameMidLeft 的最小高
    resetPicDisplay();

    // 修改正在接受图形组件 processLabel
    ui->processLabel->setText(QString("正在接收图形组件: 0/0"));

    qDebug() << "clear pic display successfully.";
    QMessageBox::information(this, "清空显示", "清空显示成功!");
}

QString Widget::secToString(int seconds)
{
    // 0 <= seconds < 60， 显示 xx 秒
    if (seconds >= 0 and seconds < 60)
        return QString("%1秒").arg(seconds);
    // 60 <= seconds < 3600， 显示 xx 分 xx 秒
    if (seconds >= 60 and seconds < 3600)
        return QString("%1分%2秒").arg(seconds / 60).arg(seconds % 60);
    // seconds >= 3600， 显示 xx 小时 xx 分 xx 秒（仅作演示，不做年月天的处理）
    else
        return QString("%1小时%2分%3秒").arg(seconds / 3600).arg(seconds % 3600 / 60).arg(seconds % 3600 % 60);
}

void Widget::resetPicDisplay()
{
    // 重置图片显示名字和内容
    for (auto &picLabel : picLabels)
    {
        delete picLabel;
        picLabel = nullptr;
    }
    picLabels.clear();

    // 修改 picNameLabel
    ui->picNameLabel->setText(QString("正在接收:"));

    // 修改正在接收图片 picRecvingLabel
    ui->picRecvingLabel->setPixmap(QPixmap());
    ui->picRecvingLabel->setText(QString("无图片"));

    // 重置 frameMidLeft 的最小高， 0 即可
    ui->frameMidLeft->setMinimumHeight(0);
}

QString Widget::nameFromChtoEn(const QString &chName)
{
    if(QString("circle") == chName)
        return QString("圆形");
    else if(QString("cross") == chName)
        return QString("十字形");
    else if(QString("ellipse") == chName)
        return QString("椭圆形");
    else if(QString("hexagon") == chName)
        return QString("六边形");
    else if(QString("hexagram") == chName)
        return QString("六角形形");
    else if(QString("octagon") == chName)
        return QString("八边形");
    else if(QString("parallelogram") == chName)
        return QString("平行四边形");
    else if(QString("pentagon") == chName)
        return QString("五边形");
    else if(QString("pentagram") == chName)
        return QString("五角星形");
    else if(QString("rectangle") == chName)
        return QString("长方形");
    else if(QString("rhombus") == chName)
        return QString("菱形");
    else if(QString("square") == chName)
        return QString("正方形");
    else if(QString("trapezoid") == chName)
        return QString("梯形");
    else if(QString("triangle") == chName)
        return QString("三角形");
    else
        return QString();
}

bool Widget::isBase64(const char &c)
{
    return isalnum(c) or ('+' == c) or ('/' == c);
}

std::string Widget::base64Decode(const std::string &encodedStr)
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
