#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>

#include <synchapi.h>
#include <QtNetwork>
#include <QDebug>

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

};
#endif // WIDGET_H
