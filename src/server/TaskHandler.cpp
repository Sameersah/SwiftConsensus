#include "TaskHandler.h"
#include "client/SwiftConsensusClient.h"
#include <iostream>
#include <limits>
#include <cstdlib>

void TaskHandler::simulateAndAssignDummyTask() {
    std::string task_id = "simulated_task_" + std::to_string(rand() % 10000);
    std::string task_data = "simulated_payload";
    double penalty = 10.0;

    auto peers = peerTable_.getAllPeers();
    std::string best_worker = "";
    double best_score = std::numeric_limits<double>::lowest();

    for (const auto& [server_id, info] : peers) {
        if (!info.is_alive) continue;

        double effective_score = info.score;
        if (server_id == selfId_) {
            effective_score -= penalty;
        }

        if (effective_score > best_score) {
            best_score = effective_score;
            best_worker = server_id;
        }
    }

    if (!best_worker.empty()) {
        std::cout << "[Leader Simulates Task Assignment] " << task_id << " → " << best_worker
                  << " (effective score: " << best_score << ")" << std::endl;

        for (auto& client : clients_) {
            if (client->GetServerAddress() == "localhost:" + best_worker.substr(7)) {
                client->AssignTask(task_id, selfId_, task_data);
                break;
            }
        }
    } else {
        std::cout << "[Simulation] No alive worker to assign task." << std::endl;
    }
}
