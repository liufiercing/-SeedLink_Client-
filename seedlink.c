#include "seedlink.h"

// 打印十六进制数据
static void print_hex(const unsigned char* data, size_t len) {
    for (size_t i = 0; i < len; i++) {
        printf("%02X ", data[i]);
    }
    printf("\n");
}

// 日志函数
void seedlink_log(LogLevel level, const char* format, ...) {
    time_t now;
    struct tm *timeinfo;
    char timestamp[20];
    va_list args;
    
    time(&now);
    timeinfo = localtime(&now);
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", timeinfo);
    
    fprintf(stderr, "[%s][%s] ", timestamp, 
            level == LOG_DEBUG ? "DEBUG" :
            level == LOG_INFO  ? "INFO"  :
            level == LOG_WARN  ? "WARN"  : "ERROR");
    
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fprintf(stderr, "\n");
}

// 创建SeedLink连接
SeedLink* seedlink_create(const char* server, int port) {
    SeedLink* sl = (SeedLink*)malloc(sizeof(SeedLink));
    if (!sl) return NULL;
    
    strncpy(sl->server_name, server, sizeof(sl->server_name)-1);
    sl->port = port;
    sl->sockfd = -1;
    
    return sl;
}

// 连接到服务器
int seedlink_connect(SeedLink* sl) {
    struct sockaddr_in server_addr;
    struct hostent *server;
    
    sl->sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sl->sockfd < 0) {
        seedlink_log(LOG_ERROR, "无法创建socket: %s", strerror(errno));
        return -1;
    }
    
    server = gethostbyname(sl->server_name);
    if (!server) {
        seedlink_log(LOG_ERROR, "无法解析服务器域名: %s", sl->server_name);
        return -1;
    }
    
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(sl->port);
    memcpy(&server_addr.sin_addr.s_addr, server->h_addr_list[0], server->h_length);
    
    if (connect(sl->sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        seedlink_log(LOG_ERROR, "连接失败: %s", strerror(errno));
        return -1;
    }
    
    return 0;
}

// 握手过程
int seedlink_handshake(SeedLink* sl) {
    char buffer[BUFFER_SIZE];
    const char* hello_msg = "HELLO\r\n";
    
    if (send(sl->sockfd, hello_msg, strlen(hello_msg), 0) < 0) {
        seedlink_log(LOG_ERROR, "发送HELLO失败: %s", strerror(errno));
        return -1;
    }
    
    int n = recv(sl->sockfd, buffer, sizeof(buffer)-1, 0);
    if (n < 0) {
        seedlink_log(LOG_ERROR, "接收响应失败: %s", strerror(errno));
        return -1;
    }
    buffer[n] = '\0';
    
    return 0;
}

// 请求多个通道的数据
int seedlink_request_channels(SeedLink* sl, const char* network, const char* station,
                            const char* location, const char* channels[], int channel_count) {
    char cmd[256];
    char buffer[1024];
    int n;
    
    // 1. 首先发送STATION命令，指定台站
    snprintf(cmd, sizeof(cmd), "STATION %s %s\r\n", station, network);
    seedlink_log(LOG_INFO, "发送: %s", cmd);
    if (send(sl->sockfd, cmd, strlen(cmd), 0) < 0) {
        seedlink_log(LOG_ERROR, "发送STATION命令失败: %s", strerror(errno));
        return -1;
    }
    
    // 等待服务器响应
    n = recv(sl->sockfd, buffer, sizeof(buffer)-1, 0);
    if (n < 0) return -1;
    buffer[n] = '\0';
    seedlink_log(LOG_INFO, "服务器响应: %s", buffer);
    
    // 2. 循环发送SELECT命令，选择每个通道
    for (int i = 0; i < channel_count; i++) {
        snprintf(cmd, sizeof(cmd), "SELECT %s%s.D\r\n", location, channels[i]);
        seedlink_log(LOG_INFO, "发送: %s", cmd);
        if (send(sl->sockfd, cmd, strlen(cmd), 0) < 0) {
            seedlink_log(LOG_ERROR, "发送SELECT命令失败: %s", strerror(errno));
            return -1;
        }
        
        // 等待服务器响应
        n = recv(sl->sockfd, buffer, sizeof(buffer)-1, 0);
        if (n < 0) return -1;
        buffer[n] = '\0';
        seedlink_log(LOG_INFO, "服务器响应: %s", buffer);
    }
    
    // 3. 发送END命令，结束通道选择
    const char* end_cmd = "END\r\n";
    seedlink_log(LOG_INFO, "发送: %s", end_cmd);
    if (send(sl->sockfd, end_cmd, strlen(end_cmd), 0) < 0) {
        seedlink_log(LOG_ERROR, "发送END命令失败: %s", strerror(errno));
        return -1;
    }
    
    // 4. 发送DATA命令，开始数据传输
    const char* data_cmd = "DATA\r\n";
    seedlink_log(LOG_INFO, "发送: %s", data_cmd);
    if (send(sl->sockfd, data_cmd, strlen(data_cmd), 0) < 0) {
        seedlink_log(LOG_ERROR, "发送DATA命令失败: %s", strerror(errno));
        return -1;
    }
    
    return 0;
}

