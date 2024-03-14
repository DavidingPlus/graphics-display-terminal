#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QtNetwork>
#include <QString>
#include <QDebug>
#include <QByteArray>
#include <QFile>
#include <QDir>
#include <QImage>
#include <QPixmap>
#include <QMessageBox>
#include <QImageReader>
#include <QTimer>
#include <QVector>
#include <QLabel>

#include <synchapi.h>

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
    Widget(QWidget *parent = nullptr);
    ~Widget();

signals:

private slots:
    void on_connectBtn_clicked();

    void on_recvPicBtn_clicked();

    void on_disconnectBtn_clicked();

    void on_clearBtn_clicked();

private:
    // 给定秒数，将其转化为 xx小时xx分xx秒的格式
    QString secToString(int seconds);

    // 重置图片显示名字和内容，以及 frameMidLeft 的最小高，做一个代码复用
    void resetPicDisplay();

    // 将图形组件名称从英文到中文，为了演示，仅支持规定好的那些图形即可
    QString nameFromChtoEn(const QString& chName);

    bool isBase64(const char &c);

    std::string base64Decode(const std::string &encodedStr);

private:
    Ui::Widget *ui;

    // TcpSocket连接对象
    QTcpSocket *clientSock = nullptr;

    // 存储传递过来的编码过后的图片字节流
    QByteArray decode;

    // 存储写入文件的指针
    QFile *file = nullptr;

    // 存储接收图片的个数
    int picNum = 0;

    // 存储图片接受过程中接收的是第几张
    int picIndex = 0;

    // 维护当前接收的图片的文件大小
    int fileSize = 0;

    // 维护当前图片接收的字节进度
    int fileBytesRecv = 0;

    // 存储发送图片名字和大小的标志位
    bool isPicNameSize = false;

    // 记录是否在进行图片传输，防止接收过程中乱点
    bool isRecvingPic = false;

    // 定时器对象，维护在线时间
    QTimer *timer = nullptr;

    // 维护当前经过的秒数（仅作演示，不考虑超出 int 上限）
    int runSeconds = 0;

    // 存储绘图的 Label 指针对象
    QVector<QLabel *> picLabels;

    // 定义绘图图片之间的间隙（由于我设置程序无法拖动，因此这里的 gap 计算出一个固定值即可，不用考虑窗口大小带来的问题）
    // 设置 x 和 y 方向相同
    int gap = 0;

    // 定义图片像素大小（长、宽相同）
    static int picSize;
};
#endif // WIDGET_H
