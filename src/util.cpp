/*
MIT License

Copyright (c) 2019 GIS Core R&D Department, NavInfo Co., Ltd

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#include <string.h>
#include <signal.h>
#include "util.h"

namespace ncserver
{
	static char _wtozu(char c)
	{
		char digit;
		if ('0' <= c && c <= '9')
			digit = c - '0';
		else if ('a' <= c && c <= 'f')
			digit = c - 'a' + 10;
		else if ('A' <= c && c <= 'F')
			digit = c - 'A' + 10;
		return digit;
	}

	size_t urlDecode(const char *src, char *dest, size_t destSize)
	{
		size_t i = 0;
		while (*src && i < destSize)
		{
			if (*src == '%')
			{
				dest[i] = (_wtozu(src[1]) << 4) | _wtozu(src[2]);
				src += 3;
			}
			else
			{
				dest[i] = *src;
				src++;
			}
			i++;
		}

		if (i == destSize)
			i--;

		dest[i++] = 0;

		return i;
	}

#if !defined(WIN32)
	void(*signal(int signo, void(*handler)(int)))(int)
	{

		struct sigaction act, oact;
		act.sa_handler = handler;
		sigemptyset(&act.sa_mask);
		act.sa_flags = 0;
#if defined(SA_INTERRUPT)
		act.sa_flags |= SA_INTERRUPT;
#endif
		if (sigaction(signo, &act, &oact) < 0)
			return SIG_ERR;
		return oact.sa_handler;
	}
#endif

} //namespace ncserver
