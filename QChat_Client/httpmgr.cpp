#include "httpmgr.h"


HttpMgr::~HttpMgr()
{

}

HttpMgr::HttpMgr() {
    connect(this,&HttpMgr::sig_http_finish,this,&HttpMgr::slot_http_finish);
}


void HttpMgr::PostHttpReq(QUrl url, QJsonObject json, ReqId req_id, Modules mod)
{
    //json对象序列化为字符串才发送
    QByteArray data = QJsonDocument(json).toJson();
    //http请求
    QNetworkRequest request(url);
    //告诉服务器，请求体中的数据是JSON格式
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");
    //设置HTTP请求体的字节长度
    request.setHeader(QNetworkRequest::ContentLengthHeader,QByteArray::number(data.length()));

    auto self = shared_from_this();
    //发送请求,获取回应
    QNetworkReply *reply = _manager.post(request,data);
    //查看是否完成
    connect(reply,&QNetworkReply::finished,[self,reply,req_id,mod](){
        //错误情况
        if(reply->error()!= QNetworkReply::NoError){
            qDebug()<<reply->errorString();
            //发送信号通知
            emit self->sig_http_finish(req_id,"",ErrorCodes::ERR_NETWORK,mod);
            reply->deleteLater();
            return;
        }
        //无错误
        QString res = reply->readAll();
        //通知其他界面
        emit self->sig_http_finish(req_id,res,ErrorCodes::SUCCESS,mod);
        reply->deleteLater();
        return;
    });
}

void HttpMgr::slot_http_finish(ReqId id, QString res, ErrorCodes err, Modules mod)
{
    if(mod == Modules::REGISTERMOD){
        //是注册模块，发送信号通知指定模块:http的注册响应结束了
        emit sig_reg_mod_finish(id,res,err);
    }
    if(mod == Modules::RESETMOD){
        //是重置模块，发送信号通知指定模块http响应结束
        emit sig_reset_mod_finish(id, res, err);
    }

    if(mod == Modules::LOGINMOD){
        //是登陆模块，发送信号通知指定模块http响应
        emit sig_login_mod_finish(id, res, err);
    }
}
