#ifndef STEIM2_H
#define STEIM2_H

#include <stdint.h>
#include <stdio.h>

// Steim2解压缩的返回状态码
#define STEIM2_SUCCESS 0
#define STEIM2_ERROR -1

// Steim2压缩格式中的一些常量
#define STEIM2_FRAME_SIZE 64           // Steim2帧大小（字节）
#define STEIM2_SAMPLES_PER_FRAME 15    // 每帧中的样本数（不包括帧头）

// 从word中提取指定位范围的值的宏
#define EXTRACTBITRANGE(VALUE, STARTBIT, LENGTH) \
    (((VALUE) >> (STARTBIT)) & ((1U << (LENGTH)) - 1))

// Steim2解压缩函数声明
int64_t msr_decode_steim2(int32_t *input,          // 输入数据
                     uint64_t inputlength,      // 输入长度（字节）
                     uint64_t samplecount,      // 样本数
                     int32_t *output,           // 输出缓冲区
                     uint64_t outputlength,     // 输出缓冲区长度（字节）
                     const char *srcname,       // 源文件名（用于日志）
                     int swapflag);             // 字节序标志




#endif // STEIM2_H 