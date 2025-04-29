# SwiftConsensus 

**SwiftConsensus** is a high-performance, decentralized, near-real-time **Leader Election and Task Distribution** system built using **C++**, **gRPC**, and **Protobuf**.

Designed for reliability, modularity, and fairness, SwiftConsensus dynamically elects a leader among nodes based on live performance metrics, assigns tasks efficiently, and automatically recovers from node failures — all without any centralized controller.

---

##  Table of Contents
- [Architecture](#architecture)
- [Design Highlights](#design-highlights)
- [Folder Structure](#folder-structure)
- [Build Instructions](#build-instructions)
- [Run Instructions](#run-instructions)
- [Sample Output](#sample-output)
- [Future Extensions](#future-extensions)

---

##  Architecture

```
+-------------------------------------------------------------+
|                         SwiftConsensus Cluster              |
|                                                             |
|  +------------------+    +------------------+              |
|  |    Node 1         |    |    Node 2         |              |
|  | (SwiftConsensus)  |    | (SwiftConsensus)  |              |
|  +------------------+    +------------------+              |
|       ↑       ↓                   ↑       ↓                 |
|       Heartbeats + Scores          Heartbeats + Scores       |
|                                                             |
|  +------------------+    +------------------+    +------------------+  |
|  |    Node 3         |    |    Node 4         |    |    Node 5         |  |
|  | (SwiftConsensus)  |    | (SwiftConsensus)  |    | (SwiftConsensus)  |  |
|  +------------------+    +------------------+    +------------------+  |
|                                                             |
|   Leader Node:                                              |
|   - Generates Tasks                                         |
|   - Assigns to best available worker                       |
|                                                             |
|   Failures:                                                 |
|   - Auto-detected and recovered                             |
|   - New leader elected dynamically                         |
+-------------------------------------------------------------+
```

---

##  Design Highlights

- **gRPC + Protobuf** based communication.
- **Dynamic Leader Election** based on live scores (CPU%, Memory%, Task Queue).
- **Heartbeat-based Failure Detection** (5-second timeout).
- **Fair Work Distribution** to healthiest nodes.
- **Full Peer-to-Peer decentralized model** (no master server).
- **Production-grade modular C++ architecture** (server/client/common layers).

---

##  Folder Structure

```
SwiftConsensus/
├── CMakeLists.txt
├── README.md
├── proto/
│   └── swiftconsensus.proto
├── generated/
│   └── (gRPC compiled code)
├── src/
│   ├── server/
│   │   ├── SwiftConsensusServer.cpp
│   │   ├── LeaderElectionManager.h / .cpp
│   │   ├── FailureDetector.h / .cpp
│   │   └── TaskHandler.h / .cpp
│   ├── client/
│   │   ├── SwiftConsensusClient.h / .cpp
│   ├── common/
│   │   ├── PeerTable.h / .cpp
│   │   └── Utils.h
└── build/ (created after build)
```

---

##  Build Instructions

1. Install **gRPC** and **Protobuf** libraries on your system.

2. Generate gRPC and Protobuf classes:
   ```bash
   protoc -I=proto/ --cpp_out=generated/ --grpc_out=generated/ --plugin=protoc-gen-grpc=`which grpc_cpp_plugin` proto/swiftconsensus.proto
   ```

3. Build using CMake:
   ```bash
   mkdir build
   cd build
   cmake ..
   make
   ```

 Executable `SwiftConsensusServer` will be created inside `build/`.

---

## 🚀 Run Instructions

Run **multiple instances** to simulate multiple nodes:

```bash
cd build
./SwiftConsensusServer 50051
./SwiftConsensusServer 50052
./SwiftConsensusServer 50053
./SwiftConsensusServer 50054
./SwiftConsensusServer 50055
```

Each node will:
- Start a gRPC server to receive heartbeats and tasks.
- Send heartbeats every 2 seconds.
- Dynamically elect the leader.
- Assign tasks if the node becomes the leader.

---

##  Sample Output

```
[Server Started] Listening on localhost:50051
[Heartbeat Received] server_50052 | Score: 78.2 | CPU: 80 | Memory: 70 | Queue: 2
[Leader Update] New Leader: server_50051
[Leader Action] Generating Task
[Leader Assigns Task] task_1 to server_50052
[Task Received] TaskID: task_1 Assigned By: server_50051 | Task Data: data_for_task_1
[Task Completed] TaskID: task_1
[Failure Detected] server_50053 is DEAD
[Leader Update] New Leader: server_50054
```

---

##  Future Extensions

- ✅ Persistent storage for tasks and peer states.
- ✅ Dynamic joining and leaving of new servers (auto discovery).
- ✅ Prioritized task queues (urgent vs normal tasks).
- ✅ Load balancing improvements using dynamic weights (cpu/io/network latency).
- ✅ Visualization dashboard (real-time system view).

---

#  Contribute

Feel free to open pull requests or issues to improve SwiftConsensus further!

