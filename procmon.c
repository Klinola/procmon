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

#include "procmon.h"

sqlite3 *db; // SQLite3 database connection
FILE *net_dev_file = NULL; // /proc/net/dev file
NET_INTERFACE *p_interface = NULL; // Local network interface information structure
HOST_STATE host_core; // Host state information structure

void init_db() {
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
}

void insert_db(double cpu_usage, double mem_usage, NET_INTERFACE *net) {
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

void get_cpuoccupy(CPU_OCCUPY *cpu) {
    char buff[BUFFSIZE];
    FILE *fd;

    fd = fopen("/proc/stat", "r");
    if (fd == NULL) {
        perror("open /proc/stat failed\n");
        exit(0);
    }
    fgets(buff, sizeof(buff), fd);

    sscanf(buff, "%s %u %u %u %u", cpu->name, &cpu->user, &cpu->nice, &cpu->system, &cpu->idle);
    fclose(fd);
}

double cal_occupy(CPU_OCCUPY *c1, CPU_OCCUPY *c2) {
    double t1, t2;
    double id, sd;
    double cpu_used;

    t1 = (double)(c1->user + c1->nice + c1->system + c1->idle);
    t2 = (double)(c2->user + c2->nice + c2->system + c2->idle);

    id = (double)(c2->user - c1->user);
    sd = (double)(c2->system - c1->system);

    cpu_used = (100 * (id + sd) / (t2 - t1));
    return cpu_used;
}

void open_sysinfo() {
    int ret = 0;
    ret = sysinfo(&info);
    if (ret != 0) {
        perror("get sys_info failed\n");
        exit(0);
    }
}

void get_mem_usage(double *mem_used) {
    FILE *fd;

    fd = fopen("/proc/meminfo", "r");
    if (fd == NULL) {
        perror("open /proc/meminfo failed\n");
        exit(0);
    }

    size_t bytes_read;
    size_t read;
    char *line = NULL;
    int index = 0;
    int avimem = 0;

    while ((read = getline(&line, &bytes_read, fd)) != -1) {
        if (++index <= 2) {
            continue;
        }
        if (strstr(line, "MemAvailable") != NULL) {
            sscanf(line, "%*s%d%*s", &avimem);
            break;
        }
    }
    free(line);
    int t = info.totalram / 1024.0;
    *mem_used = (t - avimem) * 100 / t;

#if DEBUG
    printf("\n");
    printf("total memory: %.3f\n", info.totalram * 1.0 / 1024 / 1024 / 1024);
    printf("free memory: %.3f\n", info.freeram * 1.0 / 1024 / 1024 / 1024);
    printf("shared memory: %.3f\n", info.sharedram * 1.0 / 1024 / 1024 / 1024);
    printf("buffer memory: %.3f\n", info.bufferram * 1.0 / 1024 / 1024 / 1024);
    printf("totalswap memory: %.3f\n", info.totalswap * 1.0 / 1024 / 1024 / 1024);
    printf("freeswap memory: %.3f\n", info.freeswap * 1.0 / 1024 / 1024 / 1024);
#endif

    fclose(fd);
}

void get_cpu_usage(double *cpu_used) {
    CPU_OCCUPY cpu_0, cpu_1;

    get_cpuoccupy(&cpu_0);
    sleep(1);
    get_cpuoccupy(&cpu_1);

    *cpu_used = cal_occupy(&cpu_0, &cpu_1);

#if DEBUG
    printf("cpu: %.2f%%\n", *cpu_used);
#endif
}

void get_host_runtime(int *hours, int *minutes) {
    *hours = info.uptime / 3600;
    *minutes = (info.uptime % 3600) / 60;
}

void open_netconf(FILE **fd) {
    *fd = fopen("/proc/net/dev", "r");
    if (*fd == NULL) {
        perror("open file /proc/net/dev failed!\n");
        exit(0);
    }
}

void show_netinterfaces(NET_INTERFACE *p_net, int n) {
    NET_INTERFACE *temp;
    temp = p_net;

    while (temp != NULL) {
        if (!n) {
            printf("Interface: %-16s\t IP:%16s\t MAC:%12s\n", temp->name, temp->ip, temp->mac);
        } else {
            printf("Interface: %-16s\t", temp->name);
            if (temp->speed_level & 0x1) {
                printf("download:%10.2lfMB/s\t\t", temp->d_speed);
            } else {
                printf("download:%10.2lfKB/s\t\t", temp->d_speed);
            }

            if ((temp->speed_level >> 1) & 0x1) {
                printf("upload:%10.2lfMB/s\n", temp->u_speed);
            } else {
                printf("upload:%10.2lfKB/s\n", temp->u_speed);
            }
        }

        temp = temp->next;
    }
}

int get_interface_info(NET_INTERFACE **net, int *n) {
    int fd;
    int num = 0;
    struct ifreq buf[16];
    struct ifconf ifc;

    NET_INTERFACE *p_temp = NULL;
    (*net)->next = NULL;

    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        close(fd);
        printf("socket open failed\n");
    }

    ifc.ifc_len = sizeof(buf);
    ifc.ifc_buf = (caddr_t)buf;

    if (!ioctl(fd, SIOCGIFCONF, (char *)&ifc)) {
        num = ifc.ifc_len / sizeof(struct ifreq);
        *n = num;

        while (num-- > 0) {
            strcpy((*net)->name, buf[num].ifr_name);

            if (!(ioctl(fd, SIOCGIFADDR, (char *)&buf[num]))) {
                memset((*net)->ip, 0, 16);
                strcpy((*net)->ip, inet_ntoa(((struct sockaddr_in *)(&buf[num].ifr_addr))->sin_addr));
            }

            if (!ioctl(fd, SIOCGIFHWADDR, (char *)(&buf[num]))) {
                memset((*net)->mac, 0, 13);
                snprintf((*net)->mac, 13, "%02x%02x%02x%02x%02x%02x",
                         (unsigned char)buf[num].ifr_hwaddr.sa_data[0],
                         (unsigned char)buf[num].ifr_hwaddr.sa_data[1],
                         (unsigned char)buf[num].ifr_hwaddr.sa_data[2],
                         (unsigned char)buf[num].ifr_hwaddr.sa_data[3],
                         (unsigned char)buf[num].ifr_hwaddr.sa_data[4],
                         (unsigned char)buf[num].ifr_hwaddr.sa_data[5]);
            }
            if (num >= 1) {
                p_temp = (NET_INTERFACE *)malloc(sizeof(NET_INTERFACE));
                memset(p_temp, 0, sizeof(NET_INTERFACE));
                p_temp->next = *net;
                *net = p_temp;
            }
        }
        return 0;
    } else {
        return -1;
    }
}

