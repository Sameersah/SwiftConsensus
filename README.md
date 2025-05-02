# SwiftConsensus

SwiftConsensus is a decentralized, score-based consensus leadership algorithm built using C++ and gRPC. It enables dynamic leader election, fair task assignment, failure recovery, and resource-aware load balancing — ideal for distributed scheduling systems.

---

##  Features

- ✅ Dynamic leader election (score-based)
- ✅ Peer-to-peer decentralized architecture
- ✅ Fair task assignment using CPU, memory, and queue length
- ✅ Task delegation based on effective scores (with self-penalty for leader)
- ✅ Failure detection & automatic re-election
- ✅ Support for external client-submitted tasks
- ✅ Real system metrics (CPU & memory) using system calls
- ✅ Soft-state peer table with heartbeat recovery
- ✅ Clean, modular architecture

---

##  Project Structure

```
SwiftConsensus/
├── CMakeLists.txt
├── proto/                      # Protobuf definitions
├── generated/                  # gRPC generated .pb.cc/.h files
├── src/
│   ├── server/                 # Server logic (leader, handler, failure)
│   ├── client/                 # SwiftConsensusClient & TaskSubmitterClient
│   └── common/                 # Peer table & utilities
├── run_all.sh                 # Script to launch multiple nodes
└── README.md
```

---

## ⚙ Build Instructions

### Prerequisites

- gRPC and Protocol Buffers
- CMake >= 3.10
- C++17 compiler

### Build

```bash
mkdir build
cd build
cmake ..
make
```

---

## ▶ Run the Cluster

Start 5 server nodes on ports 50051–50055:

```bash
./run_all.sh
```

This runs:
- gRPC servers
- Heartbeat exchanges every 5s
- Leader election every 15s
- Task assignment by the leader every cycle

---

##  Submit External Task (Client → Leader)

```bash
./SubmitTaskClient <task_id> <task_data>
```

The client:
1. Calls `GetLeader()` on any known node
2. Submits task to leader
3. Leader routes to the best scoring server

---

##  Testing Scenarios

###  Leader Election
- Leader chosen based on CPU/memory/queue score
- Logged as:
```
[Leader Election] ✅ New leader elected: server_50052 with score: 82.1
```

###  Task Assignment
- Leader assigns tasks via gRPC to optimal peer
- Logs:
```
[Leader Routing Task] task_5 → server_50054 (effective score: 81.3)
```

###  Failure Recovery
- Stop a leader process (`Ctrl+C`)
- Auto re-election kicks in within 5–7 seconds

###  Recovery
- Restart dead node
- Heartbeat accepted again, node re-enters election pool

---

##  Metrics Used in Scoring

```
score = 0.4 * CPU_free + 0.3 * Memory_free - 0.2 * QueueLength
```

Leader penalizes itself (`-10`) to avoid bias in delegation.

---

##  Limitations & Future Work

- Real task execution is simulated (2s delay)
- Currently static peer list — add dynamic peer discovery
- Could extend to Kubernetes pods or Docker Swarm tasks




