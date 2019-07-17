#include "stdafx.h"
#include <stdio.h>
#include <cstdarg>
#include "nc_log.h"

#ifndef WIN32
#include <signal.h>
#include <syslog.h>
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
	}

	Logger::~Logger()
	{
#ifndef WIN32
		::closelog();
#endif
	}

	void Logger::init(const char* serverName, int priority)
	{
		m_priority = priority;
#ifndef WIN32
		::openlog(serverName, LOG_CONS | LOG_PID, LOG_DEBUG);
#endif
	}

	void Logger::log(int priority, const char* file, int line, const char* func, const char *format, ...) {
		if (priority > m_priority) return;
		m_message = (char*)alloca(sizeof(char) * m_bufferSize);
		va_list args;
		va_start(args, format);
		
#ifndef WIN32
		vsnprintf(m_message, m_bufferSize, format, args);
#else
		vsnprintf_s(m_message, m_bufferSize, m_bufferSize-1, format, args);
#endif
		va_end(args);

#ifndef WIN32
		write(priority, "{\"file\":\"%s\",\"line\":%d,\"func\":\"%s\",\"msg\":\"%s\"}\0", file, line, func, m_message);
#else
		printf("\"level\":\"%s\",\"file\":\"%s\",\"line\":\"%d\",\"func\":\"%s\",\"msg\":\"%s\"\n", logLevelToString(priority), file, line, func, m_message);
		char debugMessage[8092];
		_snprintf_s(debugMessage, 8092, "\"level\":\"%s\",\"file\":\"%s\",\"line\":\"%d\",\"func\":\"%s\",\"msg\":\"%s\"\n", logLevelToString(priority), file, line, func, m_message);
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

	void _updateLogLevel(int sig)
	{
		if (sig == SIGRTMIN + 1)
		{
			Logger::instance().setPriority(LOG_DEBUG);
		}
		else if (sig == SIGRTMIN + 2)
		{
			Logger::instance().setPriority(LOG_INFO);
		}
		else if (sig == SIGRTMIN + 3)
		{
			Logger::instance().setPriority(LOG_NOTICE);
		}
		else if (sig == SIGRTMIN + 4)
		{
			Logger::instance().setPriority(LOG_WARNING);
		}
		else if (sig == SIGRTMIN + 5)
		{
			Logger::instance().setPriority(LOG_ERR);
		}
		else if (sig == SIGRTMIN + 6)
		{
			Logger::instance().setPriority(LOG_CRIT);
		}
		else if (sig == SIGRTMIN + 7)
		{
			Logger::instance().setPriority(LOG_ALERT);
		}
		else if (sig == SIGRTMIN + 8)
		{
			Logger::instance().setPriority(LOG_EMERG);
		}
	}

	void _registerUpdateLogLevelSignal()
	{
		signal(SIGRTMIN + 1, _updateLogLevel);
		signal(SIGRTMIN + 2, _updateLogLevel);
		signal(SIGRTMIN + 3, _updateLogLevel);
		signal(SIGRTMIN + 4, _updateLogLevel);
		signal(SIGRTMIN + 5, _updateLogLevel);
		signal(SIGRTMIN + 6, _updateLogLevel);
		signal(SIGRTMIN + 7, _updateLogLevel);
		signal(SIGRTMIN + 8, _updateLogLevel);
	}

#endif
}
