#ifndef STEIM2_H
#define STEIM2_H

#include <stdint.h>

// Steim2解压缩的返回状态码
#define STEIM2_SUCCESS 0
#define STEIM2_ERROR -1

// Steim2压缩格式中的一些常量
#define STEIM2_FRAME_SIZE 64           // Steim2帧大小（字节）
#define STEIM2_SAMPLES_PER_FRAME 15    // 每帧中的样本数（不包括帧头）

// Steim2解压缩函数声明
int decode_steim2(const uint32_t *encoded_data, 
                 int num_samples,
                 int32_t *decoded_data,
                 int32_t *last_sample);

// 读取并解码Steim2压缩数据
int read_steim2_data(FILE *fp, int num_samples, int data_offset);

#endif // STEIM2_H 