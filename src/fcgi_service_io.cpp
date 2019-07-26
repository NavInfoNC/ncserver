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
#include "stdafx.h"

#include "fcgi_stdio.h"
#include "fcgi_service_io.h"

namespace ncserver
{
	FCgiServiceIo::FCgiServiceIo(FCGI_FILE* file)
	{
		m_file = file;
	}

	FCgiServiceIo::~FCgiServiceIo(void)
	{
		;
	}

	void FCgiServiceIo::read(void *buffer, size_t size)
	{
		fread(buffer, size, 1, FCGI_stdin);
	}

	void FCgiServiceIo::write(void* buffer, size_t size)
	{
		FCGI_fwrite(buffer, 1, size, m_file);
	}

	int FCgiServiceIo::print(const char* format, ...)
	{
		int count;
		char buffer[8192];
		va_list argptr;

		va_start(argptr, format);
		count = vsnprintf(buffer, 8192, format, argptr);
		va_end(argptr);

		if (count >= 0)
		{
			FCGI_printf("%s", buffer);
		}

		return count;
	}

	int FCgiServiceIo::addHeaderField(const char* format, ...)
	{
		int cnt;
		char buffer[4096];
		va_list argptr;

		va_start(argptr, format);
		cnt = vsnprintf(buffer, 4096, format, argptr);
		va_end(argptr);

		if (cnt >= 0)
		{
			FCGI_printf("%s\r\n", buffer);
		}

		return cnt;
	}

	void FCgiServiceIo::endHeaderField(void)
	{
		FCGI_printf("\r\n");
	}

	void FCgiServiceIo::flush(void)
	{
		FCGI_fflush(m_file);
	}
}