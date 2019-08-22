/*
Copyright (c) 2008-2015 Jesse Beder.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#include "yaml-cpp/ostream_wrapper.h"

#include <algorithm>
#include <cstring>
#include <iostream>

namespace YAML {
ostream_wrapper::ostream_wrapper()
    : m_buffer(1, '\0'),
      m_pStream(nullptr),
      m_pos(0),
      m_row(0),
      m_col(0),
      m_comment(false) {}

ostream_wrapper::ostream_wrapper(std::ostream& stream)
    : m_buffer{},
      m_pStream(&stream),
      m_pos(0),
      m_row(0),
      m_col(0),
      m_comment(false) {}

ostream_wrapper::~ostream_wrapper() {}

void ostream_wrapper::write(const std::string& str) {
  if (m_pStream) {
    m_pStream->write(str.c_str(), str.size());
  } else {
    m_buffer.resize(std::max(m_buffer.size(), m_pos + str.size() + 1));
    std::copy(str.begin(), str.end(), m_buffer.begin() + m_pos);
  }

  for (std::size_t i = 0; i < str.size(); i++) {
    update_pos(str[i]);
  }
}

void ostream_wrapper::write(const char* str, std::size_t size) {
  if (m_pStream) {
    m_pStream->write(str, size);
  } else {
    m_buffer.resize(std::max(m_buffer.size(), m_pos + size + 1));
    std::copy(str, str + size, m_buffer.begin() + m_pos);
  }

  for (std::size_t i = 0; i < size; i++) {
    update_pos(str[i]);
  }
}

void ostream_wrapper::update_pos(char ch) {
  m_pos++;
  m_col++;

  if (ch == '\n') {
    m_row++;
    m_col = 0;
    m_comment = false;
  }
}
}  // namespace YAML
