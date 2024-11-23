#ifndef SEEDLINK_H
#define SEEDLINK_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>
#include <errno.h>
#include <stdarg.h>
#include <ctype.h>
#include "miniseed.h"  // 包含miniSEED相关定义

#define BUFFER_SIZE 1024
#define SEEDLINK_PORT 18000
#define SEEDLINK_SERVER "rtserve.iris.washington.edu"
#define SEEDLINK_PACKET_SIZE 520  // 8 bytes header + 512 bytes miniSEED

// SeedLink头结构体
typedef struct {
    char prefix[2];             // "SL"
    char sequence_number[6];    // 6位十六进制序列号
} SeedlinkHeader;

// SeedLink数据包结构体
typedef struct {
    SeedlinkHeader header;     // 8字节头
    union {
        unsigned char raw[512];  // 原始数据
        MiniSeedHeader mseed;    // 解析后的miniSEED头
    } data;
} SeedlinkPacket;

// SeedLink连接结构体
typedef struct {
    int sockfd;
    char server_name[256];
    int port;
} SeedLink;

// 日志级别
typedef enum {
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR
} LogLevel;

// 函数声明
void seedlink_log(LogLevel level, const char* format, ...);
SeedLink* seedlink_create(const char* server, int port);
int seedlink_connect(SeedLink* sl);
int seedlink_handshake(SeedLink* sl);
int seedlink_request_channels(SeedLink* sl, 
                            const char* network, 
                            const char* station,
                            const char* location, 
                            const char* channels[], 
                            int channel_count);
int seedlink_parse_packet(const char* buffer, SeedlinkPacket* packet);
void seedlink_close(SeedLink* sl);
void seedlink_destroy(SeedLink* sl);
void trim_string(char* str);
void format_mseed_filename(const MiniSeedHeader* mseed, char* filename, size_t size);

#endif 