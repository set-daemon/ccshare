/*
 * file name: debug.h
 * purpose  :
 * author   : huang chunping
 * date     : 2014-09-24
 * history  :
 */

#ifndef __DEBUG_H__
#define __DEBUG_H__

#include <stdio.h>

#if defined(DEBUG)
#define INFO(format, ...) fprintf(stdout, format,##__VA_ARGS__)
#define ERROR(format, ...) fprintf(stderr, format,##__VA_ARGS__)
#else
#define INFO(format, ...)
#define ERROR(format, ...)
#endif

#endif // __DEBUG_H__
