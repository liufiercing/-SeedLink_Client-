#ifndef MSEED_HEADER_H
#define MSEED_HEADER_H

#include <stdint.h>

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
    uint8_t numblockettes;
    int32_t time_correct;
    uint16_t data_offset;
    uint16_t blockette_offset;
} MS2FSDH;

// 函数声明
int is_leap_year(int year);
void day_to_month_day(int year, int day_of_year, int *month, int *day);
int parse_mseed_header(const unsigned char *buffer, MS2FSDH *header);
void print_mseed_header(const MS2FSDH *header);

#endif // MSEED_HEADER_H 