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

#ifndef NODE_DETAIL_BOOL_TYPE_H_62B23520_7C8E_11DE_8A39_0800200C9A66
#define NODE_DETAIL_BOOL_TYPE_H_62B23520_7C8E_11DE_8A39_0800200C9A66

#if defined(_MSC_VER) ||                                            \
    (defined(__GNUC__) && (__GNUC__ == 3 && __GNUC_MINOR__ >= 4) || \
     (__GNUC__ >= 4))  // GCC supports "pragma once" correctly since 3.4
#pragma once
#endif

namespace YAML {
namespace detail {
struct unspecified_bool {
  struct NOT_ALLOWED;
  static void true_value(NOT_ALLOWED*) {}
};
typedef void (*unspecified_bool_type)(unspecified_bool::NOT_ALLOWED*);
}
}

#define YAML_CPP_OPERATOR_BOOL()                                            \
  operator YAML::detail::unspecified_bool_type() const {                    \
    return this->operator!() ? 0                                            \
                             : &YAML::detail::unspecified_bool::true_value; \
  }

#endif  // NODE_DETAIL_BOOL_TYPE_H_62B23520_7C8E_11DE_8A39_0800200C9A66
