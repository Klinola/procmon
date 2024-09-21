#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sqlite3.h>
#include "io.h"
#include "proto/procmon.grpc.pb.h"


// Function to open database
int open_db() {
    int rc = sqlite3_open("procmon.db", &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        return rc;
    }
    return SQLITE_OK;
}

// Function to retrieve system info from the database
int get_system_info(procmon::StatusResponse* response) {
    sqlite3_stmt *stmt;
    const char *sql = "SELECT * FROM system_info ORDER BY timestamp DESC LIMIT 1";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return SQLITE_ERROR;
    }

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        response->set_cpu_usage(sqlite3_column_double(stmt, 1));
        response->set_mem_usage(sqlite3_column_double(stmt, 2));
        response->set_total_memory(sqlite3_column_double(stmt, 3));
        response->set_free_memory(sqlite3_column_double(stmt, 4));
        response->set_net_interface(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 9)));
        response->set_rx_bytes(sqlite3_column_int64(stmt, 10));
        response->set_tx_bytes(sqlite3_column_int64(stmt, 11));
        response->set_d_speed(sqlite3_column_double(stmt, 12));
        response->set_u_speed(sqlite3_column_double(stmt, 13));
    }

    sqlite3_finalize(stmt);
    return SQLITE_OK;
}

// Function to retrieve nft traffic info from the database
int get_nft_traffic(procmon::NftTrafficResponse* response) {
    sqlite3_stmt *stmt;
    const char *sql = "SELECT * FROM nft_traffic";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return SQLITE_ERROR;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        procmon::NftTraffic* traffic = response->add_nft_traffic();
        traffic->set_timestamp(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)));
        traffic->set_ip_address(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)));
        traffic->set_mac_address(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)));
        traffic->set_packets(sqlite3_column_int(stmt, 3));
        traffic->set_bytes(sqlite3_column_int(stmt, 4));
    }

    sqlite3_finalize(stmt);
    return SQLITE_OK;
}

// Function to delete the database
void delete_db() {
    if (remove("procmon.db") == 0) {
        printf("Database deleted successfully.\n");
    } else {
        perror("Failed to delete the database");
    }
}