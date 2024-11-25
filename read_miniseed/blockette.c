#include <stdio.h>
#include <string.h>
#include "blockette.h"
#include "read_mseed.h"

// 大端字节序转换函数
static uint16_t swap_uint16(uint16_t val) {
    return (val << 8) | (val >> 8);
}

static float swap_float(float val) {
    union {
        float f;
        uint32_t u;
    } src, dst;

    src.f = val;
    dst.u = ((src.u & 0xFF000000) >> 24) |
            ((src.u & 0x00FF0000) >> 8) |
            ((src.u & 0x0000FF00) << 8) |
            ((src.u & 0x000000FF) << 24);
    return dst.f;
}

static uint32_t swap_uint32(uint32_t val) {
    return ((val & 0xFF000000) >> 24) |
           ((val & 0x00FF0000) >> 8) |
           ((val & 0x0000FF00) << 8) |
           ((val & 0x000000FF) << 24);
}

// 解析Blockette
int parse_blockette(const unsigned char *blockette_start, void *blockette_data) {
    uint16_t type;
    
    // 获取Blockette类型（大端）
    memcpy(&type, blockette_start, 2);
    type = swap_uint16(type);
    
    switch(type) {
        case 100: {
            MS2Blockette100 *b100 = (MS2Blockette100 *)blockette_data;
            
            // 复制并转换字节序
            memcpy(&b100->type, blockette_start, 2);
            b100->type = swap_uint16(b100->type);
            
            memcpy(&b100->next_offset, blockette_start + 2, 2);
            b100->next_offset = swap_uint16(b100->next_offset);
            
            memcpy(&b100->samprate, blockette_start + 4, 4);
            b100->samprate = swap_float(b100->samprate);
            
            memcpy(&b100->flags, blockette_start + 8, 1);
            memcpy(b100->reserved, blockette_start + 9, 3);
            return sizeof(MS2Blockette100);
        }
        
        case 1000: {
            MS2Blockette1000 *b1000 = (MS2Blockette1000 *)blockette_data;
            
            memcpy(&b1000->type, blockette_start, 2);
            b1000->type = swap_uint16(b1000->type);
            
            memcpy(&b1000->next_offset, blockette_start + 2, 2);
            b1000->next_offset = swap_uint16(b1000->next_offset);
            
            memcpy(&b1000->encoding, blockette_start + 4, 1);
            memcpy(&b1000->byteorder, blockette_start + 5, 1);
            memcpy(&b1000->reclen, blockette_start + 6, 1);
            memcpy(&b1000->reserved, blockette_start + 7, 1);
            return sizeof(MS2Blockette1000);
        }
        
        case 1001: {
            MS2Blockette1001 *b1001 = (MS2Blockette1001 *)blockette_data;
            
            memcpy(&b1001->type, blockette_start, 2);
            b1001->type = swap_uint16(b1001->type);
            
            memcpy(&b1001->next_offset, blockette_start + 2, 2);
            b1001->next_offset = swap_uint16(b1001->next_offset);
            
            memcpy(&b1001->timing_quality, blockette_start + 4, 1);
            memcpy(&b1001->microsecond, blockette_start + 5, 1);
            memcpy(&b1001->reserved, blockette_start + 6, 1);
            memcpy(&b1001->frame_count, blockette_start + 7, 1);
            return sizeof(MS2Blockette1001);
        }
        
        case 200: {
            MS2Blockette200 *b200 = (MS2Blockette200 *)blockette_data;
            
            memcpy(&b200->type, blockette_start, 2);
            b200->type = swap_uint16(b200->type);
            
            memcpy(&b200->next_offset, blockette_start + 2, 2);
            b200->next_offset = swap_uint16(b200->next_offset);
            
            memcpy(&b200->amplitude, blockette_start + 4, 4);
            b200->amplitude = swap_float(b200->amplitude);
            
            memcpy(&b200->period, blockette_start + 8, 4);
            b200->period = swap_float(b200->period);
            
            memcpy(&b200->background_est, blockette_start + 12, 4);
            b200->background_est = swap_float(b200->background_est);
            
            memcpy(&b200->flags, blockette_start + 16, 1);
            memcpy(&b200->reserved, blockette_start + 17, 1);
            
            memcpy(&b200->year, blockette_start + 18, 2);
            b200->year = swap_uint16(b200->year);
            
            memcpy(&b200->day, blockette_start + 20, 2);
            b200->day = swap_uint16(b200->day);
            
            memcpy(&b200->hour, blockette_start + 22, 1);
            memcpy(&b200->min, blockette_start + 23, 1);
            memcpy(&b200->sec, blockette_start + 24, 1);
            memcpy(&b200->unused, blockette_start + 25, 1);
            
            memcpy(&b200->fract, blockette_start + 26, 2);
            b200->fract = swap_uint16(b200->fract);
            
            memcpy(b200->detector, blockette_start + 28, 24);
            return sizeof(MS2Blockette200);
        }
        
        case 201: {
            MS2Blockette201 *b201 = (MS2Blockette201 *)blockette_data;
            
            memcpy(&b201->type, blockette_start, 2);
            b201->type = swap_uint16(b201->type);
            
            memcpy(&b201->next_offset, blockette_start + 2, 2);
            b201->next_offset = swap_uint16(b201->next_offset);
            
            memcpy(&b201->amplitude, blockette_start + 4, 4);
            b201->amplitude = swap_float(b201->amplitude);
            
            memcpy(&b201->period, blockette_start + 8, 4);
            b201->period = swap_float(b201->period);
            
            memcpy(&b201->background_est, blockette_start + 12, 4);
            b201->background_est = swap_float(b201->background_est);
            
            memcpy(&b201->flags, blockette_start + 16, 1);
            memcpy(&b201->reserved, blockette_start + 17, 1);
            
            memcpy(&b201->year, blockette_start + 18, 2);
            b201->year = swap_uint16(b201->year);
            
            memcpy(&b201->day, blockette_start + 20, 2);
            b201->day = swap_uint16(b201->day);
            
            memcpy(&b201->hour, blockette_start + 22, 1);
            memcpy(&b201->min, blockette_start + 23, 1);
            memcpy(&b201->sec, blockette_start + 24, 1);
            memcpy(&b201->unused, blockette_start + 25, 1);
            
            memcpy(&b201->fract, blockette_start + 26, 2);
            b201->fract = swap_uint16(b201->fract);
            
            memcpy(b201->snr_values, blockette_start + 28, 6);
            memcpy(&b201->loopback, blockette_start + 34, 1);
            memcpy(&b201->pick_algorithm, blockette_start + 35, 1);
            memcpy(b201->detector, blockette_start + 36, 24);
            return sizeof(MS2Blockette201);
        }
        
        case 300: {
            MS2Blockette300 *b300 = (MS2Blockette300 *)blockette_data;
            
            memcpy(&b300->type, blockette_start, 2);
            b300->type = swap_uint16(b300->type);
            
            memcpy(&b300->next_offset, blockette_start + 2, 2);
            b300->next_offset = swap_uint16(b300->next_offset);
            
            memcpy(&b300->year, blockette_start + 4, 2);
            b300->year = swap_uint16(b300->year);
            
            memcpy(&b300->day, blockette_start + 6, 2);
            b300->day = swap_uint16(b300->day);
            
            memcpy(&b300->hour, blockette_start + 8, 1);
            memcpy(&b300->min, blockette_start + 9, 1);
            memcpy(&b300->sec, blockette_start + 10, 1);
            memcpy(&b300->unused, blockette_start + 11, 1);
            
            memcpy(&b300->fract, blockette_start + 12, 2);
            b300->fract = swap_uint16(b300->fract);
            
            memcpy(&b300->num_calibrations, blockette_start + 14, 1);
            memcpy(&b300->flags, blockette_start + 15, 1);
            
            memcpy(&b300->step_duration, blockette_start + 16, 4);
            b300->step_duration = swap_uint32(b300->step_duration);
            
            memcpy(&b300->interval_duration, blockette_start + 20, 4);
            b300->interval_duration = swap_uint32(b300->interval_duration);
            
            memcpy(&b300->amplitude, blockette_start + 24, 4);
            b300->amplitude = swap_float(b300->amplitude);
            
            memcpy(b300->input_channel, blockette_start + 28, 3);
            memcpy(&b300->reserved, blockette_start + 31, 1);
            
            memcpy(&b300->reference_amplitude, blockette_start + 32, 4);
            b300->reference_amplitude = swap_uint32(b300->reference_amplitude);
            
            memcpy(b300->coupling, blockette_start + 36, 12);
            memcpy(b300->rolloff, blockette_start + 48, 12);
            return sizeof(MS2Blockette300);
        }
        
        default:
            printf("[%s] 警告：未知的Blockette类型 %d\n", get_current_time(), type);
            return -1;
    }
}

