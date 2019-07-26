#include "stdafx.h"
#include "ncserver.h"
#include "gtest.h"
#include "nc_log.h"

using namespace ncserver;

class NcLogTest : public ::testing::Test, public NcLogDelegate
{
public:
	static void SetUpTestCase()
	{
		m_nclog = &NcLog::instance();
		m_nclog->registerUpdateLogLevelSignal();
		m_nclog->init("echo", LogLevel_info);
	}

	static void TearDownTestCase()
	{
	}

	void SetUp()
	{
		m_nclog->setDelegate(this);
		m_lastMessage = copyStr("", 0);
		m_lastLogLevel = copyStr("none", 4);
	}

	void TearDown()
	{
		free(m_lastMessage);
		free(m_lastLogLevel);
	}

	const char* lastMessage() { return m_lastMessage; }
	const char* lastLogLevel() { return m_lastLogLevel; }

	virtual void nclogWillOutputMessage(bool hasHeader, const char* message)
	{
		const char* text = NULL;
		if (hasHeader)
		{
			text = strchr(message, ']') + 2; // skip file, lineno, func name
			const char* logLevel = strchr(message, ':') + 2;
			const char* endOfLogLevel = strchr(logLevel, ':');
			m_lastLogLevel = copyStr(logLevel, endOfLogLevel - logLevel);
		}
		else
		{
			text = message;
			m_lastLogLevel = copyStr("none", 4);
		}

		free(m_lastMessage);
		m_lastMessage = copyStr(text, strlen(text));
	}

protected:
	char* m_lastMessage;
	char* m_lastLogLevel;
	static NcLog* m_nclog;

	char* copyStr(const char* str, size_t len)
	{
		char* newCopy = (char*)malloc(len + 1);
		memcpy(newCopy, str, len);
		newCopy[len] = '\0';
		return newCopy;
	}
};

NcLog* NcLogTest::m_nclog = NULL;

TEST_F(NcLogTest, basic)
{
	ASYNC_LOG_ALERT("Hello %s", "world");
	EXPECT_STREQ(lastMessage(), "Hello world");
}

TEST_F(NcLogTest, zeroParam)
{
	ASYNC_LOG_ALERT("Hello world");
	EXPECT_STREQ(lastMessage(), "Hello world");
}

TEST_F(NcLogTest, raw)
{
	ASYNC_RAW_LOG("Hello world");
	EXPECT_STREQ(lastMessage(), "Hello world");

	ASYNC_RAW_LOG("Hello %s", "cplusplus");
	EXPECT_STREQ(lastMessage(), "Hello cplusplus");
}

TEST_F(NcLogTest, 10k)
{
	// 10k*'a' = 'aaaaaaaaaaaaaaaaaaaaaaaa....'
	char largeBuffer[1024 * 10];
	memset(largeBuffer, 'a', sizeof(largeBuffer));
	largeBuffer[sizeof(largeBuffer) - 1] = 0;
	ASYNC_LOG_ALERT(largeBuffer);
	EXPECT_TRUE(lastMessage()[0] == 'a');
	EXPECT_TRUE(lastMessage()[sizeof(largeBuffer) - 2] == 'a');
	EXPECT_EQ(strlen(lastMessage()), strlen(largeBuffer));
}

TEST_F(NcLogTest, 65k)
{
	// 65k*'a' = 'aaaaaaaaaaaaaaaaaaaaaaaa....'
	char largeBuffer[1024 * 65];
	memset(largeBuffer, 'a', sizeof(largeBuffer));
	largeBuffer[sizeof(largeBuffer) - 1] = 0;
	ASYNC_LOG_ALERT(largeBuffer);
	EXPECT_EQ(strlen(lastMessage()), 0);
}

TEST_F(NcLogTest, logLevel)
{
	LogLevel originalLevel = NcLog::instance().logLevel();

 	NcLog::instance().setLogLevel(LogLevel_notice);
	EXPECT_EQ(NcLog::instance().logLevel(), LogLevel_notice);
	ASYNC_LOG_NOTICE("notice 1");
	EXPECT_STREQ(lastMessage(), "notice 1");
	EXPECT_STREQ(lastLogLevel(), LogLevel_toString(LogLevel_notice));

	NcLog::instance().setLogLevel(LogLevel_warning);
	EXPECT_EQ(NcLog::instance().logLevel(), LogLevel_warning);
	ASYNC_LOG_NOTICE("notice 2");
	ASYNC_LOG_INFO("info");
	EXPECT_STREQ(lastMessage(), "notice 1");
	EXPECT_STREQ(lastLogLevel(), LogLevel_toString(LogLevel_notice));

	ASYNC_RAW_LOG("raw");
	EXPECT_STREQ(lastMessage(), "raw");
	EXPECT_STREQ(lastLogLevel(), "none");

	NcLog::instance().setLogLevel(originalLevel);
}

#if !defined(LINUX)
TEST_F(NcLogTest, logPrintOnWindows)
{
	NcLog::instance().setDelegate(NULL);

	const char* fileName = __FILE__;
	const char* functionName = __FUNCTION__;

	int standardOut = _dup(1);
	FILE* logFile = freopen("abc.log", "w", stdout);
	ASYNC_LOG_NOTICE("This is a log that would be printed on stdout."); int line1 = __LINE__;
	ASYNC_LOG_INFO("And this log should be printed in a new line."); int line2 = __LINE__;
	fflush(logFile);
	_dup2(standardOut, 1);

	logFile = fopen("abc.log", "r");
	int start = ftell(logFile);
	fseek(logFile, 0, SEEK_END);
	int fileLen = ftell(logFile) - start;
	fseek(logFile, 0, SEEK_SET);
	EXPECT_GT(fileLen, 0);
	if (fileLen > 0)
	{
		char* actualLog = (char*)malloc(fileLen +1);
		memset(actualLog, 0, fileLen + 1);
		fread(actualLog, fileLen, 1, logFile);

		char* expectedLog = (char*)malloc(4096);
		memset(expectedLog, 0, 4096);
		sprintf(expectedLog,
			"%s(%d): %s: [%s] This is a log that would be printed on stdout.\n"
			"%s(%d): %s: [%s] And this log should be printed in a new line.\n",
			fileName, line1, LogLevel_toString(LogLevel_notice), functionName,
			fileName, line2, LogLevel_toString(LogLevel_info), functionName);

		EXPECT_STREQ(actualLog, expectedLog);
		free(expectedLog);
		free(actualLog);
	}
	fclose(logFile);
	remove("abc.log");
}

#endif