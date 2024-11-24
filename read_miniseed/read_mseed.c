

#include "mseed_header.h"
#include "steim2.h"
#include "read_mseed.h"






// 写入MAT文件的函数
void write_mat_file(const char *filename, const int32_t *data, int length) {
    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        printf("[%s] 错误：无法创建MAT文件\n", get_current_time());
        return;
    }

    fwrite(data, sizeof(int32_t), length, fp);
    fclose(fp);
    printf("[%s] 数据已写入: %s\n", get_current_time(), filename);
}

// 获取当前时间字符串的实现
char* get_current_time(void) {
    static char buffer[32];
    time_t rawtime;
    struct tm *timeinfo;

    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
    return buffer;
}

// 解析单个512字节的MSEED包
int process_mseed_record(const unsigned char *record_start, 
                        int32_t **decoded_data, 
                        int64_t *samples_decoded,
                        MS2FSDH *header) {
    const int RECORD_SIZE = 512;
    
    // 解析头部
    if (parse_mseed_header(record_start, header) != 0) {
        printf("[%s] 错误：解析头部失败\n", get_current_time());
        return -1;
    }

    // 打印头部信息
    // print_mseed_header(header);

    // 分配解码数据缓冲区
    *decoded_data = malloc(header->numsamples * sizeof(int32_t));
    if (!*decoded_data) {
        printf("[%s] 错误：内存分配失败\n", get_current_time());
        return -1;
    }

    // 计算压缩数据的位置和大小
    int data_size = RECORD_SIZE - header->data_offset;
    const int32_t *encoded_data = (const int32_t *)(record_start + header->data_offset);

    // 解压缩数据
    *samples_decoded = msr_decode_steim2(
        (int32_t *)encoded_data,
        data_size,
        header->numsamples,
        *decoded_data,
        header->numsamples * sizeof(int32_t),
        "record",
        1
    );

    if (*samples_decoded < 0) {
        printf("[%s] 错误：解压缩失败\n", get_current_time());
        free(*decoded_data);
        *decoded_data = NULL;
        return -1;
    }

    return 0;
}
