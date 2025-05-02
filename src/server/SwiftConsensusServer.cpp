#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <chrono>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <cstdlib>
#include <limits>
#include <atomic>

#include <grpcpp/grpcpp.h>
#include "swiftconsensus.grpc.pb.h"

#include "common/PeerTable.h"
#include "client/SwiftConsensusClient.h"
#include "server/LeaderElectionManager.h"
#include "server/FailureDetector.h"
#include "common/SystemMetrics.h"


using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using grpc::Channel;
using grpc::ClientContext;

using swiftconsensus::HeartbeatRequest;
using swiftconsensus::HeartbeatResponse;
using swiftconsensus::TaskAssignmentRequest;
using swiftconsensus::TaskAssignmentResponse;
using swiftconsensus::SubmitTaskRequest;
using swiftconsensus::SubmitTaskResponse;
using swiftconsensus::GetLeaderRequest;
using swiftconsensus::GetLeaderResponse;
using swiftconsensus::SwiftConsensusService;


std::string self_id;
std::string self_address;
std::atomic<int> current_queue_length{0};


class SwiftConsensusServiceImpl final : public SwiftConsensusService::Service {
public:
    SwiftConsensusServiceImpl(PeerTable& peerTable,
                              LeaderElectionManager& electionManager,
                              std::vector<std::shared_ptr<SwiftConsensusClient>>& clients)
        : peerTable_(peerTable),
          electionManager_(electionManager),
          clients_(clients) {}

    Status SendHeartbeat(ServerContext* context, const HeartbeatRequest* request,
                         HeartbeatResponse* reply) override {
        peerTable_.updatePeer(request->server_id(), request->calculated_score());
        std::cout << "[Heartbeat Received] " << request->server_id()
                  << " | Score: " << request->calculated_score()
                  << " | CPU: " << request->cpu_free_percent()
                  << " | Memory: " << request->memory_free_percent()
                  << " | Queue: " << request->task_queue_length()
                  << std::endl;

        reply->set_message("Heartbeat received");
        return Status::OK;
    }

    Status AssignTask(ServerContext* context, const TaskAssignmentRequest* request,
                      TaskAssignmentResponse* reply) override {
        std::cout << "[Task Received] TaskID: " << request->task_id()
                  << " Assigned By: " << request->assigned_by()
                  << " | Task Data: " << request->task_data() << std::endl;

        current_queue_length++;

        std::this_thread::sleep_for(std::chrono::seconds(2));

        std::cout << "[Task Completed] TaskID: " << request->task_id() << std::endl;
        current_queue_length--;

        reply->set_message("Task processed successfully");
        return Status::OK;
    }

    Status SubmitTask(ServerContext* context, const SubmitTaskRequest* request,
                      SubmitTaskResponse* reply) override {
        if (electionManager_.getCurrentLeader() != self_id) {
            reply->set_status("❌ Not the leader");
            return Status::OK;
        }

        auto peers = peerTable_.getAllPeers();
        std::string best_worker = "";
        double best_score = std::numeric_limits<double>::lowest();
        double penalty = 10.0;

        for (const auto& [server_id, info] : peers) {
            if (!info.is_alive) continue;

            double effective_score = info.score;
            if (server_id == self_id) {
                effective_score -= penalty;
            }

            if (effective_score > best_score) {
                best_score = effective_score;
                best_worker = server_id;
            }
        }

        if (best_worker.empty()) {
            reply->set_status("❌ No alive workers");
            return Status::OK;
        }

        std::string task_id = request->task_id();
        std::string task_data = request->task_data();

        std::cout << "[Leader Routing Task] " << task_id << " → " << best_worker
                  << " (effective score: " << best_score << ")" << std::endl;

        for (auto& client : clients_) {
            if (client->GetServerAddress() == "localhost:" + best_worker.substr(7)) {
                client->AssignTask(task_id, self_id, task_data);
                break;
            }
        }

        reply->set_status("✅ Task assigned to best peer");
        return Status::OK;
    }

    Status GetLeader(ServerContext* context, const GetLeaderRequest* request,
                     GetLeaderResponse* reply) override {
        std::string leader = electionManager_.getCurrentLeader();
        reply->set_leader_id(leader);
        std::cout << "[GetLeader] Responding with current leader: " << leader << std::endl;
        return Status::OK;
    }

private:
    PeerTable& peerTable_;
    LeaderElectionManager& electionManager_;
    std::vector<std::shared_ptr<SwiftConsensusClient>>& clients_;
};


void RunServer(std::string server_address,
               PeerTable& peerTable,
               LeaderElectionManager& electionManager,
               std::vector<std::shared_ptr<SwiftConsensusClient>>& clients) {
    SwiftConsensusServiceImpl service(peerTable, electionManager, clients);

    ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cout << "[Server Started] Listening on " << server_address << std::endl;

    server->Wait();
}


int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: SwiftConsensusServer <port>" << std::endl;
        return 1;
    }

    std::string port = argv[1];
    self_id = "server_" + port;
    self_address = "localhost:" + port;

    PeerTable peerTable;
    LeaderElectionManager electionManager(self_id, peerTable);
    FailureDetector failureDetector(peerTable);

    std::vector<std::shared_ptr<SwiftConsensusClient>> clients;

    std::vector<std::string> peer_addresses = {
        "localhost:50056",
        "localhost:50052",
        "localhost:50053",
        "localhost:50054",
        "localhost:50055"
    };

    for (const auto& address : peer_addresses) {
        if (address != self_address) {
            clients.push_back(std::make_shared<SwiftConsensusClient>(
                grpc::CreateChannel(address, grpc::InsecureChannelCredentials()), address));
        }
    }

    std::thread server_thread(RunServer, self_address,
                              std::ref(peerTable), std::ref(electionManager), std::ref(clients));

    std::thread failure_thread([&]() {
        while (true) {
            failureDetector.detectFailures();
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    });

    std::thread leader_thread([&]() {
        while (true) {
            electionManager.runElection();
            std::this_thread::sleep_for(std::chrono::seconds(15));
        }
    });

    while (true) {
        double cpu_free = getCPUUsage();
        double memory_free = getFreeMemoryPercent();
        int queue_size = current_queue_length.load();

        double score = 0.4 * cpu_free + 0.3 * memory_free - 0.2 * queue_size;
        peerTable.updateSelf(self_id, score);

        for (auto& client : clients) {
            client->SendHeartbeat(self_id, cpu_free, memory_free, queue_size, score);
        }

        std::this_thread::sleep_for(std::chrono::seconds(5));
    }

    server_thread.join();
    failure_thread.join();
    leader_thread.join();

    return 0;
}
