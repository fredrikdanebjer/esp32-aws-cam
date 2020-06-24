/*
* @file debug.c
*
* The MIT License (MIT)
*
* Copyright (c) 2020 Fredrik Danebjer
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*/

#include "util/debug.h"
#include <stdarg.h>
#include <stdio.h>

#define DEBUG_EMER_HEADER "[DEBUG EMERGENCY]:"
#define DEBUG_WARN_HEADER "[DEBUG WARNING]:"
#define DEBUG_INFO_HEADER "[DEBUG INFO]:"

static char* DEBUG_PRINT_HEADER_DICT[DEBUG_count] = {
  DEBUG_EMER_HEADER, 
  DEBUG_WARN_HEADER, 
  DEBUG_INFO_HEADER 
};

void DEBUG_print(DEBUG_level_t level, const char* format, ...)
{
  va_list args;
  va_start(args, format);

#ifndef DEBUG_PRINT_LEVEL
  return;
#else
  if(DEBUG_PRINT_LEVEL >= level)
  {
    // TODO: Add task protection around these calls
    printf(DEBUG_PRINT_HEADER_DICT[level]);
    vprintf(format, args);
  }
  va_end(args);
#endif
}
