#ifndef CIRCULAR_LOG_H
#define CIRCULAR_LOG_H

#include <stdarg.h>

int circular_log_create();

int circular_log(const char* fmt, ...);

#endif // CIRCULAR_LOG_H