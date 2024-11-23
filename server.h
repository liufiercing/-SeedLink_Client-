#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <errno.h>
#include "seedlink.h"

#define MAX_CLIENTS 10
#define SERVER_PORT 8000

// 前向声明
struct TCPServer;

// 先定义 ClientConnection
typedef struct {
    int sockfd;
    struct sockaddr_in addr;
    pthread_t thread;
    int is_active;
    struct TCPServer* server;  // 使用前向声明的类型
} ClientConnection;

// 再定义 TCPServer
typedef struct TCPServer {
    int server_fd;
    struct sockaddr_in addr;
    ClientConnection clients[MAX_CLIENTS];  // 现在可以使用 ClientConnection
    int client_count;
    pthread_mutex_t mutex;
} TCPServer;

// 函数声明
TCPServer* server_create(int port);
int server_start(TCPServer* server);
int server_broadcast_data(TCPServer* server, const unsigned char* data, size_t size);
void server_stop(TCPServer* server);
void server_destroy(TCPServer* server);

#endif 