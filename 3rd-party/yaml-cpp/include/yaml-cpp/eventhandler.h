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

#ifndef EVENTHANDLER_H_62B23520_7C8E_11DE_8A39_0800200C9A66
#define EVENTHANDLER_H_62B23520_7C8E_11DE_8A39_0800200C9A66

#if defined(_MSC_VER) ||                                            \
    (defined(__GNUC__) && (__GNUC__ == 3 && __GNUC_MINOR__ >= 4) || \
     (__GNUC__ >= 4))  // GCC supports "pragma once" correctly since 3.4
#pragma once
#endif

#include <string>

#include "yaml-cpp/anchor.h"
#include "yaml-cpp/emitterstyle.h"

namespace YAML {
struct Mark;

class EventHandler {
 public:
  virtual ~EventHandler() {}

  virtual void OnDocumentStart(const Mark& mark) = 0;
  virtual void OnDocumentEnd() = 0;

  virtual void OnNull(const Mark& mark, anchor_t anchor) = 0;
  virtual void OnAlias(const Mark& mark, anchor_t anchor) = 0;
  virtual void OnScalar(const Mark& mark, const std::string& tag,
                        anchor_t anchor, const std::string& value) = 0;

  virtual void OnSequenceStart(const Mark& mark, const std::string& tag,
                               anchor_t anchor, EmitterStyle::value style) = 0;
  virtual void OnSequenceEnd() = 0;

  virtual void OnMapStart(const Mark& mark, const std::string& tag,
                          anchor_t anchor, EmitterStyle::value style) = 0;
  virtual void OnMapEnd() = 0;

  virtual void OnAnchor(const Mark& /*mark*/,
                        const std::string& /*anchor_name*/) {
    // empty default implementation for compatibility
  }
};
}  // namespace YAML

#endif  // EVENTHANDLER_H_62B23520_7C8E_11DE_8A39_0800200C9A66
