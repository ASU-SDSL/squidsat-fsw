#ifndef CIRCULAR_LOG_H
#define CIRCULAR_LOG_H

#include <stdarg.h>
#include <stdint.h>

int clog_create();

int clog_log(const char* fmt, ...);

int clog_dump(uint32_t index, uint32_t line_start, uint32_t line_stop);

void clog_test();

#endif // CIRCULAR_LOG_H