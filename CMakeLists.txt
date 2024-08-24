cmake_minimum_required(VERSION 3.10)

# Project name
project(procmon CXX)

# Find the required packages
find_package(Protobuf REQUIRED)
find_package(gRPC REQUIRED)

# Include directories
include_directories(${Protobuf_INCLUDE_DIRS})
include_directories(${gRPC_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/grpc)

# Add the source files
set(SOURCES
    procmon.cpp
    io.cpp
    grpc/procmon.grpc.pb.cc
    grpc/procmon.pb.cc
)

# Define the executable
add_executable(procmon ${SOURCES})

# Link against the necessary libraries
target_link_libraries(procmon
    ${Protobuf_LIBRARIES}
    ${gRPC_LIBRARIES}
    pthread
    sqlite3
)

# C++ standard
set_property(TARGET procmon PROPERTY CXX_STANDARD 17)

# Installation rules
install(TARGETS procmon DESTINATION bin)