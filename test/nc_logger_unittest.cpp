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
		m_lastMessage = copyStr("");
	}
	void TearDown()
	{
		free(m_lastMessage);
	}

	const char* lastMessage() { return m_lastMessage; }

	virtual void nclogWillOutputMessage(bool hasHeader, const char* message)
	{
		const char* text = NULL;
		if (hasHeader)
		{
			text = strchr(message, ']') + 2; // skip file, lineno, func name
		}
		else
		{
			text = message;
		}

		free(m_lastMessage);
		m_lastMessage = copyStr(text);
	}

protected:
	char* m_lastMessage;
	static NcLog* m_nclog;

	char* copyStr(const char* str)
	{
		size_t len = strlen(str);
		char* newCopy = (char*)malloc(len + 1);
		memcpy(newCopy, str, len + 1);
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
