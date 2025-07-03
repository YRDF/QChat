#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include <QDebug>
#include "global.h"

namespace Ui {
class LoginDialog;
}

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoginDialog(QWidget *parent = nullptr);
    ~LoginDialog();
signals:
    void switchRegister();
    void switchReset();
    void sig_connect_tcp(ServerInfo);
private:
    //初始化头像
    void initHead();
    //检测email和密码,出错就向容器中写入错误
    bool checkUserValid();
    bool checkPwdValid();
    QMap<TipErr, QString> _tip_errs;
    void AddTipErr(TipErr te,QString tips);
    void DelTipErr(TipErr te);
    void showTip(QString str,bool b_ok);
    int _uid;
    QString _token;
    //登录按钮等到收到服务器验证结果才能再次被点
    bool enableBtn(bool);
    //注册服务器逻辑到map中
    void initHttpHandlers();
    QMap<ReqId, std::function<void(const QJsonObject&)>> _handlers;
    Ui::LoginDialog *ui;
public slots:
    void slot_forget_pwd();
private slots:
    void on_login_btn_clicked();
    void slot_login_mod_finish(ReqId id, QString res, ErrorCodes err);
    void slot_tcp_con_finish(bool bsuccess);
    void slot_login_failed(int);
};

#endif // LOGINDIALOG_H
