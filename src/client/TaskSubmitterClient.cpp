#include <iostream>
#include <memory>
#include <string>
#include <grpcpp/grpcpp.h>
#include "swiftconsensus.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

using swiftconsensus::SwiftConsensusService;
using swiftconsensus::SubmitTaskRequest;
using swiftconsensus::SubmitTaskResponse;
using swiftconsensus::GetLeaderRequest;
using swiftconsensus::GetLeaderResponse;

class TaskSubmitterClient {
public:
    explicit TaskSubmitterClient(std::shared_ptr<Channel> channel)
        : stub_(SwiftConsensusService::NewStub(channel)) {}

    std::string GetLeader() {
        GetLeaderRequest request;
        GetLeaderResponse reply;
        ClientContext context;

        Status status = stub_->GetLeader(&context, request, &reply);
        if (status.ok()) {
            return reply.leader_id();
        } else {
            std::cerr << "[Error] Failed to get leader: " << status.error_message() << std::endl;
            return "";
        }
    }

    void SubmitTask(const std::string& leader_address,
                    const std::string& task_id,
                    const std::string& task_data,
                    int task_size) {
        SubmitTaskRequest request;
        SubmitTaskResponse reply;
        ClientContext context;

        request.set_task_id(task_id);
        request.set_task_data(task_data);
        request.set_task_size(task_size);

        auto stub = SwiftConsensusService::NewStub(
            grpc::CreateChannel(leader_address, grpc::InsecureChannelCredentials()));
        Status status = stub->SubmitTask(&context, request, &reply);

        if (status.ok()) {
            std::cout << "[Submit Result] " << reply.status() << std::endl;
        } else {
            std::cerr << "[Error] SubmitTask RPC failed: " << status.error_message() << std::endl;
        }
    }

private:
    std::unique_ptr<SwiftConsensusService::Stub> stub_;
};

int main(int argc, char** argv) {
    if (argc != 5) {
        std::cerr << "Usage: TaskSubmitterClient <any_node_address> <task_id> <task_data> <task_size>" << std::endl;
        return 1;
    }

    std::string initial_address = argv[1];   // e.g., "localhost:50052"
    std::string task_id = argv[2];
    std::string task_data = argv[3];
    int task_size = std::stoi(argv[4]);

    TaskSubmitterClient client(grpc::CreateChannel(initial_address, grpc::InsecureChannelCredentials()));

    std::string leader_id = client.GetLeader();
    if (leader_id.empty()) {
        std::cerr << "[Error] Could not determine leader. Aborting." << std::endl;
        return 1;
    }

    std::string leader_address = "localhost:" + leader_id.substr(7);  // e.g., "server_50053" → "localhost:50053"
    std::cout << "[Info] Submitting task to leader at " << leader_address << std::endl;
    client.SubmitTask(leader_address, task_id, task_data, task_size);

    return 0;
}
