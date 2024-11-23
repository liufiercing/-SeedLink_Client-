#include <stdio.h>
#include <string.h>
#include "miniseed.h"
#include "seedlink.h"  // 为了使用日志函数

// 字节序转换
static uint16_t swap16(uint16_t value) {
    return ((value & 0xFF00) >> 8) | ((value & 0x00FF) << 8);
}

// 计算采样率
static double calculate_sample_rate(const MiniSeedHeader* header) {
    int16_t factor = swap16(header->samprate_fact);
    int16_t mult = swap16(header->samprate_mult);
    return (double)factor * mult;
}

// 获取编码格式字符串
static const char* get_encoding_str(uint8_t encoding) {
    switch(encoding) {
        case 10: return "STEIM1";
        case 11: return "STEIM2";
        default: return "Unknown";
    }
}

// 解析Blockette 1000
static void parse_blockette_1000(const unsigned char* data, int offset) {
    Blockette1000* b1000 = (Blockette1000*)(data + offset);
    uint16_t type = swap16(b1000->blockette_type);
    uint16_t next = swap16(b1000->next_blockette);
    int record_length = 1 << b1000->data_record_length;
    
    seedlink_log(LOG_INFO, 
        "B1000: type=%d next=%d 编码=%s(%d) %s 记录长度=2^%d=%d字节",
        type, next,
        get_encoding_str(b1000->encoding),
        b1000->encoding,
        b1000->byte_order == 1 ? "大端序" : "小端序",
        b1000->data_record_length,
        record_length);
}

// 解析miniSEED头
void miniseed_parse_header(const MiniSeedHeader* mseed) {
    // 解析序列号和数据质量
    char sequence[7] = {0};
    memcpy(sequence, mseed->sequence_number, 6);
    
    // 解析台站信息
    char station[6] = {0}, location[3] = {0}, channel[4] = {0}, network[3] = {0};
    memcpy(station, mseed->station, 5);
    memcpy(location, mseed->location, 2);
    memcpy(channel, mseed->channel, 3);
    memcpy(network, mseed->network, 2);
    
    // 计算和转换其他字段
    double sample_rate = calculate_sample_rate(mseed);
    uint16_t samples = swap16(mseed->numsamples);
    uint16_t year = swap16(mseed->year);
    uint16_t day = swap16(mseed->day);
    uint16_t msec = swap16(mseed->fract);
    uint16_t blockette_offset = swap16(mseed->blockette_offset);
    
    // 打印miniSEED头前8字节的十六进制
    // seedlink_log(LOG_INFO, "miniSEED头前8字节十六进制:");
    // printf("    ");
    // for(int i = 0; i < 8; i++) {
    //     printf("%02X ", ((const unsigned char*)mseed)[i]);
    // }
    // printf("\n");
    
    // 打印解析结果
    seedlink_log(LOG_INFO, 
        "miniSEED头解析: 序列号=%s 质量=%c %s.%s.%s.%s %04d-%03d %02d:%02d:%02d.%04d 采样率:%d/%d=%.1fHz 点数:%d",
        sequence,
        mseed->dataquality,
        network, station, location, channel,
        year, day,
        mseed->hour,
        mseed->min,
        mseed->sec,
        msec,
        mseed->samprate_fact,
        mseed->samprate_mult,
        sample_rate,
        samples);
    
    if (mseed->numblockettes > 0 && blockette_offset >= 48) {
        parse_blockette_1000((const unsigned char*)mseed, blockette_offset);
    }
}

// 保存miniSEED数据
int miniseed_save_data(const void* data, size_t size, const char* filename) {
    FILE* fp = fopen(filename, "ab");
    if (!fp) {
        seedlink_log(LOG_ERROR, "无法打开文件 %s: %s", filename, strerror(errno));
        return -1;
    }
    
    size_t written = fwrite(data, 1, size, fp);
    fclose(fp);
    
    if (written != size) {
        seedlink_log(LOG_ERROR, "写入miniSEED数据失败: %s", strerror(errno));
        return -1;
    }
    
    return 0;
} 