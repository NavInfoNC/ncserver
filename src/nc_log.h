#include "stdafx.h"

namespace ncserver {

	class Logger
	{
	public:
		static Logger& instance();

		/**
			@note 
			This function need to be called before used the marco ASYNC_LOG_*.
			Usually, it is called when the program is initialized.
			@example
			Logger::instance().init(@serverName, @logLevel);
		*/
		void init(const char* serverName, int priority);

		void log(int priority, const char* file, int line, const char* func, const char *format, ...);

		int priority() { return m_priority; }
		void setPriority(int priority) { m_priority = priority; }

		int bufferSize() { return m_bufferSize; }
		void setBufferSize(int bufferSize) { m_bufferSize = bufferSize; }

	private:
		Logger();
		~Logger();
#ifdef linux
		void write(int priority, const char *format, ...);
#endif
		int m_priority;
		char* m_message; 
		int m_bufferSize;
	};

#ifdef linux
	void _updateLogLevel(int sig);

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
	void _registerUpdateLogLevelSignal();
#endif

} // namespace ncserver


/** 
	LOG_EMERG         0
	LOG_ALERT         1
	LOG_CRIT          2
	LOG_ERR           3
	LOG_WARNING       4
	LOG_NOTICE        5
	LOG_INFO          6
	LOG_DEBUG         7
*/

#define ASYNC_LOG_DEBUG(fmt, ...) do { \
	ncserver::Logger::instance().log(7, __FILE__, __LINE__, __FUNCTION__, fmt, __VA_ARGS__); \
	}while (0)

#define ASYNC_LOG_INFO(fmt, ...) do { \
	ncserver::Logger::instance().log(6, __FILE__, __LINE__, __FUNCTION__, fmt, __VA_ARGS__); \
	}while (0)

#define ASYNC_LOG_NOTICE(fmt, ...) do { \
	ncserver::Logger::instance().log(5, __FILE__, __LINE__, __FUNCTION__, fmt, __VA_ARGS__); \
	}while (0)

#define ASYNC_LOG_WARNING(fmt, ...) do { \
	ncserver::Logger::instance().log(4, __FILE__, __LINE__, __FUNCTION__, fmt, __VA_ARGS__); \
}while (0)

#define ASYNC_LOG_ERR(fmt, ...) do { \
	ncserver::Logger::instance().log(3, __FILE__, __LINE__, __FUNCTION__, fmt, __VA_ARGS__); \
	}while (0)

#define ASYNC_LOG_CRIT(fmt, ...) do { \
	ncserver::Logger::instance().log(2, __FILE__, __LINE__, __FUNCTION__, fmt, __VA_ARGS__); \
	}while (0)

#define ASYNC_LOG_ALERT(fmt, ...) do { \
	ncserver::Logger::instance().log(1, __FILE__, __LINE__, __FUNCTION__, fmt, __VA_ARGS__); \
	}while (0)

#define ASYNC_LOG_EMERG(fmt, ...) do { \
	ncserver::Logger::instance().log(0, __FILE__, __LINE__, __FUNCTION__, fmt, __VA_ARGS__); \
	}while (0)
