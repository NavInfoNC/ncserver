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

#ifndef PTR_VECTOR_H_62B23520_7C8E_11DE_8A39_0800200C9A66
#define PTR_VECTOR_H_62B23520_7C8E_11DE_8A39_0800200C9A66

#if defined(_MSC_VER) ||                                            \
    (defined(__GNUC__) && (__GNUC__ == 3 && __GNUC_MINOR__ >= 4) || \
     (__GNUC__ >= 4))  // GCC supports "pragma once" correctly since 3.4
#pragma once
#endif

#include <cstddef>
#include <cstdlib>
#include <memory>
#include <vector>

namespace YAML {

// TODO: This class is no longer needed
template <typename T>
class ptr_vector {
 public:
  ptr_vector() : m_data{} {}
  ptr_vector(const ptr_vector&) = delete;
  ptr_vector& operator=(const ptr_vector&) = delete;

  void clear() { m_data.clear(); }

  std::size_t size() const { return m_data.size(); }
  bool empty() const { return m_data.empty(); }

  void push_back(std::unique_ptr<T>&& t) { m_data.push_back(std::move(t)); }
  T& operator[](std::size_t i) { return *m_data[i]; }
  const T& operator[](std::size_t i) const { return *m_data[i]; }

  T& back() { return *(m_data.back().get()); }

  const T& back() const { return *(m_data.back().get()); }

 private:
  std::vector<std::unique_ptr<T>> m_data;
};
}  // namespace YAML

#endif  // PTR_VECTOR_H_62B23520_7C8E_11DE_8A39_0800200C9A66
