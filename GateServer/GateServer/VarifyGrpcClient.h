#pragma once
#include <grpcpp/grpcpp.h>
#include "message.grpc.pb.h"
#include "const.h"
#include "Singleton.h"
#include "ConfigMgr.h"

using grpc::Channel;
using grpc::Status;
using grpc::ClientContext;

using message::GetVarifyReq;
using message::GetVarifyRsp;
using message::VarifyService;


/*因为VarifyGrpcClient是单例类，我们已经把ioc连接作成了连接池
所以如果有多个连接访问VarifyGrpcClient，就会造成冲突。
因此，我们做一个RPC连接池来处理多线程链接*/
class RPConPool
{
public:
    RPConPool(size_t poolSize, std::string host, std::string port);
    ~RPConPool();
    void Close();
    std::unique_ptr<VarifyService::Stub> getConnection();
    void returnConnection(std::unique_ptr<VarifyService::Stub> context);
private:
    std::atomic<bool> b_stop_;
    size_t poolSize_;
    std::string host_;
    std::string port_;
    std::queue<std::unique_ptr<VarifyService::Stub>> connections_;
    std::mutex mutex_;
    std::condition_variable cond_;
};


/*
    gateserver调用GetVarifyCode函数，在GetVarifyCode函数内部
    调用gRPC的服务传送email，获取VerifyServer的验证码，传回gateserver
    stub相当于服务器驻客户端代表，调用了stub的GetVarifyCode后就相当于
    调用服务器的GetVarifyCode，获取结果。
    channel是连接通道放入stub长期保活
*/
class VarifyGrpcClient:public Singleton<VarifyGrpcClient>
{
	friend class Singleton<VarifyGrpcClient>;
public:
	GetVarifyRsp GetVarifyCode(std::string email) {
        ClientContext context;
        GetVarifyRsp reply;
        GetVarifyReq request;
        request.set_email(email);

        auto stub = pool_->getConnection();

        Status status = stub->GetVarifyCode(&context, request, &reply);

        if (status.ok()) {
            pool_->returnConnection(std::move(stub));
            return reply;
        }
        else {
            pool_->returnConnection(std::move(stub));
            reply.set_error(ErrorCodes::RPCFailed);
            return reply;
        }
	}

private:
    VarifyGrpcClient() {
        auto& gCfgMgr = ConfigMgr::Inst();
        std::string host = gCfgMgr["VarifyServer"]["Host"];
        std::string port = gCfgMgr["VarifyServer"]["Port"];
        pool_.reset(new RPConPool(5, host, port));
    }
    std::unique_ptr<RPConPool> pool_;

};
