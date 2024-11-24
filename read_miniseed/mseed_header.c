#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mseed_header.h"

// 判断是否为闰年
int is_leap_year(int year)
{
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

// 将年内天数转换为月和日
void day_to_month_day(int year, int day_of_year, int *month, int *day)
{
    static const int days_in_month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int leap = is_leap_year(year);
    int days_left = day_of_year;
    int current_month = 0;

    while (days_left > days_in_month[current_month])
    {
        days_left -= days_in_month[current_month];
        if (current_month == 1 && leap)
        {
            days_left--;
        }
        current_month++;
    }

    *month = current_month + 1;
    *day = days_left;
}

// 解析SEED头部
int parse_mseed_header(const unsigned char *buffer, MS2FSDH *header) {
    if (!buffer || !header) {
        return -1;
    }

    memcpy(header->sequence_number, buffer, 6);
    header->dataquality = buffer[6];
    header->reserved = buffer[7];
    memcpy(header->station, buffer + 8, 5);
    memcpy(header->location, buffer + 13, 2);
    memcpy(header->channel, buffer + 15, 3);
    memcpy(header->network, buffer + 18, 2);

    header->year = (buffer[20] << 8) | buffer[21];
    header->day = (buffer[22] << 8) | buffer[23];
    header->hour = buffer[24];
    header->min = buffer[25];
    header->sec = buffer[26];
    header->unused = buffer[27];
    header->fract = (buffer[28] << 8) | buffer[29];

    header->numsamples = (buffer[30] << 8) | buffer[31];
    header->samprate_fact = (buffer[32] << 8) | buffer[33];
    header->samprate_mult = (buffer[34] << 8) | buffer[35];

    header->act_flags = buffer[36];
    header->io_flags = buffer[37];
    header->dq_flags = buffer[38];
    header->numblockettes = buffer[39];

    header->time_correct = (buffer[40] << 24) | (buffer[41] << 16) | 
                          (buffer[42] << 8) | buffer[43];
    header->data_offset = (buffer[44] << 8) | buffer[45];
    header->blockette_offset = (buffer[46] << 8) | buffer[47];

    return 0;
}

// 打印SEED头部信息
void print_mseed_header(const MS2FSDH *header) {
    if (!header) {
        return;
    }

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

    // 计算日期
    int month, day;
    day_to_month_day(header->year, header->day, &month, &day);

    // 获取当前时间
    char timebuf[64];  // 增大缓冲区大小
    time_t rawtime;
    struct tm *timeinfo;

    time(&rawtime);
    timeinfo = localtime(&rawtime);
    if (!timeinfo) {  // 添加错误检查
        printf("错误：获取本地时间失败\n");
        return;
    }
    
    if (strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", timeinfo) == 0) {  // 添加错误检查
        printf("错误：格式化时间失败\n");
        return;
    }

    // 输出头部信息（单行，带时间戳）
    printf("[%s] Header Info | MSEED: %.2s.%.5s.%.2s.%.3s | %d-%02d-%02d %02d:%02d:%02d.%05d | %d samples @ %.3f Hz | Quality=%c Seq=%.6s | Flags[act:0x%02X io:0x%02X dq:0x%02X] | %d blockettes, offset=%d\n",
           timebuf,
           header->network, header->station, header->location, header->channel,
           header->year, month, day,
           header->hour, header->min, header->sec,
           header->fract * 10 + header->unused,
           header->numsamples, samprate,
           header->dataquality, header->sequence_number,
           header->act_flags, header->io_flags, header->dq_flags,
           header->numblockettes, header->data_offset);
} 