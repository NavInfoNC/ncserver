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

#ifndef STREAM_H_62B23520_7C8E_11DE_8A39_0800200C9A66
#define STREAM_H_62B23520_7C8E_11DE_8A39_0800200C9A66

#if defined(_MSC_VER) ||                                            \
    (defined(__GNUC__) && (__GNUC__ == 3 && __GNUC_MINOR__ >= 4) || \
     (__GNUC__ >= 4))  // GCC supports "pragma once" correctly since 3.4
#pragma once
#endif

#include "yaml-cpp/mark.h"
#include <cstddef>
#include <deque>
#include <ios>
#include <iostream>
#include <set>
#include <string>

namespace YAML {
class Stream {
 public:
  friend class StreamCharSource;

  Stream(std::istream& input);
  Stream(const Stream&) = delete;
  Stream(Stream&&) = delete;
  Stream& operator=(const Stream&) = delete;
  Stream& operator=(Stream&&) = delete;
  ~Stream();

  operator bool() const;
  bool operator!() const { return !static_cast<bool>(*this); }

  char peek() const;
  char get();
  std::string get(int n);
  void eat(int n = 1);

  static char eof() { return 0x04; }

  const Mark mark() const { return m_mark; }
  int pos() const { return m_mark.pos; }
  int line() const { return m_mark.line; }
  int column() const { return m_mark.column; }
  void ResetColumn() { m_mark.column = 0; }

 private:
  enum CharacterSet { utf8, utf16le, utf16be, utf32le, utf32be };

  std::istream& m_input;
  Mark m_mark;

  CharacterSet m_charSet;
  mutable std::deque<char> m_readahead;
  unsigned char* const m_pPrefetched;
  mutable size_t m_nPrefetchedAvailable;
  mutable size_t m_nPrefetchedUsed;

  void AdvanceCurrent();
  char CharAt(size_t i) const;
  bool ReadAheadTo(size_t i) const;
  bool _ReadAheadTo(size_t i) const;
  void StreamInUtf8() const;
  void StreamInUtf16() const;
  void StreamInUtf32() const;
  unsigned char GetNextByte() const;
};

// CharAt
// . Unchecked access
inline char Stream::CharAt(size_t i) const { return m_readahead[i]; }

inline bool Stream::ReadAheadTo(size_t i) const {
  if (m_readahead.size() > i)
    return true;
  return _ReadAheadTo(i);
}
}  // namespace YAML

#endif  // STREAM_H_62B23520_7C8E_11DE_8A39_0800200C9A66
