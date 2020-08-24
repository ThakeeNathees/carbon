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

#define INHERITS_OBJECT_ADDNL(m_class, m_inheritance)                       \
	static void _register_class() {                                         \
		NativeClasses::set_inheritance(STR(m_class), STR(m_inheritance));   \
		_bind_data();                                                       \
	}

#define UNDEF_VAR_DEFINES
#define HAVE_OBJECT_CALL_MAP
#include "var.h/_var.h"
using namespace varh;

#include <assert.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#define _USE_MATH_DEFINES
#include <math.h>

#include <string>
#include <cstring>
#include <vector>
#include <map>
#include <type_traits>
#include <typeinfo>

#include <iostream>
#include <ostream>
#include <sstream>
#include <fstream>
#include <memory>
#include <new>

#include "error.h"
#include "native_bind.gen.h"

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

#define STR(m_) #m_
#define STRINGIFY(m_) STR(m_)
#define NOEFFECT(m_) m_
#define PLACE_HOLDER_MACRO

// platform macros
#ifdef _WIN32
#	define PLATFORM_WINDOWS
#elif defined(__APPLE__) || defined(__MACH__)
#	define PLATFORM_APPLE
#elif defined(__linux__)
#	define PLATFORM_X11
#else
#	error "PLATFORM NOT SUPPORTED."
#endif

#ifdef DEBUG_BUILD
#	ifdef _MSC_VER
#		define DEBUG_BREAK() __debugbreak()
#	else
#		define DEBUG_BREAK() __builtin_trap()
#	endif
#	define DEBUG_BREAK_COND(m_cond) if (m_cond) DEBUG_BREAK()
#else 
#	define DEBUG_BREAK()
#	define DEBUG_BREAK_COND(m_cond)
#endif


#define _CRASH()                \
do {                            \
	char* CRASH_HERE = nullptr; \
	*CRASH_HERE = '\0';         \
} while(false)

#ifdef DEBUG_BUILD
// TODO: Use debug print library.
#define DEBUG_PRINT(m_msg)                                                                                        \
do {                                                                                                              \
	printf("DEBUG_PRINT: \"%s\" at %s (%s:%i)\n", String("" m_msg).c_str(), __FUNCTION__, __FILE__, __LINE__);    \
} while (false)
#define DEBUG_PRINT_COND(m_cond, m_msg) if (m_cond) DEBUG_PRINT(m_msg)
#else
#define DEBUG_PRINT(m_msg)
#define DEBUG_PRINT_COND(m_cond, m_msg)
#endif

#if defined(DEBUG_BUILD)
#define ASSERT(m_cond)                                                                                       \
	do {                                                                                                     \
		if (!(m_cond)) {                                                                                     \
			printf("ASSERTION: at %s (%s:%i)\n%s is false", __FUNCTION__, __FILE__, __LINE__, STR(m_cond));  \
			throw Error(Error::INTERNAL_BUG);                                                                \
		}                                                                                                    \
	} while (false)

#else
#define ASSERT(m_cond)
#endif

#define MISSED_ENUM_CHECK(m_max_enum, m_max_value) \
    static_assert((int)m_max_enum == m_max_value, "MissedEnum: " STRINGIFY(m_max_enum) " != " STRINGIFY(m_value) \
        "\n\tat: " __FILE__ "(" STRINGIFY(__LINE__) ")")

#define VSNPRINTF_BUFF_SIZE 8192

#if !defined(_VAR_H) && !defined(VAR_H)
	template<typename T, typename... Targs>
	inline ptr<T> newptr(Targs... p_args) {
		return std::make_shared<T>(p_args...);
	}
	
	template<typename T1, typename T2>
	inline ptr<T1> ptrcast(T2 p_ptr) {
		return std::static_pointer_cast<T1>(p_ptr);
	}
	
	template<typename T>
	using ptr = std::shared_ptr<T>;
	
	template<typename T>
	using stdvec = std::vector<T>;
#endif

#define CLEAR_GETCHAR_BUFFER()						    \
	do {											    \
		char c;										    \
		while ((c = getchar()) != '\n' && c != EOF) {}  \
	} while (false)

namespace carbon {
typedef char byte_t;
}

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