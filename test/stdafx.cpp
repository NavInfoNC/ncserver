// stdafx.cpp : source file that includes just the standard includes
// grid_server.pch will be the pre-compiled header
// stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"

// TODO: reference any additional headers you need in STDAFX.H
// and not in this file
#ifdef WIN32
#	pragma comment( lib , "ws2_32.lib" )
#endif

char* fcgi_strtok_s(char* buf, const char* spliters, char** context)
{
	char* p = *context;
	char* token;

	if (buf != NULL)
		p = buf;

	if (p == NULL)
	{
		*context = p;
		return NULL;
	}

	while (strchr(spliters, *p) && *p != 0)
	{
		p++;
	}

	token = p;
	while (*p != 0) // or while(*p)
	{
		if (strchr(spliters, *p))
		{
			*p = 0;
			p++;
			break;
		}
		p++;
	}

	*context = p;

	if (*token)
		return token;
	else
		return NULL;
}