// 打印Blockette信息
void print_blockette(const void *blockette_data, uint16_t type) {
    switch(type) {
        case 100: {
            const MS2Blockette100 *b100 = (const MS2Blockette100 *)blockette_data;
            printf("[%s] Blockette 100 | 采样率: %.2f Hz | 标志: 0x%02X\n",
                   get_current_time(), b100->samprate, b100->flags);
            break;
        }
        
        case 1000: {
            const MS2Blockette1000 *b1000 = (const MS2Blockette1000 *)blockette_data;
            printf("[%s] Blockette 1000 | 编码格式: %d | 字节序: %d | 记录长度: %d字节 (2^%d)\n",
                   get_current_time(), 
                   b1000->encoding, 
                   b1000->byteorder, 
                   1 << b1000->reclen,  // 计算实际字节数
                   b1000->reclen);
            break;
        }
        
        case 1001: {
            const MS2Blockette1001 *b1001 = (const MS2Blockette1001 *)blockette_data;
            printf("[%s] Blockette 1001 | 计时质量: %d%% | 微秒偏移: %d μs | 帧数: %d\n",
                   get_current_time(), 
                   b1001->timing_quality, 
                   b1001->microsecond,  // 微秒偏移值，范围通常是-50到+49，或0到+99
                   b1001->frame_count);
            break;
        }
        
        case 200: {
            const MS2Blockette200 *b200 = (const MS2Blockette200 *)blockette_data;
            printf("[%s] Blockette 200 | 振幅: %.2f | 周期: %.2f | 背景: %.2f | "
                   "时间: %d-%03d %02d:%02d:%02d.%d | 检测器: %.24s\n",
                   get_current_time(), b200->amplitude, b200->period, b200->background_est,
                   b200->year, b200->day, b200->hour, b200->min, b200->sec, b200->fract,
                   b200->detector);
            break;
        }
        
        case 201: {
            const MS2Blockette201 *b201 = (const MS2Blockette201 *)blockette_data;
            printf("[%s] Blockette 201 | 振幅: %.2f | 周期: %.2f | 背景: %.2f | "
                   "时间: %d-%03d %02d:%02d:%02d.%d | SNR[0]: %d | 检测器: %.24s\n",
                   get_current_time(), b201->amplitude, b201->period, b201->background_est,
                   b201->year, b201->day, b201->hour, b201->min, b201->sec, b201->fract,
                   b201->snr_values[0], b201->detector);
            break;
        }
        
        case 300: {
            const MS2Blockette300 *b300 = (const MS2Blockette300 *)blockette_data;
            printf("[%s] Blockette 300 | 时间: %d-%03d %02d:%02d:%02d.%d | "
                   "校准次数: %d | 振幅: %.2f | 输入通道: %.3s | 参考振幅: %u\n",
                   get_current_time(), b300->year, b300->day, b300->hour, b300->min, 
                   b300->sec, b300->fract, b300->num_calibrations, b300->amplitude,
                   b300->input_channel, b300->reference_amplitude);
            break;
        }
        
        default:
            printf("[%s] 警告：未知的Blockette类型 %d\n", get_current_time(), type);
    }
} 

