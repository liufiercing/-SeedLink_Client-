#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "mseed_header.h"
#include "steim2.h"
#include "read_mseed.h"

int main(int argc, char *argv[])
{
    const char *mseed_file;
    
    if (argc != 2) {
        mseed_file = "II_BFO_00_BHE.mseed";
        printf("[%s] 使用默认文件: %s\n", get_current_time(), mseed_file);
    } else {
        mseed_file = argv[1];
    }

    FILE *fp = fopen(mseed_file, "rb");
    if (!fp) {
        printf("[%s] 错误：无法打开文件 %s\n", get_current_time(), mseed_file);
        return 1;
    }

    // 获取文件大小
    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    // 计算记录数
    int num_records = file_size / 512;
    printf("[%s] 文件大小: %ld 字节, 包含 %d 个记录\n", 
           get_current_time(), file_size, num_records);

    // 读取整个文件内容
    unsigned char *file_content = malloc(file_size);
    if (!file_content) {
        printf("[%s] 错误：内存分配失败\n", get_current_time());
        fclose(fp);
        return 1;
    }

    if (fread(file_content, 1, file_size, fp) != file_size) {
        printf("[%s] 错误：读取文件失败\n", get_current_time());
        free(file_content);
        fclose(fp);
        return 1;
    }

    // 用于累积所有解码数据
    int32_t *all_samples = NULL;
    int64_t total_samples = 0;

    // 处理每个记录
    for (int i = 0; i < num_records; i++) {
        // printf("[%s] 处理记录 %d/%d\n", get_current_time(), i+1, num_records);
        
        int32_t *record_data = NULL;
        int64_t record_samples = 0;
        MS2FSDH header;

        // 处理单个记录
        if (process_mseed_record(file_content + i * 512, 
                               &record_data, 
                               &record_samples,
                               &header) != 0) {
            printf("[%s] 错误：处理记录 %d 失败\n", get_current_time(), i+1);
            free(file_content);
            free(all_samples);
            fclose(fp);
            return 1;
        }
        // print_mseed_header(&header) ;

        // 扩展累积缓冲区
        int32_t *new_all_samples = realloc(all_samples, (total_samples + record_samples) * sizeof(int32_t));
        if (!new_all_samples) {
            printf("[%s] 错误：内存重分配失败\n", get_current_time());
            free(record_data);
            free(file_content);
            free(all_samples);
            fclose(fp);
            return 1;
        }
        all_samples = new_all_samples;

        // 复制新解码的数据
        memcpy(all_samples + total_samples, record_data, record_samples * sizeof(int32_t));
        total_samples += record_samples;

        // 释放单个记录的数据
        free(record_data);
    }

    printf("[%s] 解压缩完成，共 %ld 个采样点\n", get_current_time(), total_samples);

    // 写入MAT文件
    write_mat_file("out.mat", all_samples, total_samples);

    // 清理资源
    free(file_content);
    free(all_samples);
    fclose(fp);

    printf("[%s] 处理完成\n", get_current_time());
    return 0;
}