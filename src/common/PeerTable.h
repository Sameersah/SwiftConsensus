#ifndef PEER_TABLE_H
#define PEER_TABLE_H

#include <unordered_map>
#include <string>
#include <mutex>
#include <chrono>

struct PeerInfo {
    double score;
    std::chrono::system_clock::time_point last_heartbeat;
    bool is_alive;
};

class PeerTable {
public:
    void updatePeer(const std::string& server_id, double score);
    void updateSelf(const std::string& self_id, double score);
    void markPeerDead(const std::string& server_id);
    std::unordered_map<std::string, PeerInfo> getAllPeers();

private:
    std::unordered_map<std::string, PeerInfo> peers_;
    std::mutex mutex_;
};

#endif // PEER_TABLE_H
