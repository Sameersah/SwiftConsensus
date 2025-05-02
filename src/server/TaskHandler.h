#ifndef TASK_HANDLER_H
#define TASK_HANDLER_H

#include <memory>
#include <vector>
#include <string>
#include "common/PeerTable.h"

// Forward declaration
class SwiftConsensusClient;

class TaskHandler {
public:
    TaskHandler(std::string selfId,
                PeerTable& peerTable,
                std::vector<std::shared_ptr<SwiftConsensusClient>>& clients)
        : selfId_(std::move(selfId)), peerTable_(peerTable), clients_(clients) {}

    // You can keep this for testing simulated tasks or remove it if not needed
    void simulateAndAssignDummyTask();

private:
    std::string selfId_;
    PeerTable& peerTable_;
    std::vector<std::shared_ptr<SwiftConsensusClient>>& clients_;
};

#endif // TASK_HANDLER_H
