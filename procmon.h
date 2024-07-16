/*
 * @Description: procmon.h
 * @Version: 1.0
 */
#ifndef _PROCMON_H
#define _PROCMON_H

#define BUFFSIZE 512
#define WAIT_SECOND 1
#define MB 1024

#define DEBUG 0

typedef struct _CPU_PACKED {
    char name[16];
    unsigned int user;   // Time spent in user mode
    unsigned int nice;   // Time spent in user mode with low priority
    unsigned int system; // Time spent in system mode
    unsigned int idle;   // Time spent in idle
} CPU_OCCUPY;

/* Structure to hold system information */
struct sysinfo info;

/* Structure to hold host state information */
typedef struct _HOST_STATE {
    int hour;
    int minute;
    double cpu_used;
    double mem_used;
} HOST_STATE;

/* Structure to hold received and transmitted bytes */
typedef struct _RTX_BYTES {
    long int tx_bytes;
    long int rx_bytes;
    struct timeval rtx_time;
} RTX_BYTES;

/* Structure to hold network interface information */
typedef struct _NET_INTERFACE {
    char name[16];  /* Network interface name */
    char ip[16];    /* Network interface IP */
    double d_speed; /* Download speed */
    double u_speed; /* Upload speed */
    char mac[13];   /* Network interface MAC address */
    /* Speed level bit 7~0
     * bit[0]=d_speed
     * bit[1]=u_speed
     * 1:MB/s 0:KB/s
     */
    unsigned char speed_level; /* Speed level */
    RTX_BYTES rtx0_cnt;
    RTX_BYTES rtx1_cnt;
    struct _NET_INTERFACE *next; /* Linked list pointer */
} NET_INTERFACE;

/**
 * @description: Open sysinfo to read memory and runtime information
 * @param {*}
 * @return {*}
 */
void open_sysinfo();

/**
 * @description: Get the runtime of the host
 * @param {int} *hours
 * @param {int} *minutes
 * @return {*}
 */
void get_host_runtime(int *hours, int *minutes);

/**
 * @description: Get CPU load at a given moment
 * @param {CPU_OCCUPY} *cpu
 * @return {*}
 */
void get_cpuoccupy(CPU_OCCUPY *cpu);

/**
 * @description: Calculate CPU occupancy rate
 * @param {CPU_OCCUPY} *c1
 * @param {CPU_OCCUPY} *c2
 * @return {*}
 */
double cal_occupy(CPU_OCCUPY *c1, CPU_OCCUPY *c2);

/**
 * @description: Get memory usage rate
 * @param {double} *mem_used
 * @return {*}
 */
void get_mem_usage(double *mem_used);

/**
 * @description: Get CPU usage rate
 * @param {double} *cpu_used
 * @return {*}
 */
void get_cpu_usage(double *cpu_used);

/**
 * @description: Open network interface device file /proc/net/dev
 * @param {FILE} **fd
 * @return {*}
 */
void open_netconf(FILE **fd);

/**
 * @description: Get the number of network interfaces and their IP addresses
 * @param {NET_INTERFACE} **net
 * @param {int} *n Number of network interfaces
 * @return {*}
 */
int get_interface_info(NET_INTERFACE **net, int *n);

/**
 * @description: Display active network interfaces
 * @param {NET_INTERFACE} *p_net
 * @param {int} n
 * @return {*}
 */
void show_netinterfaces(NET_INTERFACE *p_net, int n);

/**
 * @description: Get the transmitted and received bytes of a network interface at a given moment
 * @param {char} *name
 * @param {RTX_BYTES} *rtx
 * @return {*}
 */
void get_rtx_bytes(char *name, RTX_BYTES *rtx);

/**
 * @description: Copy rtx structure
 * @param {RTX_BYTES} *dest
 * @param {RTX_BYTES} *src
 * @return {*}
 */
void rtx_bytes_copy(RTX_BYTES *dest, RTX_BYTES *src);

/**
 * @description: Calculate the upload and download speed of a network interface
 * @param {double} *u_speed
 * @param {double} *d_speed
 * @param {unsigned char} *level
 * @param {RTX_BYTES} *rtx0
 * @param {RTX_BYTES} *rtx1
 * @return {*}
 */
void cal_netinterface_speed(double *u_speed, double *d_speed,
                            unsigned char *level, RTX_BYTES *rtx0,
                            RTX_BYTES *rtx1);

/**
 * @description: Get the speed information of the host network interface
 * @param {NET_INTERFACE} *p_net
 * @return {*}
 */
void get_network_speed(NET_INTERFACE *p_net);

/**
 * @description: Network information monitoring thread
 * @param {*}
 * @return {*}
 */
void *thread_net(void *arg);

/**
 * @description: CPU, memory, and runtime monitoring thread
 * @param {*}
 * @return {*}
 */
void *thread_core(void *arg);

/**
 * @description: Network traffic monitoring thread using nftables
 * @param {*}
 * @return {*}
 */
void *thread_nft(void *arg);

#endif
