#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <chrono>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <cstdlib>

#include <grpcpp/grpcpp.h>
#include "swiftconsensus.grpc.pb.h"

#include "common/PeerTable.h"
#include "client/SwiftConsensusClient.h"  // <-- full include here
#include "server/LeaderElectionManager.h"
#include "server/FailureDetector.h"
#include "server/TaskHandler.h"


using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using grpc::Channel;
using grpc::ClientContext;
using swiftconsensus::HeartbeatRequest;
using swiftconsensus::HeartbeatResponse;
using swiftconsensus::SwiftConsensusService;
using swiftconsensus::TaskAssignmentRequest;
using swiftconsensus::TaskAssignmentResponse;

// --- Global Variables ---
std::string self_id;
std::string self_address;

// --- gRPC Server Side Implementation ---
class SwiftConsensusServiceImpl final : public swiftconsensus::SwiftConsensusService::Service {
public:
    SwiftConsensusServiceImpl(PeerTable& peerTable)
        : peerTable_(peerTable) {}

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

        // Simulate task processing
        std::this_thread::sleep_for(std::chrono::seconds(2));
        std::cout << "[Task Completed] TaskID: " << request->task_id() << std::endl;

        reply->set_message("Task processed successfully");
        return Status::OK;
    }

private:
    PeerTable& peerTable_;
};

// --- Function to Run gRPC Server ---
void RunServer(std::string server_address, PeerTable& peerTable) {
    SwiftConsensusServiceImpl service(peerTable);

    ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cout << "[Server Started] Listening on " << server_address << std::endl;

    server->Wait();
}

// --- Main ---
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

    // Hardcoded peer addresses for demo
    std::vector<std::string> peer_addresses = {
        "localhost:50051",
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

    TaskHandler taskHandler(self_id, peerTable, clients);

    // Start gRPC Server in a separate thread
    std::thread server_thread(RunServer, self_address, std::ref(peerTable));

    // Start Failure Detection and Leader Election loops
    std::thread failure_thread([&]() {
        while (true) {
            failureDetector.detectFailures();
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    });

    std::thread leader_thread([&]() {
        while (true) {
            electionManager.runElection();
            std::this_thread::sleep_for(std::chrono::seconds(5));
        }
    });

    // Main loop: Heartbeats + Task Generation
    while (true) {
        double cpu_free = rand() % 100;      // Simulated metrics
        double memory_free = rand() % 100;
        int queue_size = rand() % 10;
        double score = 0.4 * cpu_free + 0.3 * memory_free - 0.2 * queue_size;

        peerTable.updateSelf(self_id, score);

        for (auto& client : clients) {
            client->SendHeartbeat(self_id, cpu_free, memory_free, queue_size, score);
        }

        if (electionManager.getCurrentLeader() == self_id) {
            taskHandler.generateAndAssignTask();
        }

        std::this_thread::sleep_for(std::chrono::seconds(2));
    }

    server_thread.join();
    failure_thread.join();
    leader_thread.join();

    return 0;
}
