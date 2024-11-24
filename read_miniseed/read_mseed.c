#include <stdio.h>
#include <stdlib.h>
#include "mseed_header.h"
#include "steim2.h"

int main()
{
    FILE *fp;
    MS2FSDH header;
    unsigned char buffer[MS2FSDH_LENGTH];

    // 打开文件
    fp = fopen("II_BFO_00_BHE1.mseed", "rb");
    if (!fp) {
        printf("错误：无法打开文件\n");
        return 1;
    }

    // 读取固定数据头部
    if (fread(buffer, 1, MS2FSDH_LENGTH, fp) != MS2FSDH_LENGTH) {
        printf("错误：读取文件头失败\n");
        fclose(fp);
        return 1;
    }

    // 解析头部
    if (parse_mseed_header(buffer, &header) != 0) {
        printf("错误：解析头部失败\n");
        fclose(fp);
        return 1;
    }

    // 打印头部信息
    print_mseed_header(&header);

    // 读取数据部分
    if (header.data_offset > MS2FSDH_LENGTH)
    {
        // 跳过中间的blockettes
        fseek(fp, header.data_offset - MS2FSDH_LENGTH, SEEK_CUR);
    }

    // 使用Steim2解码函数读取数据
    read_steim2_data(fp, header.numsamples, header.data_offset);

    fclose(fp);
    return 0;
}