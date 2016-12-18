/*
** Copyright (c) 2016 rxi
**
** This project is free software; you can redistribute it and/or modify it
** under the terms of the MIT license. See LICENSE for details.
**/

#ifndef UTIL_H
#define UTIL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <dirent.h>


#define ERROR(...) do {           \
    fprintf(stderr, "Error: ");   \
    fprintf(stderr, __VA_ARGS__); \
    fprintf(stderr, "\n");        \
    exit(EXIT_FAILURE);           \
  } while (0)


int round_up_po2(int n);
const char *next_arg(int *argc, const char ***argv);
double next_arg_number(int *argc, const char ***argv);
int match_opt(const char *opt, const char *chr, const char *name);
void trim_file_ext(char *str);
void replace_whitespace(char *str);
void check_format_str(const char *fmt, const char *expect);

#endif
