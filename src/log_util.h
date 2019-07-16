#pragma once
#include "stdafx.h"

#ifdef MAPBAR_LINUX

#include <syslog.h>
#include <cstdarg>

namespace ncserver {

    extern void openlog(const char *argv0, const char *log_dir, int priority);
    extern void closelog();

    extern void log(int priority, const char *format, ...) __attribute__((format(printf, 2, 3)));

    typedef void (*openlog_t)(const char *, const char *, int);
    typedef void (*closelog_t)();

    typedef void (*vlog_t)(int, const char *, va_list);
    extern void setvlog(vlog_t);

    extern void setlog(openlog_t, closelog_t, vlog_t);

} // namespace ncserver

#endif
