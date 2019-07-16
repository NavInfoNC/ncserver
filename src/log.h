#include "stdafx.h"
#include "log_util.h"

#ifdef MAPBAR_LINUX

#include <stdio.h>
#include <signal.h>

namespace ncserver {

	class Logger
	{
	public:
		static Logger& instance()
		{
			static Logger logger;
			return logger;
		}

		Logger()
		{
			m_bufferSize = 1024;
			m_message = new char[m_bufferSize];
		}

		~Logger()
		{
			delete(m_message);
			ncserver::closelog();
		}

		/**
			@note 
				This function need to be called before used the marco ASYNC_LOG_*.
				Usually, it is called when the program is initialized.
			@example
				Logger::instance().init(@serverName, @logLevel);
		*/
		void init(const char* serverName, int priority) 
		{
			m_priority = priority;
			ncserver::openlog(serverName, NULL, LOG_DEBUG);
		}


		int priority() { return m_priority; }
		void setPriority(int priority) { m_priority = priority; }

		int bufferSize() { return m_bufferSize; }
		void setBufferSize(int size) { m_bufferSize = size; }

	private:
		void log(int priority, const char* file, int line, const char* func, const char *format, ...) {
			if (priority > m_priority) return;
			va_list args;
			va_start(args, format);
			vsprintf(m_message, format, args);
			va_end(args);
			ncserver::log(priority, "{\"file\":\"%s\",\"line\":%d,\"func\":\"%s\",\"msg\":\"%s\"}\0", file, line, func, m_message);
		}

		int m_priority;
		char* m_message; 
		int m_bufferSize;
	};

#define ASYNC_LOG_DEBUG(fmt, args...) do { \
	Logger::instance().log(LOG_DEBUG, __FILE__, __LINE__, __FUNCTION__, fmt, ##args); \
	}while (0)

#define ASYNC_LOG_INFO(fmt, args...) do { \
	Logger::instance().log(LOG_INFO, __FILE__, __LINE__, __FUNCTION__, fmt, ##args); \
	}while (0)

#define ASYNC_LOG_NOTICE(fmt, args...) do { \
	Logger::instance().log(LOG_NOTICE, __FILE__, __LINE__, __FUNCTION__, fmt, ##args); \
	}while (0)

#define ASYNC_LOG_WARNING(fmt, args...) do { \
	Logger::instance().log(LOG_WARNING, __FILE__, __LINE__, __FUNCTION__, fmt, ##args); \
	}while (0)

#define ASYNC_LOG_ERR(fmt, args...) do { \
	Logger::instance().log(LOG_ERR, __FILE__, __LINE__, __FUNCTION__, fmt, ##args); \
	}while (0)

#define ASYNC_LOG_CRIT(fmt, args...) do { \
	Logger::instance().log(LOG_CRIT, __FILE__, __LINE__, __FUNCTION__, fmt, ##args); \
	}while (0)

#define ASYNC_LOG_ALERT(fmt, args...) do { \
	Logger::instance().log(LOG_ALERT, __FILE__, __LINE__, __FUNCTION__, fmt, ##args); \
	}while (0)

#define ASYNC_LOG_EMERG(fmt, args...) do { \
	Logger::instance().log(LOG_EMERG, __FILE__, __LINE__, __FUNCTION__, fmt, ##args); \
	}while (0)

	static void _updateLogLevel(int sig)
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
	static void _registerUpdateLogLevelSignal()
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

} // namespace ncserver
#endif
