#include <iostream>
#include <grpcpp/grpcpp.h>
#include "go/grpc_server/gen/libcore.grpc.pb.h"

using grpc::Channel;
using libcore::LibcoreService;

class libcoreClient {
  public:
    explicit libcoreClient(std::shared_ptr<Channel> channel, std::string token);

    libcore::TestResp Test(bool *rpcOK, const libcore::TestReq &request);
  private:
    std::unique_ptr<libcore::LibcoreService::Stub> stub_;
    std::string token_;
};
