#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/sysinfo.h>
// Multi-threading library
#include <pthread.h>
// Network monitoring
#include <arpa/inet.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sqlite3.h>
#include <grpcpp/grpcpp.h>

#include "procmon.h"
#include "io.h"
#include "grpc/procmon.grpc.pb.h"

sqlite3 *db; // SQLite3 database connection
FILE *net_dev_file = NULL; // /proc/net/dev file
NET_INTERFACE *p_interface = NULL; // Local network interface information structure
HOST_STATE host_core; // Host state information structure

void init_db() {
    // 初始化数据库部分
    char *err_msg = 0;
    int rc = sqlite3_open("procmon.db", &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        exit(1);
    }

    const char *sql = "CREATE TABLE IF NOT EXISTS system_info("
                      "timestamp TEXT, "
                      "cpu_usage REAL, "
                      "mem_usage REAL, "
                      "total_memory REAL, "
                      "free_memory REAL, "
                      "shared_memory REAL, "
                      "buffer_memory REAL, "
                      "totalswap_memory REAL, "
                      "freeswap_memory REAL, "
                      "net_interface TEXT, "
                      "rx_bytes INTEGER, "
                      "tx_bytes INTEGER, "
                      "d_speed REAL, "
                      "u_speed REAL);";
    rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        sqlite3_close(db);
        exit(1);
    }

    const char *sql2 = "CREATE TABLE IF NOT EXISTS nft_traffic("
                       "timestamp TEXT, "
                       "ip_address TEXT, "
                       "mac_address TEXT, "
                       "packets INTEGER, "
                       "bytes INTEGER);";
    rc = sqlite3_exec(db, sql2, 0, 0, &err_msg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        sqlite3_close(db);
        exit(1);
    }
}

void insert_db(double cpu_usage, double mem_usage, NET_INTERFACE *net) {
    // 插入数据库的部分
    char *err_msg = 0;
    char sql[512];

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

    while (net != NULL) {
        snprintf(sql, sizeof(sql),
                 "INSERT INTO system_info (timestamp, cpu_usage, mem_usage, total_memory, free_memory, shared_memory, buffer_memory, totalswap_memory, freeswap_memory, net_interface, rx_bytes, tx_bytes, d_speed, u_speed) VALUES "
                 "('%04d-%02d-%02d %02d:%02d:%02d', %.2f, %.2f, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, '%s', %ld, %ld, %.2f, %.2f);",
                 tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec,
                 cpu_usage, mem_usage,
                 info.totalram * 1.0 / 1024 / 1024 / 1024,
                 info.freeram * 1.0 / 1024 / 1024 / 1024,
                 info.sharedram * 1.0 / 1024 / 1024 / 1024,
                 info.bufferram * 1.0 / 1024 / 1024 / 1024,
                 info.totalswap * 1.0 / 1024 / 1024 / 1024,
                 info.freeswap * 1.0 / 1024 / 1024 / 1024,
                 net->name, net->rtx1_cnt.rx_bytes, net->rtx1_cnt.tx_bytes,
                 net->d_speed, net->u_speed);

        int rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
        if (rc != SQLITE_OK) {
            fprintf(stderr, "SQL error: %s\n", err_msg);
            sqlite3_free(err_msg);
        }

        net = net->next;
    }
}

void insert_nft_traffic(const char* ip, const char* mac, int packets, int bytes) {
    // 插入nft流量的部分
    char *err_msg = 0;
    char sql[512];

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

    snprintf(sql, sizeof(sql),
             "INSERT INTO nft_traffic (timestamp, ip_address, mac_address, packets, bytes) VALUES "
             "('%04d-%02d-%02d %02d:%02d:%02d', '%s', '%s', %d, %d);",
             tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec,
             ip, mac, packets, bytes);

    int rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
    }
}

void *thread_net(void *arg) {
    printf("%s\n", __FUNCTION__);

    while (1) {
        printf("\n***********************************************************\n");
        get_network_speed(p_interface);
        show_netinterfaces(p_interface, 1);
        insert_db(host_core.cpu_used, host_core.mem_used, p_interface);
        sleep(10);
    }
}

void *thread_core(void *arg) {
    printf("%s\n", __FUNCTION__);
    while (1) {
        open_sysinfo();

        get_host_runtime(&host_core.hour, &host_core.minute);
        printf("\n[run]:%6d hours\t%2d minutes\t", host_core.hour, host_core.minute);

        get_mem_usage(&host_core.mem_used);
        printf("[mem_used]:%6.2f%%\t", host_core.mem_used);

        get_cpu_usage(&(host_core.cpu_used));
        printf("[cpu_used]:%6.2f%%\n", host_core.cpu_used);
        sleep(10);
    }
}

