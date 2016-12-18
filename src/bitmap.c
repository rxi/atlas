/*
** Copyright (c) 2016 rxi
**
** This project is free software; you can redistribute it and/or modify it
** under the terms of the MIT license. See LICENSE for details.
**/

#include "util.h"
#include "bitmap.h"


/* Blits `src` to `dst` at [x,y] with no blending */
void bitmap_blit(Bitmap *dst, Bitmap *src, int x, int y) {
  for (int i = 0; i < src->height; i++) {
    int dy = y + i;
    if (dy < 0 || dy >= dst->height) continue;
    for (int j = 0; j < src->width; j++) {
      int dx = x + j;
      if (dx < 0 || dx >= dst->width) continue;
      dst->data[dx + dy * dst->width] = src->data[i * src->width + j];
    }
  }
}


/* Blits an 8bit alpha-channel-only buffer to the source, no blending */
void bitmap_blit8(Bitmap *dst, uint8_t *src, int w, int h, int x, int y) {
  for (int i = 0; i < h; i++) {
    int dy = y + i;
    if (dy < 0 || dy >= dst->height) continue;
    for (int j = 0; j < w; j++) {
      int dx = x + j;
      if (dx < 0 || dx >= dst->width) continue;
      dst->data[dx + dy * dst->width] = 0x00ffffff | src[i * w + j] << 24;
    }
  }
}
