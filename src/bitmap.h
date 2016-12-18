/*
** Copyright (c) 2016 rxi
**
** This project is free software; you can redistribute it and/or modify it
** under the terms of the MIT license. See LICENSE for details.
**/

#ifndef BITMAP_H
#define BITMAP_H

#include <stdint.h>

typedef struct {
  uint32_t *data;
  int width, height;
} Bitmap;

void bitmap_blit(Bitmap *dst, Bitmap *src, int x, int y);
void bitmap_blit8(Bitmap *dst, uint8_t *src, int w, int h, int x, int y);

#endif
