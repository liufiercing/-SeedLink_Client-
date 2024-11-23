#include "server.h"

// 打印十六进制数据
static void print_hex_dump(const unsigned char* data, size_t size) {
    printf("\nHEX Dump (%zu bytes):\n", size);
    for (size_t i = 0; i < size; i++) {
        if (i % 16 == 0) {
            printf("%04zX: ", i);
        }
        printf("%02X ", data[i]);
        if ((i + 1) % 16 == 0) {
            printf("\n");
        }
    }
    if (size % 16 != 0) {
        printf("\n");
    }
    printf("\n");
}

// 客户端处理线程
static void* client_handler(void* arg) {
    ClientConnection* client = (ClientConnection*)arg;
    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client->addr.sin_addr, client_ip, sizeof(client_ip));
    int client_port = ntohs(client->addr.sin_port);
    
    // 设置socket选项，只保留基本的keepalive
    int keepalive = 1;
    setsockopt(client->sockfd, SOL_SOCKET, SO_KEEPALIVE, &keepalive, sizeof(keepalive));
    
    // 设置接收超时
    struct timeval tv;
    tv.tv_sec = 60;
    tv.tv_usec = 0;
    setsockopt(client->sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));
    
    // 发送欢迎消息
    const char* welcome = "Welcome to MiniSEED Server\n";
    send(client->sockfd, welcome, strlen(welcome), 0);
    
    // 等待客户端命令
    while (client->is_active) {
        char buffer[1024];
        int n = recv(client->sockfd, buffer, sizeof(buffer)-1, 0);
        
        if (n < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // 接收超时，检查连接状态
                continue;
            }
            seedlink_log(LOG_INFO, "客户端连接错误: %s:%d (%s)", 
                        client_ip, client_port, strerror(errno));
            break;
        } else if (n == 0) {
            seedlink_log(LOG_INFO, "客户端正常断开连接: %s:%d", client_ip, client_port);
            break;
        }
    }
    
    // 更新客户端状态
    pthread_mutex_lock(&client->server->mutex);
    client->is_active = 0;
    client->server->client_count--;
    seedlink_log(LOG_INFO, "客户端 %s:%d 已断开 (当前连接数: %d)", 
                 client_ip, client_port, client->server->client_count);
    pthread_mutex_unlock(&client->server->mutex);
    
    close(client->sockfd);
    return NULL;
}

// 创建服务器
TCPServer* server_create(int port) {
    TCPServer* server = (TCPServer*)malloc(sizeof(TCPServer));
    if (!server) return NULL;
    
    // 初始化所有字段
    memset(server, 0, sizeof(TCPServer));
    server->server_fd = -1;
    server->client_count = 0;
    
    // 初始化地址
    server->addr.sin_family = AF_INET;
    server->addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server->addr.sin_port = htons(port);
    
    // 初始化互斥锁
    if (pthread_mutex_init(&server->mutex, NULL) != 0) {
        free(server);
        return NULL;
    }
    
    // 初始化客户端数组
    for (int i = 0; i < MAX_CLIENTS; i++) {
        server->clients[i].sockfd = -1;
        server->clients[i].is_active = 0;
        server->clients[i].server = server;  // 设置指向服务器的指针
    }
    
    return server;
}

// 启动服务器
int server_start(TCPServer* server) {
    // 创建socket
    server->server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server->server_fd < 0) {
        seedlink_log(LOG_ERROR, "创建socket失败: %s", strerror(errno));
        return -1;
    }
    
    // 设置socket选项
    int opt = 1;
    if (setsockopt(server->server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        seedlink_log(LOG_ERROR, "设置socket选项失败: %s", strerror(errno));
        return -1;
    }
    
    // 绑定地址
    if (bind(server->server_fd, (struct sockaddr*)&server->addr, sizeof(server->addr)) < 0) {
        seedlink_log(LOG_ERROR, "绑定地址失败: %s", strerror(errno));
        return -1;
    }
    
    // 开始监听
    if (listen(server->server_fd, 3) < 0) {
        seedlink_log(LOG_ERROR, "监听失败: %s", strerror(errno));
        return -1;
    }
    
    seedlink_log(LOG_INFO, "服务器正在监听 0.0.0.0:%d", ntohs(server->addr.sin_port));
    
    // 接受客户端连接
    while (1) {
        struct sockaddr_in client_addr;
        socklen_t addrlen = sizeof(client_addr);
        
        int client_fd = accept(server->server_fd, (struct sockaddr*)&client_addr, &addrlen);
        if (client_fd < 0) {
            seedlink_log(LOG_ERROR, "接受连接失败: %s", strerror(errno));
            continue;
        }
        
        // 获取客户端IP和端口
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));
        int client_port = ntohs(client_addr.sin_port);
        
        // 查找空闲的客户端槽位
        pthread_mutex_lock(&server->mutex);
        int slot = -1;
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (!server->clients[i].is_active) {
                slot = i;
                break;
            }
        }
        
        if (slot >= 0) {
            // 初始化客户端连接
            server->clients[slot].sockfd = client_fd;
            server->clients[slot].addr = client_addr;
            server->clients[slot].is_active = 1;
            server->client_count++;
            
            // 创建客户端处理线程
            pthread_create(&server->clients[slot].thread, NULL, client_handler, &server->clients[slot]);
            
            seedlink_log(LOG_INFO, "新客户端连接: %s:%d (总连接数: %d)", 
                        client_ip, client_port, server->client_count);
        } else {
            // 达到最大客户端数量
            close(client_fd);
            seedlink_log(LOG_WARN, "拒绝客户端连接 %s:%d - 达到最大连接数 %d", 
                        client_ip, client_port, MAX_CLIENTS);
        }
        pthread_mutex_unlock(&server->mutex);
    }
    
    return 0;
}

// 广播数据给所有客户端
int server_broadcast_data(TCPServer* server, const unsigned char* data, size_t size) {
    pthread_mutex_lock(&server->mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (server->clients[i].is_active) {
            send(server->clients[i].sockfd, data, size, 0);
        }
    }
    pthread_mutex_unlock(&server->mutex);
    return 0;
}

// 停止服务器
void server_stop(TCPServer* server) {
    // 关闭所有客户端连接
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (server->clients[i].is_active) {
            close(server->clients[i].sockfd);
            server->clients[i].is_active = 0;
            pthread_join(server->clients[i].thread, NULL);
        }
    }
    
    // 关闭服务器socket
    close(server->server_fd);
}

// 销毁服务器
void server_destroy(TCPServer* server) {
    if (server) {
        server_stop(server);
        pthread_mutex_destroy(&server->mutex);
        free(server);
    }
} 