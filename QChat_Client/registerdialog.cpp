#include "registerdialog.h"
#include "ui_registerdialog.h"
#include"httpmgr.h"

RegisterDialog::RegisterDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::RegisterDialog)
{
    ui->setupUi(this);
    ui->pass_edit->setEchoMode(QLineEdit::Password);
    ui->confirm_edit->setEchoMode(QLineEdit::Password);
    ui->err_tip->setProperty("state","normal");
    repolish(ui->err_tip);

    initHttpHandlers();
    connect(HttpMgr::GetInstance().get(),&HttpMgr::sig_reg_mod_finish,
            this,&RegisterDialog::slot_reg_mod_finish);
}

RegisterDialog::~RegisterDialog()
{
    delete ui;
}

void RegisterDialog::showTip(QString str,bool judge)
{
    if(judge == false){
    ui->err_tip->setProperty("state","err");
    repolish(ui->err_tip);
    }else if(judge == true){
        ui->err_tip->setProperty("state","normal");
    }
    ui->err_tip->setText(str);
    repolish(ui->err_tip);
}

void RegisterDialog::initHttpHandlers()
{
    //注册获取验证码的回调函数ID_GET_VARIFY_CODE
    _handlers.insert(ReqId::ID_GET_VARIFY_CODE,[this](const QJsonObject& jsonObj){
        int error = jsonObj["error"].toInt();
        if(error != ErrorCodes::SUCCESS){
            showTip(tr("参数错误!"),false);
            return;
        }
        auto email = jsonObj["email"].toString();
        showTip(tr("验证码已经发送到邮箱!"),true);
        qDebug()<<"email is: "<<email;
    });

    //注册点击注册按钮成功进行注册的回调函数ID_REG_USER
    _handlers.insert(ReqId::ID_REG_USER,[this](const QJsonObject& jsonObj){
        int error = jsonObj["error"].toInt();
        if(error != ErrorCodes::SUCCESS){
            showTip(tr("参数错误!"),false);
            return;
        }
        auto user_name = jsonObj["user"].toString();
        showTip(tr("注册成功！！！"),true);
        qDebug()<<"uid is "<<jsonObj["uid"].toInt();
        qDebug()<<"new user resign success: "<<user_name;
    });
}

void RegisterDialog::on_get_code_clicked()
{
    auto email = ui->email_edit->text();
    // 邮箱地址的正则表达式
    QRegularExpression regex(R"((\w+)(\.|_)?(\w*)@(\w+)(\.(\w+))+)");
    bool match = regex.match(email).hasMatch(); // 执行正则表达式匹配
    if(match){
        //发送http请求获取验证码
        QJsonObject json_obj;
        json_obj["email"] = email;
        HttpMgr::GetInstance()->PostHttpReq(QUrl(gate_url_prefix+"/get_varifycode"),
                                            json_obj, ReqId::ID_GET_VARIFY_CODE,Modules::REGISTERMOD);
    }else{
        //提示邮箱不正确
        showTip(tr("邮箱地址不正确"),false);
    }
}

void RegisterDialog::slot_reg_mod_finish(ReqId id, QString res, ErrorCodes err)
{
    if(err !=ErrorCodes::SUCCESS){
        showTip(tr("网络请求错误"),false);
        return;
    }
    //解析json字符串
    QJsonDocument jsonDoc = QJsonDocument::fromJson(res.toUtf8());
    if(jsonDoc.isNull()){
        showTip(tr("json解析失败！"),false);
        return;
    }
    //解析错误----能否转成jsonObject
    if(!jsonDoc.isObject()){
        showTip(tr("json解析错误！"),false);
        return;
    }
    //json文档转成json对象
    //因为解析出来的是参数，我们可以把其传递给回调函数进行调用
    //回调函数根据不同参数执行不同回调
    _handlers[id](jsonDoc.object());
    return;
}


void RegisterDialog::on_confirm_btn_clicked()
{
    if(ui->user_edit->text() == ""){
        showTip(tr("用户名不能为空"), false);
        return;
    }

    if(ui->email_edit->text() == ""){
        showTip(tr("邮箱不能为空"), false);
        return;
    }

    if(ui->pass_edit->text() == ""){
        showTip(tr("密码不能为空"), false);
        return;
    }

    if(ui->confirm_edit->text() == ""){
        showTip(tr("确认密码不能为空"), false);
        return;
    }

    if(ui->confirm_edit->text() != ui->pass_edit->text()){
        showTip(tr("密码和确认密码不匹配"), false);
        return;
    }

    if(ui->varify_edit->text() == ""){
        showTip(tr("验证码不能为空"), false);
        return;
    }

    //day11 发送http请求注册用户
    QJsonObject json_obj;
    json_obj["user"] = ui->user_edit->text();
    json_obj["email"] = ui->email_edit->text();
    json_obj["passwd"] = ui->pass_edit->text();
    json_obj["confirm"] = ui->confirm_edit->text();
    json_obj["varifycode"] = ui->varify_edit->text();
    HttpMgr::GetInstance()->PostHttpReq(QUrl(gate_url_prefix+"/user_register"),
                                        json_obj, ReqId::ID_REG_USER,Modules::REGISTERMOD);
}

