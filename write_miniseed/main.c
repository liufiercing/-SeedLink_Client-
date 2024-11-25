#include <stdio.h>
#include <string.h>
#include <math.h>       // 添加数学库头文件
#include <stdlib.h>     // 添加 malloc, free, calloc 的声明
#include <time.h>       // 添加时间函数头文件

// 如果M_PI未定义，则定义它
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include "mseed_header.h"
#include "blockette.h"
#include "steim2.h"
#include "utils.h"

// 

int main() {
    // 创建一个测试用的MSEED头部结构体
    MS2FSDH header = {0};  // 初始化为0
    
    // 设置一些测试数据
    memcpy(header.sequence_number, "000001", 6);
    header.dataquality = 'D';
    memcpy(header.station, "BJSHS", 5);
    memcpy(header.location, "00", 2);
    memcpy(header.channel, "BHZ", 3);
    memcpy(header.network, "BJ", 2);
    header.reserved = 0x20;

    // 设置样本数量
    header.numsamples = 20;
    
    // 自动计算记录长度和帧数
    int reclen = calculate_record_length(header.numsamples);
    int frame_count = calculate_frame_count(header.numsamples);
    int compressed_words = calculate_compressed_size(header.numsamples);
    
    // 使用月日转换为年内天数 (uint16_t year, uint8_t month, uint8_t day, uint16_t *alldays)
    header.year = 2024;
    uint8_t month = 3;
    uint8_t day = 15;
    uint16_t day_of_year;
    header.day = month_day_to_day(header.year, month, day, &day_of_year);
    printf("2024年3月15日 转换为年内天数: %d\n", header.day);
    
    // 反向转换验证
    uint8_t check_month, check_day;
    day_to_month_day(header.year, header.day, &check_month, &check_day);
    printf("年内天数 %d 转换回日期: %d月%d日\n\n", header.day, check_month, check_day);
    
    // 设置时间为 14:30:00.0000
    header.hour = 14;
    header.min = 30;
    header.sec = 0;
    header.fract = 0;
    header.unused = 0;
    
    // 设置采样信息
    header.samprate_fact = 100;
    header.samprate_mult = 1;
    
    // 设置标志位
    header.act_flags = 0;
    header.io_flags = 0;
    header.dq_flags = 0;
    header.numblockettes = 2;
    
    // 设置偏移量
    header.time_correct = 0;
    header.data_offset = 48 + 8 + 8;
    header.blockette_offset = 48;
    
    // 1. 首先打印原始头部信息
    printf("原始头部信息：\n");
    print_mseed_header(&header);
    
    // 2. 创建并打印Blockette 1000
    MS2Blockette1000 b1000 = {
        .type = 1000,
        .next_offset = 48 + 8,
        .encoding = 11,
        .byteorder = 1,
        .reclen = reclen,        // 使用计算得到的记录长度
        .reserved = 0
    };
    print_blockette(&b1000, 1000);
    
    // 3. 创建并打印Blockette 1001
    MS2Blockette1001 b1001 = {
        .type = 1001,
        .next_offset = 0,
        .timing_quality = 100,
        .microsecond = 38,
        .reserved = 0,
        .frame_count = frame_count  // 使用计算得到的帧数
    };
    print_blockette(&b1001, 1001);
    
    // 打开文件准备写入
    FILE *fp = fopen("test.mseed", "wb");
    if (!fp) {
        printf("无法创建文件\n");
        return -1;
    }

    // 写入FSDH头部和blockettes
    write_mseed_fsdh(fp, &header);
    write_blockette_1000(fp, &b1000);
    write_blockette_1001(fp, &b1001);

    // 生成和压缩数据
    int32_t samples[header.numsamples];
    // 使用固定的种子数
    srand(12345);  // 使用固定值12345作为种子
    
    for (int i = 0; i < header.numsamples; i++) {
        // 生成基本的正弦波
        double sine_wave = 20000.0 * sin(2.0 * M_PI * i / header.numsamples);
        
        // 添加随机噪声 (±1000范围内的噪声)
        double noise = (rand() % 2001 - 1000) * 1;  // 缩小噪声幅度到±100
        
        // 合并信号和噪声
        samples[i] = (int32_t)(sine_wave + noise);
    }

    // 压缩数据
    int32_t *compressed = malloc(compressed_words * sizeof(int32_t));  // 动态分配压缩缓冲区
    if (!compressed) {
        printf("内存分配失败\n");
        fclose(fp);
        return -1;
    }

    uint32_t byteswritten;
    int64_t num_samples = msr_encode_steim2(samples, header.numsamples, compressed, 
                                          compressed_words * 4, 0,
                                          &byteswritten, "BJSHS", 1);

    if (num_samples < 0) {
        printf("压缩数据失败\n");
        free(compressed);
        fclose(fp);
        return -1;
    }

    // 移动到数据部分开始处
    fseek(fp, header.data_offset, SEEK_SET);

    // 写入压缩数据
    write_steim2_data(fp, compressed, byteswritten/4);

    // 如果需要填充，确保填充到正确的大小
    int record_size = 1 << reclen;  // 计算实际记录大小
    int remaining = record_size - (header.data_offset + byteswritten);
    if (remaining > 0) {
        unsigned char *padding = calloc(remaining, 1);
        if (padding) {
            fwrite(padding, 1, remaining, fp);
            free(padding);
        }
    }

    free(compressed);
    fclose(fp);
    printf("\n数据已写入 test.mseed 文件\n");
    printf("压缩了 %lld 个样本，写入了 %u 字节的数据\n", (long long)num_samples, byteswritten);
    printf("记录长度: 2^%d = %d 字节\n", reclen, 1 << reclen);
    printf("帧数: %d\n", frame_count);
    
    return 0;
}
