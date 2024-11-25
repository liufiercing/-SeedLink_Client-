#ifndef BLOCKETTE_H
#define BLOCKETTE_H

#include <stdint.h>

// Blockette 100 - 采样率
typedef struct {
    uint16_t type;
    uint16_t next_offset;
    float samprate;
    uint8_t flags;
    uint8_t reserved[3];
} MS2Blockette100;

// Blockette 200 - 通用事件检测
typedef struct {
    uint16_t type;
    uint16_t next_offset;
    float amplitude;
    float period;
    float background_est;
    uint8_t flags;
    uint8_t reserved;
    uint16_t year;
    uint16_t day;
    uint8_t hour;
    uint8_t min;
    uint8_t sec;
    uint8_t unused;
    uint16_t fract;
    char detector[24];
} MS2Blockette200;

// Blockette 201 - Murdock事件检测
typedef struct {
    uint16_t type;
    uint16_t next_offset;
    float amplitude;
    float period;
    float background_est;
    uint8_t flags;
    uint8_t reserved;
    uint16_t year;
    uint16_t day;
    uint8_t hour;
    uint8_t min;
    uint8_t sec;
    uint8_t unused;
    uint16_t fract;
    uint8_t snr_values[6];
    uint8_t loopback;
    uint8_t pick_algorithm;
    char detector[24];
} MS2Blockette201;

// Blockette 300 - 步进校准
typedef struct {
    uint16_t type;
    uint16_t next_offset;
    uint16_t year;
    uint16_t day;
    uint8_t hour;
    uint8_t min;
    uint8_t sec;
    uint8_t unused;
    uint16_t fract;
    uint8_t num_calibrations;
    uint8_t flags;
    uint32_t step_duration;
    uint32_t interval_duration;
    float amplitude;
    char input_channel[3];
    uint8_t reserved;
    uint32_t reference_amplitude;
    char coupling[12];
    char rolloff[12];
} MS2Blockette300;

// Blockette 1000 - miniSEED标识符 大小是8字节
typedef struct {
    uint16_t type;
    uint16_t next_offset;
    uint8_t encoding;
    uint8_t byteorder;
    uint8_t reclen;
    uint8_t reserved;
} MS2Blockette1000;

// Blockette 1001 - 数据扩展 大小是8字节
typedef struct {
    uint16_t type;
    uint16_t next_offset;
    uint8_t timing_quality;
    int8_t microsecond;
    uint8_t reserved;
    uint8_t frame_count;
} MS2Blockette1001;

// 解析Blockette的函数声明
int parse_blockette(const unsigned char *blockette_start, void *blockette_data);
void print_blockette(const void *blockette_data, uint16_t type);

// 处理记录中的所有Blockettes
int process_blockettes(const unsigned char *record_start, uint16_t first_blockette_offset, uint8_t num_blockettes);

// 在文件末尾添加这些声明
void write_blockette_1000(FILE *fp, const MS2Blockette1000 *b1000);
void write_blockette_1001(FILE *fp, const MS2Blockette1001 *b1001);

#endif // BLOCKETTE_H 