#include "stdafx.h"
#include <stdio.h>
#include <cstdarg>
#include "nc_log.h"
#include "util.h"

#ifndef WIN32
#include <signal.h>
#include "alloca.h"
#else
#include "windows.h"
#endif

namespace ncserver
{
	Logger& Logger::instance()
	{
		static Logger logger;
		return logger;
	}

	Logger::Logger()
	{
		m_bufferSize = 4096;
		m_logLevel = LogLevel_error;
	}

	Logger::~Logger()
	{
#ifndef WIN32
		::closelog();
#endif
	}

	void Logger::init(const char* serverName, int logLevel)
	{
		m_logLevel = logLevel;
#ifndef WIN32
		::openlog(serverName, LOG_CONS | LOG_PID, LOG_DEBUG);
#endif
	}

	void Logger::log(int logLevel, const char* file, int line, const char* func, const char *format, ...) {
		if (logLevel > m_logLevel) return;
		char * message = (char*)alloca(sizeof(char) * m_bufferSize);
		va_list args;
		va_start(args, format);
		
#ifndef WIN32
		vsnprintf(message, m_bufferSize, format, args);
#else
		vsnprintf_s(message, m_bufferSize, m_bufferSize-1, format, args);
#endif
		va_end(args);

#ifndef WIN32
		write(logLevel, "{\"file\":\"%s\",\"line\":%d,\"func\":\"%s\",\"msg\":\"%s\"}\0", file, line, func, message);
#else
		printf("\"level\":\"%s\",\"file\":\"%s\",\"line\":\"%d\",\"func\":\"%s\",\"msg\":\"%s\"\n", logLevelToString(logLevel), file, line, func, message);
		char* debugMessage = (char*)alloca(sizeof(char)* (m_bufferSize + 128));
		_snprintf_s(debugMessage, m_bufferSize + 128, m_bufferSize + 127, "\"level\":\"%s\",\"file\":\"%s\",\"line\":\"%d\",\"func\":\"%s\",\"msg\":\"%s\"\n", logLevelToString(logLevel), file, line, func, message);
		OutputDebugStringA(debugMessage);
#endif
	}

	const char* Logger::logLevelToString(int logLevel)
	{
		switch(logLevel)
		{
		case 0:
			return "emerg";
		case 1:
			return "alert";
		case 2:
			return "crit";
		case 3:
			return "error";
		case 4:
			return "warning";
		case 5:
			return "notice";
		case 6:
			return "info";
		case 7:
			return "debug";
		default:
			return NULL;
		}
	}

#ifndef WIN32
	void Logger::write(int priority, const char *format, ...) {
		va_list args;
		va_start(args, format);
		vsyslog(priority, format, args);
		va_end(args);
	}
	
    static void _updateLogLevel(int sig)
	{
		if (sig == SIGRTMIN + 1)
		{
			Logger::instance().setLogLevel(LOG_DEBUG);
		}
		else if (sig == SIGRTMIN + 2)
		{
			Logger::instance().setLogLevel(LOG_INFO);
		}
		else if (sig == SIGRTMIN + 3)
		{
			Logger::instance().setLogLevel(LOG_NOTICE);
		}
		else if (sig == SIGRTMIN + 4)
		{
			Logger::instance().setLogLevel(LOG_WARNING);
		}
		else if (sig == SIGRTMIN + 5)
		{
			Logger::instance().setLogLevel(LOG_ERR);
		}
		else if (sig == SIGRTMIN + 6)
		{
			Logger::instance().setLogLevel(LOG_CRIT);
		}
		else if (sig == SIGRTMIN + 7)
		{
			Logger::instance().setLogLevel(LOG_ALERT);
		}
		else if (sig == SIGRTMIN + 8)
		{
			Logger::instance().setLogLevel(LOG_EMERG);
		}
	}
#endif

	void Logger::registerUpdateLogLevelSignal()
	{
#ifndef WIN32
		signal(SIGRTMIN + 1, _updateLogLevel);
		signal(SIGRTMIN + 2, _updateLogLevel);
		signal(SIGRTMIN + 3, _updateLogLevel);
		signal(SIGRTMIN + 4, _updateLogLevel);
		signal(SIGRTMIN + 5, _updateLogLevel);
		signal(SIGRTMIN + 6, _updateLogLevel);
		signal(SIGRTMIN + 7, _updateLogLevel);
		signal(SIGRTMIN + 8, _updateLogLevel);
#endif
	}

}
