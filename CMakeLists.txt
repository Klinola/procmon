cmake_minimum_required(VERSION 3.10)

# Project name
project(procmon CXX)

# Find the required packages using pkg-config
find_package(PkgConfig REQUIRED)
pkg_check_modules(GRPC REQUIRED grpc++)
pkg_check_modules(JSONC REQUIRED json-c)
pkg_check_modules(Protobuf REQUIRED protobuf)

# 设置 Abseil 安装路径
set(ABSL_INSTALL_DIR /usr/local)
link_directories("$ENV{HOME}/.local/lib")

# Include directories
include_directories(${GRPC_INCLUDE_DIRS})
include_directories(${JSONC_INCLUDE_DIRS})
include_directories(${Protobuf_INCLUDE_DIRS})
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
    ${GRPC_LIBRARIES} 
    ${Protobuf_LIBRARIES}
    ${JSONC_LIBRARIES}
    pthread
    sqlite3
    ${ABSL_INSTALL_DIR}/lib/libabsl_base.a
    ${ABSL_INSTALL_DIR}/lib/libabsl_strings.a
    ${ABSL_INSTALL_DIR}/lib/libabsl_log_internal_check_op.a
    ${ABSL_INSTALL_DIR}/lib/libabsl_log_internal_conditions.a
    ${ABSL_INSTALL_DIR}/lib/libabsl_log_internal_fnmatch.a
    ${ABSL_INSTALL_DIR}/lib/libabsl_log_internal_format.a
    ${ABSL_INSTALL_DIR}/lib/libabsl_log_internal_globals.a
    ${ABSL_INSTALL_DIR}/lib/libabsl_log_internal_log_sink_set.a
    ${ABSL_INSTALL_DIR}/lib/libabsl_log_internal_message.a
    ${ABSL_INSTALL_DIR}/lib/libabsl_log_internal_nullguard.a
    ${ABSL_INSTALL_DIR}/lib/libabsl_log_internal_proto.a
    ${ABSL_INSTALL_DIR}/lib/libabsl_log_severity.a
    ${ABSL_INSTALL_DIR}/lib/libabsl_log_sink.a
    ${ABSL_INSTALL_DIR}/lib/libabsl_raw_logging_internal.a
    ${ABSL_INSTALL_DIR}/lib/libabsl_spinlock_wait.a
)

# C++ standard
set_property(TARGET procmon PROPERTY CXX_STANDARD 17)
