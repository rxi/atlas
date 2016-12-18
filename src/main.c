/*
** Copyright (c) 2016 rxi
**
** This project is free software; you can redistribute it and/or modify it
** under the terms of the MIT license. See LICENSE for details.
**/

#include "lib/stb/stb_rect_pack.h"
#include "lib/stb/stb_truetype.h"
#include "lib/stb/stb_image.h"
#include "lib/stb/stb_image_write.h"
#include "util.h"
#include "limits.h"
#include "bitmap.h"

#define VERSION "0.0.0"


typedef struct {
  char name[MAX_IMAGE_NAME + 1];
  Bitmap bitmap;
} Image;


static unsigned char font_buf[MAX_FONT_SIZE];
static stbrp_node nodes[MAX_ATLAS_WIDTH];
static stbrp_rect rects[MAX_IMAGES];
static Image images[MAX_IMAGES];
static int image_count = 0;


static void print_help(void) {
  printf("Usage: atlas [OPTION]... <dir>\n");
  printf("A small utility for generating a texture atlas from all the images\n");
  printf("and fonts in a directory.\n");
  printf("\n");
  printf("  -h,  --help               Display this help message\n");
  printf("  -v,  --version            Display the version number\n");
  printf("  -o,  --imageout <file>    Image output filename (default: 'out.png')\n");
  printf("  -t,  --textout <file>     Text output filename (default: 'out.txt')\n");
  printf("  -p,  --padding <pixels>   Number of pixels padding (default: '0')\n");
  printf("  -s,  --fontsize <size>    Font size (default: '16')\n");
  printf("  -f,  --linefmt <fmt>      Line format string (default: '%%s %%d %%d %%d %%d')\n");
  printf("  -g,  --glyphfmt <fmt>     Glyph name format string (default: '%%s_%%d')\n");
  printf("  -r,  --ranges <list>      Comma-separated glyph ranges (default: '32-127')\n");
  printf("  -e,  --keepext            Don't trim file extensions from names\n");
  printf("\n");
}


static Image* next_image(void) {
  if (image_count >= MAX_IMAGES) {
    ERROR("Maximum images (%d) exceeded", MAX_IMAGES);
  }
  return &images[image_count++];
}


static int load_image(const char *filename, const char *name) {
  int w, h, n;
  void *data = stbi_load(filename, &w, &h, &n, 4);
  if (data) {
    Image *img = next_image();
    strcpy(img->name, name);
    img->bitmap.data = data;
    img->bitmap.width = w;
    img->bitmap.height = h;
    return 1;
  }
  return 0;
}


static int load_font(
  const char *filename, const char *name, double fontsize,
  const char *glyphfmt, const char *glyphranges
) {
  /* Load file into buffer */
  FILE *fp = fopen(filename, "rb");
  if (!fp) {
    ERROR("Could not load '%s' for reading", filename);
  }
  int n = fread(font_buf, 1, sizeof(font_buf), fp);
  fclose(fp);
  if (n == sizeof(font_buf)) {
    ERROR("Font ('%s') size exceeds font buffer", name);
  }

  /* Init font */
  stbtt_fontinfo font;
  if (stbtt_InitFont(&font, font_buf, 0)) {
    /* Successfully inited -- Check image name size + glyph idx */
    if (strlen(name) + strlen(glyphfmt) + 16 > MAX_IMAGE_NAME) {
      ERROR("File name too long: '%s'", name);
    }

    /* Get font height and scale */
    int ascent, descent, linegrap;
    double scale = stbtt_ScaleForMappingEmToPixels(&font, fontsize);
    stbtt_GetFontVMetrics(&font, &ascent, &descent, &linegrap);
    int height = ceil((ascent - descent + linegrap) * scale) + 1;
    int baseline = ascent * scale + 1;

    /* Iterate glyph ranges and load glyphs */
    const char *p = glyphranges;
    while (*p) {
      /* Get range */
      int lo, hi, i;
      int n = sscanf(p, "%d-%d%n", &lo, &hi, &i);
      if (n != 2 || hi < 0 || lo > 0x10FFFF || lo > hi) {
        ERROR("Invalid glyph range list '%s'", glyphranges);
      }
      p += i;
      while (*p == ' ') p++;
      if (*p == ',') p++;

      /* Load glyphs from range */
      for (int c = lo; c <= hi; c++) {
        Image *img = next_image();
        Bitmap *bmp = &img->bitmap;
        sprintf(img->name, glyphfmt, name, c);

        /* Render glyph to 8bit buffer */
        int w, h, gw, x, y;
        uint8_t *p = stbtt_GetCodepointBitmap(&font, 0, scale, c, &w, &h, 0,0);
        stbtt_GetCodepointBitmapBox(&font, c, scale, scale, &x, &y, 0, 0);

        /* Init bitmap and blit 8bit buffer */
        stbtt_GetCodepointHMetrics(&font, c, &gw, NULL);
        bmp->width = gw * scale;
        bmp->height = height;
        bmp->data = calloc(4, bmp->width * bmp->height);
        bitmap_blit8(bmp, p, w, h, x, y + baseline);

        /* Free 8-bit buffer */
        free(p);
      }
    }
    /* Loaded successfully */
    return 1;
  }
  return 0;
}


