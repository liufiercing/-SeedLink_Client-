#ifndef STEIM2_H
#define STEIM2_H

#include <stdint.h>
#include <stdio.h>

// Steim2压缩帧结构
typedef struct {
    uint32_t w[16];     // 每帧16个32位字
} Steim2Frame;

// 压缩函数声明
int64_t msr_encode_steim2(int32_t *input, uint64_t samplecount, int32_t *output,
                         uint64_t outputlength, int32_t diff0, uint32_t *byteswritten,
                         const char *sid, int swapflag);

// 写入压缩数据到文件
void write_steim2_data(FILE *fp, const int32_t *compressed, uint32_t length);

// 添加BITWIDTH宏定义
#define BITWIDTH(VALUE, RESULT)                       \
  if (VALUE >= -8 && VALUE <= 7)                      \
    RESULT = 4;                                       \
  else if (VALUE >= -16 && VALUE <= 15)               \
    RESULT = 5;                                       \
  else if (VALUE >= -32 && VALUE <= 31)               \
    RESULT = 6;                                       \
  else if (VALUE >= -128 && VALUE <= 127)             \
    RESULT = 8;                                       \
  else if (VALUE >= -512 && VALUE <= 511)             \
    RESULT = 10;                                      \
  else if (VALUE >= -16384 && VALUE <= 16383)         \
    RESULT = 15;                                      \
  else if (VALUE >= -32768 && VALUE <= 32767)         \
    RESULT = 16;                                      \
  else if (VALUE >= -536870912 && VALUE <= 536870911) \
    RESULT = 30;                                      \
  else                                                \
    RESULT = 32;

// 添加字节访问联合体
union dword {
    int8_t d8[4];
    int16_t d16[2];
    int32_t d32;
};

#endif // STEIM2_H 