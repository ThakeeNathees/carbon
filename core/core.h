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

#ifndef CORE_H
#define CORE_H

#define UNDEF_VAR_DEFINES
#include "var.h/_var.h"
using namespace varh;

// https://stackoverflow.com/questions/2124339/c-preprocessor-va-args-number-of-arguments
#ifdef _MSC_VER // Microsoft compilers

#   define GET_ARG_COUNT(...)  INTERNAL_EXPAND_ARGS_PRIVATE(INTERNAL_ARGS_AUGMENTER(__VA_ARGS__))

#   define INTERNAL_ARGS_AUGMENTER(...) unused, __VA_ARGS__
#   define INTERNAL_EXPAND(x) x
#   define INTERNAL_EXPAND_ARGS_PRIVATE(...) INTERNAL_EXPAND(INTERNAL_GET_ARG_COUNT_PRIVATE(__VA_ARGS__, 69, 68, 67, 66, 65, 64, 63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0))
#   define INTERNAL_GET_ARG_COUNT_PRIVATE(_1_, _2_, _3_, _4_, _5_, _6_, _7_, _8_, _9_, _10_, _11_, _12_, _13_, _14_, _15_, _16_, _17_, _18_, _19_, _20_, _21_, _22_, _23_, _24_, _25_, _26_, _27_, _28_, _29_, _30_, _31_, _32_, _33_, _34_, _35_, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, _59, _60, _61, _62, _63, _64, _65, _66, _67, _68, _69, _70, count, ...) count

#else // Non-Microsoft compilers

#   define GET_ARG_COUNT(...) INTERNAL_GET_ARG_COUNT_PRIVATE(0, ## __VA_ARGS__, 70, 69, 68, 67, 66, 65, 64, 63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)
#   define INTERNAL_GET_ARG_COUNT_PRIVATE(_0, _1_, _2_, _3_, _4_, _5_, _6_, _7_, _8_, _9_, _10_, _11_, _12_, _13_, _14_, _15_, _16_, _17_, _18_, _19_, _20_, _21_, _22_, _23_, _24_, _25_, _26_, _27_, _28_, _29_, _30_, _31_, _32_, _33_, _34_, _35_, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, _59, _60, _61, _62, _63, _64, _65, _66, _67, _68, _69, _70, count, ...) count

#endif

#define ARG_1(_1,...) _1
#define ARG_2(_1,_2,...) _2
#define ARG_3(_1,_2,_3,...) _3
#define ARG_4(_1,_2,_3,_4,...) _4
#define ARG_5(_1,_2,_3,_4,_5,...) _5
#define ARG_6(_1,_2,_3,_4,_5,_6,...) _6
#define ARG_7(_1,_2,_3,_4,_5,_6,_7,...) _7
#define ARG_8(_1,_2,_3,_4,_5,_6,_7,_8,...) _8
#define ARG_9(_1,_2,_3,_4,_5,_6,_7,_8,_9,...) _9
#define ARG_10(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,...) _10

#define STRCAT2(m_1, m_2) m_1##m_2
#define STRCAT3(m_1, m_2, m_3) m_1##m_2##m_3
#define STRCAT4(m_1, m_2, m_3, m_4) m_1##m_2##m_3##m_4
#define STRCAT5(m_1, m_2, m_3, m_4, m_5) m_1##m_2##m_3##m_4##m_5

#define STR(m_) #m_
#define STRINGIFY(m_) STR(m_)
#define NOEFFECT(m_) m_
#define PLACE_HOLDER_MACRO

#ifdef _MSC_VER
#	define DEBUG_BREAK() __debugbreak()
#else
#	define DEBUG_BREAK() __builtin_trap()
#endif

#ifdef _DEBUG
#define DEBUG_PRINT(...)                                                                                       \
do {                                                                                                           \
	if (GET_ARG_COUNT(__VA_ARGS__) == 0)                                                                       \
		printf("DEBUG_PRINT: at %s (%s:%i)\n", __FUNCTION__, __FILE__, __LINE__);                              \
	else                                                                                                       \
		printf("DEBUG_PRINT: \"%s\" at %s (%s:%i)\n", ARG_1(__VA_ARGS__), __FUNCTION__, __FILE__, __LINE__);   \
} while (false)
#else
#define DEBUG_PRINT
#endif

#define newptr(T1, ...) std::make_shared<T1>(__VA_ARGS__);
#define newptr2(T1, T2, ...) std::make_shared<T1, T2>(__VA_ARGS__);
template<typename T>
using Ptr = std::shared_ptr<T>;


// for windows dll define CARBON_DLL, CARBON_DLL_EXPORT
#if defined(CARBON_DLL)
#	if defined(CARBON_DLL_EXPORT)
#		define CARBON_API __declspec(dllexport)
#	else
#		define CARBON_API __declspec(dllimport)
#	endif
#else
	#define CARBON_API
#endif


#endif // CORE_H