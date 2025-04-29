#ifndef LEADER_ELECTION_MANAGER_H
#define LEADER_ELECTION_MANAGER_H

#include <string>
#include <unordered_map>
#include <mutex>
#include "common/PeerTable.h"


class LeaderElectionManager {
public:
    LeaderElectionManager(std::string selfId, PeerTable& peerTable)
        : selfId_(selfId), peerTable_(peerTable), currentLeader_(selfId) {}

    void runElection();
    std::string getCurrentLeader();

private:
    std::string selfId_;
    PeerTable& peerTable_;
    std::string currentLeader_;
    std::mutex mutex_;
};

#endif // LEADER_ELECTION_MANAGER_H
