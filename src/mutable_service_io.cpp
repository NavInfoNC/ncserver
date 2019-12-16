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
#include <cstdarg>
#include "mutable_service_io.h"

namespace ncserver
{
	static char* _copyStr(const char* str, size_t len)
	{
		char* newCopy = (char*)malloc(len + 1);
		memcpy(newCopy, str, len);
		newCopy[len] = '\0';
		return newCopy;
	}

	MutableServiceIo::MutableServiceIo()
	{
		m_postData = _copyStr("", 0);
		m_buffer = _copyStr("", 0);
		m_bufferSize = 0;
	}

	MutableServiceIo::~MutableServiceIo()
	{
		free(m_buffer);
		free(m_postData);
	}

	void MutableServiceIo::read(void *buffer, size_t size)
	{
		memcpy(buffer, m_postData, size);;
	}

	void MutableServiceIo::write(void* buffer, size_t size)
	{
		m_buffer = realloc(m_buffer, m_bufferSize + size);
		memcpy((char*)m_buffer + m_bufferSize, buffer, size);
		m_bufferSize += size;
	}

	int MutableServiceIo::print(const char* format, ...)
	{
		int count;
		char buffer[8192];
		va_list argptr;

		va_start(argptr, format);
		count = vsnprintf(buffer, 8192, format, argptr);
		va_end(argptr);

		if (count >= 0)
		{
			m_buffer = realloc(m_buffer, m_bufferSize + count);
			memcpy((char*)m_buffer + m_bufferSize, buffer, count);
			m_bufferSize += count;
		}

		return count;
	}

	int MutableServiceIo::addHeaderField(const char* format, ...)
	{
		int cnt;
		char buffer[4096];
		va_list argptr;

		va_start(argptr, format);
		cnt = vsnprintf(buffer, 4096, format, argptr);
		va_end(argptr);

		if (cnt >= 0)
		{
			m_buffer = realloc(m_buffer, m_bufferSize + cnt + 2);
			memcpy((char*)m_buffer + m_bufferSize, buffer, cnt);
			m_bufferSize += cnt;
			memcpy((char*)m_buffer + strlen((char*)buffer), "\r\n", 2);
			m_bufferSize += 2;
		}

		return cnt;
	}

	void MutableServiceIo::endHeaderField(void)
	{
		m_buffer = realloc(m_buffer, m_bufferSize + 2);
		memcpy((char*)m_buffer + m_bufferSize, "\r\n", 2);
		m_bufferSize += 2;
		return;
	}

	void MutableServiceIo::flush(void)
	{
		return;
	}

	void MutableServiceIo::setPostData(void* postData, size_t size)
	{
		free(m_postData);
		m_postData = _copyStr((char*)postData, size);
	}

}