// 获取数据格式描述
static const char* get_format_description(char code) {
    switch(code) {
        case '2': return "miniSEED 2.x";
        case '3': return "miniSEED 3.x";
        case 'J': return "JSON";
        case 'X': return "XML";
        default: return "Unknown";
    }
}

// 获取子格式描述
static const char* get_subformat_description(char code) {
    switch(code) {
        case 'D': return "data/generic";
        case 'E': return "event detection";
        case 'C': return "calibration";
        case 'T': return "timing exception";
        case 'L': return "log";
        case 'O': return "opaque";
        case 'I': return "SeedLink info";
        default: return "Unknown";
    }
}

// 解析SeedLink数据包
int seedlink_parse_packet(const char* buffer, SeedlinkPacket* packet) {
    if (!buffer || !packet) return -1;
    
    const unsigned char* data = (const unsigned char*)buffer;
    
    // 打印原始十六进制数据
    // seedlink_log(LOG_INFO, "SeedLink头(8字节)十六进制: ");
    // printf("    ");
    // print_hex(data, 8);
    
    // 1. Signature (2字节)
    if (data[0] != 'S' || data[1] != 'L') {
        seedlink_log(LOG_ERROR, "无效的SeedLink签名: %.2s", data);
        return -1;
    }
    
    // 2. Data format (1字节)
    char format = data[2];
    
    // 3. Subformat (1字节)
    char subformat = data[3];
    
    // 4. Length of payload (4字节, little endian)
    uint32_t payload_length = data[4] | (data[5] << 8) | 
                             (data[6] << 16) | (data[7] << 24);
    
    // 打印解析结果
    seedlink_log(LOG_INFO, 
        "数据包解析: 签名=SL 格式=%c(%s) 子格式=%c(%s) 负载长度=%u(0x%08X)",
        format, get_format_description(format),
        subformat, get_subformat_description(subformat),
        payload_length, payload_length);
    
    // 复制512字节的miniSEED数据
    memcpy(&packet->data, buffer + 8, 512);
    
    return 0;
}

// 关闭连接
void seedlink_close(SeedLink* sl) {
    if (sl && sl->sockfd != -1) {
        close(sl->sockfd);
        sl->sockfd = -1;
    }
}

// 销毁SeedLink实例
void seedlink_destroy(SeedLink* sl) {
    if (sl) {
        seedlink_close(sl);
        free(sl);
    }
}

// 去除字符串末尾的空格
void trim_string(char* str) {
    for (int i = strlen(str)-1; i >= 0 && str[i] == ' '; i--) {
        str[i] = 0;
    }
}

// 格式化miniSEED文件名
void format_mseed_filename(const MiniSeedHeader* mseed, char* filename, size_t size) {
    char net[3] = {0}, sta[6] = {0}, loc[3] = {0}, cha[4] = {0};
    
    // 复制并确保字符串正确终止
    strncpy(net, mseed->network, 2);
    strncpy(sta, mseed->station, 5);
    strncpy(loc, mseed->location, 2);
    strncpy(cha, mseed->channel, 3);
    
    // 去除字符串末尾的空格
    trim_string(net);
    trim_string(sta);
    trim_string(loc);
    trim_string(cha);
    
    // 构造文件名：network_station_location_channel.mseed
    snprintf(filename, size, "%s_%s_%s_%s.mseed",
             net, sta, loc, cha);
} 