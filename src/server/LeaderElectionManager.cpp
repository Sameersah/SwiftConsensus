#include "LeaderElectionManager.h"
#include <mutex>
#include <iostream>

void LeaderElectionManager::runElection() {
    std::lock_guard<std::mutex> lock(mutex_);

    auto peers = peerTable_.getAllPeers();
    double best_score = -1.0;
    std::string best_server = selfId_;

    for (const auto& [server_id, info] : peers) {
        if (info.is_alive && info.score > best_score) {
            best_score = info.score;
            best_server = server_id;
        }
    }

    if (currentLeader_ != best_server) {
        currentLeader_ = best_server;
        std::cout << "[Leader Update] New Leader: " << currentLeader_ << std::endl;
    }
}

std::string LeaderElectionManager::getCurrentLeader() {
    std::lock_guard<std::mutex> lock(mutex_);
    return currentLeader_;
}