int process_blockettes(const unsigned char *record_start, uint16_t first_blockette_offset, uint8_t num_blockettes) {
    if (num_blockettes == 0) return 0;
    
    uint16_t next_blockette = first_blockette_offset;
    
    for (int i = 0; i < num_blockettes; i++) {
        if (next_blockette == 0) break;
        
        // 获取Blockette类型
        uint16_t blockette_type;
        memcpy(&blockette_type, record_start + next_blockette, 2);
        blockette_type = (blockette_type >> 8) | (blockette_type << 8); // 转换字节序
        
        // 分配内存并解析Blockette
        void *blockette_data = NULL;
        switch(blockette_type) {
            case 100:
                blockette_data = malloc(sizeof(MS2Blockette100));
                break;
            case 200:
                blockette_data = malloc(sizeof(MS2Blockette200));
                break;
            case 201:
                blockette_data = malloc(sizeof(MS2Blockette201));
                break;
            case 300:
                blockette_data = malloc(sizeof(MS2Blockette300));
                break;
            case 1000:
                blockette_data = malloc(sizeof(MS2Blockette1000));
                break;
            case 1001:
                blockette_data = malloc(sizeof(MS2Blockette1001));
                break;
            default:
                printf("[%s] 警告：跳过未知的Blockette类型 %d\n", 
                       get_current_time(), blockette_type);
                continue;
        }
        
        if (!blockette_data) {
            printf("[%s] 错误：Blockette内存分配失败\n", get_current_time());
            return -1;
        }
        
        // 解析Blockette
        int blockette_size = parse_blockette(record_start + next_blockette, 
                                           blockette_data);
        if (blockette_size < 0) {
            free(blockette_data);
            continue;
        }
        
        // 打印Blockette信息
        print_blockette(blockette_data, blockette_type);
        
        // 获取下一个Blockette的偏移量
        uint16_t next_offset;
        memcpy(&next_offset, record_start + next_blockette + 2, 2);
        next_offset = (next_offset >> 8) | (next_offset << 8); // 转换字节序
        next_blockette = next_offset;
        
        free(blockette_data);
    }
    
    return 0;
}