void *thread_nft(void *arg) {
    printf("%s\n", __FUNCTION__);
    while (1) {
        char command[256];
        FILE *fp;
        char buffer[4096];

        // Execute nft -j list set ip xray mac_set and capture output
        sprintf(command, "nft -j list set ip xray mac_set");
        fp = popen(command, "r");
        if (fp == NULL) {
            perror("Failed to execute command");
            exit(EXIT_FAILURE);
        }

        // Read JSON data from command output
        if (fgets(buffer, sizeof(buffer), fp) != NULL) {
            // Parse JSON using json-c library
            struct json_object *jobj = json_tokener_parse(buffer);
            if (jobj != NULL) {
                // Process JSON object to extract IP traffic info
                struct json_object *nftables_array = NULL;
                struct json_object *set_obj = NULL;
                struct json_object *elem_array = NULL;

                if (json_object_object_get_ex(jobj, "nftables", &nftables_array) &&
                    json_object_is_type(nftables_array, json_type_array)) {
                    for (size_t i = 0; i < json_object_array_length(nftables_array); ++i) {
                        struct json_object *nft_item = json_object_array_get_idx(nftables_array, i);
                        if (json_object_object_get_ex(nft_item, "set", &set_obj)) {
                            if (json_object_object_get_ex(set_obj, "elem", &elem_array) &&
                                json_object_is_type(elem_array, json_type_array)) {
                                for (size_t j = 0; j < json_object_array_length(elem_array); ++j) {
                                    struct json_object *elem_item = json_object_array_get_idx(elem_array, j);
                                    struct json_object *val_obj = NULL;
                                    struct json_object *counter_obj = NULL;
                                    if (json_object_object_get_ex(elem_item, "elem", &val_obj) &&
                                        json_object_object_get_ex(val_obj, "val", &val_obj) &&
                                        json_object_object_get_ex(val_obj, "concat", &val_obj) &&
                                        json_object_object_get_ex(elem_item, "counter", &counter_obj)) {

                                        const char *ip = json_object_get_string(json_object_array_get_idx(val_obj, 0));
                                        const char *mac = json_object_get_string(json_object_array_get_idx(val_obj, 1));
                                        int packets = json_object_get_int(json_object_object_get(counter_obj, "packets"));
                                        int bytes = json_object_get_int(json_object_object_get(counter_obj, "bytes"));

                                        insert_nft_traffic(ip, mac, packets, bytes);
                                    }
                                }
                            }
                                                    }
                    }
                }
                json_object_put(jobj); // Free JSON object
            }
        }
        pclose(fp);

        // Flush set ip xray mac_set
        sprintf(command, "nft flush set ip xray mac_set");
        system(command);

        // Wait for one minute before repeating
        sleep(60);
    }
}

// 实现 gRPC 服务类
class ProcmonServiceImpl final : public procmon::ProcmonService::Service {
public:
    Status GetSystemStatus(ServerContext* context, const procmon::StatusRequest* request,
                           procmon::StatusResponse* reply) override {
        // 从数据库中获取系统信息并填充 gRPC 响应
        if (open_db() == SQLITE_OK) {
            get_system_info(reply);
            sqlite3_close(db);
        } else {
            return Status::CANCELLED;
        }

        return Status::OK;
    }

    Status GetNftTraffic(ServerContext* context, const procmon::NftTrafficRequest* request,
                         procmon::NftTrafficResponse* reply) override {
        // 从数据库中获取 nft 流量信息并填充 gRPC 响应
        if (open_db() == SQLITE_OK) {
            get_nft_traffic(reply);
            sqlite3_close(db);
        } else {
            return Status::CANCELLED;
        }

        return Status::OK;
    }
};

void RunServer() {
    std::string server_address("0.0.0.0:50051");
    ProcmonServiceImpl service;

    ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
    std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cout << "Server listening on " << server_address << std::endl;
    server->Wait();
}

int main() {
    // 初始化数据库和线程
    int nums = 0;
    init_db();
    p_interface = (NET_INTERFACE *)malloc(sizeof(NET_INTERFACE));
    get_interface_info(&p_interface, &nums);
    printf("net_interface nums: %d\n", nums);

    show_netinterfaces(p_interface, 0);

    pthread_t thread_net_id, thread_core_id, thread_nft_id;
    pthread_create(&thread_net_id, NULL, (void *)thread_net, NULL);
    pthread_create(&thread_core_id, NULL, (void *)thread_core, NULL);
    pthread_create(&thread_nft_id, NULL, (void *)thread_nft, NULL);

    // 启动 gRPC 服务线程
    std::thread grpc_thread(RunServer);
    
    // 等待线程结束
    pthread_join(thread_net_id, NULL);
    pthread_join(thread_core_id, NULL);
    pthread_join(thread_nft_id, NULL);
    grpc_thread.join();

    sqlite3_close(db);
    return 0;
}