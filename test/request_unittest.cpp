#include "stdafx.h"
#include "ncserver.h"
#include "gtest.h"

using namespace ncserver;

TEST(Request, basic)
{
	Request request;
	request.reset("name=Chen%20Bowei&age=27&gender=male");
	EXPECT_STREQ(request.queryString(), "name=Chen Bowei&age=27&gender=male");
	EXPECT_STREQ(request.parameterForName("name"), "Chen Bowei");
	EXPECT_STREQ(request.parameterForName("age"), "27");
	EXPECT_STREQ(request.parameterForName("gender"), "male");
}