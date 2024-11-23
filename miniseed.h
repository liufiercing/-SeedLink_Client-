#ifndef MINISEED_H
#define MINISEED_H

#include <stdint.h>

// miniSEED 2.4 固定头结构体 (48字节)
#pragma pack(1)
typedef struct {
    char        sequence_number[6];    // 0-5   记录序列号
    char        dataquality;           // 6     数据质量标识
    char        reserved;              // 7     保留字节
    char        station[5];            // 8-12  台站代码
    char        location[2];           // 13-14 位置标识
    char        channel[3];            // 15-17 通道代码
    char        network[2];            // 18-19 台网代码
    uint16_t    year;                  // 20-21 年份
    uint16_t    day;                   // 22-23 一年中的第几天
    uint8_t     hour;                  // 24    小时
    uint8_t     min;                   // 25    分钟
    uint8_t     sec;                   // 26    秒
    uint8_t     unused;               // 27    未使用字节
    uint16_t    fract;                // 28-29 毫秒
    uint16_t    numsamples;           // 30-31 采样点数
    int16_t     samprate_fact;        // 32-33 采样率因子
    int16_t     samprate_mult;        // 34-35 采样率乘数
    uint8_t     act_flags;            // 36    活动标志
    uint8_t     io_flags;             // 37    IO标志
    uint8_t     dq_flags;             // 38    数据质量标志
    uint8_t     numblockettes;        // 39    blockette数量
    int32_t     time_correct;         // 40-43 时间校正
    uint16_t    data_offset;          // 44-45 数据开始偏移
    uint16_t    blockette_offset;     // 46-47 blockette开始偏移
} MiniSeedHeader;

// Blockette 1000 结构体 (8字节)
typedef struct {
    uint16_t blockette_type;    // 固定为1000
    uint16_t next_blockette;    // 下一个blockette的偏移
    uint8_t  encoding;          // 数据编码格式
    uint8_t  byte_order;        // 字节序
    uint8_t  data_record_length;// 数据记录长度（以2为底的对数）
    uint8_t  reserved;          // 保留
} Blockette1000;
#pragma pack()

// 函数声明
void miniseed_parse_header(const MiniSeedHeader* mseed);
int miniseed_save_data(const void* data, size_t size, const char* filename);

#endif 