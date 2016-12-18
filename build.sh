#!/bin/bash

gcc -Wall -Wextra -std=gnu99 -O2 -o atlas\
  src/main.c\
  src/util.c\
  src/bitmap.c\
  src/lib/stb/stb_image.c\
  src/lib/stb/stb_image_write.c\
  src/lib/stb/stb_rect_pack.c\
  src/lib/stb/stb_truetype.c\
  -lm

strip atlas
