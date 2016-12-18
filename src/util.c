/*
** Copyright (c) 2016 rxi
**
** This project is free software; you can redistribute it and/or modify it
** under the terms of the MIT license. See LICENSE for details.
**/

#include "util.h"


int round_up_po2(int n) {
  int x = 1;
  while (x < n) x <<= 1;
  return x;
}


const char* next_arg(int *argc, const char ***argv) {
  if (*argc == 0) {
    ERROR("Expected additional argument(s)");
  }
  (*argc)--;
  return *(*argv)++;
}


double next_arg_number(int *argc, const char ***argv) {
  char *endptr;
  const char *arg = next_arg(argc, argv);
  double res = strtod(arg, &endptr);
  if (arg == endptr) {
    ERROR("Expected numerical argument, got '%s'", arg);
  }
  return res;
}


int match_opt(const char *opt, const char *chr, const char *name) {
  if (*opt == '-') {
    opt++;
    if (*opt == '-') {
      return strcmp(opt + 1, name) == 0;
    }
    return strcmp(opt, chr) == 0;
  }
  return 0;
}


void trim_file_ext(char *str) {
  int len = strlen(str);
  char *p = str + len - 1;
  while (p != str) {
    if (*p == '.') {
      *p = '\0';
      break;
    }
    p--;
  }
}


void replace_whitespace(char *str) {
  char *p = str;
  while (*p) {
    if (*p == ' ' || *p == '\t' || *p == '\n') {
      *p = '_';
    }
    p++;
  }
}


/* Checks the format string is valid for the given values. For example `expect`
** should be "sd" if you plan to pass a string and int following the fmt */
void check_format_str(const char *fmt, const char *expect) {
  const char *p = fmt;
  const char *e = expect;
  while (*p) {
    if (*p++ == '%') {
      if (*p == *e) {
        e++;
      } else if (*p != '%') {
        if (*e) {
          ERROR("Invalid format specifier '%%%c', expected '%%%c'", *p, *e);
        } else {
          ERROR("Too many format specifiers");
        }
      }
      p++;
    }
  }
  if (*e) {
    ERROR("Too few format specifiers, expected %d", (int) strlen(expect));
  }
}
