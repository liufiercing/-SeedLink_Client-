#include <stdio.h>
#include <time.h>
#include "utils.h"
#include <stdarg.h>

// 大端序写入16位整数
void write_uint16_be(FILE *fp, uint16_t value) {
    unsigned char bytes[2];
    bytes[0] = (value >> 8) & 0xFF;
    bytes[1] = value & 0xFF;
    fwrite(bytes, 1, 2, fp);
}

// 大端序写入32位整数
void write_uint32_be(FILE *fp, uint32_t value) {
    unsigned char bytes[4];
    bytes[0] = (value >> 24) & 0xFF;
    bytes[1] = (value >> 16) & 0xFF;
    bytes[2] = (value >> 8) & 0xFF;
    bytes[3] = value & 0xFF;
    fwrite(bytes, 1, 4, fp);
}

// 获取当前时间字符串
char* get_current_time(void) {
    static char buffer[32];
    time_t rawtime;
    struct tm *timeinfo;

    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
    return buffer;
}

// 16位整数字节序转换
uint16_t swap_uint16(uint16_t val) {
    return (val << 8) | (val >> 8);
}

// 32位整数字节序转换
uint32_t swap_uint32(uint32_t val) {
    return ((val & 0xFF000000) >> 24) |
           ((val & 0x00FF0000) >> 8) |
           ((val & 0x0000FF00) << 8) |
           ((val & 0x000000FF) << 24);
}

// 浮点数字节序转换
float swap_float(float val) {
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

// 简化日志函数实现
void ms_log(int level, const char *format, ...) {
    va_list args;
    va_start(args, format);
    if (level >= 2) {  // 只输出错误级别的日志
        fprintf(stderr, "ERROR: ");
        vfprintf(stderr, format, args);
    }
    va_end(args);
}

void ms_gswap4(void *data) {
    uint32_t *data32 = (uint32_t *)data;
    *data32 = swap_uint32(*data32);
}

// void hex_dump(const char* filename) {
//     FILE *fp = fopen(filename, "rb");
//     if (!fp) {
//         printf("无法打开文件 %s\n", filename);
//         return;
//     }

//     unsigned char buffer[16];
//     size_t bytes_read;
//     long offset = 0;

//     printf("\n文件 %s 的十六进制转储:\n", filename);
//     printf("偏移量   字节内容                              ASCII\n");
//     printf("-------- ---------------------------------------- ----------------\n");

//     while ((bytes_read = fread(buffer, 1, 16, fp)) > 0) {
//         // 打印偏移量
//         printf("%08lx  ", offset);

//         // 打印十六进制值
//         for (size_t i = 0; i < 16; i++) {
//             if (i < bytes_read)
//                 printf("%02x ", buffer[i]);
//             else
//                 printf("   ");
//             if (i == 7)
//                 printf(" ");
//         }

//         // 打印ASCII值
//         printf(" |");
//         for (size_t i = 0; i < bytes_read; i++) {
//             printf("%c", (buffer[i] >= 32 && buffer[i] <= 126) ? buffer[i] : '.');
//         }
//         printf("|\n");

//         offset += 16;
//     }

//     printf("-------- ---------------------------------------- ----------------\n");
//     fclose(fp);
// }

// int compare_files(const char* file1, const char* file2) {
//     FILE *fp1 = fopen(file1, "rb");
//     FILE *fp2 = fopen(file2, "rb");
    
//     if (!fp1 || !fp2) {
//         if (fp1) fclose(fp1);
//         if (fp2) fclose(fp2);
//         return -1;
//     }

//     int result = 0;
//     long offset = 0;
//     unsigned char buf1[1024], buf2[1024];
//     size_t bytes_read1, bytes_read2;

//     printf("\n比较文件 %s 和 %s:\n", file1, file2);

//     while (1) {
//         bytes_read1 = fread(buf1, 1, sizeof(buf1), fp1);
//         bytes_read2 = fread(buf2, 1, sizeof(buf2), fp2);

//         if (bytes_read1 != bytes_read2) {
//             printf("文件大小不同 在偏移量 %ld\n", offset);
//             result = 1;
//             break;
//         }

//         if (bytes_read1 == 0) break;

//         for (size_t i = 0; i < bytes_read1; i++) {
//             if (buf1[i] != buf2[i]) {
//                 printf("差异在偏移量 %ld: %02x != %02x\n", 
//                        offset + i, buf1[i], buf2[i]);
//                 result = 1;
//             }
//         }

//         offset += bytes_read1;
//     }

//     fclose(fp1);
//     fclose(fp2);
//     return result;
// }

// void analyze_mseed_file(const char* filename) {
//     FILE *fp = fopen(filename, "rb");
//     if (!fp) {
//         printf("无法打开文件 %s\n", filename);
//         return;
//     }

//     // 读取并分析固定数据头部
//     MS2FSDH header;
//     unsigned char buffer[48];
    
//     if (fread(buffer, 1, 48, fp) != 48) {
//         printf("读取头部失败\n");
//         fclose(fp);
//         return;
//     }

//     // 解析头部
//     parse_mseed_header(buffer, &header);
    
//     printf("\n分析文件 %s:\n", filename);
//     print_mseed_header(&header);

//     // 读取并分析blockettes
//     if (header.numblockettes > 0) {
//         fseek(fp, header.blockette_offset, SEEK_SET);
//         process_blockettes(buffer, header.blockette_offset, header.numblockettes);
//     }

//     fclose(fp);
// } 