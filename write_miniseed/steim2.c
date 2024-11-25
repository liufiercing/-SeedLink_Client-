#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "steim2.h"
#include "utils.h"

// 计算一个值需要的位宽
static int __attribute__((unused)) get_bit_width(int32_t value) {
    if (value == 0) return 0;
    value = abs(value);
    int bits = 32;
    while (bits > 0 && !(value & (1u << (bits - 1)))) {
        bits--;
    }
    return bits + (value < 0 ? 1 : 0);
}

int64_t msr_encode_steim2(int32_t *input, uint64_t samplecount, int32_t *output,
                         uint64_t outputlength, int32_t diff0, uint32_t *byteswritten,
                         const char *sid, int swapflag) {
    uint32_t *frameptr;  /* Frame pointer in output */
  int32_t *Xnp = NULL; /* Reverse integration constant, aka last sample */
  int32_t diffs[7];
  int32_t bitwidth[7];
  uint64_t inputidx      = 0;
  uint64_t outputsamples = 0;
  uint64_t maxframes     = outputlength / 64;
  uint64_t frameidx;
  int diffcount     = 0;
  int packedsamples = 0;
  int startnibble;
  int widx;
  int idx;

  union dword
  {
    int8_t d8[4];
    int16_t d16[2];
    int32_t d32;
  } *word;

  if (samplecount == 0)
    return 0;

  if (!input || !output || outputlength == 0)
  {
    ms_log (2, "%s(): Required input not defined: 'input', 'output' or 'outputlength' == 0\n",
            __func__);
    return -1;
  }
  /* Add first difference to buffers */
  diffs[0] = diff0;
  BITWIDTH (diffs[0], bitwidth[0]);
  diffcount = 1;

  for (frameidx = 0; frameidx < maxframes && outputsamples < samplecount; frameidx++)
  {
    frameptr = (uint32_t *)output + (16 * frameidx);

    /* Set 64-byte frame to 0's */
    memset (frameptr, 0, 64);

    /* Save forward integration constant (X0), pointer to reverse integration constant (Xn)
     * and set the starting nibble index depending on frame. */
    if (frameidx == 0)
    {
      frameptr[1] = input[0];
      if (swapflag)
        ms_gswap4 (&frameptr[1]);

      Xnp = (int32_t *)&frameptr[2];

      startnibble = 3; /* First frame: skip nibbles, X0, and Xn */
    }
    else
    {
      startnibble = 1; /* Subsequent frames: skip nibbles */
    }

    for (widx = startnibble; widx < 16 && outputsamples < samplecount; widx++)
    {
      if (diffcount < 7)
      {
        /* Shift diffs and related bit widths to beginning of buffers */
        for (idx = 0; idx < diffcount; idx++)
        {
          diffs[idx]    = diffs[packedsamples + idx];
          bitwidth[idx] = bitwidth[packedsamples + idx];
        }

        /* Add new diffs and determine bit width needed to represent */
        for (idx = diffcount; idx < 7 && inputidx < (samplecount - 1); idx++, inputidx++)
        {
          diffs[idx] = *(input + inputidx + 1) - *(input + inputidx);
          BITWIDTH (diffs[idx], bitwidth[idx]);
          diffcount++;
        }
      }

      /* Determine optimal packing by checking, in-order:
       * 7 x 4-bit differences
       * 6 x 5-bit differences
       * 5 x 6-bit differences
       * 4 x 8-bit differences
       * 3 x 10-bit differences
       * 2 x 15-bit differences
       * 1 x 30-bit difference */

      packedsamples = 0;

      /* 7 x 4-bit differences */
      if (diffcount == 7 && bitwidth[0] <= 4 &&
          bitwidth[1] <= 4 && bitwidth[2] <= 4 && bitwidth[3] <= 4 &&
          bitwidth[4] <= 4 && bitwidth[5] <= 4 && bitwidth[6] <= 4)
      {

        /* Mask the values, shift to proper location and set in word */
        frameptr[widx] = ((uint32_t)diffs[6] & 0xFul);
        frameptr[widx] |= ((uint32_t)diffs[5] & 0xFul) << 4;
        frameptr[widx] |= ((uint32_t)diffs[4] & 0xFul) << 8;
        frameptr[widx] |= ((uint32_t)diffs[3] & 0xFul) << 12;
        frameptr[widx] |= ((uint32_t)diffs[2] & 0xFul) << 16;
        frameptr[widx] |= ((uint32_t)diffs[1] & 0xFul) << 20;
        frameptr[widx] |= ((uint32_t)diffs[0] & 0xFul) << 24;

        /* 2-bit decode nibble is 0b10 (0x2) */
        frameptr[widx] |= 0x2ul << 30;

        /* 2-bit nibble is 0b11 (0x3) */
        frameptr[0] |= 0x3ul << (30 - 2 * widx);

        packedsamples = 7;
      }
      /* 6 x 5-bit differences */
      else if (diffcount >= 6 &&
               bitwidth[0] <= 5 && bitwidth[1] <= 5 && bitwidth[2] <= 5 &&
               bitwidth[3] <= 5 && bitwidth[4] <= 5 && bitwidth[5] <= 5)
      {
        /* Mask the values, shift to proper location and set in word */
        frameptr[widx] = ((uint32_t)diffs[5] & 0x1Ful);
        frameptr[widx] |= ((uint32_t)diffs[4] & 0x1Ful) << 5;
        frameptr[widx] |= ((uint32_t)diffs[3] & 0x1Ful) << 10;
        frameptr[widx] |= ((uint32_t)diffs[2] & 0x1Ful) << 15;
        frameptr[widx] |= ((uint32_t)diffs[1] & 0x1Ful) << 20;
        frameptr[widx] |= ((uint32_t)diffs[0] & 0x1Ful) << 25;

        /* 2-bit decode nibble is 0b01 (0x1) */
        frameptr[widx] |= 0x1ul << 30;

        /* 2-bit nibble is 0b11 (0x3) */
        frameptr[0] |= 0x3ul << (30 - 2 * widx);

        packedsamples = 6;
      }
      /* 5 x 6-bit differences */
      else if (diffcount >= 5 &&
               bitwidth[0] <= 6 && bitwidth[1] <= 6 && bitwidth[2] <= 6 &&
               bitwidth[3] <= 6 && bitwidth[4] <= 6)
      {

        /* Mask the values, shift to proper location and set in word */
        frameptr[widx] = ((uint32_t)diffs[4] & 0x3Ful);
        frameptr[widx] |= ((uint32_t)diffs[3] & 0x3Ful) << 6;
        frameptr[widx] |= ((uint32_t)diffs[2] & 0x3Ful) << 12;
        frameptr[widx] |= ((uint32_t)diffs[1] & 0x3Ful) << 18;
        frameptr[widx] |= ((uint32_t)diffs[0] & 0x3Ful) << 24;

        /* 2-bit decode nibble is 0b00, nothing to set */

        /* 2-bit nibble is 0b11 (0x3) */
        frameptr[0] |= 0x3ul << (30 - 2 * widx);

        packedsamples = 5;
      }
      /* 4 x 8-bit differences */
      else if (diffcount >= 4 &&
               bitwidth[0] <= 8 && bitwidth[1] <= 8 &&
               bitwidth[2] <= 8 && bitwidth[3] <= 8)
      {

        word = (union dword *)&frameptr[widx];

        word->d8[0] = diffs[0];
        word->d8[1] = diffs[1];
        word->d8[2] = diffs[2];
        word->d8[3] = diffs[3];

        /* 2-bit nibble is 0b01, only need to set 2nd bit */
        frameptr[0] |= 0x1ul << (30 - 2 * widx);

        packedsamples = 4;
      }
      /* 3 x 10-bit differences */
      else if (diffcount >= 3 &&
               bitwidth[0] <= 10 && bitwidth[1] <= 10 && bitwidth[2] <= 10)
      {

        /* Mask the values, shift to proper location and set in word */
        frameptr[widx] = ((uint32_t)diffs[2] & 0x3FFul);
        frameptr[widx] |= ((uint32_t)diffs[1] & 0x3FFul) << 10;
        frameptr[widx] |= ((uint32_t)diffs[0] & 0x3FFul) << 20;

        /* 2-bit decode nibble is 0b11 (0x3) */
        frameptr[widx] |= 0x3ul << 30;

        /* 2-bit nibble is 0b10 (0x2) */
        frameptr[0] |= 0x2ul << (30 - 2 * widx);

        packedsamples = 3;
      }
      /* 2 x 15-bit differences */
      else if (diffcount >= 2 &&
               bitwidth[0] <= 15 && bitwidth[1] <= 15)
      {
        /* Mask the values, shift to proper location and set in word */
        frameptr[widx] = ((uint32_t)diffs[1] & 0x7FFFul);
        frameptr[widx] |= ((uint32_t)diffs[0] & 0x7FFFul) << 15;

        /* 2-bit decode nibble is 0b10 (0x2) */
        frameptr[widx] |= 0x2ul << 30;

        /* 2-bit nibble is 0b10 (0x2) */
        frameptr[0] |= 0x2ul << (30 - 2 * widx);

        packedsamples = 2;
      }
      /* 1 x 30-bit difference */
      else if (diffcount >= 1 &&
               bitwidth[0] <= 30)
      {
        /* Mask the value and set in word */
        frameptr[widx] = ((uint32_t)diffs[0] & 0x3FFFFFFFul);

        /* 2-bit decode nibble is 0b01 (0x1) */
        frameptr[widx] |= 0x1ul << 30;

        /* 2-bit nibble is 0b10 (0x2) */
        frameptr[0] |= 0x2ul << (30 - 2 * widx);

        packedsamples = 1;
      }
      else
      {
        ms_log (2, "%s: Unable to represent difference in <= 30 bits\n", sid);
        return -1;
      }

      /* Swap encoded word except for 4x8-bit samples */
      if (swapflag && packedsamples != 4)
        ms_gswap4 (&frameptr[widx]);

      diffcount -= packedsamples;
      outputsamples += packedsamples;
    } /* Done with words in frame */

    /* Swap word with nibbles */
    if (swapflag)
      ms_gswap4 (&frameptr[0]);
  } /* Done with frames */

  /* Set Xn (reverse integration constant) in first frame to last sample */
  if (Xnp)
    *Xnp = *(input + outputsamples - 1);
  if (swapflag)
    ms_gswap4 (Xnp);

  if (byteswritten)
    *byteswritten = (uint32_t)(frameidx * 64);

  return outputsamples;
}

void write_steim2_data(FILE *fp, const int32_t *compressed, uint32_t length) {
    if (!fp || !compressed) return;
    
    // 按帧写入数据
    for (uint32_t i = 0; i < length; i++) {
        // 每个32位整数都需要以小端序写入
        uint32_t value = compressed[i];
        fwrite(&value, sizeof(uint32_t), 1, fp);
    }
} 