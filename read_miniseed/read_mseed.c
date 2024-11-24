#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "steim2.h"

/* MiniSEED V2.4 固定数据头部长度 */
#define MS2FSDH_LENGTH 48

/* 定义数据记录头结构体 */
typedef struct
{
    char sequence_number[6];
    char dataquality;
    char reserved;
    char station[5];
    char location[2];
    char channel[3];
    char network[2];
    uint16_t year;
    uint16_t day;
    uint8_t hour;
    uint8_t min;
    uint8_t sec;
    uint8_t unused;
    uint16_t fract;
    uint16_t numsamples;
    int16_t samprate_fact;
    int16_t samprate_mult;
    uint8_t act_flags;
    uint8_t io_flags;
    uint8_t dq_flags;
    uint8_t numblockettes; // 后续blockette的总数（在固定头部和数据之间的控制信息块数量）
    int32_t time_correct;
    uint16_t data_offset;
    uint16_t blockette_offset;
} MS2FSDH;

// 判断是否为闰年
int is_leap_year(int year)
{
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

// 将年内天数转换为月和日
void day_to_month_day(int year, int day_of_year, int *month, int *day)
{
    // 每个月的天数（非闰年
    static const int days_in_month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    // 处理闰年
    int leap = is_leap_year(year);

    int days_left = day_of_year;
    int current_month = 0;

    // 遍历月份直到找到正确的月份
    while (days_left > days_in_month[current_month])
    {
        days_left -= days_in_month[current_month];
        if (current_month == 1 && leap)
        { // 如果是2月且是闰年
            days_left--;
        }
        current_month++;
    }

    *month = current_month + 1; // 月份从1开始
    *day = days_left;
}

// 函数声明：解析SEED头部
int parse_mseed_header(const unsigned char *buffer, MS2FSDH *header) {
    if (!buffer || !header) {
        return -1;  // 参数错误
    }

    // 解析基本信息
    memcpy(header->sequence_number, buffer, 6);
    header->dataquality = buffer[6];
    header->reserved = buffer[7];
    memcpy(header->station, buffer + 8, 5);
    memcpy(header->location, buffer + 13, 2);
    memcpy(header->channel, buffer + 15, 3);
    memcpy(header->network, buffer + 18, 2);

    // 解析时间信息（使用大端序）
    header->year = (buffer[20] << 8) | buffer[21];
    header->day = (buffer[22] << 8) | buffer[23];
    header->hour = buffer[24];
    header->min = buffer[25];
    header->sec = buffer[26];
    header->unused = buffer[27];                    // 秒的小数部分（微秒级，0.0001秒分辨率）
    header->fract = (buffer[28] << 8) | buffer[29]; // 秒的小数部分（毫秒级）

    // 解析数据信息
    header->numsamples = (buffer[30] << 8) | buffer[31];
    header->samprate_fact = (buffer[32] << 8) | buffer[33];
    header->samprate_mult = (buffer[34] << 8) | buffer[35];

    // 解析标志位
    header->act_flags = buffer[36];     // 活动标志
    header->io_flags = buffer[37];      // I/O标志
    header->dq_flags = buffer[38];      // 数据质量标志
    header->numblockettes = buffer[39]; // 后续blockette的总数

    // 解析时间校正和偏移量
    header->time_correct = (buffer[40] << 24) | (buffer[41] << 16) | 
                          (buffer[42] << 8) | buffer[43];
    header->data_offset = (buffer[44] << 8) | buffer[45];           // 数据段开始位置
    header->blockette_offset = (buffer[46] << 8) | buffer[47];      // 第一个blockette的位置

    return 0;  // 成功
}

// 函数声明：打印SEED头部信息
void print_mseed_header(const MS2FSDH *header) {
    if (!header) {
        return;
    }

    printf("\n=== SEED 数据记录头部信息 ===\n");
    printf("1. 基本信息:\n");
    printf("序列号: %.6s\n", header->sequence_number);
    printf("数据质量标识: %c\n", header->dataquality);
    printf("保留字节: %c\n", header->reserved);
    printf("台站代码: %.5s\n", header->station);
    printf("位置标识: %.2s\n", header->location);
    printf("通道标识: %.3s\n", header->channel);
    printf("网络代码: %.2s\n", header->network);

    printf("\n2. 时间信息:\n");
    int month, day;
    day_to_month_day(header->year, header->day, &month, &day);
    printf("时间: %d-%02d-%02d %02d:%02d:%02d.%05d\n",
           header->year, month, day,
           header->hour, header->min, header->sec,
           header->fract * 10 + header->unused);

    // 计算采样率
    double samprate = 0.0;
    if (header->samprate_fact != 0) {
        if (header->samprate_fact > 0) {
            if (header->samprate_mult > 0) {
                samprate = (double)header->samprate_fact * (double)header->samprate_mult;
            } else if (header->samprate_mult < 0) {
                samprate = -1.0 * (double)header->samprate_fact / (double)header->samprate_mult;
            }
        } else if (header->samprate_fact < 0) {
            if (header->samprate_mult > 0) {
                samprate = -1.0 * (double)header->samprate_mult / (double)header->samprate_fact;
            } else if (header->samprate_mult < 0) {
                samprate = 1.0 / ((double)header->samprate_fact * (double)header->samprate_mult);
            }
        }
    }

    printf("\n3. 数据信息:\n");
    printf("采样点数: %d\n", header->numsamples);
    printf("采样率因子: %d\n", header->samprate_fact);
    printf("采样率乘数: %d\n", header->samprate_mult);
    printf("计算得到的采样率: %.3f SPS\n", samprate);

    printf("\n4. 标志位信息:\n");
    printf("活动标志: 0x%02X\n", header->act_flags);
    printf("I/O标志: 0x%02X\n", header->io_flags);
    printf("数据质量标志: 0x%02X\n", header->dq_flags);

    printf("\n5. 结构信息:\n");
    printf("Blockette数量: %d (在固定头部和数据之间有%d个控制信息块)\n",
           header->numblockettes, header->numblockettes);
    printf("时间校正: %d\n", header->time_correct);
    printf("数据开始位置: %d (0x%04X)\n", header->data_offset, header->data_offset);
    printf("第一个Blockette位置: %d (0x%04X)\n", header->blockette_offset, header->blockette_offset);
}

int main()
{
    FILE *fp;
    MS2FSDH header;
    unsigned char buffer[MS2FSDH_LENGTH];

    // 打开文件
    fp = fopen("II_BFO_00_BHE1.mseed", "rb");
    if (!fp) {
        printf("错误：无法打开文件\n");
        return 1;
    }

    // 读取固定数据头部
    if (fread(buffer, 1, MS2FSDH_LENGTH, fp) != MS2FSDH_LENGTH) {
        printf("错误：读取文件头失败\n");
        fclose(fp);
        return 1;
    }

    // 解析头部
    if (parse_mseed_header(buffer, &header) != 0) {
        printf("错误：解析头部失败\n");
        fclose(fp);
        return 1;
    }

    // 打印头部信息
    print_mseed_header(&header);

    // 读取数据部分
    if (header.data_offset > MS2FSDH_LENGTH)
    {
        // 跳过中间的blockettes
        fseek(fp, header.data_offset - MS2FSDH_LENGTH, SEEK_CUR);
    }

    // 使用Steim2解码函数读取数据
    read_steim2_data(fp, header.numsamples, header.data_offset);

    fclose(fp);
    return 0;
}