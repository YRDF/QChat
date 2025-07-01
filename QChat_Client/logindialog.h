#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include <QDebug>

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
private:
    Ui::LoginDialog *ui;
public slots:
    void slot_forget_pwd();
};

#endif // LOGINDIALOG_H
