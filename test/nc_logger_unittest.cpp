#include "stdafx.h"
#include "ncserver.h"
#include "gtest.h"
#include "nc_log.h"

using namespace ncserver;

class NcLogTest : public ::testing::Test
{
public:

	static void SetUpTestCase()
	{
		NcLog::instance().registerUpdateLogLevelSignal();
		NcLog::instance().init("echo", LogLevel_info);
	}
};

TEST_F(NcLogTest, basic)
{
	ASYNC_LOG_INFO("basic log %s", "world");
}

TEST_F(NcLogTest, max)
{
	char* text = new char[8 * 1024];
	memset(text, 'a', 8 * 1024 - 1);
	text[8 * 1024 - 1] = '\0';
	ASYNC_LOG_CRIT("max log %s", text);
}

TEST_F(NcLogTest, superMax)
{
	char* text = new char[64 * 1024];
	memset(text, 'a', 64 * 1024 - 1);
	text[64 * 1024 - 1] = '\0';
	ASYNC_LOG_EMERG("superMax log %s", text);
}