void get_rtx_bytes(char *name, RTX_BYTES *rtx) {
    char *line = NULL;
    size_t bytes_read;
    size_t read;
    char str1[32];
    char str2[32];
    int i = 0;
    open_netconf(&net_dev_file);
    gettimeofday(&rtx->rtx_time, NULL);

    while ((read = getline(&line, &bytes_read, net_dev_file)) != -1) {
        if ((++i) <= 2)
            continue;
        if (strstr(line, name) != NULL) {
            memset(str1, 0x0, 32);
            memset(str2, 0x0, 32);
            sscanf(line, "%*s%s%*s%*s%*s%*s%*s%*s%*s%s", str1, str2);
            rtx->tx_bytes = atol(str2);
            rtx->rx_bytes = atol(str1);
        }
    }

    free(line);
    fclose(net_dev_file);
}

void rtx_bytes_copy(RTX_BYTES *dest, RTX_BYTES *src) {
    dest->rx_bytes = src->rx_bytes;
    dest->tx_bytes = src->tx_bytes;
    dest->rtx_time = src->rtx_time;
    src->rx_bytes = 0;
    src->tx_bytes = 0;
}

void get_network_speed(NET_INTERFACE *p_net) {
    RTX_BYTES rtx0, rtx1;
    NET_INTERFACE *temp1, *temp2, *temp3;
    temp1 = p_net;
    temp2 = p_net;
    temp3 = p_net;
    while (temp1 != NULL) {
        get_rtx_bytes(temp1->name, &rtx0);
        temp1->rtx0_cnt.tx_bytes = rtx0.tx_bytes;
        temp1->rtx0_cnt.rx_bytes = rtx0.rx_bytes;
        temp1->rtx0_cnt.rtx_time = rtx0.rtx_time;
        temp1 = temp1->next;
    }

    sleep(WAIT_SECOND);

    while (temp2 != NULL) {
        get_rtx_bytes(temp2->name, &rtx1);
        temp2->rtx1_cnt.tx_bytes = rtx1.tx_bytes;
        temp2->rtx1_cnt.rx_bytes = rtx1.rx_bytes;
        temp2->rtx1_cnt.rtx_time = rtx1.rtx_time;
        temp2->speed_level &= 0x00;
        temp2 = temp2->next;
    }
    while (temp3 != NULL) {
        cal_netinterface_speed(&temp3->u_speed, &temp3->d_speed, &temp3->speed_level,
                               (&temp3->rtx0_cnt), (&temp3->rtx1_cnt));
        temp3 = temp3->next;
    }
}

void cal_netinterface_speed(double *u_speed, double *d_speed, unsigned char *level,
                            RTX_BYTES *rtx0, RTX_BYTES *rtx1) {
    long int time_lapse;
    time_lapse = (rtx1->rtx_time.tv_sec * 1000 + rtx1->rtx_time.tv_usec / 1000) -
                 (rtx0->rtx_time.tv_sec * 1000 + rtx0->rtx_time.tv_usec / 1000);
    *d_speed = (rtx1->rx_bytes - rtx0->rx_bytes) * 1.0 / (1024 * time_lapse * 1.0 / 1000);
    *u_speed = (rtx1->tx_bytes - rtx0->tx_bytes) * 1.0 / (1024 * time_lapse * 1.0 / 1000);

    if (*d_speed >= MB * 1.0) {
        *d_speed = *d_speed / 1024;
        *level |= 0x1;
    } else {
        *level &= 0xFE;
    }

    if (*u_speed >= MB * 1.0) {
        *u_speed = *u_speed / 1024;
        *level |= 0x1 << 1;
    } else {
        *level &= 0xFD;
    }
}

int main() {
    int nums = 0;
    init_db();
    p_interface = (NET_INTERFACE *)malloc(sizeof(NET_INTERFACE));
    get_interface_info(&p_interface, &nums);
    printf("net_interface nums: %d\n", nums);

    show_netinterfaces(p_interface, 0);
    pthread_t thread_net_id, thread_core_id;
    pthread_create(&thread_net_id, NULL, (void *)thread_net, NULL);
    pthread_create(&thread_core_id, NULL, (void *)thread_core, NULL);

    pthread_join(thread_net_id, NULL);
    pthread_join(thread_core_id, NULL);

    sqlite3_close(db);
    return 0;
}
