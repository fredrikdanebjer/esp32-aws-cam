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
