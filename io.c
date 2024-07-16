#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sqlite3.h>
#include <json-c/json.h>
#include "io.h"

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 14444
#define BUFFER_SIZE 1024

sqlite3 *db; // SQLite3 database connection

// Function to open database
int open_db() {
    int rc = sqlite3_open("procmon.db", &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        return rc;
    }
    return SQLITE_OK;
}

// Function to convert database content to JSON
char* db_to_json() {
    sqlite3_stmt *stmt;
    const char *sql_system = "SELECT * FROM system_info";
    const char *sql_nft = "SELECT * FROM nft_traffic";
    struct json_object *jobj = json_object_new_object();
    struct json_object *jarray_system = json_object_new_array();
    struct json_object *jarray_nft = json_object_new_array();

    // Convert system_info table
    if (sqlite3_prepare_v2(db, sql_system, -1, &stmt, 0) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            struct json_object *jrow = json_object_new_object();
            json_object_object_add(jrow, "timestamp", json_object_new_string((char*)sqlite3_column_text(stmt, 0)));
            json_object_object_add(jrow, "cpu_usage", json_object_new_double(sqlite3_column_double(stmt, 1)));
            json_object_object_add(jrow, "mem_usage", json_object_new_double(sqlite3_column_double(stmt, 2)));
            json_object_object_add(jrow, "total_memory", json_object_new_double(sqlite3_column_double(stmt, 3)));
            json_object_object_add(jrow, "free_memory", json_object_new_double(sqlite3_column_double(stmt, 4)));
            json_object_object_add(jrow, "shared_memory", json_object_new_double(sqlite3_column_double(stmt, 5)));
            json_object_object_add(jrow, "buffer_memory", json_object_new_double(sqlite3_column_double(stmt, 6)));
            json_object_object_add(jrow, "totalswap_memory", json_object_new_double(sqlite3_column_double(stmt, 7)));
            json_object_object_add(jrow, "freeswap_memory", json_object_new_double(sqlite3_column_double(stmt, 8)));
            json_object_object_add(jrow, "net_interface", json_object_new_string((char*)sqlite3_column_text(stmt, 9)));
            json_object_object_add(jrow, "rx_bytes", json_object_new_int64(sqlite3_column_int64(stmt, 10)));
            json_object_object_add(jrow, "tx_bytes", json_object_new_int64(sqlite3_column_int64(stmt, 11)));
            json_object_object_add(jrow, "d_speed", json_object_new_double(sqlite3_column_double(stmt, 12)));
            json_object_object_add(jrow, "u_speed", json_object_new_double(sqlite3_column_double(stmt, 13)));
            json_object_array_add(jarray_system, jrow);
        }
        sqlite3_finalize(stmt);
    }

    // Convert nft_traffic table
    if (sqlite3_prepare_v2(db, sql_nft, -1, &stmt, 0) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            struct json_object *jrow = json_object_new_object();
            json_object_object_add(jrow, "timestamp", json_object_new_string((char*)sqlite3_column_text(stmt, 0)));
            json_object_object_add(jrow, "ip_address", json_object_new_string((char*)sqlite3_column_text(stmt, 1)));
            json_object_object_add(jrow, "mac_address", json_object_new_string((char*)sqlite3_column_text(stmt, 2)));
            json_object_object_add(jrow, "packets", json_object_new_int(sqlite3_column_int(stmt, 3)));
            json_object_object_add(jrow, "bytes", json_object_new_int(sqlite3_column_int(stmt, 4)));
            json_object_array_add(jarray_nft, jrow);
        }
        sqlite3_finalize(stmt);
    }

    json_object_object_add(jobj, "system_info", jarray_system);
    json_object_object_add(jobj, "nft_traffic", jarray_nft);

    return strdup(json_object_to_json_string(jobj));
}

// Function to send JSON data via TCP
int send_json(const char *json_str) {
    int sockfd;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    ssize_t n;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        return -1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        close(sockfd);
        return -1;
    }

    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        close(sockfd);
        return -1;
    }

    write(sockfd, json_str, strlen(json_str));

    // Receive confirmation
    n = read(sockfd, buffer, sizeof(buffer) - 1);
    if (n > 0) {
        buffer[n] = '\0';
        printf("Received confirmation: %s\n", buffer);
    }

    close(sockfd);
    return 0;
}

// Function to delete the database
void delete_db() {
    if (remove("procmon.db") == 0) {
        printf("Database deleted successfully.\n");
    } else {
        perror("Failed to delete the database");
    }
}

void send_db_to_server() {
    if (open_db() != SQLITE_OK) {
        return;
    }

    char *json_str = db_to_json();
    if (json_str != NULL) {
        if (send_json(json_str) == 0) {
            delete_db();
        }
        free(json_str);
    }

    sqlite3_close(db);
}
