#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>

#include <synchapi.h>
#include <QtNetwork>
#include <QDebug>
#include <QByteArray>
#include <QFile>
#include <QImage>
#include <QPixmap>

#include <iostream>
#include <string>
#include <cstring>

#define maxBufferSize 1024

QT_BEGIN_NAMESPACE
namespace Ui
{
    class Widget;
}
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget* parent = nullptr);
    ~Widget();

signals:
    // 发送图片接收和写入文件完毕的信号，准备绘图
    void recvAndWriteOver();

private:
    bool isBase64(const char& c);
    std::string base64Decode(const std::string& encodedStr);

private slots:
    void on_connectBtn_clicked();

    void on_recvBtn_clicked();

    void on_disconnectBtn_clicked();

private:
    Ui::Widget* ui;

    // TcpSocket连接对象
    QTcpSocket* clientSock = nullptr;

    // 存储传递过来的编码过后的图片字节流
    QByteArray decode;

    // 存储写入文件的指针
    QFile* file = nullptr;
};
#endif // WIDGET_H
