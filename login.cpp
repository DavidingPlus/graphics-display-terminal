#include "login.h"
#include "ui_login.h"

login::login(QWidget *parent)
    : QWidget(parent),
      ui(new Ui::login)
{
    ui->setupUi(this);
}

login::~login()
{
    delete ui;
}

void login::on_loginBtn_clicked()
{
    // 做账号密码校验
    // TODO： 目前做假校验，后续考虑连数据库或者查服务端文件
    if (ui->userNameLineEdit->text().isEmpty() or ui->passwordLineEdit->text().isEmpty())
    {
        QMessageBox::warning(this, "登录", "用户名或密码不能为空!");
        return;
    }

    if (!(QString("admin") == ui->userNameLineEdit->text() and QString("123456") == ui->passwordLineEdit->text()))
    {
        QMessageBox::warning(this, "登录", "用户名或密码错误!");
        return;
    }

    w.show();
    this->close();
}

void login::on_exitBtn_clicked()
{
    this->close();
}
