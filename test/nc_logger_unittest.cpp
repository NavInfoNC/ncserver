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
	}
	
	static void TearDownTestCase()
	{
	}
	
	void SetUp()
	{
		m_lastMessage = NULL;
	}
	void TearDown()
	{
		free(m_lastMessage);
	}

	const char* lastMessage() { return m_lastMessage; }

	virtual void nclogWillOutputMessage(const char* message)
	{
		const char* text = strchr(message, ']') + 2;	// skip file, lineno, func name
		size_t len = strlen(text);
		free(m_lastMessage);
		m_lastMessage = (char*)malloc(len + 1);
		memcpy(m_lastMessage, text, len + 1);
	}
	
protected:
	char* m_lastMessage;
};

TEST_F(NcLogTest, basic)
{
	NcLog& log = NcLog::instance();
	log.registerUpdateLogLevelSignal();
	log.init("echo", LogLevel_info);
	log.setDelegate(this);
	
	ASYNC_LOG_ALERT("Hello %s", "world");
	EXPECT_STREQ(lastMessage(), "Hello world");

	char largeBuffer8k[1024 * 8];
	memset(largeBuffer8k, 'a', sizeof(largeBuffer8k));
	largeBuffer8k[sizeof(largeBuffer8k) - 1] = 0;
	ASYNC_LOG_ALERT(largeBuffer8k);
	EXPECT_EQ(strlen(lastMessage()), strlen(largeBuffer8k));
}
