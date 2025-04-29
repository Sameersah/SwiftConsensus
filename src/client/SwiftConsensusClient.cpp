#include "SwiftConsensusClient.h"
#include <iostream>

SwiftConsensusClient::SwiftConsensusClient(std::shared_ptr<grpc::Channel> channel, std::string address)
    : stub_(swiftconsensus::SwiftConsensusService::NewStub(channel)), server_address_(address) {}

void SwiftConsensusClient::SendHeartbeat(const std::string& server_id, double cpu, double memory, int queue, double score) {
    HeartbeatRequest request;
    request.set_server_id(server_id);
    request.set_timestamp(std::chrono::system_clock::now().time_since_epoch().count());
    request.set_cpu_free_percent(cpu);
    request.set_memory_free_percent(memory);
    request.set_task_queue_length(queue);
    request.set_calculated_score(score);

    HeartbeatResponse reply;
    ClientContext context;
    Status status = stub_->SendHeartbeat(&context, request, &reply);

    if (status.ok()) {
        std::cout << "[Heartbeat ACK]: " << reply.message() << std::endl;
    } else {
        std::cout << "[Heartbeat Failed]" << std::endl;
    }
}

void SwiftConsensusClient::AssignTask(const std::string& task_id, const std::string& self_id, const std::string& task_data) {
    TaskAssignmentRequest request;
    request.set_task_id(task_id);
    request.set_assigned_by(self_id);
    request.set_task_data(task_data);

    TaskAssignmentResponse reply;
    ClientContext context;
    Status status = stub_->AssignTask(&context, request, &reply);

    if (status.ok()) {
        std::cout << "[Task ACK]: " << reply.message() << std::endl;
    } else {
        std::cout << "[Task Assignment Failed]" << std::endl;
    }
}

std::string SwiftConsensusClient::GetServerAddress() const {
    return server_address_;
}
