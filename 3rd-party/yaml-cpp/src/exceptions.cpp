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

#include "yaml-cpp/exceptions.h"

// This is here for compatibility with older versions of Visual Studio
// which don't support noexcept
#if defined(_MSC_VER) && _MSC_VER < 1900
    #define YAML_CPP_NOEXCEPT _NOEXCEPT
#else
    #define YAML_CPP_NOEXCEPT noexcept
#endif

namespace YAML {

// These destructors are defined out-of-line so the vtable is only emitted once.
Exception::~Exception() YAML_CPP_NOEXCEPT {}
ParserException::~ParserException() YAML_CPP_NOEXCEPT {}
RepresentationException::~RepresentationException() YAML_CPP_NOEXCEPT {}
InvalidScalar::~InvalidScalar() YAML_CPP_NOEXCEPT {}
KeyNotFound::~KeyNotFound() YAML_CPP_NOEXCEPT {}
InvalidNode::~InvalidNode() YAML_CPP_NOEXCEPT {}
BadConversion::~BadConversion() YAML_CPP_NOEXCEPT {}
BadDereference::~BadDereference() YAML_CPP_NOEXCEPT {}
BadSubscript::~BadSubscript() YAML_CPP_NOEXCEPT {}
BadPushback::~BadPushback() YAML_CPP_NOEXCEPT {}
BadInsert::~BadInsert() YAML_CPP_NOEXCEPT {}
EmitterException::~EmitterException() YAML_CPP_NOEXCEPT {}
BadFile::~BadFile() YAML_CPP_NOEXCEPT {}
}

#undef YAML_CPP_NOEXCEPT


