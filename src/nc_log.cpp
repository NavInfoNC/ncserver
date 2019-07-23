#include "stdafx.h"
#include <stdio.h>
#include <cstdarg>
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

	void NcLog::init(const char* serverName, int logLevel)
	{
		m_logLevel = logLevel;
#if !defined(WIN32)
		::openlog(serverName, LOG_CONS | LOG_PID, LOG_DEBUG);
#endif
	}

	void NcLog::log(bool hook, LogLevel logLevel, const char* file, int line, const char* func, const char* format, ...)
	{
		const static int MAX_MESSAGE_SIZE = 64 * 1024;

		if (!hook)
		{
			if (logLevel > m_logLevel)
				return;
		}

		char header[2048];
		if (!hook)
		{
			sprintf(header, R"(%s(%d): %s: [%s] )", file, line, LogLevel_toString(logLevel), func);
		}
		size_t headerSize = strlen(header);

		char* message = NULL;
		bool failed = false;
		int bufferSize = 4096;
		for (;;)
		{
			if (!hook)
			{
				message = (char*)alloca(bufferSize + headerSize);
				memcpy(message, header, headerSize);
			}
			else
			{
				message = (char*)alloca(bufferSize);
			}


			int requiredSize;
			va_list args;
			va_start(args, format);
			{
#if defined(WIN32) && _MSC_VER < 1900	// before visual studio 2015
				requiredSize = _vscprintf(format, args) + 1;
				if (requiredSize <= bufferSize)
				{
					if (!hook)
					{
						vsnprintf(message + headerSize, bufferSize, format, args);
					}
					else
					{
						vsnprintf(message, bufferSize, format, args);
					}
				}
#else
				if (!hook)
				{
					requiredSize = vsnprintf(message + headerSize, bufferSize, format, args) + 1;
				}
				else
				{
					requiredSize = vsnprintf(message, bufferSize, format, args) + 1;
				}
#endif
			}
			va_end(args);

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
			m_delegate->nclogWillOutputMessage(message);
		else
		{
#if defined(WIN32)
			printf(message);
			OutputDebugStringA(message);
#else
			write(logLevel, message);
#endif
		}
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
			NcLog::instance().setLogLevel(LOG_DEBUG);
		}
		else if (sig == SIGRTMIN + 2)
		{
			NcLog::instance().setLogLevel(LOG_INFO);
		}
		else if (sig == SIGRTMIN + 3)
		{
			NcLog::instance().setLogLevel(LOG_NOTICE);
		}
		else if (sig == SIGRTMIN + 4)
		{
			NcLog::instance().setLogLevel(LOG_WARNING);
		}
		else if (sig == SIGRTMIN + 5)
		{
			NcLog::instance().setLogLevel(LOG_ERR);
		}
		else if (sig == SIGRTMIN + 6)
		{
			NcLog::instance().setLogLevel(LOG_CRIT);
		}
		else if (sig == SIGRTMIN + 7)
		{
			NcLog::instance().setLogLevel(LOG_ALERT);
		}
		else if (sig == SIGRTMIN + 8)
		{
			NcLog::instance().setLogLevel(LOG_EMERG);
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

}
