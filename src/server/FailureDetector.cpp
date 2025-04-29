#include "FailureDetector.h"
#include <chrono>
#include <mutex>
#include <iostream>

void FailureDetector::detectFailures() {
    auto now = std::chrono::system_clock::now();
    auto peers = peerTable_.getAllPeers();

    for (const auto& [server_id, info] : peers) {
        auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - info.last_heartbeat).count();
        if (duration > 5 && info.is_alive) {
            peerTable_.markPeerDead(server_id);
            std::cout << "[Failure Detected] " << server_id << " marked as DEAD" << std::endl;
        }
    }
}
