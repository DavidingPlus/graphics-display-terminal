#ifndef LOGIN_H
#define LOGIN_H

#include <QWidget>
#include <QMessageBox>

#include "widget.h"

namespace Ui
{
    class login;
}

class login : public QWidget
{
    Q_OBJECT

public:
    explicit login(QWidget *parent = nullptr);
    ~login();

signals:

private slots:
    void on_loginBtn_clicked();

    void on_exitBtn_clicked();

private:
    Ui::login *ui;

    // 存放主界面对象
    Widget w;
};

#endif // LOGIN_H
