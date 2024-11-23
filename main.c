#include "seedlink.h"
#include "miniseed.h"
#include "server.h"

int main()
{
    seedlink_log(LOG_INFO, "启动SeedLink客户端");

    // 定义台站信息
    const char *network = "II";
    const char *station = "BFO";
    const char *location = "00";

    // 请求三分量数据
    const char *channels[] = {"BHZ", "BHN", "BHE"}; // 三分量数据
    int channel_count = sizeof(channels) / sizeof(channels[0]);

    // 创建TCP服务器
    seedlink_log(LOG_INFO, "正在创建TCP服务器...");
    TCPServer* server = server_create(SERVER_PORT);
    if (!server) {
        seedlink_log(LOG_ERROR, "创建TCP服务器失败");
        return 1;
    }

    // 在新线程中启动服务器
    pthread_t server_thread;
    if (pthread_create(&server_thread, NULL, (void*)server_start, server) != 0) {
        seedlink_log(LOG_ERROR, "启动服务器线程失败");
        server_destroy(server);
        return 1;
    }

    // 创建SeedLink实例
    seedlink_log(LOG_INFO, "正在创建SeedLink实例...");
    SeedLink *sl = seedlink_create(SEEDLINK_SERVER, SEEDLINK_PORT);
    if (!sl)
    {
        seedlink_log(LOG_ERROR, "创建SeedLink实例失败");
        server_destroy(server);
        return 1;
    }

    // 连接服务器
    seedlink_log(LOG_INFO, "正在连接到服务器 %s:%d...", SEEDLINK_SERVER, SEEDLINK_PORT);
    if (seedlink_connect(sl) < 0)
    {
        seedlink_log(LOG_ERROR, "连接服务器失败");
        seedlink_destroy(sl);
        server_destroy(server);
        return 1;
    }

    // 握手
    seedlink_log(LOG_INFO, "正在进行握手...");
    if (seedlink_handshake(sl) < 0)
    {
        seedlink_log(LOG_ERROR, "握手失败");
        seedlink_destroy(sl);
        server_destroy(server);
        return 1;
    }

    // 请求多个通道的数据
    seedlink_log(LOG_INFO, "正在请求台站数据: %s.%s.%s.*", network, station, location);
    if (seedlink_request_channels(sl, network, station, location, channels, channel_count) < 0)
    {
        seedlink_log(LOG_ERROR, "请求台站数据失败");
        seedlink_destroy(sl);
        server_destroy(server);
        return 1;
    }

    // 读取数据
    seedlink_log(LOG_INFO, "开始接收数据...");
    char buffer[SEEDLINK_PACKET_SIZE];
    SeedlinkPacket packet;
    char filename[256];

    while (1)
    {
        int bytes_read = recv(sl->sockfd, buffer, SEEDLINK_PACKET_SIZE, MSG_WAITALL);
        if (bytes_read <= 0)
        {
            seedlink_log(LOG_ERROR, "读取数据失败或连接关闭: %s", strerror(errno));
            break;
        }

        if (bytes_read != SEEDLINK_PACKET_SIZE)
        {
            seedlink_log(LOG_WARN, "接收到不完整的数据包: %d字节", bytes_read);
            continue;
        }

        // 解析SeedLink包头和miniSEED头
        if (seedlink_parse_packet(buffer, &packet) == 0)
        {
            miniseed_parse_header(&packet.data.mseed);

            // 构造文件名并保存数据
            format_mseed_filename(&packet.data.mseed, filename, sizeof(filename));
            miniseed_save_data(&packet.data.raw, 512, filename);

            // 直接转发miniSEED数据给所有连接的客户端
            server_broadcast_data(server, (const unsigned char*)&packet.data.raw, 512);
        }
    }

    // 清理资源
    seedlink_destroy(sl);
    server_stop(server);
    pthread_join(server_thread, NULL);
    server_destroy(server);
    
    seedlink_log(LOG_INFO, "客户端退出");
    return 0;
}