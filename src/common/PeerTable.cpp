#include "PeerTable.h"
#include <iostream>

void PeerTable::updatePeer(const std::string& server_id, double score) {
    std::lock_guard<std::mutex> lock(mutex_);
    peers_[server_id] = {score, std::chrono::system_clock::now(), true};
}

void PeerTable::updateSelf(const std::string& self_id, double score) {
    std::lock_guard<std::mutex> lock(mutex_);
    peers_[self_id] = {score, std::chrono::system_clock::now(), true};
}

void PeerTable::markPeerDead(const std::string& server_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (peers_.find(server_id) != peers_.end()) {
        peers_[server_id].is_alive = false;
    }
}

std::unordered_map<std::string, PeerInfo> PeerTable::getAllPeers() {
    std::lock_guard<std::mutex> lock(mutex_);
    return peers_;
}
