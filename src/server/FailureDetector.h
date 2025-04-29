#ifndef FAILURE_DETECTOR_H
#define FAILURE_DETECTOR_H

#include <string>
#include "common/PeerTable.h"


class FailureDetector {
public:
    FailureDetector(PeerTable& peerTable)
        : peerTable_(peerTable) {}

    void detectFailures();

private:
    PeerTable& peerTable_;
};

#endif // FAILURE_DETECTOR_H
