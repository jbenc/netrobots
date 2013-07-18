/*
 * libnetrobots, client library for netrobots
 * Copyright (C) 2008 Paolo Bonzini and others
 * Copyright (C) 2013 Jiri Benc
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 */

#ifndef SERVERUTILS_H
#define SERVERUTILS_H

#include <stdio.h>

#define MAX_ARGC	16
#define STD_ALLOC	4
#define STD_BUF		64

#define MAX_IMAGE_BYTES	(100 * 1024)

#define MAX(a, b)	((a) > (b) ? (a) : (b))
#define MIN(a, b)	((a) < (b) ? (a) : (b))

extern int debug;

/* to pass the ***argv pass &argv where argv is an array of strings */
int str_to_argv(char *str, char ***argv);
char *argv_to_str(char **argv);
void ndprintf(FILE *fd, char *fmt, ...);
void ndprintf_die(FILE *fd, char *fmt, ...);
int sockwrite(int fd, int status, char *fmt, ...);
int str_isnumber(char *str);

#endif

