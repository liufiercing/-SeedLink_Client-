#include <stdio.h>
#include <stdlib.h>
#include "steim2.h"

// 添加字节序转换函数
static int32_t swap_int32(int32_t value) {
    return ((value & 0xFF000000) >> 24) |
           ((value & 0x00FF0000) >> 8) |
           ((value & 0x0000FF00) << 8) |
           ((value & 0x000000FF) << 24);
}

// 添加16位字节序转换
static int16_t swap_int16(int16_t value) {
    return ((value & 0xFF00) >> 8) |
           ((value & 0x00FF) << 8);
}

// Steim2解压缩函数实现
int decode_steim2(const uint32_t *encoded_data, 
                 int num_samples,
                 int32_t *decoded_data,
                 int32_t *last_sample) {
    if (!encoded_data || !decoded_data || !last_sample) {
        printf("错误：输入参数无效\n");
        return STEIM2_ERROR;
    }

    int sample_count = 0;
    int frame_count = 0;
    int32_t X0, Xn;  // 第一个样本和最后一个样本

    printf("\n=== Steim2 解压缩开始 ===\n");
    printf("预期样本数: %d\n", num_samples);

    // 处理每一帧
    while (sample_count < num_samples) {
        const uint32_t *frame = encoded_data + (frame_count * 16);
        uint32_t w0 = frame[0];  // 帧头
        
        printf("\n处理帧 %d:\n", frame_count);
        printf("帧头 w0: 0x%08X\n", w0);
        
        // 获取帧的起始和结束样本
        X0 = swap_int32(frame[1]);
        Xn = swap_int32(frame[2]);
        
        printf("帧起始样本 X0: %d\n", X0);
        printf("帧结束样本 Xn: %d\n", Xn);

        // 第一帧的第一个样本
        if (frame_count == 0 && sample_count == 0) {
            decoded_data[sample_count++] = X0;
        }

        // 处理剩余的13个字
        for (int i = 3; i < 16 && sample_count < num_samples; i++) {
            uint32_t word = frame[i];
            uint32_t nibble = (w0 >> (30 - 2 * (i-3))) & 0x3;

            printf("\n处理字 %d:\n", i);
            printf("原始数据: 0x%08X\n", word);
            printf("编码类型: %d\n", nibble);

            switch (nibble) {
                case 0:  // 未压缩的32位样本
                    if (sample_count < num_samples) {
                        decoded_data[sample_count] = swap_int32((int32_t)word);
                        printf("未压缩值: %d\n", decoded_data[sample_count]);
                        sample_count++;
                    }
                    break;

                case 1:  // 4个8位差分
                    for (int j = 0; j < 4 && sample_count < num_samples; j++) {
                        int8_t diff = (word >> (24 - j * 8)) & 0xFF;
                        decoded_data[sample_count] = (sample_count > 0) ? 
                            decoded_data[sample_count-1] + diff : X0 + diff;
                        printf("8位差分值[%d]: %d, 解码值: %d\n", 
                               j, diff, decoded_data[sample_count]);
                        sample_count++;
                    }
                    break;

                case 2:  // 2个16位差分
                    for (int j = 0; j < 2 && sample_count < num_samples; j++) {
                        // 提取16位差分值
                        int16_t diff;
                        if (j == 0) {
                            // 高16位: 0xFDC0
                            diff = (word >> 16) & 0xFFFF;
                        } else {
                            // 低16位: 0x53CF
                            diff = word & 0xFFFF;
                        }
                        
                        // 转换字节序
                        diff = ((diff >> 8) & 0xFF) | ((diff & 0xFF) << 8);
                        
                        // 如果是负数，需要符号扩展
                        if (diff & 0x8000) {
                            diff |= 0xFFFF0000;
                        }
                        
                        // 计算当前样本值
                        if (sample_count == 0) {
                            decoded_data[sample_count] = X0;
                        } else {
                            decoded_data[sample_count] = decoded_data[sample_count-1] + diff;
                        }
                            
                        printf("16位差分值[%d]: %d (0x%04X), 解码值: %d\n", 
                               j, diff, (uint16_t)diff, decoded_data[sample_count]);
                        sample_count++;
                    }
                    break;

                case 3:  // 30位差分
                    {
                        int32_t diff = word & 0x3FFFFFFF;
                        if (diff & 0x20000000) {
                            diff |= 0xC0000000;  // 符号扩展
                        }
                        decoded_data[sample_count] = (sample_count > 0) ? 
                            decoded_data[sample_count-1] + diff : X0 + diff;
                        printf("30位差分值: %d, 解码值: %d\n", 
                               diff, decoded_data[sample_count]);
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
    printf("\n=== 开始读取压缩数据 ===\n");
    
    // MiniSEED记录固定为512字节
    const int RECORD_SIZE = 512;
    // 数据部分大小 = 记录大小 - 数据偏移量
    int data_size = RECORD_SIZE - data_offset;
    
    printf("样本数: %d\n", num_samples);
    printf("记录大小: %d bytes\n", RECORD_SIZE);
    printf("数据偏移量: %d bytes\n", data_offset);
    printf("数据部分大小: %d bytes\n", data_size);
    
    // 分配内存
    uint32_t *encoded_data = malloc(data_size);
    int32_t *decoded_data = malloc(num_samples * sizeof(int32_t));
    
    if (!encoded_data || !decoded_data) {
        printf("错误：内存分配失败\n");
        free(encoded_data);
        free(decoded_data);
        return -1;
    }

    // 定位到数据开始位置（已经在data_offset位置）
    printf("开始读取压缩数据...\n");
    
    // 读取压缩数据
    size_t bytes_read = fread(encoded_data, 1, data_size, fp);
    printf("实际读取字节数: %zu\n", bytes_read);
    
    if (bytes_read != data_size) {
        printf("错误：读取数据不完整（预期:%d, 实际:%zu）\n", data_size, bytes_read);
        free(encoded_data);
        free(decoded_data);
        return -1;
    }

    // 解码数据
    int32_t last_sample;
    if (decode_steim2(encoded_data, num_samples, decoded_data, &last_sample) == STEIM2_SUCCESS) {
        printf("\n解压缩后的波形数据 (前20个点):\n");
        for (int i = 0; i < 20 && i < num_samples; i++) {
            printf("[%d] %d\n", i, decoded_data[i]);
        }
    } else {
        printf("错误：解压缩失败\n");
    }

    free(encoded_data);
    free(decoded_data);
    return 0;
} 