cmake_minimum_required(VERSION 3.10)

project(ProcMon VERSION 1.0)

set(CMAKE_C_STANDARD 23)
set(CMAKE_CXX_STANDARD 23)
add_executable(procmon procmon.c)

find_package(SQLite3 REQUIRED)
target_link_libraries(procmon SQLite::SQLite3)
