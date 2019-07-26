#include "stdafx.h"
#include <stdio.h>
#include "nc_log.h"
#include "util.h"

#if defined(WIN32)
#	include "windows.h"
#else
#	include <signal.h>
#	include "alloca.h"
#endif

namespace ncserver
{
	const char* LogLevel_toString(LogLevel o)
	{
		switch (o)
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
			return "none";
		}
	}

	//////////////////////////////////////////////////////////////////////////
	NcLog& NcLog::instance()
	{
		static NcLog logger;
		return logger;
	}

	NcLog::NcLog()
	{
		m_logLevel = LogLevel_error;
		m_delegate = NULL;
	}

	NcLog::~NcLog()
	{
#if !defined(WIN32)
		::closelog();
#endif
	}

	void NcLog::init(const char* serverName, LogLevel logLevel)
	{
		m_logLevel = logLevel;
#if !defined(WIN32)
		::openlog(serverName, LOG_CONS | LOG_PID, LOG_DEBUG);
#endif
	}

#if defined (WIN32)
#define snprintf _snprintf
#endif

	void NcLog::log(LogLevel logLevel, const char* file, int line, const char* func, const char* format, ...)
	{
		char header[2048];
		snprintf(header, 2048, R"(%s(%d): %s: [%s] )", file, line, LogLevel_toString(logLevel), func);
		size_t headerSize = strlen(header);

		va_list argList;
		va_start(argList, format);
		_log(logLevel, header, headerSize, format, argList);
		va_end(argList);
	}

	void NcLog::rawLog(const char *format, ...)
	{
		va_list argList;
		va_start(argList, format);
		_log(LogLevel_info, NULL, 0, format, argList);
		va_end(argList);
	}

#if !defined(WIN32)
	void NcLog::write(LogLevel logLevel, const char *format, ...) {
		va_list args;
		va_start(args, format);
		vsyslog(logLevel, format, args);
		va_end(args);
	}

    static void _updateLogLevel(int sig)
	{
		if (sig == SIGRTMIN + 1)
		{
			NcLog::instance().setLogLevel(LogLevel_debug);
		}
		else if (sig == SIGRTMIN + 2)
		{
			NcLog::instance().setLogLevel(LogLevel_info);
		}
		else if (sig == SIGRTMIN + 3)
		{
			NcLog::instance().setLogLevel(LogLevel_notice);
		}
		else if (sig == SIGRTMIN + 4)
		{
			NcLog::instance().setLogLevel(LogLevel_warning);
		}
		else if (sig == SIGRTMIN + 5)
		{
			NcLog::instance().setLogLevel(LogLevel_error);
		}
		else if (sig == SIGRTMIN + 6)
		{
			NcLog::instance().setLogLevel(LogLevel_crit);
		}
		else if (sig == SIGRTMIN + 7)
		{
			NcLog::instance().setLogLevel(LogLevel_alert);
		}
		else if (sig == SIGRTMIN + 8)
		{
			NcLog::instance().setLogLevel(LogLevel_emerg);
		}
	}
#endif

	void NcLog::registerUpdateLogLevelSignal()
	{
#if !defined(WIN32)
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

	void NcLog::_log(LogLevel logLevel, const char* header, size_t headerSize, const char* format, va_list argList)
	{
		const static int MAX_MESSAGE_SIZE = 64 * 1024;

		// header being NULL indicates it's a raw log
		// raw logs shouldn't be filtered
		if (header != NULL && logLevel > m_logLevel)
			return;

		char* message = NULL;
		bool failed = false;
		int bufferSize = 4096;
		for (;;)
		{
			message = (char*)alloca(bufferSize + headerSize);
			memcpy(message, header, headerSize);

			int requiredSize;
#if defined(WIN32) && _MSC_VER < 1900	// before visual studio 2015
			requiredSize = _vscprintf(format, argList) + 1;
			if (requiredSize <= bufferSize)
			{
				vsnprintf(message + headerSize, bufferSize, format, argList);
			}
#else
			requiredSize = vsnprintf(message + headerSize, bufferSize, format, argList) + 1;
#endif

			if (requiredSize < 0)
			{
				failed = true;
				break;	// error
			}
			else if (requiredSize <= bufferSize)
				break;	// done
			else if (requiredSize <= MAX_MESSAGE_SIZE)
			{
				// buffer insufficient
				bufferSize = requiredSize;
			}
			else
			{
				failed = true;
				break;
			}
		}

		if (failed)
			return;

		if (m_delegate != NULL)
			m_delegate->nclogWillOutputMessage(header != NULL, message);
		else
		{
#if defined(WIN32)
			printf("%s\n", message);
			OutputDebugStringA(message);
			OutputDebugStringA("\n");
#else
			write(logLevel, "%s", message);
#endif
		}
	}

}
