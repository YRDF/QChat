#pragma once
#include <grpcpp/grpcpp.h>
#include "message.grpc.pb.h"
#include "const.h"
#include "Singleton.h"
using grpc::Channel;
using grpc::Status;
using grpc::ClientContext;

using message::GetVarifyReq;
using message::GetVarifyRsp;
using message::VarifyService;

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

        Status status = stub_->GetVarifyCode(&context, request, &reply);

        if (status.ok()) {

            return reply;
        }
        else {
            reply.set_error(ErrorCodes::RPCFailed);
            return reply;
        }
	}

private:
    VarifyGrpcClient() {
        std::shared_ptr<Channel> channel = grpc::CreateChannel("0.0.0.0:50051", grpc::InsecureChannelCredentials());
        stub_ = VarifyService::NewStub(channel);
    }

    std::unique_ptr<VarifyService::Stub> stub_;
};

