#include "stdafx.h"
#include "ncserver.h"
#include "gtest.h"
#include "nc_log.h"

using namespace ncserver;

TEST(NcLog, basic)
{
	NcLog::instance().registerUpdateLogLevelSignal();
	NcLog::instance().init("echo", LogLevel_info);
	
	ASYNC_LOG_ALERT("Hello %s", "world");
}
