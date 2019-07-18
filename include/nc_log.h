/*
MIT License

Copyright (c) 2019 GIS Core R&D Department, NavInfo Co., Ltd

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#pragma once

#ifndef WIN32
#include <syslog.h>
#endif

namespace ncserver {

	class Logger
	{
	public:
		static Logger& instance();

		/**
			@brief
				Use Linux SIGNAL control the log level.
				The relation between the Linux SIGNAL and log level:
					SIGRTMIN + 1(35) : LOG_DEBUG
					SIGRTMIN + 2(36) : LOG_INFO
					SIGRTMIN + 3(37) : LOG_NOTICE
					SIGRTMIN + 4(38) : LOG_WARNING
					SIGRTMIN + 5(39) : LOG_ERR
					SIGRTMIN + 6(40) : LOG_CRIT
					SIGRTMIN + 7(41) : LOG_ALERT
					SIGRTMIN + 8(42) : LOG_EMERG
				Log level reference to linux vsyslog supported log level.
			@note
				If you want to use the signal to control the log level,
				this function must to be called.
			@example
				ncserver::_registerUpdateLogLevelSignal();
				Logger::instance().init(@serverName, @logLevel);
		*/
		void registerUpdateLogLevelSignal();
		/**
			@note 
				This function need to be called before used the marco ASYNC_LOG_*.
				Usually, it is called when the program is initialized.
			@example
				Logger::instance().init(@serverName, @logLevel);
		*/
		void init(const char* serverName, int logLevel);

		void log(int priority, const char* file, int line, const char* func, const char *format, ...);

		int logLevel() { return m_logLevel; }
		void setLogLevel(int logLevel) { m_logLevel = logLevel; }

		int bufferSize() { return m_bufferSize; }
		void setBufferSize(int bufferSize) { m_bufferSize = bufferSize; }

	private:
		Logger();
		~Logger();
		Logger(const Logger& logger);

		const char* logLevelToString(int logLevel);

#ifndef WIN32
		void write(int priority, const char *format, ...);
#endif
		int m_logLevel;
		int m_bufferSize;
	};

} // namespace ncserver

#ifndef WIN32
#define LogLevel_debug    LOG_DEBUG
#define LogLevel_info     LOG_INFO
#define LogLevel_notice   LOG_NOTICE
#define LogLevel_warning  LOG_WARNING
#define LogLevel_error    LOG_ERR
#define LogLevel_crit     LOG_CRIT
#define LogLevel_alert    LOG_ALERT 
#define LogLevel_emerg    LOG_EMERG
#else 
#define LogLevel_debug    7
#define LogLevel_info     6
#define LogLevel_notice   5
#define LogLevel_warning  4
#define LogLevel_error    3
#define LogLevel_crit     2
#define LogLevel_alert    1 
#define LogLevel_emerg    0
#endif 

#define ASYNC_LOG_DEBUG(fmt, ...) do { \
	ncserver::Logger::instance().log(LogLevel_debug, __FILE__, __LINE__, __FUNCTION__, fmt, __VA_ARGS__); \
	}while (0)

#define ASYNC_LOG_INFO(fmt, ...) do { \
	ncserver::Logger::instance().log(LogLevel_info, __FILE__, __LINE__, __FUNCTION__, fmt, __VA_ARGS__); \
	}while (0)

#define ASYNC_LOG_NOTICE(fmt, ...) do { \
	ncserver::Logger::instance().log(LogLevel_notice, __FILE__, __LINE__, __FUNCTION__, fmt, __VA_ARGS__); \
	}while (0)

#define ASYNC_LOG_WARNING(fmt, ...) do { \
	ncserver::Logger::instance().log(LogLevel_warning, __FILE__, __LINE__, __FUNCTION__, fmt, __VA_ARGS__); \
	}while (0)

#define ASYNC_LOG_ERR(fmt, ...) do { \
	ncserver::Logger::instance().log(LogLevel_error, __FILE__, __LINE__, __FUNCTION__, fmt, __VA_ARGS__); \
	}while (0)

#define ASYNC_LOG_CRIT(fmt, ...) do { \
	ncserver::Logger::instance().log(LogLevel_crit, __FILE__, __LINE__, __FUNCTION__, fmt, __VA_ARGS__); \
	}while (0)

#define ASYNC_LOG_ALERT(fmt, ...) do { \
	ncserver::Logger::instance().log(LogLevel_alert, __FILE__, __LINE__, __FUNCTION__, fmt, __VA_ARGS__); \
	}while (0)

#define ASYNC_LOG_EMERG(fmt, ...) do { \
	ncserver::Logger::instance().log(LogLevel_emerg, __FILE__, __LINE__, __FUNCTION__, fmt, __VA_ARGS__); \
	}while (0)
