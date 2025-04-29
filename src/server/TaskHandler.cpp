#include "TaskHandler.h"
#include "client/SwiftConsensusClient.h"
#include <iostream>

void TaskHandler::generateAndAssignTask() {
    std::string new_task_id = "task_" + std::to_string(taskCounter_++);
    std::string task_data = "data_for_" + new_task_id;

    auto peers = peerTable_.getAllPeers();
    std::string best_worker = "";
    double best_score = -1.0;

    for (const auto& [server_id, info] : peers) {
        if (info.is_alive && server_id != selfId_ && info.score > best_score) {
            best_worker = server_id;
            best_score = info.score;
        }
    }

    if (!best_worker.empty()) {
        std::cout << "[Leader Assigns Task] " << new_task_id << " to " << best_worker << std::endl;

        for (auto& client : clients_) {
            if (client->GetServerAddress() == "localhost:" + best_worker.substr(7)) {
                client->AssignTask(new_task_id, selfId_, task_data);
                break;
            }
        }
    } else {
        std::cout << "[No Alive Workers] Task not assigned" << std::endl;
    }
}
