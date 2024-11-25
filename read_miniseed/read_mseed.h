#ifndef __READ_MSEED__
#define __READ_MSEED__
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mseed_header.h"

// 写入MAT文件的函数
void write_mat_file(const char *filename, const int32_t *data, int length) ;
// 获取当前时间字符串的函数声明
char* get_current_time(void);

// 解析单个512字节的MSEED包
int process_mseed_record(const unsigned char *record_start, 
                        int32_t **decoded_data, 
                        int64_t *samples_decoded,
                        MS2FSDH *header);

#endif // __READ_MSEED__ 