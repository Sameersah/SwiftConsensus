#ifndef TASK_HANDLER_H
#define TASK_HANDLER_H

#include <memory>
#include <vector>
#include <string>
#include "common/PeerTable.h"

// Forward declaration (only needed for pointer types)
class SwiftConsensusClient;

class TaskHandler {
public:
    TaskHandler(std::string selfId, PeerTable& peerTable, std::vector<std::shared_ptr<SwiftConsensusClient>>& clients)
        : selfId_(selfId), peerTable_(peerTable), clients_(clients), taskCounter_(0) {}

    void generateAndAssignTask();

private:
    std::string selfId_;
    PeerTable& peerTable_;
    std::vector<std::shared_ptr<SwiftConsensusClient>>& clients_;
    int taskCounter_;
};

#endif // TASK_HANDLER_H
