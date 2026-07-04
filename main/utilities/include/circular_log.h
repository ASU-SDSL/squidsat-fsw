#ifndef CIRCULAR_LOG_H
#define CIRCULAR_LOG_H

#include <stdarg.h>

int clog_create();

int clog_log(const char* fmt, ...);

void clog_dump();

void clog_test();

#endif // CIRCULAR_LOG_H