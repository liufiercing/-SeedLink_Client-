#include <stdio.h>
#include <stdlib.h>
#include "steim2.h"

// Steim2解压缩函数实现
int decode_steim2(const uint32_t *encoded_data, 
                 int num_samples,
                 int32_t *decoded_data,
                 int32_t *last_sample) {
    if (!encoded_data || !decoded_data || !last_sample) {
        return STEIM2_ERROR;
    }

    int sample_count = 0;
    int frame_count = 0;
    int32_t X0, Xn;  // 第一个样本和最后一个样本

    // 处理每一帧
    while (sample_count < num_samples) {
        const uint32_t *frame = encoded_data + (frame_count * 16);  // 每帧16个32位字
        uint32_t w0 = frame[0];  // 帧头
        
        // 从帧头获取编码格式
        X0 = (int32_t)frame[1];  // 帧中第一个样本
        Xn = (int32_t)frame[2];  // 帧中最后一个样本

        // 处理剩余的13个字（每个字可能包含多个差分值）
        for (int i = 3; i < 16 && sample_count < num_samples; i++) {
            uint32_t word = frame[i];
            uint32_t nibble = (w0 >> (30 - 2 * (i-3))) & 0x3;  // 获取编码类型

            switch (nibble) {
                case 0:  // 没有压缩
                    if (sample_count < num_samples) {
                        decoded_data[sample_count++] = (int32_t)word;
                    }
                    break;

                case 1:  // 4个8位差分
                    for (int j = 0; j < 4 && sample_count < num_samples; j++) {
                        int8_t diff = (word >> (24 - j * 8)) & 0xFF;
                        decoded_data[sample_count] = (sample_count > 0) ? 
                            decoded_data[sample_count-1] + diff : X0 + diff;
                        sample_count++;
                    }
                    break;

                case 2:  // 2个16位差分
                    for (int j = 0; j < 2 && sample_count < num_samples; j++) {
                        int16_t diff = (word >> (16 - j * 16)) & 0xFFFF;
                        decoded_data[sample_count] = (sample_count > 0) ? 
                            decoded_data[sample_count-1] + diff : X0 + diff;
                        sample_count++;
                    }
                    break;

                case 3:  // 1个30位差分
                    {
                        int32_t diff = (word & 0x3FFFFFFF);
                        // 处理符号扩展
                        if (diff & 0x20000000) {
                            diff |= 0xC0000000;
                        }
                        decoded_data[sample_count] = (sample_count > 0) ? 
                            decoded_data[sample_count-1] + diff : X0 + diff;
                        sample_count++;
                    }
                    break;
            }
        }
        frame_count++;
    }

    *last_sample = decoded_data[sample_count-1];
    return STEIM2_SUCCESS;
}

// 读取并解码Steim2压缩数据
int read_steim2_data(FILE *fp, int num_samples, int data_offset) {
    // 计算需要读取的帧数
    int frames_needed = (num_samples + STEIM2_SAMPLES_PER_FRAME - 1) / STEIM2_SAMPLES_PER_FRAME;
    int total_bytes = frames_needed * STEIM2_FRAME_SIZE;
    
    // 分配内存
    uint32_t *encoded_data = malloc(total_bytes);
    int32_t *decoded_data = malloc(num_samples * sizeof(int32_t));
    
    if (!encoded_data || !decoded_data) {
        free(encoded_data);
        free(decoded_data);
        return -1;
    }

    // 定位到数据开始位置
    fseek(fp, data_offset, SEEK_SET);
    
    // 读取压缩数据
    if (fread(encoded_data, 1, total_bytes, fp) != total_bytes) {
        free(encoded_data);
        free(decoded_data);
        return -1;
    }

    // 解码数据
    int32_t last_sample;
    if (decode_steim2(encoded_data, num_samples, decoded_data, &last_sample) == STEIM2_SUCCESS) {
        // 打印部分解码后的数据
        printf("\n解压缩后的波形数据 (前10个点):\n");
        for (int i = 0; i < 10 && i < num_samples; i++) {
            printf("%d ", decoded_data[i]);
        }
        printf("\n");
    }

    free(encoded_data);
    free(decoded_data);
    return 0;
} 