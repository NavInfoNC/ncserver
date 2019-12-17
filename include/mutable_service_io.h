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
#pragma once

#include "ncserver.h"

namespace ncserver
{
	class MutableServiceIo : public ServiceIo
	{
	public:
		MutableServiceIo();

		virtual ~MutableServiceIo();

		virtual void read(void *buffer, size_t size);

		virtual void write(void* buffer, size_t size);

		virtual int print(const char* format, ...);

		virtual int addHeaderField(const char* format, ...);

		virtual void endHeaderField(void);

		virtual void flush(void);

		void setPostData(void* postData, size_t size);

		void* buffer() { return m_buffer; }
		size_t bufferSize() { return m_bufferSize; }
		void initBuffer();
		void cleanupBuffer();

	private:
		void* m_postData;
		void* m_buffer;
		size_t m_bufferSize;
	};
}
