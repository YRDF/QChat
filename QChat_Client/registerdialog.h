#ifndef REGISTERDIALOG_H
#define REGISTERDIALOG_H

#include <QDialog>
#include "global.h"
#include <QTimer>

namespace Ui {
class RegisterDialog;
}

class RegisterDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RegisterDialog(QWidget *parent = nullptr);
    ~RegisterDialog();

private slots:
    void on_get_code_clicked();
    void slot_reg_mod_finish(ReqId id,QString res,ErrorCodes err);
    void on_confirm_btn_clicked();

    void on_return_btn_clicked();

private:
    Ui::RegisterDialog *ui;
    void showTip(QString str,bool judge);
    //初始化http处理器，每个请求根据id区分
    void initHttpHandlers();
    //检测各个输入框是否正确
    bool checkUserValid();
    bool checkEmailValid();
    bool checkPassValid();
    bool checkConfirmValid();
    bool checkVarifyValid();
    //向错误容器中加入，删除错误
    void AddTipErr(TipErr te, QString tips);
    void DelTipErr(TipErr te);
    //错误容器
    QMap<TipErr, QString> _tip_errs;
    //回调函数容器
    QMap<ReqId, std::function<void(const QJsonObject&)>> _handlers;

    QTimer *_countdown_timer;
    int _countdown;
    //切换界面
    void ChangeTipPage();
signals:
    void sigSwitchLogin();
};

#endif // REGISTERDIALOG_H
