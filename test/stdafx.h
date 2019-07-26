// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include <string>
#include <string.h>
#include <stdlib.h>
#include "gtest.h"

#if defined(__GNUC__)
#	include <unistd.h>
#endif

char* fcgi_strtok_s(char* buf, const char* spliters, char** context);
