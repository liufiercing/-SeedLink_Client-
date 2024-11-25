#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdint.h>

// 大端序写入函数声明
void write_uint16_be(FILE *fp, uint16_t value);
void write_uint32_be(FILE *fp, uint32_t value);

// 获取当前时间字符串
char* get_current_time(void);

// 字节序转换函数声明
uint16_t swap_uint16(uint16_t val);
uint32_t swap_uint32(uint32_t val);
float swap_float(float val);

// 修改日志函数声明
void ms_log(int level, const char *format, ...);

// 添加字节序转换函数声明
void ms_gswap4(void *data);

// 新增函数声明
// 十六进制转储函数
void hex_dump(const char* filename);
// 比较两个文件
int compare_files(const char* file1, const char* file2);
// 分析miniSEED文件头
void analyze_mseed_file(const char* filename);

#endif // UTILS_H 