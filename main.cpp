#include "widget.h"

#include <synchapi.h>
#include <QApplication>
#include <QtNetwork>
#include <QDebug>

#define maxBufferSize 1024

int main(int argc, char* argv[])
{
//    QApplication a(argc, argv);
//    Widget w;
//    w.show();
//    return a.exec();

    QTcpSocket clientSock;
    clientSock.connectToHost(QString("106.55.60.140"), uint16_t(8080));

    Sleep(1000);
}
