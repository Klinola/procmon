# Procmon

This program monitors system resources such as CPU usage, memory usage, and network traffic on Linux (preferably OpenWrt). It collects these metrics periodically and stores them in an SQLite3 database for analysis and monitoring.

## Requirements

- OpenWrt device with necessary development tools installed.
- SQLite3 library (`libsqlite3-dev`).
- C compiler (e.g., GCC).

## Run the Program:
This will start monitoring system resources in two threads:
- One thread monitors network interfaces for traffic statistics.
- Another thread monitors CPU usage, memory status, and writes data to SQLite3 database.

