
# Procmon

This program monitors system resources such as CPU usage, memory usage, and network traffic on Linux (preferably OpenWrt). It collects these metrics periodically and stores them in an SQLite3 database for analysis and monitoring. The program also exposes a gRPC service that allows clients to request system status and network traffic data.

## Requirements

- OpenWrt device with necessary development tools installed.
- SQLite3 library (`libsqlite3-dev`).
- gRPC and Protocol Buffers libraries.
- C++ compiler (e.g., GCC or Clang).

## Features

- **System Monitoring**: Tracks CPU usage, memory usage, and runtime statistics.
- **Network Monitoring**: Monitors network interfaces for traffic statistics, including download and upload speeds.
- **nftables Integration**: Captures network traffic information using nftables and stores it in the SQLite3 database.
- **gRPC Service**: Provides a gRPC service for external clients to query system status and network traffic information.

## How to Build

1. **Install Dependencies**:
   Ensure that all necessary libraries are installed, including SQLite3, gRPC, and Protocol Buffers.

   ```bash
   sudo apt-get install libsqlite3-dev
   sudo apt-get install grpc
   sudo apt-get install protobuf-compiler
   ```

2. **Clone the Repository**:
   Clone the project repository and navigate to the `procmon` directory.

   ```bash
   git clone https://github.com/Klinola/procmon.git
   cd procmon
   ```

3. **Compile the Program**:
   Use `cmake` to generate the Makefile and then compile the project.

   ```bash
   mkdir build
   cd build
   cmake ..
   make
   ```

4. **Run the Program**:
   After compiling, you can start the program, which will begin monitoring system resources and expose the gRPC service.

   ```bash
   ./procmon
   ```

   This will start monitoring system resources in three threads:
   - One thread monitors network interfaces for traffic statistics.
   - Another thread monitors CPU usage, memory status, and writes data to the SQLite3 database.
   - A third thread captures network traffic using nftables.
   - The gRPC service runs on port `50051` and provides system status and network traffic data to external clients.

## gRPC Service

The gRPC service allows clients to retrieve:
- **System Status**: CPU usage, memory usage, total memory, free memory, network interface statistics.
- **Network Traffic**: Detailed traffic data captured via nftables.

To interact with the gRPC service, clients need to compile the `proto` files located in the `grpc/` directory.

## Future Improvements

- Add more detailed monitoring and alerting features.
- Expand gRPC service to support more queries and configurations.
- Optimize performance for resource-constrained devices like OpenWrt.
