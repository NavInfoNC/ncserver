#include "stdafx.h"
#include "log_util.h"

#ifdef MAPBAR_LINUX

#include <cstdio>
#include <cstdlib>

namespace ncserver {

    static openlog_t global_openlog_ = NULL;
    static closelog_t global_closelog_ = ::closelog;
    static vlog_t global_vlog_ = vsyslog;
    static int global_priority_ = LOG_ERR;

    void openlog(const char *argv0, const char *log_dir, int priority) {
        char new_path[ 1024 ] = { 0 };
        if( NULL != log_dir && '~' == log_dir[0] ) {
            snprintf( new_path, sizeof( new_path ), "%s%s", getenv( "HOME" ), log_dir + 1 );
		} else if ( NULL != log_dir ) {
            snprintf( new_path, sizeof( new_path ), "%s", log_dir );
        }
        global_priority_ = priority;
        if( NULL != global_openlog_ ) {
            global_openlog_( argv0, new_path, priority );
		} else {
            ::openlog( argv0, LOG_CONS | LOG_PID, priority );
        }
    }

    void closelog() {
        global_closelog_();
    }

    void log(int priority, const char *format, ...) {
        if( priority > global_priority_ ) return;
        va_list args;
        va_start(args, format);
        global_vlog_(priority, format, args);
        va_end(args);
    }

    void setvlog(vlog_t vlog) {
        global_vlog_ = vlog;
    }

    void setlog(openlog_t open_log, closelog_t close_log, vlog_t vlog) {
        global_openlog_ = open_log;
        global_closelog_ = close_log;
        global_vlog_ = vlog;
    }

} // namespace ncserver

#endif
