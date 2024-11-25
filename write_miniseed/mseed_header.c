#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mseed_header.h"
#include "utils.h"

// 判断是否为闰年
int is_leap_year(uint16_t year)
{
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

// 将月和日转换为年内天数 
int month_day_to_day(uint16_t year, uint8_t month, uint8_t day, uint16_t *alldays)
{
    static const int days_in_month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int days = 0;

    // 检查输入有效性
    if (month < 1 || month > 12 || day < 1)
        return -1;

    // 检查日期是否超出当月天数
    int max_days = days_in_month[month - 1];
    if (month == 2 && is_leap_year(year))
        max_days++;
    if (day > max_days)
        return -1;

    // 计算之前月份的总天数
    for (int i = 0; i < month - 1; i++)
    {
        days += days_in_month[i];
        if (i == 1 && is_leap_year(year)) // 如果是闰年且已经过了2月
            days++;
    }

    // 加上当月的天数
    *alldays = days + day;

    return *alldays;
}

// 将年内天数转换为月和日
void day_to_month_day(uint16_t year, uint16_t day_of_year, uint8_t *month, uint8_t *day)
{
    static const int days_in_month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int leap = is_leap_year(year);
    uint16_t days_left = day_of_year;
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
int parse_mseed_header(const unsigned char *buffer, MS2FSDH *header)
{
    if (!buffer || !header)
    {
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
void print_mseed_header(const MS2FSDH *header)
{
    if (!header)
    {
        return;
    }

    // 计算采样率
    double samprate = 0.0;
    if (header->samprate_fact != 0)
    {
        if (header->samprate_fact > 0)
        {
            if (header->samprate_mult > 0)
            {
                samprate = (double)header->samprate_fact * (double)header->samprate_mult;
            }
            else if (header->samprate_mult < 0)
            {
                samprate = -1.0 * (double)header->samprate_fact / (double)header->samprate_mult;
            }
        }
        else if (header->samprate_fact < 0)
        {
            if (header->samprate_mult > 0)
            {
                samprate = -1.0 * (double)header->samprate_mult / (double)header->samprate_fact;
            }
            else if (header->samprate_mult < 0)
            {
                samprate = 1.0 / ((double)header->samprate_fact * (double)header->samprate_mult);
            }
        }
    }

    // 计算日期
    uint8_t month, day;
    day_to_month_day(header->year, header->day, &month, &day);

    // 获取当前时间
    char timebuf[64]; // 增大缓冲区大小
    time_t rawtime;
    struct tm *timeinfo;

    time(&rawtime);
    timeinfo = localtime(&rawtime);
    if (!timeinfo)
    { // 添加错误检查
        printf("错误：获取本地时间失败\n");
        return;
    }

    if (strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", timeinfo) == 0)
    { // 添加错误检查
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

// 写入FSDH头部
void write_mseed_fsdh(FILE *fp, const MS2FSDH *header)
{
    if (!fp || !header) return;

    // 写入固定长度字段
    fwrite(header->sequence_number, 1, 6, fp);
    fwrite(&header->dataquality, 1, 1, fp);
    fwrite(&header->reserved, 1, 1, fp);
    fwrite(header->station, 1, 5, fp);
    fwrite(header->location, 1, 2, fp);
    fwrite(header->channel, 1, 3, fp);
    fwrite(header->network, 1, 2, fp);
    
    // 所有数值字段使用大端序写入
    write_uint16_be(fp, header->year);
    write_uint16_be(fp, header->day);
    fwrite(&header->hour, 1, 1, fp);
    fwrite(&header->min, 1, 1, fp);
    fwrite(&header->sec, 1, 1, fp);
    fwrite(&header->unused, 1, 1, fp);
    write_uint16_be(fp, header->fract);
    write_uint16_be(fp, header->numsamples);
    write_uint16_be(fp, header->samprate_fact);
    write_uint16_be(fp, header->samprate_mult);
    
    fwrite(&header->act_flags, 1, 1, fp);
    fwrite(&header->io_flags, 1, 1, fp);
    fwrite(&header->dq_flags, 1, 1, fp);
    fwrite(&header->numblockettes, 1, 1, fp);
    
    write_uint32_be(fp, header->time_correct);
    write_uint16_be(fp, header->data_offset);
    write_uint16_be(fp, header->blockette_offset);
}

// 计算所需的记录长度（返回2的幂指数）
int calculate_record_length(int numsamples) {
    // 计算数据部分需要的字节数（Steim2压缩后）
    // 每个帧64字节，每帧最多压缩约7个样本
    int num_frames = (numsamples + 6) / 7;  // 向上取整
    int data_bytes = num_frames * 64;
    
    // 加上头部(48字节)和两个blockette(各8字节)
    int total_bytes = 48 + 8 + 8 + data_bytes;
    
    // 找到大于等于total_bytes的最小2的幂
    int reclen = 8;  // 最小是2^8 = 256字节
    while ((1 << reclen) < total_bytes && reclen <= 20) {
        reclen++;
    }
    
    return reclen;
}

// 计算frame_count
int calculate_frame_count(int numsamples) {
    return (numsamples + 6) / 7;  // 向上取整，每帧最多压缩7个样本
}

// 计算压缩数据缓冲区大小（以32位字为单位）
int calculate_compressed_size(int numsamples) {
    int num_frames = calculate_frame_count(numsamples);
    return (num_frames * 64) / 4;  // 每帧64字节，转换为32位字数量
}