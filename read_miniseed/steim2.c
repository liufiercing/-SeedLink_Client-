#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "steim2.h"

/** In-place byte swapping of 4 byte quantity */
static inline void ms_gswap4(void *data4) {
    uint32_t dat;
    memcpy(&dat, data4, 4);
    
    dat = ((dat & 0xff000000) >> 24) | ((dat & 0x000000ff) << 24) |
          ((dat & 0x00ff0000) >> 8)  | ((dat & 0x0000ff00) << 8);
    
    memcpy(data4, &dat, 4);
}

int64_t msr_decode_steim2(int32_t *input,
                         uint64_t inputlength,
                         uint64_t samplecount,
                         int32_t *output,
                         uint64_t outputlength,
                         const char *srcname,
                         int swapflag) {
    
    uint32_t frame[16];
    int32_t diff[105];
    int32_t Xn = 0;
    uint64_t outputidx;
    uint64_t maxframes = inputlength / 64;
    uint64_t frameidx;
    int diffidx;
    int startnibble;
    int nibble;
    int widx;
    int dnib;
    int idx;

    union dword {
        int8_t d8[4];
        int32_t d32;
    } *word;

    /* Bitfield specifications for sign extension */
    struct {signed int x:4;} s4;
    struct {signed int x:5;} s5;
    struct {signed int x:6;} s6;
    struct {signed int x:10;} s10;
    struct {signed int x:15;} s15;
    struct {signed int x:30;} s30;

    // 参数检查
    if (inputlength == 0) return 0;
    if (!input || !output || outputlength == 0) return -1;
    if (outputlength < (samplecount * sizeof(int32_t))) return -1;

    for (frameidx = 0, outputidx = 0;
         frameidx < maxframes && outputidx < samplecount;
         frameidx++) {
        memcpy(frame, input + (16 * frameidx), 64);
        diffidx = 0;

        // 处理第一帧
        if (frameidx == 0) {
            if (swapflag) {
                ms_gswap4(&frame[1]);
                ms_gswap4(&frame[2]);
            }
            output[0] = frame[1];
            outputidx++;
            Xn = frame[2];
            startnibble = 3;
        } else {
            startnibble = 1;
        }

        if (swapflag) {
            ms_gswap4(&frame[0]);
        }

        for (widx = startnibble; widx < 16; widx++) {
            nibble = EXTRACTBITRANGE(frame[0], (30 - (2 * widx)), 2);

            switch (nibble) {
                case 0:  // 未压缩的32位样本
                    if (swapflag) {
                        ms_gswap4(&frame[widx]);
                    }
                    diff[diffidx++] = frame[widx];
                    break;

                case 1:  // 4个8位差分
                    word = (union dword *)&frame[widx];
                    for (idx = 0; idx < 4; idx++) {
                        diff[diffidx++] = word->d8[idx];
                    }
                    break;

                case 2:  // 需要检查dnib
                    if (swapflag) {
                        ms_gswap4(&frame[widx]);
                    }
                    dnib = EXTRACTBITRANGE(frame[widx], 30, 2);

                    switch (dnib) {
                        case 0:
                            return -1;
                        case 1:  // 一个30位差分
                            diff[diffidx++] = (s30.x = EXTRACTBITRANGE(frame[widx], 0, 30));
                            break;
                        case 2:  // 两个15位差分
                            for (idx = 0; idx < 2; idx++) {
                                diff[diffidx++] = (s15.x = EXTRACTBITRANGE(frame[widx], (15 - idx * 15), 15));
                            }
                            break;
                        case 3:  // 三个10位差分
                            for (idx = 0; idx < 3; idx++) {
                                diff[diffidx++] = (s10.x = EXTRACTBITRANGE(frame[widx], (20 - idx * 10), 10));
                            }
                            break;
                    }
                    break;

                case 3:  // 需要检查dnib
                    if (swapflag) {
                        ms_gswap4(&frame[widx]);
                    }
                    dnib = EXTRACTBITRANGE(frame[widx], 30, 2);

                    switch (dnib) {
                        case 0:  // 五个6位差分
                            for (idx = 0; idx < 5; idx++) {
                                diff[diffidx++] = (s6.x = EXTRACTBITRANGE(frame[widx], (24 - idx * 6), 6));
                            }
                            break;
                        case 1:  // 六个5位差分
                            for (idx = 0; idx < 6; idx++) {
                                diff[diffidx++] = (s5.x = EXTRACTBITRANGE(frame[widx], (25 - idx * 5), 5));
                            }
                            break;
                        case 2:  // 七个4位差分
                            for (idx = 0; idx < 7; idx++) {
                                diff[diffidx++] = (s4.x = EXTRACTBITRANGE(frame[widx], (24 - idx * 4), 4));
                            }
                            break;
                        case 3:
                            return -1;
                    }
                    break;
            }
        }

        // 应用差分值
        for (idx = (frameidx == 0) ? 1 : 0;
             idx < diffidx && outputidx < samplecount;
             idx++, outputidx++) {
            output[outputidx] = output[outputidx-1] + diff[idx];
        }
    }

    // 数据完整性检查
    if (outputidx == samplecount && output[outputidx-1] != Xn) {
        printf("警告：数据完整性检查失败，最后样本=%d, Xn=%d\n",
               output[outputidx-1], Xn);
    }

    return outputidx;
}