int main(int argc, const char **argv) {
  Bitmap atlas = { NULL, MIN_ATLAS_WIDTH, MIN_ATLAS_HEIGHT };
  int dircount = 0;

  /* Init defaults */
  const char *indirs[MAX_DIRS];
  const char *imageout = "out.png";
  const char *textout = "out.txt";
  const char *linefmt = "%s %d %d %d %d";
  const char *glyphfmt = "%s_%d";
  const char *glyphranges = "32-127";
  double fontsize = 16;
  int trimext = 1;
  int padding = 0;


  /* Handle arguments */
  next_arg(&argc, &argv); /* Consume executable name */
  while (argc > 0) {
    const char *arg = next_arg(&argc, &argv);

    if (arg[0] != '-') {
      if (dircount >= MAX_DIRS) {
        ERROR("maximum input directories (%d) exceeded", MAX_DIRS);
      }
      indirs[dircount++] = arg;

    } else if (match_opt(arg, "h", "help")) {
      print_help();
      exit(EXIT_SUCCESS);

    } else if (match_opt(arg, "v", "version")) {
      printf("atlas version " VERSION "\n");
      exit(EXIT_SUCCESS);

    } else if (match_opt(arg, "o", "imageout")) {
      imageout = next_arg(&argc, &argv);

    } else if (match_opt(arg, "t", "textout")) {
      textout = next_arg(&argc, &argv);

    } else if (match_opt(arg, "p", "padding")) {
      padding = next_arg_number(&argc, &argv);
      if (padding < 0) {
        ERROR("Expected padding greater or equal to 0");
      }

    } else if (match_opt(arg, "s", "fontsize")) {
      fontsize = next_arg_number(&argc, &argv);
      if (fontsize <= 0) {
        ERROR("Expected font size greater than 0");
      }

    } else if (match_opt(arg, "f", "linefmt")) {
      linefmt = next_arg(&argc, &argv);
      check_format_str(linefmt, "sdddd");

    } else if (match_opt(arg, "g", "glyphfmt")) {
      glyphfmt = next_arg(&argc, &argv);
      check_format_str(glyphfmt, "sd");

    } else if (match_opt(arg, "r", "ranges")) {
      glyphranges = next_arg(&argc, &argv);

    } else if (match_opt(arg, "e", "keepext")) {
      trimext = 0;

    } else {
      ERROR("Invalid argument '%s'", arg);
    }
  }
  if (dircount == 0) {
    ERROR("Expected input directory");
  }


  /* Iterate each directory and load each file */
  printf("Loading assets... ");
  fflush(stdout);
  int filecount = 0;
  for (int i = 0; i < dircount; i++) {
    DIR *d = opendir(indirs[i]);
    if (!d) {
      ERROR("Could not open directory '%s'", indirs[i]);
    }
    struct dirent *ep;
    while ( (ep = readdir(d)) ) {
      /* Skip dotfiles */
      if (*ep->d_name == '.') {
        continue;
      }

      /* Check and copy filename */
      if (strlen(ep->d_name) > MAX_IMAGE_NAME) {
        ERROR("File name too long: '%s'", ep->d_name);
      }
      char name[MAX_IMAGE_NAME];
      strcpy(name, ep->d_name);

      /* Get full path */
      char fullname[512];
      sprintf(fullname, "%.240s/%.240s", indirs[i], ep->d_name);

      /* Trim file extension and replace whitespace with `_` */
      if (trimext) {
        trim_file_ext(name);
      }
      replace_whitespace(name);

      /* Try to load */
      if (load_image(fullname, name)) goto loaded;
      if (load_font(fullname, name, fontsize, glyphfmt, glyphranges)) goto loaded;

      ERROR("Could not load file '%s'", ep->d_name);

loaded:
      filecount++;
    }
    closedir(d);
  }
  printf("Done (%d files)\n", filecount);


  /* Init rects array and pack */
pack:
  for (int i = 0; i < image_count; i++) {
    Bitmap *bitmap = &images[i].bitmap;
    /* Init rect */
    rects[i].id = i;
    rects[i].w = bitmap->width + padding * 2;
    rects[i].h = bitmap->height + padding * 2;
    /* Update atlas size if image size is larger */
    if (rects[i].w > atlas.width) atlas.width = round_up_po2(rects[i].w);
    if (rects[i].h > atlas.height) atlas.height = round_up_po2(rects[i].h);
  }
  stbrp_context ctx;
  stbrp_init_target(&ctx, atlas.width, atlas.height, nodes, MAX_ATLAS_WIDTH);
  stbrp_pack_rects(&ctx, rects, image_count);


  /* Check all rects were packed, if not increase atlas size and try again */
  for (int i = 0; i < image_count; i++) {
    if (!rects[i].was_packed) {
      /* Increase size */
      if (atlas.height < atlas.width) {
        atlas.height = atlas.width;
      } else {
        atlas.width *= 2;
      }
      /* Bounds check */
      if (atlas.width > MAX_ATLAS_WIDTH) {
        ERROR("Max atlas width (%d) exceeded", MAX_ATLAS_WIDTH);
      }
      if (atlas.height > MAX_ATLAS_HEIGHT) {
        ERROR("Max atlas height (%d) exceeded", MAX_ATLAS_HEIGHT);
      }
      /* Retry */
      goto pack;
    }
  }


  /* Allocate pixel buffer for atlas */
  atlas.data = calloc(4, atlas.width * atlas.height);

  /* Blit image bitmaps to atlas */
  for (int i = 0; i < image_count; i++) {
    bitmap_blit(
      &atlas, &images[rects[i].id].bitmap,
      rects[i].x + padding, rects[i].y + padding);
  }


  /* Write output image file */
  printf("Writing image file... ");
  fflush(stdout);
  if (strstr(imageout, ".png")) {
    stbi_write_png(imageout, atlas.width, atlas.height, 4, atlas.data, 0);
  } else if (strstr(imageout, ".tga")) {
    stbi_write_tga(imageout, atlas.width, atlas.height, 4, atlas.data);
  } else {
    ERROR("Unsupported output image format ('%s'), try '.png'", imageout);
  }
  printf("Done (%dx%d)\n", atlas.width, atlas.height);


  /* Write output text file */
  printf("Writing text file... ");
  fflush(stdout);
  FILE *fp = fopen(textout, "wb");
  if (!fp) {
    ERROR("Could not open `%s` for writing", textout);
  }
  for (int i = 0; i < image_count; i++) {
    Image *img = &images[rects[i].id];
    Bitmap *bmp = &img->bitmap;
    fprintf(
      fp, linefmt,
      img->name, rects[i].x + padding, rects[i].y + padding,
      bmp->width, bmp->height);
    fprintf(fp, "\n");
  }
  fclose(fp);
  printf("Done (%d images)\n", image_count);


  /* Clear up  */
  for (int i = 0; i < image_count; i++) {
    free(images[i].bitmap.data);
  }
  free(atlas.data);


  return EXIT_SUCCESS;
}
