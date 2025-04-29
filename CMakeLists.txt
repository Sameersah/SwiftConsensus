cmake_minimum_required(VERSION 3.10)
project(SwiftConsensus)

find_package(Protobuf CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)

include_directories(proto/)
include_directories(generated/)
include_directories(src/)
include_directories(src/server/)
include_directories(src/client/)
include_directories(src/common/)

add_executable(SwiftConsensusServer
        src/server/SwiftConsensusServer.cpp
        src/server/LeaderElectionManager.cpp
        src/server/FailureDetector.cpp
        src/server/TaskHandler.cpp
        src/client/SwiftConsensusClient.cpp
        src/common/PeerTable.cpp
        generated/swiftconsensus.pb.cc
        generated/swiftconsensus.grpc.pb.cc
)

target_link_libraries(SwiftConsensusServer
        gRPC::grpc++
        protobuf::libprotobuf
)
