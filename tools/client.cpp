#include <iostream>

#include "tools/client.hpp"
#include <grpc/support/log.h>
#include <grpcpp/grpcpp.h>
#include "go/grpc_server/gen/libcore.grpc.pb.h"


using grpc::Channel;
using grpc::ClientAsyncResponseReader;
using grpc::ClientContext;
using grpc::CompletionQueue;
using grpc::Status;
using libcore::LibcoreService;


libcoreClient::libcoreClient(std::shared_ptr<Channel> channel, std::string token){
    this -> stub_ = LibcoreService::NewStub(channel);
    this -> token_ = token;
}

libcore::TestResp libcoreClient::Test(bool *rpcOK, const libcore::TestReq &request) {
    libcore::TestResp reply;
    // qDebug() << "befor grpc";
    ClientContext context;
    context.AddMetadata("nekoray_auth", token_);
    Status status = stub_->Test(&context, request, &reply);
    // qDebug() << "here we go";
    if (status.ok()) {
        *rpcOK = true; 
        return reply;
    } else {
        std::cout << status.error_message() << std::endl;
        *rpcOK = false; 
        return reply;
    }
}
