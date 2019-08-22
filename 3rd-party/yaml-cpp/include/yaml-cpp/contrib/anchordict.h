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

#ifndef ANCHORDICT_H_62B23520_7C8E_11DE_8A39_0800200C9A66
#define ANCHORDICT_H_62B23520_7C8E_11DE_8A39_0800200C9A66

#if defined(_MSC_VER) ||                                            \
    (defined(__GNUC__) && (__GNUC__ == 3 && __GNUC_MINOR__ >= 4) || \
     (__GNUC__ >= 4))  // GCC supports "pragma once" correctly since 3.4
#pragma once
#endif

#include <vector>

#include "../anchor.h"

namespace YAML {
/**
 * An object that stores and retrieves values correlating to {@link anchor_t}
 * values.
 *
 * <p>Efficient implementation that can make assumptions about how
 * {@code anchor_t} values are assigned by the {@link Parser} class.
 */
template <class T>
class AnchorDict {
 public:
  AnchorDict() : m_data{} {}
  void Register(anchor_t anchor, T value) {
    if (anchor > m_data.size()) {
      m_data.resize(anchor);
    }
    m_data[anchor - 1] = value;
  }

  T Get(anchor_t anchor) const { return m_data[anchor - 1]; }

 private:
  std::vector<T> m_data;
};
}  // namespace YAML

#endif  // ANCHORDICT_H_62B23520_7C8E_11DE_8A39_0800200C9A66
