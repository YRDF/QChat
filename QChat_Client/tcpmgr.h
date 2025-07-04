#ifndef TCPMGR_H
#define TCPMGR_H
#include<QTcpSocket>
#include"singleton.h"
#include"global.h"
#include<functional>
#include<QObject>
#include <QAbstractSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

class TcpMgr:public QObject,public Singleton<TcpMgr>,
               public std::enable_shared_from_this<TcpMgr>
{
    Q_OBJECT
public:
    ~ TcpMgr();
private:
    friend class Singleton<TcpMgr>;
    TcpMgr();
    //注册一些回调函数
    void initHandlers();
    //处理数据(其实就是在map中寻找回调进行调用)
    void handleMsg(ReqId id, int len, QByteArray data);
    //tcp连接和切包
    QTcpSocket _socket;
    QString _host;
    uint16_t _port;
    QByteArray _buffer;
    bool _b_recv_pending;
    quint16 _message_id;
    quint16 _message_len;
    QMap<ReqId, std::function<void(ReqId id, int len, QByteArray data)>> _handlers;
public slots:
    void slot_tcp_connect(ServerInfo si);
    void slot_send_data(ReqId reqId, QByteArray dataBytes);
signals:
    void sig_con_success(bool bsuccess);
    void sig_send_data(ReqId reqId, QByteArray data);
    void sig_swich_chatdlg();
    void sig_load_apply_list(QJsonArray json_array);
    void sig_login_failed(int);
    // void sig_user_search(std::shared_ptr<SearchInfo>);
    // void sig_friend_apply(std::shared_ptr<AddFriendApply>);
    // void sig_add_auth_friend(std::shared_ptr<AuthInfo>);
    // void sig_auth_rsp(std::shared_ptr<AuthRsp>);
    // void sig_text_chat_msg(std::shared_ptr<TextChatMsg> msg);
};

#endif // TCPMGR_H
