cmake_minimum_required(VERSION 3.10)

# Project name
project(procmon C)

# Find the required packages
find_package(Protobuf REQUIRED)
find_package(gRPC REQUIRED)

# Include directories
include_directories(${Protobuf_INCLUDE_DIRS})
include_directories(${gRPC_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/grpc)

# Add the source files
set(SOURCES
    procmon.c
    io.c
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

# Installation rules
install(TARGETS procmon DESTINATION bin)