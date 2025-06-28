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


/*��ΪVarifyGrpcClient�ǵ����࣬�����Ѿ���ioc�������������ӳ�
��������ж�����ӷ���VarifyGrpcClient���ͻ���ɳ�ͻ��
��ˣ�������һ��RPC���ӳ���������߳�����*/
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
    gateserver����GetVarifyCode��������GetVarifyCode�����ڲ�
    ����gRPC�ķ�����email����ȡVerifyServer����֤�룬����gateserver
    stub�൱�ڷ�����פ�ͻ��˴���������stub��GetVarifyCode����൱��
    ���÷�������GetVarifyCode����ȡ�����
    channel������ͨ������stub���ڱ���
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
