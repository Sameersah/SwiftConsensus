#ifndef SWIFT_CONSENSUS_CLIENT_H
#define SWIFT_CONSENSUS_CLIENT_H

#include <memory>
#include <string>
#include <grpcpp/grpcpp.h>
#include "swiftconsensus.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using swiftconsensus::HeartbeatRequest;
using swiftconsensus::HeartbeatResponse;
using swiftconsensus::SwiftConsensusService;
using swiftconsensus::TaskAssignmentRequest;
using swiftconsensus::TaskAssignmentResponse;

class SwiftConsensusClient {
public:
    SwiftConsensusClient(std::shared_ptr<Channel> channel, std::string address);

    void SendHeartbeat(const std::string& server_id, double cpu, double memory, int queue, double score);
    void AssignTask(const std::string& task_id, const std::string& self_id, const std::string& task_data);
    std::string GetServerAddress() const;

private:
    std::unique_ptr<SwiftConsensusService::Stub> stub_;
    std::string server_address_;
};

#endif // SWIFT_CONSENSUS_CLIENT_H
