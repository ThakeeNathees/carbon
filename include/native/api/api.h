//------------------------------------------------------------------------------
// MIT License
//------------------------------------------------------------------------------
// 
// Copyright (c) 2020 Thakee Nathees
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//------------------------------------------------------------------------------

#ifndef CARBON_API_H
#define CARBON_API_H

#include "varptr.h"
#include "nativeapi.h"
#include "wrappers.h"

#if defined(_MSC_VER) //  Microsoft 
#define EXPORT __declspec(dllexport)
#elif defined(__GNUC__) //  GCC
#define EXPORT __attribute__((visibility("default")))
#else // unknown
#define EXPORT
#endif

/* ------------------------ USAGE ---------------------------

#define SINGLE_IMPLEMENTATION
#include "carbon/api.h"
using namespace carbon;

#ifdef __cplusplus
extern "C" {
#endif

EXPORT void varapi_init(nativeapi* _api) {
	api = _api;
}

EXPORT varptr your_function(int argc, varptr* argv) {
	return varptr();
}

#ifdef __cplusplus
}
#endif

---------------------------------------------------------*/

#endif // CARBON_API_H
