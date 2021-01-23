//------------------------------------------------------------------------------
// MIT License
//------------------------------------------------------------------------------
// 
// Copyright (c) 2020-2021 Thakee Nathees
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

// TODO: write a better usage here:
// #define CARBON_IMPLEMENTATION
// #include "carbon.h"
// 
// define CARBON_NO_MAIN before including "carbon.h" if you don't want to
// implement the carbon entrypoint and embed carbon into your project.
// 
// define CARBON_INCLUDE_CRASH_HANDLER_MAIN with CARBON_NO_MAIN to only
// use crash handler main and define _main(int argc, char** argv) for the
// entry point


#ifndef CARBON_H
#define CARBON_H

// core imports

#ifndef CORE_INTERNAL_H
#define CORE_INTERNAL_H

#include <assert.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#define _USE_MATH_DEFINES
#include <math.h>

#ifdef __cplusplus

#include <algorithm>
#include <functional>
#include <type_traits>
#include <typeinfo>
#include <bitset>

#include <vector>
#include <cstring>
#include <string>
#include <stack>
#include <map>
#include <unordered_map>

#include <chrono>
#include <iostream>
#include <ostream>
#include <sstream>
#include <memory>
#include <fstream>
#include <new>
#include <thread>
#include <mutex>

#endif // __cplusplus

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
	#define PLATFORM_WINDOWS
#elif defined(__APPLE__) || defined(__MACH__)
	#include <TargetConditionals.h>
	#if TARGET_IPHONE_SIMULATOR
		#define PLATFORM_IOS_SIMULATOR
	#elif TARGET_OS_IPHONE
		#define PLATFORM_IOS
	#elif TARGET_OS_MAC
		#define PLATFORM_APPLE
	#else
		#error "Unknown Apple platform"
	#endif
#elif defined(__linux__)
	#define PLATFORM_LINUX
#elif defined(__unix__)
	#define PLATFORM_UNIX
#elif defined(_POSIX_VERSION)
	#define PLATFORM_POSIX
#else
	#error "Unknown Platform"
#endif

#if defined(CARBON_DLL)
	#if defined(CARBON_DLL_EXPORT)
		#define CARBON_API __declspec(dllexport)
	#else
		#define CARBON_API __declspec(dllimport)
	#endif
#else
#define CARBON_API
#endif

// https://stackoverflow.com/questions/2124339/c-preprocessor-va-args-number-of-arguments
#ifdef _MSC_VER // Microsoft compilers
	#define GET_ARG_COUNT(...)  INTERNAL_EXPAND_ARGS_PRIVATE(INTERNAL_ARGS_AUGMENTER(__VA_ARGS__))

	#define INTERNAL_ARGS_AUGMENTER(...) unused, __VA_ARGS__
	#define INTERNAL_EXPAND(x) x
	#define INTERNAL_EXPAND_ARGS_PRIVATE(...) INTERNAL_EXPAND(INTERNAL_GET_ARG_COUNT_PRIVATE(__VA_ARGS__, 69, 68, 67, 66, 65, 64, 63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0))
	#define INTERNAL_GET_ARG_COUNT_PRIVATE(_1_, _2_, _3_, _4_, _5_, _6_, _7_, _8_, _9_, _10_, _11_, _12_, _13_, _14_, _15_, _16_, _17_, _18_, _19_, _20_, _21_, _22_, _23_, _24_, _25_, _26_, _27_, _28_, _29_, _30_, _31_, _32_, _33_, _34_, _35_, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, _59, _60, _61, _62, _63, _64, _65, _66, _67, _68, _69, _70, count, ...) count	
#else // Non-Microsoft compilers
	#define GET_ARG_COUNT(...) INTERNAL_GET_ARG_COUNT_PRIVATE(0, ## __VA_ARGS__, 70, 69, 68, 67, 66, 65, 64, 63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)
	#define INTERNAL_GET_ARG_COUNT_PRIVATE(_0, _1_, _2_, _3_, _4_, _5_, _6_, _7_, _8_, _9_, _10_, _11_, _12_, _13_, _14_, _15_, _16_, _17_, _18_, _19_, _20_, _21_, _22_, _23_, _24_, _25_, _26_, _27_, _28_, _29_, _30_, _31_, _32_, _33_, _34_, _35_, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, _59, _60, _61, _62, _63, _64, _65, _66, _67, _68, _69, _70, count, ...) count
#endif

#ifndef DEBUG_BUILD
	#if defined(_DEBUG) || defined(DEBUG)
		#define DEBUG_BUILD
	#endif
#endif

#ifdef DEBUG_BUILD
	#ifdef _MSC_VER
		#define DEBUG_BREAK() __debugbreak()
	#else
		#define DEBUG_BREAK() __builtin_trap()
	#endif
	#define DEBUG_BREAK_COND(m_cond) if (m_cond) DEBUG_BREAK()
#else 
	#define DEBUG_BREAK()
	#define DEBUG_BREAK_COND(m_cond)
#endif

#define _CRASH()                \
do {                            \
	char* CRASH_HERE = nullptr; \
	*CRASH_HERE = '\0';         \
} while(false)

#ifdef DEBUG_BUILD
	#define DEBUG_PRINT(m_msg)                                                                                        \
	do {                                                                                                              \
		printf("DEBUG_PRINT: \"%s\" at %s (%s:%i)\n", String("" m_msg).c_str(), __FUNCTION__, __FILE__, __LINE__);    \
	} while (false)
	#define DEBUG_PRINT_COND(m_cond, m_msg) do { if (m_cond) DEBUG_PRINT(m_msg); } while (false)

#ifdef __cplusplus
	#define ASSERT(m_cond)                                                                                            \
		do {                                                                                                          \
			if (!(m_cond)) {                                                                                          \
				DEBUG_BREAK();                                                                                        \
				THROW_ERROR(Error::BUG, String::format("ASSERTION FAILED: at %s (%s:%i)\n%s is false", __FUNCTION__, __FILE__, __LINE__, STR(m_cond)));  \
			}                                                                                                         \
		} while (false)
#endif // __cplusplus

#else
	#define DEBUG_PRINT(m_msg)
	#define DEBUG_PRINT_COND(m_cond, m_msg)
	#define ASSERT(m_cond)
#endif

#ifdef __cplusplus
#define MISSED_ENUM_CHECK(m_max_enum, m_max_value)                                                               \
    static_assert((int)m_max_enum == m_max_value, "MissedEnum: " STRINGIFY(m_max_enum) " != " STRINGIFY(m_value) \
        "\n\tat: " __FILE__ "(" STRINGIFY(__LINE__) ")")
#endif // __cplusplus


#define STR(m_) #m_
#define STRINGIFY(m_) STR(m_)
#define PLACE_HOLDER_MACRO
#define NO_ERRECT_MACRO(m) m
#define VSNPRINTF_BUFF_SIZE 8192

#ifdef __cplusplus
namespace carbon {
	typedef unsigned char byte_t;

	unsigned constexpr __const_hash(char const* input) {
		return *input ? static_cast<unsigned int>(*input) + 33 * __const_hash(input + 1) : 5381;
	}

	std::size_t constexpr operator "" _hash(const char* s, size_t) {
		return __const_hash(s);
	}

	// custom free operator for malloc in shared ptr
	struct _mem_free_t {
		void operator()(void* _mem) { free(_mem); }
	};

	// custom destructor for malloc in shared ptr
	struct _mem_delete_t {
		typedef void (*_clean_f)(void*);
		_mem_delete_t(_clean_f fp) : fp(fp) {}
		_clean_f fp;
		void operator()(void* _mem) {
			fp(_mem);
		}
	};

	template<typename T>
	using ptr = std::shared_ptr<T>;

	template<typename T>
	using stdvec = std::vector<T>;

	template<typename T1, typename T2>
	using stdmap = std::map<T1, T2>;

	template<typename T1, typename T2>
	using stdhashtable = std::unordered_map<T1, T2>;

	template<typename T, typename... Targs>
	inline ptr<T> newptr(Targs... p_args) {
		return std::make_shared<T>(p_args...);
	}
	
	template<typename T1, typename T2>
	inline ptr<T1> ptrcast(T2 p_ptr) {
		return std::static_pointer_cast<T1>(p_ptr);
	}

	template<typename T, typename... Targs>
	stdvec<T> make_stdvec(Targs... p_args) { return { p_args... }; }

} // namespace carbon
#endif // __cplusplus

#endif // CORE_INTERNAL_H


#ifndef _ERRORS_H
#define _ERRORS_H


#ifndef LOGGER_H
#define LOGGER_H

#include <stdarg.h> // for va_list

#ifndef CONSOLE_H
#define CONSOLE_H

//#include "internal.h"

namespace carbon {

class Console {
public:
	enum class Color {
		DEFAULT = -1,
		BLACK = 0,

		L_BLUE = 1,
		L_GREEN = 2,
		L_SKYBLUE = 3,
		L_RED = 4,
		L_PINK = 5,
		L_YELLOW = 6,
		L_WHITE = 7,
		L_GRAY = 8,

		D_BLUE = 9,
		D_GREEN = 10,
		D_SKYBLUE = 11,
		D_RED = 12,
		D_PINK = 13,
		D_YELLOW = 14,
		D_WHITE = 15,

		__COLOR_MAX__,
	};

	static void initialize();
	static void cleanup();

	static void set_cursor(int p_line, int p_column);
	static void get_cursor(int* p_line, int* p_column);

	static std::string getline();

	static void logf(const char* p_fmt, ...);
	static void logf_stderr(const char* p_fmt, ...);
	static void logf(const char* p_fmt, va_list p_args, bool p_stderr = false, Console::Color p_forground = Console::Color::DEFAULT, Console::Color p_background = Console::Color::DEFAULT);
	static void log(const char* p_message, bool p_stderr = false, Console::Color p_forground = Console::Color::DEFAULT, Console::Color p_background = Console::Color::DEFAULT);

private:

};


}

#endif // CONSOLE_H


// TODO: implement log to file, redirect error logs, ...

namespace carbon {

class Logger {
public:
	enum LogLevel 
	{
		JUST_LOG = 0,

		VERBOSE = 1,
		INFO    = 2,
		SUCCESS = 3,
		WARNING = 4,
		ERROR   = 5,
	};

	static void initialize();
	static void cleanup();

	static void set_level(LogLevel p_level);
	static void reset_level();
	static LogLevel get_level();
	static bool is_level(LogLevel p_level);
	
	static void log(const char* p_msg, Console::Color p_fg = Console::Color::L_WHITE, Console::Color p_bg = Console::Color::BLACK);
	static void log(const char* p_msg, LogLevel p_level, Console::Color p_fg = Console::Color::L_WHITE, Console::Color p_bg = Console::Color::BLACK);

	static void log_verbose(const char* p_msg);
	static void log_info(const char* p_msg);
	static void log_success(const char* p_msg);
	static void log_warning(const char* p_msg);
	static void log_error(const char* p_msg);

	static void logf_verbose(const char* p_fmt, ...);
	static void logf_info(const char* p_fmt, ...);
	static void logf_success(const char* p_fmt, ...);
	static void logf_warning(const char* p_fmt, ...);
	static void logf_error(const char* p_fmt, ...);

private:
	static LogLevel level;
	static LogLevel last_level;

	static void _log(const char* p_message, bool p_stderr, Console::Color p_forground = Console::Color::DEFAULT, Console::Color p_background = Console::Color::DEFAULT);
	static void _logf(const char* p_fmt, va_list p_args, bool p_err, Console::Color p_forground = Console::Color::DEFAULT, Console::Color p_background = Console::Color::DEFAULT);
};

}

#endif // LOGGER_H

//#include "internal.h"

#define _DBG_SOURCE DBGSourceInfo(__FILE__, __LINE__, __FUNCTION__)

#define THROW_INVALID_INDEX(m_size, m_ind)                                                              \
if (m_ind < 0 || m_size <= (size_t)m_ind) {                                                             \
	throw Error(Error::INVALID_INDEX, String::format("index %s = %lli is out of bounds (%s = %lli).",   \
				STRINGIFY(m_size), m_size, STRINGIFY(m_ind), m_ind),                                    \
		_DBG_SOURCE);                                                                                   \
} else ((void)0)

#define THROW_IF_NULLPTR(m_ptr)                                                                           \
if (m_ptr == nullptr){                                                                                    \
	throw Error(Error::INVALID_INDEX, String::format("the pointer \"%s\" is null.", STRINGIFY(m_ptr)),    \
		_DBG_SOURCE);                                                                                     \
} else ((void)0)


#define THROW_ERROR(m_type, m_msg) throw Error(m_type, m_msg, _DBG_SOURCE)
#define THROW_BUG(m_msg)                                                                \
	do{																	                \
		Logger::log_error((String("BUG : ") + m_msg + " (please report)\n").c_str());	\
		DEBUG_BREAK();													                \
		THROW_ERROR(Error::BUG, m_msg);									                \
	} while (false)


namespace carbon {

struct DBGSourceInfo {
	std::string func;
	std::string file;
	uint32_t line = 0;

	std::string line_before;
	std::string line_str;
	std::string line_after;

	std::pair<int, int> pos;
	uint32_t width = 0;

	DBGSourceInfo();
	DBGSourceInfo(const std::string& p_file, uint32_t p_line, const std::string& p_func);
	DBGSourceInfo(const std::string& p_file, const std::string& p_line_str, std::pair<int, int> p_pos, uint32_t p_width, const std::string& p_func = "");
	std::string get_pos_str() const;
};

class Throwable : public std::exception {
public:
	enum Kind {
		ERROR,
		COMPILE_TIME,
		WARNING,
		TRACEBACK,
	};

	enum Type {
		OK = 0,
		BUG,

		NULL_POINTER,
		OPERATOR_NOT_SUPPORTED,
		NOT_IMPLEMENTED,
		ZERO_DIVISION,
		TYPE_ERROR,
		VALUE_ERROR,
		ATTRIBUTE_ERROR,
		INVALID_ARG_COUNT,
		INVALID_INDEX,
		IO_ERROR,

		// compile time only errors
		SYNTAX_ERROR,
		ASSERTION,
		UNEXPECTED_EOF,
		NAME_ERROR,

		// warnings
		VARIABLE_SHADOWING,
		MISSED_ENUM_IN_SWITCH,
		NON_TERMINATING_LOOP,
		UNREACHABLE_CODE,
		STAND_ALONE_EXPRESSION,

		// runtime error
		RETHROW,
		STACK_OVERFLOW,

		_ERROR_MAX_,
	};

	Throwable(Type p_type, const std::string& p_what, const DBGSourceInfo& p_source_info = DBGSourceInfo());
	static std::string get_err_name(Throwable::Type p_type);

	const char* what() const noexcept override { return _what.c_str(); }
	virtual Kind get_kind() const = 0;
	virtual void console_log() const = 0;

	void set_source_info(const DBGSourceInfo& p_source_info);
	Type get_type() const { return _type; }

	void _add_nested(ptr<Throwable> p_err) { _nested = p_err; }
	Throwable* _get_nested() const { return _nested.get(); }
	//void _set_owner(Throwable* p_owner) { _owner = p_owner; }
	//Throwable* _get_owner()  const { return _owner; }

protected:
	Type _type;
	std::string _what;
	DBGSourceInfo source_info;

	mutable ptr<Throwable> _nested;
	//mutable Throwable* _owner = nullptr;
};

// ---------------------------------------------------

class Error : public Throwable {
public:
	Error(Type p_type, const std::string& p_what, const DBGSourceInfo& p_dbg_info = DBGSourceInfo());
	virtual Kind get_kind() const { return ERROR; }
	void console_log() const override;
};

class CompileTimeError : public Throwable {
public:
	CompileTimeError(Type p_type, const std::string& p_what,
		const DBGSourceInfo& p_cb_info = DBGSourceInfo(), const DBGSourceInfo& p_dbg_info = DBGSourceInfo());
	virtual Kind get_kind() const { return COMPILE_TIME; }
	void console_log() const override;

private:
	DBGSourceInfo _cb_dbg_info;
};

class Warning : public Throwable {
public:
	Warning(Type p_type, const std::string& p_what,
		const DBGSourceInfo& p_cb_info = DBGSourceInfo(), const DBGSourceInfo& p_dbg_info = DBGSourceInfo());
	virtual Kind get_kind() const { return WARNING; }
	void console_log() const override;

private:
	DBGSourceInfo _cb_dbg_info;
};

class TraceBack : public Throwable {
public:
	TraceBack(Type p_type, const std::string& p_what,
		const DBGSourceInfo& p_cb_info = DBGSourceInfo(), const DBGSourceInfo& p_dbg_info = DBGSourceInfo());
	TraceBack(const ptr<Throwable> p_nested,
		const DBGSourceInfo& p_cb_info = DBGSourceInfo(), const DBGSourceInfo& p_dbg_info = DBGSourceInfo());

	virtual Kind get_kind() const { return TRACEBACK; }
	void console_log() const override;

private:
	DBGSourceInfo _cb_dbg_info;
};


}

#endif // _ERRORS_H

//#include "core/logger.h"
//#include "core/console.h"

//#include "internal.h"
//#include "console.h"
//#include "logger.h"

#ifndef _PLATFORM_H
#define _PLATFORM_H

namespace carbon {

class _Platform {

public:
	static void console_get_cursor(int* p_line, int* p_column);
	static void console_set_cursor(int p_line, int p_column);
	static void console_logf(const char* p_fmt, va_list p_args, bool p_stderr = false, Console::Color p_forground = Console::Color::DEFAULT, Console::Color p_background = Console::Color::DEFAULT);
	static void console_log(const char* p_message, bool p_stderr = false, Console::Color p_forground = Console::Color::DEFAULT, Console::Color p_background = Console::Color::DEFAULT);

	static std::string os_getcwd();
	static void os_chdir(const std::string& p_path);

	static std::string path_absolute(const std::string& p_path);
	static bool path_exists(const std::string& p_path);
	static bool path_isdir(const std::string& p_path);
	static stdvec<std::string> path_listdir(const std::string& p_path);
};

}

#endif //_PLATFORM_H


#ifndef _TYPE_INFO_H
#define _TYPE_INFO_H


#ifndef _VAR_H
#define _VAR_H

//#include "core/_error.h"


#ifndef STRING_H
#define STRING_H

//#include "core/internal.h"

namespace carbon {

class var;
class Array;

class String {
public:

	// Constructors.
	String();
	String(const std::string& p_copy);
	String(const char* p_copy);
	String(const String& p_copy);
	String(char p_char);
	explicit String(int p_i);
	explicit String(int64_t p_i);
	explicit String(size_t p_i);
	explicit String(double p_d);
	~String();

	constexpr static  const char* get_type_name_s() { return "String"; }

	// reflection methods.
	var call_method(const String& p_method, const stdvec<var*>& p_args);
	static String format(const char* p_format, ...);
	int64_t to_int() const;
	double to_float() const;
	//String get_line(uint64_t p_line) const;
	size_t hash() const;
	size_t const_hash() const;

	String upper() const;
	String lower() const;
	String substr(int64_t p_start, int64_t p_end) const;
	bool endswith(const String& p_str) const;
	bool startswith(const String& p_str) const;
	Array split(const String& p_delimiter = "") const;
	String strip() const; // lstrip, rstrip
	String join(const Array& p_elements) const;
	String replace(const String& p_with, const String& p_what) const;
	int64_t find(const String& p_what, int64_t p_offset = 0) const;
	bool contains(const String& p_what) const;

	// operators.
	char operator[](int64_t p_index) const;
	char& operator[](int64_t p_index);

	// operator bool() {} don't implement this don't even delete
	operator std::string() const { return *_data; } // <-- TODO: remove this and find alternative.
	operator std::string& () { return *_data; }
	operator const std::string& () const { return *_data; }
	
	String operator %(const var& p_other) const;

	bool operator==(const String& p_other) const;
	bool operator!=(const String& p_other) const;
	bool operator<(const String& p_other) const;
	bool operator<=(const String& p_other) const;
	bool operator>(const String& p_other) const;
	bool operator>=(const String& p_other) const;

	String operator+(char p_c) const;
	String operator+(const char* p_cstr) const;
	String operator+(const String& p_other) const;
	//String operator+(int p_i) const;
	//String operator+(double p_d) const;
	// String operator+(var& p_obj) const          { TODO: }

	String& operator+=(char p_c);
	String& operator+=(const char* p_cstr);
	String& operator+=(const String& p_other);
	//String& operator+=(int p_i);
	//String& operator+=(double p_d);
	// String& operator+(var& p_obj)               { TODO: }

	String& operator=(char p_c);
	String& operator=(const char* p_cstr);
	String& operator=(const String& p_other);
	//String& operator=(int p_i);
	//String& operator=(double p_d);
	// String& operator=(var& p_obj)               { TODO: }

	// Wrappers.
	size_t size() const;
	const char* c_str() const;
	void* get_data();
	String& append(const String& p_other);

private:
	friend class var;
	friend std::ostream& operator<<(std::ostream& p_ostream, const String& p_str);
	friend std::istream& operator>>(std::istream& p_ostream, String& p_str);
	std::string* _data = nullptr;
};

// Global operators. TODO: implement more
bool operator==(const char* p_cstr, const String& p_str);
bool operator!=(const char* p_cstr, const String& p_str);


}

#endif // STRING_H


#ifndef  ARRAY_H
#define  ARRAY_H

//#include "core/internal.h"

namespace carbon {

class String;
class var;

class Array {
	friend class var;
public:
	constexpr static  const char* get_type_name_s() { return "Array"; }

	Array();
	//Array(const ptr<stdvec<var>>& p_data);
	//Array(const stdvec<var>& p_data);
	Array(const Array& p_copy);

	template <typename... Targs>
	Array(Targs... p_args) {
		_data = newptr<stdvec<var>>();
		_make_va_arg_array(p_args...);
	}

	Array copy(bool p_deep = true) const;
	var call_method(const String& p_method, const stdvec<var*>& p_args);
	const stdvec<var>* get_stdvec() const;
	void* get_data() const;

	// wrappers. TODO: throw errors
	size_t size() const;
	bool empty() const;
	void push_back(const var& p_var);
	void pop_back();
	Array& append(const var& p_var);
	var pop();
	void clear();
	void insert(int64_t p_index, const var& p_var);
	var& at(int64_t p_index);
	void resize(size_t p_size);
	void reserve(size_t p_size);
	Array& sort();
	var& back();
	var& front();
	bool contains(const var& p_elem);

	String to_string() const;
	operator bool() const;
	bool operator ==(const Array& p_other) const;
	Array& operator=(const Array& p_other);
	Array operator+(const Array& p_other) const;
	Array& operator+=(const Array& p_other);
	var& operator[](int64_t p_index) const;
	var& operator[](int64_t p_index);

private:
	ptr<std::vector<var>> _data;
	friend std::ostream& operator<<(std::ostream& p_ostream, const Array& p_arr);

	// va_args constructor internal
	template <typename T, typename... Targs>
	void _make_va_arg_array(T p_val, Targs... p_args) {
		push_back(p_val);
		_make_va_arg_array(p_args...);
	}
	void _make_va_arg_array() { return; }
};

}

#endif // ARRAY_H


#ifndef  MAP_H
#define  MAP_H

//#include "core/internal.h"

namespace carbon {

class String;
class var;

class Map {
	friend class var;
public:
	struct _KeyValue;
	struct _Comparator {
		bool operator() (const var& l, const var& r) const;
	};
	//typedef stdmap<size_t, _KeyValue> _map_internal_t;
	typedef std::map<var, var, _Comparator> _map_internal_t;

	constexpr static  const char* get_type_name_s() { return "Map"; }

	// methods.
	Map();
	Map(const ptr<_map_internal_t>& p_data);
	Map(const Map& p_copy);

	void* get_data() const;
	Map copy(bool p_deep = true) const;
	var call_method(const String& p_method, const stdvec<var*>& p_args);

	// Wrappers.
	size_t size() const;
	bool empty() const;
	void insert(const var& p_key, const var& p_value);
	void clear();
	bool has(const var& p_key) const;
	var at(const var& p_key) const;

	String to_string() const;
	operator bool() const;
	bool operator ==(const Map& p_other) const;
	Map& operator=(const Map& p_other);
	var operator[](const var& p_key) const;
	var& operator[](const var& p_key);
	var operator[](const char* p_key) const;
	var& operator[](const char* p_key);

private:
	_map_internal_t::iterator begin() const;
	_map_internal_t::iterator end() const;
	_map_internal_t::iterator find(const var& p_key) const;

	ptr<_map_internal_t> _data;
	friend std::ostream& operator<<(std::ostream& p_ostream, const Map& p_dict);
};

}

#endif // MAP_H


#ifndef OBJECT_H
#define OBJECT_H

//#include "core/internal.h"

#define REGISTER_CLASS(m_class, m_inherits)                                                          \
public:                                                                                              \
	typedef m_inherits Super;                                                                        \
	static ptr<Object> __new() { return newptr<m_class>(); }                                         \
	static constexpr const char* get_type_name_s() { return STR(m_class); }                          \
	static constexpr const char* get_base_type_name_s() { return STR(m_inherits); }                  \
	virtual const char* get_base_type_name() const override { return get_base_type_name_s(); }       \
	virtual const char* get_type_name() const override { return get_type_name_s(); }                 \
	static void _bind_data(NativeClasses* p_native_classes)

namespace carbon {

class var;
class String;
class NativeClasses;

class Object {
public:

	// REGISTER_CLASS methods.
	static ptr<Object> __new() { return newptr<Object>(); }
	constexpr static  const char* get_type_name_s() { return "Object"; }
	constexpr static  const char* get_base_type_name_s() { return ""; }
	virtual const char* get_type_name() const { return get_type_name_s(); }
	virtual const char* get_base_type_name() const { return get_base_type_name_s(); }
	static void _bind_data(NativeClasses* p_native_classes);

	// Operators.
	Object& operator=(const Object& p_copy) = default;
	operator String(); // const
	var operator()(stdvec<var*>& p_args);

	bool operator==(const var& p_other) /*const*/;
	bool operator!=(const var& p_other) /*const*/;
	bool operator<=(const var& p_other) /*const*/;
	bool operator>=(const var& p_other) /*const*/;
	bool operator< (const var& p_other) /*const*/;
	bool operator> (const var& p_other) /*const*/;

	var operator+(const var& p_other) /*const*/;
	var operator-(const var& p_other) /*const*/;
	var operator*(const var& p_other) /*const*/;
	var operator/(const var& p_other) /*const*/;

	var& operator+=(const var& p_other);
	var& operator-=(const var& p_other);
	var& operator*=(const var& p_other);
	var& operator/=(const var& p_other);

	var operator[](const var& p_key) const;
	var& operator[](const var& p_key);

	// TODO: move them to native
	static var call_method_s(ptr<Object>& p_self, const String& p_name, stdvec<var*>& p_args);
	static var get_member_s(ptr<Object>& p_self, const String& p_name);
	static void set_member_s(ptr<Object>& p_self, const String& p_name, var& p_value);

	virtual var call_method(const String& p_method_name, stdvec<var*>& p_args);
	virtual var get_member(const String& p_member_name);
	virtual void set_member(const String& p_member_name, var& p_value);

	virtual ptr<Object> copy(bool p_deep) /*const*/;
	virtual void* get_data();

	// operators.
	virtual var __call(stdvec<var*>& p_vars);

	virtual var __iter_begin();
	virtual bool __iter_has_next();
	virtual var __iter_next();

	virtual var __get_mapped(const var& p_key) /*const*/;
	virtual void __set_mapped(const var& p_key, const var& p_val);
	virtual int64_t __hash() /*const*/;

	virtual var __add(const var& p_other) /*const*/;
	virtual var __sub(const var& p_other) /*const*/;
	virtual var __mul(const var& p_other) /*const*/;
	virtual var __div(const var& p_other) /*const*/;

	virtual var& __add_eq(const var& p_other);
	virtual var& __sub_eq(const var& p_other);
	virtual var& __mul_eq(const var& p_other);
	virtual var& __div_eq(const var& p_other);

	virtual bool __gt(const var& p_other) /*const*/;
	virtual bool __lt(const var& p_other) /*const*/;
	virtual bool __eq(const var& p_other) /*const*/;

	virtual String to_string() /*const*/;

	// this could be a bad design but a better workaround for now instead of using is_instance_of<Type>();
	virtual bool _is_native_ref() const;
	virtual String _get_native_ref() const;
	virtual bool _is_registered() const;

private:
	friend class var;
};

}


#endif //OBJECT_H


#ifndef VECTOR_H
#define VECTOR_H

//#include "core/internal.h"

#define VECT2_DEFAULT_CMP_OP_OVERLOAD(m_op)            \
bool operator m_op (const Vect2<T>& p_other) const {   \
	return get_length() m_op p_other.get_length();     \
}
#define VECT3_DEFAULT_CMP_OP_OVERLOAD(m_op)            \
bool operator m_op (const Vect3<T>& p_other) const {   \
	return get_length() m_op p_other.get_length();     \
}

namespace carbon {

class MemberInfo;

template<typename T>
struct Vect2 {
	union { T x, width; };
	union { T y, height; };

	// Constructors
	Vect2(T p_x = 0, T p_y = 0) : x(p_x), y(p_y) {}
	Vect2(const T* p_arr) : x(p_arr[0]), y(p_arr[1]) {}
	template<typename T2>
	Vect2(const Vect2<T2>& p_copy) : x((T)p_copy.x), y((T)p_copy.y) {}
	constexpr static  const char* get_type_name_s() { return "Vect2<T>"; }

	// Methods.
	double get_length() const {
		return sqrtf((x * x + y * y));
	}
	float get_angle() const {
		if (x == 0) return (float)M_PI_2;
		return atanf(y / x);
	}

	// Operators.
	Vect2<T> operator+(const Vect2<T>& p_other) const {
		return Vect2<T>(x + p_other.x, y + p_other.y);
	}
	Vect2<T> operator-(const Vect2<T>& p_other) const {
		return Vect2<T>(x - p_other.x, y - p_other.y);
	}
	Vect2<T> operator*(const Vect2<T>& p_other) const {
		return Vect2<T>(x * p_other.x, y * p_other.y);
	}
	template <typename T2>
	Vect2<T> operator*(T2 p_val) const {
		return Vect2<T>(x * p_val, y * p_val);
	}
	Vect2<T> operator/(const Vect2<T>& p_other) const {
		return Vect2<T>(x / p_other.x, y / p_other.y);
	}

	VECT2_DEFAULT_CMP_OP_OVERLOAD(<);
	VECT2_DEFAULT_CMP_OP_OVERLOAD(<=);
	VECT2_DEFAULT_CMP_OP_OVERLOAD(>);
	VECT2_DEFAULT_CMP_OP_OVERLOAD(>=);
	bool operator==(const Vect2<T>& p_other) const {
		return x == p_other.x && y == p_other.y;
	}
	bool operator!=(const Vect2<T>& p_other) const {
		return !operator==(p_other);
	}

	Vect2<T>& operator+=(const Vect2<T>& p_other) {
		x += p_other.x; y += p_other.y;
		return *this;
	}
	Vect2<T>& operator-=(const Vect2<T>& p_other) {
		x -= p_other.x; y -= p_other.y;
		return *this;
	}

	Vect2<T>& operator*=(const Vect2<T>& p_other) {
		x *= p_other.x; y *= p_other.y;
		return *this;
	}

	Vect2<T>& operator/=(const Vect2<T>& p_other) {
		x /= p_other.x; y /= p_other.y;
		return *this;
	}

	operator bool() const { return x == 0 && y == 0; }
	String to_string() const {
		return String("Vect2(")
			.append(std::to_string(x)).append(", ")
			.append(std::to_string(y)).append(")");
	}
};


template<typename T>
struct Vect3 {
	union { T x, width; };
	union { T y, height; };
	union { T z, depth; };

	// Constructors.
	Vect3(T p_x = 0, T p_y = 0, T p_z = 0) : x(p_x), y(p_y), z(p_z) {}
	Vect3(const T* p_arr) : x(p_arr[0]), y(p_arr[1]), z(p_arr[2]) {}
	template<typename T2>
	Vect3(const Vect3<T2>& p_copy) : x((T)p_copy.x), y((T)p_copy.y), z((T)p_copy.z) {}
	constexpr static  const char* get_type_name_s() { return "Vect3<T>"; }

	// Methods.
	double get_length() const {
		return sqrtf((x * x + y * y + z * z));
	}

	// Operators.
	Vect3<T> operator+(const Vect3<T>& p_other) const {
		return Vect3<T>(x + p_other.x, y + p_other.y, z + p_other.z);
	}
	Vect3<T> operator-(const Vect3<T>& p_other) const {
		return Vect3<T>(x - p_other.x, y - p_other.y, z - p_other.z);
	}
	Vect3<T> operator*(const Vect3<T>& p_other) const {
		return Vect3<T>(x * p_other.x, y * p_other.y, z * p_other.z);
	}
	Vect3<T> operator/(const Vect3<T>& p_other) const {
		return Vect3<T>(x / p_other.x, y / p_other.y, z / p_other.z);
	}

	VECT2_DEFAULT_CMP_OP_OVERLOAD(<);
	VECT2_DEFAULT_CMP_OP_OVERLOAD(<=);
	VECT2_DEFAULT_CMP_OP_OVERLOAD(>);
	VECT2_DEFAULT_CMP_OP_OVERLOAD(>=);
	bool operator==(const Vect3<T>& p_other) const {
		return x == p_other.x && y == p_other.y && z == p_other.z;
	}
	bool operator!=(const Vect3<T>& p_other) const {
		return !operator==(p_other);
	}

	Vect3<T>& operator+=(const Vect3<T>& p_other) {
		x += p_other.x; y += p_other.y; z += p_other.z;
		return *this;
	}
	Vect3<T>& operator-=(const Vect3<T>& p_other) {
		x -= p_other.x; y -= p_other.y; z -= p_other.z;
		return *this;
	}
	Vect3<T>& operator*=(const Vect3<T>& p_other) {
		x *= p_other.x; y *= p_other.y; z *= p_other.z;
		return *this;
	}
#define M_OP_TEMPLATE(T2)                    \
	Vect3<T>& operator*=(T2 p_val) const {   \
		x *= p_val; y *= p_val; z *= p_val;  \
		return *this;                        \
	}
	M_OP_TEMPLATE(float)
	M_OP_TEMPLATE(int)
#undef M_OP_TEMPLATE

	template<typename T2>
	Vect3<T>& operator/=(const Vect3<T2>& p_other) {
		x /= (T)p_other.x; y /= (T)p_other.y; z /= (T)p_other.z;
		return *this;
	}

	operator bool() const { return x == 0 && y == 0 && z == 0; }
	String to_string() const {
		return String("Vect3(")
			.append(std::to_string(x)).append(", ")
			.append(std::to_string(y)).append(", ")
			.append(std::to_string(z)).append(")");
	}
};

template<typename T>
std::ostream& operator<<(std::ostream& p_ostream, const Vect2<T>& p_vect) {
	p_ostream << p_vect.to_string();
	return p_ostream;
}
template<typename T>
std::ostream& operator<<(std::ostream& p_ostream, const Vect3<T>& p_vect) {
	p_ostream << p_vect.to_string();
	return p_ostream;
}

/* typedefs */
typedef Vect2<double> Vect2f;
typedef Vect2<int64_t> Vect2i;
typedef Vect3<double> Vect3f;
typedef Vect3<int64_t> Vect3i;

typedef Vect2f Size;
typedef Vect2f Point;

}

#endif //VECTOR_H


#define DATA_PTR_CONST(T) reinterpret_cast<const T *>(_data._mem)
#define DATA_PTR_OTHER_CONST(T) reinterpret_cast<const T *>(p_other._data._mem)

#define DATA_PTR(T) reinterpret_cast<T *>(_data._mem)
#define DATA_PTR_OTHER(T) reinterpret_cast<T *>(p_other._data._mem)

#define _TRY_VAR_STL(m_statement)                    \
do {												 \
	try {											 \
		m_statement;								 \
	} catch (std::exception& err) {					 \
		THROW_ERROR(Error::VALUE_ERROR, err.what()); \
	}												 \
} while (false)

namespace carbon {

class var {
public:
	enum Type {
		_NULL, // not initialized.
		VAR,   // any type used only for member info.

		BOOL,
		INT,
		FLOAT,
		STRING,

		// misc types
		ARRAY,
		MAP,
		OBJECT,

		_TYPE_MAX_,
	};

	enum Operator {

		// methamatical operators
		OP_ASSIGNMENT,
		OP_ADDITION,
		OP_SUBTRACTION,
		OP_MULTIPLICATION,
		OP_DIVISION,
		OP_MODULO,
		OP_POSITIVE, // unary
		OP_NEGATIVE, // unary

		// locical operators
		OP_EQ_CHECK,
		OP_NOT_EQ_CHECK,
		OP_LT,
		OP_LTEQ,
		OP_GT,
		OP_GTEQ,
		OP_AND,
		OP_OR,
		OP_NOT, // unary

		// bitwise operators
		OP_BIT_LSHIFT,
		OP_BIT_RSHIFT,
		OP_BIT_AND,
		OP_BIT_OR,
		OP_BIT_XOR,
		OP_BIT_NOT, // unary

		_OP_MAX_,
	};

	/* constructors */
	var();
	var(const var& p_copy);
	var(bool p_bool);
	var(int p_int);
	var(size_t p_int);
	var(int64_t p_int);
	var(float p_float);
	var(double p_double);
	var(const char* p_cstring);
	var(const String& p_string);
	var(const Array& p_array);
	var(const Map& p_map);
	var(const ptr<Object>& p_other);
	~var();
	
	template <typename T=Object>
	var(const ptr<T>& p_ptr) {
		type = OBJECT;
		new(&_data._obj) ptr<Object>(p_ptr);
	}

	static bool is_hashable(var::Type p_type);
	static bool is_compatible(var::Type p_type1, var::Type p_type2);
	static const char* get_type_name_s(var::Type p_type);
	static String get_op_name_s(Operator op);

	// methods.
	Type get_type() const;
	String get_type_name() const;
	void* get_data();
	size_t hash() const;
	void clear();
	var copy(bool p_deep = false) const;
	String to_string() const;

	// Operators.
	operator bool() const;
	operator int64_t() const;
	operator double() const;
	operator String() const;
	operator Array() const;
	operator Map() const;
	// operator const char* () const; <-- never implement this

	template <typename T>
	operator ptr<T>() const {
		if (type != OBJECT) {
			THROW_ERROR(Error::TYPE_ERROR, String::format("can't cast \"%s\" to \"Object\".", get_type_name().c_str()));
		}

		// TODO: check if type compatable
		return ptrcast<T>(_data._obj);
	}

	operator int();
	operator size_t();
	operator float();
	operator int () const;
	operator size_t () const;
	operator float() const;

	operator bool* ();
	operator int64_t* ();
	operator double* ();
	operator String* ();
	operator Array* ();
	operator Map* ();

	operator bool&();
	operator int64_t&();
	operator double&();
	operator String&();
	operator Array&();
	operator Map&();

	operator const String& () const;
	operator const Array& () const;
	operator const Map& () const;
	

#define _VAR_OP_DECL(m_ret, m_op, m_access)                                                        \
	m_ret operator m_op (bool p_other) m_access { return operator m_op (var(p_other)); }           \
	m_ret operator m_op (int64_t p_other) m_access { return operator m_op (var(p_other)); }        \
	m_ret operator m_op (int p_other)     m_access { return operator m_op (var(p_other)); }        \
	m_ret operator m_op (double p_other) m_access { return operator m_op (var(p_other)); }         \
	m_ret operator m_op (const char* p_other) m_access { return operator m_op (var(p_other)); }    \
	m_ret operator m_op (const var& p_other) m_access
#define VAR_OP_DECL(m_ret, m_op, m_access) _VAR_OP_DECL(m_ret, m_op, m_access)

	VAR_OP_DECL(bool, ==, const);
	VAR_OP_DECL(bool, !=, const);
	VAR_OP_DECL(bool, < , const);
	VAR_OP_DECL(bool, > , const);
	VAR_OP_DECL(bool, <=, const);
	VAR_OP_DECL(bool, >=, const);

	var operator++();
	var operator++(int);
	var operator--();
	var operator--(int);
	bool operator !() const { return !operator bool(); }
	var operator[](const var& p_key) const;
	var operator[](int index) const;
	// var& operator[](const var& p_key); // TODO:

	var __get_mapped(const var& p_key) const;
	void __set_mapped(const var& p_key, const var& p_value);

	var __iter_begin();
	bool __iter_has_next();
	var __iter_next();

	//template <typename... Targs>
	//var __call(Targs... p_args) {
	//	stdvec<var> _args = make_stdvec<var>(p_args...);
	//	stdvec<var*> args; for (var& v : _args) args.push_back(&v);
	//	return __call_internal(args);
	//}
	//template <typename... Targs>
	//var operator()(Targs... p_args) {
	//	return __call(p_args...);
	//}

	//template <typename... Targs>
	//var call_method(const String& p_method, Targs... p_args) {
	//	stdvec<var> _args = make_stdvec<var>(p_args...);
	//	stdvec<var*> args; for (var& v : _args) args.push_back(&v);
	//	return call_method_internal(p_method, args);
	//}

	var __call(stdvec<var*>& p_args);
	var call_method(const String& p_method, stdvec<var*>& p_args);

	var get_member(const String& p_name);
	void set_member(const String& p_name, var& p_value);

	VAR_OP_DECL(var, +, const);
	VAR_OP_DECL(var, -, const);
	VAR_OP_DECL(var, *, const);
	VAR_OP_DECL(var, /, const);
	VAR_OP_DECL(var, %, const);

	/* assignments */
	var& operator=(const var& p_other);
	template<typename T=Object>
	var& operator=(const ptr<T>& p_other) {
		clear_data();
		type = OBJECT;
		new(&_data._obj) ptr<Object>(ptrcast<Object>(p_other));
		return *this;
	}
	VAR_OP_DECL(var&, +=, PLACE_HOLDER_MACRO);
	VAR_OP_DECL(var&, -=, PLACE_HOLDER_MACRO);
	VAR_OP_DECL(var&, *=, PLACE_HOLDER_MACRO);
	VAR_OP_DECL(var&, /=, PLACE_HOLDER_MACRO);
	VAR_OP_DECL(var&, %=, PLACE_HOLDER_MACRO);

private:
	struct VarData {
		VarData() : _float(.0f) {}
		~VarData() {}


		union {
			ptr<Object> _obj;
			Map _map;
			Array _arr;
			String _string;

			bool _bool = false;
			int64_t _int;
			double _float;
		};
	};

	// private methods.
	void copy_data(const var& p_other);
	void clear_data();

	// private members.
	Type type = _NULL;
	VarData _data;
	friend std::ostream& operator<<(std::ostream& p_ostream, const var& p_var);
};


// THIS NEEDS TO BE DEFINED HERE (NEED VAR DEFINITION BEFORE)
struct Map::_KeyValue {
	var key;
	var value;
	_KeyValue() {}
	_KeyValue(const var& p_key, const var& p_value) : key(p_key), value(p_value) {}
};

}

#endif // _VAR_H


namespace carbon {

class MemberInfo {
public:
	enum Type {
		METHOD,
		PROPERTY,
		ENUM,
		ENUM_VALUE,
		CLASS,
	};

	virtual Type get_type() const = 0;
	virtual const String& get_name() const = 0;
};

struct VarTypeInfo {
	var::Type type = var::_NULL;
	const char* class_name = "";
	VarTypeInfo(var::Type p_type = var::VAR) : type(p_type) {}
	VarTypeInfo(var::Type p_type, const char* p_class_name) : type(p_type), class_name(p_class_name) {}

	bool operator==(const VarTypeInfo p_other) const {
		if (type != var::OBJECT) return type == p_other.type;
		return strcmp(class_name, p_other.class_name) == 0;
	}
	bool operator!=(const VarTypeInfo p_other) const { return !(operator==(p_other)); }
};

class ClassInfo : public MemberInfo, public Object {
	REGISTER_CLASS(ClassInfo, Object);
	public: ClassInfo() {} // default constructor needed for inherit Object

private:
	String name;
	var _class; // compiled version of the class.
public:
	ClassInfo(const String& p_name, const var& p_class) :name(p_name), _class(p_class) { }
	virtual Type get_type() const { return  CLASS; };
	virtual const String& get_name() const { return name; };

	bool _is_registered() const override { return false; }
	var get_member(const String& p_name) override {
		switch (p_name.const_hash()) {
			case "name"_hash: return name;
			case "_class"_hash: return _class;
			default: return Super::get_member(p_name);
		}
	}
};

class MethodInfo : public MemberInfo, public Object {
	REGISTER_CLASS(MethodInfo, Object);
public: MethodInfo() {} // default constructor needed for inherit Object

private:
	String name;
	bool _is_static = false;
	int arg_count = 0; // -1 is va args, -2 is unknown
	stdvec<String> arg_names;
	stdvec<var> default_args;
	stdvec<VarTypeInfo> arg_types;
	VarTypeInfo return_type;

	// to call a function using method info
	// we need method bind
	void* _bind = nullptr;

public:
	virtual Type get_type() const override { return Type::METHOD; }
	virtual const String& get_name() const override { return name; }

	// complete constructor.
	MethodInfo(
		String p_name,
		stdvec<String> p_arg_names,
		stdvec<VarTypeInfo> p_arg_types = stdvec<VarTypeInfo>(),
		VarTypeInfo p_return_type = var::_NULL,
		bool p__is_static = false,
		stdvec<var> p_default_args = stdvec<var>(),
		int p_arg_count = -2
	) {
		name = p_name;
		arg_names = p_arg_names;
		arg_types = p_arg_types;
		return_type = p_return_type;
		_is_static = p__is_static;
		default_args = p_default_args;

		// if va_arg it should be set manually to -1
		if (p_arg_count == -2) arg_count = (int)arg_names.size();
		else arg_count = p_arg_count;
	}

	// zero parameter constructor
	MethodInfo(
		String p_name,
		VarTypeInfo p_return_type = var::_NULL,
		bool p__is_static = false
	) {
		name = p_name;
		return_type = p_return_type;
		_is_static = p__is_static;
	}

	void _set_bind(void* p_bind) { _bind = p_bind; }
	void* _get_bind() { return _bind; }

	// defined in native.cpp
	var __call(stdvec<var*>& p_args) override;

	int get_arg_count() const { return arg_count; }
	int get_default_arg_count() const { return (int)default_args.size(); }
	bool is_static() const { return _is_static; }
	const stdvec<String>& get_arg_names() const { return arg_names; }
	const stdvec<var>& get_default_args() const { return default_args; }
	const stdvec<VarTypeInfo>& get_arg_types() const { return arg_types; }
	VarTypeInfo get_return_type() const { return return_type; }

	bool _is_registered() const override { return false; }
	var get_member(const String& p_name) override {
		switch (p_name.const_hash()) {
			case "name"_hash: return name;
			case "is_static"_hash: return _is_static;
			case "arg_count"_hash: return arg_count;
			case "default_arg_count"_hash: return default_args.size();
			default: return Super::get_member(p_name);
		}
	}
};

class PropertyInfo : public MemberInfo, public Object {
	REGISTER_CLASS(PropertyInfo, Object);
	public: PropertyInfo() {} // default constructor needed for inherit Object

private:
	String name;
	VarTypeInfo datatype;
	var value;
	bool _is_const = false;
	bool _is_static = false;

	void* _bind = nullptr;

public:
	virtual Type get_type() const override { return Type::PROPERTY; }
	virtual const String& get_name() const override { return name; }

	PropertyInfo(
		const String& p_name,
		VarTypeInfo p_datatype = var::VAR,
		var p_value = var(),
		bool p__is_const = false,
		bool p__is_static = false
	) {
		name = p_name;
		datatype = p_datatype;
		value = p_value;
		_is_const = p__is_const;
		_is_static = p__is_static;
	}

	void _set_bind(void* p_bind) { _bind = p_bind; }
	void* _get_bind() { return _bind; }

	// defined in native.cpp
	var __call(stdvec<var*>& p_args) override;

	bool is_static() const { return _is_static; }
	bool is_const() const { return _is_const; }
	VarTypeInfo get_datatype() const { return datatype; }
	const var& get_value() const { return value; }         // value for constants.
	const var& get_default_value() const { return value; } // defalut_value for variables.

	bool _is_registered() const override { return false; }
	var get_member(const String& p_name) override {
		switch (p_name.const_hash()) {
			case "name"_hash: return name;
			case "value"_hash: return value;
			default: return Super::get_member(p_name);
		}
	}
};

class EnumInfo : public MemberInfo, public Object {
	REGISTER_CLASS(EnumInfo, Object);
	public: EnumInfo() {} // default constructor needed for inherit Object

private:
	String _name;
	stdmap<String, int64_t> _values;

public:
	virtual Type get_type() const override { return Type::ENUM; }
	virtual const String& get_name() const override { return _name; }

	EnumInfo(const String& p_name) : _name(p_name) {}
	EnumInfo(const String& p_name, const stdvec<std::pair<String, int64_t>>& p_values) {
		_name = p_name;
		for (const std::pair<String, int64_t>& p : p_values) {
			_values[p.first] = p.second;
		}
	}

	bool _is_registered() const override { return false; }
	var get_member(const String& p_name) override {
		stdmap<String, int64_t>::iterator it = _values.find(p_name);
		if (it != _values.end()) return it->second;
		return Super::get_member(p_name);
	}
	void set_member(const String& p_name, var& p_value) override {
		stdmap<String, int64_t>::iterator it = _values.find(p_name);
		if (it != _values.end()) throw "TODO:";// TODO: more throw_error to _type_info.cpp => THROW_ERROR(Error::ATTRIBUTE_ERROR, String::format("cannot assign a value to enum value."));
		else Super::set_member(p_name, p_value);
	}
	var call_method(const String& p_name, stdvec<var*>& p_args) override {
		switch (p_name.const_hash()) {
			case "as_map"_hash: {
				Map as_map;
				for (auto p : _values) {
					as_map[p.first] = p.second;
				}
				return as_map;
			} break;
		}
		return Super::call_method(p_name, p_args);
	}

	const stdmap<String, int64_t>& get_values() const { return _values; }
	stdmap<String, int64_t>& get_edit_values() { return _values; }
};

class EnumValueInfo : public MemberInfo {
private:
	String name;
	int64_t value;

public:
	virtual Type get_type() const override { return Type::ENUM_VALUE; }
	virtual const String& get_name() const override { return name; }

	EnumValueInfo(const String& p_name, int64_t p_value) {
		name = p_name;
		value = p_value;
	}

	int64_t get_value() const { return value; }
};


// ---------------------------------------------------------------------------------

class TypeInfo {

public:
	static const ptr<MemberInfo> get_member_info(const var& p_var, const String& p_name);
	static const stdmap<size_t, ptr<MemberInfo>>& get_member_info_list(const var& p_var);

	static const ptr<MemberInfo> get_member_info(var::Type p_type, const String& p_name);
	static const stdmap<size_t, ptr<MemberInfo>>& get_member_info_list(var::Type p_type);

	static const ptr<MemberInfo> get_member_info_object(const Object* p_instance, const String& p_member);
	static const stdmap<size_t, ptr<MemberInfo>>& get_member_info_list_object(const Object* p_instance);

	static const ptr<MemberInfo> get_member_info_string(const String& p_member);
	static const stdmap<size_t, ptr<MemberInfo>>& get_member_info_list_string();

	static const ptr<MemberInfo> get_member_info_array(const String& p_member);
	static const stdmap<size_t, ptr<MemberInfo>>& get_member_info_list_array();

	static const ptr<MemberInfo> get_member_info_map(const String& p_member);
	static const stdmap<size_t, ptr<MemberInfo>>& get_member_info_list_map();

};




}

#endif // _TYPE_INFO_H


#ifndef NATIVE_CLASSES_H
#define NATIVE_CLASSES_H

//#include "var/var.h"
//#include "type_info.h"

// !!! AUTO GENERATED DO NOT EDIT !!!

#ifndef NATIVE_BIND_GEN_H
#define NATIVE_BIND_GEN_H

namespace carbon {


template<typename T> struct is_shared_ptr : std::false_type {};
template<typename T> struct is_shared_ptr<ptr<T>> : std::true_type {};

#define DECLARE_VAR_TYPE(m_var_type, m_T) 	VarTypeInfo m_var_type = _remove_and_get_type_info<m_T>();


template <typename T, typename std::enable_if<std::is_same<T, void>::value, bool>::type = true>
VarTypeInfo _get_type_info() { return var::_NULL; }

template <typename T, typename std::enable_if<std::is_integral<T>::value
			&& !std::is_const<T>::value && !std::is_reference<T>::value, bool>::type = true>
VarTypeInfo _get_type_info() {
	if (std::is_same<T, bool>::value) return var::BOOL;
	return var::INT;
}

template <typename T, typename std::enable_if<std::is_floating_point<T>::value
			&& !std::is_const<T>::value && !std::is_reference<T>::value, bool>::type = true>
VarTypeInfo _get_type_info() { return var::FLOAT; }

template <typename T, typename std::enable_if<std::is_same<T, String>::value, bool>::type = true>
VarTypeInfo _get_type_info() { return var::STRING; }

template <typename T, typename std::enable_if<std::is_same<T, const char*>::value, bool>::type = true>
VarTypeInfo _get_type_info() { return var::STRING; }

template <typename T, typename std::enable_if<std::is_same<T, Array>::value, bool>::type = true>
VarTypeInfo _get_type_info() { return var::ARRAY; }

template <typename T, typename std::enable_if<std::is_same<T, Map>::value, bool>::type = true>
VarTypeInfo _get_type_info() { return var::MAP; }

template <typename T, typename std::enable_if<std::is_same<T, var>::value, bool>::type = true>
VarTypeInfo _get_type_info() { return var::VAR; }

template <typename T, typename std::enable_if<is_shared_ptr<T>::value, bool>::type = true>
VarTypeInfo _get_type_info() { return { var::OBJECT, T::element_type::get_type_name_s() }; }

// -- for no overloaded method found compiler errors ------------
template <typename T, typename std::enable_if<std::is_reference<T>::value, bool>::type = true>
VarTypeInfo _get_type_info() { throw "INTERNAL BUG"; }
template <typename T, typename std::enable_if<std::is_const<T>::value, bool>::type = true>
VarTypeInfo _get_type_info() { throw "INTERNAL BUG"; }
template <typename T, typename std::enable_if<std::is_pointer<T>::value
	&& !std::is_same<T, const char*>::value, bool>::type = true>
VarTypeInfo _get_type_info() { throw "INTERNAL BUG"; }
// --------------------------------------------------------------

template <typename T>
VarTypeInfo _remove_and_get_type_info() {

	if (std::is_reference<T>::value) {
		return _remove_and_get_type_info<typename std::remove_reference<T>::type>();

	} else if (std::is_const<T>::value){
		return _remove_and_get_type_info<typename std::remove_const<T>::type>();

	} else if (std::is_pointer<T>::value){
		if (std::is_same<T, const char*>::value) {
			return _get_type_info<T>();
		} else {
			return _remove_and_get_type_info<typename std::remove_pointer<T>::type>();
		}

	} else {
		return _get_type_info<T>();
	}
	/* done return here : let compiler warn if missed anything */
}


//template <typename T, typename std::enable_if<std::is_same<T, Array>::value, bool>::type = true>
//VarTypeInfo _get_type_info() { return var::ARRAY; }


class BindData {
protected:
	const char* name;
	const char* class_name;

public:
	enum Type {
		METHOD,
		STATIC_FUNC,
		MEMBER_VAR,
		STATIC_VAR,
		STATIC_CONST,
		ENUM,
		ENUM_VALUE,
	};
	virtual Type get_type() const = 0;
	virtual const char* get_name() const { return name; }
	virtual const char* get_type_name() const { return class_name; }
	virtual int get_argc() const { DEBUG_BREAK(); THROW_ERROR(Error::BUG, "invalid call"); }
	virtual const ptr<MemberInfo> get_member_info() const = 0;
};

class MethodBind : public BindData {
protected:
	int argc = 0;
	ptr<MethodInfo> mi;

public:
	virtual BindData::Type get_type() const { return BindData::METHOD; }
	virtual int get_argc() const { return argc; }

	virtual var call(Object* self, stdvec<var*>& args) const = 0;
	const ptr<MethodInfo> get_method_info() const { return mi; }
	const ptr<MemberInfo> get_member_info() const override { return mi; }
};

class StaticFuncBind : public BindData {
protected:
	int argc;
	ptr<MethodInfo> mi;

public:
	virtual BindData::Type get_type()   const { return BindData::STATIC_FUNC; }
	virtual int get_argc()              const { return argc; }

	virtual var call(stdvec<var*>& args) const = 0;
	const ptr<MethodInfo> get_method_info() const { return mi; }
	const ptr<MemberInfo> get_member_info() const override { return mi; }
};

// ---------------- MEMBER BIND START --------------------------------------
class PropertyBind : public BindData {
protected:
	ptr<PropertyInfo> pi;
public:
	virtual BindData::Type get_type() const { return BindData::MEMBER_VAR; }
	virtual var& get(Object* self) const = 0;

	const ptr<PropertyInfo> get_prop_info() const { return pi; }
	const ptr<MemberInfo> get_member_info() const override { return pi; }
};

template<typename Class>
class _PropertyBind : public PropertyBind {
protected:
	typedef var Class::* member_ptr_t;
	member_ptr_t member_ptr;
public:
	_PropertyBind(const char* p_name, const char* p_class_name, member_ptr_t p_member_ptr, ptr<PropertyInfo> p_pi) {
		name = p_name;
		class_name = p_class_name;
		member_ptr = p_member_ptr;
		pi = p_pi;
		pi->_set_bind((void*)this);
	}

	virtual var& get(Object* self) const override {
		return ((Class*)(self))->*member_ptr;
	}
};

template<typename Class>
ptr<PropertyBind> _bind_member(const char* p_name, const char* p_class_name, var Class::* p_member_ptr, const var& p_value = var()) {
	var Class::* member_ptr = p_member_ptr;
	return newptr<_PropertyBind<Class>>(p_name, p_class_name, member_ptr, newptr<PropertyInfo>(p_name, var::VAR, p_value));
}
// ------------------------------------------------------------------------


// ---------------- STATIC MEMBER BIND START ------------------------------
class StaticPropertyBind : public BindData {
	var* member = nullptr;
	ptr<PropertyInfo> pi;
public:
	virtual BindData::Type get_type() const { return BindData::STATIC_VAR; }

	StaticPropertyBind(const char* p_name, const char* p_class_name, var* p_member, ptr<PropertyInfo> p_pi) {
		name = p_name;
		class_name = p_class_name;
		member = p_member;
		pi = p_pi;
		pi->_set_bind((void*)this);
	}
	virtual var& get() const { return *member; }
	const ptr<PropertyInfo> get_prop_info() const { return pi; }
	const ptr<MemberInfo> get_member_info() const override { return pi; }
};

inline ptr<StaticPropertyBind> _bind_static_member(const char* p_name, const char* p_class_name, var* p_member) {
	return newptr<StaticPropertyBind>(p_name, p_class_name, p_member, newptr<PropertyInfo>(p_name, var::VAR, *p_member));
}
// ------------------------------------------------------------------------

// ---------------- STATIC CONST BIND START ------------------------------
class ConstantBind : public BindData {
protected:
	ptr<PropertyInfo> pi;
public:
	virtual BindData::Type get_type() const { return BindData::STATIC_CONST; }
	virtual var get() const = 0;

	const ptr<PropertyInfo> get_prop_info() const { return pi; }
	const ptr<MemberInfo> get_member_info() const override { return pi; }
};

template<typename T>
class _ConstantBind : public ConstantBind {
	T* _const = nullptr;
public:
	_ConstantBind(const char* p_name, const char* p_class_name, T* p_const, ptr<PropertyInfo> p_pi) {
		name = p_name;
		class_name = p_class_name;
		_const = p_const;
		pi = p_pi;
		pi->_set_bind((void*)this);
	}

	virtual var get() const override {
		return *_const;
	}
};

template<typename T>
ptr<ConstantBind> _bind_static_const(const char* p_name, const char* p_class_name, T* p_const) {
	DECLARE_VAR_TYPE(datatype, T);
	return newptr<_ConstantBind<T>>(p_name, p_class_name, p_const, newptr<PropertyInfo>(p_name, datatype, *p_const));
}
// ------------------------------------------------------------------------

// ---------------- ENUM BIND START ------------------------------

class EnumBind : public BindData {
	ptr<EnumInfo> ei;
public:
	EnumBind(const char* p_name, const char* p_class_name, ptr<EnumInfo> p_ei) {
		name = p_name;
		class_name = p_class_name;
		ei = p_ei;
	}
	virtual BindData::Type get_type() const { return BindData::ENUM; }
	int64_t get(const String& p_value_name) const {
		const stdmap<String, int64_t>& values = ei->get_values();
		stdmap<String, int64_t>::const_iterator it = values.find(p_value_name);
		if (it != values.end()) return it->second;
		throw Error(Error::ATTRIBUTE_ERROR, String::format("value \"%s\" isn't exists on enum %s.", p_value_name.c_str(), name));
	}
	const ptr<EnumInfo> get() const { return ei; }
	const ptr<EnumInfo> get_enum_info() const { return ei; }
	const ptr<MemberInfo> get_member_info() const override { return ei; }
};
inline ptr<EnumBind> _bind_enum(const char* p_name, const char* p_class_name, const stdvec<std::pair<String, int64_t>>& p_values) {
	return newptr<EnumBind>(p_name, p_class_name, newptr<EnumInfo>(p_name, p_values));
}

class EnumValueBind : public BindData {
	ptr<EnumValueInfo> evi;
	int64_t value;
public:
	EnumValueBind(const char* p_name, const char* p_class_name, int64_t p_value, ptr<EnumValueInfo> p_evi) {
		name = p_name;
		class_name = p_class_name;
		value = p_value;
		evi = newptr<EnumValueInfo>(p_name, p_value);
	}
	virtual BindData::Type get_type() const { return BindData::ENUM_VALUE; }
	int64_t get() const { return value; }

	const ptr<EnumValueInfo> get_enum_value_info() const { return evi; }
	const ptr<MemberInfo> get_member_info() const override { return evi; }
};

// -----------------------------------------------------------------------

template<typename T, typename R>
using M0 = R(T::*)();

template<typename T, typename R>
using M0_c = R(T::*)() const;

template<typename T, typename R, typename a0>
using M1 = R(T::*)(a0);

template<typename T, typename R, typename a0>
using M1_c = R(T::*)(a0) const;

template<typename T, typename R, typename a0, typename a1>
using M2 = R(T::*)(a0, a1);

template<typename T, typename R, typename a0, typename a1>
using M2_c = R(T::*)(a0, a1) const;

template<typename T, typename R, typename a0, typename a1, typename a2>
using M3 = R(T::*)(a0, a1, a2);

template<typename T, typename R, typename a0, typename a1, typename a2>
using M3_c = R(T::*)(a0, a1, a2) const;

template<typename T, typename R, typename a0, typename a1, typename a2, typename a3>
using M4 = R(T::*)(a0, a1, a2, a3);

template<typename T, typename R, typename a0, typename a1, typename a2, typename a3>
using M4_c = R(T::*)(a0, a1, a2, a3) const;

template<typename T, typename R, typename a0, typename a1, typename a2, typename a3, typename a4>
using M5 = R(T::*)(a0, a1, a2, a3, a4);

template<typename T, typename R, typename a0, typename a1, typename a2, typename a3, typename a4>
using M5_c = R(T::*)(a0, a1, a2, a3, a4) const;

template<typename T, typename R, typename a0, typename a1, typename a2, typename a3, typename a4, typename a5>
using M6 = R(T::*)(a0, a1, a2, a3, a4, a5);

template<typename T, typename R, typename a0, typename a1, typename a2, typename a3, typename a4, typename a5>
using M6_c = R(T::*)(a0, a1, a2, a3, a4, a5) const;

template<typename T, typename R, typename a0, typename a1, typename a2, typename a3, typename a4, typename a5, typename a6>
using M7 = R(T::*)(a0, a1, a2, a3, a4, a5, a6);

template<typename T, typename R, typename a0, typename a1, typename a2, typename a3, typename a4, typename a5, typename a6>
using M7_c = R(T::*)(a0, a1, a2, a3, a4, a5, a6) const;

template<typename R>
using F0 = R(*)();

template<typename R, typename a0>
using F1 = R(*)(a0);

template<typename R, typename a0, typename a1>
using F2 = R(*)(a0, a1);

template<typename R, typename a0, typename a1, typename a2>
using F3 = R(*)(a0, a1, a2);

template<typename R, typename a0, typename a1, typename a2, typename a3>
using F4 = R(*)(a0, a1, a2, a3);

template<typename R, typename a0, typename a1, typename a2, typename a3, typename a4>
using F5 = R(*)(a0, a1, a2, a3, a4);

template<typename R, typename a0, typename a1, typename a2, typename a3, typename a4, typename a5>
using F6 = R(*)(a0, a1, a2, a3, a4, a5);

template<typename R, typename a0, typename a1, typename a2, typename a3, typename a4, typename a5, typename a6>
using F7 = R(*)(a0, a1, a2, a3, a4, a5, a6);


template <typename T, class M, typename _TRet, typename std::enable_if<!std::is_same<_TRet, void>::value, bool>::type = true>
inline var _internal_call_method_0(Object* self, const M& method, stdvec<var*>& args) {
	return (((T*)self)->*method)();
}
template <typename T, class M, typename _TRet, typename std::enable_if<std::is_same<_TRet, void>::value, bool>::type = true>
inline var _internal_call_method_0(Object* self, const M& method, stdvec<var*>& args) {
	(((T*)self)->*method)(); return var();
}

template<typename T, typename R>
class _MethodBind_M0 : public MethodBind {
	typedef M0<T, R> _Tmethod;
	_Tmethod method;
public:
	_MethodBind_M0(const char* p_name, const char* p_class_name, int p_argc, M0<T, R> p_method, ptr<MethodInfo> p_mi) {
		name = p_name;
		class_name = p_class_name;
		argc = p_argc;
		method = p_method;
		mi = p_mi;
		mi->_set_bind((void*)this);
	}
	virtual var call(Object* self, stdvec<var*>& args) const override {

		int default_arg_count = mi->get_default_arg_count();
		int args_given = (int)args.size();
		if (args_given + default_arg_count < 0) { /* Args not enough. */
			if (default_arg_count == 0) THROW_ERROR(Error::INVALID_ARG_COUNT, "expected at exactly 0 argument(s).");
			else THROW_ERROR(Error::INVALID_ARG_COUNT, String::format("expected at least %i argument(s).", 0 - default_arg_count));
		} else if (args_given > 0) { /* More args proveded.	*/
			if (default_arg_count == 0) THROW_ERROR(Error::INVALID_ARG_COUNT, "expected at exactly 0 argument(s).");
			else THROW_ERROR(Error::INVALID_ARG_COUNT, String::format( "expected minimum of %i argument(s) and maximum of 0 argument(s).", 0 - default_arg_count));
		}

		stdvec<var> default_args_copy;
		for (int i = 0 - args_given; i > 0 ; i--) {
			default_args_copy.push_back(mi->get_default_args()[default_arg_count - i]);
		}
		for (var& v : default_args_copy) args.push_back(&v);

		return _internal_call_method_0<T, _Tmethod, R>(self, method, args);
	}
};
template<typename T, typename R>
class _MethodBind_M0_c : public MethodBind {
	typedef M0_c<T, R> _Tmethod;
	_Tmethod method;
public:
	_MethodBind_M0_c(const char* p_name, const char* p_class_name, int p_argc, M0_c<T, R> p_method, ptr<MethodInfo> p_mi) {
		name = p_name;
		class_name = p_class_name;
		argc = p_argc;
		method = p_method;
		mi = p_mi;
		mi->_set_bind((void*)this);
	}
	virtual var call(Object* self, stdvec<var*>& args) const override {

		int default_arg_count = mi->get_default_arg_count();
		int args_given = (int)args.size();
		if (args_given + default_arg_count < 0) { /* Args not enough. */
			if (default_arg_count == 0) THROW_ERROR(Error::INVALID_ARG_COUNT, "expected at exactly 0 argument(s).");
			else THROW_ERROR(Error::INVALID_ARG_COUNT, String::format("expected at least %i argument(s).", 0 - default_arg_count));
		} else if (args_given > 0) { /* More args proveded.	*/
			if (default_arg_count == 0) THROW_ERROR(Error::INVALID_ARG_COUNT, "expected at exactly 0 argument(s).");
			else THROW_ERROR(Error::INVALID_ARG_COUNT, String::format( "expected minimum of %i argument(s) and maximum of 0 argument(s).", 0 - default_arg_count));
		}

		stdvec<var> default_args_copy;
		for (int i = 0 - args_given; i > 0 ; i--) {
			default_args_copy.push_back(mi->get_default_args()[default_arg_count - i]);
		}
		for (var& v : default_args_copy) args.push_back(&v);

		return _internal_call_method_0<T, _Tmethod, R>(self, method, args);
	}
};

template <typename T, class M, typename _TRet, typename std::enable_if<!std::is_same<_TRet, void>::value, bool>::type = true>
inline var _internal_call_method_1(Object* self, const M& method, stdvec<var*>& args) {
	return (((T*)self)->*method)(*args[0]);
}
template <typename T, class M, typename _TRet, typename std::enable_if<std::is_same<_TRet, void>::value, bool>::type = true>
inline var _internal_call_method_1(Object* self, const M& method, stdvec<var*>& args) {
	(((T*)self)->*method)(*args[0]); return var();
}

template<typename T, typename R, typename a0>
class _MethodBind_M1 : public MethodBind {
	typedef M1<T, R, a0> _Tmethod;
	_Tmethod method;
public:
	_MethodBind_M1(const char* p_name, const char* p_class_name, int p_argc, M1<T, R, a0> p_method, ptr<MethodInfo> p_mi) {
		name = p_name;
		class_name = p_class_name;
		argc = p_argc;
		method = p_method;
		mi = p_mi;
		mi->_set_bind((void*)this);
	}
	virtual var call(Object* self, stdvec<var*>& args) const override {

		int default_arg_count = mi->get_default_arg_count();
		int args_given = (int)args.size();
		if (args_given + default_arg_count < 1) { /* Args not enough. */
			if (default_arg_count == 0) THROW_ERROR(Error::INVALID_ARG_COUNT, "expected at exactly 1 argument(s).");
			else THROW_ERROR(Error::INVALID_ARG_COUNT, String::format("expected at least %i argument(s).", 1 - default_arg_count));
		} else if (args_given > 1) { /* More args proveded.	*/
			if (default_arg_count == 0) THROW_ERROR(Error::INVALID_ARG_COUNT, "expected at exactly 1 argument(s).");
			else THROW_ERROR(Error::INVALID_ARG_COUNT, String::format( "expected minimum of %i argument(s) and maximum of 1 argument(s).", 1 - default_arg_count));
		}

		stdvec<var> default_args_copy;
		for (int i = 1 - args_given; i > 0 ; i--) {
			default_args_copy.push_back(mi->get_default_args()[default_arg_count - i]);
		}
		for (var& v : default_args_copy) args.push_back(&v);

		return _internal_call_method_1<T, _Tmethod, R>(self, method, args);
	}
};
template<typename T, typename R, typename a0>
class _MethodBind_M1_c : public MethodBind {
	typedef M1_c<T, R, a0> _Tmethod;
	_Tmethod method;
public:
	_MethodBind_M1_c(const char* p_name, const char* p_class_name, int p_argc, M1_c<T, R, a0> p_method, ptr<MethodInfo> p_mi) {
		name = p_name;
		class_name = p_class_name;
		argc = p_argc;
		method = p_method;
		mi = p_mi;
		mi->_set_bind((void*)this);
	}
	virtual var call(Object* self, stdvec<var*>& args) const override {

		int default_arg_count = mi->get_default_arg_count();
		int args_given = (int)args.size();
		if (args_given + default_arg_count < 1) { /* Args not enough. */
			if (default_arg_count == 0) THROW_ERROR(Error::INVALID_ARG_COUNT, "expected at exactly 1 argument(s).");
			else THROW_ERROR(Error::INVALID_ARG_COUNT, String::format("expected at least %i argument(s).", 1 - default_arg_count));
		} else if (args_given > 1) { /* More args proveded.	*/
			if (default_arg_count == 0) THROW_ERROR(Error::INVALID_ARG_COUNT, "expected at exactly 1 argument(s).");
			else THROW_ERROR(Error::INVALID_ARG_COUNT, String::format( "expected minimum of %i argument(s) and maximum of 1 argument(s).", 1 - default_arg_count));
		}

		stdvec<var> default_args_copy;
		for (int i = 1 - args_given; i > 0 ; i--) {
			default_args_copy.push_back(mi->get_default_args()[default_arg_count - i]);
		}
		for (var& v : default_args_copy) args.push_back(&v);

		return _internal_call_method_1<T, _Tmethod, R>(self, method, args);
	}
};

template <typename T, class M, typename _TRet, typename std::enable_if<!std::is_same<_TRet, void>::value, bool>::type = true>
inline var _internal_call_method_2(Object* self, const M& method, stdvec<var*>& args) {
	return (((T*)self)->*method)(*args[0], *args[1]);
}
template <typename T, class M, typename _TRet, typename std::enable_if<std::is_same<_TRet, void>::value, bool>::type = true>
inline var _internal_call_method_2(Object* self, const M& method, stdvec<var*>& args) {
	(((T*)self)->*method)(*args[0], *args[1]); return var();
}

template<typename T, typename R, typename a0, typename a1>
class _MethodBind_M2 : public MethodBind {
	typedef M2<T, R, a0, a1> _Tmethod;
	_Tmethod method;
public:
	_MethodBind_M2(const char* p_name, const char* p_class_name, int p_argc, M2<T, R, a0, a1> p_method, ptr<MethodInfo> p_mi) {
		name = p_name;
		class_name = p_class_name;
		argc = p_argc;
		method = p_method;
		mi = p_mi;
		mi->_set_bind((void*)this);
	}
	virtual var call(Object* self, stdvec<var*>& args) const override {

		int default_arg_count = mi->get_default_arg_count();
		int args_given = (int)args.size();
		if (args_given + default_arg_count < 2) { /* Args not enough. */
			if (default_arg_count == 0) THROW_ERROR(Error::INVALID_ARG_COUNT, "expected at exactly 2 argument(s).");
			else THROW_ERROR(Error::INVALID_ARG_COUNT, String::format("expected at least %i argument(s).", 2 - default_arg_count));
		} else if (args_given > 2) { /* More args proveded.	*/
			if (default_arg_count == 0) THROW_ERROR(Error::INVALID_ARG_COUNT, "expected at exactly 2 argument(s).");
			else THROW_ERROR(Error::INVALID_ARG_COUNT, String::format( "expected minimum of %i argument(s) and maximum of 2 argument(s).", 2 - default_arg_count));
		}

		stdvec<var> default_args_copy;
		for (int i = 2 - args_given; i > 0 ; i--) {
			default_args_copy.push_back(mi->get_default_args()[default_arg_count - i]);
		}
		for (var& v : default_args_copy) args.push_back(&v);

		return _internal_call_method_2<T, _Tmethod, R>(self, method, args);
	}
};
template<typename T, typename R, typename a0, typename a1>
class _MethodBind_M2_c : public MethodBind {
	typedef M2_c<T, R, a0, a1> _Tmethod;
	_Tmethod method;
public:
	_MethodBind_M2_c(const char* p_name, const char* p_class_name, int p_argc, M2_c<T, R, a0, a1> p_method, ptr<MethodInfo> p_mi) {
		name = p_name;
		class_name = p_class_name;
		argc = p_argc;
		method = p_method;
		mi = p_mi;
		mi->_set_bind((void*)this);
	}
	virtual var call(Object* self, stdvec<var*>& args) const override {

		int default_arg_count = mi->get_default_arg_count();
		int args_given = (int)args.size();
		if (args_given + default_arg_count < 2) { /* Args not enough. */
			if (default_arg_count == 0) THROW_ERROR(Error::INVALID_ARG_COUNT, "expected at exactly 2 argument(s).");
			else THROW_ERROR(Error::INVALID_ARG_COUNT, String::format("expected at least %i argument(s).", 2 - default_arg_count));
		} else if (args_given > 2) { /* More args proveded.	*/
			if (default_arg_count == 0) THROW_ERROR(Error::INVALID_ARG_COUNT, "expected at exactly 2 argument(s).");
			else THROW_ERROR(Error::INVALID_ARG_COUNT, String::format( "expected minimum of %i argument(s) and maximum of 2 argument(s).", 2 - default_arg_count));
		}

		stdvec<var> default_args_copy;
		for (int i = 2 - args_given; i > 0 ; i--) {
			default_args_copy.push_back(mi->get_default_args()[default_arg_count - i]);
		}
		for (var& v : default_args_copy) args.push_back(&v);

		return _internal_call_method_2<T, _Tmethod, R>(self, method, args);
	}
};

template <typename T, class M, typename _TRet, typename std::enable_if<!std::is_same<_TRet, void>::value, bool>::type = true>
inline var _internal_call_method_3(Object* self, const M& method, stdvec<var*>& args) {
	return (((T*)self)->*method)(*args[0], *args[1], *args[2]);
}
template <typename T, class M, typename _TRet, typename std::enable_if<std::is_same<_TRet, void>::value, bool>::type = true>
inline var _internal_call_method_3(Object* self, const M& method, stdvec<var*>& args) {
	(((T*)self)->*method)(*args[0], *args[1], *args[2]); return var();
}

template<typename T, typename R, typename a0, typename a1, typename a2>
class _MethodBind_M3 : public MethodBind {
	typedef M3<T, R, a0, a1, a2> _Tmethod;
	_Tmethod method;
public:
	_MethodBind_M3(const char* p_name, const char* p_class_name, int p_argc, M3<T, R, a0, a1, a2> p_method, ptr<MethodInfo> p_mi) {
		name = p_name;
		class_name = p_class_name;
		argc = p_argc;
		method = p_method;
		mi = p_mi;
		mi->_set_bind((void*)this);
	}
	virtual var call(Object* self, stdvec<var*>& args) const override {

		int default_arg_count = mi->get_default_arg_count();
		int args_given = (int)args.size();
		if (args_given + default_arg_count < 3) { /* Args not enough. */
			if (default_arg_count == 0) THROW_ERROR(Error::INVALID_ARG_COUNT, "expected at exactly 3 argument(s).");
			else THROW_ERROR(Error::INVALID_ARG_COUNT, String::format("expected at least %i argument(s).", 3 - default_arg_count));
		} else if (args_given > 3) { /* More args proveded.	*/
			if (default_arg_count == 0) THROW_ERROR(Error::INVALID_ARG_COUNT, "expected at exactly 3 argument(s).");
			else THROW_ERROR(Error::INVALID_ARG_COUNT, String::format( "expected minimum of %i argument(s) and maximum of 3 argument(s).", 3 - default_arg_count));
		}

		stdvec<var> default_args_copy;
		for (int i = 3 - args_given; i > 0 ; i--) {
			default_args_copy.push_back(mi->get_default_args()[default_arg_count - i]);
		}
		for (var& v : default_args_copy) args.push_back(&v);

		return _internal_call_method_3<T, _Tmethod, R>(self, method, args);
	}
};
template<typename T, typename R, typename a0, typename a1, typename a2>
class _MethodBind_M3_c : public MethodBind {
	typedef M3_c<T, R, a0, a1, a2> _Tmethod;
	_Tmethod method;
public:
	_MethodBind_M3_c(const char* p_name, const char* p_class_name, int p_argc, M3_c<T, R, a0, a1, a2> p_method, ptr<MethodInfo> p_mi) {
		name = p_name;
		class_name = p_class_name;
		argc = p_argc;
		method = p_method;
		mi = p_mi;
		mi->_set_bind((void*)this);
	}
	virtual var call(Object* self, stdvec<var*>& args) const override {

		int default_arg_count = mi->get_default_arg_count();
		int args_given = (int)args.size();
		if (args_given + default_arg_count < 3) { /* Args not enough. */
			if (default_arg_count == 0) THROW_ERROR(Error::INVALID_ARG_COUNT, "expected at exactly 3 argument(s).");
			else THROW_ERROR(Error::INVALID_ARG_COUNT, String::format("expected at least %i argument(s).", 3 - default_arg_count));
		} else if (args_given > 3) { /* More args proveded.	*/
			if (default_arg_count == 0) THROW_ERROR(Error::INVALID_ARG_COUNT, "expected at exactly 3 argument(s).");
			else THROW_ERROR(Error::INVALID_ARG_COUNT, String::format( "expected minimum of %i argument(s) and maximum of 3 argument(s).", 3 - default_arg_count));
		}

		stdvec<var> default_args_copy;
		for (int i = 3 - args_given; i > 0 ; i--) {
			default_args_copy.push_back(mi->get_default_args()[default_arg_count - i]);
		}
		for (var& v : default_args_copy) args.push_back(&v);

		return _internal_call_method_3<T, _Tmethod, R>(self, method, args);
	}
};

template <typename T, class M, typename _TRet, typename std::enable_if<!std::is_same<_TRet, void>::value, bool>::type = true>
inline var _internal_call_method_4(Object* self, const M& method, stdvec<var*>& args) {
	return (((T*)self)->*method)(*args[0], *args[1], *args[2], *args[3]);
}
template <typename T, class M, typename _TRet, typename std::enable_if<std::is_same<_TRet, void>::value, bool>::type = true>
inline var _internal_call_method_4(Object* self, const M& method, stdvec<var*>& args) {
	(((T*)self)->*method)(*args[0], *args[1], *args[2], *args[3]); return var();
}

template<typename T, typename R, typename a0, typename a1, typename a2, typename a3>
class _MethodBind_M4 : public MethodBind {
	typedef M4<T, R, a0, a1, a2, a3> _Tmethod;
	_Tmethod method;
public:
	_MethodBind_M4(const char* p_name, const char* p_class_name, int p_argc, M4<T, R, a0, a1, a2, a3> p_method, ptr<MethodInfo> p_mi) {
		name = p_name;
		class_name = p_class_name;
		argc = p_argc;
		method = p_method;
		mi = p_mi;
		mi->_set_bind((void*)this);
	}
	virtual var call(Object* self, stdvec<var*>& args) const override {

		int default_arg_count = mi->get_default_arg_count();
		int args_given = (int)args.size();
		if (args_given + default_arg_count < 4) { /* Args not enough. */
			if (default_arg_count == 0) THROW_ERROR(Error::INVALID_ARG_COUNT, "expected at exactly 4 argument(s).");
			else THROW_ERROR(Error::INVALID_ARG_COUNT, String::format("expected at least %i argument(s).", 4 - default_arg_count));
		} else if (args_given > 4) { /* More args proveded.	*/
			if (default_arg_count == 0) THROW_ERROR(Error::INVALID_ARG_COUNT, "expected at exactly 4 argument(s).");
			else THROW_ERROR(Error::INVALID_ARG_COUNT, String::format( "expected minimum of %i argument(s) and maximum of 4 argument(s).", 4 - default_arg_count));
		}

		stdvec<var> default_args_copy;
		for (int i = 4 - args_given; i > 0 ; i--) {
			default_args_copy.push_back(mi->get_default_args()[default_arg_count - i]);
		}
		for (var& v : default_args_copy) args.push_back(&v);

		return _internal_call_method_4<T, _Tmethod, R>(self, method, args);
	}
};
template<typename T, typename R, typename a0, typename a1, typename a2, typename a3>
class _MethodBind_M4_c : public MethodBind {
	typedef M4_c<T, R, a0, a1, a2, a3> _Tmethod;
	_Tmethod method;
public:
	_MethodBind_M4_c(const char* p_name, const char* p_class_name, int p_argc, M4_c<T, R, a0, a1, a2, a3> p_method, ptr<MethodInfo> p_mi) {
		name = p_name;
		class_name = p_class_name;
		argc = p_argc;
		method = p_method;
		mi = p_mi;
		mi->_set_bind((void*)this);
	}
	virtual var call(Object* self, stdvec<var*>& args) const override {

		int default_arg_count = mi->get_default_arg_count();
		int args_given = (int)args.size();
		if (args_given + default_arg_count < 4) { /* Args not enough. */
			if (default_arg_count == 0) THROW_ERROR(Error::INVALID_ARG_COUNT, "expected at exactly 4 argument(s).");
			else THROW_ERROR(Error::INVALID_ARG_COUNT, String::format("expected at least %i argument(s).", 4 - default_arg_count));
		} else if (args_given > 4) { /* More args proveded.	*/
			if (default_arg_count == 0) THROW_ERROR(Error::INVALID_ARG_COUNT, "expected at exactly 4 argument(s).");
			else THROW_ERROR(Error::INVALID_ARG_COUNT, String::format( "expected minimum of %i argument(s) and maximum of 4 argument(s).", 4 - default_arg_count));
		}

		stdvec<var> default_args_copy;
		for (int i = 4 - args_given; i > 0 ; i--) {
			default_args_copy.push_back(mi->get_default_args()[default_arg_count - i]);
		}
		for (var& v : default_args_copy) args.push_back(&v);

		return _internal_call_method_4<T, _Tmethod, R>(self, method, args);
	}
};

template <typename T, class M, typename _TRet, typename std::enable_if<!std::is_same<_TRet, void>::value, bool>::type = true>
inline var _internal_call_method_5(Object* self, const M& method, stdvec<var*>& args) {
	return (((T*)self)->*method)(*args[0], *args[1], *args[2], *args[3], *args[4]);
}
template <typename T, class M, typename _TRet, typename std::enable_if<std::is_same<_TRet, void>::value, bool>::type = true>
inline var _internal_call_method_5(Object* self, const M& method, stdvec<var*>& args) {
	(((T*)self)->*method)(*args[0], *args[1], *args[2], *args[3], *args[4]); return var();
}

template<typename T, typename R, typename a0, typename a1, typename a2, typename a3, typename a4>
class _MethodBind_M5 : public MethodBind {
	typedef M5<T, R, a0, a1, a2, a3, a4> _Tmethod;
	_Tmethod method;
public:
	_MethodBind_M5(const char* p_name, const char* p_class_name, int p_argc, M5<T, R, a0, a1, a2, a3, a4> p_method, ptr<MethodInfo> p_mi) {
		name = p_name;
		class_name = p_class_name;
		argc = p_argc;
		method = p_method;
		mi = p_mi;
		mi->_set_bind((void*)this);
	}
	virtual var call(Object* self, stdvec<var*>& args) const override {

		int default_arg_count = mi->get_default_arg_count();
		int args_given = (int)args.size();
		if (args_given + default_arg_count < 5) { /* Args not enough. */
			if (default_arg_count == 0) THROW_ERROR(Error::INVALID_ARG_COUNT, "expected at exactly 5 argument(s).");
			else THROW_ERROR(Error::INVALID_ARG_COUNT, String::format("expected at least %i argument(s).", 5 - default_arg_count));
		} else if (args_given > 5) { /* More args proveded.	*/
			if (default_arg_count == 0) THROW_ERROR(Error::INVALID_ARG_COUNT, "expected at exactly 5 argument(s).");
			else THROW_ERROR(Error::INVALID_ARG_COUNT, String::format( "expected minimum of %i argument(s) and maximum of 5 argument(s).", 5 - default_arg_count));
		}

		stdvec<var> default_args_copy;
		for (int i = 5 - args_given; i > 0 ; i--) {
			default_args_copy.push_back(mi->get_default_args()[default_arg_count - i]);
		}
		for (var& v : default_args_copy) args.push_back(&v);

		return _internal_call_method_5<T, _Tmethod, R>(self, method, args);
	}
};
template<typename T, typename R, typename a0, typename a1, typename a2, typename a3, typename a4>
class _MethodBind_M5_c : public MethodBind {
	typedef M5_c<T, R, a0, a1, a2, a3, a4> _Tmethod;
	_Tmethod method;
public:
	_MethodBind_M5_c(const char* p_name, const char* p_class_name, int p_argc, M5_c<T, R, a0, a1, a2, a3, a4> p_method, ptr<MethodInfo> p_mi) {
		name = p_name;
		class_name = p_class_name;
		argc = p_argc;
		method = p_method;
		mi = p_mi;
		mi->_set_bind((void*)this);
	}
	virtual var call(Object* self, stdvec<var*>& args) const override {

		int default_arg_count = mi->get_default_arg_count();
		int args_given = (int)args.size();
		if (args_given + default_arg_count < 5) { /* Args not enough. */
			if (default_arg_count == 0) THROW_ERROR(Error::INVALID_ARG_COUNT, "expected at exactly 5 argument(s).");
			else THROW_ERROR(Error::INVALID_ARG_COUNT, String::format("expected at least %i argument(s).", 5 - default_arg_count));
		} else if (args_given > 5) { /* More args proveded.	*/
			if (default_arg_count == 0) THROW_ERROR(Error::INVALID_ARG_COUNT, "expected at exactly 5 argument(s).");
			else THROW_ERROR(Error::INVALID_ARG_COUNT, String::format( "expected minimum of %i argument(s) and maximum of 5 argument(s).", 5 - default_arg_count));
		}

		stdvec<var> default_args_copy;
		for (int i = 5 - args_given; i > 0 ; i--) {
			default_args_copy.push_back(mi->get_default_args()[default_arg_count - i]);
		}
		for (var& v : default_args_copy) args.push_back(&v);

		return _internal_call_method_5<T, _Tmethod, R>(self, method, args);
	}
};

template <typename T, class M, typename _TRet, typename std::enable_if<!std::is_same<_TRet, void>::value, bool>::type = true>
inline var _internal_call_method_6(Object* self, const M& method, stdvec<var*>& args) {
	return (((T*)self)->*method)(*args[0], *args[1], *args[2], *args[3], *args[4], *args[5]);
}
template <typename T, class M, typename _TRet, typename std::enable_if<std::is_same<_TRet, void>::value, bool>::type = true>
inline var _internal_call_method_6(Object* self, const M& method, stdvec<var*>& args) {
	(((T*)self)->*method)(*args[0], *args[1], *args[2], *args[3], *args[4], *args[5]); return var();
}

template<typename T, typename R, typename a0, typename a1, typename a2, typename a3, typename a4, typename a5>
class _MethodBind_M6 : public MethodBind {
	typedef M6<T, R, a0, a1, a2, a3, a4, a5> _Tmethod;
	_Tmethod method;
public:
	_MethodBind_M6(const char* p_name, const char* p_class_name, int p_argc, M6<T, R, a0, a1, a2, a3, a4, a5> p_method, ptr<MethodInfo> p_mi) {
		name = p_name;
		class_name = p_class_name;
		argc = p_argc;
		method = p_method;
		mi = p_mi;
		mi->_set_bind((void*)this);
	}
	virtual var call(Object* self, stdvec<var*>& args) const override {

		int default_arg_count = mi->get_default_arg_count();
		int args_given = (int)args.size();
		if (args_given + default_arg_count < 6) { /* Args not enough. */
			if (default_arg_count == 0) THROW_ERROR(Error::INVALID_ARG_COUNT, "expected at exactly 6 argument(s).");
			else THROW_ERROR(Error::INVALID_ARG_COUNT, String::format("expected at least %i argument(s).", 6 - default_arg_count));
		} else if (args_given > 6) { /* More args proveded.	*/
			if (default_arg_count == 0) THROW_ERROR(Error::INVALID_ARG_COUNT, "expected at exactly 6 argument(s).");
			else THROW_ERROR(Error::INVALID_ARG_COUNT, String::format( "expected minimum of %i argument(s) and maximum of 6 argument(s).", 6 - default_arg_count));
		}

		stdvec<var> default_args_copy;
		for (int i = 6 - args_given; i > 0 ; i--) {
			default_args_copy.push_back(mi->get_default_args()[default_arg_count - i]);
		}
		for (var& v : default_args_copy) args.push_back(&v);

		return _internal_call_method_6<T, _Tmethod, R>(self, method, args);
	}
};
template<typename T, typename R, typename a0, typename a1, typename a2, typename a3, typename a4, typename a5>
class _MethodBind_M6_c : public MethodBind {
	typedef M6_c<T, R, a0, a1, a2, a3, a4, a5> _Tmethod;
	_Tmethod method;
public:
	_MethodBind_M6_c(const char* p_name, const char* p_class_name, int p_argc, M6_c<T, R, a0, a1, a2, a3, a4, a5> p_method, ptr<MethodInfo> p_mi) {
		name = p_name;
		class_name = p_class_name;
		argc = p_argc;
		method = p_method;
		mi = p_mi;
		mi->_set_bind((void*)this);
	}
	virtual var call(Object* self, stdvec<var*>& args) const override {

		int default_arg_count = mi->get_default_arg_count();
		int args_given = (int)args.size();
		if (args_given + default_arg_count < 6) { /* Args not enough. */
			if (default_arg_count == 0) THROW_ERROR(Error::INVALID_ARG_COUNT, "expected at exactly 6 argument(s).");
			else THROW_ERROR(Error::INVALID_ARG_COUNT, String::format("expected at least %i argument(s).", 6 - default_arg_count));
		} else if (args_given > 6) { /* More args proveded.	*/
			if (default_arg_count == 0) THROW_ERROR(Error::INVALID_ARG_COUNT, "expected at exactly 6 argument(s).");
			else THROW_ERROR(Error::INVALID_ARG_COUNT, String::format( "expected minimum of %i argument(s) and maximum of 6 argument(s).", 6 - default_arg_count));
		}

		stdvec<var> default_args_copy;
		for (int i = 6 - args_given; i > 0 ; i--) {
			default_args_copy.push_back(mi->get_default_args()[default_arg_count - i]);
		}
		for (var& v : default_args_copy) args.push_back(&v);

		return _internal_call_method_6<T, _Tmethod, R>(self, method, args);
	}
};

template <typename T, class M, typename _TRet, typename std::enable_if<!std::is_same<_TRet, void>::value, bool>::type = true>
inline var _internal_call_method_7(Object* self, const M& method, stdvec<var*>& args) {
	return (((T*)self)->*method)(*args[0], *args[1], *args[2], *args[3], *args[4], *args[5], *args[6]);
}
template <typename T, class M, typename _TRet, typename std::enable_if<std::is_same<_TRet, void>::value, bool>::type = true>
inline var _internal_call_method_7(Object* self, const M& method, stdvec<var*>& args) {
	(((T*)self)->*method)(*args[0], *args[1], *args[2], *args[3], *args[4], *args[5], *args[6]); return var();
}

template<typename T, typename R, typename a0, typename a1, typename a2, typename a3, typename a4, typename a5, typename a6>
class _MethodBind_M7 : public MethodBind {
	typedef M7<T, R, a0, a1, a2, a3, a4, a5, a6> _Tmethod;
	_Tmethod method;
public:
	_MethodBind_M7(const char* p_name, const char* p_class_name, int p_argc, M7<T, R, a0, a1, a2, a3, a4, a5, a6> p_method, ptr<MethodInfo> p_mi) {
		name = p_name;
		class_name = p_class_name;
		argc = p_argc;
		method = p_method;
		mi = p_mi;
		mi->_set_bind((void*)this);
	}
	virtual var call(Object* self, stdvec<var*>& args) const override {

		int default_arg_count = mi->get_default_arg_count();
		int args_given = (int)args.size();
		if (args_given + default_arg_count < 7) { /* Args not enough. */
			if (default_arg_count == 0) THROW_ERROR(Error::INVALID_ARG_COUNT, "expected at exactly 7 argument(s).");
			else THROW_ERROR(Error::INVALID_ARG_COUNT, String::format("expected at least %i argument(s).", 7 - default_arg_count));
		} else if (args_given > 7) { /* More args proveded.	*/
			if (default_arg_count == 0) THROW_ERROR(Error::INVALID_ARG_COUNT, "expected at exactly 7 argument(s).");
			else THROW_ERROR(Error::INVALID_ARG_COUNT, String::format( "expected minimum of %i argument(s) and maximum of 7 argument(s).", 7 - default_arg_count));
		}

		stdvec<var> default_args_copy;
		for (int i = 7 - args_given; i > 0 ; i--) {
			default_args_copy.push_back(mi->get_default_args()[default_arg_count - i]);
		}
		for (var& v : default_args_copy) args.push_back(&v);

		return _internal_call_method_7<T, _Tmethod, R>(self, method, args);
	}
};
template<typename T, typename R, typename a0, typename a1, typename a2, typename a3, typename a4, typename a5, typename a6>
class _MethodBind_M7_c : public MethodBind {
	typedef M7_c<T, R, a0, a1, a2, a3, a4, a5, a6> _Tmethod;
	_Tmethod method;
public:
	_MethodBind_M7_c(const char* p_name, const char* p_class_name, int p_argc, M7_c<T, R, a0, a1, a2, a3, a4, a5, a6> p_method, ptr<MethodInfo> p_mi) {
		name = p_name;
		class_name = p_class_name;
		argc = p_argc;
		method = p_method;
		mi = p_mi;
		mi->_set_bind((void*)this);
	}
	virtual var call(Object* self, stdvec<var*>& args) const override {

		int default_arg_count = mi->get_default_arg_count();
		int args_given = (int)args.size();
		if (args_given + default_arg_count < 7) { /* Args not enough. */
			if (default_arg_count == 0) THROW_ERROR(Error::INVALID_ARG_COUNT, "expected at exactly 7 argument(s).");
			else THROW_ERROR(Error::INVALID_ARG_COUNT, String::format("expected at least %i argument(s).", 7 - default_arg_count));
		} else if (args_given > 7) { /* More args proveded.	*/
			if (default_arg_count == 0) THROW_ERROR(Error::INVALID_ARG_COUNT, "expected at exactly 7 argument(s).");
			else THROW_ERROR(Error::INVALID_ARG_COUNT, String::format( "expected minimum of %i argument(s) and maximum of 7 argument(s).", 7 - default_arg_count));
		}

		stdvec<var> default_args_copy;
		for (int i = 7 - args_given; i > 0 ; i--) {
			default_args_copy.push_back(mi->get_default_args()[default_arg_count - i]);
		}
		for (var& v : default_args_copy) args.push_back(&v);

		return _internal_call_method_7<T, _Tmethod, R>(self, method, args);
	}
};
template <class F, typename _TRet, typename std::enable_if<!std::is_same<_TRet, void>::value, bool>::type = true>
inline var _internal_call_static_func_0(const F& static_func, stdvec<var*>& args) {
	return static_func();
}
template <class F, typename _TRet, typename std::enable_if<std::is_same<_TRet, void>::value, bool>::type = true>
inline var _internal_call_static_func_0(const F& static_func, stdvec<var*>& args) {
	static_func(); return var();
}

template<typename R>
class _StaticFuncBind_F0 : public StaticFuncBind {
	typedef F0<R> _Tfunc;
	_Tfunc static_func;
public:
	_StaticFuncBind_F0(const char* p_name, const char* p_class_name, int p_argc, F0<R> p_func, ptr<MethodInfo> p_mi) {
		name = p_name;
		class_name = p_class_name;
		argc = p_argc;
		static_func = p_func;
		mi = p_mi;
		mi->_set_bind((void*)this);
	}
	virtual var call(stdvec<var*>& args) const override {

		int default_arg_count = mi->get_default_arg_count();
		int args_given = (int)args.size();
		if (args_given + default_arg_count < 0) { /* Args not enough. */
			if (default_arg_count == 0) THROW_ERROR(Error::INVALID_ARG_COUNT, "expected at exactly 0 argument(s).");
			else THROW_ERROR(Error::INVALID_ARG_COUNT, String::format("expected at least %i argument(s).", 0 - default_arg_count));
		} else if (args_given > 0) { /* More args proveded.	*/
			if (default_arg_count == 0) THROW_ERROR(Error::INVALID_ARG_COUNT, "expected at exactly 0 argument(s).");
			else THROW_ERROR(Error::INVALID_ARG_COUNT, String::format( "expected minimum of %i argument(s) and maximum of 0 argument(s).", 0 - default_arg_count));
		}

		stdvec<var> default_args_copy;
		for (int i = 0 - args_given; i > 0 ; i--) {
			default_args_copy.push_back(mi->get_default_args()[default_arg_count - i]);
		}
		for (var& v : default_args_copy) args.push_back(&v);

		return _internal_call_static_func_0<_Tfunc, R>(static_func, args);
	}

};

template <class F, typename _TRet, typename std::enable_if<!std::is_same<_TRet, void>::value, bool>::type = true>
inline var _internal_call_static_func_1(const F& static_func, stdvec<var*>& args) {
	return static_func(*args[0]);
}
template <class F, typename _TRet, typename std::enable_if<std::is_same<_TRet, void>::value, bool>::type = true>
inline var _internal_call_static_func_1(const F& static_func, stdvec<var*>& args) {
	static_func(*args[0]); return var();
}

template<typename R, typename a0>
class _StaticFuncBind_F1 : public StaticFuncBind {
	typedef F1<R, a0> _Tfunc;
	_Tfunc static_func;
public:
	_StaticFuncBind_F1(const char* p_name, const char* p_class_name, int p_argc, F1<R, a0> p_func, ptr<MethodInfo> p_mi) {
		name = p_name;
		class_name = p_class_name;
		argc = p_argc;
		static_func = p_func;
		mi = p_mi;
		mi->_set_bind((void*)this);
	}
	virtual var call(stdvec<var*>& args) const override {

		int default_arg_count = mi->get_default_arg_count();
		int args_given = (int)args.size();
		if (args_given + default_arg_count < 1) { /* Args not enough. */
			if (default_arg_count == 0) THROW_ERROR(Error::INVALID_ARG_COUNT, "expected at exactly 1 argument(s).");
			else THROW_ERROR(Error::INVALID_ARG_COUNT, String::format("expected at least %i argument(s).", 1 - default_arg_count));
		} else if (args_given > 1) { /* More args proveded.	*/
			if (default_arg_count == 0) THROW_ERROR(Error::INVALID_ARG_COUNT, "expected at exactly 1 argument(s).");
			else THROW_ERROR(Error::INVALID_ARG_COUNT, String::format( "expected minimum of %i argument(s) and maximum of 1 argument(s).", 1 - default_arg_count));
		}

		stdvec<var> default_args_copy;
		for (int i = 1 - args_given; i > 0 ; i--) {
			default_args_copy.push_back(mi->get_default_args()[default_arg_count - i]);
		}
		for (var& v : default_args_copy) args.push_back(&v);

		return _internal_call_static_func_1<_Tfunc, R>(static_func, args);
	}

};

template <class F, typename _TRet, typename std::enable_if<!std::is_same<_TRet, void>::value, bool>::type = true>
inline var _internal_call_static_func_2(const F& static_func, stdvec<var*>& args) {
	return static_func(*args[0], *args[1]);
}
template <class F, typename _TRet, typename std::enable_if<std::is_same<_TRet, void>::value, bool>::type = true>
inline var _internal_call_static_func_2(const F& static_func, stdvec<var*>& args) {
	static_func(*args[0], *args[1]); return var();
}

template<typename R, typename a0, typename a1>
class _StaticFuncBind_F2 : public StaticFuncBind {
	typedef F2<R, a0, a1> _Tfunc;
	_Tfunc static_func;
public:
	_StaticFuncBind_F2(const char* p_name, const char* p_class_name, int p_argc, F2<R, a0, a1> p_func, ptr<MethodInfo> p_mi) {
		name = p_name;
		class_name = p_class_name;
		argc = p_argc;
		static_func = p_func;
		mi = p_mi;
		mi->_set_bind((void*)this);
	}
	virtual var call(stdvec<var*>& args) const override {

		int default_arg_count = mi->get_default_arg_count();
		int args_given = (int)args.size();
		if (args_given + default_arg_count < 2) { /* Args not enough. */
			if (default_arg_count == 0) THROW_ERROR(Error::INVALID_ARG_COUNT, "expected at exactly 2 argument(s).");
			else THROW_ERROR(Error::INVALID_ARG_COUNT, String::format("expected at least %i argument(s).", 2 - default_arg_count));
		} else if (args_given > 2) { /* More args proveded.	*/
			if (default_arg_count == 0) THROW_ERROR(Error::INVALID_ARG_COUNT, "expected at exactly 2 argument(s).");
			else THROW_ERROR(Error::INVALID_ARG_COUNT, String::format( "expected minimum of %i argument(s) and maximum of 2 argument(s).", 2 - default_arg_count));
		}

		stdvec<var> default_args_copy;
		for (int i = 2 - args_given; i > 0 ; i--) {
			default_args_copy.push_back(mi->get_default_args()[default_arg_count - i]);
		}
		for (var& v : default_args_copy) args.push_back(&v);

		return _internal_call_static_func_2<_Tfunc, R>(static_func, args);
	}

};

template <class F, typename _TRet, typename std::enable_if<!std::is_same<_TRet, void>::value, bool>::type = true>
inline var _internal_call_static_func_3(const F& static_func, stdvec<var*>& args) {
	return static_func(*args[0], *args[1], *args[2]);
}
template <class F, typename _TRet, typename std::enable_if<std::is_same<_TRet, void>::value, bool>::type = true>
inline var _internal_call_static_func_3(const F& static_func, stdvec<var*>& args) {
	static_func(*args[0], *args[1], *args[2]); return var();
}

template<typename R, typename a0, typename a1, typename a2>
class _StaticFuncBind_F3 : public StaticFuncBind {
	typedef F3<R, a0, a1, a2> _Tfunc;
	_Tfunc static_func;
public:
	_StaticFuncBind_F3(const char* p_name, const char* p_class_name, int p_argc, F3<R, a0, a1, a2> p_func, ptr<MethodInfo> p_mi) {
		name = p_name;
		class_name = p_class_name;
		argc = p_argc;
		static_func = p_func;
		mi = p_mi;
		mi->_set_bind((void*)this);
	}
	virtual var call(stdvec<var*>& args) const override {

		int default_arg_count = mi->get_default_arg_count();
		int args_given = (int)args.size();
		if (args_given + default_arg_count < 3) { /* Args not enough. */
			if (default_arg_count == 0) THROW_ERROR(Error::INVALID_ARG_COUNT, "expected at exactly 3 argument(s).");
			else THROW_ERROR(Error::INVALID_ARG_COUNT, String::format("expected at least %i argument(s).", 3 - default_arg_count));
		} else if (args_given > 3) { /* More args proveded.	*/
			if (default_arg_count == 0) THROW_ERROR(Error::INVALID_ARG_COUNT, "expected at exactly 3 argument(s).");
			else THROW_ERROR(Error::INVALID_ARG_COUNT, String::format( "expected minimum of %i argument(s) and maximum of 3 argument(s).", 3 - default_arg_count));
		}

		stdvec<var> default_args_copy;
		for (int i = 3 - args_given; i > 0 ; i--) {
			default_args_copy.push_back(mi->get_default_args()[default_arg_count - i]);
		}
		for (var& v : default_args_copy) args.push_back(&v);

		return _internal_call_static_func_3<_Tfunc, R>(static_func, args);
	}

};

template <class F, typename _TRet, typename std::enable_if<!std::is_same<_TRet, void>::value, bool>::type = true>
inline var _internal_call_static_func_4(const F& static_func, stdvec<var*>& args) {
	return static_func(*args[0], *args[1], *args[2], *args[3]);
}
template <class F, typename _TRet, typename std::enable_if<std::is_same<_TRet, void>::value, bool>::type = true>
inline var _internal_call_static_func_4(const F& static_func, stdvec<var*>& args) {
	static_func(*args[0], *args[1], *args[2], *args[3]); return var();
}

template<typename R, typename a0, typename a1, typename a2, typename a3>
class _StaticFuncBind_F4 : public StaticFuncBind {
	typedef F4<R, a0, a1, a2, a3> _Tfunc;
	_Tfunc static_func;
public:
	_StaticFuncBind_F4(const char* p_name, const char* p_class_name, int p_argc, F4<R, a0, a1, a2, a3> p_func, ptr<MethodInfo> p_mi) {
		name = p_name;
		class_name = p_class_name;
		argc = p_argc;
		static_func = p_func;
		mi = p_mi;
		mi->_set_bind((void*)this);
	}
	virtual var call(stdvec<var*>& args) const override {

		int default_arg_count = mi->get_default_arg_count();
		int args_given = (int)args.size();
		if (args_given + default_arg_count < 4) { /* Args not enough. */
			if (default_arg_count == 0) THROW_ERROR(Error::INVALID_ARG_COUNT, "expected at exactly 4 argument(s).");
			else THROW_ERROR(Error::INVALID_ARG_COUNT, String::format("expected at least %i argument(s).", 4 - default_arg_count));
		} else if (args_given > 4) { /* More args proveded.	*/
			if (default_arg_count == 0) THROW_ERROR(Error::INVALID_ARG_COUNT, "expected at exactly 4 argument(s).");
			else THROW_ERROR(Error::INVALID_ARG_COUNT, String::format( "expected minimum of %i argument(s) and maximum of 4 argument(s).", 4 - default_arg_count));
		}

		stdvec<var> default_args_copy;
		for (int i = 4 - args_given; i > 0 ; i--) {
			default_args_copy.push_back(mi->get_default_args()[default_arg_count - i]);
		}
		for (var& v : default_args_copy) args.push_back(&v);

		return _internal_call_static_func_4<_Tfunc, R>(static_func, args);
	}

};

template <class F, typename _TRet, typename std::enable_if<!std::is_same<_TRet, void>::value, bool>::type = true>
inline var _internal_call_static_func_5(const F& static_func, stdvec<var*>& args) {
	return static_func(*args[0], *args[1], *args[2], *args[3], *args[4]);
}
template <class F, typename _TRet, typename std::enable_if<std::is_same<_TRet, void>::value, bool>::type = true>
inline var _internal_call_static_func_5(const F& static_func, stdvec<var*>& args) {
	static_func(*args[0], *args[1], *args[2], *args[3], *args[4]); return var();
}

template<typename R, typename a0, typename a1, typename a2, typename a3, typename a4>
class _StaticFuncBind_F5 : public StaticFuncBind {
	typedef F5<R, a0, a1, a2, a3, a4> _Tfunc;
	_Tfunc static_func;
public:
	_StaticFuncBind_F5(const char* p_name, const char* p_class_name, int p_argc, F5<R, a0, a1, a2, a3, a4> p_func, ptr<MethodInfo> p_mi) {
		name = p_name;
		class_name = p_class_name;
		argc = p_argc;
		static_func = p_func;
		mi = p_mi;
		mi->_set_bind((void*)this);
	}
	virtual var call(stdvec<var*>& args) const override {

		int default_arg_count = mi->get_default_arg_count();
		int args_given = (int)args.size();
		if (args_given + default_arg_count < 5) { /* Args not enough. */
			if (default_arg_count == 0) THROW_ERROR(Error::INVALID_ARG_COUNT, "expected at exactly 5 argument(s).");
			else THROW_ERROR(Error::INVALID_ARG_COUNT, String::format("expected at least %i argument(s).", 5 - default_arg_count));
		} else if (args_given > 5) { /* More args proveded.	*/
			if (default_arg_count == 0) THROW_ERROR(Error::INVALID_ARG_COUNT, "expected at exactly 5 argument(s).");
			else THROW_ERROR(Error::INVALID_ARG_COUNT, String::format( "expected minimum of %i argument(s) and maximum of 5 argument(s).", 5 - default_arg_count));
		}

		stdvec<var> default_args_copy;
		for (int i = 5 - args_given; i > 0 ; i--) {
			default_args_copy.push_back(mi->get_default_args()[default_arg_count - i]);
		}
		for (var& v : default_args_copy) args.push_back(&v);

		return _internal_call_static_func_5<_Tfunc, R>(static_func, args);
	}

};

template <class F, typename _TRet, typename std::enable_if<!std::is_same<_TRet, void>::value, bool>::type = true>
inline var _internal_call_static_func_6(const F& static_func, stdvec<var*>& args) {
	return static_func(*args[0], *args[1], *args[2], *args[3], *args[4], *args[5]);
}
template <class F, typename _TRet, typename std::enable_if<std::is_same<_TRet, void>::value, bool>::type = true>
inline var _internal_call_static_func_6(const F& static_func, stdvec<var*>& args) {
	static_func(*args[0], *args[1], *args[2], *args[3], *args[4], *args[5]); return var();
}

template<typename R, typename a0, typename a1, typename a2, typename a3, typename a4, typename a5>
class _StaticFuncBind_F6 : public StaticFuncBind {
	typedef F6<R, a0, a1, a2, a3, a4, a5> _Tfunc;
	_Tfunc static_func;
public:
	_StaticFuncBind_F6(const char* p_name, const char* p_class_name, int p_argc, F6<R, a0, a1, a2, a3, a4, a5> p_func, ptr<MethodInfo> p_mi) {
		name = p_name;
		class_name = p_class_name;
		argc = p_argc;
		static_func = p_func;
		mi = p_mi;
		mi->_set_bind((void*)this);
	}
	virtual var call(stdvec<var*>& args) const override {

		int default_arg_count = mi->get_default_arg_count();
		int args_given = (int)args.size();
		if (args_given + default_arg_count < 6) { /* Args not enough. */
			if (default_arg_count == 0) THROW_ERROR(Error::INVALID_ARG_COUNT, "expected at exactly 6 argument(s).");
			else THROW_ERROR(Error::INVALID_ARG_COUNT, String::format("expected at least %i argument(s).", 6 - default_arg_count));
		} else if (args_given > 6) { /* More args proveded.	*/
			if (default_arg_count == 0) THROW_ERROR(Error::INVALID_ARG_COUNT, "expected at exactly 6 argument(s).");
			else THROW_ERROR(Error::INVALID_ARG_COUNT, String::format( "expected minimum of %i argument(s) and maximum of 6 argument(s).", 6 - default_arg_count));
		}

		stdvec<var> default_args_copy;
		for (int i = 6 - args_given; i > 0 ; i--) {
			default_args_copy.push_back(mi->get_default_args()[default_arg_count - i]);
		}
		for (var& v : default_args_copy) args.push_back(&v);

		return _internal_call_static_func_6<_Tfunc, R>(static_func, args);
	}

};

template <class F, typename _TRet, typename std::enable_if<!std::is_same<_TRet, void>::value, bool>::type = true>
inline var _internal_call_static_func_7(const F& static_func, stdvec<var*>& args) {
	return static_func(*args[0], *args[1], *args[2], *args[3], *args[4], *args[5], *args[6]);
}
template <class F, typename _TRet, typename std::enable_if<std::is_same<_TRet, void>::value, bool>::type = true>
inline var _internal_call_static_func_7(const F& static_func, stdvec<var*>& args) {
	static_func(*args[0], *args[1], *args[2], *args[3], *args[4], *args[5], *args[6]); return var();
}

template<typename R, typename a0, typename a1, typename a2, typename a3, typename a4, typename a5, typename a6>
class _StaticFuncBind_F7 : public StaticFuncBind {
	typedef F7<R, a0, a1, a2, a3, a4, a5, a6> _Tfunc;
	_Tfunc static_func;
public:
	_StaticFuncBind_F7(const char* p_name, const char* p_class_name, int p_argc, F7<R, a0, a1, a2, a3, a4, a5, a6> p_func, ptr<MethodInfo> p_mi) {
		name = p_name;
		class_name = p_class_name;
		argc = p_argc;
		static_func = p_func;
		mi = p_mi;
		mi->_set_bind((void*)this);
	}
	virtual var call(stdvec<var*>& args) const override {

		int default_arg_count = mi->get_default_arg_count();
		int args_given = (int)args.size();
		if (args_given + default_arg_count < 7) { /* Args not enough. */
			if (default_arg_count == 0) THROW_ERROR(Error::INVALID_ARG_COUNT, "expected at exactly 7 argument(s).");
			else THROW_ERROR(Error::INVALID_ARG_COUNT, String::format("expected at least %i argument(s).", 7 - default_arg_count));
		} else if (args_given > 7) { /* More args proveded.	*/
			if (default_arg_count == 0) THROW_ERROR(Error::INVALID_ARG_COUNT, "expected at exactly 7 argument(s).");
			else THROW_ERROR(Error::INVALID_ARG_COUNT, String::format( "expected minimum of %i argument(s) and maximum of 7 argument(s).", 7 - default_arg_count));
		}

		stdvec<var> default_args_copy;
		for (int i = 7 - args_given; i > 0 ; i--) {
			default_args_copy.push_back(mi->get_default_args()[default_arg_count - i]);
		}
		for (var& v : default_args_copy) args.push_back(&v);

		return _internal_call_static_func_7<_Tfunc, R>(static_func, args);
	}

};

template<typename T, typename R>
ptr<MethodBind> _bind_method(const char* method_name, const char* p_class_name, M0<T, R> m,
		 stdvec<var> default_args = {}) {
		
		DECLARE_VAR_TYPE(ret, R);
		ptr<MethodInfo> mi = newptr<MethodInfo>(method_name, make_stdvec<String>( ), make_stdvec<VarTypeInfo>(), ret, false, default_args, 0 );
	return newptr<_MethodBind_M0<T, R>>(method_name, p_class_name, 0, m, mi);
}

template<typename T, typename R>
ptr<MethodBind> _bind_method(const char* method_name, const char* p_class_name, M0_c<T, R> m,
		 stdvec<var> default_args = {}) {
		
		DECLARE_VAR_TYPE(ret, R);
		ptr<MethodInfo> mi = newptr<MethodInfo>(method_name, make_stdvec<String>( ), make_stdvec<VarTypeInfo>(), ret, false, default_args, 0 );
	return newptr<_MethodBind_M0_c<T, R>>(method_name, p_class_name, 0, m, mi);
}

template<typename T, typename R, typename a0>
ptr<MethodBind> _bind_method(const char* method_name, const char* p_class_name, M1<T, R, a0> m,
		const String& name0, stdvec<var> default_args = {}) {
		DECLARE_VAR_TYPE(vt0, a0);
		DECLARE_VAR_TYPE(ret, R);
		ptr<MethodInfo> mi = newptr<MethodInfo>(method_name, make_stdvec<String>(name0 ), make_stdvec<VarTypeInfo>(vt0), ret, false, default_args, 1 );
	return newptr<_MethodBind_M1<T, R, a0>>(method_name, p_class_name, 1, m, mi);
}

template<typename T, typename R, typename a0>
ptr<MethodBind> _bind_method(const char* method_name, const char* p_class_name, M1_c<T, R, a0> m,
		const String& name0, stdvec<var> default_args = {}) {
		DECLARE_VAR_TYPE(vt0, a0);
		DECLARE_VAR_TYPE(ret, R);
		ptr<MethodInfo> mi = newptr<MethodInfo>(method_name, make_stdvec<String>(name0 ), make_stdvec<VarTypeInfo>(vt0), ret, false, default_args, 1 );
	return newptr<_MethodBind_M1_c<T, R, a0>>(method_name, p_class_name, 1, m, mi);
}

template<typename T, typename R, typename a0, typename a1>
ptr<MethodBind> _bind_method(const char* method_name, const char* p_class_name, M2<T, R, a0, a1> m,
		const String& name0, const String& name1, stdvec<var> default_args = {}) {
		DECLARE_VAR_TYPE(vt0, a0); DECLARE_VAR_TYPE(vt1, a1);
		DECLARE_VAR_TYPE(ret, R);
		ptr<MethodInfo> mi = newptr<MethodInfo>(method_name, make_stdvec<String>(name0, name1 ), make_stdvec<VarTypeInfo>(vt0, vt1), ret, false, default_args, 2 );
	return newptr<_MethodBind_M2<T, R, a0, a1>>(method_name, p_class_name, 2, m, mi);
}

template<typename T, typename R, typename a0, typename a1>
ptr<MethodBind> _bind_method(const char* method_name, const char* p_class_name, M2_c<T, R, a0, a1> m,
		const String& name0, const String& name1, stdvec<var> default_args = {}) {
		DECLARE_VAR_TYPE(vt0, a0); DECLARE_VAR_TYPE(vt1, a1);
		DECLARE_VAR_TYPE(ret, R);
		ptr<MethodInfo> mi = newptr<MethodInfo>(method_name, make_stdvec<String>(name0, name1 ), make_stdvec<VarTypeInfo>(vt0, vt1), ret, false, default_args, 2 );
	return newptr<_MethodBind_M2_c<T, R, a0, a1>>(method_name, p_class_name, 2, m, mi);
}

template<typename T, typename R, typename a0, typename a1, typename a2>
ptr<MethodBind> _bind_method(const char* method_name, const char* p_class_name, M3<T, R, a0, a1, a2> m,
		const String& name0, const String& name1, const String& name2, stdvec<var> default_args = {}) {
		DECLARE_VAR_TYPE(vt0, a0); DECLARE_VAR_TYPE(vt1, a1); DECLARE_VAR_TYPE(vt2, a2);
		DECLARE_VAR_TYPE(ret, R);
		ptr<MethodInfo> mi = newptr<MethodInfo>(method_name, make_stdvec<String>(name0, name1, name2 ), make_stdvec<VarTypeInfo>(vt0, vt1, vt2), ret, false, default_args, 3 );
	return newptr<_MethodBind_M3<T, R, a0, a1, a2>>(method_name, p_class_name, 3, m, mi);
}

template<typename T, typename R, typename a0, typename a1, typename a2>
ptr<MethodBind> _bind_method(const char* method_name, const char* p_class_name, M3_c<T, R, a0, a1, a2> m,
		const String& name0, const String& name1, const String& name2, stdvec<var> default_args = {}) {
		DECLARE_VAR_TYPE(vt0, a0); DECLARE_VAR_TYPE(vt1, a1); DECLARE_VAR_TYPE(vt2, a2);
		DECLARE_VAR_TYPE(ret, R);
		ptr<MethodInfo> mi = newptr<MethodInfo>(method_name, make_stdvec<String>(name0, name1, name2 ), make_stdvec<VarTypeInfo>(vt0, vt1, vt2), ret, false, default_args, 3 );
	return newptr<_MethodBind_M3_c<T, R, a0, a1, a2>>(method_name, p_class_name, 3, m, mi);
}

template<typename T, typename R, typename a0, typename a1, typename a2, typename a3>
ptr<MethodBind> _bind_method(const char* method_name, const char* p_class_name, M4<T, R, a0, a1, a2, a3> m,
		const String& name0, const String& name1, const String& name2, const String& name3, stdvec<var> default_args = {}) {
		DECLARE_VAR_TYPE(vt0, a0); DECLARE_VAR_TYPE(vt1, a1); DECLARE_VAR_TYPE(vt2, a2); DECLARE_VAR_TYPE(vt3, a3);
		DECLARE_VAR_TYPE(ret, R);
		ptr<MethodInfo> mi = newptr<MethodInfo>(method_name, make_stdvec<String>(name0, name1, name2, name3 ), make_stdvec<VarTypeInfo>(vt0, vt1, vt2, vt3), ret, false, default_args, 4 );
	return newptr<_MethodBind_M4<T, R, a0, a1, a2, a3>>(method_name, p_class_name, 4, m, mi);
}

template<typename T, typename R, typename a0, typename a1, typename a2, typename a3>
ptr<MethodBind> _bind_method(const char* method_name, const char* p_class_name, M4_c<T, R, a0, a1, a2, a3> m,
		const String& name0, const String& name1, const String& name2, const String& name3, stdvec<var> default_args = {}) {
		DECLARE_VAR_TYPE(vt0, a0); DECLARE_VAR_TYPE(vt1, a1); DECLARE_VAR_TYPE(vt2, a2); DECLARE_VAR_TYPE(vt3, a3);
		DECLARE_VAR_TYPE(ret, R);
		ptr<MethodInfo> mi = newptr<MethodInfo>(method_name, make_stdvec<String>(name0, name1, name2, name3 ), make_stdvec<VarTypeInfo>(vt0, vt1, vt2, vt3), ret, false, default_args, 4 );
	return newptr<_MethodBind_M4_c<T, R, a0, a1, a2, a3>>(method_name, p_class_name, 4, m, mi);
}

template<typename T, typename R, typename a0, typename a1, typename a2, typename a3, typename a4>
ptr<MethodBind> _bind_method(const char* method_name, const char* p_class_name, M5<T, R, a0, a1, a2, a3, a4> m,
		const String& name0, const String& name1, const String& name2, const String& name3, const String& name4, stdvec<var> default_args = {}) {
		DECLARE_VAR_TYPE(vt0, a0); DECLARE_VAR_TYPE(vt1, a1); DECLARE_VAR_TYPE(vt2, a2); DECLARE_VAR_TYPE(vt3, a3); DECLARE_VAR_TYPE(vt4, a4);
		DECLARE_VAR_TYPE(ret, R);
		ptr<MethodInfo> mi = newptr<MethodInfo>(method_name, make_stdvec<String>(name0, name1, name2, name3, name4 ), make_stdvec<VarTypeInfo>(vt0, vt1, vt2, vt3, vt4), ret, false, default_args, 5 );
	return newptr<_MethodBind_M5<T, R, a0, a1, a2, a3, a4>>(method_name, p_class_name, 5, m, mi);
}

template<typename T, typename R, typename a0, typename a1, typename a2, typename a3, typename a4>
ptr<MethodBind> _bind_method(const char* method_name, const char* p_class_name, M5_c<T, R, a0, a1, a2, a3, a4> m,
		const String& name0, const String& name1, const String& name2, const String& name3, const String& name4, stdvec<var> default_args = {}) {
		DECLARE_VAR_TYPE(vt0, a0); DECLARE_VAR_TYPE(vt1, a1); DECLARE_VAR_TYPE(vt2, a2); DECLARE_VAR_TYPE(vt3, a3); DECLARE_VAR_TYPE(vt4, a4);
		DECLARE_VAR_TYPE(ret, R);
		ptr<MethodInfo> mi = newptr<MethodInfo>(method_name, make_stdvec<String>(name0, name1, name2, name3, name4 ), make_stdvec<VarTypeInfo>(vt0, vt1, vt2, vt3, vt4), ret, false, default_args, 5 );
	return newptr<_MethodBind_M5_c<T, R, a0, a1, a2, a3, a4>>(method_name, p_class_name, 5, m, mi);
}

template<typename T, typename R, typename a0, typename a1, typename a2, typename a3, typename a4, typename a5>
ptr<MethodBind> _bind_method(const char* method_name, const char* p_class_name, M6<T, R, a0, a1, a2, a3, a4, a5> m,
		const String& name0, const String& name1, const String& name2, const String& name3, const String& name4, const String& name5, stdvec<var> default_args = {}) {
		DECLARE_VAR_TYPE(vt0, a0); DECLARE_VAR_TYPE(vt1, a1); DECLARE_VAR_TYPE(vt2, a2); DECLARE_VAR_TYPE(vt3, a3); DECLARE_VAR_TYPE(vt4, a4); DECLARE_VAR_TYPE(vt5, a5);
		DECLARE_VAR_TYPE(ret, R);
		ptr<MethodInfo> mi = newptr<MethodInfo>(method_name, make_stdvec<String>(name0, name1, name2, name3, name4, name5 ), make_stdvec<VarTypeInfo>(vt0, vt1, vt2, vt3, vt4, vt5), ret, false, default_args, 6 );
	return newptr<_MethodBind_M6<T, R, a0, a1, a2, a3, a4, a5>>(method_name, p_class_name, 6, m, mi);
}

template<typename T, typename R, typename a0, typename a1, typename a2, typename a3, typename a4, typename a5>
ptr<MethodBind> _bind_method(const char* method_name, const char* p_class_name, M6_c<T, R, a0, a1, a2, a3, a4, a5> m,
		const String& name0, const String& name1, const String& name2, const String& name3, const String& name4, const String& name5, stdvec<var> default_args = {}) {
		DECLARE_VAR_TYPE(vt0, a0); DECLARE_VAR_TYPE(vt1, a1); DECLARE_VAR_TYPE(vt2, a2); DECLARE_VAR_TYPE(vt3, a3); DECLARE_VAR_TYPE(vt4, a4); DECLARE_VAR_TYPE(vt5, a5);
		DECLARE_VAR_TYPE(ret, R);
		ptr<MethodInfo> mi = newptr<MethodInfo>(method_name, make_stdvec<String>(name0, name1, name2, name3, name4, name5 ), make_stdvec<VarTypeInfo>(vt0, vt1, vt2, vt3, vt4, vt5), ret, false, default_args, 6 );
	return newptr<_MethodBind_M6_c<T, R, a0, a1, a2, a3, a4, a5>>(method_name, p_class_name, 6, m, mi);
}

template<typename T, typename R, typename a0, typename a1, typename a2, typename a3, typename a4, typename a5, typename a6>
ptr<MethodBind> _bind_method(const char* method_name, const char* p_class_name, M7<T, R, a0, a1, a2, a3, a4, a5, a6> m,
		const String& name0, const String& name1, const String& name2, const String& name3, const String& name4, const String& name5, const String& name6, stdvec<var> default_args = {}) {
		DECLARE_VAR_TYPE(vt0, a0); DECLARE_VAR_TYPE(vt1, a1); DECLARE_VAR_TYPE(vt2, a2); DECLARE_VAR_TYPE(vt3, a3); DECLARE_VAR_TYPE(vt4, a4); DECLARE_VAR_TYPE(vt5, a5); DECLARE_VAR_TYPE(vt6, a6);
		DECLARE_VAR_TYPE(ret, R);
		ptr<MethodInfo> mi = newptr<MethodInfo>(method_name, make_stdvec<String>(name0, name1, name2, name3, name4, name5, name6 ), make_stdvec<VarTypeInfo>(vt0, vt1, vt2, vt3, vt4, vt5, vt6), ret, false, default_args, 7 );
	return newptr<_MethodBind_M7<T, R, a0, a1, a2, a3, a4, a5, a6>>(method_name, p_class_name, 7, m, mi);
}

template<typename T, typename R, typename a0, typename a1, typename a2, typename a3, typename a4, typename a5, typename a6>
ptr<MethodBind> _bind_method(const char* method_name, const char* p_class_name, M7_c<T, R, a0, a1, a2, a3, a4, a5, a6> m,
		const String& name0, const String& name1, const String& name2, const String& name3, const String& name4, const String& name5, const String& name6, stdvec<var> default_args = {}) {
		DECLARE_VAR_TYPE(vt0, a0); DECLARE_VAR_TYPE(vt1, a1); DECLARE_VAR_TYPE(vt2, a2); DECLARE_VAR_TYPE(vt3, a3); DECLARE_VAR_TYPE(vt4, a4); DECLARE_VAR_TYPE(vt5, a5); DECLARE_VAR_TYPE(vt6, a6);
		DECLARE_VAR_TYPE(ret, R);
		ptr<MethodInfo> mi = newptr<MethodInfo>(method_name, make_stdvec<String>(name0, name1, name2, name3, name4, name5, name6 ), make_stdvec<VarTypeInfo>(vt0, vt1, vt2, vt3, vt4, vt5, vt6), ret, false, default_args, 7 );
	return newptr<_MethodBind_M7_c<T, R, a0, a1, a2, a3, a4, a5, a6>>(method_name, p_class_name, 7, m, mi);
}

template<typename R>
ptr<StaticFuncBind> _bind_static_func(const char* func_name, const char* p_class_name, F0<R> f,
		 stdvec<var> default_args = {}) {
		
		DECLARE_VAR_TYPE(ret, R);
		ptr<MethodInfo> mi = newptr<MethodInfo>(func_name, make_stdvec<String>( ), make_stdvec<VarTypeInfo>(), ret, true, default_args, 0 );
	return newptr<_StaticFuncBind_F0<R>>(func_name, p_class_name, 0, f, mi);
}

template<typename R, typename a0>
ptr<StaticFuncBind> _bind_static_func(const char* func_name, const char* p_class_name, F1<R, a0> f,
		const String& name0, stdvec<var> default_args = {}) {
		DECLARE_VAR_TYPE(vt0, a0);
		DECLARE_VAR_TYPE(ret, R);
		ptr<MethodInfo> mi = newptr<MethodInfo>(func_name, make_stdvec<String>(name0 ), make_stdvec<VarTypeInfo>(vt0), ret, true, default_args, 1 );
	return newptr<_StaticFuncBind_F1<R, a0>>(func_name, p_class_name, 1, f, mi);
}

template<typename R, typename a0, typename a1>
ptr<StaticFuncBind> _bind_static_func(const char* func_name, const char* p_class_name, F2<R, a0, a1> f,
		const String& name0, const String& name1, stdvec<var> default_args = {}) {
		DECLARE_VAR_TYPE(vt0, a0); DECLARE_VAR_TYPE(vt1, a1);
		DECLARE_VAR_TYPE(ret, R);
		ptr<MethodInfo> mi = newptr<MethodInfo>(func_name, make_stdvec<String>(name0, name1 ), make_stdvec<VarTypeInfo>(vt0, vt1), ret, true, default_args, 2 );
	return newptr<_StaticFuncBind_F2<R, a0, a1>>(func_name, p_class_name, 2, f, mi);
}

template<typename R, typename a0, typename a1, typename a2>
ptr<StaticFuncBind> _bind_static_func(const char* func_name, const char* p_class_name, F3<R, a0, a1, a2> f,
		const String& name0, const String& name1, const String& name2, stdvec<var> default_args = {}) {
		DECLARE_VAR_TYPE(vt0, a0); DECLARE_VAR_TYPE(vt1, a1); DECLARE_VAR_TYPE(vt2, a2);
		DECLARE_VAR_TYPE(ret, R);
		ptr<MethodInfo> mi = newptr<MethodInfo>(func_name, make_stdvec<String>(name0, name1, name2 ), make_stdvec<VarTypeInfo>(vt0, vt1, vt2), ret, true, default_args, 3 );
	return newptr<_StaticFuncBind_F3<R, a0, a1, a2>>(func_name, p_class_name, 3, f, mi);
}

template<typename R, typename a0, typename a1, typename a2, typename a3>
ptr<StaticFuncBind> _bind_static_func(const char* func_name, const char* p_class_name, F4<R, a0, a1, a2, a3> f,
		const String& name0, const String& name1, const String& name2, const String& name3, stdvec<var> default_args = {}) {
		DECLARE_VAR_TYPE(vt0, a0); DECLARE_VAR_TYPE(vt1, a1); DECLARE_VAR_TYPE(vt2, a2); DECLARE_VAR_TYPE(vt3, a3);
		DECLARE_VAR_TYPE(ret, R);
		ptr<MethodInfo> mi = newptr<MethodInfo>(func_name, make_stdvec<String>(name0, name1, name2, name3 ), make_stdvec<VarTypeInfo>(vt0, vt1, vt2, vt3), ret, true, default_args, 4 );
	return newptr<_StaticFuncBind_F4<R, a0, a1, a2, a3>>(func_name, p_class_name, 4, f, mi);
}

template<typename R, typename a0, typename a1, typename a2, typename a3, typename a4>
ptr<StaticFuncBind> _bind_static_func(const char* func_name, const char* p_class_name, F5<R, a0, a1, a2, a3, a4> f,
		const String& name0, const String& name1, const String& name2, const String& name3, const String& name4, stdvec<var> default_args = {}) {
		DECLARE_VAR_TYPE(vt0, a0); DECLARE_VAR_TYPE(vt1, a1); DECLARE_VAR_TYPE(vt2, a2); DECLARE_VAR_TYPE(vt3, a3); DECLARE_VAR_TYPE(vt4, a4);
		DECLARE_VAR_TYPE(ret, R);
		ptr<MethodInfo> mi = newptr<MethodInfo>(func_name, make_stdvec<String>(name0, name1, name2, name3, name4 ), make_stdvec<VarTypeInfo>(vt0, vt1, vt2, vt3, vt4), ret, true, default_args, 5 );
	return newptr<_StaticFuncBind_F5<R, a0, a1, a2, a3, a4>>(func_name, p_class_name, 5, f, mi);
}

template<typename R, typename a0, typename a1, typename a2, typename a3, typename a4, typename a5>
ptr<StaticFuncBind> _bind_static_func(const char* func_name, const char* p_class_name, F6<R, a0, a1, a2, a3, a4, a5> f,
		const String& name0, const String& name1, const String& name2, const String& name3, const String& name4, const String& name5, stdvec<var> default_args = {}) {
		DECLARE_VAR_TYPE(vt0, a0); DECLARE_VAR_TYPE(vt1, a1); DECLARE_VAR_TYPE(vt2, a2); DECLARE_VAR_TYPE(vt3, a3); DECLARE_VAR_TYPE(vt4, a4); DECLARE_VAR_TYPE(vt5, a5);
		DECLARE_VAR_TYPE(ret, R);
		ptr<MethodInfo> mi = newptr<MethodInfo>(func_name, make_stdvec<String>(name0, name1, name2, name3, name4, name5 ), make_stdvec<VarTypeInfo>(vt0, vt1, vt2, vt3, vt4, vt5), ret, true, default_args, 6 );
	return newptr<_StaticFuncBind_F6<R, a0, a1, a2, a3, a4, a5>>(func_name, p_class_name, 6, f, mi);
}

template<typename R, typename a0, typename a1, typename a2, typename a3, typename a4, typename a5, typename a6>
ptr<StaticFuncBind> _bind_static_func(const char* func_name, const char* p_class_name, F7<R, a0, a1, a2, a3, a4, a5, a6> f,
		const String& name0, const String& name1, const String& name2, const String& name3, const String& name4, const String& name5, const String& name6, stdvec<var> default_args = {}) {
		DECLARE_VAR_TYPE(vt0, a0); DECLARE_VAR_TYPE(vt1, a1); DECLARE_VAR_TYPE(vt2, a2); DECLARE_VAR_TYPE(vt3, a3); DECLARE_VAR_TYPE(vt4, a4); DECLARE_VAR_TYPE(vt5, a5); DECLARE_VAR_TYPE(vt6, a6);
		DECLARE_VAR_TYPE(ret, R);
		ptr<MethodInfo> mi = newptr<MethodInfo>(func_name, make_stdvec<String>(name0, name1, name2, name3, name4, name5, name6 ), make_stdvec<VarTypeInfo>(vt0, vt1, vt2, vt3, vt4, vt5, vt6), ret, true, default_args, 7 );
	return newptr<_StaticFuncBind_F7<R, a0, a1, a2, a3, a4, a5, a6>>(func_name, p_class_name, 7, f, mi);
}


template<typename T, typename R>
using MVA = R(T::*)(stdvec<var*>&);

template<typename R>
using FVA = R(*)(stdvec<var*>&);


template <typename T, class M, typename _TRet, typename std::enable_if<!std::is_same<_TRet, void>::value, bool>::type = true>
inline var _internal_call_static_func_va(Object* self, const M& method, stdvec<var*>& args) {
	return (((T*)self)->*method)(args);
}

template <typename T, class M, typename _TRet, typename std::enable_if<std::is_same<_TRet, void>::value, bool>::type = true>
inline var _internal_call_static_func_va(Object* self, const M& method, stdvec<var*>& args) {{
	(((T*)self)->*method)(args); return var();
}}

template<typename T, typename R>
class _MethodBind_MVA : public MethodBind {
	typedef MVA<T, R> _Tmethod_va;
	_Tmethod_va method;
public:
	_MethodBind_MVA(const char* p_name, const char* p_class_name, MVA<T, R> p_method, ptr<MethodInfo> p_mi) {
		name = p_name;
		class_name = p_class_name;
		argc = -1;
		method = p_method;
		mi = p_mi;
		mi->_set_bind((void*)this);
	}
	virtual var call(Object* self, stdvec<var*>& args) const override {
		return _internal_call_static_func_va<T, _Tmethod_va, R>(self, method, args);
	}

	const ptr<MethodInfo> get_method_info() const { return mi; }
};

// -----------------------------------------------------------

template <class F, typename _TRet, typename std::enable_if<!std::is_same<_TRet, void>::value, bool>::type = true>
inline var _internal_call_static_func_va(const F& static_func, stdvec<var*>& args) {
	return static_func(args);
}

template <class F, typename _TRet, typename std::enable_if<std::is_same<_TRet, void>::value, bool>::type = true>
inline var _internal_call_static_func_va(const F& static_func, stdvec<var*>& args) {{
	static_func(args); return var();
}}


template<typename R>
class _StaticFuncBind_FVA : public StaticFuncBind {
	typedef FVA<R> _Tfunc_va;
	_Tfunc_va static_func;
public:
	_StaticFuncBind_FVA(const char* p_name, const char* p_class_name, FVA<R> p_func, ptr<MethodInfo> p_mi) {
		name = p_name;
		class_name = p_class_name;
		argc = -1;
		static_func = p_func;
		mi = p_mi;
		mi->_set_bind((void*)this);
	}
	virtual var call(stdvec<var*>& args) const override {
		return _internal_call_static_func_va<_Tfunc_va, R>(static_func, args);
	}

	const ptr<MethodInfo> get_method_info() const { return mi; }
};

template<typename T, typename R>
ptr<MethodBind> _bind_va_method(const char* method_name, const char* p_class_name, MVA<T, R> m) {
	DECLARE_VAR_TYPE(ret, R);
	ptr<MethodInfo> mi = newptr<MethodInfo>( method_name, make_stdvec<String>(), make_stdvec<VarTypeInfo>(), ret, false, make_stdvec<var>(), -1);
	return newptr<_MethodBind_MVA<T, R>>(method_name, p_class_name, m, mi);
}

template<typename R>
ptr<StaticFuncBind> _bind_va_static_func(const char* func_name, const char* p_class_name, FVA<R> f) {
	DECLARE_VAR_TYPE(ret, R);
	ptr<MethodInfo> mi = newptr<MethodInfo>( func_name, make_stdvec<String>(), make_stdvec<VarTypeInfo>(), ret, true, make_stdvec<var>(), -1);
	return newptr<_StaticFuncBind_FVA<R>>(func_name, p_class_name, f, mi);
}
} // namespace

#endif // NATIVE_BIND_GEN_H


#define DEFVAL(m_val) m_val
#define DEFVALUES(...) make_stdvec<var>(__VA_ARGS__)
#define PARAMS(...) __VA_ARGS__

#define BIND_METHOD(m_name, m_method, ...)    p_native_classes->bind_data(_bind_method(m_name, get_type_name_s(), m_method, ##__VA_ARGS__))
#define BIND_METHOD_VA(m_name, m_method)      p_native_classes->bind_data(_bind_va_method(m_name, get_type_name_s(), m_method))
#define BIND_STATIC_FUNC(m_name, m_func, ...) p_native_classes->bind_data(_bind_static_func(m_name, get_type_name_s(), m_func, ##__VA_ARGS__))
#define BIND_STATIC_FUNC_VA(m_name, m_func)   p_native_classes->bind_data(_bind_va_static_func(m_name, get_type_name_s(), m_func))
#define BIND_MEMBER(m_name, m_member, ...)    p_native_classes->bind_data(_bind_member(m_name, get_type_name_s(), m_member, ##__VA_ARGS__))
#define BIND_STATIC_MEMBER(m_name, m_member)  p_native_classes->bind_data(_bind_static_member(m_name, get_type_name_s(), m_member))
#define BIND_CONST(m_name, m_const)           p_native_classes->bind_data(_bind_static_const(m_name, get_type_name_s(), m_const))
#define BIND_ENUM(m_name, ...)                p_native_classes->bind_data(_bind_enum(m_name, get_type_name_s(), ##__VA_ARGS__));
#define BIND_ENUM_VALUE(m_name, m_value)      p_native_classes->bind_data(newptr<EnumValueBind>(m_name, get_type_name_s(), m_value, newptr<EnumValueInfo>(m_name, m_value)));


namespace carbon {
typedef ptr<Object>(*__new_f)();

class NativeClasses {
	struct ClassEntries {
		String class_name;
		String parent_class_name;
		__new_f __new = nullptr;
		const StaticFuncBind* __constructor = nullptr;
		stdmap<size_t, ptr<BindData>> bind_data;
		stdmap<size_t, ptr<MemberInfo>> member_info;
	};

private:
	NativeClasses() {}
	static NativeClasses* _singleton;
	stdhashtable<size_t, ClassEntries> classes;

public:
	static NativeClasses* singleton();
	static void cleanup();

	void bind_data(ptr<BindData> p_bind_data);
	void set_parent(const String& p_class_name, const String& p_parent_class_name);
	void set_new_function(const String& p_class_name, __new_f p__new);

	ptr<BindData> get_bind_data(const String& p_class_name, const String& attrib);
	ptr<BindData> find_bind_data(const String& p_class_name, const String& attrib);
	const ptr<MemberInfo> get_member_info(const String& p_class_name, const String& attrib);
	String get_inheritance(const String& p_class_name);
	bool is_class_registered(const String& p_class_name);
	const stdmap<size_t, ptr<BindData>>& get_bind_data_list(const String& p_class_name);
	const stdmap<size_t, ptr<MemberInfo>>& get_member_info_list(const String& p_class_name);

	ptr<Object> _new(const String& p_class_name);
	const StaticFuncBind* get_constructor(const String& p_class_name);
	ptr<Object> construct(const String& p_class_name, stdvec<var*>& p_args);
	var call_static(const String& p_base_name, const String& p_attrib, stdvec<var*>& p_args);
	var call_method_on(ptr<Object>& p_on, const String& p_attrib, stdvec<var*>& p_args);

	template<typename T>
	void register_class() {
		set_parent(T::get_type_name_s(), T::get_base_type_name_s());
		set_new_function(T::get_type_name_s(), &T::__new);
		T::_bind_data(this);
	}

	template<typename T>
	void unregister_class() {
		throw "TODO:";
	}

};


}

#endif // NATIVE_CLASSES_H


// var imports

#ifndef VAR_PRIVATE_H
#define VAR_PRIVATE_H
namespace carbon {

#define _PARAMS(...) make_stdvec<String>(__VA_ARGS__)
#define _TYPES(...) make_stdvec<VarTypeInfo>(__VA_ARGS__)
#define _DEFVALS(...) make_stdvec<var>(__VA_ARGS__)
#define _NEW_METHOD_INFO(m_name, ...) { String(m_name).hash(), newptr<MethodInfo>(m_name, __VA_ARGS__) }

template <typename T>
static void _check_method_and_args(const String& p_method, const stdvec<var*>& p_args, ptr<MemberInfo> p_mi) {
	if (p_mi != nullptr) {
		if (p_mi->get_type() != MemberInfo::METHOD)
			THROW_ERROR(Error::TYPE_ERROR, String::format("member \"%s\" is not callable.", p_method.c_str()));
		const MethodInfo* mp = static_cast<const MethodInfo*>(p_mi.get());
		int arg_count = mp->get_arg_count();
		int default_arg_count = mp->get_default_arg_count();
		if (arg_count != -1) {
			if ((int)p_args.size() + default_arg_count < arg_count) { /* Args not enough. */
				if (default_arg_count == 0) THROW_ERROR(Error::INVALID_ARG_COUNT, String::format("expected at exactly %i argument(s).", arg_count));
				else THROW_ERROR(Error::INVALID_ARG_COUNT, String::format("expected at least %i argument(s).", arg_count - default_arg_count));
			} else if ((int)p_args.size() > arg_count) { /* More args proveded.    */
				if (default_arg_count == 0) THROW_ERROR(Error::INVALID_ARG_COUNT, String::format("expected at exactly %i argument(s).", arg_count));
				else THROW_ERROR(Error::INVALID_ARG_COUNT, String::format(
					"expected minimum of %i argument(s) and maximum of %i argument(s).", arg_count - default_arg_count, arg_count));
			}
		}
		for (int j = 0; j < (int)mp->get_arg_types().size(); j++) {
			if (mp->get_arg_types()[j] == VarTypeInfo(var::VAR)) continue; /* can't be _NULL. */
			if (p_args.size() == j) break; /* rest are default args. */
			if (mp->get_arg_types()[j] != VarTypeInfo(p_args[j]->get_type(), p_args[j]->get_type_name().c_str()))
				THROW_ERROR(Error::TYPE_ERROR, String::format(
					"expected type %s at argument %i.", var::get_type_name_s(mp->get_arg_types()[j].type), j));
		}
	} else {
		THROW_ERROR(Error::ATTRIBUTE_ERROR, String::format("attribute \"%s\" not exists on base %s.", p_method.c_str(), T::get_type_name_s()));
	}
}


}
#endif // VAR_PRIVATE_H

//#include "var/_string.h"
//#include "var/_array.h"
//#include "var/_map.h"
//#include "var/_object.h"
//#include "var/var.h"

// native imports

#ifndef _RUNTIME_TYPES_H
#define _RUNTIME_TYPES_H

//#include "core/native.h"

namespace carbon {

class NativeClasses;

// TODO: implement it += 1;

class _Iterator_int : public Object {
	REGISTER_CLASS(_Iterator_int, Object) {}

	int64_t _it = 0;
	int64_t _max = 0;
	_Iterator_int(int64_t p_max = 0) { _max = p_max; }

	bool _is_registered() const override { return false; }
	bool __iter_has_next() override { return _it != _max; }
	var __iter_next() override { return _it++; }
};

class _Iterator_String : public Object {
	REGISTER_CLASS(_Iterator_String, Object) {}

	size_t _it = 0;
	const String* _str_data = nullptr;

public:
	_Iterator_String() {}
	_Iterator_String(const String* p_str) : _str_data(p_str) {}

	bool _is_registered() const override { return false; }
	bool __iter_has_next() override { return _it != _str_data->size(); }
	var __iter_next() override { return String(_str_data->operator[](_it++)); }
};

class _Iterator_Array : public Object {
	REGISTER_CLASS(_Iterator_Array, Object) {}

	stdvec<var>::const_iterator _it;
	const stdvec<var>* _array_data = nullptr;
public:
	_Iterator_Array() {}
	_Iterator_Array(const Array* p_array) {
		_array_data = p_array->get_stdvec();
		_it = _array_data->begin();
	}

	bool _is_registered() const override { return false; }
	virtual bool __iter_has_next() override { return _it != _array_data->end(); }
	virtual var __iter_next() override { return *(_it++); }

};

class _Map_KeyValue_Pair : public Object {
	REGISTER_CLASS(_Map_KeyValue_Pair, Object) {}

public:
	_Map_KeyValue_Pair() {}
	_Map_KeyValue_Pair(const var* p_key, var* p_value) : key(p_key), value(p_value) {}

	bool _is_registered() const override { return false; }

	String to_string() override {
		// TODO: implement and use __repr__() here
		return String::format("MapKVPair(%s:%s)", key->to_string().c_str(), value->to_string().c_str());
	}

	var get_member(const String& p_name) override {
		switch (p_name.const_hash()) {
			case "key"_hash: return *key;
			case "value"_hash: return *value;
			default: return Super::get_member(p_name);
		}
	}

	void set_member(const String& p_name, var& p_value) override {
		switch (p_name.const_hash()) {
			case "key"_hash: {
				THROW_ERROR(Error::ATTRIBUTE_ERROR, "key of a map is immutable");
			} break;
			case "value"_hash: *value = p_value; return;
			default: Super::set_member(p_name, p_value); return;
		}
	}

	const var* key;
	var* value;
};

class _Iterator_Map : public Object {
	REGISTER_CLASS(_Iterator_Map, Object) {}

	Map::_map_internal_t::iterator _it;
	Map::_map_internal_t* _map_data = nullptr;

public:

	_Iterator_Map() {}
	_Iterator_Map(Map* p_map) {
		_map_data = (Map::_map_internal_t*)p_map->get_data();
		_it = _map_data->begin();
	}

	bool _is_registered() const override { return false; }
	virtual bool __iter_has_next() override { return _it != _map_data->end(); }

	virtual var __iter_next() override {
		ptr<_Map_KeyValue_Pair> ret = newptr<_Map_KeyValue_Pair>(&(_it->first), &(_it->second));
		_it++;
		return ret;
	}
};

}

#endif // _RUNTIME_TYPES_H


#ifndef FILE_H
#define FILE_H

//#include "core/native.h"

#ifndef BUFFER_H
#define BUFFER_H

//#include "core/native.h"

namespace carbon {

class Buffer : public Object {
	REGISTER_CLASS(Buffer, Object) {
		// TODO: the below default value mis match
		BIND_STATIC_FUNC("Buffer", &Buffer::_Buffer, PARAMS("self", "size"), DEFVALUES(0));
		BIND_METHOD("alloc", &Buffer::alloc, PARAMS("size"));
		BIND_METHOD("size", &Buffer::size);
	}

public:

	Buffer(size_t p_size = 0);
	static void _Buffer(ptr<Buffer> self, int64_t p_size = 0);

	// Methods.
	void alloc(size_t p_size);

	byte_t* front();
	void* get_data() override;
	size_t size() const;

	byte_t& operator[](size_t p_index);
	const byte_t& operator[](size_t p_index) const;

	// operators
	virtual var __get_mapped(const var& p_key) /*const*/ override;
	virtual void __set_mapped(const var& p_key, const var& p_value) override;

private:
	// Members.
	std::shared_ptr<byte_t> _buffer;
	size_t _size = 0;
};

}

#endif // BUFFER_H


namespace carbon {

class File : public Object {

	REGISTER_CLASS(File, Object) {
		BIND_STATIC_FUNC("File", &File::_File, PARAMS("self", "path", "mode"), DEFVALUES("", DEFAULT));

		BIND_ENUM_VALUE("READ",   READ);
		BIND_ENUM_VALUE("WRITE",  WRITE);
		BIND_ENUM_VALUE("APPEND", APPEND);
		BIND_ENUM_VALUE("BINARY", BINARY);
		BIND_ENUM_VALUE("EXTRA",  EXTRA);
		//BIND_ENUM_VALUE("DEFAULT", DEFAULT);

		BIND_METHOD("open",      &File::open,  PARAMS("path", "mode"), DEFVALUES(DEFAULT));
		BIND_METHOD("read",      &File::read);
		BIND_METHOD("read_line", &File::read_line);
		BIND_METHOD("write",     &File::write, PARAMS("what"));
		BIND_METHOD("close",     &File::close);
	}

public:
	enum {
		READ   = 1 << 0, // "r"
		WRITE  = 1 << 1, // "w"
		APPEND = 1 << 2, // "a"
		BINARY = 1 << 3, // "b"
		EXTRA  = 1 << 4, // "+"

		DEFAULT = READ,
	};

	File(const String& p_path = "", int p_mode = DEFAULT);
	~File();
	static void _File(ptr<File> p_self, const String& p_path = "", int p_mode = DEFAULT);

	// Methods.
	inline bool is_open() const { return _file != NULL; }
	void open(const String& p_path, int p_mode = DEFAULT);
	void close();
	long size();
	String get_path() const { return path; }
	int get_mode() const { return mode; }

	String read_text();
	String read_line();
	void write_text(const String& p_text);

	ptr<Buffer> read_bytes();
	void write_bytes(const ptr<Buffer>& p_bytes);

	var read();
	void write(const var& p_what);

	var __iter_begin() override;


protected:

private:
	//std::fstream file;
	FILE* _file = NULL;
	String path;
	int mode = DEFAULT;

};
}

#endif // FILE_H



#ifndef	PATH_H
#define	PATH_H

//#include "core/native.h"

// TODO: split path to path and dir

// TODO: add copy options for overwrite

namespace carbon {
//namespace fs = std::filesystem;
class Path : public Object {

	REGISTER_CLASS(Path, Object) {
		BIND_STATIC_FUNC("Path",   &Path::_Path,     PARAMS("self", "path"), DEFVALUES(""));

		BIND_METHOD("absolute",   &Path::absolute);
		BIND_METHOD("join",       &Path::join,      PARAMS("path"));
		BIND_METHOD("parent",     &Path::parent);
		BIND_METHOD("filename",   &Path::filename);
		BIND_METHOD("extension",  &Path::extension);
		BIND_METHOD("exists",     &Path::exists);
		BIND_METHOD("isdir",      &Path::isdir);
		BIND_METHOD("listdir",    &Path::listdir);

	}

public:
	Path(const String& p_path = "");
	static void _Path(ptr<Path> p_self, const String& p_path);

	String absolute(); // TODO: return Path instance (ptr<Path>)
	String parent();   // TODO: return Path instance
	String filename();
	String extension();
	bool exists();
	bool isdir();
	Array listdir();

	ptr<Path> join(const String& p_path) const;
	ptr<Path> operator /(const Path& p_other) const;
	var __div(const var& p_other) override;

	String to_string() override;
	operator const String& () const;

private:
	String _path;

};

#undef PATH_TRY

}

#endif // PATH_H


#ifndef OS_H
#define OS_H

//#include "core/native.h"

namespace carbon {

class OS : public Object {
	REGISTER_CLASS(OS, Object) {
		BIND_STATIC_FUNC("unix_time", &OS::unix_time);
		BIND_STATIC_FUNC("system", &OS::syscall, PARAMS("command"));


		// TODO: implement cross-platform manner
		BIND_STATIC_FUNC("getcwd", &OS::getcwd);
		BIND_STATIC_FUNC("chdir", &OS::chdir, PARAMS("path"));
		//BIND_STATIC_FUNC("mkdir", &OS::mkdir, PARAMS("path", "recursive"), DEFVALUES(false));
		//BIND_STATIC_FUNC("copy_file", &OS::copy_file, PARAMS("from", "to"));
		//BIND_STATIC_FUNC("copy_tree", &OS::copy_tree, PARAMS("from", "to"));
		//BIND_STATIC_FUNC("remove", &OS::remove, PARAMS("path", "recursive"), DEFVALUES(false));
		//BIND_STATIC_FUNC("rename", &OS::rename, PARAMS("old", "new")); // move and rename are the same.
		//BIND_STATIC_FUNC("listdir", &OS::listdir, PARAMS("path", "recursive"), DEFVALUES(false));
	}

public:
	static int64_t unix_time();
	static int syscall(const String& p_cmd);

	// dir/path related
	static String getcwd();
	static void chdir(const String& p_path);
	//static void mkdir(const String& p_path, bool p_recursive = false);
	//static void copy_file(const String& p_from, const String& p_to);
	//static void copy_tree(const String& p_from, const String& p_to);
	//static void remove(const String& p_path, bool p_recursive = false);
	//static void rename(const String& p_path, const String& p_new);
	//static Array listdir(const String& p_path, bool p_recursive = false);

};

}
#endif // OS_H


// compilation pipeline

#ifndef TOKENIZER_H
#define TOKENIZER_H

//#include "var/var.h"
//#include "native/file.h"

#ifndef BUILTIN_H
#define BUILTIN_H

//#include "var/var.h"

/******************************************************************************************************************/
/*                                          BUILTIN TYPES                                                         */
/******************************************************************************************************************/

namespace carbon {

class BuiltinTypes {
public:
	enum Type {
		UNKNOWN,

		_NULL,
		BOOL,
		INT,
		FLOAT,
		STRING,
		STR,
		ARRAY,
		MAP,
		//OBJECT,

		_TYPE_MAX_,
	};

private: // members
	static stdmap<Type, String> _type_list;

public:

	static String get_type_name(Type p_type);
	static Type get_type_type(const String& p_type);
	static var::Type get_var_type(Type p_type);
	static var construct(Type p_type, const stdvec<var*>& p_args);
	static bool can_construct_compile_time(Type p_type);
};

}

/******************************************************************************************************************/
/*                                          BUILTIN FUNCTIONS                                                     */
/******************************************************************************************************************/

namespace carbon {

	class BuiltinFunctions {
	public:
		enum Type {
			UNKNOWN,

			// __compiletime_functions.
			__ASSERT,
			__FUNC,
			__LINE,
			__FILE,

			// runtime functions.
			PRINT,
			PRINTLN,
			INPUT,

			HEX,
			BIN,

			MATH_MIN,
			MATH_MAX,
			MATH_POW,

			_FUNC_MAX_,
		};

	private: // members
		static stdmap<Type, String> _func_list;

	public:

		// Methods.
		static String get_func_name(Type p_func);
		static Type get_func_type(const String& p_func); // returns UNKNOWN if not valid 
		static int get_arg_count(Type p_func); // returns -1 if variadic.
		static bool can_const_fold(Type p_func);
		static bool is_compiletime(Type p_func);
		static void call(Type p_func, const stdvec<var*>& p_args, var& r_ret);
	};

}

/******************************************************************************************************************/
/*                                          NATIVE REFERENCE                                                      */
/******************************************************************************************************************/

namespace carbon {

	class NativeClassRef : public Object {
		REGISTER_CLASS(NativeClassRef, Object) {}

	private:
		String _name;

	public:
		NativeClassRef() {}
		NativeClassRef(const String& p_native_class);

		bool _is_native_ref() const override { return true; }
		String _get_native_ref() const { return _name; }

		var __call(stdvec<var*>& p_args) override; // construct
		var call_method(const String& p_name, stdvec<var*>& p_args) override; // static method call
		var get_member(const String& p_name) override; // static member, constants, functions, ...
		void set_member(const String& p_name, var& p_value) override; // static members
	};

	class BuiltinFuncRef : public Object {
		REGISTER_CLASS(BuiltinFuncRef, Object) {}

	private: // members
		BuiltinFunctions::Type _type;

	public:
		BuiltinFuncRef();
		BuiltinFuncRef(BuiltinFunctions::Type p_type);

		var __call(stdvec<var*>& p_args) override;
		String to_string() override;

		// TODO: add wrapper methods.
	};

	class BuiltinTypeRef : public Object {
		REGISTER_CLASS(BuiltinTypeRef, Object) {}

	private: // members
		BuiltinTypes::Type _type;

	public:
		BuiltinTypeRef();
		BuiltinTypeRef(BuiltinTypes::Type p_type);

		var __call(stdvec<var*>& p_args) override;
		String to_string() override;

	};

}

#endif // BUILTIN_H


namespace carbon {

enum class Token {
	UNKNOWN,
	_EOF, // EOF already a macro in <stdio.h>
	
	SYM_DOT,
	SYM_COMMA,
	SYM_COLLON,
	SYM_SEMI_COLLON,
	SYM_AT,
	SYM_HASH,
	SYM_DOLLAR,
	SYM_QUESTION,
	BRACKET_LPARAN,
	BRACKET_RPARAN,
	BRACKET_LCUR,
	BRACKET_RCUR,
	BRACKET_RSQ,
	BRACKET_LSQ,

	OP_EQ,
	OP_EQEQ,
	OP_PLUS,
	OP_PLUSEQ,
	OP_MINUS,
	OP_MINUSEQ,
	OP_MUL,
	OP_MULEQ,
	OP_DIV,
	OP_DIVEQ,
	OP_MOD,
	OP_MOD_EQ,
	OP_LT,
	OP_LTEQ,
	OP_GT,
	OP_GTEQ,
	OP_AND,
	OP_OR,
	OP_NOT,
	OP_NOTEQ,

	OP_BIT_NOT,
	OP_BIT_LSHIFT,
	OP_BIT_LSHIFT_EQ,
	OP_BIT_RSHIFT,
	OP_BIT_RSHIFT_EQ,
	OP_BIT_OR,
	OP_BIT_OR_EQ,
	OP_BIT_AND,
	OP_BIT_AND_EQ,
	OP_BIT_XOR,
	OP_BIT_XOR_EQ,

	IDENTIFIER,
	//BUILTIN_FUNC, // also identifier
	BUILTIN_TYPE,  // bool, int, String, ... cant be identifiers
	//native type, // also identifier

	KWORD_IMPORT,
	KWORD_CLASS,
	KWORD_ENUM,
	KWORD_FUNC,
	KWORD_VAR,
	KWORD_CONST,
	KWORD_NULL,
	KWORD_TRUE,
	KWORD_FALSE,
	KWORD_IF,
	KWORD_ELSE,
	KWORD_WHILE,
	KWORD_FOR,
	KWORD_SWITCH,
	KWORD_CASE,
	KWORD_DEFAULT,
	KWORD_BREAK,
	KWORD_CONTINUE,
	KWORD_STATIC,
	KWORD_THIS,
	KWORD_SUPER,
	KWORD_RETURN,
	KWORD_AND,
	KWORD_OR,
	KWORD_NOT,

	VALUE_NULL,
	VALUE_STRING,
	VALUE_INT,
	VALUE_FLOAT,
	VALUE_BOOL,

	_TK_MAX_,
};

struct TokenData {
	Token type = Token::UNKNOWN;
	int line = 0, col = 0;
	
	var constant;
	String identifier;
	BuiltinTypes::Type builtin_type = BuiltinTypes::UNKNOWN;

	TokenData() {}
	TokenData(Token p_type) { type = p_type; }

	String to_string() const;
	Vect2i get_pos() const {  return Vect2i(line, col);  }
	uint32_t get_width() const { return (uint32_t)to_string().size(); }
};

class Tokenizer {

private: // members.
	String source;
	String source_path;
	stdvec<TokenData> tokens;
	int cur_line = 1, cur_col = 1;
	int char_ptr = 0;
	int token_ptr = 0;
	// The float .3 length must be 2 but constant.to_string() result a longer size
	// and it'll set wrong token column. here is a dirty way to prevent that.
	int __const_val_token_len = 0;

	CompileTimeError _tokenize_error(Error::Type m_err_type, const String& m_msg, const DBGSourceInfo& p_dbg_info) const;

public:
	// methods.
	void tokenize(ptr<File> p_file);
	void tokenize(const String& p_source, const String& p_source_path = "<PATH-NOT-SET>");

	const TokenData& next(int p_offset = 0);
	const TokenData& peek(int p_offset = 0, bool p_safe = false) const;
	Vect2i get_pos() const;
	uint32_t get_width() const;
	const TokenData& get_token_at(const Vect2i& p_pos, bool p_safe = false) const;

	const String& get_source() const;
	const String& get_source_path() const;

	static const char* get_token_name(Token p_tk);

private:
	// methods.
	void _eat_escape(String& p_str);
	void _eat_token(Token p_tk, int p_eat_size = 1);
	void _eat_eof();
	void _eat_const_value(const var& p_value, int p_eat_size = 0);
	void _eat_identifier(const String& p_idf, int p_eat_size = 0);

	void _clear();

};

}

#endif // TOKENIZER_H


#ifndef PARSER_H
#define PARSER_H

//#include "var/var.h"
//#include "core/logger.h"
//#include "tokenizer.h"

#ifndef BYTECODE_H
#define BYTECODE_H

//#include "var/var.h"
//#include "core/type_info.h"

#ifndef OPCODES_H
#define OPCODES_H

//#include "var/var.h"
//#include "builtin.h"

namespace carbon {
	class Function;
	class Bytecode;

enum Opcode {
	GET,
	SET,
	GET_MAPPED,
	SET_MAPPED,
	SET_TRUE,
	SET_FALSE,

	OPERATOR,
	ASSIGN,

	CONSTRUCT_BUILTIN,
	CONSTRUCT_NATIVE,
	CONSTRUCT_CARBON,
	CONSTRUCT_LITERAL_ARRAY,
	CONSTRUCT_LITERAL_MAP,

	// Native and other types constructed from calling
	CALL,                // a_var(...); -> a_var.__call(...);
	CALL_FUNC,           // f(...); calling a function
	CALL_METHOD,         // a.method(...)
	CALL_BUILTIN,        // pritnln(...)
	CALL_SUPER_CTOR,     // super();
	CALL_SUPER_METHOD,   // super.method(...);

	JUMP,
	JUMP_IF,
	JUMP_IF_NOT,
	RETURN,
	ITER_BEGIN,
	ITER_NEXT,

	END,
};

struct Address {
	static constexpr int ADDR_BITS = 32;
	static constexpr int ADDR_TYPE_BITS = 8;
	static constexpr int ADDR_INDEX_BITS = ADDR_BITS - ADDR_TYPE_BITS;
	static constexpr int ADDR_TYPE_MASK = ((1 << ADDR_TYPE_BITS) - 1) << ADDR_INDEX_BITS;
	static constexpr int ADDR_INDEX_MASK = (1 << ADDR_INDEX_BITS) - 1;

	enum Type {
		_NULL = 0,
		STACK,
		PARAMETER,
		THIS,

		EXTERN,         // current translation unit or imported one
		NATIVE_CLASS,   // native class ref
		BUILTIN_FUNC,   // builtin function ref
		BUILTIN_TYPE,   // builtin type ref

		MEMBER_VAR,     // only member variables with index with offset
		STATIC_MEMBER,  // constant, function, enums, enum value, static vars, static function ... are static var

		CONST_VALUE, // searched in _global_const_values

	};

private:
	Type type = _NULL;
	bool temp = false;
	uint32_t index = 0;

public:
	Address() {}
	Address(Type p_type, uint32_t p_index, bool p_temp = false) :type(p_type), index(p_index), temp(p_temp) {}
	Address(uint32_t p_addr) :type(get_type_s(p_addr)), index(get_index_s(p_addr)) {}

	static Type get_type_s(uint32_t p_addr) { return  (Type)((p_addr & ADDR_TYPE_MASK) >> ADDR_INDEX_BITS); }
	static uint32_t get_index_s(uint32_t p_addr) { return p_addr & ADDR_INDEX_MASK; }
	static String get_type_name_s(Type p_type);

	Type get_type() const { return type; }
	uint32_t get_index() const { return index; }
	uint32_t get_address() const { return index | (type << ADDR_INDEX_BITS); }
	bool is_temp() const { return temp; }
	String as_string(const stdvec<String>* _global_names_array = nullptr, const stdvec<var>* _global_const_values = nullptr) const;

	bool operator==(const Address& p_other) const { return type == p_other.type && index == p_other.index; }
	bool operator!=(const Address& p_other) const { return !operator==(p_other); }
};

struct Opcodes {
	stdvec<uint32_t> opcodes;

	stdmap<uint32_t, uint32_t>* op_dbg; // opcode index to line

	std::stack<uint32_t> jump_out_if;
	std::stack<uint32_t> jump_out_switch;
	std::stack<uint32_t> jump_out_while;
	std::stack<uint32_t> jump_out_for;
	std::stack<uint32_t> jump_out_foreach;
	std::stack<uint32_t> jump_to_continue;
	std::stack<uint32_t> jump_out_and;
	std::stack<uint32_t> jump_out_or;
	std::stack<stdvec<uint32_t>> jump_out_break; // multiple break statement jump out to one addr.

	uint32_t last(); // last instruction
	uint32_t next(); // next instruction

	static String get_opcode_name(Opcode p_opcode);
	void insert_dbg(uint32_t p_line);

	void insert(uint32_t p_opcode);
	void insert(const Address& p_addr);
	void insert(Opcode p_opcode);

	void write_assign(const Address& dst, const Address& src);
	void write_if(const Address& p_cond);
	void write_else();
	void write_endif();
	void write_while(const Address& p_cond);
	void write_endwhile();
	void write_foreach(const Address& p_iter_value, const Address& p_iterator, const Address& p_on);
	void write_endforeach();
	void write_for(const Address& p_cond);
	void write_endfor(bool p_has_cond);
	void write_break();
	void write_continue();
	void write_return(const Address& p_ret_value);

	void write_assign_bool(const Address& dst, bool value);
	void write_and_left(const Address& p_left);
	void write_and_right(const Address& p_right, const Address& p_dst);
	void write_or_left(const Address& p_left);
	void write_or_right(const Address& p_right, const Address& p_dst);

	//void write_get_member(const Address& p_name, const Address& p_dst);
	void write_get_index(const Address& p_on, uint32_t p_name, const Address& p_dst);
	void write_set_index(const Address& p_on, uint32_t p_name, const Address& p_value);
	void write_get_mapped(const Address& p_on, const Address& p_key, const Address& p_dst);
	void write_set_mapped(const Address& p_on, const Address& p_key, const Address& p_value);
	void write_array_literal(const Address& p_dst, const stdvec<Address>& p_values);
	void write_map_literal(const Address& p_dst, const stdvec<Address>& p_keys, const stdvec<Address>& p_values);
	void write_construct_builtin_type(const Address& p_dst, BuiltinTypes::Type p_type, const stdvec<Address>& p_args);
	void write_construct_native(const Address& p_dst, uint32_t p_name, const stdvec<Address>& p_args);
	void write_construct_carbon(const Address& p_dst, uint32_t p_name, const stdvec<Address>& p_args);
	void write_call_builtin(const Address& p_ret, BuiltinFunctions::Type p_func, const stdvec<Address>& p_args);
	void write_call(const Address& p_ret, const Address& p_on, const stdvec<Address>& p_args);
	void write_call_func(const Address& p_ret, uint32_t p_name, const stdvec<Address>& p_args);
	void write_call_method(const Address& p_ret, Address& p_on, uint32_t p_method, const stdvec<Address>& p_args);
	void write_call_super_constructor(const stdvec<Address>& p_args);
	void write_call_super_method(const Address& p_ret, uint32_t p_method, const stdvec<Address>& p_args);
	void write_operator(const Address& p_dst, var::Operator p_op, const Address& p_left, const Address& p_right);

};

}

#endif // OPCODES_H


namespace carbon {

class Bytecode : public Object, public std::enable_shared_from_this<Bytecode> {
	REGISTER_CLASS(Bytecode, Object) {}

	friend class CodeGen;
	friend struct CGContext;
	friend class Function;

private: // members.
	bool _is_class = false;
	String _name;                            // name for class, path for file.

	// for ClassNode
	bool _has_base = false;
	bool _is_base_native = false;
	ptr<Bytecode> _base = nullptr;
	void* _pending_base = nullptr;           // incase base isn't fully compiled yet. Parser::ClassNode*
	String _base_native;
	ptr<Bytecode> _file = nullptr;

	stdmap<String, ptr<Bytecode>> _classes;  // for FileNode
	stdmap<String, ptr<Bytecode>> _externs;  // imported for FileNode

	stdmap<String, uint32_t> _members;       // member index. offset willbe added for inherited instances.
	stdmap<String, var> _static_vars;
	stdmap<String, var> _constants;
	stdmap<String, int64_t> _unnamed_enums;
	stdmap<String, ptr<EnumInfo>> _enums;
	stdmap<String, ptr<Function>> _functions;

	stdvec<String> _global_names_array;
	stdmap<String, uint32_t> _global_names;
	stdvec<var> _global_const_values;

	bool _member_info_built = false;          // set to true after _member_info is built
	stdmap<size_t, ptr<MemberInfo>> _member_info;
	stdmap<String, var> _member_vars;         // all members as var (constructed at runtime)

	union {
		Function* _main = nullptr;      // file
		Function* _constructor;         // class
	};
	ptr<Function> _static_initializer = nullptr;
	ptr<Function> _member_initializer = nullptr; // class
	bool _static_initialized = false;

	bool _is_compiled = false;
	bool _is_compiling = false;

//------------------------------------------------------------------------

public:
	Bytecode() {}

	void initialize();

	var __call(stdvec<var*>& p_args) override;                                    // constructor
	var call_method(const String& p_method_name, stdvec<var*>& p_args) override;  // static methods.
	var get_member(const String& p_member_name) override;                         // static member, constants, enums, functions ...
	void set_member(const String& p_member_name, var& p_value) override;          // static members.

	bool is_class() const;
	const String& get_name() const;
	var* _get_member_var_ptr(const String& p_member_name);

	bool has_base() const;
	bool is_base_native() const;
	const ptr<Bytecode>& get_base_binary() const;
	const String& get_base_native() const;

	int get_member_count() const;
	int get_member_offset() const;
	uint32_t get_member_index(const String& p_name);
	const ptr<MemberInfo> get_member_info(const String& p_member_name);
	const stdmap<size_t, ptr<MemberInfo>>& get_member_info_list();

	stdmap<String, ptr<Bytecode>>& get_classes();
	stdmap<String, ptr<Bytecode>>& get_externs();
	const stdmap<String, ptr<Function>>& get_functions() const;
	const stdmap<String, var>& get_constants();
	stdmap<String, var>& get_static_vars();

	ptr<Bytecode> get_class(const String& p_name);
	ptr<Bytecode> get_import(const String& p_name);
	ptr<Function> get_function(const String& p_name);
	var* get_static_var(const String& p_name);
	var get_constant(const String& p_name);

	const ptr<Bytecode>&  get_file() const;
	const Function* get_main() const;
	const Function* get_constructor() const;
	const Function* get_member_initializer() const;
	const Function* get_static_initializer() const;

	const String& get_global_name(uint32_t p_pos);
	var* get_global_const_value(uint32_t p_index);

private:
	uint32_t _global_name_get(const String& p_name);
	void _build_global_names_array();
	uint32_t _global_const_value_get(const var& p_value);

};

}

#endif // BYTECODE_H


namespace carbon {

#define PARSER_ERROR(m_type, m_msg, m_pos) _parser_error(m_type, m_msg, m_pos, _DBG_SOURCE)
#define UNEXP_TOKEN_ERROR(m_expected) _unexp_token_error(m_expected, _DBG_SOURCE)
#define PREDEFINED_ERROR(m_what, m_name, m_pos) _predefined_error(m_what, m_name, m_pos, _DBG_SOURCE)

class Parser {
public:
	struct Node {
		enum class Type {
			UNKNOWN,

			IMPORT,
			FILE,
			CLASS,
			ENUM,
			FUNCTION,
			BLOCK,
			IDENTIFIER,
			VAR,
			CONST,
			CONST_VALUE, // evaluvated to compile time constants ex: "str", 3.14, Array(1, 2), ...
			ARRAY,       // literal array ex: [1, 2, [3]]
			MAP,         // literal map   ex: { "key":"value", 1:[2, 3] }
			THIS,
			SUPER,
			BUILTIN_FUNCTION,
			BUILTIN_TYPE,
			CALL,
			INDEX,
			MAPPED_INDEX,
			OPERATOR,
			CONTROL_FLOW,

			_NODE_MAX_,
		};
		Type type = Type::UNKNOWN;
		Vect2i pos;
		uint32_t width = 1; // width of the node ^^^^ (dbg).
		ptr<Node> parernt_node;
		bool is_reduced = false;
		bool _is_reducing = false;
		static const char* get_node_type_name(Type p_type);
	};

	struct ClassNode;
	struct EnumNode;
	struct BuiltinFunctionNode;
	struct BuiltinTypeNode;
	struct FunctionNode;
	struct BlockNode;
	struct IdentifierNode;
	struct VarNode;
	struct ConstNode;
	struct ConstValueNode;
	struct ArrayNode;
	struct MapNode;
	struct CallNode;
	struct IndexNode;
	struct MappedIndexNode;
	struct OperatorNode;
	struct ControlFlowNode;

	struct MemberContainer : public Node {
		MemberContainer(Type type) { this->type = type; }

		ptr<EnumNode> unnamed_enum;
		stdvec<ptr<EnumNode>> enums;
		stdvec<ptr<VarNode>> vars;
		stdvec<ptr<ConstNode>> constants;
		stdvec<ptr<FunctionNode>> functions;
		stdvec<ptr<CallNode>> compiletime_functions;
	};

	struct ImportNode : public Node {
		String name;
		ptr<Bytecode> bytecode;
	};

	struct FileNode : public MemberContainer {
		String path, source;

		stdvec<ptr<ClassNode>> classes;
		stdvec<ptr<ImportNode>> imports;

		FileNode() : MemberContainer(Type::FILE) { }

	};

	struct ClassNode : public MemberContainer {
		String name;

		enum BaseType {
			NO_BASE,
			BASE_LOCAL,
			BASE_NATIVE,
			BASE_EXTERN
		};
		BaseType base_type = NO_BASE;

		String base_class_name;
		ClassNode* base_class = nullptr;
		ptr<Bytecode> base_binary = nullptr;

		bool has_super_ctor_call = false;
		FunctionNode* constructor = nullptr;

		uint32_t get_member_offset() {
			if (base_type == BASE_EXTERN) return base_binary->get_member_count();
			else if (base_type == BASE_LOCAL) return base_class->get_member_offset() + (uint32_t)base_class->vars.size();
			else return 0;
		}

		uint32_t get_member_index(const String& p_name) {
			for (uint32_t i = 0; i < (uint32_t)vars.size(); i++) {
				if (vars[i]->name == p_name) return get_member_offset() + i;
			}
			if (base_type == BASE_EXTERN) return base_binary->get_member_index(p_name);
			else if (base_type == BASE_LOCAL) return base_class->get_member_index(p_name);

			THROW_BUG("member not found");
		}

		ClassNode() : MemberContainer(Type::CLASS) { }
	};

	struct EnumValueNode {
		Vect2i pos = Vect2i(-1, -1);
		ptr<Node> expr;
		bool is_reduced = false;
		bool _is_reducing = false; // for cyclic dependancy.
		int64_t value = 0;
		EnumNode* _enum = nullptr; // if not named enum it'll be nullptr.
		EnumValueNode() {}
		EnumValueNode(ptr<Node> p_expr, Vect2i p_pos, EnumNode* p_enum = nullptr) {
			pos = p_pos;
			expr = p_expr;
			_enum = p_enum;
		}
	};
	struct EnumNode : public Node {
		String name;
		bool named_enum = false;
		// EnumValueNode could be nullptr if no custom value.
		std::map<String, EnumValueNode> values;
		EnumNode() {
			type = Type::ENUM;
		}
	};

	struct ParameterNode {
		Vect2i pos = Vect2i(-1, -1);
		String name;
		bool is_reference = false;
		ptr<Node> default_value;
		ParameterNode() {}
		ParameterNode(String p_name, Vect2i p_pos) {
			name = p_name;
			pos = p_pos;
		}
	};
	struct FunctionNode : public Node {
		String name;
		bool is_static = false;
		bool has_return = false;
		bool is_constructor = false;
		uint32_t end_line = -1; // needed for debugger, it's where destructor called
		stdvec<ParameterNode> args;
		stdvec<var> default_args;
		ptr<BlockNode> body;
		Node* parent_node;
		FunctionNode() {
			type = Type::FUNCTION;
		}
	};

	struct BlockNode : public Node {
		stdvec<ptr<Node>> statements;
		// quick reference instead of searching from statement (change to VarNode* maybe).
		stdvec<ptr<VarNode>> local_vars;
		stdvec<ptr<ConstNode>> local_const;
		BlockNode() {
			type = Type::BLOCK;
		}
	};

	struct IdentifierNode : public Node {
		String name;
		// TODO: declared_block haven't added.
		BlockNode* declared_block = nullptr; // For search in local vars.

		enum IdentifierReferenceBase {
			BASE_UNKNOWN,
			BASE_LOCAL,
			BASE_NATIVE,
			BASE_EXTERN,
		};

		enum IdentifierReference {
			REF_UNKNOWN,
			REF_PARAMETER,
			REF_LOCAL_VAR,
			REF_LOCAL_CONST,

			// these will have base.
			REF_MEMBER_VAR,
			REF_STATIC_VAR,
			REF_MEMBER_CONST,
			REF_ENUM_NAME,
			REF_ENUM_VALUE,
			REF_FUNCTION,

			REF_CARBON_CLASS,
			REF_NATIVE_CLASS,
			REF_EXTERN, // TODO: extern class and extern file.
		};

		IdentifierReferenceBase ref_base = BASE_UNKNOWN;
		IdentifierReference ref = REF_UNKNOWN;
		union {
			int param_index = 0;

			// reference from carbon local.
			VarNode* _var;
			ConstNode* _const;
			EnumValueNode* _enum_value;
			EnumNode* _enum_node;
			const ClassNode* _class;
			const FunctionNode* _func;
			BuiltinFunctions::Type _bi_func;
			
			// reference from native.
			const MethodInfo* _method_info;
			const PropertyInfo* _prop_info;
			const EnumInfo* _enum_info;
			const EnumValueInfo* _enum_value_info;
			const ClassInfo* _class_info;
			const Bytecode* _bytecode;
		};

		IdentifierNode() {
			type = Type::IDENTIFIER;
		}
		IdentifierNode(const String& p_name) {
			type = Type::IDENTIFIER;
			name = p_name;
		}
	};

	struct VarNode : public Node {
		String name;
		bool is_static = false;
		ptr<Node> assignment;
		VarNode() {
			type = Type::VAR;
		}
	};

	struct ConstNode : public Node {
		String name; // Every const are static.
		ptr<Node> assignment;
		var value;
		ConstNode() {
			type = Type::CONST;
		}
	};

	// Note: ConstValueNode isn't constant values but they are compile time known variables.
	//       and could be Array(1, 2, 3), Map if it has a literal.
	struct ConstValueNode : public Node {
		var value;
		ConstValueNode() {
			type = Type::CONST_VALUE;
		}
		ConstValueNode(const var& p_value) {
			type = Type::CONST_VALUE;
			value = p_value;
		}
	};

	struct ArrayNode : public Node {
		stdvec<ptr<Node>> elements;
		bool _can_const_fold = false;
		ArrayNode() {
			type = Type::ARRAY;
		}
	};

	struct MapNode : public Node {
		struct Pair {
			ptr<Node> key;
			ptr<Node> value;
			Pair() {}
			Pair(ptr<Node>& p_key, ptr<Node>& p_value) { key = p_key; value = p_value; }
		};
		stdvec<Pair> elements;
		bool _can_const_fold = false;
		MapNode() {
			type = Type::MAP;
		}
	};

	struct ThisNode : public Node {
		ThisNode() {
			type = Node::Type::THIS;
		}
	};

	struct SuperNode : public Node {
		SuperNode() {
			type = Node::Type::SUPER;
		}
	};

	struct BuiltinFunctionNode : public Node {
		BuiltinFunctions::Type func = BuiltinFunctions::UNKNOWN;
		BuiltinFunctionNode() {
			type = Type::BUILTIN_FUNCTION;
		}
		BuiltinFunctionNode(BuiltinFunctions::Type p_func) {
			type = Type::BUILTIN_FUNCTION;
			func = p_func;
		}
	};

	struct BuiltinTypeNode : public Node {
		BuiltinTypes::Type builtin_type = BuiltinTypes::UNKNOWN;
		BuiltinTypeNode() {
			type = Type::BUILTIN_TYPE;
		}
		BuiltinTypeNode(BuiltinTypes::Type p_cls) {
			type = Type::BUILTIN_TYPE;
			builtin_type = p_cls;
		}
	};

	struct CallNode : public Node {
		ptr<Node> base;
		// should be Node (instead of identifier node) for reduce the identifier.
		// if the method is nullptr and base is a var `a_var(...)` -> `a_var.__call(...)` will be called.
		ptr<Node> method;
		bool is_compilttime = false;
		stdvec<ptr<Node>> args;
		CallNode() {
			type = Node::Type::CALL;
		}
	};

	struct IndexNode : public Node {
		ptr<Node> base;
		ptr<IdentifierNode> member = nullptr;

		bool _ref_reduced = false;
		//ptr<IdentifierNode> _ref = nullptr; // reduced index node reference.
		IndexNode() {
			type = Node::Type::INDEX;
		}
	};

	struct MappedIndexNode : public Node {
		ptr<Node> base;
		ptr<Node> key;
		MappedIndexNode() {
			type = Node::Type::MAPPED_INDEX;
		}
	};

	struct OperatorNode : public Node {
		enum OpType {
			OP_EQ,
			OP_EQEQ,
			OP_PLUS,
			OP_PLUSEQ,
			OP_MINUS,
			OP_MINUSEQ,
			OP_MUL,
			OP_MULEQ,
			OP_DIV,
			OP_DIVEQ,
			OP_MOD,
			OP_MOD_EQ,
			OP_LT,
			OP_LTEQ,
			OP_GT,
			OP_GTEQ,
			OP_AND,
			OP_OR,
			OP_NOT,
			OP_NOTEQ,

			OP_BIT_NOT,
			OP_BIT_LSHIFT,
			OP_BIT_LSHIFT_EQ,
			OP_BIT_RSHIFT,
			OP_BIT_RSHIFT_EQ,
			OP_BIT_OR,
			OP_BIT_OR_EQ,
			OP_BIT_AND,
			OP_BIT_AND_EQ,
			OP_BIT_XOR,
			OP_BIT_XOR_EQ,

			OP_POSITIVE,
			OP_NEGATIVE,

			_OP_MAX_,
		};
		OpType op_type;
		stdvec<ptr<Node>> args;
		OperatorNode() {
			type = Type::OPERATOR;
		}
		OperatorNode(OpType p_type) {
			type = Type::OPERATOR;
			op_type = p_type;
		}
		static bool is_assignment(OpType p_op_type) {
			return
				p_op_type == OperatorNode::OpType::OP_EQ ||
				p_op_type == OperatorNode::OpType::OP_PLUSEQ ||
				p_op_type == OperatorNode::OpType::OP_MINUSEQ ||
				p_op_type == OperatorNode::OpType::OP_MULEQ ||
				p_op_type == OperatorNode::OpType::OP_DIVEQ ||
				p_op_type == OperatorNode::OpType::OP_MOD_EQ ||
				p_op_type == OperatorNode::OpType::OP_BIT_LSHIFT_EQ ||
				p_op_type == OperatorNode::OpType::OP_BIT_RSHIFT_EQ ||
				p_op_type == OperatorNode::OpType::OP_BIT_OR_EQ ||
				p_op_type == OperatorNode::OpType::OP_BIT_AND_EQ ||
				p_op_type == OperatorNode::OpType::OP_BIT_XOR_EQ;
			}
		static const char* get_op_name(OpType p_op);
	};

	struct ControlFlowNode : public Node {
		enum CfType {
			IF,
			SWITCH,
			WHILE,
			FOR,
			FOREACH,
			BREAK,
			CONTINUE,
			RETURN,

			_CF_MAX_,
		};
		struct SwitchCase {
			Vect2i pos;
			ptr<Node> expr;
			int64_t value;
			ptr<BlockNode> body;
			bool default_case = false;
		};
		CfType cf_type;
		stdvec<ptr<Node>> args;
		ptr<BlockNode> body;
		ptr<BlockNode> body_else;
		stdvec<SwitchCase> switch_cases;
		
		ControlFlowNode* break_continue = nullptr;
		FunctionNode* _return = nullptr;
		bool has_break = false;
		bool has_continue = false;

		ControlFlowNode() {
			type = Type::CONTROL_FLOW;
		}
		ControlFlowNode(CfType p_cf_type) {
			type = Type::CONTROL_FLOW;
			cf_type = p_cf_type;
		}
		static const char* get_cftype_name(CfType p_type);
	};

	/* How if block parsed
	-----------------------------------------
	if (c1){
	} else if (c2) {
	} else if (c3) {
	} else {}
	---- the above parsed into --------------
	if (c1){
	} else {
		if (c2){
		} else {
			if (c3) {
			} else {}
		}
	}
	-----------------------------------------
	*/

	// Methods.
	//void parse(const String& p_source, const String& p_file_path);
	void parse(ptr<Tokenizer> p_tokenizer);
#ifdef DEBUG_BUILD
	void print_tree() const;
#endif

private:
	friend class Analyzer;
	friend class CodeGen;
	struct Expr {
		Expr(OperatorNode::OpType p_op, const Vect2i& p_pos) { _is_op = true; op = p_op; pos = p_pos; }
		Expr(const ptr<Node>& p_node) { _is_op = false; expr = p_node; pos = p_node->pos; }
		Expr(const Expr& p_other) {
			if (p_other._is_op) { _is_op = true; op = p_other.op; } else { _is_op = false; expr = p_other.expr; }
			pos = p_other.pos;
		}
		Expr& operator=(const Expr& p_other) {
			if (p_other._is_op) { _is_op = true; op = p_other.op; } else { _is_op = false; expr = p_other.expr; }
			pos = p_other.pos;
			return *this;
		}
		~Expr() { if (!_is_op) { expr = nullptr; } }

		bool is_op() const { return _is_op; }
		Vect2i get_pos() const { return pos; }
		OperatorNode::OpType get_op() const { return op; }
		ptr<Node>& get_expr() { return expr; }
	private:
		bool _is_op = true;
		Vect2i pos;
		OperatorNode::OpType op;
		ptr<Node> expr;
	};

	struct ParserContext {
		ClassNode* current_class = nullptr;
		VarNode* current_var = nullptr;
		ConstNode* current_const = nullptr;
		FunctionNode* current_func = nullptr;
		BlockNode* current_block = nullptr;
		EnumNode* current_enum = nullptr;
		int current_statement_ind = -1;

		ControlFlowNode* current_break = nullptr;
		ControlFlowNode* current_continue = nullptr;
	};

	// members.
	ptr<FileNode> file_node;
	ptr<Tokenizer> tokenizer;
	ParserContext parser_context;

	// methods.
	template<typename T=Node, typename... Targs>
	ptr<T> new_node(Targs... p_args) {
		ptr<T> ret = newptr<T>(p_args...);
		ret->pos = tokenizer->get_pos();
		ret->width = tokenizer->get_pos();
		return ret;
	}

	CompileTimeError _parser_error(Error::Type p_type, const String& p_msg, Vect2i p_pos, const DBGSourceInfo& p_dbg_info) const;
	CompileTimeError _unexp_token_error(const char* p_expected, const DBGSourceInfo& p_dbg_info) const;
	CompileTimeError _predefined_error(const String& p_what, const String& p_name, Vect2i p_pos, const DBGSourceInfo& p_dbg_info) const;

	ptr<ImportNode> _parse_import();
	ptr<ClassNode> _parse_class();
	ptr<EnumNode> _parse_enum(ptr<Node> p_parent);
	stdvec<ptr<VarNode>> _parse_var(ptr<Node> p_parent);
	ptr<ConstNode> _parse_const(ptr<Node> p_parent);
	ptr<FunctionNode> _parse_func(ptr<Node> p_parent);

	ptr<BlockNode> _parse_block(const ptr<Node>& p_parent, bool p_single_statement = false, stdvec<Token> p_termination = { Token::BRACKET_RCUR } );
	ptr<ControlFlowNode> _parse_if_block(const ptr<BlockNode>& p_parent);

	ptr<Node> _parse_expression(const ptr<Node>& p_parent, bool p_allow_assign);
	stdvec<ptr<Node>> _parse_arguments(const ptr<Node>& p_parent);

	ptr<Node> _build_operator_tree(stdvec<Expr>& p_expr);
	static int _get_operator_precedence(OperatorNode::OpType p_op);
	void _check_identifier_predefinition(const String& p_name, Node* p_scope) const;
};


}
#endif // PARSER_H


#ifndef ANALYZER_H
#define ANALYZER_H

//#include "var/var.h"
//#include "parser.h"

#ifndef GLOBALS_H
#define GLOBALS_H

namespace carbon {

class GlobalStrings {
#define _GLOBAL_STR(m_name) constexpr static const char* m_name = #m_name
public:
	// function names
	_GLOBAL_STR(main);
	_GLOBAL_STR(copy);
	_GLOBAL_STR(to_string);

	// operator function names
	_GLOBAL_STR(__call);
	_GLOBAL_STR(__iter_begin);
	_GLOBAL_STR(__iter_has_next);
	_GLOBAL_STR(__iter_next);
	_GLOBAL_STR(__get_mapped);
	_GLOBAL_STR(__set_mapped);
	_GLOBAL_STR(__hash);

	_GLOBAL_STR(__add);
	_GLOBAL_STR(__sub);
	_GLOBAL_STR(__mul);
	_GLOBAL_STR(__div);

	//_GLOBAL_STR(__add_eq);
	//_GLOBAL_STR(__sub_eq);
	//_GLOBAL_STR(__mul_eq);
	//_GLOBAL_STR(__div_eq);

	_GLOBAL_STR(__gt);
	_GLOBAL_STR(__lt);
	_GLOBAL_STR(__eq);
};
#undef _GLOBAL_STR
}

#endif // GLOBALS_H




#define ANALYZER_ERROR(m_type, m_msg, m_pos) _analyzer_error(m_type, m_msg, m_pos, _DBG_SOURCE)
#define ANALYZER_WARNING(m_type, m_msg, m_pos) _analyzer_warning(m_type, m_msg, m_pos, _DBG_SOURCE)

namespace carbon {

class Analyzer {
public:
	void analyze(ptr<Parser> p_parser);
	const stdvec<Warning>& get_warnings() const;
private:
	friend class CodeGen;
	ptr<Parser> parser;
	ptr<Parser::FileNode> file_node; // Quick access.
	stdvec<Warning> warnings;

	template<typename T = Parser::Node, typename... Targs>
	ptr<T> new_node(Targs... p_args) {
		ptr<T> ret = newptr<T>(p_args...);
		return ret;
	}

	CompileTimeError _analyzer_error(Error::Type p_type, const String& p_msg, Vect2i p_pos, const DBGSourceInfo& p_dbg_info) const;
	 void _analyzer_warning(Warning::Type p_type, const String& p_msg, Vect2i p_pos, const DBGSourceInfo& p_dbg_info);

	var _call_compiletime_func(Parser::BuiltinFunctionNode* p_func, stdvec<var*>& args);
	void _resolve_compiletime_funcs(const ptr<Parser::CallNode>& p_func);

	void _check_identifier(ptr<Parser::Node>& p_expr);
	void _check_member_var_shadow(void* p_base, Parser::ClassNode::BaseType p_base_type, stdvec<ptr<Parser::VarNode>>& p_vars);
	void _check_operator_methods(const Parser::FunctionNode* p_func);
	void _check_super_constructor_call(const Parser::BlockNode* p_block);
	void _check_arg_count(int p_argc, int p_default_argc, int p_args_given, Vect2i p_err_pos = Vect2i(0, 0));

	void _resolve_inheritance(Parser::ClassNode* p_class);
	void _resolve_constant(Parser::ConstNode* p_const);
	void _resolve_parameters(Parser::FunctionNode* p_func);
	void _resolve_enumvalue(Parser::EnumValueNode& p_enumvalue, int* p_possible = nullptr);

	void _reduce_expression(ptr<Parser::Node>& p_expr);
	void _reduce_block(ptr<Parser::BlockNode>& p_block);
	void _reduce_identifier(ptr<Parser::Node>& p_expr);
	void _reduce_call(ptr<Parser::Node>& p_expr);
	void _reduce_indexing(ptr<Parser::Node>& p_expr);


	Parser::IdentifierNode _find_member(const Parser::MemberContainer* p_member, const String& p_name);
};

}

#endif // ANALYZER_H


#ifndef CODEGEN_H
#define CODEGEN_H

//#include "var/var.h"
//#include "globals.h"
//#include "analyzer.h"
//#include "bytecode.h"

#ifndef _FUNCTION_H
#define _FUNCTION_H

//#include "core/native.h"
//#include "opcodes.h"

namespace carbon {

class Bytecode;

class Function : public Object {
	REGISTER_CLASS(Function, Object) {}
	friend class CodeGen;

private: // members
	Bytecode* _owner;
	String _name;
	bool _is_static;
	int _arg_count; // TODO: = _is_reference.size(); maybe remove this
	stdvec<bool> _is_reference;
	stdvec<var> _default_args;
	stdvec<uint32_t> _opcodes;
	stdmap<uint32_t, uint32_t> op_dbg; // opcode line to pos
	uint32_t _stack_size;
	
public:
	const String& get_name() const;
	bool is_static() const;
	int get_arg_count() const;
	const stdvec<var>& get_default_args() const;
	const stdvec<bool>& get_is_args_ref() const;
	// TODO: parameter names : only for debugging
	uint32_t get_stack_size() const;
	const Bytecode* get_owner() const;
	
	const stdvec<uint32_t>& get_opcodes() const;
	const stdmap<uint32_t, uint32_t>& get_op_dbg() const;

	var __call(stdvec<var*>& p_args) override;
};

}

#endif // _FUNCTION_H


namespace carbon {

struct CGContext {
	// members
	Bytecode* bytecode = nullptr;
	const Parser::ClassNode* curr_class = nullptr;
	Function* function = nullptr;

	std::stack<stdmap<String, uint32_t>> stack_locals_frames;
	stdmap<String, uint32_t> stack_locals;
	stdvec<String> parameters;
	uint32_t curr_stack_temps = 0;
	uint32_t stack_max_size = 0;

	ptr<Opcodes> opcodes;
	void insert_dbg(const Parser::Node* p_node);

	void clear();
	void push_stack_locals();
	void pop_stack_locals();
	void pop_stack_temp();

	Address add_stack_local(const String& p_name);
	Address get_stack_local(const String& p_name);
	Address get_parameter(const String& p_name);
	Address add_stack_temp();

};

class CodeGen {

private: // members
	CGContext _context;
	Bytecode* _bytecode = nullptr; // the file node version.
	const Parser::FileNode* _file_node = nullptr;

public:
	ptr<Bytecode> generate(ptr<Analyzer> p_analyzer);

private:
	void _generate_members(Parser::MemberContainer* p_container, Bytecode* p_bytecode);

	ptr<Function> _generate_function(const Parser::FunctionNode* p_func, const Parser::ClassNode* p_class, Bytecode* p_bytecode);
	ptr<Function> _generate_initializer(bool p_static, Bytecode* p_bytecode, Parser::MemberContainer* p_container);
	void _generate_block(const Parser::BlockNode* p_block);
	void _generate_control_flow(const Parser::ControlFlowNode* p_cflow);
	Address _generate_expression(const Parser::Node* p_expr, Address* p_dst = nullptr);

	Address add_global_const_value(const var& p_value);
	uint32_t add_global_name(const String& p_name);

	void _pop_addr_if_temp(const Address& m_addr);
};

}

#endif // CODEGEN_H


#ifndef VM_H
#define VM_H

//#include "var/var.h"
//#include "builtin.h"
//#include "bytecode.h"
//#include "function.h"

#ifndef INSTANCE_H
#define INSTANCE_H

//#include "var/var.h"
//#include "globals.h"
//#include "bytecode.h"

namespace carbon {

class Instance : public Object, public std::enable_shared_from_this<Instance> {
	friend class VM;
	friend struct RuntimeContext;
	
	//REGISTER_CLASS(Instance, Object) {}
public:
	typedef Object Super;
	static ptr<Object> __new() { return newptr<Instance>(); }
	static constexpr const char* get_base_type_name_s() { return STR(Object); }
	virtual const char* get_base_type_name() const override { return get_base_type_name_s(); }
	static constexpr const char* get_type_name_s() { return STR(Instance); }
	static void _bind_data(NativeClasses* p_native_classes) {}
	/* overriding from regsiter class */
	virtual const char* get_type_name() const override { return blueprint->get_name().c_str(); }

private: // members
	ptr<Bytecode> blueprint;
	ptr<Object> native_instance;
	stdvec<var> members;

public:
	Instance();
	Instance(ptr<Bytecode>& p_blueprint);

	bool _is_registered() const override;
	var call_method(const String& p_method_name, stdvec<var*>& p_args) override;
	var get_member(const String& p_name) override;
	void set_member(const String& p_name, var& p_value) override;

	ptr<Object> copy(bool p_deep) /*const*/ override;
	void* get_data() override;

	// TODO: implement all the operator methods here.
	String to_string() override;
	var __call(stdvec<var*>& p_args) override;

	var __iter_begin() override;
	bool __iter_has_next() override;
	var __iter_next() override;

	var __get_mapped(const var& p_key) /*const*/ override;
	void __set_mapped(const var& p_key, const var& p_val) override;
	int64_t __hash() /*const*/ override;

	var __add(const var& p_other) /*const*/ override;
	var __sub(const var& p_other) /*const*/ override;
	var __mul(const var& p_other) /*const*/ override;
	var __div(const var& p_other) /*const*/ override;

	// += are change into (+) and (=) at compile time and can't return var& here
	//var& __add_eq(const var& p_other) override;
	//var& __sub_eq(const var& p_other) override;
	//var& __mul_eq(const var& p_other) override;
	//var& __div_eq(const var& p_other) override;

	bool __gt(const var& p_other) /*const*/ override;
	bool __lt(const var& p_other) /*const*/ override;
	bool __eq(const var& p_other) /*const*/ override;
};

}

#endif // INSTANCE_H


namespace carbon {

class VM;

class VMStack {
private: // members
	ptr<stdvec<var>> _stack;

public:
	VMStack(uint32_t p_max_size = 0);
	var* get_at(uint32_t p_pos);
};

struct RuntimeContext {
	VM* vm = nullptr;
	VMStack* stack = nullptr;
	stdvec<var*>* args = nullptr;
	var self;
	Bytecode* bytecode_class = nullptr;  // static member reference
	Bytecode* bytecode_file = nullptr;   // file node blueprint

	const Function* curr_fn = nullptr; // current function
	stdvec<var> value_args;                  // for pass by value args
	int get_arg_value_index(int p_arg_ind) const;

	var* get_var_at(const Address& p_addr);
	const String& get_name_at(uint32_t p_pos);
};

class VM {
	friend struct RuntimeContext;

private: // members
	stdmap<String, var> _native_ref;
	stdmap<uint32_t, var> _builtin_func_ref;
	stdmap<uint32_t, var> _builtin_type_ref;

public:
	int run(ptr<Bytecode> bytecode, stdvec<String> args);
	var call_function(const String& p_func_name, Bytecode* p_bytecode, ptr<Instance> p_self, stdvec<var*>& p_args);
	var call_function(const Function* p_func, Bytecode* p_bytecode, ptr<Instance> p_self, stdvec<var*>& p_args, int __stack = 0);

	static VM* singleton();
	static void cleanup();

private:
	VM() {} // singleton
	var* _get_native_ref(const String& p_name);
	var* _get_builtin_func_ref(uint32_t p_type);
	var* _get_builtin_type_ref(uint32_t p_type);

	static VM* _singleton;
	const int STACK_MAX = 1024; // TODO: increase

};

}

#endif // VM_H


#ifndef COMPILER_H
#define COMPILER_H

//#include "var/var.h"
//#include "codegen.h"

namespace carbon {

class Compiler {
public:
	enum CompileFlags {
		// TODO: bitfield
	};

private:
	static Compiler* _singleton;
	struct _Cache {
		bool compiling = true;
		ptr<Bytecode> bytecode = nullptr;
	};
	stdmap<String, _Cache> _cache;
	uint32_t _flags;
	stdvec<String> _include_dirs;
	std::stack<String> _cwd;

	Compiler() {} // private constructor singleton;

public:
	static Compiler* singleton();
	static void cleanup();

	void add_flag(CompileFlags p_flag);
	void add_include_dir(const String& p_dir);
	ptr<Bytecode> compile(const String& p_path, bool p_use_cache = true);
	ptr<Bytecode> _compile(const String& p_path);
	//ptr<Bytecode> compile_string(const String& p_source, const String& p_path = "<string-soruce>");

};

}

#endif // COMPILER_H

//#include "compiler/function.h"
//#include "compiler/bytecode.h"


namespace carbon {

inline void carbon_initialize() {
	Logger::initialize();
	Console::initialize();

	// Register native classes.
	NativeClasses::singleton()->register_class<Object>();
	NativeClasses::singleton()->register_class<OS>();
	NativeClasses::singleton()->register_class<File>();
	NativeClasses::singleton()->register_class<Path>();
	NativeClasses::singleton()->register_class<Buffer>();

	NativeClasses::singleton()->register_class<Bytecode>();
	NativeClasses::singleton()->register_class<Function>();
}

inline void carbon_cleanup() {
	Logger::cleanup();
	Console::cleanup();
	NativeClasses::cleanup();
	VM::cleanup();
	Compiler::cleanup();
}

}


#endif // CARBON_H

#ifdef CARBON_IMPLEMENTATION


#ifndef _FILE_SRC_CORE_CONSOLE_CPP_


namespace carbon {

void Console::initialize() {}
void Console::cleanup() {}

void Console::set_cursor(int p_line, int p_column) {
	_Platform::console_set_cursor(p_line, p_column);
}

void Console::get_cursor(int* p_line, int* p_column) {
	_Platform::console_get_cursor(p_line, p_column);
}

std::string Console::getline() {
	std::string ret;
	std::getline(std::cin, ret);
	return ret;
}

void Console::logf(const char* p_fmt, ...) {
	va_list args;
	va_start(args, p_fmt);
	vprintf(p_fmt, args);
	va_end(args);
}

void Console::logf_stderr(const char* p_fmt, ...) {
	va_list args;
	va_start(args, p_fmt);
	vfprintf(stderr, p_fmt, args);
	va_end(args);
}

void Console::logf(const char* p_fmt, va_list p_args, bool p_stderr, Console::Color p_forground, Console::Color p_background) {
	_Platform::console_logf(p_fmt, p_args, p_stderr, p_forground, p_background);
}

void Console::log(const char* p_message, bool p_stderr, Console::Color p_forground, Console::Color p_background) {
	_Platform::console_log(p_message, p_stderr, p_forground, p_background);
}

}

#endif //_FILE_SRC_CORE_CONSOLE_CPP_

#ifndef _FILE_SRC_CORE_LOGGER_CPP_


namespace carbon {

Logger::LogLevel Logger::level = Logger::LogLevel::VERBOSE;
Logger::LogLevel Logger::last_level = Logger::LogLevel::VERBOSE;


void Logger::initialize() {}
void Logger::cleanup() {}

void Logger::set_level(LogLevel p_level) {
	last_level = level;
	level = p_level;
}

void Logger::reset_level() {
	level = last_level;
}

Logger::LogLevel Logger::get_level() {
	return level;
}

bool Logger::is_level(LogLevel p_level) {
	return (int)p_level >= (int)level;
}

void Logger::log(const char* p_msg, Console::Color p_fg, Console::Color p_bg) {
	_log(p_msg, false, p_fg, p_bg);
}

void Logger::log(const char* p_msg, LogLevel p_level, Console::Color p_fg, Console::Color p_bg) {
	if (!is_level(p_level)) return; log(p_msg, p_fg, p_bg);
}

void Logger::log_verbose(const char* p_msg) { if (!is_level(LogLevel::VERBOSE)) return;  _log(p_msg, false);                          }
void Logger::log_info(const char* p_msg)    { if (!is_level(LogLevel::INFO))    return;  _log(p_msg, false, Console::Color::L_WHITE); }
void Logger::log_success(const char* p_msg) { if (!is_level(LogLevel::SUCCESS)) return;  _log(p_msg, false, Console::Color::L_GREEN); }
void Logger::log_warning(const char* p_msg) { if (!is_level(LogLevel::WARNING)) return;  _log(p_msg, true, Console::Color::D_YELLOW); }
void Logger::log_error(const char* p_msg)   { if (!is_level(LogLevel::ERROR))   return;  _log(p_msg, true, Console::Color::D_RED);    }

void Logger::logf_verbose(const char* p_fmt, ...) {
	if (!is_level(LogLevel::VERBOSE)) return;
	va_list args;
	va_start(args, p_fmt);
	_logf(p_fmt, args, false);
	va_end(args);
}

void Logger::logf_info(const char* p_fmt, ...) {
	if (!is_level(LogLevel::INFO)) return;
	va_list args;
	va_start(args, p_fmt);
	_logf(p_fmt, args, false, Console::Color::L_WHITE);
	va_end(args);
}

void Logger::logf_success(const char* p_fmt, ...) {
	if (!is_level(LogLevel::SUCCESS)) return;
	va_list args;
	va_start(args, p_fmt);
	_logf(p_fmt, args, false, Console::Color::L_GREEN);
	va_end(args);
}

void Logger::logf_warning(const char* p_fmt, ...) {
	if (!is_level(LogLevel::WARNING)) return;
	va_list args;
	va_start(args, p_fmt);
	_logf(p_fmt, args, true, Console::Color::D_YELLOW);
	va_end(args);
}

void Logger::logf_error(const char* p_fmt, ...) {
	if (!is_level(LogLevel::ERROR)) return;
	va_list args;
	va_start(args, p_fmt);
	_logf(p_fmt, args, true, Console::Color::D_RED);
	va_end(args);
}

void Logger::_log(const char* p_message, bool p_stderr, Console::Color p_forground, Console::Color p_background) {
	Console::log(p_message, p_stderr, p_forground, p_background);
}

void Logger::_logf(const char* p_fmt, va_list p_args, bool p_err, Console::Color p_forground, Console::Color p_background) {
	Console::logf(p_fmt, p_args, p_err, p_forground, p_background);
}

}

#endif //_FILE_SRC_CORE_LOGGER_CPP_

#ifndef _FILE_SRC_CORE_NATIVE_CPP_


namespace carbon {

NativeClasses* NativeClasses::_singleton = nullptr;

NativeClasses* NativeClasses::singleton() {
	if (_singleton == nullptr) _singleton = new NativeClasses();
	return _singleton;
}

void NativeClasses::cleanup() {
	delete _singleton;
}

void NativeClasses::bind_data(ptr<BindData> p_bind_data) {
	String class_name = p_bind_data->get_type_name();
	String data_name = p_bind_data->get_name();

	ClassEntries& entries = classes[class_name.hash()];

	if (entries.bind_data[data_name.hash()] != nullptr) {
		THROW_ERROR(Error::ATTRIBUTE_ERROR, 
			String::format("entry \"%s\" already exists on class \"%s\".", p_bind_data->get_name(), p_bind_data->get_type_name())
		);
	}
	// check initializer.
	if (class_name == data_name) {
		if (p_bind_data->get_type() != BindData::STATIC_FUNC)
			THROW_ERROR(Error::ATTRIBUTE_ERROR, String::format("attribute \"%s\" can't be the same as type name.", data_name.c_str()));
		const MethodInfo* mi = ptrcast<StaticFuncBind>(p_bind_data)->get_method_info().get();
		if (mi->get_return_type().type != var::_NULL) THROW_ERROR(Error::TYPE_ERROR, "constructor initializer must not return anything.");
		if (mi->get_arg_count() < 1 || mi->get_arg_types()[0].type != var::OBJECT) THROW_ERROR(Error::TYPE_ERROR, "constructor initializer must take the instance as the first argument.");
		entries.__constructor = ptrcast<StaticFuncBind>(p_bind_data).get();		
	}
	entries.bind_data[data_name.hash()] = p_bind_data;
}

ptr<BindData> NativeClasses::get_bind_data(const String& cls, const String& attrib) {
	if (!is_class_registered(cls))
		THROW_ERROR(Error::ATTRIBUTE_ERROR, String::format("class \"%s\" not registered on NativeClasses entries.", cls.c_str()));
	auto it = classes[cls.hash()].bind_data.find(attrib.hash());
	if (it == classes[cls.hash()].bind_data.end()) return nullptr;
	return it->second;
}

// TODO: rename to get_bind_data and add p_include_base as a parameter
ptr<BindData> NativeClasses::find_bind_data(const String& cls, const String& attrib) {
	String class_name = cls;
	while (class_name.size() != 0) {
		ptr<BindData> bind_data = NativeClasses::get_bind_data(class_name, attrib);
		if (bind_data != nullptr) return bind_data;
		class_name = NativeClasses::get_inheritance(class_name);
	}
	return nullptr;
}

const ptr<MemberInfo> NativeClasses::get_member_info(const String& p_class_name, const String& attrib) {
	ptr<BindData> bd = find_bind_data(p_class_name, attrib);
	if (bd == nullptr) return nullptr;
	return bd->get_member_info();
}

void NativeClasses::set_parent(const String& p_class_name, const String& p_parent_class_name) {
	if (is_class_registered(p_class_name))
		THROW_ERROR(Error::ATTRIBUTE_ERROR, String::format("class \"%s\" already exists on NativeClasses entries.", p_class_name.c_str()));

	classes[p_class_name.hash()].class_name = p_class_name;
	classes[p_class_name.hash()].parent_class_name = p_parent_class_name;
}

void NativeClasses::set_new_function(const String& p_class_name, __new_f p__new) {
	if (!is_class_registered(p_class_name))
		THROW_ERROR(Error::ATTRIBUTE_ERROR, String::format("class \"%s\" not registered on NativeClasses entries.", p_class_name.c_str()));
	classes[p_class_name.hash()].__new = p__new;
}

String NativeClasses::get_inheritance(const String& p_class_name) {
	if (classes[p_class_name.hash()].class_name.size() == 0)
		THROW_ERROR(Error::ATTRIBUTE_ERROR, String::format("class \"%s\" isn't registered in native class entries.", p_class_name.c_str()));
	return classes[p_class_name.hash()].parent_class_name;
}

bool NativeClasses::is_class_registered(const String& p_class_name) {
	return classes[p_class_name.hash()].class_name.size() != 0;
}

ptr<Object> NativeClasses::_new(const String& p_class_name) {
	if (!is_class_registered(p_class_name))
		THROW_ERROR(Error::ATTRIBUTE_ERROR, String::format("the class \"%s\" isn't registered in native class entries.", p_class_name.c_str()));
	return classes[p_class_name.hash()].__new();
}

const StaticFuncBind* NativeClasses::get_constructor(const String& p_class_name) {
	if (!is_class_registered(p_class_name))
		THROW_ERROR(Error::ATTRIBUTE_ERROR, String::format("the class \"%s\" isn't registered in native class entries.", p_class_name.c_str()));
	return classes[p_class_name.hash()].__constructor;
}

ptr<Object> NativeClasses::construct(const String& p_class_name, stdvec<var*>& p_args) {
	var instance = _new(p_class_name);
	p_args.insert(p_args.begin(), &instance);
	const StaticFuncBind* constructor = get_constructor(p_class_name);
	if (constructor) constructor->call(p_args);
	return instance;
}

const stdmap<size_t, ptr<BindData>>& NativeClasses::get_bind_data_list(const String& p_class_name) {
	if (!is_class_registered(p_class_name))
		THROW_ERROR(Error::ATTRIBUTE_ERROR, String::format("the class \"%s\" isn't registered in native class entries.", p_class_name.c_str()));
	return classes[p_class_name.hash()].bind_data;
}

const stdmap<size_t, ptr<MemberInfo>>& NativeClasses::get_member_info_list(const String& p_class_name) {
	if (!is_class_registered(p_class_name))
		THROW_ERROR(Error::ATTRIBUTE_ERROR, String::format("the class \"%s\" isn't registered in native class entries.", p_class_name.c_str()));
	return classes[p_class_name.hash()].member_info;
}

var NativeClasses::call_static(const String& p_base, const String& p_attrib, stdvec<var*>& p_args) {
	ptr<BindData> bd = NativeClasses::singleton()->find_bind_data(p_base, p_attrib);
	if (bd != nullptr) {
		switch (bd->get_type()) {
			case BindData::METHOD:
			case BindData::MEMBER_VAR:
				THROW_ERROR(Error::ATTRIBUTE_ERROR, String::format("cannot access non static member \"%s\" statically on base %s", p_attrib.c_str(), p_base.c_str()));
			case BindData::STATIC_FUNC:
				return static_cast<StaticFuncBind*>(bd.get())->call(p_args);
			case BindData::STATIC_VAR:
				return static_cast<StaticPropertyBind*>(bd.get())->get().__call(p_args);
			case BindData::STATIC_CONST:
			case BindData::ENUM:
			case BindData::ENUM_VALUE:
				THROW_ERROR(Error::ATTRIBUTE_ERROR, String::format("member \"%s\" on base %s isn't callable", p_attrib.c_str(), p_base.c_str()));
		}
	}
	THROW_ERROR(Error::ATTRIBUTE_ERROR, String::format("no attribute named \"%s\" base %s", p_attrib.c_str(), p_base.c_str()));
}

var NativeClasses::call_method_on(ptr<Object>& p_on, const String& p_attrib, stdvec<var*>& p_args) {
	// TODO: this is no more needed see Object::call_method_s <-- move that to here
	BindData* bd = find_bind_data(p_on->get_type_name(), p_attrib).get();
	if (bd != nullptr) {
		switch (bd->get_type()) {
			case BindData::METHOD:
				return static_cast<MethodBind*>(bd)->call(p_on.get(), p_args);
			case BindData::STATIC_FUNC:
				return static_cast<StaticFuncBind*>(bd)->call(p_args);
			case BindData::MEMBER_VAR:
				return static_cast<PropertyBind*>(bd)->get(p_on.get()).__call(p_args);
			case BindData::STATIC_VAR:
				return static_cast<StaticPropertyBind*>(bd)->get().__call(p_args);
			case BindData::STATIC_CONST:
				return static_cast<ConstantBind*>(bd)->get().__call(p_args);
			case BindData::ENUM:
				return static_cast<EnumBind*>(bd)->get()->__call(p_args);
			case BindData::ENUM_VALUE:
				THROW_ERROR(Error::OPERATOR_NOT_SUPPORTED, "enums are not callable.");
		}
	}
	THROW_ERROR(Error::ATTRIBUTE_ERROR, String::format("no attribute named \"%s\" base %s", p_attrib.c_str(), p_on->get_type_name()));
	return var();
}

// call using member info /////////////////////////

// defined in native.cpp
var MethodInfo::__call(stdvec<var*>& p_args) {
	THROW_IF_NULLPTR(_bind);
	if (_is_static) {
		return ((StaticFuncBind*)_bind)->call(p_args);
	} else {
		THROW_BUG("not sure how to call method (->* operator in c++)");
	}
	return var();
}

var PropertyInfo::__call(stdvec<var*>& p_args) {
	THROW_IF_NULLPTR(_bind);
	if (_is_static) {
		return ((StaticPropertyBind*)_bind)->get().__call(p_args);
	} else {
		THROW_BUG("not sure how to call method (->* operator in c++)");
	}
	return var();
}

}

#endif //_FILE_SRC_CORE_NATIVE_CPP_

#ifndef _FILE_SRC_CORE_TYPE_INFO_CPP_


namespace carbon {

const ptr<MemberInfo> TypeInfo::get_member_info(const var& p_var, const String& p_name) {
	if (p_var.get_type() == var::OBJECT) return get_member_info_object(p_var.operator carbon::ptr<carbon::Object>().get(), p_name);
	else return get_member_info(p_var.get_type(), p_name);
}

const ptr<MemberInfo> TypeInfo::get_member_info(var::Type p_type, const String& p_name) {
	// TODO: not using member info in var lib for encaptulation
	THROW_BUG("TODO: not completed yet.");
	switch (p_type) {
		case var::_NULL:
		case var::BOOL:
		case var::INT:
		case var::FLOAT:
			return nullptr;
		case var::STRING: return get_member_info_string(p_name);
		case var::ARRAY:  return get_member_info_array(p_name);
		case var::MAP:    return get_member_info_map(p_name);
		case var::OBJECT: return get_member_info_object(nullptr, p_name);
			break;
	}
	MISSED_ENUM_CHECK(var::_TYPE_MAX_, 9);
	DEBUG_BREAK(); THROW_ERROR(Error::BUG, "can't reach here.");
}

const stdmap<size_t, ptr<MemberInfo>>& TypeInfo::get_member_info_list(const var& p_var) {
	if (p_var.get_type() == var::OBJECT) return get_member_info_list_object(p_var.operator carbon::ptr<carbon::Object>().get());
	else return get_member_info_list(p_var.get_type());
}

const stdmap<size_t, ptr<MemberInfo>>& TypeInfo::get_member_info_list(var::Type p_type) {
	static stdmap<size_t, ptr<MemberInfo>> _null;
	switch (p_type) {
		case var::_NULL:
		case var::BOOL:
		case var::INT:
		case var::FLOAT:
			return _null;
		case var::STRING:
			return get_member_info_list_string();
		case var::ARRAY:
			return get_member_info_list_array();
		case var::MAP:
			return get_member_info_list_map();
		case var::OBJECT:
			return get_member_info_list_object(nullptr);
	}
	MISSED_ENUM_CHECK(var::_TYPE_MAX_, 9);
	DEBUG_BREAK(); THROW_ERROR(Error::BUG, "can't reach here.");
}


const ptr<MemberInfo> TypeInfo::get_member_info_object(const Object* p_instance, const String& p_member) {
	if (p_instance) return NativeClasses::singleton()->get_member_info(p_instance->get_type_name(), p_member);
	else return NativeClasses::singleton()->get_member_info(Object::get_type_name_s(), p_member);
}
const stdmap<size_t, ptr<MemberInfo>>& TypeInfo::get_member_info_list_object(const Object* p_instance) {
	if (p_instance) return NativeClasses::singleton()->get_member_info_list(p_instance->get_type_name());
	else return NativeClasses::singleton()->get_member_info_list(Object::get_type_name_s());
}


const ptr<MemberInfo> TypeInfo::get_member_info_string(const String& p_member) {
	auto& list = get_member_info_list_string();
	auto it = list.find(p_member.hash());
	if (it != list.end()) return it->second;
	return nullptr;
}

const ptr<MemberInfo> TypeInfo::get_member_info_array(const String& p_member) {
	auto& list = get_member_info_list_array();
	auto it = list.find(p_member.hash());
	if (it != list.end()) return it->second;
	return nullptr;
}

const ptr<MemberInfo> TypeInfo::get_member_info_map(const String& p_member) {
	auto& list = get_member_info_list_map();
	auto it = list.find(p_member.hash());
	if (it != list.end()) return it->second;
	return nullptr;
}

}

#endif //_FILE_SRC_CORE_TYPE_INFO_CPP_

#ifndef _FILE_SRC_CORE__ERROR_CPP_


namespace carbon {

static const char* _error_names[Error::_ERROR_MAX_] = {
	"OK",
	"BUG",
	"NULL_POINTER",
	"OPERATOR_NOT_SUPPORTED",
	"NOT_IMPLEMENTED",
	"ZERO_DIVISION",
	"TYPE_ERROR",
	"VALUE_ERROR",
	"ATTRIBUTE_ERROR",
	"INVALID_ARG_COUNT",
	"INVALID_INDEX",
	"IO_ERROR",
	"SYNTAX_ERROR",
	"ASSERTION",
	"UNEXPECTED_EOF",
	"NAME_ERROR",
	"VARIABLE_SHADOWING",
	"MISSED_ENUM_IN_SWITCH",
	"NON_TERMINATING_LOOP",
	"UNREACHABLE_CODE",
	"STAND_ALONE_EXPRESSION",
	"RETHROW",
	"STACK_OVERFLOW",
	//_ERROR_MAX_
};
MISSED_ENUM_CHECK(Error::Type::_ERROR_MAX_, 23);

std::string Throwable::get_err_name(Throwable::Type p_type) {
	THROW_INVALID_INDEX(_ERROR_MAX_, (int)p_type);
	return _error_names[p_type];
}

DBGSourceInfo::DBGSourceInfo() {}
DBGSourceInfo::DBGSourceInfo(const std::string& p_file, uint32_t p_line, const std::string& p_func) : func(p_func), file(p_file), line(p_line) {}
DBGSourceInfo::DBGSourceInfo(const std::string& p_file, const std::string& p_source, std::pair<int, int> p_pos, uint32_t p_width, const std::string& p_func)
	: func(p_func), file(p_file), pos(p_pos), line((uint32_t)p_pos.first), width(p_width) {

	const char* source = p_source.c_str();
	uint64_t cur_line = 1;
	std::stringstream ss_line;
	
	while (char c = *source) {
		if (c == '\n') {
			if (cur_line >= line + 2) break;
			cur_line++;
		} else if (cur_line == line - 1) {
			line_before += c;
		} else if (cur_line == line) {
			line_str += c;
		} else if (cur_line == line + 1) {
			line_after += c;
		}
		source++;
	}
}

Throwable::Throwable(Type p_type, const std::string& p_what, const DBGSourceInfo& p_source_info)
	: _type(p_type), _what(p_what), source_info(p_source_info) {
}
void Throwable::set_source_info(const DBGSourceInfo& p_source_info) { source_info = p_source_info; }

Error::Error(Type p_type, const std::string& p_what, const DBGSourceInfo& p_dbg_info)
	: Throwable(p_type, p_what, p_dbg_info) {}

CompileTimeError::CompileTimeError(Type p_type, const std::string& p_what, const DBGSourceInfo& p_cb_dbg, const DBGSourceInfo& p_dbg_info)
	: _cb_dbg_info(p_cb_dbg), Throwable(p_type, p_what, p_dbg_info) {}

Warning::Warning(Type p_type, const std::string& p_what, const DBGSourceInfo& p_cb_dbg, const DBGSourceInfo& p_dbg_info)
	: _cb_dbg_info(p_cb_dbg), Throwable(p_type, p_what, p_dbg_info) {}


TraceBack::TraceBack(Type p_type, const std::string& p_what,
	const DBGSourceInfo& p_cb_info, const DBGSourceInfo& p_dbg_info)
	: _cb_dbg_info(p_cb_info), Throwable(p_type, p_what, p_dbg_info) {}

TraceBack::TraceBack(const ptr<Throwable> p_nested,
	const DBGSourceInfo& p_cb_info, const DBGSourceInfo& p_dbg_info)
	: _cb_dbg_info(p_cb_info), Throwable(RETHROW, "", p_dbg_info) {
	_add_nested(p_nested);
}

void Error::console_log() const {
	Logger::set_level(Logger::JUST_LOG);
	Logger::logf_error("Error(%s) : %s\n", Error::get_err_name(get_type()).c_str(), what());
	Logger::log(String::format(  "    source : %s (%s:%i)\n", source_info.func.c_str(), source_info.file.c_str(), source_info.line).c_str(),
		Console::Color::L_SKYBLUE
	);
	Logger::reset_level();
}

void CompileTimeError::console_log() const {
	Logger::set_level(Logger::JUST_LOG);
	Logger::logf_error("Error(%s) : %s\n",      Error::get_err_name(get_type()).c_str(), what());
	Logger::log(String::format("    source : %s (%s:%i)\n", source_info.func.c_str(), source_info.file.c_str(), source_info.line).c_str(),
		Console::Color::L_SKYBLUE);
	Logger::log(String::format("        at : (%s:%i)\n", _cb_dbg_info.file.c_str(), _cb_dbg_info.line).c_str(), Console::Color::L_WHITE);

	if (_cb_dbg_info.line - 1 >= 1) // first line may not be available to log
	Logger::logf_info("%3i | %s\n",           _cb_dbg_info.line - 1,  _cb_dbg_info.line_before.c_str());
	Logger::logf_info("%3i | %s\n    | %s\n", _cb_dbg_info.line, _cb_dbg_info.line_str.c_str(), _cb_dbg_info.get_pos_str().c_str());
	Logger::logf_info("%3i | %s\n",           _cb_dbg_info.line + 1,  _cb_dbg_info.line_after.c_str());
	Logger::reset_level();
}

void Warning::console_log() const {
	Logger::set_level(Logger::JUST_LOG);
	Logger::logf_warning("Warning(%s) : %s\n", Error::get_err_name(get_type()).c_str(), what());
	Logger::log(String::format("    source : %s (%s:%i)\n", source_info.func.c_str(), source_info.file.c_str(), source_info.line).c_str(),
		Console::Color::L_SKYBLUE);
	Logger::log(String::format("        at : (%s:%i)\n", _cb_dbg_info.file.c_str(), _cb_dbg_info.line).c_str(), Console::Color::L_WHITE);

	if (_cb_dbg_info.line - 1 >= 1) // first line may not be available to log
		Logger::logf_info("%3i | %s\n", _cb_dbg_info.line - 1, _cb_dbg_info.line_before.c_str());
	Logger::logf_info("%3i | %s\n    | %s\n", _cb_dbg_info.line, _cb_dbg_info.line_str.c_str(), _cb_dbg_info.get_pos_str().c_str());
	Logger::logf_info("%3i | %s\n", _cb_dbg_info.line + 1, _cb_dbg_info.line_after.c_str());
	printf("\n"); // <-- TODO: inconsistance with the log ending.
	Logger::reset_level();
}

void TraceBack::console_log() const {
	
	// TODO: for carbon stack overflow print less.
	if (_nested != nullptr) _nested->console_log();

	Logger::set_level(Logger::JUST_LOG);
	Logger::logf_info(" > %s (%s:%i)\n", _cb_dbg_info.func.c_str(), _cb_dbg_info.file.c_str(), _cb_dbg_info.line);
	Logger::reset_level();
}

//-----------------------------------------------

std::string DBGSourceInfo::get_pos_str() const {
	// var x = blabla;
	//         ^^^^^^

	std::stringstream ss_pos;
	size_t cur_col = 0;
	bool done = false;
	for (size_t i = 0; i < line_str.size(); i++) {
		cur_col++;
		if (cur_col == pos.second) {
			for (uint32_t i = 0; i < width; i++) {
				ss_pos << '^';
			}
			done = true;
			break;
		} else if (line_str[i] != '\t') {
			ss_pos << ' ';
		} else {
			ss_pos << '\t';
		}
	}
	if (!done) ss_pos << '^';
	return ss_pos.str();
}



}

#endif //_FILE_SRC_CORE__ERROR_CPP_

#ifndef _FILE_SRC_VAR_VAR_CPP_


namespace carbon {

std::ostream& operator<<(std::ostream& p_ostream, const String& p_str) {
	p_ostream << p_str.operator std::string();
	return p_ostream;
}
std::istream& operator>>(std::istream& p_istream, String& p_str) {
	p_istream >> *p_str._data;
	return p_istream;
}
std::ostream& operator<<(std::ostream& p_ostream, const var& p_var) {
	p_ostream << p_var.to_string();
	return p_ostream;
}
std::ostream& operator<<(std::ostream& p_ostream, const Array& p_arr) {
	p_ostream << p_arr.to_string();
	return p_ostream;
}
std::ostream& operator<<(std::ostream& p_ostream, const Map& p_map) {
	p_ostream << p_map.to_string();
	return p_ostream;
}

String var::get_op_name_s(Operator op) {
	static const char* _names[] = {
		"OP_ASSIGNMENT",
		"OP_ADDITION",
		"OP_SUBTRACTION",
		"OP_MULTIPLICATION",
		"OP_DIVISION",
		"OP_MODULO",
		"OP_POSITIVE",
		"OP_NEGATIVE",
		"OP_EQ_CHECK",
		"OP_NOT_EQ_CHECK",
		"OP_LT",
		"OP_LTEQ",
		"OP_GT",
		"OP_GTEQ",
		"OP_AND",
		"OP_OR",
		"OP_NOT",
		"OP_BIT_LSHIFT",
		"OP_BIT_RSHIFT",
		"OP_BIT_AND",
		"OP_BIT_OR",
		"OP_BIT_XOR",
		"OP_BIT_NOT",
		"_OP_MAX_",
	};
	MISSED_ENUM_CHECK(Operator::_OP_MAX_, 23);
	return _names[op];
}

var::Type var::get_type() const { return type; }

void* var::get_data() {
	switch (type) {
		case _NULL:  return nullptr;
		case BOOL:   return (void*)&_data._bool;
		case INT:    return (void*)&_data._int;
		case FLOAT:  return (void*)&_data._float;
		case STRING: return _data._string.get_data();
		case ARRAY:  return _data._arr.get_data();
		case MAP:    return _data._map.get_data();
		case OBJECT: return _data._obj->get_data();
	}
	MISSED_ENUM_CHECK(_TYPE_MAX_, 9);
	THROW_ERROR(Error::BUG, "can't reach here.");
}

size_t var::hash() const {
	switch (type) {
		case _NULL:
			THROW_ERROR(Error::NULL_POINTER, "");
		case BOOL:   return std::hash<bool>{}(_data._bool);
		case INT:    return std::hash<int64_t>{}(_data._int);
		case FLOAT:  return std::hash<double>{}(_data._float);
		case STRING: return _data._string.hash();
		case ARRAY:
		case MAP:
			THROW_ERROR(Error::TYPE_ERROR, String::format("key of typt %s is unhashable.", get_type_name().c_str()));
		case OBJECT: DEBUG_BREAK(); // TODO: add hash method for objects?
			break;
	}
	MISSED_ENUM_CHECK(_TYPE_MAX_, 9);
	DEBUG_BREAK(); THROW_ERROR(Error::BUG, "can't reach here.");
}

bool var::is_hashable(var::Type p_type) {
	switch (p_type) {
		case _NULL:  return false;
		case BOOL:
		case INT:
		case FLOAT:
		case STRING:
			return true;
		case ARRAY:
		case MAP:
			return false;
		case OBJECT:
			return true;
	}
	MISSED_ENUM_CHECK(_TYPE_MAX_, 9);
	DEBUG_BREAK(); THROW_ERROR(Error::BUG, "can't reach here.");
}

bool var::is_compatible(var::Type p_type1, var::Type p_type2) {
	switch (p_type1) {
		case _NULL:  return p_type2 == _NULL;
		case VAR: return true;
		case BOOL: return (p_type2 == BOOL) || (p_type2 == INT) || (p_type2 == FLOAT);
		case INT: return (p_type2 == BOOL) || (p_type2 == INT) || (p_type2 == FLOAT);
		case FLOAT: return (p_type2 == BOOL) || (p_type2 == INT) || (p_type2 == FLOAT);
		case STRING: return p_type2 == STRING;
		case ARRAY: return p_type2 == ARRAY;
		case MAP: return p_type2 == MAP;
		case OBJECT: return p_type2 == OBJECT;
	}
	MISSED_ENUM_CHECK(_TYPE_MAX_, 9);
	DEBUG_BREAK(); THROW_ERROR(Error::BUG, "can't reach here.");
}

const char* var::get_type_name_s(var::Type p_type) {
	switch (p_type) {
		case var::_NULL:  return "null";
		case var::VAR:    return "var";
		case var::BOOL:   return "bool";
		case var::INT:    return "int";
		case var::FLOAT:  return "float";
		case var::STRING: return "String";
		case var::ARRAY:  return "Array";
		case var::MAP:    return "Map";
		case var::OBJECT: return "Object";
		default:
			return "";
	}
	MISSED_ENUM_CHECK(_TYPE_MAX_, 9);
}

void var::clear() {
	clear_data();
	type = _NULL;
}

var var::copy(bool p_deep) const {
	switch (type) {
		case _NULL:
		case BOOL:
		case INT:
		case FLOAT:
		case STRING:
			return *this;
		case ARRAY: return _data._arr.copy(p_deep);
		case MAP: return _data._map.copy(p_deep);
		case OBJECT: return _data._obj->copy(p_deep);
			break;
	}
	MISSED_ENUM_CHECK(_TYPE_MAX_, 9);
	THROW_ERROR(Error::BUG, "can't reach here.");
}


/* constructors */
var::var() {
	_data._bool = false;
	type = _NULL;
}

var::var(const var& p_copy) {
	copy_data(p_copy);
	type = p_copy.type;
}

var::var(bool p_bool) {
	type = BOOL;
	_data._bool = p_bool;
}

var::var(int p_int) {
	type = INT;
	_data._int = p_int;
}

var::var(size_t p_int) {
	type = INT;
	_data._int = p_int;
}

var::var(int64_t p_int) {
	type = INT;
	_data._int = p_int;
}

var::var(float p_float) {
	type = FLOAT;
	_data._float = p_float;
}

var::var(double p_double) {
	type = FLOAT;
	_data._float = p_double;
}

var::var(const char* p_cstring) {
	if (p_cstring == nullptr) {
		type = _NULL;
	} else {
		type = STRING;
		new(&_data._string) String(p_cstring);
	}
}

var::var(const String& p_string) {
	type = STRING;
	new(&_data._string) String(p_string);
}

var::var(const Array& p_array) {
	type = ARRAY;
	new(&_data._arr) Array(p_array);
}

var::var(const Map& p_map) {
	type = MAP;
	new(&_data._map) Map(p_map);
}

var::var(const ptr<Object>& p_other) {
	type = OBJECT;
	new(&_data._obj) ptr<Object>(p_other);
}

var::~var() {
	clear();
}

/* operator overloading */

#define VAR_OP_PRE_INCR_DECR(m_op)                                                                      \
var var::operator m_op () {                                                                             \
	switch (type) {                                                                                     \
		case INT:  return m_op _data._int;                                                              \
		case FLOAT: return m_op _data._float;                                                           \
		default: THROW_ERROR(Error::OPERATOR_NOT_SUPPORTED,                                             \
			String::format("operator " #m_op " not supported on base %s.", get_type_name().c_str()));   \
	}                                                                                                   \
	return var();                                                                                       \
}

#define VAR_OP_POST_INCR_DECR(m_op)                                                                     \
var var::operator m_op(int) {                                                                           \
	switch (type) {                                                                                     \
		case INT: return _data._int m_op;                                                               \
		case FLOAT: return _data._float m_op;                                                           \
		default: THROW_ERROR(Error::OPERATOR_NOT_SUPPORTED,                                             \
			String::format("operator " #m_op " not supported on base %s.", get_type_name().c_str()));   \
	}                                                                                                   \
	return var();                                                                                       \
}
VAR_OP_PRE_INCR_DECR(++)
	VAR_OP_PRE_INCR_DECR(--)
	VAR_OP_POST_INCR_DECR(++)
	VAR_OP_POST_INCR_DECR(--)
#undef VAR_OP_PRE_INCR_DECR
#undef VAR_OP_POST_INCR_DECR

	var& var::operator=(const var& p_other) {
	copy_data(p_other);
	return *this;
}

var var::operator[](const var& p_key) const {
	switch (type) {
		// strings can't return char as var&
		case STRING: return _data._string[p_key.operator int64_t()];
		case ARRAY:  return _data._arr[p_key.operator int64_t()];
		case MAP:    return _data._map[p_key];
		case OBJECT: return _data._obj->__get_mapped(p_key);
	}
	THROW_ERROR(Error::OPERATOR_NOT_SUPPORTED, String::format("operator[] not supported on base %s", get_type_name().c_str()));
}

var var::operator[](int index) const {
	return operator[](var(index));
}

var var::__get_mapped(const var& p_key) const {
	switch (type) {
		case STRING: {
			if (p_key.get_type() != var::INT) THROW_ERROR(Error::TYPE_ERROR, "expected a numeric value for indexing.");
			int64_t index = p_key;
			return String(_data._string[index]);
		} break;
		case ARRAY: {
			if (p_key.get_type() != var::INT) THROW_ERROR(Error::TYPE_ERROR, "expected a numeric value for indexing.");
			int64_t index = p_key;
			return _data._arr[index];
		} break;
		case MAP:
			if (!_data._map.has(p_key)) THROW_ERROR(Error::ATTRIBUTE_ERROR, String::format("key %s does not exists on base Map.", p_key.to_string()));
			return _data._map[p_key];
		case OBJECT:
			return _data._obj->__get_mapped(p_key);
	}
	THROW_ERROR(Error::OPERATOR_NOT_SUPPORTED, String::format("operator[] not supported on base %s", get_type_name().c_str()));
}

void var::__set_mapped(const var& p_key, const var& p_value) {
	switch (type) {
		case STRING: {
			if (p_key.get_type() != var::INT) THROW_ERROR(Error::TYPE_ERROR, "expected a numeric value for indexing.");
			if (p_value.get_type() != var::STRING) THROW_ERROR(Error::TYPE_ERROR, "expected a string value to assign");
			if (p_value.operator String().size() != 1) THROW_ERROR(Error::TYPE_ERROR, "expected a string of size 1 to assign");
			_data._string[p_key.operator int64_t()] = p_value.operator String()[0];
			return;
		} break;
		case ARRAY: {
			if (p_key.get_type() != var::INT) THROW_ERROR(Error::TYPE_ERROR, "expected a numeric value for indexing.");
			_data._arr[p_key.operator int64_t()] = p_value;
			return;
		} break;
		case MAP:
			_data._map[p_key] = p_value;
			return;
		case OBJECT:
			_data._obj->__set_mapped(p_key, p_value);
			return;
	}
	THROW_ERROR(Error::OPERATOR_NOT_SUPPORTED, String::format("operator[] not supported on base %s", get_type_name().c_str()));
}

var var::__iter_begin() {
	switch (type) {
		case var::_NULL:  THROW_ERROR(Error::NULL_POINTER, "");
		case var::BOOL:   THROW_ERROR(Error::OPERATOR_NOT_SUPPORTED, "boolean is not iterable.");
		case var::INT:    return newptr<_Iterator_int>(_data._int);
		case var::FLOAT:  THROW_ERROR(Error::OPERATOR_NOT_SUPPORTED, "float is not iterable.");
		case var::STRING: return newptr<_Iterator_String>(&_data._string);
		case var::ARRAY:  return newptr<_Iterator_Array>(&_data._arr);
		case var::MAP:    return newptr<_Iterator_Map>(&_data._map);
		case var::OBJECT: return _data._obj.get()->__iter_begin();
	}
	MISSED_ENUM_CHECK(_TYPE_MAX_, 9);
	DEBUG_BREAK(); THROW_ERROR(Error::BUG, "can't reach here.");
}

bool var::__iter_has_next() {
	switch (type) {
		case var::_NULL:  THROW_ERROR(Error::NULL_POINTER, "");
		case var::BOOL:   THROW_ERROR(Error::OPERATOR_NOT_SUPPORTED, "boolean is not iterable.");
		case var::INT:    THROW_ERROR(Error::OPERATOR_NOT_SUPPORTED, "integer is not iterable.");
		case var::FLOAT:  THROW_ERROR(Error::OPERATOR_NOT_SUPPORTED, "float is not iterable.");
		case var::STRING: THROW_ERROR(Error::OPERATOR_NOT_SUPPORTED, "String instance is not iterable (use __iter_begin on strings for string iterator).");
		case var::ARRAY:  THROW_ERROR(Error::OPERATOR_NOT_SUPPORTED, "Array instance is not iterable (use __iter_begin on array for array iterator).");
		case var::MAP:    THROW_ERROR(Error::OPERATOR_NOT_SUPPORTED, "Mpa instance is not iterable (use __iter_begin on map for map iterator).");
		case var::OBJECT: return _data._obj.get()->__iter_has_next();
	}
	MISSED_ENUM_CHECK(_TYPE_MAX_, 9);
	DEBUG_BREAK(); THROW_ERROR(Error::BUG, "can't reach here.");
}

var var::__iter_next() {
	switch (type) {
		case var::_NULL:  THROW_ERROR(Error::NULL_POINTER, "");
		case var::BOOL:   THROW_ERROR(Error::OPERATOR_NOT_SUPPORTED, "boolean is not iterable.");
		case var::INT:    THROW_ERROR(Error::OPERATOR_NOT_SUPPORTED, "integer is not iterable.");
		case var::FLOAT:  THROW_ERROR(Error::OPERATOR_NOT_SUPPORTED, "float is not iterable.");
		case var::STRING: THROW_ERROR(Error::OPERATOR_NOT_SUPPORTED, "String instance is not iterable (use __iter_begin on strings for string iterator).");
		case var::ARRAY:  THROW_ERROR(Error::OPERATOR_NOT_SUPPORTED, "Array instance is not iterable (use __iter_begin on array for array iterator).");
		case var::MAP:    THROW_ERROR(Error::OPERATOR_NOT_SUPPORTED, "Mpa instance is not iterable (use __iter_begin on map for map iterator).");
		case var::OBJECT: return _data._obj.get()->__iter_next();
	}
	MISSED_ENUM_CHECK(_TYPE_MAX_, 9);
	DEBUG_BREAK(); THROW_ERROR(Error::BUG, "can't reach here.");
}

var var::__call(stdvec<var*>& p_args) {
	switch (type) {
		case var::_NULL:  THROW_ERROR(Error::NULL_POINTER, "");
		case var::BOOL:   THROW_ERROR(Error::OPERATOR_NOT_SUPPORTED, "boolean is not callable.");
		case var::INT:    THROW_ERROR(Error::OPERATOR_NOT_SUPPORTED, "integer is not callable.");
		case var::FLOAT:  THROW_ERROR(Error::OPERATOR_NOT_SUPPORTED, "float is not callable.");
		case var::STRING: THROW_ERROR(Error::OPERATOR_NOT_SUPPORTED, "String is not callable.");
		case var::ARRAY:  THROW_ERROR(Error::OPERATOR_NOT_SUPPORTED, "Array is not callable.");
		case var::MAP:    THROW_ERROR(Error::OPERATOR_NOT_SUPPORTED, "Map is not callable.");
		case var::OBJECT: return _data._obj.get()->__call(p_args);
	}
	MISSED_ENUM_CHECK(_TYPE_MAX_, 9);
	DEBUG_BREAK(); THROW_ERROR(Error::BUG, "can't reach here.");
}

var var::call_method(const String& p_method, stdvec<var*>& p_args) {

	// check var methods.
	switch (p_method.const_hash()) {
		case "to_string"_hash:
			if (p_args.size() != 0) THROW_ERROR(Error::INVALID_ARG_COUNT, "expected at exactly 0 argument.");
			return to_string();
		case "copy"_hash:
			if (p_args.size() >= 2) THROW_ERROR(Error::INVALID_ARG_COUNT, "expected at maximum 1 argument.");
			if (p_args.size() == 0) return copy();
			return copy(p_args[0]->operator bool());
		case "hash"_hash:
			if (p_args.size() != 0) THROW_ERROR(Error::INVALID_ARG_COUNT, "expected at exactly 0 argument.");
			return (int64_t)hash();
		case "get_type_name"_hash:
			if (p_args.size() != 0) THROW_ERROR(Error::INVALID_ARG_COUNT, "expected at exactly 0 argument.");
			return get_type_name();

		// operators.
		case "__iter_begin"_hash:
			if (p_args.size() != 0) THROW_ERROR(Error::INVALID_ARG_COUNT, "expected at exactly 0 argument.");
			return __iter_begin();
		case "__iter_has_next"_hash:
			if (p_args.size() != 0) THROW_ERROR(Error::INVALID_ARG_COUNT, "expected at exactly 0 argument.");
			return __iter_has_next();
		case "__iter_next"_hash:
			if (p_args.size() != 0) THROW_ERROR(Error::INVALID_ARG_COUNT, "expected at exactly 0 argument.");
			return __iter_next();
		case "__get_mapped"_hash:
			if (p_args.size() != 1) THROW_ERROR(Error::INVALID_ARG_COUNT, "expected at exactly 1 argument.");
			return __get_mapped(*p_args[0]);
		case "__set_mapped"_hash:
			if (p_args.size() != 2) THROW_ERROR(Error::INVALID_ARG_COUNT, "expected at exactly 1 argument.");
			__set_mapped(*p_args[0], *p_args[1]); return var();
		case "__add"_hash:
			if (p_args.size() != 1) THROW_ERROR(Error::INVALID_ARG_COUNT, "expected at exactly 1 argument.");
			return operator +(*p_args[0]);
		case "__sub"_hash:
			if (p_args.size() != 1) THROW_ERROR(Error::INVALID_ARG_COUNT, "expected at exactly 1 argument.");
			return operator -(*p_args[0]);
		case "__mul"_hash:
			if (p_args.size() != 1) THROW_ERROR(Error::INVALID_ARG_COUNT, "expected at exactly 1 argument.");
			return operator *(*p_args[0]);
		case "__div"_hash:
			if (p_args.size() != 1) THROW_ERROR(Error::INVALID_ARG_COUNT, "expected at exactly 1 argument.");
			return operator /(*p_args[0]);
		case "__add_eq"_hash:
			if (p_args.size() != 1) THROW_ERROR(Error::INVALID_ARG_COUNT, "expected at exactly 1 argument.");
			return operator +=(*p_args[0]);
		case "__sub_eq"_hash:
			if (p_args.size() != 1) THROW_ERROR(Error::INVALID_ARG_COUNT, "expected at exactly 1 argument.");
			return operator -=(*p_args[0]);
		case "__mul_eq"_hash:
			if (p_args.size() != 1) THROW_ERROR(Error::INVALID_ARG_COUNT, "expected at exactly 1 argument.");
			return operator *=(*p_args[0]);
		case "__div_eq"_hash:
			if (p_args.size() != 1) THROW_ERROR(Error::INVALID_ARG_COUNT, "expected at exactly 1 argument.");
			return operator /=(*p_args[0]);
		case "__gt"_hash:
			if (p_args.size() != 1) THROW_ERROR(Error::INVALID_ARG_COUNT, "expected at exactly 1 argument.");
			return operator >(*p_args[0]);
		case "__lt"_hash:
			if (p_args.size() != 1) THROW_ERROR(Error::INVALID_ARG_COUNT, "expected at exactly 1 argument.");
			return operator <(*p_args[0]);
		case "__eq"_hash:
			if (p_args.size() != 1) THROW_ERROR(Error::INVALID_ARG_COUNT, "expected at exactly 1 argument.");
			return operator ==(*p_args[0]);

		case "__call"_hash:
			return __call(p_args);
	}

	// type methods.
	switch (type) {
		case var::_NULL:  THROW_ERROR(Error::NULL_POINTER, "");
		case var::BOOL:   THROW_ERROR(Error::ATTRIBUTE_ERROR, String::format("boolean has no attribute \"%s\".", p_method.c_str()));
		case var::INT:    THROW_ERROR(Error::ATTRIBUTE_ERROR, String::format("int has no attribute \"%s\".", p_method.c_str()));
		case var::FLOAT:  THROW_ERROR(Error::ATTRIBUTE_ERROR, String::format("float has no attribute \"%s\".", p_method.c_str()));
		case var::STRING: return _data._string.call_method(p_method, p_args);
		case var::ARRAY:  return _data._arr.call_method(p_method, p_args);
		case var::MAP:    return _data._map.call_method(p_method, p_args);
		case var::OBJECT: return Object::call_method_s(_data._obj, p_method, p_args);
	}
	MISSED_ENUM_CHECK(_TYPE_MAX_, 9);
	DEBUG_BREAK(); THROW_ERROR(Error::BUG, "can't reach here.");
}

var var::get_member(const String& p_name) {
	switch (type) {
		case var::_NULL:   THROW_ERROR(Error::NULL_POINTER, "");
		case var::BOOL:    THROW_ERROR(Error::ATTRIBUTE_ERROR, String::format("boolean has no attribute \"%s\".", p_name.c_str()));
		case var::INT:     THROW_ERROR(Error::ATTRIBUTE_ERROR, String::format("int has no attribute \"%s\".", p_name.c_str()));
		case var::FLOAT:   THROW_ERROR(Error::ATTRIBUTE_ERROR, String::format("float has no attribute \"%s\".", p_name.c_str()));
		case var::STRING:
		case var::ARRAY:
		case var::MAP:
			THROW_ERROR(Error::ATTRIBUTE_ERROR, String::format("attribute \"%s\" does not exists on base %s.", p_name.c_str(), get_type_name().c_str()));
		case var::OBJECT: return Object::get_member_s(_data._obj, p_name);
	}
	MISSED_ENUM_CHECK(_TYPE_MAX_, 9);
	DEBUG_BREAK(); THROW_ERROR(Error::BUG, "can't reach here.");
}

void var::set_member(const String& p_name, var& p_value) {
	switch (type) {
		case var::_NULL:  THROW_ERROR(Error::NULL_POINTER, "");
		case var::BOOL:   THROW_ERROR(Error::ATTRIBUTE_ERROR, String::format("boolean has no attribute \"%s\".", p_name.c_str()));
		case var::INT:    THROW_ERROR(Error::ATTRIBUTE_ERROR, String::format("int has no attribute \"%s\".", p_name.c_str()));
		case var::FLOAT:  THROW_ERROR(Error::ATTRIBUTE_ERROR, String::format("float has no attribute \"%s\".", p_name.c_str()));
		case var::STRING:
		case var::ARRAY:
		case var::MAP:
			THROW_ERROR(Error::ATTRIBUTE_ERROR, String::format("attribute \"%s\" does not exists on \"%s\".", p_name.c_str(), get_type_name().c_str()));
		case var::OBJECT:
			Object::set_member_s(_data._obj, p_name, p_value);
			return;
	}
	MISSED_ENUM_CHECK(_TYPE_MAX_, 9);
	DEBUG_BREAK(); THROW_ERROR(Error::BUG, "can't reach here.");
}

String var::to_string() const {
	switch (type) {
		case _NULL: return "null";
		case BOOL: return (_data._bool) ? "true" : "false";
		case INT: return String(_data._int);
		case FLOAT: return String(_data._float);
		case STRING: return _data._string;
		case ARRAY: return _data._arr.to_string();
		case MAP: return _data._map.to_string();
		case OBJECT: return _data._obj->to_string();
	}
	MISSED_ENUM_CHECK(_TYPE_MAX_, 9);
	DEBUG_BREAK(); THROW_ERROR(Error::BUG, "can't reach here.");
}

/* casting */
var::operator bool() const {
	switch (type) {
		case _NULL:  return false;
		case BOOL:   return _data._bool;
		case INT:    return _data._int != 0;
		case FLOAT:  return _data._float != 0;
		case STRING: return _data._string.size() != 0;
		case ARRAY:  return !_data._arr.empty();
		case MAP:    return !_data._map.empty();
		case OBJECT: return _data._obj.operator bool();
	}
	MISSED_ENUM_CHECK(_TYPE_MAX_, 9);
	THROW_ERROR(Error::TYPE_ERROR, String::format("can't cast \"%s\" to \"bool\".", get_type_name().c_str()));
}

var::operator int64_t() const {
	switch (type) {
		case BOOL: return _data._bool;
		case INT: return _data._int;
		case FLOAT: return (int)_data._float;
		//case STRING: return  _data._string.to_int(); // throws std::exception
	}
	THROW_ERROR(Error::TYPE_ERROR, String::format("can't cast \"%s\" to \"int\".", get_type_name().c_str()));
}

var::operator double() const {
	switch (type) {
		case BOOL: return (double)_data._bool;
		case INT: return (double)_data._int;
		case FLOAT: return _data._float;
		//case STRING: return  _data._string.to_float();
	}
	THROW_ERROR(Error::TYPE_ERROR, String::format("can't cast \"%s\" to \"float\".", get_type_name().c_str()));
}

var::operator String() const {
	if (type != STRING)
		THROW_ERROR(Error::TYPE_ERROR, String::format("can't cast \"%s\" to \"String\".", get_type_name().c_str()));
	return _data._string;
}

var::operator Array() const {
	if (type == ARRAY) {
		return _data._arr;
	}
	THROW_ERROR(Error::TYPE_ERROR, String::format("can't cast \"%s\" to \"Array\".", get_type_name().c_str()));
}

var::operator Map() const {
	if (type == MAP) {
		return _data._map;
	}
	THROW_ERROR(Error::TYPE_ERROR, String::format("can't cast \"%s\" to \"Map\".", get_type_name().c_str()));
}

var::operator int()    { return (int)operator int64_t(); }
var::operator size_t() { return (size_t)operator int64_t(); }
var::operator float()  { return (float)operator double(); }

var::operator int()    const { return (int)operator int64_t(); }
var::operator size_t() const { return (size_t)operator int64_t(); }
var::operator float()  const { return (float)operator double(); }

// casting to pointer
var::operator bool* () {
	if (type == BOOL) return &_data._bool;
	return nullptr;
}

var::operator int64_t* () {
	if (type == INT) return &_data._int;
	return nullptr;
}

var::operator double* () {
	if (type == FLOAT) return &_data._float;
	return nullptr;
}

var::operator String* () {
	if (type == STRING) return &_data._string;
	return nullptr;
}

var::operator Array* () {
	if (type == ARRAY) return &_data._arr;
	return nullptr;
}

var::operator Map* () {
	if (type == MAP) return &_data._map;
	return nullptr;
}

// casting to reference // TODO: move error messages to a general location
var::operator bool&() {
	if (type == BOOL) return _data._bool;
	THROW_ERROR(Error::TYPE_ERROR, String::format("can't cast \"%s\" to \"bool&\".", get_type_name().c_str()));
}

var::operator int64_t&() {
	if (type == INT) return _data._int;
	THROW_ERROR(Error::TYPE_ERROR, String::format("can't cast \"%s\" to \"int64_t&\".", get_type_name().c_str()));
}

var::operator double&() {
	if (type == FLOAT) return _data._float;
	THROW_ERROR(Error::TYPE_ERROR, String::format("can't cast \"%s\" to \"double&\".", get_type_name().c_str()));
}

var::operator String&() {
	if (type == STRING) return _data._string;
	THROW_ERROR(Error::TYPE_ERROR, String::format("can't cast \"%s\" to \"String&\".", get_type_name().c_str()));
}

var::operator Array&() {
	if (type == ARRAY) return _data._arr;
	THROW_ERROR(Error::TYPE_ERROR, String::format("can't cast \"%s\" to \"Array&\".", get_type_name().c_str()));
}

var::operator Map&() {
	if (type == MAP) return _data._map;
	THROW_ERROR(Error::TYPE_ERROR, String::format("can't cast \"%s\" to \"Map&\".", get_type_name().c_str()));
}

// - const& -
var::operator const String&() const {
	if (type == STRING) return _data._string;
	THROW_ERROR(Error::TYPE_ERROR, String::format("can't cast \"%s\" to \"String&\".", get_type_name().c_str()));
}

var::operator const Array&() const {
	if (type == ARRAY) return _data._arr;
	THROW_ERROR(Error::TYPE_ERROR, String::format("can't cast \"%s\" to \"Array&\".", get_type_name().c_str()));
}

var::operator const Map&() const {
	if (type == MAP) return _data._map;
	THROW_ERROR(Error::TYPE_ERROR, String::format("can't cast \"%s\" to \"Map&\".", get_type_name().c_str()));
}

/* operator overloading */
		/* comparison */

#define THROW_OPERATOR_NOT_SUPPORTED(m_op)                                                         \
do {                                                                                               \
	THROW_ERROR(Error::OPERATOR_NOT_SUPPORTED,                                                     \
		String::format("operator \"" STR(m_op) "\" not supported on operands \"%s\" and \"%s\".",  \
			get_type_name().c_str(), p_other.get_type_name().c_str())                              \
	);                                                                                             \
} while(false)

bool var::operator==(const var& p_other) const {

	switch (type) {
		case _NULL: return p_other.type == _NULL;
		case BOOL:   {
			switch (p_other.type) {
				case BOOL:  return _data._bool == p_other._data._bool;
				case INT:   return _data._bool == (bool) p_other._data._int;
				case FLOAT: return _data._bool == (bool) p_other._data._float;
			}
		} break;
		case INT:    {
			switch (p_other.type) {
				case BOOL:  return (bool) _data._int == p_other._data._bool;
				case INT:   return _data._int        == p_other._data._int;
				case FLOAT: return _data._int        == p_other._data._float;
			}
		} break;
		case FLOAT:  {
			switch (p_other.type) {
				case BOOL:  return (bool)_data._float == p_other._data._bool;
				case INT:   return _data._float       == p_other._data._int;
				case FLOAT: return _data._float       == p_other._data._float;
			}
		} break;
		case STRING: {
			if (p_other.type == STRING)
				return _data._string == p_other._data._string;
			break;
		}
		case ARRAY: {
			if (p_other.type == ARRAY) {
				return _data._arr == p_other.operator Array();
			}
			break;
		}
		case MAP: {
			if (p_other.type == MAP) {
				return _data._map == p_other.operator Map();
			}
			break;
		}
		case OBJECT: {
			return _data._obj->__eq(p_other);
		}
	}
	MISSED_ENUM_CHECK(_TYPE_MAX_, 9);
	return false; // different types.
}

bool var::operator!=(const var& p_other) const {
	return !operator==(p_other);
}

bool var::operator<(const var& p_other) const {
	switch (type) {
		case _NULL: break;
		case BOOL:  {
			switch (p_other.type) {
				case BOOL:  return (int)    _data._bool < (int)p_other._data._bool;
				case INT:   return (int)    _data._bool < p_other._data._int;
				case FLOAT: return (double) _data._bool < p_other._data._float;
			}
		} break;
		case INT:   {
			switch (p_other.type) {
				case BOOL:  return _data._int < (int)p_other._data._bool;
				case INT:   return _data._int < p_other._data._int;
				case FLOAT: return _data._int < p_other._data._float;
			}
		} break;
		case FLOAT: {
			switch (p_other.type) {
				case BOOL:  return _data._float < (double) p_other._data._bool;
				case INT:   return _data._float < p_other._data._int;
				case FLOAT: return _data._float < p_other._data._float;
			}
		} break;
		case STRING: {
			if (p_other.type == STRING)
				return _data._string < p_other._data._string;
			break;
		}
		case ARRAY: {
			if (p_other.type == ARRAY)
				return *_data._arr._data.get() < *p_other.operator Array()._data.get();
			break;
		}
		case MAP: {
			break;
		}
		case OBJECT: {
			return _data._obj->__lt(p_other);
		}
	}
	MISSED_ENUM_CHECK(_TYPE_MAX_, 9);
	THROW_OPERATOR_NOT_SUPPORTED(<);
}

bool var::operator>(const var& p_other) const {
	switch (type) {
		case _NULL: break;
		case BOOL:  {
			switch (p_other.type) {
				case BOOL:  return (int)    _data._bool > (int) p_other._data._bool;
				case INT:   return (int)    _data._bool > p_other._data._int;
				case FLOAT: return (double) _data._bool > p_other._data._float;
			}
		} break;
		case INT:   {
			switch (p_other.type) {
				case BOOL:  return _data._int > (int) p_other._data._bool;
				case INT:   return _data._int > p_other._data._int;
				case FLOAT: return _data._int > p_other._data._float;
			}
		} break;
		case FLOAT: {
			switch (p_other.type) {
				case BOOL:  return _data._float > (int)p_other._data._bool;
				case INT:   return _data._float > p_other._data._int;
				case FLOAT: return _data._float > p_other._data._float;
			}
		} break;
		case STRING: {
			if (p_other.type == STRING)
				return _data._string < p_other._data._string;
			break;
		}
		case ARRAY: {
			if (p_other.type == ARRAY)
				return *_data._arr._data.get() > *p_other.operator Array()._data.get();
			break;
		}
		case MAP:
			break;
		case OBJECT: {
			return _data._obj->__gt(p_other);
		}
	}
	MISSED_ENUM_CHECK(_TYPE_MAX_, 9);
	THROW_OPERATOR_NOT_SUPPORTED(>);
}

bool var::operator<=(const var& p_other) const {
	return *this == p_other || *this < p_other;
}
bool var::operator>=(const var& p_other) const {
	return *this == p_other || *this > p_other;
}

var var::operator +(const var& p_other) const {
	switch (type) {
		case _NULL: break;
		case BOOL:  {
			switch (p_other.type) {
				case BOOL:  return (int64_t) _data._bool + (int64_t) p_other._data._bool;
				case INT:   return (int64_t) _data._bool + p_other._data._int;
				case FLOAT: return (double) _data._bool  + p_other._data._float;
			}
		} break;
		case INT:   {
			switch (p_other.type) {
				case BOOL:  return _data._int          + (int64_t) p_other._data._bool;
				case INT:   return _data._int          + p_other._data._int;
				case FLOAT: return (double) _data._int + p_other._data._float;
			}

		} break;
		case FLOAT: {
			switch (p_other.type) {
				case BOOL:  return _data._float + (double) p_other._data._bool;
				case INT:   return _data._float + (double) p_other._data._int;
				case FLOAT: return _data._float + p_other._data._float;
			}
		} break;
		case STRING: {
			if (p_other.type == STRING)
				return _data._string + p_other._data._string;
			break;
		}
		case ARRAY: {
			if (p_other.type == ARRAY) {
				return _data._arr + p_other._data._arr;
			}
			break;
		}
		case MAP:
			break;
		case OBJECT:
			return _data._obj->__add(p_other);
	}
	MISSED_ENUM_CHECK(_TYPE_MAX_, 9);
	THROW_OPERATOR_NOT_SUPPORTED(+);
}

var var::operator-(const var& p_other) const {
	switch (type) {
		case _NULL: break;
		case BOOL:  {
			switch (p_other.type) {
				case BOOL:  return (int64_t) _data._bool - (int64_t) p_other._data._bool;
				case INT:   return (int64_t) _data._bool - p_other._data._int;
				case FLOAT: return (double) _data._bool  - p_other._data._float;
			}
		} break;
		case INT:   {
			switch (p_other.type) {
				case BOOL:  return _data._int         - (int64_t)p_other._data._bool;
				case INT:   return _data._int         - p_other._data._int;
				case FLOAT: return (double)_data._int - p_other._data._float;
			}
		} break;
		case FLOAT: {
			switch (p_other.type) {
				case BOOL:  return _data._float - (double)p_other._data._bool;
				case INT:   return _data._float - (double)p_other._data._int;
				case FLOAT: return _data._float - p_other._data._float;
			}
		} break;
		case STRING:
			break;
		case ARRAY:
		case MAP:
			break;
		case OBJECT:
			return _data._obj->__sub(p_other);
	}
	MISSED_ENUM_CHECK(_TYPE_MAX_, 9);
	THROW_OPERATOR_NOT_SUPPORTED(-);
}

var var::operator *(const var& p_other) const {
	switch (type) {
		case _NULL: break;
		case BOOL:  {
			switch (p_other.type) {
				case BOOL:  return (int64_t)_data._bool * (int64_t)p_other._data._bool;
				case INT:   return (int64_t)_data._bool * p_other._data._int;
				case FLOAT: return (double)_data._bool  * p_other._data._float;
			}
		} break;
		case INT:   {
			switch (p_other.type) {
				case BOOL:  return _data._int         * (int64_t)p_other._data._bool;
				case INT:   return _data._int         * p_other._data._int;
				case FLOAT: return (double)_data._int * p_other._data._float;
				case STRING: {
					String ret;
					for (int64_t i = 0; i < _data._int; i++) {
						ret += p_other._data._string;
					}
					return ret;
				}
				case ARRAY: {
					Array ret;
					ret._data->reserve(ret._data->size() * _data._int);
					for (int64_t i = 0; i < _data._int; i++) {
						ret._data->insert(ret._data->end(), p_other._data._arr._data->begin(), p_other._data._arr._data->end());
					}
					return ret;
					break;
				}
			}
		} break;
		case FLOAT: {
			switch (p_other.type) {
				case BOOL:  return _data._float * (double)p_other._data._bool;
				case INT:   return _data._float * (double)p_other._data._int;
				case FLOAT: return _data._float * p_other._data._float;
			}
		} break;
		case STRING:
			if (p_other.type == INT) {
				String ret;
				for (int64_t i = 0; i < p_other._data._int; i++) {
					ret += _data._string;
				}
				return ret;
			}
			break;
		case ARRAY:
			if (p_other.type == INT) {
				Array ret;
				ret._data->reserve(_data._arr.size() * p_other._data._int);
				for (int64_t i = 0; i < p_other._data._int; i++) {
					ret._data->insert(ret._data->end(), _data._arr._data->begin(), _data._arr._data->end());
				}
				return ret;
			}
			break;
		case MAP:
			break;
		case OBJECT:
			return _data._obj->__mul(p_other);
	}
	MISSED_ENUM_CHECK(_TYPE_MAX_, 9);
	THROW_OPERATOR_NOT_SUPPORTED(*);
}

var var::operator /(const var& p_other) const {
	switch (type) {
		case _NULL: break;
		case BOOL:  {
			switch (p_other.type) {
				case BOOL:
					if (p_other._data._bool == false) THROW_ERROR(Error::ZERO_DIVISION, "");
					return (int64_t)_data._bool / (int64_t)p_other._data._bool;
				case INT:
					if (p_other._data._int == 0) THROW_ERROR(Error::ZERO_DIVISION, "");
					return (int64_t)_data._bool / p_other._data._int;
				case FLOAT:
					if (p_other._data._float == 0.0) THROW_ERROR(Error::ZERO_DIVISION, "");
					return (double)_data._bool  / p_other._data._float;
			}
		} break;
		case INT:   {
			switch (p_other.type) {
				case BOOL:
					if (p_other._data._bool == false) THROW_ERROR(Error::ZERO_DIVISION, "");
					return _data._int         / (int64_t)p_other._data._bool;
				case INT:
					if (p_other._data._int == 0) THROW_ERROR(Error::ZERO_DIVISION, "");
					return _data._int         / p_other._data._int;
				case FLOAT:
					if (p_other._data._float == 0.0) THROW_ERROR(Error::ZERO_DIVISION, "");
					return (double)_data._int / p_other._data._float;
			}
		} break;
		case FLOAT: {
			switch (p_other.type) {
				case BOOL:
					if (p_other._data._bool == false) THROW_ERROR(Error::ZERO_DIVISION, "");
					return _data._float / (double)p_other._data._bool;
				case INT:
					if (p_other._data._int == 0) THROW_ERROR(Error::ZERO_DIVISION, "");
					return _data._float / (double)p_other._data._int;
				case FLOAT:
					if (p_other._data._float == 0.0) THROW_ERROR(Error::ZERO_DIVISION, "");
					return _data._float / p_other._data._float;
			}
		} break;
		case STRING:
			break;
		case ARRAY:
		case MAP:
			break;
		case OBJECT:
			return _data._obj->__div(p_other);
	}
	MISSED_ENUM_CHECK(_TYPE_MAX_, 9);
	THROW_OPERATOR_NOT_SUPPORTED(/);
}

var var::operator %(const var& p_other) const {
	switch (type) {
		case _NULL: break;
		case BOOL: {
			switch (p_other.type) {
				case BOOL: return (int)(_data._bool) % (int)(p_other._data._bool);
				case INT: return (int)_data._bool % p_other._data._int;
			}
		}
		case INT: {
			switch (p_other.type) {
				case BOOL: return _data._int % (int)(p_other._data._bool);
				case INT: return _data._int % p_other._data._int;
			}
		}
		case STRING: {
			return _data._string % p_other;
		}
	}
	MISSED_ENUM_CHECK(_TYPE_MAX_, 9);
	THROW_OPERATOR_NOT_SUPPORTED(%%);
}

var& var::operator+=(const var& p_other) {
	switch (type) {
		case _NULL: break;
		case BOOL: {
			switch (p_other.type) {
				case BOOL:  type = INT;   _data._int   = (int)_data._bool    + (int)p_other._data._bool; return *this;
				case INT:   type = INT;   _data._int   = (int)_data._bool    + p_other._data._int;       return *this;
				case FLOAT: type = FLOAT; _data._float = (double)_data._bool + p_other._data._float;     return *this;
			}
		} break;
		case INT:   {
			switch (p_other.type) {
				case BOOL:                _data._int   =         _data._int + (int)p_other._data._bool;  return *this;
				case INT:                 _data._int   =         _data._int + p_other._data._int;        return *this;
				case FLOAT: type = FLOAT; _data._float = (double)_data._int + p_other._data._float;      return *this;
			}
		} break;
		case FLOAT: {
			switch (p_other.type) {
				case BOOL:  _data._float = _data._float + (double)p_other._data._bool;  return *this;
				case INT:   _data._float = _data._float + (double)p_other._data._int;   return *this;
				case FLOAT: _data._float = _data._float + p_other._data._float;         return *this;
			}
		} break;
		case STRING: {
			if (p_other.type == STRING) {
				_data._string += p_other._data._string;
				return *this;
			}
			break;
		}
		case ARRAY: {
			if (p_other.type == ARRAY) {
				_data._arr += p_other._data._arr;
				return *this;
			}
			break;
		}
		case MAP:
			break;
		case OBJECT:
			return _data._obj->__add_eq(p_other);
	}
	MISSED_ENUM_CHECK(_TYPE_MAX_, 9);
	THROW_OPERATOR_NOT_SUPPORTED(+=);
}

var& var::operator-=(const var& p_other) {
	switch (type) {
		case _NULL: break;
		case BOOL: {
			switch (p_other.type) {
				case BOOL:  type = INT;   _data._int   = (int)_data._bool    - (int)p_other._data._bool; return *this;
				case INT:   type = INT;   _data._int   = (int)_data._bool    - p_other._data._int;       return *this;
				case FLOAT: type = FLOAT; _data._float = (double)_data._bool - p_other._data._float;     return *this;
			}
		} break;
		case INT:   {
			switch (p_other.type) {
				case BOOL:                _data._int   = (int)   _data._int - (int)p_other._data._bool;  return *this;
				case INT:                 _data._int   =         _data._int - p_other._data._int;        return *this;
				case FLOAT: type = FLOAT; _data._float = (double)_data._int - p_other._data._float;      return *this;
			}
		} break;
		case FLOAT: {
			switch (p_other.type) {
				case BOOL:  _data._float = _data._float - (double)p_other._data._bool;  return *this;
				case INT:   _data._float = _data._float - (double)p_other._data._int;   return *this;
				case FLOAT: _data._float = _data._float - p_other._data._float;         return *this;
			}
		} break;
		case STRING:
		case ARRAY:
		case MAP:
			break;
		case OBJECT:
			return _data._obj->__sub_eq(p_other);
	}
	MISSED_ENUM_CHECK(_TYPE_MAX_, 9);
	THROW_OPERATOR_NOT_SUPPORTED(-=);
}


var& var::operator*=(const var& p_other) {
	switch (type) {
		case _NULL: break;
		case BOOL: {
			switch (p_other.type) {
				case BOOL:  type = INT;   _data._int   = (int64_t)_data._bool    * (int64_t)p_other._data._bool; return *this;
				case INT:   type = INT;   _data._int   = (int64_t)_data._bool    * p_other._data._int;       return *this;
				case FLOAT: type = FLOAT; _data._float = (double)_data._bool     * p_other._data._float;     return *this;
			}
		} break;
		case INT:   {
			switch (p_other.type) {
				case BOOL:                _data._int   = (int64_t)_data._int * (int64_t)p_other._data._bool;  return *this;
				case INT:                 _data._int   =         _data._int  * p_other._data._int;        return *this;
				case FLOAT: type = FLOAT; _data._float = (double)_data._int  * p_other._data._float;      return *this;
				case STRING: {
					String self;
					for (int64_t i = 0; i < _data._int; i++) {
						self += p_other._data._string;
					}
					type = STRING; new(&_data._string) String(self); return *this;
				}
				case ARRAY: {
					Array self;
					for (int64_t i = 0; i < _data._int; i++) {
						self._data->insert(self._data->end(), p_other._data._arr._data->begin(), p_other._data._arr._data->end());
					}
					clear();
					type = ARRAY; _data._arr = self; return *this;
				}
			}
		} break;
		case FLOAT: {
			switch (p_other.type) {
				case BOOL:  _data._float = _data._float * (double)p_other._data._bool;  return *this;
				case INT:   _data._float = _data._float * (double)p_other._data._int;   return *this;
				case FLOAT: _data._float = _data._float * p_other._data._float;         return *this;
			}
		} break;
		case STRING:
			if (p_other.type == INT) {
				String self;
				for (int64_t i = 0; i < p_other._data._int; i++) {
					self += _data._string;
				}
				_data._string = self; return *this;
			}
			break;
		case ARRAY: {
			if (p_other.type == INT) {
				_data._arr.reserve(_data._arr.size() * p_other._data._int);
				for (int64_t i = 0; i < p_other._data._int -1; i++) {
					_data._arr._data->insert(_data._arr._data->end(), _data._arr._data->begin(), _data._arr._data->end());
				}
				return *this;
			}
		}
		case MAP:
			break;
		case OBJECT:
			return _data._obj->__mul_eq(p_other);
	}
	MISSED_ENUM_CHECK(_TYPE_MAX_, 9);
	THROW_OPERATOR_NOT_SUPPORTED(*=);
}

var& var::operator/=(const var& p_other) {
	switch (type) {
		case _NULL: break;
		case BOOL: {
			switch (p_other.type) {
				case BOOL:
					if (p_other._data._bool == false) THROW_ERROR(Error::ZERO_DIVISION, "");
					type = INT;   _data._int   = (int)_data._bool    / (int)p_other._data._bool; return *this;
				case INT:
					if (p_other._data._int == 0) THROW_ERROR(Error::ZERO_DIVISION, "");
					type = INT;   _data._int   = (int)_data._bool    / p_other._data._int;       return *this;
				case FLOAT:
					if (p_other._data._float == 0.0) THROW_ERROR(Error::ZERO_DIVISION, "");
					type = FLOAT; _data._float = (double)_data._bool / p_other._data._float;     return *this;
			}
		} break;
		case INT:   {
			switch (p_other.type) {
				case BOOL:
					if (p_other._data._bool == false) THROW_ERROR(Error::ZERO_DIVISION, "");
					_data._int   = (int)   _data._int / (int)p_other._data._bool;  return *this;
				case INT:
					if (p_other._data._int == 0) THROW_ERROR(Error::ZERO_DIVISION, "");
					_data._int   =         _data._int / p_other._data._int;        return *this;
				case FLOAT:
					if (p_other._data._float == 0.0) THROW_ERROR(Error::ZERO_DIVISION, "");
					type = FLOAT; _data._float = (double)_data._int / p_other._data._float;      return *this;
			}
		} break;
		case FLOAT: {
			switch (p_other.type) {
				case BOOL:
					if (p_other._data._bool == false) THROW_ERROR(Error::ZERO_DIVISION, "");
					_data._float = _data._float / (double)p_other._data._bool;  return *this;
				case INT:
					if (p_other._data._int == 0) THROW_ERROR(Error::ZERO_DIVISION, "");
					_data._float = _data._float / (double)p_other._data._int;   return *this;
				case FLOAT:
					if (p_other._data._float == 0.0) THROW_ERROR(Error::ZERO_DIVISION, "");
					_data._float = _data._float / p_other._data._float;         return *this;
			}
		} break;
		case STRING:
			break;
		case ARRAY:
		case MAP:
			break;
		case OBJECT:
			return _data._obj->__mul_eq(p_other);
	}
	MISSED_ENUM_CHECK(_TYPE_MAX_, 9);
	THROW_OPERATOR_NOT_SUPPORTED(/=);
}

var& var::operator %=(const var& p_other) {
	switch (type) {
		case _NULL: break;
		//case BOOL: break;
		case INT: {
			switch (p_other.type) {
				//case BOOL: _data._int %= (int)(p_other._data._bool); return *this;
				case INT: _data._int %= p_other._data._int; return *this;
			}
		}
	}
	THROW_OPERATOR_NOT_SUPPORTED(%=);
}

void var::copy_data(const var& p_other) {
	clear_data();
	type = p_other.type;
	switch (p_other.type) {
		case var::_NULL: break;
		case var::BOOL:
			_data._bool = p_other._data._bool;
			break;
		case var::INT:
			_data._int = p_other._data._int;
			break;
		case var::FLOAT:
			_data._float = p_other._data._float;
			break;
		case var::STRING:
			new(&_data._string) String(p_other._data._string);
			break;
		case var::ARRAY:
			new(&_data._arr) Array(p_other._data._arr);
			break;
		case var::MAP:
			new(&_data._map) Map(p_other._data._map);
			break;
		case var::OBJECT:
			new(&_data._obj) ptr<Object>(p_other._data._obj);
			return;
	}
	MISSED_ENUM_CHECK(_TYPE_MAX_, 9);
}

void var::clear_data() {
	switch (type) {
		case var::_NULL:
		case var::BOOL:
		case var::INT:
		case var::FLOAT:
			return;
		case var::STRING:
			_data._string.~String();
			return;
		case var::ARRAY:
			_data._arr.~Array();
			break;
		case var::MAP:
			_data._map.~Map();
			break;
		case var::OBJECT:
			_data._obj = nullptr;
			break;
	}
	MISSED_ENUM_CHECK(_TYPE_MAX_, 9);
}

String var::get_type_name() const {
	if (type == OBJECT) {
		return _data._obj->get_type_name();
	} else {
		return get_type_name_s(type);
	}
}

}

#endif //_FILE_SRC_VAR_VAR_CPP_

#ifndef _FILE_SRC_VAR__ARRAY_CPP_



namespace carbon {
const stdmap<size_t, ptr<MemberInfo>>& TypeInfo::get_member_info_list_array() {
	static stdmap<size_t, ptr<MemberInfo>> member_info_s = {
		_NEW_METHOD_INFO("size",                                                    var::INT   ),
		_NEW_METHOD_INFO("empty",                                                   var::BOOL  ),
		_NEW_METHOD_INFO("push_back",    _PARAMS("element"),  _TYPES(var::VAR),     var::_NULL ),
		_NEW_METHOD_INFO("pop_back",                                                var::_NULL ),
		_NEW_METHOD_INFO("append",       _PARAMS("element"),  _TYPES(var::VAR),     var::ARRAY ),
		_NEW_METHOD_INFO("pop",                                                     var::VAR   ),
		_NEW_METHOD_INFO("clear",                                                   var::_NULL ),
		_NEW_METHOD_INFO("at",           _PARAMS("index"),    _TYPES(var::INT),     var::VAR   ),
		_NEW_METHOD_INFO("resize",       _PARAMS("size" ),    _TYPES(var::INT),     var::_NULL ),
		_NEW_METHOD_INFO("reserve",      _PARAMS("size" ),    _TYPES(var::INT),     var::_NULL ),
		_NEW_METHOD_INFO("sort",                                                    var::ARRAY ),
		_NEW_METHOD_INFO("contains",    _PARAMS("element"),   _TYPES(var::VAR),     var::BOOL  ),
	};
	return member_info_s;
}

var Array::call_method(const String& p_method, const stdvec<var*>& p_args) {
	_check_method_and_args<Array>(p_method, p_args, TypeInfo::get_member_info_array(p_method));
	switch (p_method.const_hash()) {
		case "size"_hash:      return (int64_t)size();
		case "empty"_hash:     return empty();
		case "push_back"_hash: { push_back(*p_args[0]); return var(); }
		case "pop_back"_hash: { pop_back(); return var(); }
		case "append"_hash:    return append(*p_args[0]);
		case "pop"_hash:       return pop();
		case "clear"_hash: { clear(); return var(); }
		case "insert"_hash: { insert(*p_args[0], *p_args[1]); return var(); }
		case "at"_hash:        return at(p_args[0]->operator int64_t());
		case "resize"_hash: { resize((size_t)p_args[0]->operator int64_t()); return var(); }
		case "reserve"_hash: { reserve((size_t)p_args[0]->operator int64_t()); return var(); }
		case "sort"_hash: { return sort(); }
		case "contains"_hash: { return contains(p_args[0]); }
	}
	// TODO: add more.
	DEBUG_BREAK(); THROW_ERROR(Error::BUG, "can't reach here.");
}

Array::Array() { _data = newptr<stdvec<var>>(); }
Array::Array(const Array& p_copy) { _data = p_copy._data; }

const stdvec<var>* Array::get_stdvec() const {
	return _data.operator->();
}

void* Array::get_data() const {
	return (void*)_data->data();
}

size_t Array::size() const { return _data->size(); }
bool   Array::empty() const { return _data->empty(); }
void   Array::push_back(const var& p_var) { _data->push_back(p_var); }
void   Array::pop_back() { _data->pop_back(); }
Array& Array::append(const var& p_var) { push_back(p_var); return *this; }
void   Array::clear() { (*_data).clear(); }
void   Array::insert(int64_t p_index, const var& p_var) { _data->insert(_data->begin() + (int)p_index, p_var); }
void   Array::resize(size_t p_size) { _data->resize(p_size); }
void   Array::reserve(size_t p_size) { _data->reserve(p_size); }
Array& Array::sort() { std::sort(_data->begin(), _data->end()); return *this; }
var&   Array::back() { return (*_data).back(); }
var&   Array::front() { return (*_data).front(); }

bool Array::contains(const var& p_elem) {
	for (const var& e : (*_data)) {
		if (e == p_elem) return true;
	}
	return false;
}

var& Array::at(int64_t p_index) {
	if (0 <= p_index && p_index < (int64_t)size())
		return (*_data).at((size_t)p_index);
	if ((int64_t)size() * -1 <= p_index && p_index < 0)
		return (*_data).at(size() + (size_t)p_index);
	THROW_ERROR(Error::INVALID_INDEX, String::format("Array index %i is invalid.", p_index));
}

var& Array::operator[](int64_t p_index) const {
	if (0 <= p_index && p_index < (int64_t)size())
		return _data->operator[]((size_t)p_index);
	if ((int64_t)size() * -1 <= p_index && p_index < 0)
		return _data->operator[](size() + (size_t)p_index);
	THROW_ERROR(Error::INVALID_INDEX, String::format("Array index %i is invalid.", p_index));
}
var& Array::operator[](int64_t p_index) {
	if (0 <= p_index && p_index < (int64_t)size())
		return _data->operator[]((size_t)p_index);
	if ((int64_t)size() * -1 <= p_index && p_index < 0)
		return _data->operator[](size() + (size_t)p_index);
	THROW_ERROR(Error::INVALID_INDEX, String::format("Array index %i is invalid.", p_index));
}


String Array::to_string() const {
	// TODO: if the array contains itself it'll stack overflow.
	std::stringstream ss;
	ss << "[ ";
	for (unsigned int i = 0; i < _data->size(); i++) {
		if ((*_data)[i].get_type() == var::STRING) ss << "\"";
		ss << (*_data)[i].to_string();
		if ((*_data)[i].get_type() == var::STRING) ss << "\"";
		if (i != _data->size() - 1) ss << ", ";
		else ss << " ";
	}
	ss << "]";
	return ss.str();
}

var Array::pop() { var ret = this->operator[](size() - 1); pop_back(); return ret; }

bool Array::operator ==(const Array& p_other) const {
	if (size() != p_other.size())
		return false;
	for (size_t i = 0; i < size(); i++) {
		if (operator[](i) != p_other[i])
			return false;
	}
	return true;
}

Array Array::copy(bool p_deep) const {
	Array ret;
	for (size_t i = 0; i < size(); i++) {
		if (p_deep)
			ret.push_back(operator[](i).copy(true));
		else
			ret.push_back(operator[](i));
	}
	return ret;
}

Array::operator bool() const { return empty(); }

Array Array::operator+(const Array& p_other) const {
	Array ret = copy();
	for (size_t i = 0; i < p_other.size(); i++) {
		ret.push_back(p_other[i].copy());
	}
	return ret;
}

Array& Array::operator+=(const Array& p_other) {
	for (size_t i = 0; i < p_other.size(); i++) {
		push_back(p_other[i].copy());
	}
	return *this;
}

Array& Array::operator=(const Array& p_other) {
	_data = p_other._data;
	return *this;
}


} // namespace carbon

#endif //_FILE_SRC_VAR__ARRAY_CPP_

#ifndef _FILE_SRC_VAR__MAP_CPP_



namespace carbon {
	
const stdmap<size_t, ptr<MemberInfo>>& TypeInfo::get_member_info_list_map() {
	static stdmap<size_t, ptr<MemberInfo>> member_info = {
		_NEW_METHOD_INFO("size",                                                            var::INT   ),
		_NEW_METHOD_INFO("empty",                                                           var::BOOL  ),
		_NEW_METHOD_INFO("insert",  _PARAMS("key", "value"),  _TYPES(var::VAR, var::VAR),   var::_NULL ),
		_NEW_METHOD_INFO("clear",                                                           var::_NULL ),
		_NEW_METHOD_INFO("has",     _PARAMS("what"),         _TYPES(var::VAR),              var::BOOL  ),
		_NEW_METHOD_INFO("at",      _PARAMS("key"),          _TYPES(var::VAR),              var::VAR   ),
	};
	return member_info;
}

var Map::call_method(const String& p_method, const stdvec<var*>& p_args) {
	_check_method_and_args<Map>(p_method, p_args, TypeInfo::get_member_info_map(p_method));
	switch (p_method.const_hash()) {
		case "size"_hash:   return (int64_t)size();
		case "empty"_hash:  return empty();
		case "insert"_hash: insert(*p_args[0], *p_args[1]); return var();
		case "clear"_hash:  clear(); return var();
		case "has"_hash:    return has(*p_args[0]);
		case "at"_hash:     return at(*p_args[0]);
	}
	// TODO: more.
	DEBUG_BREAK(); THROW_ERROR(Error::BUG, "can't reach here.");
}

Map::Map() {
	_data = newptr<_map_internal_t>();
}
Map::Map(const ptr<_map_internal_t>& p_data) {
	_data = p_data;
}
Map::Map(const Map& p_copy) {
	_data = p_copy._data;
}

void* Map::get_data() const { return _data.operator->(); }

String Map::to_string() const {
	std::stringstream ss;
	ss << "{ ";
	for (_map_internal_t::iterator it = (*_data).begin(); it != (*_data).end(); it++) {
		if (it != (*_data).begin()) ss << ", ";
		
		if (it->first.get_type() == var::STRING) ss << "\"";
		ss << it->first.to_string();
		if (it->first.get_type() == var::STRING) ss << "\"";

		ss << " : ";

		if (it->second.get_type() == var::STRING) ss << "\"";
		ss << it->second.to_string();
		if (it->second.get_type() == var::STRING) ss << "\"";
	}
	ss << " }";
	return ss.str();
}

Map Map::copy(bool p_deep) const {
	Map ret;
	for (_map_internal_t::iterator it = (*_data).begin(); it != (*_data).end(); it++) {
		if (p_deep) {
			ret[it->first] = it->second.copy(true);
		} else {
			ret[it->first] = it->second;
		}
	}
	return ret;
}

bool Map::_Comparator::operator() (const var& l, const var& r) const {
	switch (l.get_type()) {
		case var::_NULL:
			if (r.get_type() == var::_NULL) return false;
			break;
		case var::BOOL:
			if (r.get_type() == var::BOOL) { return (int)l.operator bool() < (int)r.operator bool(); }
			break;
		case var::INT:
			if (r.get_type() == var::INT) { return l.operator int64_t() < r.operator int64_t(); }
			break;
		case var::FLOAT:
			if (r.get_type() == var::FLOAT) { return l.operator double() < r.operator double(); }
			break;
		case var::STRING:
			if (r.get_type() == var::STRING) { return l.operator const String&() < r.operator const String&(); }
			break;

		case var::ARRAY: // [[FALLTHROUGHT]]
		case var::MAP: {
			//	.hash() method will throw unhashable type error
			return l.hash() < r.hash();
		} break;

		case var::OBJECT: {
			// TODO: maybe use __lt() / user defined method
			if (r.get_type() == var::OBJECT) return l.hash() < r.hash();
		} break;
	}
	MISSED_ENUM_CHECK(var::_TYPE_MAX_, 9);

	return (int)l.get_type() < (int)r.get_type();
}

// TODO: error message.
//#define _INSERT_KEY_IF_HAVENT(m_key)                          \
//	_map_internal_t::iterator it = (*_data).find(m_key.hash()); \
//	if (it == _data->end()) (*_data)[m_key.hash()].key = m_key

var Map::operator[](const var& p_key) const  { /*_INSERT_KEY_IF_HAVENT(p_key);      */ return (*_data)[p_key]; }
var& Map::operator[](const var& p_key)       { /*_INSERT_KEY_IF_HAVENT(p_key);      */ return (*_data)[p_key]; }
var Map::operator[](const char* p_key) const { /*_INSERT_KEY_IF_HAVENT(var(p_key)); */ return (*_data)[p_key]; }
var& Map::operator[](const char* p_key)      { /*_INSERT_KEY_IF_HAVENT(var(p_key)); */ return (*_data)[p_key]; }

Map::_map_internal_t::iterator Map::begin() const { return (*_data).begin(); }
Map::_map_internal_t::iterator Map::end() const { return (*_data).end(); }
Map::_map_internal_t::iterator Map::find(const var& p_key) const { return (*_data).find(p_key); }

void Map::insert(const var& p_key, const var& p_value) {
	(*_data)[p_key] = p_value;
	//(*_data).insert(std::pair<size_t, _KeyValue>(p_key.hash(), _KeyValue(p_key, p_value)));
}

bool Map::has(const var& p_key) const { return find(p_key) != end(); }
var Map::at(const var& p_key) const {
	_TRY_VAR_STL(
		return _data->at(p_key));
}
void Map::clear() { _data->clear(); }

size_t Map::size() const { return _data->size(); }
bool Map::empty() const { return _data->empty(); }

bool Map::operator ==(const Map& p_other) const {
	if (size() != p_other.size())
		return false;
	for (_map_internal_t::iterator it_other = p_other.begin(); it_other != p_other.end(); it_other++) {
		_map_internal_t::iterator it_self = find(it_other->first);
		if (it_self == end()) return false;
		if (it_self->second != it_other->second) return false;

	}
	return true;
}

Map::operator bool() const { return empty(); }

Map& Map::operator=(const Map& p_other) {
	_data = p_other._data;
	return *this;
}

} // namespace carbon

#endif //_FILE_SRC_VAR__MAP_CPP_

#ifndef _FILE_SRC_VAR__OBJECT_CPP_


namespace carbon {


ptr<Object> Object::copy(bool p_deep) /*const*/ { THROW_ERROR(Error::NOT_IMPLEMENTED, "Virtual method \"copy()\" not implemented on type \"Object\"."); }

Object::operator String() { return to_string(); }
var Object::operator()(stdvec<var*>& p_args) { return __call(p_args); }

bool Object::operator==(const var& p_other) /*const*/ { return __eq(p_other); }
bool Object::operator!=(const var& p_other) /*const*/ { return !operator == (p_other); }
bool Object::operator<=(const var& p_other) /*const*/ { return __lt(p_other) || __eq(p_other); }
bool Object::operator>=(const var& p_other) /*const*/ { return __gt(p_other) || __eq(p_other); }
bool Object::operator< (const var& p_other) /*const*/ { return __lt(p_other); }
bool Object::operator> (const var& p_other) /*const*/ { return __gt(p_other); }

var Object::operator+(const var& p_other) /*const*/ { return __add(p_other); }
var Object::operator-(const var& p_other) /*const*/ { return __sub(p_other); }
var Object::operator*(const var& p_other) /*const*/ { return __mul(p_other); }
var Object::operator/(const var& p_other) /*const*/ { return __div(p_other); }

var& Object::operator+=(const var& p_other) { return __add_eq(p_other); }
var& Object::operator-=(const var& p_other) { return __sub_eq(p_other); }
var& Object::operator*=(const var& p_other) { return __mul_eq(p_other); }
var& Object::operator/=(const var& p_other) { return __div_eq(p_other); }

#define _OBJ_THROW_NOT_IMPL(m_name)\
	THROW_ERROR(Error::NOT_IMPLEMENTED, String("operator " #m_name " not implemented on base ") + get_type_name() + ".")

var Object::operator[](const var& p_key) const { _OBJ_THROW_NOT_IMPL(operator[]); }
var& Object::operator[](const var& p_key) { _OBJ_THROW_NOT_IMPL(operator[]); }

var Object::__call(stdvec<var*>& p_vars) { _OBJ_THROW_NOT_IMPL(__call()); }
var Object::call_method(const String& p_method_name, stdvec<var*>& p_args) {
	THROW_ERROR(Error::ATTRIBUTE_ERROR, String::format("type %s has no method named \"%s\".", get_type_name(), p_method_name.c_str()));
}
//var Object::operator()(stdvec<var>& p_vars) { return __call(p_vars); }

var  Object::get_member(const String& p_member_name) {
	THROW_ERROR(Error::ATTRIBUTE_ERROR, String::format("type %s has no member named \"%s\"", get_type_name(), p_member_name.c_str()));
}
void Object::set_member(const String& p_member_name, var& p_value) {
	THROW_ERROR(Error::ATTRIBUTE_ERROR, String::format("type %s has no writable member named \"%s\".", get_type_name(), p_member_name.c_str()));
}

var Object::__get_mapped(const var& p_key) /*const*/ { _OBJ_THROW_NOT_IMPL(__get_mapped()); }
void Object::__set_mapped(const var& p_key, const var& p_val) { _OBJ_THROW_NOT_IMPL(__set_mapped()); }
int64_t Object::__hash() /*const*/ { _OBJ_THROW_NOT_IMPL(__hash()); }

var  Object::__iter_begin() { _OBJ_THROW_NOT_IMPL(__iter_begin()); }
bool Object::__iter_has_next() { _OBJ_THROW_NOT_IMPL(__iter_has_next()); }
var  Object::__iter_next() { _OBJ_THROW_NOT_IMPL(__iter_next()); }

var Object::__add(const var& p_other) /*const*/ { _OBJ_THROW_NOT_IMPL(__add()); }
var Object::__sub(const var& p_other) /*const*/ { _OBJ_THROW_NOT_IMPL(__sub()); }
var Object::__mul(const var& p_other) /*const*/ { _OBJ_THROW_NOT_IMPL(__mul()); }
var Object::__div(const var& p_other) /*const*/ { _OBJ_THROW_NOT_IMPL(__div()); }

var& Object::__add_eq(const var& p_other) { _OBJ_THROW_NOT_IMPL(__add_eq()); }
var& Object::__sub_eq(const var& p_other) { _OBJ_THROW_NOT_IMPL(__sub_eq()); }
var& Object::__mul_eq(const var& p_other) { _OBJ_THROW_NOT_IMPL(__mul_eq()); }
var& Object::__div_eq(const var& p_other) { _OBJ_THROW_NOT_IMPL(__div_eq()); }

bool Object::__gt(const var& p_other) /*const*/ { _OBJ_THROW_NOT_IMPL(__gt()); }
bool Object::__lt(const var& p_other) /*const*/ { _OBJ_THROW_NOT_IMPL(__lt()); }
bool Object::__eq(const var& p_other) /*const*/ {
	if (p_other.get_type() != var::OBJECT) return false;
	return this == p_other.operator carbon::ptr<carbon::Object>().get();
}


String Object::to_string() /*const*/ { return String::format("[%s:%i]", get_type_name(), this); }
bool Object::_is_native_ref() const { return false; }
String Object::_get_native_ref() const { return ""; }
bool Object::_is_registered() const { return true; }

void* Object::get_data() { return (void*)this; }

void Object::_bind_data(NativeClasses* p_native_classes) {
	BIND_METHOD("get_type_name", &Object::get_type_name);
	BIND_METHOD("get_base_type_name", &Object::get_base_type_name);
}

// call_method() should call it's parent if method not exists.
var Object::call_method_s(ptr<Object>& p_self, const String& p_name, stdvec<var*>& p_args) {

	if (!p_self->_is_registered()) return p_self->call_method(p_name, p_args);

	String class_name = (p_self->_is_native_ref()) ? p_self->_get_native_ref() : p_self->get_type_name();
	String method_name = p_name;

	ptr<BindData> bind_data = nullptr;
	if (NativeClasses::singleton()->is_class_registered(class_name)) {
		bind_data = NativeClasses::singleton()->find_bind_data(class_name, p_name);
	}

	if (bind_data != nullptr) {
		if (bind_data->get_type() == BindData::METHOD) {
			if (!p_self->_is_native_ref()) return ptrcast<MethodBind>(bind_data)->call(p_self.get(), p_args);
			else THROW_ERROR(Error::ATTRIBUTE_ERROR, String::format("cannot call a non static method named \"%s\" (on type %s) statically.", method_name.c_str(), p_self->get_type_name()));

		} else if (bind_data->get_type() == BindData::STATIC_FUNC) {
			return ptrcast<StaticFuncBind>(bind_data)->call(p_args);

		} else if (bind_data->get_type() == BindData::STATIC_VAR) {
			return ptrcast<StaticPropertyBind>(bind_data)->get().__call(p_args);

		} else if (bind_data->get_type() == BindData::MEMBER_VAR) {
			return ptrcast<PropertyBind>(bind_data)->get(p_self.get()).__call(p_args);

		} else {
			THROW_ERROR(Error::TYPE_ERROR, String::format("attribute named \"%s\" on type %s is not callable.", method_name.c_str(), p_self->get_type_name()));
		}
	} else {
		return p_self->call_method(p_name, p_args);
	}

	THROW_ERROR(Error::ATTRIBUTE_ERROR, String::format("type %s has no method named \"%s\".", p_self->get_type_name(), method_name.c_str()));
}

var Object::get_member_s(ptr<Object>& p_self, const String& p_name) {

	if (!p_self->_is_registered()) return p_self->get_member(p_name);

	String class_name = (p_self->_is_native_ref()) ? p_self->_get_native_ref() : p_self->get_type_name();
	String member_name = p_name;

	ptr<BindData> bind_data = nullptr;
	if (NativeClasses::singleton()->is_class_registered(class_name)) {
		bind_data = NativeClasses::singleton()->find_bind_data(class_name, member_name);
	}

	if (bind_data != nullptr) {
		if (bind_data->get_type() == BindData::MEMBER_VAR) {
			if (!p_self->_is_native_ref()) return ptrcast<PropertyBind>(bind_data)->get(p_self.get());
			else THROW_ERROR(Error::ATTRIBUTE_ERROR, String::format("cannot access a non static attribute named \"%s\" (on type %s) statically.", member_name.c_str(), class_name.c_str()));
		} else if (bind_data->get_type() == BindData::STATIC_VAR) {
			return ptrcast<StaticPropertyBind>(bind_data)->get();
		} else if (bind_data->get_type() == BindData::STATIC_CONST) {
			return ptrcast<ConstantBind>(bind_data)->get();
		} else if (bind_data->get_type() == BindData::ENUM_VALUE) {
			return ptrcast<EnumValueBind>(bind_data)->get();
		} else if (bind_data->get_type() == BindData::ENUM) {
			return ptrcast<EnumBind>(bind_data)->get();
		} else if (bind_data->get_type() == BindData::STATIC_FUNC) {
			return ptrcast<StaticFuncBind>(bind_data)->get_method_info();
		} else {
			// TODO: what if non satic method.
			THROW_ERROR(Error::TYPE_ERROR, String::format("cannot access a non static attribute named \"%s\" on type %s.", member_name.c_str(), class_name.c_str()));
		}
	} else {
		return p_self->get_member(p_name);
	}
	THROW_ERROR(Error::ATTRIBUTE_ERROR, String::format("type %s has no attribute named \"%s\"", p_self->get_type_name(), member_name.c_str()));
}


void Object::set_member_s(ptr<Object>& p_self, const String& p_name, var& p_value) {

	if (!p_self->_is_registered()) { p_self->set_member(p_name, p_value); return; }

	String class_name = (p_self->_is_native_ref()) ? p_self->_get_native_ref() : p_self->get_type_name();
	String member_name = p_name;

	ptr<BindData> bind_data = nullptr;
	if (NativeClasses::singleton()->is_class_registered(class_name)) {
		bind_data = NativeClasses::singleton()->find_bind_data(class_name, member_name);
	}

	if (bind_data != nullptr) {
		if (bind_data->get_type() == BindData::MEMBER_VAR) {
			if (!p_self->_is_native_ref()) {
				ptrcast<PropertyBind>(bind_data)->get(p_self.get()) = p_value;
				return;
			} else {
				THROW_ERROR(Error::ATTRIBUTE_ERROR, String::format("cannot access a non static attribute named \"%s\" (on type %s) statically.", member_name.c_str(), class_name.c_str()));
			}
		} else if (bind_data->get_type() == BindData::STATIC_VAR) {
			ptrcast<StaticPropertyBind>(bind_data)->get() = p_value;
			return;
		} else if (bind_data->get_type() == BindData::STATIC_CONST) {
			THROW_ERROR(Error::TYPE_ERROR, String::format("can't assign a value to constant named \"%s\" on type \"%s\".", member_name.c_str(), class_name.c_str()));
		} else if (bind_data->get_type() == BindData::ENUM_VALUE) {
			THROW_ERROR(Error::TYPE_ERROR, String::format("can't assign a value to enum value named \"%s\" on type \"%s\".", member_name.c_str(), class_name.c_str()));
		} else {
			THROW_ERROR(Error::TYPE_ERROR, String::format("attribute named \"%s\" on type \"%s\" is not a property.", member_name.c_str(), class_name.c_str()));
		}
	} else {
		p_self->set_member(p_name, p_value);
		return;
	}
	//THROW_ERROR(Error::ATTRIBUTE_ERROR, String::format("type %s has no member named \"%s\".", p_self->get_type_name(), member_name.c_str()));
}

#undef _OBJ_THROW_NOT_IMPL

} // namespace carbon

#endif //_FILE_SRC_VAR__OBJECT_CPP_

#ifndef _FILE_SRC_VAR__STRING_CPP_



namespace carbon {

const stdmap<size_t, ptr<MemberInfo>>& TypeInfo::get_member_info_list_string() {
	static stdmap<size_t, ptr<MemberInfo>> member_info_s = {
		_NEW_METHOD_INFO("size",                                                                     var::INT     ),
		_NEW_METHOD_INFO("length",                                                                   var::INT     ),
		_NEW_METHOD_INFO("to_int",                                                                   var::INT     ),
		_NEW_METHOD_INFO("to_float",                                                                 var::FLOAT   ),
		_NEW_METHOD_INFO("hash",                                                                     var::INT     ),
		_NEW_METHOD_INFO("upper",                                                                    var::STRING  ),
		_NEW_METHOD_INFO("lower",                                                                    var::STRING  ),
		_NEW_METHOD_INFO("substr",     _PARAMS("start", "end"),  _TYPES(var::INT, var::INT),         var::STRING  ),
		_NEW_METHOD_INFO("endswith",   _PARAMS("what" ),         _TYPES(var::STRING),                var::BOOL    ),
		_NEW_METHOD_INFO("startswith", _PARAMS("what" ),         _TYPES(var::STRING),                var::BOOL    ),
		_NEW_METHOD_INFO("split",      _PARAMS("delimiter" ),    _TYPES(var::STRING),                var::BOOL,     false, _DEFVALS("")),
		_NEW_METHOD_INFO("strip",                                                                    var::STRING  ),
		_NEW_METHOD_INFO("join",       _PARAMS("strings" ),      _TYPES(var::ARRAY),                 var::STRING  ),
		_NEW_METHOD_INFO("replace",    _PARAMS("with", "what"),  _TYPES(var::STRING, var::STRING),   var::STRING  ),
		_NEW_METHOD_INFO("find",       _PARAMS("what", "offset"),_TYPES(var::STRING, var::INT),      var::INT,     false, _DEFVALS(0)),
		_NEW_METHOD_INFO("contains",   _PARAMS("what"),          _TYPES(var::STRING),                var::BOOL    ),
	};
	return member_info_s;
}

var String::call_method(const String& p_method, const stdvec<var*>& p_args) {
	_check_method_and_args<String>(p_method, p_args, TypeInfo::get_member_info_string(p_method));
	switch (p_method.const_hash()) {
		case "size"_hash:        // [[fallthrough]]
		case "length"_hash:     return (int64_t)size();
		case "to_int"_hash:     return to_int();
		case "to_float"_hash:   return to_float();
		case "hash"_hash:       return (int64_t)hash();
		case "upper"_hash:      return upper();
		case "lower"_hash:      return lower();
		case "substr"_hash:     return substr((size_t)p_args[0]->operator int64_t(), (size_t)p_args[1]->operator int64_t());
		case "endswith"_hash:   return endswith(p_args[0]->operator String());
		case "startswith"_hash: return startswith(p_args[0]->operator String());
		case "strip"_hash:      return strip();
		case "split"_hash: {
			if (p_args.size() == 0) return split();
			else return split(p_args[0]->operator String());
		}
		case "join"_hash:       return join(p_args[0]->operator Array());
		case "replace"_hash:    return replace(p_args[0]->operator const String & (), p_args[1]->operator const String & ());
		case "find"_hash: {
			if (p_args.size() == 1) return find(p_args[0]->operator const String & (), 0);
			else return find(p_args[0]->operator const String & (), p_args[1]->operator int64_t());
		}
	}
	// TODO: more.
	THROW_ERROR(Error::BUG, "can't reach here.");
}

String String::format(const char* p_format, ...) {
	va_list argp;

	va_start(argp, p_format);

	static const unsigned int BUFFER_SIZE = VSNPRINTF_BUFF_SIZE;
	char buffer[BUFFER_SIZE + 1]; // +1 for the terminating character
	int len = vsnprintf(buffer, BUFFER_SIZE, p_format, argp);

	va_end(argp);

	if (len == 0) return String();
	return String(buffer);
}

String::String() : _data(new std::string("")) {}
String::String(const std::string& p_copy) { _data = new std::string(p_copy); }
String::String(const char* p_copy) { _data = new std::string(p_copy); }
String::String(const String& p_copy) { if (p_copy._data) _data = new std::string(*p_copy._data); }
String::String(char p_char) { _data = new std::string(1, p_char); }
String::String(int p_i) { _data = new std::string(std::to_string(p_i).c_str()); }
String::String(int64_t p_i) { _data = new std::string(std::to_string(p_i).c_str()); }
String::String(size_t p_i) { _data = new std::string(std::to_string(p_i).c_str()); }
String::String(double p_d) { _data = new std::string(std::to_string(p_d).c_str()); }
String::~String() { delete _data; }

double String::to_float() const {
	_TRY_VAR_STL(return std::stod(*_data));
}

size_t String::hash() const {
	return std::hash<std::string>{}(*_data);
}

size_t String::const_hash() const {
	return __const_hash(_data->c_str());
}

String String::operator %(const var& p_other) const {
	// TODO: implement a better algorithm (it's for testing purposes and will optimized in the future)
	switch (p_other.get_type()) {
		case var::MAP: {
			THROW_ERROR(Error::OPERATOR_NOT_SUPPORTED,
				String::format("operator \"%%\" not supported on operands String and \"%s\".", p_other.get_type_name().c_str()));
		} break;
		default : {
			String ret;

			const Array* arr = nullptr;
			const var* _var = &p_other;
			if (p_other.get_type() == var::ARRAY) arr = &p_other.operator const carbon::Array& ();

			int arr_i = 0;
			for (int i = 0; i < (int)_data->size(); i++) {
				switch ((*_data)[i]) {
					case '%': {
						if (i == _data->size() - 1)
							THROW_ERROR(Error::VALUE_ERROR, "incomplete formated string");

						switch ((*_data)[i + 1]) {
							case '%': {
								ret += '%';
								i += 1;
							} break;
							case 's': {
								if (arr) {
									ret += (*arr)[arr_i++].to_string();
								} else if(_var) {
									ret += _var->to_string();
									_var = nullptr; // done with _var.
								} else {
									THROW_ERROR(Error::INVALID_ARG_COUNT, "not enough arguments for format string");
								}
								i += 1;
							} break;
							default: {
								THROW_ERROR(Error::VALUE_ERROR, String::format("unsupported formated character %%%c", (*_data)[i + 1]));
							}
						}
					} break; // formated character

					default: {
						ret += (*_data)[i];
					}
				} // switch character
			}
			if (arr) {
				if (arr_i != arr->size()) THROW_ERROR(Error::TYPE_ERROR, "not all arguments converted during string formatting");
			} else if (_var != nullptr) {
				THROW_ERROR(Error::TYPE_ERROR, "not all arguments converted during string formatting");
			}
			return ret;
		} break;
	}
}

bool String::operator==(const String& p_other) const { return *_data == *p_other._data; }
bool String::operator!=(const String& p_other) const { return *_data != *p_other._data; }
bool String::operator<(const String& p_other) const { return *_data < *p_other._data; }
bool String::operator<=(const String& p_other) const { return *_data <= *p_other._data; }
bool String::operator>(const String& p_other) const { return *_data > * p_other._data; }
bool String::operator>=(const String& p_other) const { return *_data >= *p_other._data; }

String String::operator+(char p_c) const              { return *_data + p_c; }
//String String::operator+(int p_i) const             { return *_data + std::to_string(p_i); }
//String String::operator+(double p_d) const          { return *_data + std::to_string(p_d); }
String String::operator+(const char* p_cstr) const    { return *_data + p_cstr; }
String String::operator+(const String& p_other) const { return *_data + *p_other._data; }

String& String::operator+=(char p_c)              { *_data += p_c;                 return *this; }
//String& String::operator+=(int p_i)             { *_data += std::to_string(p_i); return *this; }
//String& String::operator+=(double p_d)          { *_data += std::to_string(p_d); return *this; }
String& String::operator+=(const char* p_cstr)    { *_data += p_cstr;              return *this; }
String& String::operator+=(const String& p_other) { *_data += *p_other._data;      return *this; }

String& String::operator=(char p_c)              { *_data = p_c;                 return *this; }
//String& String::operator=(int p_i)             { *_data = std::to_string(p_i); return *this; }
//String& String::operator=(double p_d)          { *_data = std::to_string(p_d); return *this; }
String& String::operator=(const char* p_cstr)    { *_data = p_cstr;              return *this; }
String& String::operator=(const String& p_other) { *_data = *p_other._data;      return *this; }


bool operator==(const char* p_cstr, const String& p_str) {
	return p_str == String(p_cstr);
}
bool operator!=(const char* p_cstr, const String& p_str) {
	return p_str != String(p_cstr);
}

char String::operator[](int64_t p_index) const {
	if (0 <= p_index && p_index < (int64_t)size())
		return (*_data)[(size_t)p_index];
	if ((int64_t)size() * -1 <= p_index && p_index < 0)
		return (*_data)[size() + (size_t)p_index];
	THROW_ERROR(Error::INVALID_INDEX, String::format("String index %i is invalid.", p_index));
}
char& String::operator[](int64_t p_index) {
	if (0 <= p_index && p_index < (int64_t)size())
		return (*_data)[(size_t)p_index];
	if ((int64_t)size() * -1 <= p_index && p_index < 0)
		return (*_data)[size() + (size_t)p_index];
	THROW_ERROR(Error::INVALID_INDEX, String::format("String index %i is invalid.", p_index));
}


size_t String::size() const { return _data->size(); }
const char* String::c_str() const { return _data->c_str(); }
void* String::get_data() { return (void*)_data->data(); }
String& String::append(const String& p_other) { _data->append(p_other.operator std::string()); return *this; }

String String::upper() const {
	String ret = *this;
	for (size_t i = 0; i < ret.size(); i++) {
		char& c = (*ret._data)[i];
		if ('a' <= c && c <= 'z') {
			c += ('A' - 'a');
		}
	}
	return ret;
}

String String::lower() const {
	String ret = *this;
	for (size_t i = 0; i < ret.size(); i++) {
		char& c = (*ret._data)[i];
		if ('A' <= c && c <= 'Z') {
			c += ('a' - 'A');
		}
	}
	return ret;
}

int64_t String::to_int() const {
	// TODO: this should throw std::exceptions
	if (startswith("0x") || startswith("0X")) {
		_TRY_VAR_STL(
			return std::stoll(*_data, nullptr, 16));
	} else if (startswith("0b") || startswith("0B")) {
		_TRY_VAR_STL(
			return std::stoll(substr(2, size()), nullptr, 2));
	} else {
		_TRY_VAR_STL(
			return std::stoll(*_data));
	}
}

String String::substr(int64_t p_start, int64_t p_end) const {
	// TODO: inconsistance withs arr[-1]
	return _data->substr((size_t)p_start, (size_t)(p_end - p_start));
}
bool String::endswith(const String& p_str) const {
	if (p_str.size() > _data->size()) return false;
	for (size_t i = 1; i <= p_str.size(); i++) {
		if ((*_data)[_data->size() - i] != p_str[p_str.size() - i]) {
			return false;
		}
	}
	return true;
}
bool String::startswith(const String& p_str) const {
	if (p_str.size() > _data->size()) return false;
	for (size_t i = 0; i < p_str.size(); i++) {
		if ((*_data)[i] != p_str[i]) {
			return false;
		}
	}
	return true;
}

Array String::split(const String& p_delimiter) const {
	Array ret;
	if (size() == 0) return ret;

	size_t start = 0, end = 0;
	if (p_delimiter == "") { // split by white space
		while (true) {
			// skip all white spaces
			while (isspace((*_data)[start])) {
				start++;
				if (start >= size()) return ret;
			}
			end = start;
			while (end < size() && !isspace((*_data)[end])) {
				end++;
			}

			ret.append(substr(start, end));
			start = end;
			if (start >= size()) return ret;
		}
	} else {
		while (true) {
			end = _data->find(p_delimiter.operator std::string(), start);
			if (end == std::string::npos) {
				ret.append(substr(start, size()));
				break;
			} else {
				ret.append(substr(start, end));
				start = end + p_delimiter.size();
			}
		}
		return ret;
	}

	return ret;
}

String String::strip() const {
	if (size() == 0) return "";
	size_t start = 0, end = size() - 1;
	
#define IS_WHITE_SPACE(c) ( (c) == ' ' || (c) == '\t' || (c) == '\n' )

	while (IS_WHITE_SPACE((*_data)[start])) {
		start++;
		if (start >= size()) return "";
	}
	while (IS_WHITE_SPACE((*_data)[end])) {
		end--;
		if (end < 0) return "";
	}
	return _data->substr(start, end - start + 1);
	
#undef IS_WHITE_SPACE
}

String String::join(const Array& p_elements) const {
	String ret;
	for (int i = 0; i < (int)p_elements.size(); i++) {
		if (i > 0) ret += *this;
		ret += p_elements[i].operator String();
	}
	return ret;
}

String String::replace(const String& p_with, const String& p_what) const {
	std::string ret = *_data;
	std::string::size_type n = 0;
	while ((n = ret.find(p_with.operator const std::string &(), n)) != std::string::npos) {
		ret.replace(n, p_with.size(), p_what.operator const std::string &());
		n += p_what.size();
	}
	return ret;
}

int64_t String::find(const String& p_what, int64_t p_offset) const {
	 std::size_t pos = _data->find(p_what.operator const std::string & (), (size_t)p_offset);
	 if (pos == std::string::npos) return -1;
	 return (int64_t)pos;
}

bool String::contains(const String& p_what) const {
	return find(p_what) != -1;
}

} // namespace carbon

#endif //_FILE_SRC_VAR__STRING_CPP_

#ifndef _FILE_SRC_NATIVE_BUFFER_CPP_



namespace carbon {

Buffer::Buffer(size_t p_size) { alloc(p_size); }
void Buffer::_Buffer(ptr<Buffer> self, int64_t p_size) { self->alloc(p_size); }

void Buffer::alloc(size_t p_size) {
	_buffer = std::shared_ptr<byte_t>(new byte_t[p_size], [](byte_t* bytes) { delete[] bytes; });
	_size = p_size;
}

byte_t* Buffer::front() {
	return _buffer.get();
}

void* Buffer::get_data() {
	return (void*)front();
}

size_t  Buffer::size() const { return _size; }

byte_t& Buffer::operator[](size_t p_index) {
	THROW_INVALID_INDEX(_size, p_index);
	return _buffer.get()[p_index];
}
const byte_t& Buffer::operator[](size_t p_index) const {
	THROW_INVALID_INDEX(_size, p_index);
	return _buffer.get()[p_index];
}

var Buffer::__get_mapped(const var& p_key) /*const*/ {
	if (p_key.get_type() != var::INT) THROW_ERROR(Error::TYPE_ERROR, "expected an integer as key.");
	int64_t key = p_key.operator int64_t();
	THROW_INVALID_INDEX(_size, key);
	return (int64_t)_buffer.get()[key];
}

void Buffer::__set_mapped(const var& p_key, const var& p_value) {
	if (p_key.get_type() != var::INT) THROW_ERROR(Error::TYPE_ERROR, "expected an integer as key.");
	if (p_value.get_type() != var::INT) THROW_ERROR(Error::TYPE_ERROR, "expected an integer as value.");
	int64_t key = p_key.operator int64_t();
	int64_t value = p_value.operator int64_t();
	if (value < 0 || 0xff < value) THROW_ERROR(Error::TYPE_ERROR, String::format("integer value %lli is invalid for a buffer byte of range 0 - 0xff", value));
	THROW_INVALID_INDEX(_size, key);
	_buffer.get()[key] = (byte_t)value;
}

}

#endif //_FILE_SRC_NATIVE_BUFFER_CPP_

#ifndef _FILE_SRC_NATIVE_FILE_CPP_



namespace carbon {

void File::close() {
	if (is_open()) {
		fclose(_file);
		_file = NULL;
	} else {
		// TODO: Warn here ??
	}
}

void File::open(const String& p_path, int p_mode) {
	if (p_mode < READ || (p_mode > (READ | WRITE | APPEND | BINARY | EXTRA))) THROW_ERROR(Error::IO_ERROR, "invalid mode flag set in file.");

	// TODO: fix the error message, print given combination.
	if ((p_mode & READ) && (p_mode & WRITE) && (p_mode & APPEND)) THROW_ERROR(Error::IO_ERROR, "invalid combination of flags (WRITE & APPEND) in file.");
	if (!(p_mode & READ) && !(p_mode & WRITE) && !(p_mode & APPEND)) THROW_ERROR(Error::IO_ERROR, "invalid combination of flags (WRITE & APPEND) in file.");

	path = p_path;
	mode = p_mode;

	String _strmode;
	bool binary = (p_mode & BINARY);
	bool extra = (p_mode & EXTRA);
	if (p_mode & READ) {
		if (binary && extra) _strmode = "rb+";
		else if (extra)      _strmode = "r+";
		else if (binary)     _strmode = "rb";
		else                 _strmode = "r";
		
	} else if (p_mode & WRITE) {
		if (binary && extra) _strmode = "wb+";
		else if (extra)      _strmode = "w+";
		else if (binary)     _strmode = "wb";
		else                 _strmode = "w";

	} else { // APPEND
		if (binary && extra) _strmode = "ab+";
		else if (extra)      _strmode = "a+";
		else if (binary)     _strmode = "ab";
		else                 _strmode = "a";
	}
	_file = fopen(path.c_str(), _strmode.c_str());
	
	if (_file == NULL) {
		THROW_ERROR(Error::IO_ERROR, String::format("can't open file at \"%s\".", path.c_str()));
	}
}

long File::size() {
	fseek(_file, 0, SEEK_END);
	long _size = ftell(_file);
	fseek(_file, 0, SEEK_SET);
	return _size;
}

String File::read_text() {
	if (!is_open()) THROW_ERROR(Error::IO_ERROR, "can't read on a closed file.");
	if (!(mode & READ) && !(mode & EXTRA)) THROW_ERROR(Error::IO_ERROR, "opened file mode isn't supported for reading.");
	if (mode & BINARY) THROW_ERROR(Error::IO_ERROR, "opened file mode is binary not supported for text.");
	long _file_size = size();
	if (_file_size == 0) return String();
	
	char* buff = new char[_file_size + 1];
	size_t read = fread(buff, sizeof(char), _file_size, _file);
	buff[read] = '\0';
	String text = buff;
	delete[] buff;
	
	return text;
}

String File::read_line() {
	String line;
	while (true) {
		char c = getc(_file);
		if (c == EOF) return line;
		else if (c == '\n') return line + c;
		line += c;
	}
}

void File::write_text(const String& p_text) {
	if (!is_open()) THROW_ERROR(Error::IO_ERROR, "can't write on a closed file.");
	if (!(mode & WRITE) && !(mode & APPEND) && !(mode & EXTRA)) THROW_ERROR(Error::IO_ERROR, "opened file mode isn't supported for writing.");
	fprintf(_file, "%s", p_text.c_str());
}

ptr<Buffer> File::read_bytes() {
	if (!is_open()) THROW_ERROR(Error::IO_ERROR, "can't read on a closed file.");
	if (!(mode & READ) && !(mode & EXTRA)) THROW_ERROR(Error::IO_ERROR, "opened file mode isn't supported for reading.");
	if (!(mode & BINARY)) THROW_ERROR(Error::IO_ERROR, "opened file mode is text not supported for binary.");
	long file_size = size();
	ptr<Buffer> buff = newptr<Buffer>(file_size);
	fseek(_file, 0, SEEK_SET);
	fread(buff->front(), sizeof(byte_t), file_size, _file);
	return buff;
}

void File::write_bytes(const ptr<Buffer>& p_bytes) {
	if (!is_open()) THROW_ERROR(Error::IO_ERROR, "can't write on a closed file.");
	if (!(mode & WRITE) && !(mode & APPEND) && !(mode & EXTRA)) THROW_ERROR(Error::IO_ERROR, "opened file mode isn't supported for writing.");
	fwrite(p_bytes->front(), sizeof(byte_t), p_bytes->size(), _file);
}

var File::read() {
	if (mode & BINARY) {
		return read_bytes();
	} else {
		return read_text();
	}
}

void File::write(const var& p_what) {
	if (mode & BINARY) {
		if (p_what.get_type() != var::OBJECT || p_what.get_type_name() != Buffer::get_type_name_s()) {
			THROW_ERROR(Error::TYPE_ERROR, String::format("expected type %s at argument 0.", Buffer::get_type_name_s()));
		}
		return write_bytes(ptrcast<Buffer>(p_what.operator ptr<Object>()));
	} else {
		if (p_what.get_type() != var::STRING) {
			THROW_ERROR(Error::TYPE_ERROR, String::format("expected type %s at argument 0.", var::get_type_name_s(var::STRING)));
		}
		return write_text(p_what.to_string());
	}
}

File::File(const String& p_path, int p_mode) {
	path = p_path;
	if (path.size() != 0) open(path, p_mode);
}

File::~File() {
	close();
}

void File::_File(ptr<File> p_self, const String& p_path, int p_mode) {
	p_self->path = p_path;
	if (p_self->path.size() != 0) p_self->open(p_self->path, p_mode);
}

// file iterator ------------------------------------------

class _Iterator_File : public Object {
	REGISTER_CLASS(_Iterator_File, Object) {}

	File* _file = nullptr;
	String _line;
public:
	_Iterator_File() {}
	_Iterator_File(File* p_file) {
		_file = p_file;
		_line = _file->read_line();
	}

	bool _is_registered() const override { return false; }
	virtual bool __iter_has_next() override { return _line != ""; }
	virtual var __iter_next() override { return _line = _file->read_line(); }

};

var File::__iter_begin() {
	return newptr<_Iterator_File>(this);
}

}


#endif //_FILE_SRC_NATIVE_FILE_CPP_

#ifndef _FILE_SRC_NATIVE_OS_CPP_


namespace carbon {

int64_t OS::unix_time() {
	using namespace std::chrono;
	return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}

int OS::syscall(const String& p_cmd) {
	return system(p_cmd.c_str());
}

String OS::getcwd() {
	return _Platform::os_getcwd();
}

void OS::chdir(const String& p_path) {
	_Platform::os_chdir(p_path.c_str());
}

}

#endif //_FILE_SRC_NATIVE_OS_CPP_

#ifndef _FILE_SRC_NATIVE_PATH_CPP_



namespace carbon {

// constructors
Path::Path(const String& p_path) :_path(p_path) {}
void Path::_Path(ptr<Path> p_self, const String& p_path) {
	p_self->_path = p_path;
}

String Path::absolute() {
	return _Platform::path_absolute(_path);
}

bool Path::exists() {
	return _Platform::path_exists(_path);
}

bool Path::isdir() {
	return _Platform::path_isdir(_path);
}

Array Path::listdir() {
	Array ret;
	stdvec<std::string> dirs = _Platform::path_listdir(_path);
	for (const std::string& dir : dirs) {
		if (dir == "." || dir == "..") continue;
		ret.push_back(newptr<Path>(dir));
	}
	return ret;
}

String Path::parent() {
	const std::string& str = _path;
	size_t found = str.find_last_of("/\\");
	if (found == std::string::npos) return "";
	return str.substr(0, found);
}

String Path::filename() {
	const std::string& str = _path;
	size_t found = str.find_last_of("/\\");
	if (found == std::string::npos) return "";
	return str.substr(found + 1);
}

String Path::extension() {
	const std::string& str = _path;
	size_t found = str.rfind('.');
	if (found == std::string::npos) return "";
	return str.substr(found);
}

ptr<Path> Path::join(const String& p_path) const {
	// TODO: this is temp and try corss platform libraries (boost/filesystem?)
	if (p_path.size() == 0) return newptr<Path>(_path);
	if (_path.size() == 0 || p_path[0] == '\\' || p_path[0] == '/') newptr<Path>(p_path);
	if (_path[0] == '\\' || p_path[0] == '/') {
		return newptr<Path>(_path + p_path);
	}
	return newptr<Path>(_path + '/' + p_path);
}

ptr<Path> Path::operator /(const Path& p_other) const {
	return join(p_other);
}

var Path::__div(const var& p_other) {
	if (p_other.get_type() == var::STRING) {
		return join(p_other.operator String());
	}
	if (p_other.get_type_name() != get_type_name_s()) {
		THROW_ERROR(Error::TYPE_ERROR, String::format("can't cast \"%s\" to \"Object\".", p_other.get_type_name().c_str()));
	}
	return join(p_other.operator ptr<Path>()->operator const String &());
}

String Path::to_string() {
	return _path;
}

Path::operator const String& () const {
	return _path;
}

}

#endif //_FILE_SRC_NATIVE_PATH_CPP_

#ifndef _FILE_SRC_COMPILER_ANALYZER_CPP_


/******************************************************************************************************************/
/*                                         ANALYZER                                                               */
/******************************************************************************************************************/

namespace carbon {

CompileTimeError Analyzer::_analyzer_error(Error::Type p_type, const String& p_msg, Vect2i p_pos, const DBGSourceInfo& p_dbg_info) const {
	uint32_t err_len = 1;
	String token_str = "";
	if (p_pos.x > 0 && p_pos.y > 0) token_str = parser->tokenizer->get_token_at(p_pos).to_string();
	else token_str = parser->tokenizer->peek(-1, true).to_string();
	if (token_str.size() > 1 && token_str[0] == '<' && token_str[token_str.size() - 1] == '>') err_len = 1;
	else err_len = (uint32_t)token_str.size();

	Vect2i pos = (p_pos.x > 0 && p_pos.y > 0) ? p_pos : parser->tokenizer->peek(-1, true).get_pos();
	return CompileTimeError(p_type, p_msg, DBGSourceInfo(file_node->path, file_node->source,
		std::pair<int, int>((int)pos.x, (int)pos.y), err_len), p_dbg_info);
}

void Analyzer::_analyzer_warning(Warning::Type p_type, const String& p_msg, Vect2i p_pos, const DBGSourceInfo& p_dbg_info) {
	uint32_t err_len = 1;
	String token_str = "";
	if (p_pos.x > 0 && p_pos.y > 0) token_str = parser->tokenizer->get_token_at(p_pos).to_string();
	else token_str = parser->tokenizer->peek(-1, true).to_string();
	if (token_str.size() > 1 && token_str[0] == '<' && token_str[token_str.size() - 1] == '>') err_len = 1;
	else err_len = (uint32_t)token_str.size();

	Vect2i pos = (p_pos.x > 0 && p_pos.y > 0) ? p_pos : parser->tokenizer->peek(-1, true).get_pos();
	Warning warning(p_type, p_msg,
		DBGSourceInfo(file_node->path, file_node->source,
			std::pair<int, int>((int)pos.x, (int)pos.y), err_len), p_dbg_info);
	
	warnings.push_back(warning);
}

const stdvec<Warning>& Analyzer::get_warnings() const {
	return warnings;
}

void Analyzer::analyze(ptr<Parser> p_parser) {

	parser = p_parser;
	file_node = parser->file_node;

	parser->parser_context.current_class = nullptr;
	parser->parser_context.current_func = nullptr;
	parser->parser_context.current_block = nullptr;
	parser->parser_context.current_enum = nullptr;

	for (int i = 0; i < (int)file_node->classes.size(); i++) {
		_resolve_inheritance(file_node->classes[i].get());
	}

	// File/class level constants.
	for (size_t i = 0; i < file_node->constants.size(); i++) {
		parser->parser_context.current_const = file_node->constants[i].get();
		_resolve_constant(file_node->constants[i].get());
	}
	for (size_t i = 0; i < file_node->classes.size(); i++) {
		parser->parser_context.current_class = file_node->classes[i].get();
		for (size_t j = 0; j < file_node->classes[i]->constants.size(); j++) {
				parser->parser_context.current_const = file_node->classes[i]->constants[j].get();
				_resolve_constant(file_node->classes[i]->constants[j].get());
		}
	}
	parser->parser_context.current_const = nullptr;
	parser->parser_context.current_class = nullptr;

	// File/class enums/unnamed_enums.
	for (size_t i = 0; i < file_node->enums.size(); i++) {
		parser->parser_context.current_enum = file_node->enums[i].get();
		int _possible_value = 0;
		for (std::pair<String, Parser::EnumValueNode> pair : file_node->enums[i]->values) {
			_resolve_enumvalue(file_node->enums[i]->values[pair.first], &_possible_value);
		}
	}
	if (file_node->unnamed_enum != nullptr) {
		parser->parser_context.current_enum = file_node->unnamed_enum.get();
		int _possible_value = 0;
		for (std::pair<String, Parser::EnumValueNode> pair : file_node->unnamed_enum->values) {
			_resolve_enumvalue(file_node->unnamed_enum->values[pair.first], &_possible_value);
		}
	}
	parser->parser_context.current_enum = nullptr;

	for (size_t i = 0; i < file_node->classes.size(); i++) {
		parser->parser_context.current_class = file_node->classes[i].get();
		for (size_t j = 0; j < file_node->classes[i]->enums.size(); j++) {
			parser->parser_context.current_enum = file_node->classes[i]->enums[j].get();
			int _possible_value = 0;
			for (std::pair<String, Parser::EnumValueNode> pair : file_node->classes[i]->enums[j]->values) {
				_resolve_enumvalue(file_node->classes[i]->enums[j]->values[pair.first], &_possible_value);
			}
		}
		if (file_node->classes[i]->unnamed_enum != nullptr) {
			parser->parser_context.current_enum = file_node->classes[i]->unnamed_enum.get();
			int _possible_value = 0;
			for (std::pair<String, Parser::EnumValueNode> pair : file_node->classes[i]->unnamed_enum->values) {
				_resolve_enumvalue(file_node->classes[i]->unnamed_enum->values[pair.first], &_possible_value);
			}
		}
	}
	parser->parser_context.current_class = nullptr;
	parser->parser_context.current_enum = nullptr;

	// call compile time functions.
	for (auto& func : file_node->compiletime_functions) _resolve_compiletime_funcs(func);
	for (size_t i = 0; i < file_node->classes.size(); i++) {
		parser->parser_context.current_class = file_node->classes[i].get();
		for (auto& func : file_node->classes[i]->compiletime_functions)
			_resolve_compiletime_funcs(func);
	}
	parser->parser_context.current_class = nullptr;

	// File/class level variables.
	for (size_t i = 0; i < file_node->vars.size(); i++) {
		if (file_node->vars[i]->assignment != nullptr) {
			parser->parser_context.current_var = file_node->vars[i].get();
			_reduce_expression(file_node->vars[i]->assignment);
		}
	}
	for (size_t i = 0; i < file_node->classes.size(); i++) {
		parser->parser_context.current_class = file_node->classes[i].get();
		for (size_t j = 0; j < file_node->classes[i]->vars.size(); j++) {
			if (file_node->classes[i]->vars[j]->assignment != nullptr) {
				parser->parser_context.current_var = file_node->classes[i]->vars[j].get();
				_reduce_expression(file_node->classes[i]->vars[j]->assignment);
			}
		}
	}
	parser->parser_context.current_var = nullptr;
	parser->parser_context.current_class = nullptr;

	// resolve parameters.
	for (size_t i = 0; i < file_node->functions.size(); i++) {
		Parser::FunctionNode* fn = file_node->functions[i].get();
		_resolve_parameters(fn);
	}

	for (size_t i = 0; i < file_node->classes.size(); i++) {
		parser->parser_context.current_class = file_node->classes[i].get();
		for (size_t j = 0; j < file_node->classes[i]->functions.size(); j++) {
			Parser::FunctionNode* fn = file_node->classes[i]->functions[j].get();
			_resolve_parameters(fn);
		}
		parser->parser_context.current_class = nullptr;
	}

	// file level function.
	for (size_t i = 0; i < file_node->functions.size(); i++) {
		parser->parser_context.current_func = file_node->functions[i].get();
		Parser::FunctionNode* fn = file_node->functions[i].get();

		if (fn->name == GlobalStrings::main) {
			if (fn->args.size() >= 2) throw ANALYZER_ERROR(Error::INVALID_ARG_COUNT, "main function takes at most 1 argument.", fn->pos);
		}

		_reduce_block(file_node->functions[i]->body);
	}
	parser->parser_context.current_func = nullptr;

	// class function.
	for (size_t i = 0; i < file_node->classes.size(); i++) {

		parser->parser_context.current_class = file_node->classes[i].get();
		for (size_t j = 0; j < file_node->classes[i]->functions.size(); j++) {
			// check magic methods arguments
			_check_operator_methods(file_node->classes[i]->functions[j].get());

			parser->parser_context.current_func = file_node->classes[i]->functions[j].get();
			_reduce_block(file_node->classes[i]->functions[j]->body);
		}

		// add default constructor
		Parser::ClassNode* cls = file_node->classes[i].get();
		if (!cls->has_super_ctor_call && cls->base_type != Parser::ClassNode::NO_BASE) {

			bool can_add_default_ctor = true;
			switch (cls->base_type) {
				case Parser::ClassNode::BASE_LOCAL: {
					const Parser::FunctionNode* ctor = cls->base_class->constructor;
					if (ctor && ctor->args.size() - ctor->default_args.size() != 0) can_add_default_ctor = false;
				} break;
				case Parser::ClassNode::BASE_NATIVE: {
					const StaticFuncBind* ctor = NativeClasses::singleton()->get_constructor(cls->base_class_name);
					if (ctor && ctor->get_method_info()->get_arg_count()-1/*self*/ - ctor->get_method_info()->get_default_arg_count() != 0)
						can_add_default_ctor = false;
				} break;
				case Parser::ClassNode::BASE_EXTERN: {
					const Function* ctor = cls->base_binary->get_constructor();
					if (ctor && ctor->get_arg_count() - ctor->get_default_args().size() != 0) can_add_default_ctor = false;
				} break;
			}
			if (!can_add_default_ctor)
				throw ANALYZER_ERROR(Error::TYPE_ERROR, "super constructor call needed since base class doesn't have a default constructor.", cls->pos);

			Parser::FunctionNode* fn = cls->constructor;
			if (fn == nullptr) {
				ptr<Parser::FunctionNode> new_fn = newptr<Parser::FunctionNode>();
				new_fn->name = cls->name;
				new_fn->is_reduced = true;
				new_fn->parent_node = cls;
				new_fn->pos = cls->pos;
				new_fn->body = newptr<Parser::BlockNode>();
				cls->functions.push_back(new_fn);
				cls->constructor = new_fn.get();
				fn = new_fn.get();
			}
			ptr<Parser::CallNode> super_call = newptr<Parser::CallNode>(); super_call->pos = cls->pos;
			super_call->base = newptr<Parser::SuperNode>(); super_call->base->pos = cls->pos;
			fn->body->statements.insert(fn->body->statements.begin(), super_call);
		}
	}
	parser->parser_context.current_class = nullptr;
	parser->parser_context.current_func = nullptr;
}

var Analyzer::_call_compiletime_func(Parser::BuiltinFunctionNode* p_func, stdvec<var*>& args) {
	switch (p_func->func) {
		case BuiltinFunctions::__ASSERT: {
			if (args.size() != 1) throw ANALYZER_ERROR(Error::INVALID_ARG_COUNT, "expected exactly 1 argument.", p_func->pos);
			if (!args[0]->operator bool()) throw ANALYZER_ERROR(Error::ASSERTION, "assertion failed.", p_func->pos);
		} break;
		case BuiltinFunctions::__FUNC: {
			if (!parser->parser_context.current_func) throw ANALYZER_ERROR(Error::SYNTAX_ERROR, "__func() must be called inside a function.", p_func->pos);
			if (parser->parser_context.current_class) return parser->parser_context.current_class->name + "." + parser->parser_context.current_func->name;
			else  return parser->parser_context.current_func->name;
		} break;
		case BuiltinFunctions::__LINE: {
			return p_func->pos.x;
		} break;
		case BuiltinFunctions::__FILE: {
			return parser->file_node->path;
		} break;
		default:
			ASSERT(false);
	}
	return var();
}

void Analyzer::_resolve_compiletime_funcs(const ptr<Parser::CallNode>& p_func) {
	Parser::CallNode* call = p_func.get();
	ASSERT(call->is_compilttime);
	ASSERT(call->base->type == Parser::Node::Type::BUILTIN_FUNCTION);
	Parser::BuiltinFunctionNode* bf = ptrcast<Parser::BuiltinFunctionNode>(call->base).get();
	stdvec<var*> args;
	for (int j = 0; j < (int)call->args.size(); j++) {
		_reduce_expression(call->args[j]);
		if (call->args[j]->type != Parser::Node::Type::CONST_VALUE) {
			throw ANALYZER_ERROR(Error::TYPE_ERROR, String::format("compiletime function arguments must be compile time known values."), p_func->args[j]->pos);
		}
		args.push_back(&ptrcast<Parser::ConstValueNode>(call->args[j])->value);
	}
	_call_compiletime_func(bf, args);
}

void Analyzer::_check_member_var_shadow(void* p_base, Parser::ClassNode::BaseType p_base_type, stdvec<ptr<Parser::VarNode>>& p_vars) {
	switch (p_base_type) {
		case Parser::ClassNode::NO_BASE: // can't be
			return;
		case Parser::ClassNode::BASE_NATIVE: {
			String* base = (String*)p_base;
			for (const ptr<Parser::VarNode>& v : p_vars) {
				ptr<MemberInfo> mi = NativeClasses::singleton()->get_member_info(*base, v->name);
				if (mi == nullptr) continue;
				if (mi->get_type() == MemberInfo::PROPERTY) {
					const PropertyInfo* pi = static_cast<const PropertyInfo*>(mi.get());
					if (!pi->is_static()) throw ANALYZER_ERROR(Error::ATTRIBUTE_ERROR,
						String::format("member named \"%s\" already exists in base \"%s\"", v->name.c_str(), base->c_str()), v->pos);
				}
			}
			String parent = NativeClasses::singleton()->get_inheritance(*base);
			if (parent != "") _check_member_var_shadow((void*)&parent, Parser::ClassNode::BASE_NATIVE, p_vars);
		} break;
		case Parser::ClassNode::BASE_EXTERN: {
			Bytecode* base = (Bytecode*)p_base;
			for (const ptr<Parser::VarNode>& v : p_vars) {
				const ptr<MemberInfo> mi = base->get_member_info(v->name);
				if (mi == nullptr) continue;
				if (mi->get_type() == MemberInfo::PROPERTY) {
					const PropertyInfo* pi = static_cast<const PropertyInfo*>(mi.get());
					if (!pi->is_static()) throw ANALYZER_ERROR(Error::ATTRIBUTE_ERROR,
						String::format("member named \"%s\" already exists in base \"%s\"", v->name.c_str(), base->get_name().c_str()), v->pos);
				}
			}
			if (base->has_base()) {
				if (base->is_base_native()) _check_member_var_shadow((void*)&base->get_base_native(), Parser::ClassNode::BASE_NATIVE, p_vars);
				else _check_member_var_shadow(base->get_base_binary().get(), Parser::ClassNode::BASE_EXTERN, p_vars);
			}
		} break;
		case Parser::ClassNode::BASE_LOCAL: {
			Parser::ClassNode* base = (Parser::ClassNode*)p_base;
			for (const ptr<Parser::VarNode>& v : p_vars) {
				if (v->is_static) continue;
				for (const ptr<Parser::VarNode>& _v : base->vars) {
					if (_v->is_static) continue;
					if (_v->name == v->name) throw ANALYZER_ERROR(Error::ATTRIBUTE_ERROR,
						String::format("member named \"%s\" already exists in base \"%s\"", v->name.c_str(), base->name.c_str()), v->pos);
				}
			}
			if (base->base_type == Parser::ClassNode::BASE_LOCAL) _check_member_var_shadow((void*)base->base_class, Parser::ClassNode::BASE_LOCAL, p_vars);
			else if (base->base_type == Parser::ClassNode::BASE_EXTERN) _check_member_var_shadow((void*)base->base_binary.get(), Parser::ClassNode::BASE_EXTERN, p_vars);
			else if (base->base_type == Parser::ClassNode::BASE_NATIVE) _check_member_var_shadow((void*)&base->base_class_name, Parser::ClassNode::BASE_NATIVE, p_vars);
		} break;

	}
}

void Analyzer::_resolve_inheritance(Parser::ClassNode* p_class) {

	if (p_class->is_reduced) return;
	if (p_class->_is_reducing) throw ANALYZER_ERROR(Error::TYPE_ERROR, "cyclic inheritance. class inherits itself isn't allowed.", p_class->pos);
	p_class->_is_reducing = true;

	// resolve inheritance.
	if (p_class->base_type == Parser::ClassNode::BASE_LOCAL) {
		bool found = false;
		for (int i = 0; i < (int)file_node->classes.size(); i++) {
			if (p_class->base_class_name == file_node->classes[i]->name) {
				found = true;
				_resolve_inheritance(file_node->classes[i].get());
				p_class->base_class = file_node->classes[i].get();
			}
		}
		if (!found) throw ANALYZER_ERROR(Error::TYPE_ERROR, String::format("base class \"%s\" not found.", p_class->base_class_name.c_str()), p_class->pos);
	}

	// check if a member is already exists in the parent class.
	if (p_class->base_type == Parser::ClassNode::BASE_LOCAL) _check_member_var_shadow((void*)p_class->base_class, Parser::ClassNode::BASE_LOCAL, p_class->vars);
	else if (p_class->base_type == Parser::ClassNode::BASE_EXTERN) _check_member_var_shadow((void*)p_class->base_binary.get(), Parser::ClassNode::BASE_EXTERN, p_class->vars);
	else if (p_class->base_type == Parser::ClassNode::BASE_NATIVE) _check_member_var_shadow((void*)&p_class->base_class_name, Parser::ClassNode::BASE_NATIVE, p_class->vars);

	p_class->_is_reducing = false;
	p_class->is_reduced = true;
}

void Analyzer::_resolve_constant(Parser::ConstNode* p_const) {
	if (p_const->is_reduced) return;
	if (p_const->_is_reducing) throw ANALYZER_ERROR(Error::TYPE_ERROR, "cyclic constant value dependancy found.", p_const->pos);
	p_const->_is_reducing = true;

	ASSERT(p_const->assignment != nullptr);
	_reduce_expression(p_const->assignment);

	if (p_const->assignment->type == Parser::Node::Type::CONST_VALUE) {
		ptr<Parser::ConstValueNode> cv = ptrcast<Parser::ConstValueNode>(p_const->assignment);
		if (cv->value.get_type() != var::INT && cv->value.get_type() != var::FLOAT &&
			cv->value.get_type() != var::BOOL && cv->value.get_type() != var::STRING &&
			cv->value.get_type() != var::_NULL) {
			throw ANALYZER_ERROR(Error::TYPE_ERROR, "expected a constant expression.", p_const->assignment->pos);
		}
		p_const->value = cv->value;
		p_const->_is_reducing = false;
		p_const->is_reduced = true;

	} else throw ANALYZER_ERROR(Error::TYPE_ERROR, "expected a contant expression.", p_const->assignment->pos);
}

void Analyzer::_resolve_parameters(Parser::FunctionNode* p_func) {
	for (int i = 0; i < p_func->args.size(); i++) {
		if (p_func->args[i].default_value != nullptr) {
			_reduce_expression(p_func->args[i].default_value);
			if (p_func->args[i].default_value->type != Parser::Node::Type::CONST_VALUE) 
				throw ANALYZER_ERROR(Error::TYPE_ERROR, "expected a contant expression.", p_func->args[i].default_value->pos);
			ptr<Parser::ConstValueNode> cv = ptrcast<Parser::ConstValueNode>(p_func->args[i].default_value);
			if (cv->value.get_type() != var::INT && cv->value.get_type() != var::FLOAT &&
				cv->value.get_type() != var::BOOL && cv->value.get_type() != var::STRING &&
				cv->value.get_type() != var::_NULL) {
				throw ANALYZER_ERROR(Error::TYPE_ERROR, "expected a constant expression.", p_func->args[i].default_value->pos);
			}
			p_func->default_args.push_back(cv->value);
		}
	}
}

void Analyzer::_resolve_enumvalue(Parser::EnumValueNode& p_enumvalue, int* p_possible) {
	if (p_enumvalue.is_reduced) return;
	if (p_enumvalue._is_reducing) throw ANALYZER_ERROR(Error::TYPE_ERROR, "cyclic enum value dependancy found.", p_enumvalue.expr->pos);
	p_enumvalue._is_reducing = true;

	if (p_enumvalue.expr != nullptr) {
		_reduce_expression(p_enumvalue.expr);
		if (p_enumvalue.expr->type != Parser::Node::Type::CONST_VALUE)
			throw ANALYZER_ERROR(Error::TYPE_ERROR, "enum value must be a constant integer.", p_enumvalue.expr->pos);
		ptr<Parser::ConstValueNode> cv = ptrcast<Parser::ConstValueNode>(p_enumvalue.expr);
		if (cv->value.get_type() != var::INT) throw ANALYZER_ERROR(Error::TYPE_ERROR, "enum value must be a constant integer.", p_enumvalue.expr->pos);
		p_enumvalue.value = cv->value;
	} else {
		p_enumvalue.value = (p_possible) ? *p_possible: -1;
	}
	if (p_possible) *p_possible = (int)p_enumvalue.value + 1;

	p_enumvalue._is_reducing = false;
	p_enumvalue.is_reduced = true;
}

void Analyzer::_check_operator_methods(const Parser::FunctionNode* p_func) {
	const String& name = p_func->name;
	const int params = (int)p_func->args.size();
	int required = 0;
	if (name == GlobalStrings::copy) required = 1;
	else if (name == GlobalStrings::to_string) required = 0;
	else if (name == GlobalStrings::__call) return;
	else if (name == GlobalStrings::__iter_begin) required = 0;
	else if (name == GlobalStrings::__iter_has_next) required = 0;
	else if (name == GlobalStrings::__iter_next) required = 0;
	else if (name == GlobalStrings::__get_mapped) required = 0;
	else if (name == GlobalStrings::__set_mapped) required = 1;
	else if (name == GlobalStrings::__hash) required = 0;
	else if (name == GlobalStrings::__add) required = 1;
	else if (name == GlobalStrings::__sub) required = 1;
	else if (name == GlobalStrings::__mul) required = 1;
	else if (name == GlobalStrings::__div) required = 1;
	else if (name == GlobalStrings::__gt) required = 1;
	else if (name == GlobalStrings::__lt) required = 1;
	else if (name == GlobalStrings::__eq) required = 1;
	else return;

	if (params != required)
		throw ANALYZER_ERROR(Error::INVALID_ARG_COUNT, String::format("method \"%s\" required %i parameter(s) %i given.", name.c_str(), required, params), p_func->pos);
}

void Analyzer::_check_super_constructor_call(const Parser::BlockNode* p_block) {
	int constructor_argc = 0;
	int default_argc = 0;

	Parser::ClassNode* current_class = parser->parser_context.current_class;
	switch (parser->parser_context.current_class->base_type) {
		case Parser::ClassNode::BASE_LOCAL: {
			Parser::FunctionNode* constructor = current_class->base_class->constructor;
			if (constructor == nullptr) return;
			constructor_argc = (int)constructor->args.size();
			default_argc = (int)constructor->default_args.size();
		} break;
		case Parser::ClassNode::BASE_EXTERN: {
			const Function* constructor = current_class->base_binary->get_constructor();
			if (constructor == nullptr) return;
			constructor_argc = (int)constructor->get_arg_count();
			default_argc = (int)constructor->get_default_args().size();
		} break;
		case Parser::ClassNode::BASE_NATIVE: {
			const StaticFuncBind* constructor = NativeClasses::singleton()->get_constructor(current_class->base_class_name);
			constructor_argc = (constructor) ? constructor->get_argc() : 0;
			default_argc = (constructor) ? constructor->get_method_info()->get_default_arg_count() : 0;
		} break;
	}

	if (constructor_argc - default_argc > 0) { // super call needed.
		if ((p_block->statements.size() == 0) || (p_block->statements[0]->type != Parser::Node::Type::CALL))
			throw ANALYZER_ERROR(Error::NOT_IMPLEMENTED, "super constructor call expected since base class doesn't have a default constructor.", p_block->pos);
		const Parser::CallNode* call = static_cast<const Parser::CallNode*>(p_block->statements[0].get());
		if (call->base->type != Parser::Node::Type::SUPER || call->method != nullptr)
			throw ANALYZER_ERROR(Error::NOT_IMPLEMENTED, "super constructor call expected since base class doesn't have a default constructor.", call->pos);
		current_class->has_super_ctor_call = true;
		_check_arg_count(constructor_argc, default_argc, (int)call->args.size(), call->pos);
	}

	if ((p_block->statements.size() > 0) && (p_block->statements[0]->type == Parser::Node::Type::CALL)) {
		const Parser::CallNode* call = static_cast<const Parser::CallNode*>(p_block->statements[0].get());
		if (call->base->type == Parser::Node::Type::SUPER && call->method == nullptr) current_class->has_super_ctor_call = true;
	}
}

void Analyzer::_check_arg_count(int p_argc, int p_default_argc, int p_args_given, Vect2i p_err_pos) {
	if (p_argc == -1 /*va args*/) return;

	int required_min_argc = p_argc - p_default_argc;
	if (required_min_argc > 0) {
		if (p_default_argc == 0) {
			if (p_args_given != p_argc) throw ANALYZER_ERROR(Error::INVALID_ARG_COUNT,
				String::format("expected excatly %i argument(s) for super constructor call", p_argc), p_err_pos);
		} else {
			if (p_args_given < required_min_argc) throw ANALYZER_ERROR(Error::INVALID_ARG_COUNT,
				String::format("expected at least %i argument(s) for super constructor call", required_min_argc), p_err_pos);
			else if (p_args_given > p_argc) throw ANALYZER_ERROR(Error::INVALID_ARG_COUNT,
				String::format("expected at most %i argument(s) for super constructor call", p_argc), p_err_pos);
		}
		// no argument is required -> check if argc exceeding
	} else if (p_args_given > p_argc) {
		throw ANALYZER_ERROR(Error::INVALID_ARG_COUNT,
			String::format("expected at most %i argument(s) for super constructor call", p_argc), p_err_pos);
	}
}

}

/******************************************************************************************************************/
/*                                         REDUCE BLOCK                                                           */
/******************************************************************************************************************/

namespace carbon {

void Analyzer::_reduce_block(ptr<Parser::BlockNode>& p_block) {

	Parser::BlockNode* parent_block = parser->parser_context.current_block;
	parser->parser_context.current_block = p_block.get();
	class ScopeDestruct {
	public:
		Parser::ParserContext* context = nullptr;
		Parser::BlockNode* parent_block = nullptr;
		ScopeDestruct(Parser::ParserContext* p_context, Parser::BlockNode* p_parent_block) {
			context = p_context;
			parent_block = p_parent_block;
		}
		~ScopeDestruct() {
			context->current_block = parent_block;
		}
	};
	ScopeDestruct destruct = ScopeDestruct(&parser->parser_context, parent_block);

	// if reducing constructor -> check super() call
	if (parser->parser_context.current_class && parser->parser_context.current_class->base_type != Parser::ClassNode::NO_BASE) {
		if (parser->parser_context.current_class->constructor == parser->parser_context.current_func) {
			_check_super_constructor_call(p_block.get());
		}
	}

	for (int i = 0; i < (int)p_block->statements.size(); i++) {
		parser->parser_context.current_statement_ind = i;

		switch (p_block->statements[i]->type) {
			case Parser::Node::Type::UNKNOWN:
			case Parser::Node::Type::FILE:
			case Parser::Node::Type::CLASS:
			case Parser::Node::Type::ENUM:
			case Parser::Node::Type::FUNCTION:
			case Parser::Node::Type::BLOCK:
				THROW_BUG("invalid statement type in analyzer.");

			case Parser::Node::Type::IDENTIFIER: {
				_reduce_expression(p_block->statements[i]); // to check if the identifier exists.
				ANALYZER_WARNING(Warning::STAND_ALONE_EXPRESSION, "", p_block->statements[i]->pos);
				p_block->statements.erase(p_block->statements.begin() + i--);
			} break;

			case Parser::Node::Type::VAR: {
				ptr<Parser::VarNode> var_node = ptrcast<Parser::VarNode>(p_block->statements[i]);
				if (var_node->assignment != nullptr) {
					parser->parser_context.current_var = var_node.get();
					_reduce_expression(var_node->assignment);
					parser->parser_context.current_var = nullptr;
				}
			} break;

			case Parser::Node::Type::CONST: {
				ptr<Parser::ConstNode> const_node = ptrcast<Parser::ConstNode>(p_block->statements[i]);
				parser->parser_context.current_const = const_node.get();
				_resolve_constant(const_node.get());
				parser->parser_context.current_const = nullptr;
			} break;

			case Parser::Node::Type::CONST_VALUE:
			case Parser::Node::Type::ARRAY:
			case Parser::Node::Type::MAP:
				ANALYZER_WARNING(Warning::STAND_ALONE_EXPRESSION, "", p_block->statements[i]->pos);
				p_block->statements.erase(p_block->statements.begin() + i--);
				break;

			case Parser::Node::Type::THIS:
			case Parser::Node::Type::SUPER:
				ANALYZER_WARNING(Warning::STAND_ALONE_EXPRESSION, "", p_block->statements[i]->pos);
				p_block->statements.erase(p_block->statements.begin() + i--);
				break;

			case Parser::Node::Type::BUILTIN_FUNCTION:
			case Parser::Node::Type::BUILTIN_TYPE:
				ANALYZER_WARNING(Warning::STAND_ALONE_EXPRESSION, "", p_block->statements[i]->pos);
				p_block->statements.erase(p_block->statements.begin() + i--);
				break;

			case Parser::Node::Type::CALL: {
				ptr<Parser::CallNode> call = ptrcast<Parser::CallNode>(p_block->statements[i]);
				if (call->is_compilttime) {
					_resolve_compiletime_funcs(call);
					p_block->statements.erase(p_block->statements.begin() + i--);
					break;
				}
			} // [[FALLTHROUGH]]
			case Parser::Node::Type::INDEX:
			case Parser::Node::Type::MAPPED_INDEX:
			case Parser::Node::Type::OPERATOR: {
				_reduce_expression(p_block->statements[i]);
			} break;

			case Parser::Node::Type::CONTROL_FLOW: {
				ptr<Parser::ControlFlowNode> cf_node = ptrcast<Parser::ControlFlowNode>(p_block->statements[i]);
				switch (cf_node->cf_type) {

					case Parser::ControlFlowNode::IF: {
						ASSERT(cf_node->args.size() == 1);
						_reduce_expression(cf_node->args[0]);
						_reduce_block(cf_node->body);
						if (cf_node->body_else != nullptr) {
							_reduce_block(cf_node->body_else);
							// if it's statements cleared it needto be removed.
							if (cf_node->body_else->statements.size() == 0) cf_node->body_else = nullptr;
						}
					} break;

					case Parser::ControlFlowNode::SWITCH: {
						ASSERT(cf_node->args.size() == 1);
						_reduce_expression(cf_node->args[0]);

						Parser::EnumNode* _switch_enum = nullptr;
						int _enum_case_count = 0; bool _check_missed_enum = true;
						if (cf_node->switch_cases.size() > 1 && cf_node->switch_cases[0].expr->type == Parser::Node::Type::IDENTIFIER) {
							Parser::IdentifierNode* id = ptrcast<Parser::IdentifierNode>(cf_node->switch_cases[0].expr).get();
							if (id->ref == Parser::IdentifierNode::REF_ENUM_VALUE) {
								_switch_enum = id->_enum_value->_enum;
							}
						}

						for (int j = 0; j < (int)cf_node->switch_cases.size(); j++) {
							if (cf_node->switch_cases[j].default_case) _check_missed_enum = false;
							if (_check_missed_enum && cf_node->switch_cases[j].expr->type == Parser::Node::Type::IDENTIFIER) {
								Parser::IdentifierNode* id = ptrcast<Parser::IdentifierNode>(cf_node->switch_cases[j].expr).get();
								if (id->ref == Parser::IdentifierNode::REF_ENUM_VALUE) {
									if (id->_enum_value->_enum == _switch_enum) _enum_case_count++;
								} else _check_missed_enum = false;
							} else _check_missed_enum = false;

							_reduce_expression(cf_node->switch_cases[j].expr);
							if (cf_node->switch_cases[j].expr->type != Parser::Node::Type::CONST_VALUE)
								throw ANALYZER_ERROR(Error::TYPE_ERROR, "switch case value must be a constant integer.", cf_node->switch_cases[j].expr->pos);
							ptr<Parser::ConstValueNode> cv = ptrcast<Parser::ConstValueNode>(cf_node->switch_cases[j].expr);
							if (cv->value.get_type() != var::INT)
								throw ANALYZER_ERROR(Error::TYPE_ERROR, "switch case must be a constant integer.", cf_node->switch_cases[j].expr->pos);
							cf_node->switch_cases[j].value = cv->value;

							for (int _j = 0; _j < j; _j++) {
								if (cf_node->switch_cases[_j].value == cf_node->switch_cases[j].value) {
									throw ANALYZER_ERROR(Error::TYPE_ERROR, String::format("case value %lli has already defined at line %lli.",
										cf_node->switch_cases[j].value, cf_node->switch_cases[_j].pos.y), cf_node->switch_cases[j].pos);
								}
							}

							_reduce_block(cf_node->switch_cases[j].body);
						}

						if (_check_missed_enum && _enum_case_count != _switch_enum->values.size()) {
							ANALYZER_WARNING(Warning::MISSED_ENUM_IN_SWITCH, "", cf_node->pos);
						}

					} break;

					case Parser::ControlFlowNode::WHILE: {
						ASSERT(cf_node->args.size() == 1);
						_reduce_expression(cf_node->args[0]);
						if (cf_node->args[0]->type == Parser::Node::Type::CONST_VALUE) {
							if (ptrcast<Parser::ConstValueNode>(cf_node->args[0])->value.operator bool()) {
								if (!cf_node->has_break) {
									ANALYZER_WARNING(Warning::NON_TERMINATING_LOOP, "", cf_node->args[0]->pos);
								}
							} else {
								ANALYZER_WARNING(Warning::UNREACHABLE_CODE, "", cf_node->args[0]->pos);
								p_block->statements.erase(p_block->statements.begin() + i--);
							}
						}
						_reduce_block(cf_node->body);
					} break;

					case Parser::ControlFlowNode::FOR: {
						ASSERT(cf_node->args.size() == 3);

						// reduce loop arguments.
						Parser::BlockNode* parent_block = parser->parser_context.current_block;
						parser->parser_context.current_block = cf_node->body.get();
						if (cf_node->args[0] != nullptr && cf_node->args[0]->type == Parser::Node::Type::VAR) {
							cf_node->body->local_vars.push_back(ptrcast<Parser::VarNode>(cf_node->args[0]));
							_reduce_expression(ptrcast<Parser::VarNode>(cf_node->args[0])->assignment);
						} else _reduce_expression(cf_node->args[0]);
						_reduce_expression(cf_node->args[1]);
						_reduce_expression(cf_node->args[2]);
						parser->parser_context.current_block = parent_block;

						_reduce_block(cf_node->body);
						if (cf_node->args[0] == nullptr && cf_node->args[1] == nullptr && cf_node->args[2] == nullptr) {
							if (!cf_node->has_break) {
								ANALYZER_WARNING(Warning::NON_TERMINATING_LOOP, "", cf_node->pos);
							}
						}
					} break;

					case Parser::ControlFlowNode::FOREACH: {
						ASSERT(cf_node->args.size() == 2);

						// reduce loop arguments.
						Parser::BlockNode* parent_block = parser->parser_context.current_block;
						parser->parser_context.current_block = cf_node->body.get();
						cf_node->body->local_vars.push_back(ptrcast<Parser::VarNode>(cf_node->args[0]));
						_reduce_expression(ptrcast<Parser::VarNode>(cf_node->args[0])->assignment);
						_reduce_expression(cf_node->args[1]);
						parser->parser_context.current_block = parent_block;

						_reduce_block(cf_node->body);
					} break;

					case Parser::ControlFlowNode::BREAK:
					case Parser::ControlFlowNode::CONTINUE: {
						ASSERT(cf_node->args.size() == 0);
					} break;
					case Parser::ControlFlowNode::RETURN: {
						ASSERT(cf_node->args.size() <= 1);
						if (cf_node->args.size() == 1) {
							_reduce_expression(cf_node->args[0]);
						}
					} break;
				}
			} break;
		} // statement switch ends.
	}
	parser->parser_context.current_statement_ind = -1;

	// remove reduced && un-wanted statements.
	for (int i = 0; i < (int)p_block->statements.size(); i++) {
		// remove all local constant statments. no need anymore.
		if (p_block->statements[i]->type == Parser::Node::Type::CONST) {
			p_block->statements.erase(p_block->statements.begin() + i--);

		} else if (p_block->statements[i]->type == Parser::Node::Type::CONST_VALUE) {
			ANALYZER_WARNING(Warning::STAND_ALONE_EXPRESSION, "", p_block->statements[i]->pos);
			p_block->statements.erase(p_block->statements.begin() + i--);

			// remove all statements after return
		} else if (p_block->statements[i]->type == Parser::Node::Type::CONTROL_FLOW) {
			Parser::ControlFlowNode* cf = ptrcast<Parser::ControlFlowNode>(p_block->statements[i]).get();
			if (cf->cf_type == Parser::ControlFlowNode::RETURN) {
				if (i != p_block->statements.size() - 1) {
					ANALYZER_WARNING(Warning::UNREACHABLE_CODE, "", p_block->statements[i + 1]->pos);
					p_block->statements.erase(p_block->statements.begin() + i + 1, p_block->statements.end());
				}
			} else if (cf->cf_type == Parser::ControlFlowNode::IF) {
				if (cf->args[0]->type == Parser::Node::Type::CONST_VALUE && ptrcast<Parser::ConstValueNode>(cf->args[0])->value.operator bool() == false) {
					if (cf->body_else == nullptr) {
						ANALYZER_WARNING(Warning::UNREACHABLE_CODE, "", p_block->statements[i]->pos);
						p_block->statements.erase(p_block->statements.begin() + i--);
					}
				}
			}

			// remove all compile time functions.
		} else if (p_block->statements[i]->type == Parser::Node::Type::CALL) {
			Parser::CallNode* call = ptrcast<Parser::CallNode>(p_block->statements[i]).get();
			if (call->base->type == Parser::Node::Type::BUILTIN_FUNCTION) {
				if (BuiltinFunctions::is_compiletime(ptrcast<Parser::BuiltinFunctionNode>(call->base)->func)) {
					p_block->statements.erase(p_block->statements.begin() + i--);
				}
			}
		}
	}
}

} // namespace carbon

/******************************************************************************************************************/
/*                                         REDUCE EXPRESSION                                                      */
/******************************************************************************************************************/


namespace carbon {

void Analyzer::_reduce_expression(ptr<Parser::Node>& p_expr) {

	if (p_expr == nullptr) return;

	// Prevent stack overflow.
	if (p_expr->is_reduced) return;
	p_expr->is_reduced = true;

	switch (p_expr->type) {

		case Parser::Node::Type::THIS:
		case Parser::Node::Type::SUPER:
		case Parser::Node::Type::BUILTIN_TYPE:
		case Parser::Node::Type::BUILTIN_FUNCTION:
			break;

		case Parser::Node::Type::IDENTIFIER: {
			_reduce_identifier(p_expr);
			_check_identifier(p_expr);
		} break;

			// reduce ArrayNode
		case Parser::Node::Type::ARRAY: {
			ptr<Parser::ArrayNode> arr = ptrcast<Parser::ArrayNode>(p_expr);
			bool all_const = true;
			for (int i = 0; i < (int)arr->elements.size(); i++) {
				_reduce_expression(arr->elements[i]);
				if (arr->elements[i]->type != Parser::Node::Type::CONST_VALUE) {
					all_const = false;
				}
			}
			arr->_can_const_fold = all_const;

			if (all_const && parser->parser_context.current_const != nullptr) {
				Array arr_v;
				for (int i = 0; i < (int)arr->elements.size(); i++) {
					arr_v.push_back(ptrcast<Parser::ConstValueNode>(arr->elements[i])->value);
				}
				ptr<Parser::ConstValueNode> cv = new_node<Parser::ConstValueNode>(arr_v);
				cv->pos = arr->pos; p_expr = cv;
			}
		} break;

			// reduce MapNode
		case Parser::Node::Type::MAP: {
			ptr<Parser::MapNode> map = ptrcast<Parser::MapNode>(p_expr);
			bool all_const = true;
			for (int i = 0; i < (int)map->elements.size(); i++) {
				_reduce_expression(map->elements[i].key);
				if (map->elements[i].key->type == Parser::Node::Type::CONST_VALUE) {
					var& key_v = ptrcast<Parser::ConstValueNode>(map->elements[i].key)->value;
					if (!var::is_hashable(key_v.get_type())) throw ANALYZER_ERROR(Error::TYPE_ERROR, String::format("unhasnable type %s used as map key.", key_v.get_type_name().c_str()), map->pos);
				}
				_reduce_expression(map->elements[i].value);

				if (map->elements[i].key->type != Parser::Node::Type::CONST_VALUE || map->elements[i].value->type != Parser::Node::Type::CONST_VALUE) {
					all_const = false;
				}
			}
			map->_can_const_fold = all_const;

			if (all_const && parser->parser_context.current_const != nullptr) {
				Map map_v;
				for (int i = 0; i < (int)map->elements.size(); i++) {
					var& _key = ptrcast<Parser::ConstValueNode>(map->elements[i].key)->value;
					var& _val = ptrcast<Parser::ConstValueNode>(map->elements[i].value)->value;
					map_v[_key] = _val;
				}
				ptr<Parser::ConstValueNode> cv = new_node<Parser::ConstValueNode>(map_v);
				cv->pos = map->pos; p_expr = cv;
			}
		} break;

		case Parser::Node::Type::CALL: {
			_reduce_call(p_expr);
		} break;

		case Parser::Node::Type::INDEX: {
			_reduce_indexing(p_expr);
		} break;

			// reduce MappedIndexNode
		case Parser::Node::Type::MAPPED_INDEX: {
			ptr<Parser::MappedIndexNode> mapped_index = ptrcast<Parser::MappedIndexNode>(p_expr);
			_reduce_expression(mapped_index->base);
			_reduce_expression(mapped_index->key);
			if (mapped_index->base->type == Parser::Node::Type::CONST_VALUE && mapped_index->key->type == Parser::Node::Type::CONST_VALUE) {
				Parser::ConstValueNode* base = ptrcast<Parser::ConstValueNode>(mapped_index->base).get();
				Parser::ConstValueNode* key = ptrcast<Parser::ConstValueNode>(mapped_index->key).get();
				try {
					ptr<Parser::ConstValueNode> cv = new_node<Parser::ConstValueNode>(base->value.__get_mapped(key->value));
					cv->pos = base->pos; p_expr = cv;
				} catch (Error& err) {
					throw ANALYZER_ERROR(err.get_type(), err.what(), key->pos);
				}
			}
		} break;

			///////////////////////////////////////////////////////////////////////////////////////////////

		case Parser::Node::Type::OPERATOR: {
			ptr<Parser::OperatorNode> op = ptrcast<Parser::OperatorNode>(p_expr);

			bool all_const = true;
			for (int i = 0; i < (int)op->args.size(); i++) {
				_reduce_expression(op->args[i]);
				if (op->args[i]->type != Parser::Node::Type::CONST_VALUE) all_const = false;
			}

			switch (op->op_type) {

				case Parser::OperatorNode::OpType::OP_EQ:
				case Parser::OperatorNode::OpType::OP_PLUSEQ:
				case Parser::OperatorNode::OpType::OP_MINUSEQ:
				case Parser::OperatorNode::OpType::OP_MULEQ:
				case Parser::OperatorNode::OpType::OP_DIVEQ:
				case Parser::OperatorNode::OpType::OP_MOD_EQ:
				case Parser::OperatorNode::OpType::OP_LTEQ:
				case Parser::OperatorNode::OpType::OP_GTEQ:
				case Parser::OperatorNode::OpType::OP_BIT_LSHIFT_EQ:
				case Parser::OperatorNode::OpType::OP_BIT_RSHIFT_EQ:
				case Parser::OperatorNode::OpType::OP_BIT_OR_EQ:
				case Parser::OperatorNode::OpType::OP_BIT_AND_EQ:
				case Parser::OperatorNode::OpType::OP_BIT_XOR_EQ: {

					if (op->args[0]->type == Parser::Node::Type::IDENTIFIER) {
						switch (ptrcast<Parser::IdentifierNode>(op->args[0])->ref) {
							case Parser::IdentifierNode::REF_PARAMETER:
							case Parser::IdentifierNode::REF_LOCAL_VAR:
							case Parser::IdentifierNode::REF_MEMBER_VAR:
							case Parser::IdentifierNode::REF_STATIC_VAR:
								break;
							case Parser::IdentifierNode::REF_LOCAL_CONST:
							case Parser::IdentifierNode::REF_MEMBER_CONST:
							case Parser::IdentifierNode::REF_ENUM_NAME:
							case Parser::IdentifierNode::REF_ENUM_VALUE:
							case Parser::IdentifierNode::REF_FUNCTION:
							case Parser::IdentifierNode::REF_CARBON_CLASS:
							case Parser::IdentifierNode::REF_NATIVE_CLASS:
							case Parser::IdentifierNode::REF_EXTERN:
								throw ANALYZER_ERROR(Error::TYPE_ERROR, "invalid assignment (only assignment on variables/parameters are valid).", op->args[0]->pos);
						}
					} else if (op->args[0]->type == Parser::Node::Type::THIS) {
						throw ANALYZER_ERROR(Error::TYPE_ERROR, "can't assign anything to \"this\".", op->args[0]->pos);
					} else if (op->args[0]->type == Parser::Node::Type::SUPER) {
						throw ANALYZER_ERROR(Error::TYPE_ERROR, "can't assign anything to \"super\".", op->args[0]->pos);
					} else if (op->args[0]->type == Parser::Node::Type::CONST_VALUE) {
						throw ANALYZER_ERROR(Error::TYPE_ERROR, "can't assign anything to constant values.", op->args[0]->pos);
					} else if (op->args[0]->type == Parser::Node::Type::ARRAY) {
						throw ANALYZER_ERROR(Error::TYPE_ERROR, "can't assign anything to array literal.", op->args[0]->pos);
					} else if (op->args[0]->type == Parser::Node::Type::MAP) {
						throw ANALYZER_ERROR(Error::TYPE_ERROR, "can't assign anything to map literal.", op->args[0]->pos);
					} else if (op->args[0]->type == Parser::Node::Type::BUILTIN_FUNCTION) {
						throw ANALYZER_ERROR(Error::TYPE_ERROR, "can't assign anything to builtin function.", op->args[0]->pos);
					} else if (op->args[0]->type == Parser::Node::Type::BUILTIN_TYPE) {
						throw ANALYZER_ERROR(Error::TYPE_ERROR, "can't assign anything to builtin type.", op->args[0]->pos);
					}
				} break;

				default: { // Remaining binary/unary operators.
					if (!all_const) break;
					stdvec<var*> args;
					for (int i = 0; i < (int)op->args.size(); i++) args.push_back(&ptrcast<Parser::ConstValueNode>(op->args[i])->value);
				#define SET_EXPR_CONST_NODE(m_expr, m_pos)											      \
					do {                                                                                  \
						var value;																		  \
						try {																			  \
							value = (m_expr);														      \
						} catch (Throwable& err) {														  \
							throw ANALYZER_ERROR(err.get_type(), err.what(), op->pos);					  \
						}																				  \
						ptr<Parser::ConstValueNode> cv = new_node<Parser::ConstValueNode>(value);         \
						cv->pos = m_pos;                                                                  \
						p_expr = cv;                                                                      \
					} while (false)

					switch (op->op_type) {
						case Parser::OperatorNode::OpType::OP_EQEQ:
							SET_EXPR_CONST_NODE(*args[0] == *args[1], op->pos);
							break;
						case Parser::OperatorNode::OpType::OP_PLUS:
							SET_EXPR_CONST_NODE(*args[0] + *args[1], op->pos);
							break;
						case Parser::OperatorNode::OpType::OP_MINUS:
							SET_EXPR_CONST_NODE(*args[0] - *args[1], op->pos);
							break;
						case Parser::OperatorNode::OpType::OP_MUL:
							SET_EXPR_CONST_NODE(*args[0] * *args[1], op->pos);
							break;
						case Parser::OperatorNode::OpType::OP_DIV:
							SET_EXPR_CONST_NODE(*args[0] / *args[1], op->pos);
							break;
						case Parser::OperatorNode::OpType::OP_MOD:
							SET_EXPR_CONST_NODE(*args[0] % *args[1], op->pos);
							break;
						case Parser::OperatorNode::OpType::OP_LT:
							SET_EXPR_CONST_NODE(*args[0] < *args[1], op->pos);
							break;
						case Parser::OperatorNode::OpType::OP_GT:
							SET_EXPR_CONST_NODE(*args[0] > * args[1], op->pos);
							break;
						case Parser::OperatorNode::OpType::OP_AND:
							SET_EXPR_CONST_NODE(args[0]->operator bool() && args[1]->operator bool(), op->pos);
							break;
						case Parser::OperatorNode::OpType::OP_OR:
							SET_EXPR_CONST_NODE(args[0]->operator bool() || args[1]->operator bool(), op->pos);
							break;
						case Parser::OperatorNode::OpType::OP_NOTEQ:
							SET_EXPR_CONST_NODE(*args[0] != *args[1], op->pos);
							break;
						case Parser::OperatorNode::OpType::OP_BIT_LSHIFT:
							SET_EXPR_CONST_NODE(args[0]->operator int64_t() << args[1]->operator int64_t(), op->pos);
							break;
						case Parser::OperatorNode::OpType::OP_BIT_RSHIFT:
							SET_EXPR_CONST_NODE(args[0]->operator int64_t() >> args[1]->operator int64_t(), op->pos);
							break;
						case Parser::OperatorNode::OpType::OP_BIT_OR:
							SET_EXPR_CONST_NODE(args[0]->operator int64_t() | args[1]->operator int64_t(), op->pos);
							break;
						case Parser::OperatorNode::OpType::OP_BIT_AND:
							SET_EXPR_CONST_NODE(args[0]->operator int64_t() & args[1]->operator int64_t(), op->pos);
							break;
						case Parser::OperatorNode::OpType::OP_BIT_XOR:
							SET_EXPR_CONST_NODE(args[0]->operator int64_t() ^ args[1]->operator int64_t(), op->pos);
							break;

						case Parser::OperatorNode::OpType::OP_NOT:
							SET_EXPR_CONST_NODE(!args[0]->operator bool(), op->pos);
							break;
						case Parser::OperatorNode::OpType::OP_BIT_NOT:
							SET_EXPR_CONST_NODE(~args[0]->operator int64_t(), op->pos);
							break;
						case Parser::OperatorNode::OpType::OP_POSITIVE:
							switch (args[0]->get_type()) {
								case var::BOOL:
								case var::INT:
								case var::FLOAT: {
									SET_EXPR_CONST_NODE(*args[0], op->pos);
								} break;
								default:
									throw ANALYZER_ERROR(Error::OPERATOR_NOT_SUPPORTED,
										String::format("unary operator \"+\" not supported on %s.", args[0]->get_type_name()), op->pos);
							}
							break;
						case Parser::OperatorNode::OpType::OP_NEGATIVE:
							switch (args[0]->get_type()) {
								case var::BOOL:
								case var::INT:
									SET_EXPR_CONST_NODE(-args[0]->operator int64_t(), op->pos);
									break;
								case var::FLOAT:
									SET_EXPR_CONST_NODE(-args[0]->operator double(), op->pos);
									break;
								default:
									throw ANALYZER_ERROR(Error::OPERATOR_NOT_SUPPORTED,
										String::format("unary operator \"-\" not supported on %s.", args[0]->get_type_name()), op->pos);
							}
							break;
					}
					#undef SET_EXPR_CONST_NODE
					MISSED_ENUM_CHECK(Parser::OperatorNode::OpType::_OP_MAX_, 33);
				}
			}
		} break;

		case Parser::Node::Type::CONST_VALUE: {
		} break; // Can't reduce anymore.

		default: {
			THROW_BUG("Invalid expression node.");
		}
	}
}
} // namespace carbon

/******************************************************************************************************************/
/*                                         REDUCE IDENTIFIER                                                      */
/******************************************************************************************************************/

namespace carbon {

Parser::IdentifierNode Analyzer::_find_member(const Parser::MemberContainer* p_container, const String& p_name) {

	Parser::IdentifierNode id; id.name = p_name;
	if (!p_container) return id;

	id.ref_base = Parser::IdentifierNode::BASE_LOCAL;

	for (int i = 0; i < (int)p_container->vars.size(); i++) {
		if (p_container->vars[i]->name == id.name) {
			if (p_container->vars[i]->is_static) id.ref = Parser::IdentifierNode::REF_STATIC_VAR;
			else id.ref = Parser::IdentifierNode::REF_MEMBER_VAR;
			id._var = p_container->vars[i].get();
			return id;
		}
	}
	for (int i = 0; i < (int)p_container->functions.size(); i++) {
		// constructors are REF_CARBON_CLASS 
		if (!p_container->functions[i]->is_constructor && p_container->functions[i]->name == id.name) {
			id.ref = Parser::IdentifierNode::REF_FUNCTION;
			id._func = p_container->functions[i].get();
			return id;
		}
	}
	for (int i = 0; i < (int)p_container->constants.size(); i++) {
		if (p_container->constants[i]->name == id.name) {
			id.ref = Parser::IdentifierNode::REF_MEMBER_CONST;
			_resolve_constant(p_container->constants[i].get());
			id._const = p_container->constants[i].get();
			return id;
		}
	}
	if (p_container->unnamed_enum != nullptr) {
		for (std::pair<String, Parser::EnumValueNode> pair : p_container->unnamed_enum->values) {
			if (pair.first == id.name) {
				id.ref = Parser::IdentifierNode::REF_ENUM_VALUE;
				_resolve_enumvalue(p_container->unnamed_enum->values[pair.first]);
				id._enum_value = &p_container->unnamed_enum->values[pair.first];
				return id;
			}
		}
	}
	for (int i = 0; i < (int)p_container->enums.size(); i++) {
		if (p_container->enums[i]->name == id.name) {
			id.ref = Parser::IdentifierNode::REF_ENUM_NAME;
			id._enum_node = p_container->enums[i].get();
			return id;
		}
	}

	ASSERT(id.ref == Parser::IdentifierNode::REF_UNKNOWN);
	if (p_container->type == Parser::Node::Type::CLASS) {
		const Parser::ClassNode* _class = static_cast<const Parser::ClassNode*>(p_container);

		switch (_class->base_type) {
			case Parser::ClassNode::BASE_LOCAL:
				return _find_member(_class->base_class, p_name);
			case Parser::ClassNode::BASE_NATIVE: {
				ptr<BindData> bd = NativeClasses::singleton()->find_bind_data(_class->base_class_name, p_name);
				if (bd) {
					id.ref_base = Parser::IdentifierNode::BASE_NATIVE;
					switch (bd->get_member_info()->get_type()) {
						case MemberInfo::Type::METHOD: {
							id.ref = Parser::IdentifierNode::REF_FUNCTION;
							id._method_info = ptrcast<MethodBind>(bd)->get_method_info().get();
							return id;
						} break;
						case MemberInfo::Type::PROPERTY: {
							id._prop_info = ptrcast<PropertyBind>(bd)->get_prop_info().get();
							if (id._prop_info->is_const()) id.ref = Parser::IdentifierNode::REF_MEMBER_CONST;
							else if (id._prop_info->is_static()) id.ref = Parser::IdentifierNode::REF_STATIC_VAR;
							else id.ref = Parser::IdentifierNode::REF_MEMBER_VAR;
							return id;
						} break;
						case MemberInfo::Type::ENUM: {
							id.ref = Parser::IdentifierNode::REF_ENUM_NAME;
							id._enum_info = ptrcast<EnumBind>(bd)->get_enum_info().get();
							return id;
						} break;
						case MemberInfo::Type::ENUM_VALUE: {
							id.ref = Parser::IdentifierNode::REF_ENUM_VALUE;
							id._enum_value_info = ptrcast<EnumValueBind>(bd)->get_enum_value_info().get();
							return id;
						} break;
					}
				}
			} break;
			case Parser::ClassNode::BASE_EXTERN:
				id.ref_base = Parser::IdentifierNode::BASE_EXTERN;
				MemberInfo* mi = _class->base_binary->get_member_info(p_name).get();
				if (mi == nullptr) break;
				switch (mi->get_type()) {
					case MemberInfo::Type::CLASS: {
						id.ref = Parser::IdentifierNode::REF_EXTERN; // extern class
						id._class_info = static_cast<ClassInfo*>(mi);
						return id;
					} break;
					case MemberInfo::Type::METHOD: {
						id.ref = Parser::IdentifierNode::REF_FUNCTION;
						id._method_info = static_cast<MethodInfo*>(mi);
						return id;
					} break;
					case MemberInfo::Type::PROPERTY: {
						id._prop_info = static_cast<PropertyInfo*>(mi);
						if (id._prop_info->is_const()) id.ref = Parser::IdentifierNode::REF_MEMBER_CONST;
						else if (id._prop_info->is_static()) id.ref = Parser::IdentifierNode::REF_STATIC_VAR;
						else id.ref = Parser::IdentifierNode::REF_MEMBER_VAR;
						return id;
					} break;
					case MemberInfo::Type::ENUM: {
						id.ref = Parser::IdentifierNode::REF_ENUM_NAME;
						id._enum_info = static_cast<EnumInfo*>(mi);
						return id;
					} break;
					case MemberInfo::Type::ENUM_VALUE: {
						id.ref = Parser::IdentifierNode::REF_ENUM_VALUE;
						id._enum_value_info = static_cast<EnumValueInfo*>(mi);
						return id;
					} break;
				}
		}

	} else { // container is FileNode
		const Parser::FileNode* file_node = static_cast<const Parser::FileNode*>(p_container);

		for (int i = 0; i < (int)file_node->classes.size(); i++) {
			if (file_node->classes[i]->name == id.name) {
				id.ref = Parser::IdentifierNode::REF_CARBON_CLASS;
				id._class = file_node->classes[i].get();
				return id;
			}
		}

		if (NativeClasses::singleton()->is_class_registered(id.name)) {
			id.ref = Parser::IdentifierNode::REF_NATIVE_CLASS;
			id.ref_base = Parser::IdentifierNode::BASE_NATIVE;
			return id;
		}
	}

	id.ref_base = Parser::IdentifierNode::BASE_UNKNOWN;
	return id;
}

void Analyzer::_reduce_identifier(ptr<Parser::Node>& p_expr) {
	ASSERT(p_expr->type == Parser::Node::Type::IDENTIFIER);

	// search parameters.
	ptr<Parser::IdentifierNode> id = ptrcast<Parser::IdentifierNode>(p_expr);
	if (parser->parser_context.current_func) {
		for (int i = 0; i < (int)parser->parser_context.current_func->args.size(); i++) {
			if (parser->parser_context.current_func->args[i].name == id->name) {
				id->ref = Parser::IdentifierNode::REF_PARAMETER;
				id->ref_base = Parser::IdentifierNode::BASE_LOCAL;
				id->param_index = i;
				return;
			}
		}
	}

	// search in locals (var, const)
	Parser::BlockNode* outer_block = parser->parser_context.current_block;
	while (outer_block != nullptr && id->ref == Parser::IdentifierNode::REF_UNKNOWN) {
		for (int i = 0; i < (int)outer_block->local_vars.size(); i++) {
			Parser::VarNode* local_var = outer_block->local_vars[i].get();
			if (local_var->name == id->name) {

				if (p_expr->pos.x < local_var->pos.x || (p_expr->pos.x == local_var->pos.x && p_expr->pos.y < local_var->pos.y))
					throw ANALYZER_ERROR(Error::NAME_ERROR, String::format("local variable \"%s\" referenced before assigned", local_var->name.c_str()), id->pos);
				id->ref = Parser::IdentifierNode::REF_LOCAL_VAR;
				id->ref_base = Parser::IdentifierNode::BASE_LOCAL;
				id->_var = outer_block->local_vars[i].get();
				return;
			}
		}

		for (int i = 0; i < (int)outer_block->local_const.size(); i++) {
			if (outer_block->local_const[i]->name == id->name) {
				id->ref = Parser::IdentifierNode::REF_LOCAL_CONST;
				id->ref_base = Parser::IdentifierNode::BASE_LOCAL;
				_resolve_constant(outer_block->local_const[i].get());
				id->_const = outer_block->local_const[i].get();
				return;
			}
		}

		if (outer_block->parernt_node->type == Parser::Node::Type::BLOCK) {
			outer_block = ptrcast<Parser::BlockNode>(outer_block->parernt_node).get();
		} else {
			outer_block = nullptr;
		}
	}

	// if analyzing enum search in enums
	if (parser->parser_context.current_enum != nullptr) {
		for (std::pair<String, Parser::EnumValueNode> pair : parser->parser_context.current_enum->values) {
			if (pair.first == id->name) {
				id->ref = Parser::IdentifierNode::REF_ENUM_VALUE;
				_resolve_enumvalue(parser->parser_context.current_enum->values[pair.first]);
				id->_enum_value = &pair.second;
				return;
			}
		}
	}

	// search in current class.
	Parser::IdentifierNode _id = _find_member(parser->parser_context.current_class, id->name);
	if (_id.ref != Parser::IdentifierNode::REF_UNKNOWN) {
		_id.pos = id->pos; p_expr = newptr<Parser::IdentifierNode>(_id);
		return;
	}

	// search in current file.
	_id = _find_member(parser->file_node.get(), id->name);
	if (_id.ref != Parser::IdentifierNode::REF_UNKNOWN) {
		_id.pos = id->pos; p_expr = newptr<Parser::IdentifierNode>(_id);
		return;
	}

	// search in imports.
	for (int i = 0; i < (int)parser->file_node->imports.size(); i++) {
		if (parser->file_node->imports[i]->name == id->name) {
			id->ref = Parser::IdentifierNode::REF_EXTERN;
			id->_bytecode = parser->file_node->imports[i]->bytecode.get();
			return;
		}
	}
}


void Analyzer::_check_identifier(ptr<Parser::Node>& p_expr) {
	ASSERT(p_expr->type == Parser::Node::Type::IDENTIFIER);

	ptr<Parser::IdentifierNode> id = ptrcast<Parser::IdentifierNode>(p_expr);
	switch (id->ref) {
		case Parser::IdentifierNode::REF_UNKNOWN:
			throw ANALYZER_ERROR(Error::NAME_ERROR, String::format("identifier \"%s\" isn't defined.", id->name.c_str()), id->pos);
		case Parser::IdentifierNode::REF_LOCAL_CONST:
		case Parser::IdentifierNode::REF_MEMBER_CONST: {
			ptr<Parser::ConstValueNode> cv = new_node<Parser::ConstValueNode>(id->_const->value);
			cv->pos = id->pos; p_expr = cv;
		} break;
		case Parser::IdentifierNode::REF_ENUM_VALUE: {
			ptr<Parser::ConstValueNode> cv = new_node<Parser::ConstValueNode>(id->_enum_value->value);
			cv->pos = id->pos; p_expr = cv;
		} break;

		case Parser::IdentifierNode::REF_LOCAL_VAR:
		case Parser::IdentifierNode::REF_STATIC_VAR:
		case Parser::IdentifierNode::REF_MEMBER_VAR: {
			if (id->ref_base == Parser::IdentifierNode::BASE_LOCAL && parser->parser_context.current_var) {
				if (parser->parser_context.current_var->name == id->name) {
					throw ANALYZER_ERROR(Error::ATTRIBUTE_ERROR, String::format("invalid attribute access \"%s\" can't be used in it's own initialization.", id->name.c_str()), id->pos);
				}
			}
		} // [[fallthrought]]
		default: { // variable, parameter, function name, ...
			p_expr = id;
			break;
		}
	}
}

} // namespace carbon


/******************************************************************************************************************/
/*                                         REDUCE INDEXING                                                        */
/******************************************************************************************************************/

namespace carbon {

void Analyzer::_reduce_indexing(ptr < Parser::Node>& p_expr) {
	ASSERT(p_expr->type == Parser::Node::Type::INDEX);

	ptr<Parser::IndexNode> index = ptrcast<Parser::IndexNode>(p_expr);
	_reduce_expression(index->base);
	ASSERT(index->member->type == Parser::Node::Type::IDENTIFIER);
	ptr<Parser::IdentifierNode> member = ptrcast<Parser::IdentifierNode>(index->member);

	switch (index->base->type) {

		// String.prop; index base on built in type
		case Parser::Node::Type::BUILTIN_TYPE: {
			Parser::BuiltinTypeNode* bt = ptrcast<Parser::BuiltinTypeNode>(index->base).get();
			const MemberInfo* mi = TypeInfo::get_member_info(BuiltinTypes::get_var_type(bt->builtin_type), member->name).get();
			if (!mi) throw ANALYZER_ERROR(Error::NAME_ERROR, String::format("attribute \"%s\" doesn't exists on base %s.", member->name.c_str(), BuiltinTypes::get_type_name(bt->builtin_type).c_str()), member->pos);

			switch (mi->get_type()) {
				// var x = String.format;
				case MemberInfo::METHOD:
					break;

					// var x = int.something ? <-- is this even valid
				case MemberInfo::PROPERTY: {
					PropertyInfo* pi = (PropertyInfo*)mi;
					if (pi->is_const()) {
						ptr<Parser::ConstValueNode> cv = new_node<Parser::ConstValueNode>(pi->get_value());
						cv->pos = index->pos, p_expr = cv;
					}
					else THROW_BUG("can't be."); // builtin types can only have a constant property
				} break;

					// built in types cannot contain enum inside.
				case MemberInfo::ENUM:
				case MemberInfo::ENUM_VALUE:
					THROW_BUG("can't be.");
			}
		} break;

			// "string".member;
		case Parser::Node::Type::CONST_VALUE: {
			Parser::ConstValueNode* base = ptrcast<Parser::ConstValueNode>(index->base).get();
			try {
				ptr<Parser::ConstValueNode> cv = new_node<Parser::ConstValueNode>(base->value.get_member(member->name));
				cv->pos = member->pos; p_expr = cv;
			} catch (Error& err) {
				throw ANALYZER_ERROR(err.get_type(), err.what(), index->pos);
			}
		} break;

			// this.member; super.member; idf.member;
		case Parser::Node::Type::INDEX: { // <-- base is index node but reference reduced.
			Parser::IndexNode* _ind = ptrcast<Parser::IndexNode>(index->base).get();
			if (!_ind->_ref_reduced) break;
		}  // [[ FALLTHROUGH ]]
		case Parser::Node::Type::THIS:
		case Parser::Node::Type::SUPER:
		case Parser::Node::Type::IDENTIFIER: {

			Parser::IdentifierNode* base;
			enum _BaseClassRef { _THIS, _SUPER, _NEITHER };
			_BaseClassRef _base_class_ref = _NEITHER;

			if (index->base->type == Parser::Node::Type::THIS) {
				ptr<Parser::IdentifierNode> _id = newptr<Parser::IdentifierNode>(parser->parser_context.current_class->name);
				_id->ref = Parser::IdentifierNode::REF_CARBON_CLASS;
				_id->_class = parser->parser_context.current_class;
				index->base = _id;
				_base_class_ref = _THIS;
			} else if (index->base->type == Parser::Node::Type::SUPER) {
				if (parser->parser_context.current_class->base_type == Parser::ClassNode::BASE_LOCAL) {
					ptr<Parser::IdentifierNode> _id = newptr<Parser::IdentifierNode>(parser->parser_context.current_class->base_class->name);
					_id->ref = Parser::IdentifierNode::REF_CARBON_CLASS;
					_id->_class = parser->parser_context.current_class->base_class;
					index->base = _id;
				} else if (parser->parser_context.current_class->base_type == Parser::ClassNode::BASE_EXTERN) {
					ptr<Parser::IdentifierNode> _id = newptr<Parser::IdentifierNode>(parser->parser_context.current_class->base_class->name);
					_id->ref = Parser::IdentifierNode::REF_EXTERN;
					_id->_bytecode = parser->parser_context.current_class->base_binary.get();
					index->base = _id;
				}
				_base_class_ref = _SUPER;
			}

			if (index->base->type == Parser::Node::Type::INDEX) {
				base = ptrcast<Parser::IndexNode>(index->base)->member.get();
			} else {
				base = ptrcast<Parser::IdentifierNode>(index->base).get();
			}

			switch (base->ref) {
				case Parser::IdentifierNode::REF_UNKNOWN: {
					THROW_BUG("base can't be unknown.");
				} break;

				case Parser::IdentifierNode::REF_PARAMETER:
				case Parser::IdentifierNode::REF_LOCAL_VAR:
				case Parser::IdentifierNode::REF_MEMBER_VAR:
					break; // Can't reduce anymore.

				case Parser::IdentifierNode::REF_LOCAL_CONST:
				case Parser::IdentifierNode::REF_MEMBER_CONST: {
					THROW_BUG("there isn't any contant value currently support attribute access and most probably in the future");
				} break;

				// EnumClass.prop; <-- TODO: could the prop be a method?
				case Parser::IdentifierNode::REF_ENUM_NAME: {
					if (base->ref_base == Parser::IdentifierNode::BASE_LOCAL) {
						stdmap<String, Parser::EnumValueNode>::iterator it = base->_enum_node->values.find(member->name);
						if (it != base->_enum_node->values.end()) {
							member->ref = Parser::IdentifierNode::REF_ENUM_VALUE;
							member->ref_base = Parser::IdentifierNode::BASE_LOCAL;
							member->_enum_value = &(it->second);
							_resolve_enumvalue(base->_enum_node->values[it->first]);
							ptr<Parser::ConstValueNode> cv = new_node<Parser::ConstValueNode>(base->_enum_node->values[it->first].value);
							cv->pos = member->pos; p_expr = cv;
						} else {
							throw ANALYZER_ERROR(Error::NAME_ERROR,
								String::format("\"%s\" doesn't exists on base enum \"%s\"%.", member->name.c_str(), base->_enum_node->name.c_str()),
								member->pos);
						}
					} else { // ref on base native/extern.
						stdmap<String, int64_t>::const_iterator it = base->_enum_info->get_values().find(member->name);
						if (it != base->_enum_info->get_values().end()) {
							member->ref = Parser::IdentifierNode::REF_ENUM_VALUE;
							member->ref_base = base->ref_base;
							ptr<Parser::ConstValueNode> cv = new_node<Parser::ConstValueNode>(it->second);
							cv->pos = member->pos; p_expr = cv;
						} else {
							throw ANALYZER_ERROR(Error::NAME_ERROR,
								String::format("\"%s\" doesn't exists on base enum \"%s\"%.", member->name.c_str(), base->_enum_info->get_name().c_str()),
								member->pos);
						}
					}
				} break;

				case Parser::IdentifierNode::REF_ENUM_VALUE:
					throw ANALYZER_ERROR(Error::OPERATOR_NOT_SUPPORTED, "enum value doesn't support attribute access.", member->pos);

					// Aclass.prop;
				case Parser::IdentifierNode::REF_CARBON_CLASS: {
					Parser::IdentifierNode _id = _find_member(base->_class, member->name);
					_id.pos = member->pos;

					switch (_id.ref) {
						case Parser::IdentifierNode::REF_UNKNOWN:
							throw ANALYZER_ERROR(Error::ATTRIBUTE_ERROR, String::format("attribute \"%s\" isn't exists in base \"%s\".", member->name.c_str(), base->name.c_str()), member->pos);
						case Parser::IdentifierNode::REF_PARAMETER:
						case Parser::IdentifierNode::REF_LOCAL_VAR:
						case Parser::IdentifierNode::REF_LOCAL_CONST:
							THROW_BUG("can't be.");

							// Aclass.a_var <-- only valid if the base is this `this.a_var`
						case Parser::IdentifierNode::REF_MEMBER_VAR: {
							_id.pos = member->pos;
							if (_base_class_ref != _THIS && !_id._var->is_static) {
								throw ANALYZER_ERROR(Error::ATTRIBUTE_ERROR, String::format("non-static attribute \"%s\" cannot be access with a class reference \"%s\".", member->name.c_str(), base->name.c_str()), member->pos);
							}
							if (_base_class_ref == _THIS) {
								p_expr = newptr<Parser::IdentifierNode>(_id);
							} else {
								index->member = newptr<Parser::IdentifierNode>(_id);
								index->_ref_reduced = true;
							}
						} break;

							// Aclass.CONST;
						case Parser::IdentifierNode::REF_MEMBER_CONST: {
							var value;
							if (_id.ref_base == Parser::IdentifierNode::BASE_LOCAL) value = _id._const->value;
							else value = _id._prop_info->get_value();
							ptr<Parser::ConstValueNode> cv = new_node<Parser::ConstValueNode>(value);
							cv->pos = member->pos; p_expr = cv;
						} break;

							// Aclass.EnumClass;
						case Parser::IdentifierNode::REF_ENUM_NAME: {
							_id.pos = member->pos;
							if (_base_class_ref == _THIS) {
								p_expr = newptr<Parser::IdentifierNode>(_id);
							} else {
								index->member = newptr<Parser::IdentifierNode>(_id);
								index->_ref_reduced = true;
							}
						} break;

							// Aclass.ENUM_VALUE
						case Parser::IdentifierNode::REF_ENUM_VALUE: {
							int64_t value = 0;
							if (_id.ref_base == Parser::IdentifierNode::BASE_LOCAL) value = _id._enum_value->value;
							else value = _id._enum_value_info->get_value();
							ptr<Parser::ConstValueNode> cv = new_node<Parser::ConstValueNode>(value);
							cv->pos = member->pos; p_expr = cv;
						} break;

							// Aclass.a_function;
						case Parser::IdentifierNode::REF_FUNCTION: {
							_id.pos = member->pos;

							if (_base_class_ref != _THIS && !_id._func->is_static) {
								throw ANALYZER_ERROR(Error::ATTRIBUTE_ERROR, String::format("non-static attribute \"%s\" cannot be access with a class reference \"%s\".", member->name.c_str(), base->name.c_str()), member->pos);
							}

							if (_base_class_ref == _THIS) {
								p_expr = newptr<Parser::IdentifierNode>(_id);
							} else {
								index->member = newptr<Parser::IdentifierNode>(_id);
								index->_ref_reduced = true;
							}
						} break;

						case Parser::IdentifierNode::REF_CARBON_CLASS:
						case Parser::IdentifierNode::REF_NATIVE_CLASS:
						case Parser::IdentifierNode::REF_EXTERN:
							THROW_BUG("can't be");
					}
				} break;

					// File.prop;
				case Parser::IdentifierNode::REF_NATIVE_CLASS: {
					ASSERT(NativeClasses::singleton()->is_class_registered(base->name));
					BindData* bd = NativeClasses::singleton()->find_bind_data(base->name, member->name).get();
					if (!bd) throw ANALYZER_ERROR(Error::NAME_ERROR, String::format("attribute \"%s\" doesn't exists on base %s.", member->name.c_str(), base->name.c_str()), member->pos);
					switch (bd->get_type()) {
						case BindData::METHOD:
						case BindData::STATIC_FUNC:
						case BindData::MEMBER_VAR:
						case BindData::STATIC_VAR:
						case BindData::ENUM:
							break;

							// NativeClass.CONT_VALUE
						case BindData::STATIC_CONST: {
							ptr<Parser::ConstValueNode> cv = new_node<Parser::ConstValueNode>(((ConstantBind*)bd)->get());
							cv->pos = member->pos; p_expr = cv;
						} break;

							// File.READ
						case BindData::ENUM_VALUE: {
							ptr<Parser::ConstValueNode> cv = new_node<Parser::ConstValueNode>(((EnumValueBind*)bd)->get());
							cv->pos = member->pos; p_expr = cv;
						} break;
					}

				} break;

					// f.attrib;
				case Parser::IdentifierNode::REF_FUNCTION: {
					// at runtime it'll return a ptr<CarbonFunction> reference.
					// TODO: check attribute.
				} break;

					// extern_class.prop;
				case Parser::IdentifierNode::REF_EXTERN: {
					// TODO: check attrib
				} break;
			}
		}

		default:
			break;
			// RUNTIME.
	}
}

} // namespace carbon

/******************************************************************************************************************/
/*                                         REDUCE CALL                                                            */
/******************************************************************************************************************/

namespace carbon {

#define GET_ARGS(m_nodes)                                                             \
	stdvec<var*> args;                                                                \
	for (int i = 0; i < (int)m_nodes.size(); i++) {                                   \
	    args.push_back(&ptrcast<Parser::ConstValueNode>(m_nodes[i])->value);          \
	}

#define SET_EXPR_CONST_NODE(m_var, m_pos)                                             \
do {                                                                                  \
	ptr<Parser::ConstValueNode> cv = new_node<Parser::ConstValueNode>(m_var);         \
	cv->pos = m_pos, p_expr = cv;                                                     \
} while (false)


void Analyzer::_reduce_call(ptr<Parser::Node>& p_expr) {
	ASSERT(p_expr->type == Parser::Node::Type::CALL);

	ptr<Parser::CallNode> call = ptrcast<Parser::CallNode>(p_expr);

	// reduce arguments.
	bool all_const = true;
	for (int i = 0; i < (int)call->args.size(); i++) {
		_reduce_expression(call->args[i]);
		if (call->args[i]->type != Parser::Node::Type::CONST_VALUE) {
			all_const = false;
		}
	}

	// reduce base.
	if (call->base->type == Parser::Node::Type::BUILTIN_FUNCTION || call->base->type == Parser::Node::Type::BUILTIN_TYPE) {
		// don't_reduce_anything();
	} else {
		if (call->base->type == Parser::Node::Type::UNKNOWN) {
			_reduce_expression(call->method);
			if (call->method->type == Parser::Node::Type::CONST_VALUE)
				throw ANALYZER_ERROR(Error::TYPE_ERROR, String::format("constant value is not callable."), call->pos);
			ASSERT(call->method->type == Parser::Node::Type::IDENTIFIER);
		} else {
			_reduce_expression(call->base);
		}
	}

	switch (call->base->type) {

		// print.a_method(); print(); call base is builtin function.
		case Parser::Node::Type::BUILTIN_FUNCTION: {

			if (call->method == nullptr) { // print();
				if (all_const) {
					ptr<Parser::BuiltinFunctionNode> bf = ptrcast<Parser::BuiltinFunctionNode>(call->base);
					if (BuiltinFunctions::can_const_fold(bf->func)) {
						GET_ARGS(call->args);
						if (BuiltinFunctions::is_compiletime(bf->func)) {
							var ret = _call_compiletime_func(bf.get(), args);
							SET_EXPR_CONST_NODE(ret, call->pos);
						} else {
							try {
								var ret;
								BuiltinFunctions::call(bf->func, args, ret);
								SET_EXPR_CONST_NODE(ret, call->pos);
							} catch (Error& err) {
								throw ANALYZER_ERROR(err.get_type(), err.what(), call->pos);
							}
						}
					}
				}
			} else { // print.a_method();
				// TODO: check method exists, if (all_const) reduce();
			}

		} break;

			// String(); String.format(...); method call on base built in type
		case Parser::Node::Type::BUILTIN_TYPE: {
			if (call->method == nullptr) { // String(...); constructor.
				Parser::BuiltinTypeNode* bt = static_cast<Parser::BuiltinTypeNode*>(call->base.get());
				if (all_const && BuiltinTypes::can_construct_compile_time(bt->builtin_type)) {
					try {
						GET_ARGS(call->args);
						var ret = BuiltinTypes::construct(bt->builtin_type, args);
						SET_EXPR_CONST_NODE(ret, call->pos);
					} catch (Error& err) {
						throw ANALYZER_ERROR(err.get_type(), err.what(), call->base->pos);
					}
				}
			} else { // String.format(); static func call.
				// TODO: check if exists, reduce if compile time callable.
			}

		} break;

			// method call on base const value.
		case Parser::Node::Type::CONST_VALUE: {
			if (all_const) {
				try {
					ASSERT(call->method->type == Parser::Node::Type::IDENTIFIER);
					GET_ARGS(call->args); // 0 : const value, 1: name, ... args.
					var ret = ptrcast<Parser::ConstValueNode>(call->base)->value.call_method(ptrcast<Parser::IdentifierNode>(call->method)->name, args);
					SET_EXPR_CONST_NODE(ret, call->pos);
				} catch (const Error& err) {
					throw ANALYZER_ERROR(err.get_type(), err.what(), call->method->pos);
				}
			}
		} break;

			// call base is unknown. search method from this to super, or static function.
		case Parser::Node::Type::UNKNOWN: {

			Parser::IdentifierNode* id = ptrcast<Parser::IdentifierNode>(call->method).get();
			switch (id->ref) {

				// a_var(); call `__call` method on the variable.
				case Parser::IdentifierNode::REF_PARAMETER:
				case Parser::IdentifierNode::REF_LOCAL_VAR:
				case Parser::IdentifierNode::REF_STATIC_VAR:
				case Parser::IdentifierNode::REF_MEMBER_VAR: {
					call->base = call->method; // param(args...); -> will call param.__call(args...);
					call->method = nullptr;
				} break;

					// f(); calling a local carbon function.
				case Parser::IdentifierNode::REF_FUNCTION: {

					bool is_illegal_call = false;
					int argc = 0;
					int argc_default = 0;
					int argc_given = (int)call->args.size();

					switch (id->ref_base) {
						case Parser::IdentifierNode::BASE_UNKNOWN:
							THROW_BUG("can't be"); // call base is empty.
						case Parser::IdentifierNode::BASE_EXTERN:
							THROW_BUG("TODO:");
						case Parser::IdentifierNode::BASE_NATIVE: {
							// is_illegal_call = //TODO: impl
							argc = id->_method_info->get_arg_count();
							argc_default = id->_method_info->get_default_arg_count();
						} break;
						case Parser::IdentifierNode::BASE_LOCAL: {
							// TODO: this logic may be false.
							is_illegal_call = parser->parser_context.current_class && !id->_func->is_static;
							argc = (int)id->_func->args.size();
							argc_default = (int)id->_func->default_args.size();
						} break;
					}

					if (is_illegal_call) { // can't call a non-static function.
						if ((parser->parser_context.current_func && parser->parser_context.current_func->is_static) ||
							(parser->parser_context.current_var && parser->parser_context.current_var->is_static)) {
							throw ANALYZER_ERROR(Error::ATTRIBUTE_ERROR, String::format("can't access non-static attribute \"%s\" statically", id->name.c_str()), id->pos);
						}
					}

					_check_arg_count(argc, argc_default, argc_given, call->pos);

				} break;

					// Aclass(...); calling carbon class constructor.
				case Parser::IdentifierNode::REF_CARBON_CLASS: {
					if (id->_class->constructor) {
						int argc = (int)id->_class->constructor->args.size();
						int argc_default = (int)id->_class->constructor->default_args.size();
						int argc_given = (int)call->args.size();
						if (argc_given + argc_default < argc) {
							if (argc_default == 0) throw ANALYZER_ERROR(Error::INVALID_ARG_COUNT, String::format("expected exactly %i argument(s).", argc), id->pos);
							else throw ANALYZER_ERROR(Error::INVALID_ARG_COUNT, String::format("expected at least %i argument(s).", argc - argc_default), id->pos);
						} else if (argc_given > argc) {
							if (argc_default == 0) throw ANALYZER_ERROR(Error::INVALID_ARG_COUNT, String::format("expected exactly %i argument(s).", argc), id->pos);
							else throw ANALYZER_ERROR(Error::INVALID_ARG_COUNT, String::format("expected minimum of %i argument(s) and maximum of %i argument(s).", argc - argc_default, argc), id->pos);
						}
					} else {
						if (call->args.size() != 0)
							throw ANALYZER_ERROR(Error::INVALID_ARG_COUNT, "default constructor takes exactly 0 argument.", id->pos);
					}
				} break;

					// File(...); calling a native class constructor.
				case Parser::IdentifierNode::REF_NATIVE_CLASS: {
					ASSERT(NativeClasses::singleton()->is_class_registered(id->name));
					const StaticFuncBind* initializer = NativeClasses::singleton()->get_constructor(id->name);
					if (initializer) {
						// check arg counts.
						int argc = initializer->get_method_info()->get_arg_count() - 1; // -1 for self argument.
						int argc_default = initializer->get_method_info()->get_default_arg_count();
						int argc_given = (int)call->args.size();
						if (argc_given + argc_default < argc) {
							if (argc_default == 0) throw ANALYZER_ERROR(Error::INVALID_ARG_COUNT, String::format("expected exactly %i argument(s).", argc), id->pos);
							else throw ANALYZER_ERROR(Error::INVALID_ARG_COUNT, String::format("expected at least %i argument(s).", argc - argc_default), id->pos);
						} else if (argc_given > argc) {
							if (argc_default == 0) throw ANALYZER_ERROR(Error::INVALID_ARG_COUNT, String::format("expected exactly %i argument(s).", argc), id->pos);
							else throw ANALYZER_ERROR(Error::INVALID_ARG_COUNT, String::format("expected minimum of %i argument(s) and maximum of %i argument(s).", argc - argc_default, argc), id->pos);
						}
						// check arg types.
						const stdvec<VarTypeInfo>& arg_types = initializer->get_method_info()->get_arg_types();
						for (int i = 0; i < argc_given; i++) {
							if (call->args[i]->type == Parser::Node::Type::CONST_VALUE) {
								var value = ptrcast<Parser::ConstValueNode>(call->args[i])->value;
								if (!var::is_compatible(value.get_type(), arg_types[i + 1].type)) // +1 for skip self argument.
									throw ANALYZER_ERROR(Error::TYPE_ERROR, String::format("expected type \"%s\" at argument %i.", var::get_type_name_s(arg_types[i + 1].type), i), call->args[i]->pos);
							}
						}
					} else {
						if (call->args.size() != 0)
							throw ANALYZER_ERROR(Error::INVALID_ARG_COUNT, "default constructor takes exactly 0 argument.", id->pos);
					}
				} break;

					// invalid callables.
					//case Parser::IdentifierNode::REF_ENUM_NAME:
					//case Parser::IdentifierNode::REF_ENUM_VALUE:
					//case Parser::IdentifierNode::REF_FILE:
					//case Parser::IdentifierNode::REF_LOCAL_CONST:
					//case Parser::IdentifierNode::REF_MEMBER_CONST:
				default: {
					throw ANALYZER_ERROR(Error::TYPE_ERROR, String::format("attribute \"%s\" is not callable.", id->name.c_str()), id->pos);
				}
			}

		} break;

			// this.method(); super.method();
		case Parser::Node::Type::THIS:
		case Parser::Node::Type::SUPER: {
			const Parser::ClassNode* curr_class = parser->parser_context.current_class;

			if (call->method == nullptr) {
				if (call->base->type == Parser::Node::Type::THIS) { // this(); = __call() = operator ()()
					const Parser::FunctionNode* func = nullptr;
					for (const ptr<Parser::FunctionNode>& fn : curr_class->functions) {
						if (fn->name == GlobalStrings::__call) {
							func = fn.get(); break;
						}
					}
					if (func == nullptr) throw ANALYZER_ERROR(Error::NOT_IMPLEMENTED, String::format("operator method __call not implemented on base %s", curr_class->name.c_str()), call->pos);
					_check_arg_count((int)func->args.size(), (int)func->default_args.size(), (int)call->args.size(), call->pos);
				} else { // super();
					if (parser->parser_context.current_class == nullptr || parser->parser_context.current_class->base_type == Parser::ClassNode::NO_BASE ||
						(parser->parser_context.current_class->constructor != parser->parser_context.current_func))
						throw ANALYZER_ERROR(Error::SYNTAX_ERROR, "invalid super call.", call->pos);
					if ((parser->parser_context.current_statement_ind != 0) || (parser->parser_context.current_block->statements[0].get() != p_expr.get()))
						throw ANALYZER_ERROR(Error::SYNTAX_ERROR, "super call should be the first and stand-alone statement of a constructor.", call->pos);

					switch (curr_class->base_type) {
						case Parser::ClassNode::NO_BASE:
							THROW_BUG("it should be an analyzer error");
						case Parser::ClassNode::BASE_LOCAL: {
							const Parser::FunctionNode* base_constructor = curr_class->base_class->constructor;
							if (base_constructor == nullptr) _check_arg_count(0, 0, (int)call->args.size(), call->pos);
							else _check_arg_count((int)base_constructor->args.size(), (int)base_constructor->default_args.size(), (int)call->args.size(), call->pos);
						} break;
						case Parser::ClassNode::BASE_NATIVE: {
							const StaticFuncBind* base_constructor = NativeClasses::singleton()->get_constructor(curr_class->base_class_name);
							if (base_constructor == nullptr) _check_arg_count(0, 0, (int)call->args.size(), call->pos);
							else _check_arg_count(base_constructor->get_argc(), base_constructor->get_method_info()->get_default_arg_count(), (int)call->args.size(), call->pos);
						} break;
						case Parser::ClassNode::BASE_EXTERN: {
							const Function* base_constructor = curr_class->base_binary->get_constructor();
							if (base_constructor == nullptr) _check_arg_count(0, 0, (int)call->args.size(), call->pos);
							else _check_arg_count(base_constructor->get_arg_count(), (int)base_constructor->get_default_args().size(), (int)call->args.size(), call->pos);
						} break;
					}
				}

			} else {
				const String& method_name = ptrcast<Parser::IdentifierNode>(call->method)->name;
				if (call->base->type == Parser::Node::Type::THIS) {
					// this.method();
					Parser::IdentifierNode _id = _find_member(curr_class, method_name);
					switch (_id.ref) {
						case Parser::IdentifierNode::REF_UNKNOWN:
							throw ANALYZER_ERROR(Error::ATTRIBUTE_ERROR, String::format("attribute \"%s\" isn't exists in base \"%s\".", method_name.c_str(), curr_class->name.c_str()), call->pos);

							// this.a_var();
						case Parser::IdentifierNode::REF_MEMBER_VAR: {
							call->base = call->method;
							call->method = nullptr;
						} break;

							// this.CONST(); inavlid callables.
						case Parser::IdentifierNode::REF_MEMBER_CONST:
						case Parser::IdentifierNode::REF_ENUM_NAME:
						case Parser::IdentifierNode::REF_ENUM_VALUE:
							throw ANALYZER_ERROR(Error::TYPE_ERROR, String::format("constant value is not callable.", method_name.c_str()), call->pos);

							// this.f(); // function call on this.
						case Parser::IdentifierNode::REF_FUNCTION: {
							if (parser->parser_context.current_func->is_static && !_id._func->is_static) {
								throw ANALYZER_ERROR(Error::ATTRIBUTE_ERROR, String::format("can't access non-static attribute \"%s\" statically", _id.name.c_str()), _id.pos);
							}

							int argc = (int)_id._func->args.size();
							int argc_default = (int)_id._func->default_args.size();
							_check_arg_count(argc, argc_default, (int)call->args.size(), call->pos);

						} break;

						case Parser::IdentifierNode::REF_PARAMETER:
						case Parser::IdentifierNode::REF_LOCAL_VAR:
						case Parser::IdentifierNode::REF_LOCAL_CONST:
						case Parser::IdentifierNode::REF_CARBON_CLASS:
						case Parser::IdentifierNode::REF_NATIVE_CLASS:
						case Parser::IdentifierNode::REF_EXTERN:
							THROW_BUG("can't be");
					}
				} else { // super.method();


					switch (curr_class->base_type) {
						case Parser::ClassNode::NO_BASE: THROW_BUG("it should be an analyzer error");


						case Parser::ClassNode::BASE_LOCAL: {
							Parser::IdentifierNode _id = _find_member(curr_class->base_class, method_name);
							switch (_id.ref) {
								case Parser::IdentifierNode::REF_UNKNOWN:
									throw ANALYZER_ERROR(Error::ATTRIBUTE_ERROR, String::format("attribute \"%s\" isn't exists in base \"%s\".", method_name.c_str(), curr_class->name.c_str()), call->pos);

									// super.a_var();
								case Parser::IdentifierNode::REF_MEMBER_VAR: {
									throw ANALYZER_ERROR(Error::ATTRIBUTE_ERROR, String::format("attribute \"%s\" cannot be access with with \"super\" use \"this\" instead.", method_name.c_str()), call->pos);
								} break;

									// super.CONST(); inavlid callables.
								case Parser::IdentifierNode::REF_MEMBER_CONST:
								case Parser::IdentifierNode::REF_ENUM_NAME:
								case Parser::IdentifierNode::REF_ENUM_VALUE:
									throw ANALYZER_ERROR(Error::TYPE_ERROR, String::format("constant value is not callable.", method_name.c_str()), call->pos);

									// super.f(); // function call on super.
								case Parser::IdentifierNode::REF_FUNCTION: {
									if (parser->parser_context.current_func->is_static && !_id._func->is_static) {
										throw ANALYZER_ERROR(Error::ATTRIBUTE_ERROR, String::format("can't access non-static attribute \"%s\" statically", _id.name.c_str()), call->pos);
									}

									int argc = (int)_id._func->args.size();
									int argc_default = (int)_id._func->default_args.size();
									_check_arg_count(argc, argc_default, (int)call->args.size(), call->pos);

								} break;

								case Parser::IdentifierNode::REF_PARAMETER:
								case Parser::IdentifierNode::REF_LOCAL_VAR:
								case Parser::IdentifierNode::REF_LOCAL_CONST:
								case Parser::IdentifierNode::REF_CARBON_CLASS:
								case Parser::IdentifierNode::REF_NATIVE_CLASS:
								case Parser::IdentifierNode::REF_EXTERN:
									THROW_BUG("can't be");
							}
						} break;

							// super.method(); // super is native
						case Parser::ClassNode::BASE_NATIVE: {
							ptr<BindData> bd = NativeClasses::singleton()->find_bind_data(curr_class->base_class_name, method_name);
							if (bd == nullptr) throw ANALYZER_ERROR(Error::ATTRIBUTE_ERROR, String::format("attribute \"%s\" isn't exists in base \"%s\".", method_name.c_str(), curr_class->base_class_name.c_str()), call->pos);
							switch (bd->get_type()) {
								case BindData::METHOD: { // super.method();
									if (parser->parser_context.current_func->is_static) { // calling super method from static function.
										throw ANALYZER_ERROR(Error::ATTRIBUTE_ERROR, String::format("can't access non-static attribute \"%s\" statically", method_name.c_str()), call->pos);
									}
									const MethodInfo* mi = ptrcast<MethodBind>(bd)->get_method_info().get();
									_check_arg_count(mi->get_arg_count(), mi->get_default_arg_count(), (int)call->args.size(), call->pos);
								} break;
								case BindData::STATIC_FUNC: { // super.sfunc();
									const MethodInfo* mi = ptrcast<StaticFuncBind>(bd)->get_method_info().get();
									_check_arg_count(mi->get_arg_count(), mi->get_default_arg_count(), (int)call->args.size(), call->pos);
								} break;
								case BindData::MEMBER_VAR: { // super.a_var();
									throw ANALYZER_ERROR(Error::ATTRIBUTE_ERROR, String::format("attribute \"%s\" cannot be access with with \"super\" use \"this\" instead.", method_name.c_str()), call->pos);
								} break;
								case BindData::STATIC_VAR:
									break; // OK
								case BindData::STATIC_CONST:
								case BindData::ENUM:
								case BindData::ENUM_VALUE:
									throw ANALYZER_ERROR(Error::TYPE_ERROR, String::format("constant value is not callable.", method_name.c_str()), call->pos);
									break;
							}
						} break;

							// super.method() // super is extern
						case Parser::ClassNode::BASE_EXTERN: {
							const MemberInfo* info = curr_class->base_binary->get_member_info(method_name).get();
							if (info == nullptr) throw ANALYZER_ERROR(Error::ATTRIBUTE_ERROR, String::format("attribute \"%s\" isn't exists in base \"%s\".", method_name.c_str(), curr_class->base_class_name.c_str()), call->pos);
							switch (info->get_type()) {
								case MemberInfo::METHOD: {
									const MethodInfo* mi = static_cast<const MethodInfo*>(info);
									_check_arg_count(mi->get_arg_count(), mi->get_default_arg_count(), (int)call->args.size(), call->pos);
								} break;
								case MemberInfo::PROPERTY: {
									const PropertyInfo* pi = static_cast<const PropertyInfo*>(info);
									if (!pi->is_static()) throw ANALYZER_ERROR(Error::ATTRIBUTE_ERROR, String::format("attribute \"%s\" cannot be access with with \"super\" use \"this\" instead.", method_name.c_str()), call->pos);
								} break;
								case MemberInfo::ENUM:
								case MemberInfo::ENUM_VALUE:
									throw ANALYZER_ERROR(Error::TYPE_ERROR, String::format("constant value is not callable.", method_name.c_str()), call->pos);
								case MemberInfo::CLASS:
									THROW_BUG("can't be");
							}
						} break;
					}
				}
			}

		} break;

			// get_func()(); arr[0](); (a + b)(); base().method(); [o1, o2][1].method(); (x + y).method();
		case Parser::Node::Type::CALL:
		case Parser::Node::Type::INDEX:
		case Parser::Node::Type::MAPPED_INDEX:
		case Parser::Node::Type::OPERATOR:
			ASSERT(call->method == nullptr || call->method->type == Parser::Node::Type::IDENTIFIER);
			break;

		case Parser::Node::Type::ARRAY: // TODO: the method could be validated.
		case Parser::Node::Type::MAP:   // TODO: the method could be validated.
			ASSERT(call->method->type == Parser::Node::Type::IDENTIFIER);
			break;


			// idf.method(); method call on base with identifier id.
		case Parser::Node::Type::IDENTIFIER: {
			ASSERT(call->method->type == Parser::Node::Type::IDENTIFIER);
			Parser::IdentifierNode* base = ptrcast<Parser::IdentifierNode>(call->base).get();
			Parser::IdentifierNode* id = ptrcast<Parser::IdentifierNode>(call->method).get();

			switch (base->ref) {

				// IF IDENTIFIER IS UNKNOWN IT'S ALREADY A NAME ERROR BY NOW.
				case Parser::IdentifierNode::REF_UNKNOWN: THROW_BUG("can't be");

					// p_param.method(); a_var.method(); a_member.method();
				case Parser::IdentifierNode::REF_PARAMETER:
				case Parser::IdentifierNode::REF_LOCAL_VAR:
				case Parser::IdentifierNode::REF_STATIC_VAR:
				case Parser::IdentifierNode::REF_MEMBER_VAR: {
				} break;

					// IF AN IDENTIFIER IS REFERENCE TO A CONSTANT IT'LL BE `ConstValueNode` BY NOW.
				case Parser::IdentifierNode::REF_LOCAL_CONST:
				case Parser::IdentifierNode::REF_MEMBER_CONST: {
					THROW_BUG("can't be.");
				} break;

					// Aclass.id();
				case Parser::IdentifierNode::REF_CARBON_CLASS: {

					Parser::IdentifierNode _id = _find_member(base->_class, id->name);
					_id.pos = id->pos; id = &_id;
					switch (_id.ref) {
						case Parser::IdentifierNode::REF_UNKNOWN:
							throw ANALYZER_ERROR(Error::NAME_ERROR, String::format("attribute \"%s\" doesn't exists on base %s", id->name.c_str(), base->_class->name.c_str()), id->pos);

							// Aclass.a_var();
						case Parser::IdentifierNode::REF_MEMBER_VAR: {
							bool _is_member_static = false;
							switch (id->ref_base) {
								case Parser::IdentifierNode::BASE_UNKNOWN: ASSERT(false && "I must forgotten here");
								case Parser::IdentifierNode::BASE_LOCAL:
									_is_member_static = id->_var->is_static;
									break;
								case Parser::IdentifierNode::BASE_NATIVE:
								case Parser::IdentifierNode::BASE_EXTERN:
									_is_member_static = id->_prop_info->is_static();
									break;
							}

							if (_is_member_static) {
								break; // Class.var(args...);
							} else {
								throw ANALYZER_ERROR(Error::ATTRIBUTE_ERROR, String::format("can't access non-static attribute \"%s\" statically", id->name.c_str()), id->pos);
							}
						} break;

							// Aclass.CONST(args...);
						case Parser::IdentifierNode::REF_LOCAL_CONST:
						case Parser::IdentifierNode::REF_MEMBER_CONST: {
							throw ANALYZER_ERROR(Error::TYPE_ERROR, String::format("constant value (\"%s.%s()\") is not callable.", base->name.c_str(), id->name.c_str()), id->pos);
						} break;

							// Aclass.Enum();
						case Parser::IdentifierNode::REF_ENUM_NAME: throw ANALYZER_ERROR(Error::TYPE_ERROR, String::format("enums (\"%s.%s()\") are not callable.", base->name.c_str(), id->name.c_str()), id->pos);
						case Parser::IdentifierNode::REF_ENUM_VALUE: throw ANALYZER_ERROR(Error::TYPE_ERROR, String::format("enum value (\"%s.%s()\") is not callable.", base->name.c_str(), id->name.c_str()), id->pos);

						case Parser::IdentifierNode::REF_FUNCTION: {

							bool _is_func_static = false;
							switch (id->ref_base) {
								case Parser::IdentifierNode::BASE_UNKNOWN: ASSERT(false && "I must forgotten here");
								case Parser::IdentifierNode::BASE_LOCAL:
									_is_func_static = _id._func->is_static;
									break;
								case Parser::IdentifierNode::BASE_NATIVE:
								case Parser::IdentifierNode::BASE_EXTERN:
									_is_func_static = id->_method_info->is_static();
									break;
							}

							if (_is_func_static) {
								break; // Class.static_func(args...);
							} else {
								throw ANALYZER_ERROR(Error::ATTRIBUTE_ERROR, String::format("can't call non-static method\"%s\" statically", id->name.c_str()), id->pos);
							}
						} break;

							// Aclass.Lib();
						case Parser::IdentifierNode::REF_EXTERN: {
							throw ANALYZER_ERROR(Error::TYPE_ERROR, String::format("external libraries (\"%s.%s()\") are not callable.", base->name.c_str(), id->name.c_str()), id->pos);
						} break;
					}
				} break;

				// File.method(); base is a native class.
				case Parser::IdentifierNode::REF_NATIVE_CLASS: {

					BindData* bd = NativeClasses::singleton()->find_bind_data(base->name, id->name).get();
					if (!bd) throw ANALYZER_ERROR(Error::ATTRIBUTE_ERROR, String::format("attribute \"%s\" does not exists on base %s.", id->name.c_str(), base->name.c_str()), id->pos);
					switch (bd->get_type()) {
						case BindData::STATIC_FUNC: {

							const MemberInfo* memi = bd->get_member_info().get();
							if (memi->get_type() != MemberInfo::METHOD) THROW_BUG("native member reference mismatch.");
							const MethodInfo* mi = (const MethodInfo*)memi;
							if (!mi->is_static()) THROW_BUG("native method reference mismatch.");

							int argc_given = (int)call->args.size();
							int argc = mi->get_arg_count(), argc_default = mi->get_default_arg_count();
							if (argc_given + argc_default < argc) {
								if (argc_default == 0) throw ANALYZER_ERROR(Error::INVALID_ARG_COUNT, String::format("expected exactly %i argument(s).", argc), id->pos);
								else throw ANALYZER_ERROR(Error::INVALID_ARG_COUNT, String::format("expected at least %i argument(s).", argc - argc_default), id->pos);
							} else if (argc_given > argc) {
								if (argc_default == 0) throw ANALYZER_ERROR(Error::INVALID_ARG_COUNT, String::format("expected exactly %i argument(s).", argc), id->pos);
								else throw ANALYZER_ERROR(Error::INVALID_ARG_COUNT, String::format("expected minimum of %i argument(s) and maximum of %i argument(s).", argc - argc_default, argc), id->pos);
							}

							for (int i = 0; i < (int)call->args.size(); i++) {
								if (call->args[i]->type == Parser::Node::Type::CONST_VALUE) {
									if (!var::is_compatible(mi->get_arg_types()[i].type, ptrcast<Parser::ConstValueNode>(call->args[i])->value.get_type())) {
										throw ANALYZER_ERROR(Error::TYPE_ERROR, String::format("expected type \"%s\" at argument %i.", var::get_type_name_s(mi->get_arg_types()[i].type), i), call->args[i]->pos);
									}
								}
							}

							// TODO: check if the static function is ok to call at compile time
							//       ex: read a file at a location, print something... are runtime.
							//if (all_const) {
							//	try {
							//		GET_ARGS(call->r_args);
							//		var ret = ptrcast<StaticFuncBind>(bd)->call(args);
							//		SET_EXPR_CONST_NODE(ret, call->pos);
							//	} catch (Error& err) {
							//		throw ANALYZER_ERROR(err.get_type(), err.what(), call->pos);
							//	}
							//}

						} break;
						case BindData::STATIC_VAR: break; // calls the "__call" at runtime.
						case BindData::STATIC_CONST: throw ANALYZER_ERROR(Error::TYPE_ERROR, String::format("constant value \"%s.%s()\" is not callable.", base->name.c_str(), id->name.c_str()), id->pos);
						case BindData::METHOD: throw ANALYZER_ERROR(Error::TYPE_ERROR, String::format("can't call non-static method \"%s.%s()\" statically.", base->name.c_str(), id->name.c_str()), id->pos);
						case BindData::MEMBER_VAR:  throw ANALYZER_ERROR(Error::TYPE_ERROR, String::format("can't call non-static member \"%s.%s()\" statically.", base->name.c_str(), id->name.c_str()), id->pos);
						case BindData::ENUM:  throw ANALYZER_ERROR(Error::TYPE_ERROR, String::format("enums (\"%s.%s()\") are not callable.", base->name.c_str(), id->name.c_str()), id->pos);
						case BindData::ENUM_VALUE: throw ANALYZER_ERROR(Error::TYPE_ERROR, String::format("enum value (\"%s.%s()\") are not callable.", base->name.c_str(), id->name.c_str()), id->pos);
					}

				} break;

					// fn.get_default_args(), fn.get_name(), ...
				case Parser::IdentifierNode::REF_FUNCTION: {
					// TODO: check if method exists and args.
				} break;

				case Parser::IdentifierNode::REF_EXTERN: {
					// TODO: check if function exists and check args.
				} break;

				case Parser::IdentifierNode::REF_ENUM_NAME: {
					// TODO: check if method exists
				} break;

					// TODO: EnumType.get_value_count();
					//case Parser::IdentifierNode::REF_ENUM_VALUE:
				default: {
					throw ANALYZER_ERROR(Error::TYPE_ERROR, String::format("attribute \"%s\" doesn't support method calls.", base->name.c_str()), base->pos);
				}
			}

		} break;

		default: {
			THROW_BUG("can't reach here.");
		}

	}
#undef SET_EXPR_CONST_NODE
#undef GET_ARGS
}

} // namespace carbon

#endif //_FILE_SRC_COMPILER_ANALYZER_CPP_

#ifndef _FILE_SRC_COMPILER_BUILTIN_CPP_


/******************************************************************************************************************/
/*                                          BUILTIN TYPES                                                         */
/******************************************************************************************************************/

namespace carbon {

String BuiltinTypes::get_type_name(Type p_type) {
	return _type_list[p_type];
}

BuiltinTypes::Type BuiltinTypes::get_type_type(const String& p_type) {
	for (const std::pair<Type, String>& pair : _type_list) {
		if (pair.second == p_type) {
			return pair.first;
		}
	}
	return BuiltinTypes::UNKNOWN;
}

var::Type BuiltinTypes::get_var_type(Type p_type) {
	switch (p_type) {
		case UNKNOWN:
		case _NULL:  return var::_NULL;
		case BOOL:   return var::BOOL;
		case INT:    return var::INT;
		case FLOAT:  return var::FLOAT;
		case STR: // [[FALLTHROUGH]]
		case STRING: return var::STRING;
		case ARRAY:  return var::ARRAY;
		case MAP:    return var::MAP;
	}
	THROW_BUG("can't reach here.");
}
MISSED_ENUM_CHECK(BuiltinTypes::Type::_TYPE_MAX_, 9);

bool BuiltinTypes::can_construct_compile_time(Type p_type) {
	switch (p_type) {
		case UNKNOWN:
		case _NULL:
		case BOOL:
		case INT:
		case FLOAT:
		case STRING:
		case STR:
			return true;
		case ARRAY:
		case MAP:
			return false;
	}
	THROW_BUG("can't reach here.");
}
MISSED_ENUM_CHECK(BuiltinTypes::Type::_TYPE_MAX_, 9);

var BuiltinTypes::construct(Type p_type, const stdvec<var*>& p_args) {
	switch (p_type) {
		case _NULL:
			THROW_ERROR(Error::OPERATOR_NOT_SUPPORTED, "can't construct null instance.");
		case BOOL:
			if (p_args.size() != 1) THROW_ERROR(Error::INVALID_ARG_COUNT, "expected exactly 1 argument.");
			return p_args[0]->operator bool();
		case INT:
			if (p_args.size() != 1) THROW_ERROR(Error::INVALID_ARG_COUNT, "expected exactly 1 argument.");
			switch (p_args[0]->get_type()) {
				case var::INT:
				case var::FLOAT:
					return p_args[0]->operator int64_t();
				case  var::STRING:
					return p_args[0]->operator String().to_int();
				default: {
					THROW_ERROR(Error::TYPE_ERROR, String::format("cannot construct integer from type %s", p_args[0]->get_type_name().c_str()));
				}
			}
		case FLOAT:
			if (p_args.size() != 1) THROW_ERROR(Error::INVALID_ARG_COUNT, "expected exactly 1 argument.");
			switch (p_args[0]->get_type()) {
				case var::INT:
				case var::FLOAT:
					return p_args[0]->operator double();
				case  var::STRING:
					return p_args[0]->operator String().to_float();
				default: {
					THROW_ERROR(Error::TYPE_ERROR, String::format("cannot construct float from type %s", p_args[0]->get_type_name().c_str()));
				}
			}

		case STR: // [[FALLTHROUGH]]
		case STRING: {
			if (p_args.size() >= 2) THROW_ERROR(Error::INVALID_ARG_COUNT, "expected at most 1 argument."); // TODO: what if multiple args??
			if (p_args.size() == 0) return String();
			return p_args[0]->to_string();

		} break;
		case ARRAY: {
			Array ret;
			for (size_t i = 0; i < p_args.size(); i++) {
				ret.push_back(*p_args[i]);
			}
			return ret;
		} break;

		case MAP: {
			if (p_args.size() != 0) THROW_ERROR(Error::INVALID_ARG_COUNT, "Expected exactly 0 argument.");
			return Map();
		}
		default: {
			ASSERT(false); // TODO: throw internal bug.
		}
	}

	MISSED_ENUM_CHECK(BuiltinTypes::Type::_TYPE_MAX_, 9);
	THROW_BUG("can't reach here");
}

} // namespace carbon

/******************************************************************************************************************/
/*                                          BUILTIN FUNCTIONS                                                     */
/******************************************************************************************************************/

namespace carbon {

String BuiltinFunctions::get_func_name(BuiltinFunctions::Type p_func) {
	return _func_list[p_func];
}

BuiltinFunctions::Type BuiltinFunctions::get_func_type(const String& p_func) {
	for (const std::pair<Type, String>& pair : _func_list) {
		if (pair.second == p_func) {
			return pair.first;
		}
	}
	return BuiltinFunctions::UNKNOWN;
}

int BuiltinFunctions::get_arg_count(BuiltinFunctions::Type p_func) {
	switch (p_func) {
		case Type::__ASSERT:
			return 1;
		case Type::__FUNC:
		case Type::__LINE:
		case Type::__FILE:
		case Type::PRINT:
		case Type::PRINTLN:
		case Type::INPUT:
			return -1;

		case Type::HEX:
		case Type::BIN:
			return 1;

		case Type::MATH_MAX:
		case Type::MATH_MIN:
			return -1;
		case Type::MATH_POW:
			return 2;
	}
	return 0;
	MISSED_ENUM_CHECK(BuiltinFunctions::Type::_FUNC_MAX_, 13);
}

bool BuiltinFunctions::can_const_fold(Type p_func) {
	switch (p_func) {
		case Type::PRINT:
		case Type::PRINTLN:
		case Type::INPUT:
			return false;
		default:
			return true;
	}
	MISSED_ENUM_CHECK(BuiltinFunctions::Type::_FUNC_MAX_, 13);
}

bool BuiltinFunctions::is_compiletime(Type p_func) {
	switch (p_func) {
		case __ASSERT:
		case __FUNC:
		case __LINE:
		case __FILE:
			return true;
		default:
			return false;
	}
	MISSED_ENUM_CHECK(BuiltinFunctions::Type::_FUNC_MAX_, 13);
}

// TODO: change this to return r_ret for consistancy.
void BuiltinFunctions::call(Type p_func, const stdvec<var*>& p_args, var& r_ret) {
	switch (p_func) {

		case Type::__ASSERT:
		case Type::__FUNC:
		case Type::__LINE:
		case Type::__FILE:
			THROW_BUG("the compile time func should be called by the analyzer.");

		case Type::PRINT: // [[FALLTHROUGH]]
		case Type::PRINTLN: {
			for (int i = 0; i < (int)p_args.size(); i++) {
				if (i > 0) Console::log(" ");
				Console::log(p_args[i]->to_string().c_str());
			}

			if (p_func == Type::PRINTLN) Console::log("\n");

		} break;

		case Type::INPUT: {
			if (p_args.size() >= 2) THROW_ERROR(Error::INVALID_ARG_COUNT, "Expected at most 1 argument.");
			if (p_args.size() == 1) Console::log(p_args[0]->operator String().c_str());
			r_ret = String(Console::getline());
		} break;

		case Type::HEX: {
			if (p_args.size() != 1) THROW_ERROR(Error::INVALID_ARG_COUNT, "Expected exctly 1 argument.");

			std::stringstream ss;
			ss << "0x" << std::hex << p_args[0]->operator int64_t();
			r_ret = String(ss.str());
		} break;

		case Type::BIN: {
			if (p_args.size() != 1) THROW_ERROR(Error::INVALID_ARG_COUNT, "Expected exctly 1 argument.");

			std::stringstream ss;
			ss << "0b" << std::bitset<sizeof(int64_t)>(p_args[0]->operator int64_t());
			r_ret = String(ss.str());
		} break;

		case Type::MATH_MAX: {
			if (p_args.size() <= 1) THROW_ERROR(Error::INVALID_ARG_COUNT, "Expected at least 2 arguments.");
			var min = *p_args[0];
			for (int i = 1; i < (int)p_args.size(); i++) {
				if (*p_args[i] < min) {
					min = *p_args[i];
				}
			}
			r_ret = min;
		} break;

		case Type::MATH_MIN: {
			if (p_args.size() <= 1) THROW_ERROR(Error::INVALID_ARG_COUNT, "expected at least 2 arguments.");
			var max = p_args[0];
			for (int i = 1; i < (int)p_args.size(); i++) {
				if (*p_args[i] > max) {
					max = *p_args[i];
				}
			}
			r_ret = max;
		} break;

		case Type::MATH_POW: {
			if (p_args.size() != 2) THROW_ERROR(Error::INVALID_ARG_COUNT, "Expected exactly 2 arguments.");
			if (p_args[0]->get_type() != var::INT && p_args[1]->get_type() != var::FLOAT)
				THROW_ERROR(Error::TYPE_ERROR, "expected a numeric value at argument 0.");
			if (p_args[1]->get_type() != var::INT && p_args[1]->get_type() != var::FLOAT)
				THROW_ERROR(Error::TYPE_ERROR, "expected a numeric value at argument 1.");
			r_ret = pow(p_args[0]->operator double(), p_args[1]->operator double());
		} break;

	}
	MISSED_ENUM_CHECK(BuiltinFunctions::Type::_FUNC_MAX_, 13);
}

} // namespace carbon

/******************************************************************************************************************/
/*                                          NATIVE REFERENCE                                                      */
/******************************************************************************************************************/

namespace carbon {

NativeClassRef::NativeClassRef(const String& p_native_class) {
	_name = p_native_class;
	if (!NativeClasses::singleton()->is_class_registered(_name))
		THROW_BUG(String::format("class \"%s\" not registered in native classes entries.", _name.c_str()));
}

var NativeClassRef::__call(stdvec<var*>& p_args) {
	return NativeClasses::singleton()->construct(_name, p_args);
}

// TODO:
var  NativeClassRef::call_method(const String& p_name, stdvec<var*>& p_args) { return var(); }
var  NativeClassRef::get_member(const String& p_name) { return var(); }
void NativeClassRef::set_member(const String& p_name, var& p_value) {}

//---------------------------------------------------------------------------

BuiltinFuncRef::BuiltinFuncRef() {}
BuiltinFuncRef::BuiltinFuncRef(BuiltinFunctions::Type p_type) : _type(p_type) {}

var BuiltinFuncRef::__call(stdvec<var*>& p_args) {
	var ret;
	BuiltinFunctions::call(_type, p_args, ret);
	return ret;
}

String BuiltinFuncRef::to_string() {
	return String("[builtin_function:") + BuiltinFunctions::get_func_name(_type) + "]";
}

//---------------------------------------------------------------------------

BuiltinTypeRef::BuiltinTypeRef() {}
BuiltinTypeRef::BuiltinTypeRef(BuiltinTypes::Type p_type) :_type(p_type) {}

var BuiltinTypeRef::__call(stdvec<var*>& p_args) {
	return BuiltinTypes::construct(_type, p_args);
}

String BuiltinTypeRef::to_string() {
	return String("[builtin_type:") + BuiltinTypes::get_type_name(_type) + "]";
}

}

#endif //_FILE_SRC_COMPILER_BUILTIN_CPP_

#ifndef _FILE_SRC_COMPILER_BYTECODE_CPP_



namespace carbon {

void Bytecode::initialize() {
	if (_static_initialized) return;
	_static_initialized = true;
	stdvec<var*> _args;
	if (is_class()) {
		_file->initialize();
		if (_static_initializer) VM::singleton()->call_function(_static_initializer.get(), this, nullptr, _args);
	} else {
		for (auto p : _externs) p.second->initialize();
		if (_static_initializer) VM::singleton()->call_function(_static_initializer.get(), this, nullptr, _args);
		for (auto p : _classes) p.second->initialize();
	}
}
bool Bytecode::is_class() const { return _is_class; }
const String& Bytecode::get_name() const { return _name; }

bool Bytecode::has_base() const { ASSERT(_is_class); return _has_base; }
bool Bytecode::is_base_native() const { ASSERT(_is_class); return _is_base_native; }
const ptr<Bytecode>& Bytecode::get_base_binary() const { ASSERT(_is_class); return _base; }
const String& Bytecode::get_base_native() const { return _base_native; }

int Bytecode::get_member_count() const { return get_member_offset() + (int)_members.size(); }
stdmap<String, ptr<Bytecode>>& Bytecode::get_classes() { ASSERT(!_is_class); return _classes; }
stdmap<String, ptr<Bytecode>>& Bytecode::get_externs() { ASSERT(!_is_class); return _externs; }
const stdmap<String, var>& Bytecode::get_constants() { return _constants; }
const stdmap<String, ptr<Function>>& Bytecode::get_functions() const { return _functions; }
stdmap<String, var>& Bytecode::get_static_vars() { return _static_vars; }

const ptr<Bytecode>&  Bytecode::get_file() const { ASSERT(_is_class); return _file; }
const Function* Bytecode::get_main() const { ASSERT(!_is_class); return _main; }
const Function* Bytecode::get_constructor() const { ASSERT(_is_class); return _constructor; }
const Function* Bytecode::get_member_initializer() const { ASSERT(_is_class); return _member_initializer.get(); }
const Function* Bytecode::get_static_initializer() const { return _static_initializer.get(); }

const String& Bytecode::get_global_name(uint32_t p_pos) {
	THROW_INVALID_INDEX(_global_names_array.size(), p_pos);
	return _global_names_array[p_pos];
}

var* Bytecode::get_global_const_value(uint32_t p_index) {
	THROW_INVALID_INDEX(_global_const_values.size(), p_index);
	return &_global_const_values[p_index];
}

#define _GET_OR_NULL(m_map, m_addr)         \
	auto it = m_map.find(p_name);			\
	if (it == m_map.end()) return nullptr;	\
	return m_addr it->second
ptr<Bytecode> Bytecode::get_class(const String& p_name) { ASSERT(!_is_class); _GET_OR_NULL(_classes, PLACE_HOLDER_MACRO); }
ptr<Bytecode> Bytecode::get_import(const String& p_name) { ASSERT(!_is_class); _GET_OR_NULL(_externs, PLACE_HOLDER_MACRO); }
ptr<Function> Bytecode::get_function(const String& p_name) { _GET_OR_NULL(_functions, PLACE_HOLDER_MACRO); }
var* Bytecode::get_static_var(const String& p_name) { _GET_OR_NULL(_static_vars, &); }
var Bytecode::get_constant(const String& p_name) { _GET_OR_NULL(_constants, PLACE_HOLDER_MACRO); }

var Bytecode::__call(stdvec<var*>& p_args) {
	if (!is_class())
		THROW_ERROR(Error::ATTRIBUTE_ERROR, "Bytecode module is not callable");

	ptr<Instance> instance = newptr<Instance>(shared_from_this());
	const Function* member_initializer = get_member_initializer();
	stdvec<var*> _args;
	if (member_initializer) VM::singleton()->call_function(member_initializer, this, instance, _args);
	const Function* constructor = get_constructor();
	if (constructor) VM::singleton()->call_function(constructor, this, instance, p_args);
	return instance;
}

var Bytecode::call_method(const String& p_method_name, stdvec<var*>& p_args) {

	auto it_f = _functions.find(p_method_name);
	if (it_f != _functions.end()) {
		if (!it_f->second->is_static()) throw "TODO: throw error here";
		return VM::singleton()->call_function(it_f->second.get(), this, nullptr, p_args);
	}

	auto it_sm = _static_vars.find(p_method_name);
	if (it_sm != _static_vars.end()) {
		return it_sm->second.__call(p_args);
	}

	if (_has_base) {
		if (_is_base_native) {
			ptr<BindData> bd = NativeClasses::singleton()->find_bind_data(_base_native, p_method_name);
			if (bd != nullptr) {
				switch (bd->get_type()) {
					case BindData::METHOD:
					case BindData::MEMBER_VAR:
						THROW_ERROR(Error::ATTRIBUTE_ERROR, "cannot call a non static attribute statically");

					case BindData::STATIC_FUNC: {
						const StaticFuncBind* sf = static_cast<const StaticFuncBind*>(bd.get());
						return sf->call(p_args);
					} break;

					case BindData::STATIC_VAR: {
						const StaticPropertyBind* pb = static_cast<const StaticPropertyBind*>(bd.get());
						return pb->get().__call(p_args);
					} break;

					case BindData::STATIC_CONST:
					case BindData::ENUM:
					case BindData::ENUM_VALUE:
						THROW_ERROR(Error::ATTRIBUTE_ERROR, String::format("%s isn't callable on base %s", p_method_name.c_str(), get_name().c_str()));
				}
			}
		} else {
			_base->call_method(p_method_name, p_args);
		}
	}

	if (!is_class()) {
		ptr<Bytecode> _class = get_class(p_method_name);
		if (_class != nullptr) {
			// TODO: abstract construction and everything.
			ptr<Instance> instance = newptr<Instance>(_class);
			const Function* member_initializer = _class->get_member_initializer();
			stdvec<var*> _args;
			if (member_initializer) VM::singleton()->call_function(member_initializer, _class.get(), instance, _args);
			const Function* constructor = _class->get_constructor();
			if (constructor) VM::singleton()->call_function(constructor, _class.get(), instance, p_args);
			return instance;
		}
	}
	THROW_ERROR(Error::ATTRIBUTE_ERROR, String::format("attribute \"%s\" isn't exists on base %s", p_method_name.c_str(), get_name().c_str()));
}

var Bytecode::get_member(const String& p_member_name) {

	var* _member = _get_member_var_ptr(p_member_name);
	if (_member != nullptr) return *_member;
	
	if (_base != nullptr) {
		return _base->get_member(p_member_name);
	} else {
		THROW_ERROR(Error::ATTRIBUTE_ERROR,
			String::format("%s %s has no member named \"%s\".", ((_is_class) ? "type" : "file at"), _name.c_str(), p_member_name.c_str()));
	}
}

void Bytecode::set_member(const String& p_member_name, var& p_value) {

	auto it = _static_vars.find(p_member_name);
	if (it != _static_vars.end()) {
		it->second = p_value;
		return;
	}

	// check other members for better error message.
	auto it_const = _constants.find(p_member_name);
	if (it_const != _constants.end()) THROW_ERROR(Error::ATTRIBUTE_ERROR,
		String::format("cannot assign to a constant value named \"%s\".", p_member_name.c_str()));

	auto it_en = _enums.find(p_member_name);
	if (it_en != _enums.end())  THROW_ERROR(Error::ATTRIBUTE_ERROR,
		String::format("cannot assign to an enum type named \"%s\".", p_member_name.c_str()));

	auto it_uen = _unnamed_enums.find(p_member_name);
	if (it_uen != _unnamed_enums.end())  THROW_ERROR(Error::ATTRIBUTE_ERROR,
		String::format("cannot assign to an enum value named \"%s\".", p_member_name.c_str()));

	auto it_fn = _functions.find(p_member_name);
	if (it_fn != _functions.end()) THROW_ERROR(Error::ATTRIBUTE_ERROR,
		String::format("cannot assign to a function pointer named \"%s\".", p_member_name.c_str()));

	if (_base != nullptr) {
		_base->set_member(p_member_name, p_value);
	} else {
		THROW_ERROR(Error::ATTRIBUTE_ERROR,
			String::format("%s %s has no member named \"%s\".", ((_is_class) ? "type" : "file at"), _name.c_str(), p_member_name.c_str()));
	}
}

var* Bytecode::_get_member_var_ptr(const String& p_member_name) {

	// if already constructed var* return it
	auto it = _member_vars.find(p_member_name);
	if (it != _member_vars.end()) { return &it->second; }

	// search and construct var*

	auto it_static = _static_vars.find(p_member_name);
	if (it_static != _static_vars.end()) return &it_static->second;

	auto it_const = _constants.find(p_member_name);
	if (it_const != _constants.end()) return &it_const->second; // { _member_vars[p_member_name] = it_const->second; return &_member_vars[p_member_name]; }

	auto it_en = _enums.find(p_member_name);
	if (it_en != _enums.end()) { _member_vars[p_member_name] = it_en->second; return &_member_vars[p_member_name]; }

	auto it_uen = _unnamed_enums.find(p_member_name);
	if (it_uen != _unnamed_enums.end()) { _member_vars[p_member_name] = it_uen->second; return &_member_vars[p_member_name]; }

	auto it_fn = _functions.find(p_member_name);
	if (it_fn != _functions.end()) { _member_vars[p_member_name] = it_fn->second; return &_member_vars[p_member_name]; }

	if (!_is_class) {
		auto it_cls = _classes.find(p_member_name);
		if (it_cls != _classes.end()) { _member_vars[p_member_name] = it_cls->second; return &_member_vars[p_member_name]; }

		auto it_ex = _externs.find(p_member_name);
		if (it_ex != _externs.end()) { _member_vars[p_member_name] = it_ex->second; return &_member_vars[p_member_name]; }
	}

	if (_base != nullptr) { return _base->_get_member_var_ptr(p_member_name); }
	return nullptr;
}

const stdmap<size_t, ptr<MemberInfo>>& Bytecode::get_member_info_list() {
	if (_member_info_built) return _member_info;

	for (auto& pair : _classes) {
		ptr<ClassInfo> class_info = newptr<ClassInfo>(pair.first, pair.second);
		_member_info[pair.first.hash()] = class_info;
	}

	for (auto& pair : _members) {
		ptr<PropertyInfo> prop_info = newptr<PropertyInfo>(pair.first);
		_member_info[pair.first.hash()] = prop_info;
	}
	for (auto& pair : _static_vars) {
		ptr<PropertyInfo> prop_info = newptr<PropertyInfo>(pair.first, var::VAR, pair.second, false, true);
		_member_info[pair.first.hash()] = prop_info;
	}
	for (auto& pair : _constants) {
		ptr<PropertyInfo> prop_info = newptr<PropertyInfo>(pair.first, var::VAR, pair.second, true, true);
		_member_info[pair.first.hash()] = prop_info;
	}
	for (auto& pair : _unnamed_enums) {
		ptr<EnumValueInfo> ev_info = newptr<EnumValueInfo>(pair.first, pair.second);
		_member_info[pair.first.hash()] = ev_info;
	}
	for (auto& pair : _enums) {
		ptr<EnumInfo> en_info = pair.second;
		_member_info[pair.first.hash()] = en_info;
	}

	// TODO: method info

	_member_info_built = true;
	return _member_info;
}

int Bytecode::get_member_offset() const {
	if (_base != nullptr) return _base->get_member_count();
	if (_pending_base != nullptr) {
		Parser::ClassNode* cls = (Parser::ClassNode*)_pending_base;
		uint32_t member_count = 0;
		for (auto& v : cls->vars) if (!v->is_static) member_count++;
		return cls->get_member_offset() + member_count;
	}
	return 0;
}

uint32_t Bytecode::get_member_index(const String& p_name) {
	auto it = _members.find(p_name);
	if (it == _members.end()) {
		if (_base == nullptr) {
			if (_pending_base != nullptr) return ((Parser::ClassNode*)_pending_base)->get_member_index(p_name);
			THROW_ERROR(Error::ATTRIBUTE_ERROR, String::format("no member named \"%s\" on base %s.",
				p_name.c_str(), _name.c_str()));
		} else {
			return _base->get_member_index(p_name);
		}
	} else {
		return get_member_offset() + it->second;
	}
}

const ptr<MemberInfo> Bytecode::get_member_info(const String& p_member_name) {

	if (!_member_info_built) get_member_info_list(); // this will build.

	auto it = _member_info.find(p_member_name.hash());
	if (it != _member_info.end()) return it->second;
	
	return nullptr;
}

// ------------ private -------------------

uint32_t Bytecode::_global_name_get(const String& p_name) {
	ASSERT(!_is_class); // global names only available at file.
	stdmap<String, uint32_t>::iterator it = _global_names.find(p_name);
	if (it == _global_names.end()) {
		uint32_t pos = (uint32_t)_global_names.size();
		_global_names[p_name] = pos;
		return pos;
	} else {
		return it->second;
	}
}

void Bytecode::_build_global_names_array() {
	_global_names_array.resize(_global_names.size());
	for (auto& it : _global_names) {
		_global_names_array[it.second] = it.first;
	}
}

uint32_t Bytecode::_global_const_value_get(const var& p_value) {
	for (int i = 0; i < (int)_global_const_values.size(); i++) {
		if (_global_const_values[i].get_type() == p_value.get_type() && _global_const_values[i] == p_value) {
			return i;
		}
	}
	_global_const_values.push_back(p_value);
	return (uint32_t)(_global_const_values.size() - 1);
}

}

#endif //_FILE_SRC_COMPILER_BYTECODE_CPP_

#ifndef _FILE_SRC_COMPILER_CODEGEN_CPP_


/******************************************************************************************************************/
/*                                         CODEGEN                                                                */
/******************************************************************************************************************/

namespace carbon {

void CGContext::insert_dbg(const Parser::Node* p_node) {
	//if (p_node == nullptr) return;
	opcodes->insert_dbg((uint32_t)p_node->pos.x);
}

void CGContext::clear() {
	curr_class = nullptr;
	function = nullptr;
	while (!stack_locals_frames.empty()) stack_locals_frames.pop();
	stack_locals.clear();
	parameters.clear();
	curr_stack_temps = 0;
	stack_max_size = 0;
	opcodes = newptr<Opcodes>();
}

void CGContext::push_stack_locals() {
	stack_locals_frames.push(stack_locals);
}

void CGContext::pop_stack_locals() {
	stack_locals = stack_locals_frames.top();
	stack_locals_frames.pop();
}

void CGContext::pop_stack_temp() {
	curr_stack_temps--;
}

Address CGContext::add_stack_local(const String& p_name) {
	ASSERT(stack_locals.find(p_name) == stack_locals.end());

	uint32_t stack_size = (uint32_t)stack_locals.size() + curr_stack_temps;
	stack_locals[p_name] = stack_size;
	stack_max_size = std::max(stack_max_size, stack_size + 1);
	return Address(Address::STACK, stack_size);
}

Address CGContext::get_stack_local(const String& p_name) {
	ASSERT(stack_locals.find(p_name) != stack_locals.end());
	return Address(Address::STACK, stack_locals[p_name]);
}

Address CGContext::get_parameter(const String& p_name) {
	for (int i = 0; i < (int)parameters.size(); i++) {
		if (parameters[i] == p_name) {
			return Address(Address::PARAMETER, i);
		}
	}
	THROW_BUG("parameter not found.");
}

Address CGContext::add_stack_temp() {
	uint32_t temp_pos = (uint32_t)stack_locals.size() + (curr_stack_temps++);
	stack_max_size = std::max(stack_max_size, temp_pos + 1);
	return Address(Address::STACK, temp_pos, true);
}
//--------------------------------------------------------------------

void CodeGen::_pop_addr_if_temp(const Address& m_addr) {
	if (m_addr.is_temp()) _context.pop_stack_temp();
}

Address CodeGen::add_global_const_value(const var& p_value) {
	uint32_t pos = _bytecode->_global_const_value_get(p_value);
	return Address(Address::CONST_VALUE, pos);
}

uint32_t CodeGen::add_global_name(const String& p_name) {
	return _bytecode->_global_name_get(p_name);
}

ptr<Bytecode> CodeGen::generate(ptr<Analyzer> p_analyzer) {
	ptr<Bytecode> bytecode = newptr<Bytecode>();
	_bytecode = bytecode.get();
	Parser::FileNode* root = p_analyzer->parser->file_node.get();

	_file_node = root;
	_context.bytecode = bytecode.get();

	bytecode->_name = root->path;
	_generate_members(static_cast<Parser::MemberContainer*>(root), bytecode.get());

	for (ptr<Parser::ImportNode>& import_node : root->imports) {
		_bytecode->_externs[import_node->name] = import_node->bytecode;
	}

	stdmap<Bytecode*, String> pending_inheritance;
	for (ptr<Parser::ClassNode>& class_node : root->classes) {
		ptr<Bytecode>_class = newptr<Bytecode>();
		_class->_file = bytecode;
		_class->_is_class = true;
		_class->_name = class_node->name;

		// set inheritance.
		_class->_has_base = class_node->base_type != Parser::ClassNode::NO_BASE;
		switch (class_node->base_type) {
			case Parser::ClassNode::NO_BASE:
				break;
			case Parser::ClassNode::BASE_LOCAL:
				pending_inheritance[_class.get()] = class_node->base_class->name;
				_class->_pending_base = (void*)class_node->base_class;
				break;
			case Parser::ClassNode::BASE_NATIVE:
				_class->_is_base_native = true;
				_class->_base_native = class_node->base_class_name;
				break;
			case Parser::ClassNode::BASE_EXTERN:
				_class->_is_base_native = false;
				_class->_base = class_node->base_binary;
				break;
		}
		_generate_members(static_cast<Parser::MemberContainer*>(class_node.get()), _class.get());
		_class->_pending_base = nullptr;
		bytecode->_classes[_class->_name] = _class;
	}

	for (auto it : pending_inheritance) {
		it.first->_base = bytecode->_classes.at(it.second);
	}

	_context.curr_class = nullptr;

	bytecode->_build_global_names_array();
	return bytecode;
}

void CodeGen::_generate_members(Parser::MemberContainer* p_container, Bytecode* p_bytecode) {

	// members/ static vars
	bool static_var_init_fn_need = false, member_var_init_fn_need = false;
	int member_index = 0;
	for (ptr<Parser::VarNode>& var_node : p_container->vars) {
		if (var_node->is_static) {
			var default_value; // default value set at runtime `static var x = f();`
			p_bytecode->_static_vars[var_node->name] = default_value;
			if (var_node->assignment != nullptr) static_var_init_fn_need = true;
		} else {
			p_bytecode->_members[var_node->name] = member_index++;
			if (var_node->assignment != nullptr) member_var_init_fn_need = true;
		}
	}

	// constants
	for (ptr<Parser::ConstNode>& const_node : p_container->constants) {
		p_bytecode->_constants[const_node->name] = const_node->value;
	}

	// unnamed enums
	if (p_container->unnamed_enum != nullptr) {
		for (std::pair<String, Parser::EnumValueNode> value : p_container->unnamed_enum->values) {
			p_bytecode->_unnamed_enums[value.first] = value.second.value;
		}
	}

	// named enums
	for (ptr<Parser::EnumNode> en : p_container->enums) {
		ptr<EnumInfo> ei = newptr<EnumInfo>(en->name);
		for (std::pair<String, Parser::EnumValueNode> value : en->values) {
			ei->get_edit_values()[value.first] = value.second.value;
		}
		p_bytecode->_enums[en->name] = ei;
	}

	if (member_var_init_fn_need) {
		ASSERT(p_bytecode->is_class());
		p_bytecode->_member_initializer = _generate_initializer(false, p_bytecode, p_container);
	}
	if (static_var_init_fn_need) {
		p_bytecode->_static_initializer = _generate_initializer(true, p_bytecode, p_container);
	}

	// functions
	for (ptr<Parser::FunctionNode> fn : p_container->functions) {
		const Parser::ClassNode* class_node = nullptr;
		if (p_container->type == Parser::Node::Type::CLASS) class_node = static_cast<const Parser::ClassNode*>(p_container);
		ptr<Function> cfn = _generate_function(fn.get(), class_node, p_bytecode);
		p_bytecode->_functions[cfn->_name] = cfn;

		if (fn->name == GlobalStrings::main) p_bytecode->_main = cfn.get();
		if (class_node && class_node->constructor == fn.get()) p_bytecode->_constructor = cfn.get();
	}
}


ptr<Function> CodeGen::_generate_initializer(bool p_static, Bytecode* p_bytecode, Parser::MemberContainer* p_container) {

	ptr<Function> cfn = newptr<Function>();

	const Parser::ClassNode* class_node = nullptr;
	if (p_container->type == Parser::Node::Type::CLASS) class_node = static_cast<const Parser::ClassNode*>(p_container);

	_context.clear();
	_context.function = cfn.get();
	_context.bytecode = p_bytecode;
	_context.curr_class = class_node;
	_context.opcodes->op_dbg = &cfn->op_dbg;

	for (ptr<Parser::VarNode>& var_node : p_container->vars) {
		if (var_node->is_static == p_static && var_node->assignment != nullptr) {
			Address member;
			if (p_static) {
				Bytecode* bytecode_file = (p_bytecode->is_class()) ? p_bytecode->get_file().get() : p_bytecode;
				member = Address(Address::STATIC_MEMBER, bytecode_file->_global_name_get(var_node->name));
			} else {
				member = Address(Address::MEMBER_VAR, _context.bytecode->get_member_index(var_node->name));
			}
			Address value = _generate_expression(var_node->assignment.get(), &member);
			if (member != value) {
				_context.insert_dbg(var_node.get());
				_context.opcodes->write_assign(member, value);
			}
			_pop_addr_if_temp(value);
		}
	}

	_context.opcodes->insert(Opcode::END);

	cfn->_name = (p_static) ? "@static_initializer" : "@member_initializer";
	cfn->_is_static = p_static;
	cfn->_owner = _context.bytecode;
	cfn->_opcodes = _context.opcodes->opcodes;
	cfn->_stack_size = _context.stack_max_size;

	return cfn;
}

ptr<Function> CodeGen::_generate_function(const Parser::FunctionNode* p_func, const Parser::ClassNode* p_class, Bytecode* p_bytecode) {
	ptr<Function> cfn = newptr<Function>();

	_context.clear();
	_context.function = cfn.get();
	_context.bytecode = p_bytecode;
	_context.curr_class = p_class;
	_context.opcodes->op_dbg = &cfn->op_dbg;
	for (int i = 0; i < (int)p_func->args.size(); i++) {
		cfn->_is_reference.push_back(p_func->args[i].is_reference);
		_context.parameters.push_back(p_func->args[i].name);
	}

	_generate_block(p_func->body.get());

	// Opcode::END dbg position
	_context.opcodes->insert_dbg(p_func->end_line);
	_context.opcodes->insert(Opcode::END);

	cfn->_name = p_func->name;
	cfn->_is_static = p_func->is_static;
	cfn->_arg_count = (int)p_func->args.size();
	cfn->_default_args = p_func->default_args;
	cfn->_owner = _context.bytecode;
	cfn->_default_args = p_func->default_args;
	cfn->_opcodes = _context.opcodes->opcodes;
	cfn->_stack_size = _context.stack_max_size;

	return cfn;
}

void CodeGen::_generate_block(const Parser::BlockNode* p_block) {

	_context.push_stack_locals();

	for (int i = 0; i < (int)p_block->statements.size(); i++) {
		const Parser::Node* statement = p_block->statements[i].get();

		switch (statement->type) {
			case Parser::Node::Type::UNKNOWN:
			case Parser::Node::Type::FILE:
			case Parser::Node::Type::CLASS:
			case Parser::Node::Type::ENUM:
			case Parser::Node::Type::FUNCTION:
			case Parser::Node::Type::BLOCK:
				THROW_BUG("invalid statement type in codegen.");

			case Parser::Node::Type::IDENTIFIER: {
				THROW_BUG("standalone expression didn't cleaned.");
			} break;

			case Parser::Node::Type::VAR: {
				const Parser::VarNode* var_node = static_cast<const Parser::VarNode*>(statement);
				Address local_var = _context.add_stack_local(var_node->name);
				if (var_node->assignment != nullptr) {
					Address assign_value = _generate_expression(var_node->assignment.get(), &local_var);
					if (assign_value != local_var) {
						_context.insert_dbg(var_node);
						_context.opcodes->write_assign(local_var, assign_value);
					}
					_pop_addr_if_temp(assign_value);
				}
			} break;

			case Parser::Node::Type::CONST: {
				THROW_BUG("local constants should be cleaned by now");
			} break;

			case Parser::Node::Type::CONST_VALUE:
			case Parser::Node::Type::ARRAY:
			case Parser::Node::Type::MAP:
			case Parser::Node::Type::THIS:
			case Parser::Node::Type::SUPER:
			case Parser::Node::Type::BUILTIN_FUNCTION:
			case Parser::Node::Type::BUILTIN_TYPE: {
				THROW_BUG("stand alone expression didn't cleaned.");
			} break;

			case Parser::Node::Type::CALL:
			case Parser::Node::Type::INDEX:
			case Parser::Node::Type::MAPPED_INDEX:
			case Parser::Node::Type::OPERATOR: {
				Address expr = _generate_expression(statement);
				_pop_addr_if_temp(expr);
			} break;

			case Parser::Node::Type::CONTROL_FLOW: {
				_generate_control_flow(static_cast<const Parser::ControlFlowNode*>(statement));
			} break;
		}

	}

	_context.pop_stack_locals();

}

}

/******************************************************************************************************************/
/*                                         GEN CONTROLFLOAT                                                       */
/******************************************************************************************************************/

namespace carbon {

void CodeGen::_generate_control_flow(const Parser::ControlFlowNode* p_cflow) {
	switch (p_cflow->cf_type) {
		case Parser::ControlFlowNode::CfType::IF: {
			ASSERT(p_cflow->args.size() == 1);
			Address cond = _generate_expression(p_cflow->args[0].get());
			_context.insert_dbg(p_cflow);
			_context.opcodes->write_if(cond);
			if (cond.is_temp()) _context.pop_stack_temp();

			_generate_block(p_cflow->body.get());

			if (p_cflow->body_else != nullptr) {
				_context.insert_dbg(p_cflow->body_else.get());
				_context.opcodes->write_else();
				_generate_block(p_cflow->body_else.get());
			}

			_context.opcodes->write_endif();
		} break;

		case Parser::ControlFlowNode::CfType::SWITCH: {
			// TODO:
		} break;

		case Parser::ControlFlowNode::CfType::WHILE: {
			ASSERT(p_cflow->args.size() == 1);
			_context.opcodes->jump_to_continue.push(_context.opcodes->next());
			Address cond = _generate_expression(p_cflow->args[0].get());
			_context.insert_dbg(p_cflow);
			_context.opcodes->write_while(cond);
			if (cond.is_temp()) _context.pop_stack_temp();
			_generate_block(p_cflow->body.get());
			_context.opcodes->write_endwhile();
		} break;

		case Parser::ControlFlowNode::CfType::FOR: {
			ASSERT(p_cflow->args.size() == 3);
			_context.push_stack_locals();

			// iterator
			if (p_cflow->args[0] != nullptr) {
				const Parser::VarNode* var_node = static_cast<const Parser::VarNode*>(p_cflow->args[0].get());
				Address iterator = _context.add_stack_local(var_node->name);
				if (var_node->assignment != nullptr) {
					Address assign_value = _generate_expression(var_node->assignment.get());
					_context.insert_dbg(var_node);
					_context.opcodes->write_assign(iterator, assign_value);
					if (assign_value.is_temp()) _context.pop_stack_temp();
				}
			}

			// condition.
			Address cond;
			_context.opcodes->jump_to_continue.push(_context.opcodes->next());
			if (p_cflow->args[1] != nullptr) {
				const Parser::Node* cond_node = p_cflow->args[1].get();
				cond = _generate_expression(cond_node);
			}
			_context.insert_dbg(p_cflow);
			_context.opcodes->write_for(cond);
			if (cond.is_temp()) _context.pop_stack_temp();

			// body
			_generate_block(p_cflow->body.get());

			// end
			Address end_statement;
			if (p_cflow->args[2] != nullptr) {
				const Parser::Node* end_statement_node = p_cflow->args[2].get();
				end_statement = _generate_expression(end_statement_node);
				if (end_statement.is_temp()) _context.pop_stack_temp();
			}
			_context.opcodes->write_endfor(cond.get_type() != Address::_NULL);

			_context.pop_stack_locals();
		} break;

		case Parser::ControlFlowNode::CfType::FOREACH: {
			ASSERT(p_cflow->args.size() == 2);
			_context.push_stack_locals();

			const String& iterator_name = static_cast<const Parser::VarNode*>(p_cflow->args[0].get())->name;
			Address iter_value = _context.add_stack_local(iterator_name);
			Address iterator = _context.add_stack_temp();
			Address on = _generate_expression(p_cflow->args[1].get());

			_context.insert_dbg(p_cflow);
			_context.opcodes->write_foreach(iter_value, iterator, on);
			_generate_block(p_cflow->body.get());
			_pop_addr_if_temp(iterator);
			_context.opcodes->write_endforeach();
			if (on.is_temp()) _context.pop_stack_temp();

			_context.pop_stack_locals();
		} break;

		case Parser::ControlFlowNode::CfType::BREAK: {
			ASSERT(p_cflow->args.size() == 0);
			_context.insert_dbg(p_cflow);
			_context.opcodes->write_break();
		} break;
		case Parser::ControlFlowNode::CfType::CONTINUE: {
			ASSERT(p_cflow->args.size() == 0);
			_context.insert_dbg(p_cflow);
			_context.opcodes->write_continue();
		} break;
		case Parser::ControlFlowNode::CfType::RETURN: {
			ASSERT(p_cflow->args.size() <= 1);
			Address ret;
			if (p_cflow->args.size() == 1) ret = _generate_expression(p_cflow->args[0].get());
			_context.insert_dbg(p_cflow);
			_context.opcodes->write_return(ret);
			if (ret.is_temp()) _context.pop_stack_temp();
			
		} break;
	}
	MISSED_ENUM_CHECK(Parser::ControlFlowNode::CfType::_CF_MAX_, 8);
}

}

/******************************************************************************************************************/
/*                                         GEN EXPRESSION                                                         */
/******************************************************************************************************************/

namespace carbon {

Address CodeGen::_generate_expression(const Parser::Node* p_expr, Address* p_dst) {

#define ADDR_DST() (p_dst) ? * p_dst : _context.add_stack_temp()

	switch (p_expr->type) {
		case Parser::Node::Type::IMPORT:
		case Parser::Node::Type::FILE:
		case Parser::Node::Type::CLASS:
		case Parser::Node::Type::ENUM:
		case Parser::Node::Type::FUNCTION:
		case Parser::Node::Type::BLOCK:
			THROW_BUG("invalid expression type found in codegen");

		case Parser::Node::Type::IDENTIFIER: {
			const Parser::IdentifierNode* id = static_cast<const Parser::IdentifierNode*>(p_expr);
			switch (id->ref) {
				case Parser::IdentifierNode::REF_UNKNOWN:
					THROW_BUG("unknown identifier should be analyzer error (can't reach to codegen)");

				case Parser::IdentifierNode::REF_PARAMETER: {
					return _context.get_parameter(id->name);
				} break;

				case Parser::IdentifierNode::REF_LOCAL_VAR: {
					return _context.get_stack_local(id->name);
				} break;

				case Parser::IdentifierNode::REF_LOCAL_CONST:
					THROW_BUG("identifier to local const should be reduced at analyzing phace");

				case Parser::IdentifierNode::REF_MEMBER_VAR: {
					return Address(Address::MEMBER_VAR, _context.bytecode->get_member_index(id->name));
				} break;

				case Parser::IdentifierNode::REF_STATIC_VAR:
				case Parser::IdentifierNode::REF_MEMBER_CONST:
				case Parser::IdentifierNode::REF_ENUM_NAME:
				case Parser::IdentifierNode::REF_ENUM_VALUE:
				case Parser::IdentifierNode::REF_FUNCTION:
				case Parser::IdentifierNode::REF_CARBON_CLASS: {
					return Address(Address::STATIC_MEMBER, _bytecode->_global_name_get(id->name));
				} break;

				case Parser::IdentifierNode::REF_NATIVE_CLASS: {
					return Address(Address::NATIVE_CLASS, _bytecode->_global_name_get(id->name));
				} break;

				case Parser::IdentifierNode::REF_EXTERN: {
					return Address(Address::EXTERN, _bytecode->_global_name_get(id->name));
				} break;
			}
		} break;

		case Parser::Node::Type::VAR:
		case Parser::Node::Type::CONST: {
			THROW_BUG("invalid expression node found in codegen.");
		} break;

		case Parser::Node::Type::CONST_VALUE: {
			return add_global_const_value(static_cast<const Parser::ConstValueNode*>(p_expr)->value);
		} break;

		case Parser::Node::Type::ARRAY: {
			const Parser::ArrayNode* arr = static_cast<const Parser::ArrayNode*>(p_expr);

			Address arr_dst = ADDR_DST();

			stdvec<Address> values;
			for (int i = 0; i < (int)arr->elements.size(); i++) {
				Address val = _generate_expression(arr->elements[i].get());
				values.push_back(val);
			}
			_context.insert_dbg(p_expr);
			_context.opcodes->write_array_literal(arr_dst, values);

			for (Address& addr : values) {
				_pop_addr_if_temp(addr);
			}

			return arr_dst;
		} break;

		case Parser::Node::Type::MAP: {
			const Parser::MapNode* map = static_cast<const Parser::MapNode*>(p_expr);

			Address map_dst = ADDR_DST();

			stdvec<Address> keys, values;
			for (auto& pair : map->elements) {
				Address key = _generate_expression(pair.key.get());
				Address value = _generate_expression(pair.value.get());

				keys.push_back(key);
				values.push_back(value);
			}
			_context.insert_dbg(p_expr);
			_context.opcodes->write_map_literal(map_dst, keys, values);

			for (Address& addr : keys) _pop_addr_if_temp(addr);
			for (Address& addr : values) _pop_addr_if_temp(addr);

			return map_dst;
		} break;

		case Parser::Node::Type::THIS: {
			return Address(Address::THIS, 0);
		} break;
		case Parser::Node::Type::SUPER: {
			THROW_BUG("TODO:");
			// TODO: super cannot be a stand alone expression (throw error)
		} break;

		case Parser::Node::Type::BUILTIN_FUNCTION: {
			return Address(Address::BUILTIN_FUNC, (uint32_t)static_cast<const Parser::BuiltinFunctionNode*>(p_expr)->func);
		} break;

		case Parser::Node::Type::BUILTIN_TYPE: {
			return Address(Address::BUILTIN_TYPE, (uint32_t)static_cast<const Parser::BuiltinTypeNode*>(p_expr)->builtin_type);
		} break;

		case Parser::Node::Type::CALL: {
			const Parser::CallNode* call = static_cast<const Parser::CallNode*>(p_expr);
			Address ret = ADDR_DST();

			stdvec<Address> args;
			for (int i = 0; i < (int)call->args.size(); i++) {
				Address arg = _generate_expression(call->args[i].get());
				args.push_back(arg);
			}

			switch (call->base->type) {

				// print(); builtin func call
				case Parser::Node::Type::BUILTIN_FUNCTION: {
					if (call->method == nullptr) { // print(...);
						_context.insert_dbg(p_expr);
						_context.opcodes->write_call_builtin(ret, static_cast<const Parser::BuiltinFunctionNode*>(call->base.get())->func, args);
					} else { // print.member(...);
						Address base = _generate_expression(call->base.get());
						ASSERT(call->method->type == Parser::Node::Type::IDENTIFIER);
						uint32_t name = add_global_name(static_cast<const Parser::IdentifierNode*>(call->method.get())->name);
						_context.insert_dbg(call->method.get());
						_context.opcodes->write_call_method(ret, base, name,  args);
						_pop_addr_if_temp(base);
					}
				} break;

				// Array(); builtin type constructor
				case Parser::Node::Type::BUILTIN_TYPE: {
					if (call->method == nullptr) { // Array(); constructor
						_context.insert_dbg(p_expr);
						_context.opcodes->write_construct_builtin_type(ret, static_cast<const Parser::BuiltinTypeNode*>(call->base.get())->builtin_type, args);
					} else { // String.format(); // static method call on builtin type
						Address base = _generate_expression(call->base.get());
						ASSERT(call->method->type == Parser::Node::Type::IDENTIFIER);
						uint32_t name = add_global_name(static_cast<const Parser::IdentifierNode*>(call->method.get())->name);
						_context.insert_dbg(call->method.get());
						_context.opcodes->write_call_method(ret, base, name, args);
						_pop_addr_if_temp(base);
					}
				} break;

				case Parser::Node::Type::SUPER: {

					if (call->method == nullptr) { // super(...); if used in constructor -> super constructor else call same func on super.
						_context.insert_dbg(p_expr);
						_context.opcodes->write_call_super_constructor(args);
					} else { // super.f();
						ASSERT(call->method->type == Parser::Node::Type::IDENTIFIER);
						const Parser::IdentifierNode* method = ptrcast<Parser::IdentifierNode>(call->method).get();
						uint32_t name = add_global_name(method->name);
						_context.insert_dbg(call->method.get());
						_context.opcodes->write_call_super_method(ret, name, args);
					}

				} break;

				case Parser::Node::Type::UNKNOWN: {
					ASSERT(call->method->type == Parser::Node::Type::IDENTIFIER);
					const Parser::IdentifierNode* func = ptrcast<Parser::IdentifierNode>(call->method).get();
					uint32_t name = add_global_name(func->name);
					switch (func->ref) {
						case  Parser::IdentifierNode::REF_FUNCTION: {
							_context.insert_dbg(p_expr);
							_context.opcodes->write_call_func(ret, name, args);
						} break;
						case  Parser::IdentifierNode::REF_CARBON_CLASS: {
							_context.insert_dbg(p_expr);
							_context.opcodes->write_construct_carbon(ret, name, args);
						} break;
						case Parser::IdentifierNode::REF_NATIVE_CLASS: {
							_context.insert_dbg(p_expr);
							_context.opcodes->write_construct_native(ret, name, args);
						} break;
						default: {
							THROW_BUG("can't reach here"); // TODO: refactor
						}
					}
				} break;

				default: {
					Address base = _generate_expression(call->base.get());
					if (call->method != nullptr) {
						ASSERT(call->method->type == Parser::Node::Type::IDENTIFIER);
						uint32_t name = add_global_name(static_cast<const Parser::IdentifierNode*>(call->method.get())->name);
						_context.insert_dbg(call->method.get());
						_context.opcodes->write_call_method(ret, base, name, args);
					} else {
						_context.insert_dbg(p_expr);
						_context.opcodes->write_call(ret, base, args);
					}
					_pop_addr_if_temp(base);
				} break;
			}
			for (Address& addr : args) _pop_addr_if_temp(addr);
			return ret;
		} break;

		case Parser::Node::Type::INDEX: {
			Address dst = ADDR_DST();
			const Parser::IndexNode* index_node = static_cast<const Parser::IndexNode*>(p_expr);
			Address on = _generate_expression(index_node->base.get());
			uint32_t name = add_global_name(index_node->member->name);
			_context.insert_dbg(index_node->member.get());
			_context.opcodes->write_get_index(on, name, dst);
			_pop_addr_if_temp(on);
			return dst;
		} break;

		case Parser::Node::Type::MAPPED_INDEX: {
			Address dst = ADDR_DST();
			const Parser::MappedIndexNode* index_node = static_cast<const Parser::MappedIndexNode*>(p_expr);
			Address on = _generate_expression(index_node->base.get());
			Address key = _generate_expression(index_node->key.get());

			_context.insert_dbg(index_node->key.get());
			_context.opcodes->write_get_mapped(on, key, dst);

			_pop_addr_if_temp(on);
			_pop_addr_if_temp(key);
			return dst;
		} break;
		case Parser::Node::Type::OPERATOR: {
			const Parser::OperatorNode* op = static_cast<const Parser::OperatorNode*>(p_expr);

			var::Operator var_op = var::_OP_MAX_;
			switch (op->op_type) {
				case Parser::OperatorNode::OP_EQ:                                             goto _addr_operator_assign_;
				case Parser::OperatorNode::OP_PLUSEQ:        var_op = var::OP_ADDITION;       goto _addr_operator_assign_;
				case Parser::OperatorNode::OP_MINUSEQ:       var_op = var::OP_SUBTRACTION;	  goto _addr_operator_assign_;
				case Parser::OperatorNode::OP_MULEQ:         var_op = var::OP_MULTIPLICATION; goto _addr_operator_assign_;
				case Parser::OperatorNode::OP_DIVEQ:         var_op = var::OP_DIVISION;		  goto _addr_operator_assign_;
				case Parser::OperatorNode::OP_MOD_EQ:        var_op = var::OP_MODULO;		  goto _addr_operator_assign_;
				case Parser::OperatorNode::OP_BIT_LSHIFT_EQ: var_op = var::OP_BIT_LSHIFT;	  goto _addr_operator_assign_;
				case Parser::OperatorNode::OP_BIT_RSHIFT_EQ: var_op = var::OP_BIT_RSHIFT;	  goto _addr_operator_assign_;
				case Parser::OperatorNode::OP_BIT_OR_EQ:     var_op = var::OP_BIT_OR;		  goto _addr_operator_assign_;
				case Parser::OperatorNode::OP_BIT_AND_EQ:    var_op = var::OP_BIT_AND;		  goto _addr_operator_assign_;
				case Parser::OperatorNode::OP_BIT_XOR_EQ: {  var_op = var::OP_BIT_XOR;
				_addr_operator_assign_:
					// indexing, mapped indexing is special case.
					if (op->args[0]->type == Parser::Node::Type::INDEX) {
						const Parser::IndexNode* index = static_cast<Parser::IndexNode*>(op->args[0].get());
						Address on = _generate_expression(index->base.get());
						uint32_t name = add_global_name(ptrcast<Parser::IdentifierNode>(index->member)->name);
						Address value = _generate_expression(op->args[1].get());

						if (var_op != var::_OP_MAX_) {
							Address tmp = _context.add_stack_temp();
							_context.insert_dbg(index->member.get());
							_context.opcodes->write_get_index(on, name, tmp);

							_context.insert_dbg(p_expr);
							_context.opcodes->write_operator(tmp, var_op, tmp, value);

							_context.insert_dbg(index->member.get());
							_context.opcodes->write_set_index(on, name, tmp);

							_pop_addr_if_temp(on);
							return tmp;

						} else {

							_context.insert_dbg(index->member.get());
							_context.opcodes->write_set_index(on, name, value);

							_pop_addr_if_temp(on);
							return value;
						}

					} else if (op->args[0]->type == Parser::Node::Type::MAPPED_INDEX) {
						const Parser::MappedIndexNode* mapped = static_cast<const Parser::MappedIndexNode*>(op->args[0].get());
						Address on = _generate_expression(mapped->base.get());
						Address key = _generate_expression(mapped->key.get());
						Address value = _generate_expression(op->args[1].get());

						if (var_op != var::_OP_MAX_) {
							Address tmp = _context.add_stack_temp();
							_context.insert_dbg(mapped->key.get());
							_context.opcodes->write_get_mapped(on, key, tmp);

							_context.insert_dbg(p_expr);
							_context.opcodes->write_operator(tmp, var_op, tmp, value);

							_context.insert_dbg(mapped->key.get());
							_context.opcodes->write_set_mapped(on, key, tmp);

							_pop_addr_if_temp(on);
							_pop_addr_if_temp(key);
							return tmp;

						} else {
							_context.insert_dbg(mapped->key.get());
							_context.opcodes->write_set_mapped(on, key, value);

							_pop_addr_if_temp(on);
							_pop_addr_if_temp(key);
							return value;
						}

					} else {
						Address left = _generate_expression(op->args[0].get());
						if (left.is_temp()) THROW_ERROR(Error::SYNTAX_ERROR, "invalid assignment to an expression"); // f() = 12; TODO: throw with dbg info.
						if (var_op != var::_OP_MAX_) {
							Address right = _generate_expression(op->args[1].get());
							_context.insert_dbg(p_expr);
							_context.opcodes->write_operator(left, var_op, left, right);
							_pop_addr_if_temp(right);
						} else {
							Address right = _generate_expression(op->args[1].get(), &left);
							_context.insert_dbg(p_expr);
							if (left != right) _context.opcodes->write_assign(left, right);
							_pop_addr_if_temp(right);
						}
						return left;
					}
				} break;

				case Parser::OperatorNode::OP_AND: {
					Address dst = ADDR_DST();
					_context.insert_dbg(p_expr);
					_context.opcodes->write_assign_bool(dst, false);
					Address left = _generate_expression(op->args[0].get());
					_context.insert_dbg(p_expr);
					_context.opcodes->write_and_left(left);
					Address right = _generate_expression(op->args[1].get());
					_context.insert_dbg(p_expr);
					_context.opcodes->write_and_right(right, dst);
					_pop_addr_if_temp(left);
					_pop_addr_if_temp(right);
					return dst;
				} break;

				case Parser::OperatorNode::OP_OR: {
					Address dst = ADDR_DST();
					_context.insert_dbg(p_expr);
					_context.opcodes->write_assign_bool(dst, true);
					Address left = _generate_expression(op->args[0].get());
					_context.insert_dbg(p_expr);
					_context.opcodes->write_or_left(left);
					Address right = _generate_expression(op->args[1].get());
					_context.insert_dbg(p_expr);
					_context.opcodes->write_or_right(right, dst);
					_pop_addr_if_temp(left);
					_pop_addr_if_temp(right);
					return dst;
				} break;

				case Parser::OperatorNode::OP_EQEQ:       var_op = var::OP_EQ_CHECK;       goto _addr_operator_;
				case Parser::OperatorNode::OP_NOTEQ:      var_op = var::OP_NOT_EQ_CHECK;   goto _addr_operator_;
				case Parser::OperatorNode::OP_LTEQ:       var_op = var::OP_LTEQ;		   goto _addr_operator_;
				case Parser::OperatorNode::OP_GTEQ:       var_op = var::OP_GTEQ;		   goto _addr_operator_;
				case Parser::OperatorNode::OP_PLUS:       var_op = var::OP_ADDITION;	   goto _addr_operator_;
				case Parser::OperatorNode::OP_MINUS:      var_op = var::OP_SUBTRACTION;	   goto _addr_operator_;
				case Parser::OperatorNode::OP_MUL:        var_op = var::OP_MULTIPLICATION; goto _addr_operator_;
				case Parser::OperatorNode::OP_DIV:        var_op = var::OP_DIVISION;	   goto _addr_operator_;
				case Parser::OperatorNode::OP_MOD:        var_op = var::OP_MODULO;		   goto _addr_operator_;
				case Parser::OperatorNode::OP_LT:         var_op = var::OP_LT;			   goto _addr_operator_;
				case Parser::OperatorNode::OP_GT:         var_op = var::OP_GT;			   goto _addr_operator_;
				case Parser::OperatorNode::OP_BIT_LSHIFT: var_op = var::OP_BIT_LSHIFT;	   goto _addr_operator_;
				case Parser::OperatorNode::OP_BIT_RSHIFT: var_op = var::OP_BIT_RSHIFT;	   goto _addr_operator_;
				case Parser::OperatorNode::OP_BIT_OR:     var_op = var::OP_BIT_OR;		   goto _addr_operator_;
				case Parser::OperatorNode::OP_BIT_AND:    var_op = var::OP_BIT_AND;		   goto _addr_operator_;
				case Parser::OperatorNode::OP_BIT_XOR: {  var_op = var::OP_BIT_XOR;
					_addr_operator_:
					Address dst = ADDR_DST();
					Address left = _generate_expression(op->args[0].get());
					Address right = _generate_expression(op->args[1].get());
					_context.insert_dbg(p_expr);
					_context.opcodes->write_operator(dst, var_op, left, right);
					_pop_addr_if_temp(left);
					_pop_addr_if_temp(right);
					return dst;
				} break;

				case Parser::OperatorNode::OP_NOT: {
					Address dst = ADDR_DST();
					Address left = _generate_expression(op->args[0].get());
					_context.insert_dbg(p_expr);
					_context.opcodes->write_operator(dst, var::OP_NOT, left, Address());
					_pop_addr_if_temp(left);
					return dst;
				} break;

				case Parser::OperatorNode::OP_BIT_NOT: {
					Address dst = ADDR_DST();
					Address left = _generate_expression(op->args[0].get());
					_context.insert_dbg(p_expr);
					_context.opcodes->write_operator(dst, var::OP_BIT_NOT, left, Address());
					_pop_addr_if_temp(left);
					return dst;
				} break;

				case Parser::OperatorNode::OP_POSITIVE: {
					Address dst = ADDR_DST();
					Address left = _generate_expression(op->args[0].get());
					_context.insert_dbg(p_expr);
					_context.opcodes->write_operator(dst, var::OP_POSITIVE, left, Address());
					_pop_addr_if_temp(left);
					return dst;
				} break;
				case Parser::OperatorNode::OP_NEGATIVE: {
					Address dst = ADDR_DST();
					Address left = _generate_expression(op->args[0].get());
					_context.insert_dbg(p_expr);
					_context.opcodes->write_operator(dst, var::OP_NEGATIVE, left, Address());
					_pop_addr_if_temp(left);
					return dst;
				} break;
			}
		} break;

		case Parser::Node::Type::CONTROL_FLOW: {
			THROW_BUG("invalid expression node found in codegen.");
		} break;
	}


	return Address();
}

}

#endif //_FILE_SRC_COMPILER_CODEGEN_CPP_

#ifndef _FILE_SRC_COMPILER_COMPILER_CPP_



namespace carbon {

Compiler* Compiler::_singleton = nullptr;
Compiler* Compiler::singleton() {
	if (_singleton == nullptr) _singleton = new Compiler();
	return _singleton;
}

void Compiler::cleanup() {
	if (_singleton != nullptr) delete _singleton;
}

void Compiler::add_flag(CompileFlags p_flag) { _flags |= p_flag; }
void Compiler::add_include_dir(const String& p_dir) {
	if (!Path(p_dir).isdir()) {
		// TODO: throw error / warning (ignore for now)
	} else {
		_include_dirs.push_back(Path(p_dir).absolute());
	}
}

ptr<Bytecode> Compiler::_compile(const String& p_path) {

	// TODO: print only if serialize to bytecode.
	//Logger::log(String::format("compiling: %s\n", p_path.c_str()).c_str());

	class ScopeDestruct {
	public:
		std::stack<String>* _cwd_ptr = nullptr;
		ScopeDestruct(std::stack<String>* p_cwd_ptr) {
			_cwd_ptr = p_cwd_ptr;
		}
		~ScopeDestruct() {
			OS::chdir(_cwd_ptr->top());
			_cwd_ptr->pop();
		}
	};
	ScopeDestruct destruct = ScopeDestruct(&_cwd);
	_cwd.push(OS::getcwd());
	OS::chdir(Path(p_path).parent());

	ptr<File> file = newptr<File>();
	ptr<Tokenizer> tokenizer = newptr<Tokenizer>();
	ptr<Parser> parser = newptr<Parser>();
	ptr<Analyzer> analyzer = newptr<Analyzer>();
	ptr<CodeGen> codegen = newptr<CodeGen>();
	ptr<Bytecode> bytecode;

	file->open(p_path, File::READ);
	tokenizer->tokenize(file);
	parser->parse(tokenizer);
	analyzer->analyze(parser);
	bytecode = codegen->generate(analyzer);
	file->close();

	for (const Warning& warning : analyzer->get_warnings()) {
		warning.console_log(); // TODO: it shouldn't print, add to warnings list instead.
	}

	return bytecode;
}

ptr<Bytecode> Compiler::compile(const String& p_path, bool p_use_cache) {

	if (!Path(p_path).exists()) THROW_ERROR(Error::IO_ERROR, String::format("path \"%s\" does not exists.", p_path.c_str()));

	String path = Path(p_path).absolute();
	auto it = _cache.find(path);
	if (it != _cache.end()) {
		if (it->second.compiling)  THROW_ERROR(Error::IO_ERROR, String::format("cyclic import found in \"%s\"", path.c_str()));
		if (p_use_cache) return it->second.bytecode;
	} else {
		_cache[path] = _Cache();
	}

	ptr<Bytecode> bytecode = _compile(path);

	_cache[path].bytecode = bytecode;
	_cache[path].compiling = false;
	return bytecode;
}

}

#endif //_FILE_SRC_COMPILER_COMPILER_CPP_

#ifndef _FILE_SRC_COMPILER_FUNCTION_CPP_


namespace carbon {

const String& Function::get_name() const { return _name; }
bool Function::is_static() const { return _is_static; }
int Function::get_arg_count() const { return _arg_count; }
const stdvec<var>& Function::get_default_args() const { return _default_args; }
const stdvec<bool>& Function::get_is_args_ref() const { return _is_reference; }
uint32_t Function::get_stack_size() const { return _stack_size; }
const Bytecode* Function::get_owner() const { return _owner; }

const stdvec<uint32_t>& Function::get_opcodes() const { return _opcodes; }
const stdmap<uint32_t, uint32_t>& Function::get_op_dbg() const { return op_dbg; }


var Function::__call(stdvec<var*>& p_args) {
	return VM::singleton()->call_function(this, _owner, nullptr, p_args);
}

}

#endif //_FILE_SRC_COMPILER_FUNCTION_CPP_

#ifndef _FILE_SRC_COMPILER_INSTANCE_CPP_


namespace carbon {

Instance::Instance() {}
Instance::Instance(ptr<Bytecode>& p_blueprint) {
	blueprint = p_blueprint;
	members.resize(blueprint->get_member_count());
}

bool Instance::_is_registered() const { return false; }

var Instance::get_member(const String& p_name) {
	// try members
	try {
		uint32_t pos = blueprint->get_member_index(p_name);
		return members[pos];
	} catch (Error& err) {
		if (err.get_type() != Error::ATTRIBUTE_ERROR) throw err;
	}

	// not found in members try static
	try {
		return blueprint->get_member(p_name);
	} catch (Error& err) {
		if (err.get_type() != Error::ATTRIBUTE_ERROR) throw err;
	}
	// throw here for better error message
	THROW_ERROR(Error::ATTRIBUTE_ERROR, String::format("no member named \"%s\" on base %s.",
		p_name.c_str(), blueprint->get_name().c_str()));
}

void Instance::set_member(const String& p_name, var& p_value) {
	uint32_t pos = blueprint->get_member_index(p_name);
	members[pos] = p_value;
}

var Instance::call_method(const String& p_method_name, stdvec<var*>& p_args) {

	Function* fn = nullptr;
	Bytecode* _class = blueprint.get();
	while (_class) {
		auto& functions = _class->get_functions();
		// TODO: if not found in functions check in member/static members too.
		auto it = functions.find(p_method_name);
		if (it != functions.end()) {
			fn = it->second.get();
			break;
		} else {
			if (!_class->has_base()) {
				THROW_ERROR(Error::ATTRIBUTE_ERROR,
					String::format("attribute \"%s\" doesn't exists on base %s.", p_method_name.c_str(), blueprint->get_name().c_str()));
			}
			if (_class->is_base_native()) {
				return Object::call_method_s(native_instance, p_method_name, p_args);
				// TODO: move the above method to native like below
				//return NativeClasses::singleton()->call_method_on(native_instance, p_method_name, p_args);
			} else {
				_class = _class->get_base_binary().get();
			}
		}
	}

	ASSERT(fn != nullptr);
	if (fn->is_static()) { // calling static method using instance (acceptable)
		return VM::singleton()->call_function(fn, _class, nullptr, p_args);
	} else {
		return VM::singleton()->call_function(fn, _class, shared_from_this(), p_args);
	}
}

ptr<Object> Instance::copy(bool p_deep) {
	ptr<Instance> ins_copy = newptr<Instance>(blueprint);
	for (int i = 0; i < (int)members.size(); i++) {
		ins_copy->members[i] = members[i].copy(p_deep);
	}
	return ins_copy;
}

void* Instance::get_data() {
	if (native_instance != nullptr) return native_instance->get_data();
	return Super::get_data();
}

String Instance::to_string() {
	ptr<Function> fn = blueprint->get_function(GlobalStrings::to_string);
	if (fn == nullptr) return Super::to_string();
	stdvec<var*> _args;
	return VM::singleton()->call_function(fn.get(), blueprint.get(), shared_from_this(), _args);
}


var Instance::__call(stdvec<var*>& p_args) {
	ptr<Function> fn = blueprint->get_function(GlobalStrings::__call);
	if (fn == nullptr) return Super::__call(p_args);
	return VM::singleton()->call_function(fn.get(), blueprint.get(), shared_from_this(), p_args);
}

var Instance::__iter_begin() {
	ptr<Function> fn = blueprint->get_function(GlobalStrings::__iter_begin);
	if (fn == nullptr) return Super::__iter_begin();
	stdvec<var*> _args;
	return VM::singleton()->call_function(fn.get(), blueprint.get(), shared_from_this(), _args);
}
bool Instance::__iter_has_next() {
	ptr<Function> fn = blueprint->get_function(GlobalStrings::__iter_has_next);
	if (fn == nullptr) return Super::__iter_has_next();
	stdvec<var*> _args;
	return VM::singleton()->call_function(fn.get(), blueprint.get(), shared_from_this(), _args);
}
var Instance::__iter_next() {
	ptr<Function> fn = blueprint->get_function(GlobalStrings::__iter_next);
	if (fn == nullptr) return Super::__iter_next();
	stdvec<var*> _args;
	return VM::singleton()->call_function(fn.get(), blueprint.get(), shared_from_this(), _args);
}

var Instance::__get_mapped(const var& p_key) {
	ptr<Function> fn = blueprint->get_function(GlobalStrings::__get_mapped);
	if (fn == nullptr) return Super::__get_mapped(p_key);
	stdvec<var*> args; args.push_back(const_cast<var*>(&p_key));
	return VM::singleton()->call_function(fn.get(), blueprint.get(), shared_from_this(), args);
}
void Instance::__set_mapped(const var& p_key, const var& p_val) {
	ptr<Function> fn = blueprint->get_function(GlobalStrings::__set_mapped);
	if (fn == nullptr) Super::__set_mapped(p_key, p_val);
	stdvec<var*> args = { const_cast<var*>(&p_key), const_cast<var*>(&p_val) };
	VM::singleton()->call_function(fn.get(), blueprint.get(), shared_from_this(), args);
}
int64_t Instance::__hash() {
	ptr<Function> fn = blueprint->get_function(GlobalStrings::__hash);
	if (fn == nullptr) return Super::__hash();
	stdvec<var*> _args;
	return VM::singleton()->call_function(fn.get(), blueprint.get(), shared_from_this(), _args);
}

#define CALL_OPERATOR(m_ret, m_operator)                                                            \
	m_ret Instance::m_operator(const var& p_other) {												\
		ptr<Function> fn = blueprint->get_function(GlobalStrings::m_operator);				\
		if (fn == nullptr) return Super::m_operator(p_other);										\
		stdvec<var*> args = { const_cast<var*>(&p_other) };										    \
		return VM::singleton()->call_function(fn.get(), blueprint.get(), shared_from_this(), args);	\
	}
CALL_OPERATOR(bool, __gt);
CALL_OPERATOR(bool, __lt);
CALL_OPERATOR(bool, __eq);

CALL_OPERATOR(var, __add);
CALL_OPERATOR(var, __sub);
CALL_OPERATOR(var, __mul);
CALL_OPERATOR(var, __div);


}

#endif //_FILE_SRC_COMPILER_INSTANCE_CPP_

#ifndef _FILE_SRC_COMPILER_OPCODES_CPP_


namespace carbon {

String Opcodes::get_opcode_name(Opcode p_opcode) {
	static const char* _names[] = {
		"GET",
		"SET",
		"GET_MAPPED",
		"SET_MAPPED",
		"SET_TRUE",
		"SET_FALSE",
		"OPERATOR",
		"ASSIGN",
		"CONSTRUCT_BUILTIN",
		"CONSTRUCT_NATIVE",
		"CONSTRUCT_CARBON",
		"CONSTRUCT_LITERAL_ARRAY",
		"CONSTRUCT_LITERAL_DICT",
		"CALL",
		"CALL_FUNC",
		"CALL_METHOD",
		"CALL_BUILTIN",
		"CALL_SUPER_CTOR",
		"CALL_SUPER_METHOD",
		"JUMP",
		"JUMP_IF",
		"JUMP_IF_NOT",
		"RETURN",
		"ITER_BEGIN",
		"ITER_NEXT",
		"END",
	};
	MISSED_ENUM_CHECK(END, 25);
	return _names[p_opcode];
}

String Address::get_type_name_s(Address::Type p_type) {
	static const char* _names[] = {
		"Address::_NULL",
		"Address::STACK",
		"Address::PARAMETER",
		"Address::THIS",
		"Address::EXTERN",
		"Address::NATIVE_CLASS",
		"Address::BUILTIN_FUNC",
		"Address::BUILTIN_TYPE",
		"Address::MEMBER_VAR",
		"Address::STATIC_MEMBER",
		"Address::CONST_VALUE",
	};
	MISSED_ENUM_CHECK(CONST_VALUE, 10);
	return _names[p_type];
}

String Address::as_string(const stdvec<String>* _global_names_array, const stdvec<var>* _global_const_values) const {

	switch (type) {
		case _NULL:
		case THIS:
			return get_type_name_s(type);

		case STACK:
			return get_type_name_s(type) + "(" + std::to_string(index) + ")";

		case PARAMETER:
			return get_type_name_s(type) + "(" + std::to_string(index) + ")";

		case EXTERN:
		case NATIVE_CLASS:
		case STATIC_MEMBER:
			return get_type_name_s(type) + "(" + std::to_string(index) + ")" + (
				(_global_names_array) ? String(" // \"") + (*_global_names_array)[index] + "\"" : "");
		case CONST_VALUE:
			if (_global_const_values) {
				if ((*_global_const_values)[index].get_type() == var::STRING) {
					return get_type_name_s(type) + "(" + std::to_string(index) + ")" + String(" // \"") + (*_global_const_values)[index].to_string() + "\"";
				} else {
					return get_type_name_s(type) + "(" + std::to_string(index) + ")" + String(" // ") + (*_global_const_values)[index].to_string();
				}
			} else {
				return get_type_name_s(type) + "(" + std::to_string(index) + ")";
			}

		case BUILTIN_FUNC:
			return get_type_name_s(type) + "(" + std::to_string(index) + ") // " + BuiltinFunctions::get_func_name((BuiltinFunctions::Type)index);
		case BUILTIN_TYPE:
			return get_type_name_s(type) + "(" + std::to_string(index) + ") // " + BuiltinTypes::get_type_name((BuiltinTypes::Type)index);

		case MEMBER_VAR:
			return get_type_name_s(type) + "(" + std::to_string(index) + ")";
	}
	MISSED_ENUM_CHECK(CONST_VALUE, 10);
	THROW_BUG("can't reach here");
}

void Opcodes::insert_dbg(uint32_t p_line) {
	(*op_dbg)[next()] = p_line;
}

uint32_t Opcodes::last() { return (uint32_t)opcodes.size() - 1; }
uint32_t Opcodes::next() { return (uint32_t)opcodes.size(); }

void Opcodes::insert(uint32_t p_opcode) {
	opcodes.push_back(p_opcode);
}
void Opcodes::insert(const Address& p_addr) {
	opcodes.push_back(p_addr.get_address());
}
void Opcodes::insert(Opcode p_opcode) {
	opcodes.push_back((uint32_t)p_opcode);
}

void Opcodes::write_assign(const Address& dst, const Address& src) {
	insert(Opcode::ASSIGN);
	insert(dst);
	insert(src);
}

void Opcodes::write_if(const Address& p_cond) {
	insert(Opcode::JUMP_IF_NOT);
	insert(p_cond);
	insert(0); // addr to jump out of if.

	jump_out_if.push(last());
}

void Opcodes::write_else() {
	insert(Opcode::JUMP);
	insert(0); // addr to jump out of else

	// if not jump to next();
	opcodes[jump_out_if.top()] = next();
	jump_out_if.pop();
	jump_out_if.push(last());
}

void Opcodes::write_endif() {
	opcodes[jump_out_if.top()] = next();
	jump_out_if.pop();
}

void Opcodes::write_while(const Address& p_cond) {
	//jump_to_continue.push(next()); <-- above the condition experssion.
	jump_out_break.push(stdvec<uint32_t>());

	insert(Opcode::JUMP_IF_NOT);
	insert(p_cond);
	insert(0); // addr to jump out of while.

	jump_out_while.push(last());
}

void Opcodes::write_endwhile() {
	insert(Opcode::JUMP);
	insert(jump_to_continue.top());

	jump_to_continue.pop();
	opcodes[jump_out_while.top()] = next();
	jump_out_while.pop();

	for (uint32_t pos : jump_out_break.top()) {
		opcodes[pos] = next();
	}
	jump_out_break.pop();
}

void Opcodes::write_foreach(const Address& p_iter_value, const Address& p_iterator, const Address& p_on) {

	insert(Opcode::ITER_BEGIN);
	insert(p_iterator);
	insert(p_on);

	jump_to_continue.push(next());
	jump_out_break.push(stdvec<uint32_t>());

	// check if can iterate more
	insert(Opcode::ITER_NEXT);
	insert(p_iter_value);
	insert(p_iterator);
	insert(0); // addr to jump out of foreach;

	jump_out_foreach.push(last());
}

void Opcodes::write_endforeach() {
	insert(Opcode::JUMP);
	insert(jump_to_continue.top());

	jump_to_continue.pop();
	opcodes[jump_out_foreach.top()] = next();
	jump_out_foreach.pop();

	for (uint32_t pos : jump_out_break.top()) {
		opcodes[pos] = next();
	}
	jump_out_break.pop();
}

void Opcodes::write_for(const Address& p_cond) {

	// jump_to_continue.push(next()); // <-- above the condition expression
	jump_out_break.push(stdvec<uint32_t>());

	if (p_cond.get_type() != Address::_NULL) { // for (;;) {}
		insert(Opcode::JUMP_IF_NOT);
		insert(p_cond);
		insert(0); // addr to jump out of for;

		jump_out_for.push(last());
	}
}

void Opcodes::write_endfor(bool p_has_cond) {
	insert(Opcode::JUMP);
	insert(jump_to_continue.top());

	jump_to_continue.pop();

	if (p_has_cond) {
		opcodes[jump_out_for.top()] = next();
		jump_out_for.pop();
	}

	for (uint32_t pos : jump_out_break.top()) {
		opcodes[pos] = next();
	}
	jump_out_break.pop();
}

void Opcodes::write_break() {
	insert(Opcode::JUMP);
	insert(0); // addr to jump out.

	jump_out_break.top().push_back(last());
}

void Opcodes::write_continue() {
	insert(Opcode::JUMP);
	insert(jump_to_continue.top());
}

void Opcodes::write_return(const Address& p_ret_value) {
	insert(Opcode::RETURN);
	insert(p_ret_value);
}

void Opcodes::write_assign_bool(const Address& dst, bool value) {
	if (value) insert(Opcode::SET_TRUE);
	else insert(Opcode::SET_FALSE);
	insert(dst);
}

void Opcodes::write_and_left(const Address& p_left) {
	insert(Opcode::JUMP_IF_NOT);
	insert(p_left);
	insert(0); // addr to jump out if false

	jump_out_and.push(last());
}

void Opcodes::write_and_right(const Address& p_right, const Address& p_dst) {
	insert(Opcode::JUMP_IF_NOT);
	insert(p_right);
	insert(0); // <-- if false jump over the set_true below

	uint32_t above = last();

	insert(Opcode::SET_TRUE);
	insert(p_dst);

	opcodes[above] = next();
	opcodes[jump_out_and.top()] = next();
	jump_out_and.pop();
}


void Opcodes::write_or_left(const Address& p_left) {
	insert(Opcode::JUMP_IF);
	insert(p_left);
	insert(0); // <-- if true jump out

	jump_out_or.push(last());
}
void Opcodes::write_or_right(const Address& p_right, const Address& p_dst) {
	insert(Opcode::JUMP_IF);
	insert(p_right);
	insert(0); // <-- if true jump over the set_false

	uint32_t above = last();

	insert(Opcode::SET_FALSE);
	insert(p_dst);

	opcodes[above] = next();
	opcodes[jump_out_or.top()] = next();
	jump_out_or.pop();
}

/// expressions

//void Opcodes::write_get_member(const Address& p_member, const Address& p_dst) {
//	insert(Opcode::GET_MEMBER);
//	insert(p_member);
//	insert(p_dst);
//}

void Opcodes::write_get_index(const Address& p_on, uint32_t p_name, const Address& p_dst) {
	insert(Opcode::GET);
	insert(p_on);
	insert(p_name);
	insert(p_dst);
}

void Opcodes::write_set_index(const Address& p_on, uint32_t p_name, const Address& p_value) {
	insert(Opcode::SET);
	insert(p_on);
	insert(p_name);
	insert(p_value);
}

void Opcodes::write_get_mapped(const Address& p_on, const Address& p_key, const Address& p_dst) {
	insert(Opcode::GET_MAPPED);
	insert(p_on);
	insert(p_key);
	insert(p_dst);
}

void Opcodes::write_set_mapped(const Address& p_on, const Address& p_key, const Address& p_value) {
	insert(Opcode::SET_MAPPED);
	insert(p_on);
	insert(p_key);
	insert(p_value);
}

void Opcodes::write_array_literal(const Address& p_dst, const stdvec<Address>& p_values) {
	insert(Opcode::CONSTRUCT_LITERAL_ARRAY);
	insert((uint32_t)p_values.size());
	for (const Address& addr : p_values) insert(addr);
	insert(p_dst);
}

void Opcodes::write_map_literal(const Address& p_dst, const stdvec<Address>& p_keys, const stdvec<Address>& p_values) {
	ASSERT(p_keys.size() == p_values.size());

	insert(Opcode::CONSTRUCT_LITERAL_MAP);
	insert((uint32_t)p_keys.size());
	for (int i = 0; i < (int)p_keys.size(); i++) {
		insert(p_keys[i]);
		insert(p_values[i]);
	}
	insert(p_dst);
}

void Opcodes::write_construct_builtin_type(const Address& p_dst, BuiltinTypes::Type p_type, const stdvec<Address>& p_args) {
	insert(Opcode::CONSTRUCT_BUILTIN);
	insert((uint32_t)p_type);
	insert((uint32_t)p_args.size());
	for (const Address& addr : p_args) {
		insert(addr);
	}
	insert(p_dst);
}

void Opcodes::write_construct_native(const Address& p_dst, uint32_t p_name, const stdvec<Address>& p_args) {
	insert(Opcode::CONSTRUCT_NATIVE);
	insert(p_name);
	insert((uint32_t)p_args.size());
	for (const Address& addr : p_args) {
		insert(addr);
	}
	insert(p_dst);
}
void Opcodes::write_construct_carbon(const Address& p_dst, uint32_t p_name, const stdvec<Address>& p_args) {
	insert(Opcode::CONSTRUCT_CARBON);
	insert(p_name);
	insert((uint32_t)p_args.size());
	for (const Address& addr : p_args) {
		insert(addr);
	}
	insert(p_dst);
}

void Opcodes::write_call_builtin(const Address& p_ret, BuiltinFunctions::Type p_func, const stdvec<Address>& p_args) {
	insert(Opcode::CALL_BUILTIN);
	insert((uint32_t)p_func);
	insert((uint32_t)p_args.size());
	for (const Address& addr : p_args) {
		insert(addr);
	}
	insert(p_ret);
}

void Opcodes::write_call(const Address& p_ret, const Address& p_on, const stdvec<Address>& p_args) {
	insert(Opcode::CALL);
	insert(p_on);
	insert((uint32_t)p_args.size());
	for (const Address& addr : p_args) {
		insert(addr);
	}
	insert(p_ret);
}

void Opcodes::write_call_func(const Address& p_ret, uint32_t p_name, const stdvec<Address>& p_args) {
	insert(Opcode::CALL_FUNC);
	insert(p_name);
	insert((uint32_t)p_args.size());
	for (const Address& addr : p_args) {
		insert(addr);
	}
	insert(p_ret);
}

void Opcodes::write_call_method(const Address& p_ret, Address& p_on, uint32_t p_method, const stdvec<Address>& p_args) {
	insert(Opcode::CALL_METHOD);
	insert(p_on);
	insert(p_method);
	insert((uint32_t)p_args.size());
	for (const Address& addr : p_args) {
		insert(addr);
	}
	insert(p_ret);
}

void Opcodes::write_call_super_constructor(const stdvec<Address>& p_args) {
	insert(Opcode::CALL_SUPER_CTOR);
	insert((uint32_t)p_args.size());
	for (const Address& addr : p_args) {
		insert(addr);
	}
}

void Opcodes::write_call_super_method(const Address& p_ret, uint32_t p_method, const stdvec<Address>& p_args) {
	insert(Opcode::CALL_SUPER_METHOD);
	insert(p_method);
	insert((uint32_t)p_args.size());
	for (const Address& addr : p_args) {
		insert(addr);
	}
	insert(p_ret);
}

void Opcodes::write_operator(const Address& p_dst, var::Operator p_op, const Address& p_left, const Address& p_right) {
	insert(Opcode::OPERATOR);
	insert((uint32_t)p_op);
	insert(p_left);
	insert(p_right);
	insert(p_dst);
}


}

#endif //_FILE_SRC_COMPILER_OPCODES_CPP_

#ifndef _FILE_SRC_COMPILER_PARSER_CPP_



/******************************************************************************************************************/
/*                                         PARSER                                                                 */
/******************************************************************************************************************/

namespace carbon {

CompileTimeError Parser::_unexp_token_error(const char* p_exptected, const DBGSourceInfo& p_dbg_info) const {
	Error::Type err_type = Error::SYNTAX_ERROR;
	if (tokenizer->peek(-1, true).type == Token::_EOF) err_type = Error::UNEXPECTED_EOF;
	if (p_exptected != nullptr) {
		return _parser_error(err_type, String::format("unexpected token(\"%s\"). expected %s.",
				Tokenizer::get_token_name(tokenizer->peek(-1, true).type), p_exptected).c_str(), Vect2i(), p_dbg_info);
	} else {
		return _parser_error(err_type, String::format("unexpected token(\"%s\").",
			Tokenizer::get_token_name(tokenizer->peek(-1, true).type)).c_str(), Vect2i(), p_dbg_info);
	}    
}

CompileTimeError Parser::_parser_error(Error::Type p_type, const String& p_msg, Vect2i p_pos, const DBGSourceInfo& p_dbg_info) const {
	String token_str = "";
	if (p_pos.x > 0 && p_pos.y > 0) token_str = tokenizer->get_token_at(p_pos).to_string();
	else token_str = tokenizer->peek(-1, true).to_string();
	uint32_t err_len = (uint32_t)token_str.size();

	Vect2i pos = (p_pos.x > 0 && p_pos.y > 0) ? p_pos : tokenizer->peek(-1, true).get_pos();
	return CompileTimeError(p_type, p_msg, DBGSourceInfo(file_node->path, file_node->source,
		std::pair<int, int>((int)pos.x, (int)pos.y), err_len), p_dbg_info);
}

CompileTimeError Parser::_predefined_error(const String& p_what, const String& p_name, Vect2i p_pos, const DBGSourceInfo& p_dbg_info) const {
	return _parser_error(Error::NAME_ERROR,
		String::format((p_what + " named \"%s\" already exists at (line:%i, col:%i)").c_str(), p_name.c_str(), p_pos.x, p_pos.y),
		Vect2i(), p_dbg_info);
}

void Parser::parse(ptr<Tokenizer> p_tokenizer) {
	
	tokenizer = p_tokenizer;
	file_node = new_node<FileNode>();

	// TODO: maybe redundant
	file_node->source = tokenizer->get_source();
	file_node->path = tokenizer->get_source_path();

	while (true) {
	
		const TokenData& token = tokenizer->next();
		switch (token.type) {
			case  Token::_EOF:
				return;

			case Token::KWORD_IMPORT: {
				file_node->imports.push_back(_parse_import());
			} break;

			case Token::KWORD_CLASS: {
				file_node->classes.push_back(_parse_class());
			} break;

			case Token::KWORD_ENUM: {
				ptr<EnumNode> _enum = _parse_enum(file_node);
				if (_enum->named_enum) {
					file_node->enums.push_back(_enum);
				} else {
					if (file_node->unnamed_enum == nullptr) {
						file_node->unnamed_enum = _enum;
					} else {
						for (auto it = _enum->values.begin(); it != _enum->values.end(); it++) {
							file_node->unnamed_enum->values[it->first] = it->second;
						}
					}
				}
			} break;

			case Token::KWORD_FUNC: {
				ptr<FunctionNode> func = _parse_func(file_node);
				file_node->functions.push_back(func);
			} break;

			case Token::KWORD_VAR: {
				stdvec<ptr<VarNode>> vars = _parse_var(file_node);
				for (ptr<VarNode>& _var : vars) {
					file_node->vars.push_back(_var);
				}
			} break;

			case Token::KWORD_CONST: {
				ptr<ConstNode> _const = _parse_const(file_node);
				file_node->constants.push_back(_const);
			} break;

			// Ignore.
			case Token::SYM_SEMI_COLLON: 
			case Token::VALUE_STRING:
				break;

			// compile time function call.
			case Token::IDENTIFIER: {
				BuiltinFunctions::Type builtin_func = BuiltinFunctions::get_func_type(token.identifier);
				if (builtin_func != BuiltinFunctions::UNKNOWN && BuiltinFunctions::is_compiletime(builtin_func)) {
					ptr<CallNode> call = new_node<CallNode>();
					call->base = new_node<BuiltinFunctionNode>(builtin_func);
					if (tokenizer->next().type != Token::BRACKET_LPARAN) throw UNEXP_TOKEN_ERROR("symbol \"(\"");
					call->args = _parse_arguments(file_node);
					call->is_compilttime = true;
					file_node->compiletime_functions.push_back(call);
					break;
				}
			} // [[fallthrough]]

			default:
				throw UNEXP_TOKEN_ERROR(nullptr);
		}

	} // while true
}

void Parser::_check_identifier_predefinition(const String& p_name, Node* p_scope) const {
	const TokenData* tk = &tokenizer->peek(-1, true);

	if (NativeClasses::singleton()->is_class_registered(p_name)) {
		throw PARSER_ERROR(Error::NAME_ERROR, String::format("a native type named %s already exists", p_name.c_str()), Vect2i());
	}

	for (ptr<ImportNode>& in : file_node->imports) {
		if (p_name == in->name) throw PREDEFINED_ERROR("an imported file", p_name, in->pos);
	}

	if (p_scope == nullptr || p_scope->type == Node::Type::CLASS || p_scope->type == Node::Type::FILE) {
		const MemberContainer* scope = nullptr;

		if (p_scope == nullptr) scope = file_node.get();
		else scope = static_cast<const MemberContainer*>(p_scope);

		for (int i = 0; i < (int)scope->vars.size(); i++) {
			if (scope->vars[i]->name == p_name) {
				throw PREDEFINED_ERROR("a var", scope->vars[i]->name, scope->vars[i]->pos);
			}
		}

		for (int i = 0; i < (int)scope->constants.size(); i++) {
			if (scope->constants[i]->name == p_name) {
				throw PREDEFINED_ERROR("a constant", scope->constants[i]->name, scope->constants[i]->pos);
			}
		}

		for (int i = 0; i < (int)scope->functions.size(); i++) {
			if (scope->functions[i]->name == p_name) {
				throw PREDEFINED_ERROR("a function", scope->functions[i]->name, scope->functions[i]->pos);
			}
		}

		for (int i = 0; i < (int)scope->enums.size(); i++) {
			if (scope->enums[i]->name == p_name) {
				throw PREDEFINED_ERROR("an enum", scope->enums[i]->name, scope->enums[i]->pos);
			}
		}

		if (scope->unnamed_enum != nullptr) {
			for (auto it = scope->unnamed_enum->values.begin(); it != scope->unnamed_enum->values.end(); it++) {
				if (it->first == p_name) {
					throw PREDEFINED_ERROR("an enum value", p_name, it->second.pos);
				}
			}
		}

		if (scope->type == Node::Type::FILE) {
			const Parser::FileNode* p_file = static_cast<const Parser::FileNode*>(scope);
			for (int i = 0; i < (int)p_file->classes.size(); i++) {
				if (p_file->classes[i]->name == p_name) {
					throw PREDEFINED_ERROR("a classe", p_file->classes[i]->name, p_file->classes[i]->pos);
				}
			}
		}
	} else if (p_scope->type == Node::Type::BLOCK) {
		ASSERT(parser_context.current_func != nullptr);
		for (int i = 0; i < (int)parser_context.current_func->args.size(); i++) {
			if (parser_context.current_func->args[i].name == p_name) {
				throw PREDEFINED_ERROR("an argument", p_name, parser_context.current_func->args[i].pos);
			}
		}
		const BlockNode* block = static_cast<const BlockNode*>(p_scope);
		while (block) {
			for (int i = 0; i < (int)block->local_vars.size(); i++) {
				if (block->local_vars[i]->name == p_name) {
					throw  PREDEFINED_ERROR("a variable", p_name, block->local_vars[i]->pos);
				}
			}
			if (block->parernt_node->type == Node::Type::FUNCTION) break;
			block = ptrcast<BlockNode>(block->parernt_node).get();
		}
	} else {
		ASSERT(false);
	}
}

ptr<Parser::ImportNode> Parser::_parse_import() {
	ASSERT(tokenizer->peek(-1).type == Token::KWORD_IMPORT);

	ptr<ImportNode> import_node = new_node<ImportNode>();

	const TokenData* tk = &tokenizer->next();
	if (tk->type != Token::IDENTIFIER) throw UNEXP_TOKEN_ERROR("an identifier");

	String name = tk->identifier;
	_check_identifier_predefinition(name, nullptr);
	import_node->name = name;

	if (tokenizer->next().type != Token::OP_EQ) throw UNEXP_TOKEN_ERROR("symbol \"=\"");
	tk = &tokenizer->next();
	if (tk->type != Token::VALUE_STRING) throw UNEXP_TOKEN_ERROR("string path to source");
	String path = tk->constant.operator String();

	import_node->bytecode = Compiler::singleton()->compile(path);

	tk = &tokenizer->next();
	if (tk->type != Token::SYM_SEMI_COLLON) throw UNEXP_TOKEN_ERROR("symbol \";\"");

	return import_node;
}

ptr<Parser::ClassNode> Parser::_parse_class() {
	ASSERT(tokenizer->peek(-1).type == Token::KWORD_CLASS);
	ptr<ClassNode> class_node = new_node<ClassNode>();
	class_node->parernt_node = file_node;

	parser_context.current_class = class_node.get();
	class ScopeDestruct {
	public:
		Parser::ParserContext* context = nullptr;
		ScopeDestruct(Parser::ParserContext* p_context) {
			context = p_context;
		}
		~ScopeDestruct() {
			context->current_class = nullptr;
		}
	};
	ScopeDestruct destruct = ScopeDestruct(&parser_context);

	const TokenData* tk = &tokenizer->next();
	if (tk->type != Token::IDENTIFIER) throw UNEXP_TOKEN_ERROR("an identifier");
	_check_identifier_predefinition(tk->identifier, nullptr);

	class_node->name = tk->identifier;

	// Inheritance.
	tk = &tokenizer->next();
	if (tk->type == Token::SYM_COLLON) {

		tk = &tokenizer->next();
		if (tk->type == Token::BUILTIN_TYPE) throw PARSER_ERROR(Error::TYPE_ERROR, "cannot inherit a builtin type.", Vect2i());
		if (tk->type != Token::IDENTIFIER) throw UNEXP_TOKEN_ERROR("an identifier");
		class_node->base_class_name = tk->identifier;

		tk = &tokenizer->next();
		if (tk->type == Token::SYM_DOT) {
			tk = &tokenizer->next();
			if (tk->type != Token::IDENTIFIER) throw UNEXP_TOKEN_ERROR("an identifier");

			String base_file_name  = class_node->base_class_name;
			String base_class_name = tk->identifier;
			class_node->base_type = ClassNode::BASE_EXTERN;

			Bytecode* base_file = nullptr;
			for (ptr<ImportNode>& in : file_node->imports) {
				if (in->name == base_file_name) {
					base_file = in->bytecode.get();
					break;
				}
			}
			if (!base_file)
				throw PARSER_ERROR(Error::NAME_ERROR,
					String::format("base file name \"%s\" not found from the imported libs.", base_file_name.c_str()), Vect2i());

			ptr<Bytecode> base_binary = nullptr;
			for (const std::pair<String, ptr<Bytecode>>& cls : base_file->get_classes()) {
				if (cls.first == base_class_name) {
					base_binary = cls.second;
					break;
				}
			}
			if (!base_binary) {
				throw PARSER_ERROR(Error::NAME_ERROR,
					String::format("base class name \"%s\" not found from the imported lib \"%s\".", base_class_name.c_str(), base_file_name.c_str()),
					Vect2i());
			}

			class_node->base_binary = base_binary;
			class_node->base_class_name = base_class_name;

			tk = &tokenizer->next();
		} else {
			if (NativeClasses::singleton()->is_class_registered(class_node->base_class_name)) {
				class_node->base_type = ClassNode::BASE_NATIVE;
			} else {
				if (class_node->base_class_name == class_node->name)
					throw PARSER_ERROR(Error::TYPE_ERROR, "cyclic inheritance. class inherits itself isn't allowed.", tokenizer->peek(-2, true).get_pos());
				class_node->base_type = ClassNode::BASE_LOCAL;
			}
		}
	}

	if (tk->type != Token::BRACKET_LCUR) throw UNEXP_TOKEN_ERROR("symbol \"{\"");
	
	while (true) {
		const TokenData& token = tokenizer->next();
		switch (token.type) {

			case Token::_EOF: {
				throw PARSER_ERROR(Error::UNEXPECTED_EOF, "Unexpected end of file.", Vect2i());
			} break;

			case Token::BRACKET_RCUR: {
				return class_node;
			}

			case Token::SYM_SEMI_COLLON: { // ignore
			} break;

			case Token::KWORD_ENUM: {
				ptr<EnumNode> _enum = _parse_enum(class_node);
				if (_enum->named_enum) {
					class_node->enums.push_back(_enum);
				} else {
					if (class_node->unnamed_enum == nullptr) {
						class_node->unnamed_enum = _enum;
					} else {
						for (auto it = _enum->values.begin(); it != _enum->values.end(); it++) {
							class_node->unnamed_enum->values[it->first] = it->second;
						}
					}
				}
			} break;

			case Token::KWORD_STATIC: {
				if (tokenizer->peek().type != Token::KWORD_FUNC && tokenizer->peek().type != Token::KWORD_VAR) {
					throw PARSER_ERROR(Error::SYNTAX_ERROR, "expected keyword \"func\" or \"var\" after static", Vect2i());
				}

			} break;

			case Token::KWORD_FUNC: {
				ptr<FunctionNode> func = _parse_func(class_node);
				class_node->functions.push_back(func);
			} break;

			case Token::KWORD_VAR: {
				stdvec<ptr<VarNode>> vars = _parse_var(class_node);
				for (ptr<VarNode>& _var : vars) {
					class_node->vars.push_back(_var);
				}
			} break;

			case Token::KWORD_CONST: {
				ptr<ConstNode> _const = _parse_const(class_node);
				class_node->constants.push_back(_const);
			} break;

			// compile time function call.
			case Token::IDENTIFIER: {

				ptr<CallNode> call = new_node<CallNode>();
				BuiltinFunctions::Type builtin_func = BuiltinFunctions::get_func_type(token.identifier);
				if (builtin_func != BuiltinFunctions::UNKNOWN && BuiltinFunctions::is_compiletime(builtin_func)) {
					call->base = new_node<BuiltinFunctionNode>(builtin_func);
					if (tokenizer->next().type != Token::BRACKET_LPARAN) throw UNEXP_TOKEN_ERROR("symbol \"(\"");
					call->args = _parse_arguments(class_node);
					call->is_compilttime = true;
					class_node->compiletime_functions.push_back(call);
					break;
				}
			} // [[fallthrough]]
			default: {
				throw UNEXP_TOKEN_ERROR(nullptr);
			}
		}
	}
}

ptr<Parser::EnumNode> Parser::_parse_enum(ptr<Node> p_parent) {
	ASSERT(tokenizer->peek(-1).type == Token::KWORD_ENUM);
	ASSERT(p_parent->type == Node::Type::FILE || p_parent->type == Node::Type::CLASS);

	ptr<EnumNode> enum_node = new_node<EnumNode>();
	enum_node->parernt_node = p_parent;

	parser_context.current_enum = enum_node.get();
	class ScopeDestruct {
	public:
		Parser::ParserContext* context = nullptr;
		ScopeDestruct(Parser::ParserContext* p_context) {
			context = p_context;
		}
		~ScopeDestruct() {
			context->current_enum = nullptr;
		}
	};
	ScopeDestruct destruct = ScopeDestruct(&parser_context);

	const TokenData* tk = &tokenizer->next();
	if (tk->type != Token::IDENTIFIER && tk->type != Token::BRACKET_LCUR)
		throw UNEXP_TOKEN_ERROR("an identifier or symbol \"{\"");	

	if (tk->type == Token::IDENTIFIER) {
		_check_identifier_predefinition(tk->identifier, p_parent.get());

		enum_node->name = tk->identifier;
		enum_node->named_enum = true;
		tk = &tokenizer->next();
	}

	if (tk->type != Token::BRACKET_LCUR) throw UNEXP_TOKEN_ERROR("symbol \"{\"");

	bool comma_valid = false;
	int64_t next_value = 0;
	while (true) {
		const TokenData& token = tokenizer->next();
		switch (token.type) {

			case Token::_EOF: {
				throw PARSER_ERROR(Error::UNEXPECTED_EOF, "Unexpected end of file.", Vect2i());
			} break;

			case Token::BRACKET_RCUR: {
				return enum_node;
			} break;

			case Token::SYM_COMMA: {
				if (!comma_valid) throw UNEXP_TOKEN_ERROR("an identifier or symbol \"}\"");
				comma_valid = false;
			} break;

			case Token::IDENTIFIER: {
				for (const std::pair<String, EnumValueNode>& value : enum_node->values) {
					if (value.first == token.identifier) throw PREDEFINED_ERROR("an enum value", value.first, value.second.pos);
				}

				if (!enum_node->named_enum) {

					// TODO: check if it's compile time function.
					//BuiltinFunctions::Type builtin_func = BuiltinFunctions::get_func_type(token.identifier);
					//if (builtin_func != BuiltinFunctions::UNKNOWN && BuiltinFunctions::is_compiletime(builtin_func)) {
					//	ptr<CallNode> call = new_node<CallNode>();
					//	call->base = new_node<BuiltinFunctionNode>(builtin_func);
					//	if (tokenizer->next().type != Token::BRACKET_LPARAN) throw UNEXP_TOKEN_ERROR("symbol \"(\"");
					//	call->args = _parse_arguments(file_node);
					//	call->is_compilttime = true;
					//	if (parser_context.current_class != nullptr) parser_context.current_class->compiletime_functions.push_back(call);
					//	else file_node->compiletime_functions.push_back(call);
					//	break;
					//}

					_check_identifier_predefinition(token.identifier, p_parent.get());
				}
				
				const TokenData* tk = &tokenizer->peek();
				if (tk->type == Token::OP_EQ) {
					tk = &tokenizer->next(); // eat "=".
					ptr<Node> expr = _parse_expression(enum_node, false);
					enum_node->values[token.identifier] = EnumValueNode(expr, token.get_pos(), (enum_node->named_enum) ? enum_node.get() : nullptr);
				} else {
					enum_node->values[token.identifier] = EnumValueNode(nullptr, token.get_pos(), (enum_node->named_enum) ? enum_node.get() : nullptr);
				}

				comma_valid = true;
			} break;

			default: {
				throw UNEXP_TOKEN_ERROR("an identifier");
			} break;
		}
	}
}

stdvec<ptr<Parser::VarNode>> Parser::_parse_var(ptr<Node> p_parent) {
	ASSERT(tokenizer->peek(-1).type == Token::KWORD_VAR);
	ASSERT(p_parent != nullptr);
	ASSERT(p_parent->type == Node::Type::FILE || p_parent->type == Node::Type::BLOCK || p_parent->type == Node::Type::CLASS);

	bool _static = p_parent->type == Node::Type::FILE || tokenizer->peek(-2, true).type == Token::KWORD_STATIC;

	const TokenData* tk;
	stdvec<ptr<VarNode>> vars;

	while (true) {
		tk = &tokenizer->next();

		if (tk->type != Token::IDENTIFIER) throw UNEXP_TOKEN_ERROR("an identifier");
		_check_identifier_predefinition(tk->identifier, p_parent.get());

		ptr<VarNode> var_node = new_node<VarNode>();
		var_node->parernt_node = p_parent;
		var_node->is_static = _static;
		var_node->name = tk->identifier;

		parser_context.current_var = var_node.get();
		class ScopeDestruct {
		public:
			Parser::ParserContext* context = nullptr;
			ScopeDestruct(Parser::ParserContext* p_context) {
				context = p_context;
			}
			~ScopeDestruct() {
				context->current_var = nullptr;
			}
		};
		ScopeDestruct destruct = ScopeDestruct(&parser_context);

		tk = &tokenizer->next();
		if (tk->type == Token::OP_EQ) {
			ptr<Node> expr = _parse_expression(p_parent, false);
			var_node->assignment = expr;

			tk = &tokenizer->next();
			if (tk->type == Token::SYM_COMMA) {
			} else if (tk->type == Token::SYM_SEMI_COLLON) {
				vars.push_back(var_node);
				break;
			} else {
				throw UNEXP_TOKEN_ERROR("symbol \",\" or \";\"");
			}
		} else if (tk->type == Token::SYM_COMMA) {
		} else if (tk->type == Token::SYM_SEMI_COLLON) {
			vars.push_back(var_node);
			break;
		} else {
			throw UNEXP_TOKEN_ERROR("symbol \",\" or \";\"");
		}
		vars.push_back(var_node);
	}
	return vars;
}

ptr<Parser::ConstNode> Parser::_parse_const(ptr<Node> p_parent) {
	ASSERT(tokenizer->peek(-1).type == Token::KWORD_CONST);
	ASSERT(p_parent != nullptr);
	ASSERT(p_parent->type == Node::Type::FILE || p_parent->type == Node::Type::BLOCK || p_parent->type == Node::Type::CLASS);

	const TokenData* tk;
	tk = &tokenizer->next();

	if (tk->type != Token::IDENTIFIER) throw UNEXP_TOKEN_ERROR("an identifier");
	_check_identifier_predefinition(tk->identifier, p_parent.get());

	ptr<ConstNode> const_node = new_node<ConstNode>();
	const_node->parernt_node = p_parent;
	const_node->name = tk->identifier;

	parser_context.current_const = const_node.get();
	class ScopeDestruct {
	public:
		Parser::ParserContext* context = nullptr;
		ScopeDestruct(Parser::ParserContext* p_context) {
			context = p_context;
		}
		~ScopeDestruct() {
			context->current_const = nullptr;
		}
	};
	ScopeDestruct destruct = ScopeDestruct(&parser_context);

	tk = &tokenizer->next();
	if (tk->type != Token::OP_EQ) throw UNEXP_TOKEN_ERROR("symbol \"=\"");
	ptr<Node> expr = _parse_expression(p_parent, false);
	const_node->assignment = expr;

	tk = &tokenizer->next();
	if (tk->type != Token::SYM_SEMI_COLLON) throw UNEXP_TOKEN_ERROR("symbol \";\"");

	return const_node;
}

ptr<Parser::FunctionNode> Parser::_parse_func(ptr<Node> p_parent) {
	ASSERT(tokenizer->peek(-1).type == Token::KWORD_FUNC);
	ASSERT(p_parent->type == Node::Type::FILE || p_parent->type == Node::Type::CLASS);

	ptr<FunctionNode> func_node = new_node<FunctionNode>();
	func_node->parent_node = p_parent.get();
	if (p_parent->type == Node::Type::FILE || tokenizer->peek(-2, true).type == Token::KWORD_STATIC) {
		func_node->is_static = true;
	}

	parser_context.current_func = func_node.get();
	class ScopeDestruct {
	public:
		Parser::ParserContext* context = nullptr;
		ScopeDestruct(Parser::ParserContext* p_context) {
			context = p_context;
		}
		~ScopeDestruct() {
			context->current_func = nullptr;
		}
	};
	ScopeDestruct destruct = ScopeDestruct(&parser_context);

	const TokenData* tk = &tokenizer->next();
	if (tk->type != Token::IDENTIFIER) throw UNEXP_TOKEN_ERROR("an identifier");
	_check_identifier_predefinition(tk->identifier, p_parent.get());

	func_node->name = tk->identifier;
	if (parser_context.current_class && parser_context.current_class->name == tk->identifier) {
		func_node->is_constructor = true;
		parser_context.current_class->constructor = func_node.get();
	}

	tk = &tokenizer->next();
	if (tk->type != Token::BRACKET_LPARAN) throw UNEXP_TOKEN_ERROR("symbol \"(\"");
	tk = &tokenizer->next();

	bool has_default = false;
	if (tk->type != Token::BRACKET_RPARAN) {
		while (true) {
			if (tk->type != Token::IDENTIFIER) throw UNEXP_TOKEN_ERROR("an identifier");
			for (int i = 0; i < (int)func_node->args.size(); i++) {
				if (func_node->args[i].name == tk->identifier)
					throw PARSER_ERROR(Error::NAME_ERROR, String::format("identifier \"%s\" already defined in arguments", tk->identifier.c_str()), Vect2i());
			}

			ParameterNode parameter = ParameterNode(tk->identifier, tk->get_pos());
			tk = &tokenizer->next();

			if (tk->type == Token::OP_BIT_AND) {
				parameter.is_reference = true;
				tk = &tokenizer->next();
			}

			if (tk->type == Token::OP_EQ) {
				has_default = true;
				parameter.default_value = _parse_expression(p_parent, false);
				tk = &tokenizer->next();
			} else {
				if (has_default)
					throw PARSER_ERROR(Error::SYNTAX_ERROR, "default parameter expected.", Vect2i());
			}
			func_node->args.push_back(parameter);

			if (tk->type == Token::SYM_COMMA) {
				tk = &tokenizer->next(); // eat ','
			} else if (tk->type == Token::BRACKET_RPARAN) {
				break;
			} else {
				throw UNEXP_TOKEN_ERROR(nullptr);
			}
		}
	}

	const TokenData& _next = tokenizer->next();
	bool _single_expr = false;

	if (_next.type == Token::OP_EQ) {
		_single_expr = true;
	} else if (_next.type != Token::BRACKET_LCUR) {
		throw UNEXP_TOKEN_ERROR("symbol \"{\"");
	}

	// TODO: this could be used to inline.
	if (_single_expr) {

		if (parser_context.current_class && parser_context.current_class->constructor) {
			if (parser_context.current_class->constructor == parser_context.current_func) {
				throw PARSER_ERROR(Error::SYNTAX_ERROR, "constructor can't return anything.", _next.get_pos());
			}
		}

		ptr<BlockNode> block_node = newptr<BlockNode>();
		block_node->parernt_node = func_node;

		ptr<ControlFlowNode> _return = new_node<ControlFlowNode>(ControlFlowNode::RETURN);
		_return->args.push_back(_parse_expression(func_node, false));
		_return->parernt_node = func_node;
		_return->_return = parser_context.current_func;
		parser_context.current_func->has_return = true;
		block_node->statements.push_back(_return);

		tk = &tokenizer->next();
		if (tk->type != Token::SYM_SEMI_COLLON) throw UNEXP_TOKEN_ERROR("symbol \";\"");

		func_node->body = block_node;

	} else {
		func_node->body = _parse_block(func_node);
		if (tokenizer->next().type != Token::BRACKET_RCUR) {
			throw UNEXP_TOKEN_ERROR("symbol \"}\"");
		}
		func_node->end_line = (uint32_t)tokenizer->get_pos().x;
	}

	return func_node;
}

}

/******************************************************************************************************************/
/*                                         BLOCK                                                                  */
/******************************************************************************************************************/

namespace carbon {

ptr<Parser::BlockNode> Parser::_parse_block(const ptr<Node>& p_parent, bool p_single_statement, stdvec<Token> p_termination) {
	ptr<BlockNode> block_node = newptr<BlockNode>();
	block_node->parernt_node = p_parent;

	parser_context.current_block = block_node.get();
	class ScopeDestruct {
	public:
		Parser::ParserContext* context = nullptr;
		ScopeDestruct(Parser::ParserContext* p_context) {
			context = p_context;
		}
		~ScopeDestruct() {
			context->current_block = nullptr;
		}
	};
	ScopeDestruct distruct = ScopeDestruct(&parser_context);

	const TokenData* tk;

	while (true) {
		tk = &tokenizer->peek();
		switch (tk->type) {
			case Token::_EOF: {
				throw PARSER_ERROR(Error::UNEXPECTED_EOF, "Unexpected end of file.", Vect2i());
			} break;

			case Token::KWORD_VAR: {
				tokenizer->next(); // eat "var"
				stdvec<ptr<VarNode>> vars = _parse_var(block_node);
				for (ptr<VarNode>& _var : vars) {
					block_node->local_vars.push_back(_var); // for quick access.
					block_node->statements.push_back(_var);
				}
			} break;

			case Token::KWORD_CONST: {
				tokenizer->next(); // ear "const"
				ptr<ConstNode> _const = _parse_const(block_node);
				block_node->local_const.push_back(_const);
				block_node->statements.push_back(_const);
			} break;

				// Ignore.
			case Token::SYM_SEMI_COLLON:
				tokenizer->next(); // eat ";"
				break;

				// could be 1/2 or something, default case
				//case Token::VALUE_NULL:
				//case Token::VALUE_BOOL:
				//case Token::VALUE_INT:
				//case Token::VALUE_FLOAT:
				//case Token::VALUE_STRING: {
				//	tk = &tokenizer->next(); // will be ignored by analyzer
				//	ptr<ConstValueNode> value = new_node<ConstValueNode>(tk->constant);
				//	block_node->statements.push_back(value);
				//} break;

			case Token::KWORD_IF: {
				tokenizer->next(); // eat "if"
				block_node->statements.push_back(_parse_if_block(block_node));
			} break;

			case Token::KWORD_SWITCH: {
				tk = &tokenizer->next(); // eat "switch"
				ptr<ControlFlowNode> switch_block = new_node<ControlFlowNode>(ControlFlowNode::SWITCH);
				switch_block->parernt_node = p_parent;
				ControlFlowNode* outer_break = parser_context.current_break;
				parser_context.current_break = switch_block.get();

				switch_block->args.push_back(_parse_expression(block_node, false));
				if (tokenizer->next().type != Token::BRACKET_LCUR) throw UNEXP_TOKEN_ERROR("symbol \"{\"");

				while (true) {
					tk = &tokenizer->next();
					if (tk->type == Token::KWORD_CASE) {
						ControlFlowNode::SwitchCase _case;
						_case.pos = tk->get_pos();
						_case.expr = _parse_expression(block_node, false);
						if (tokenizer->next().type != Token::SYM_COLLON) throw UNEXP_TOKEN_ERROR("symbol \":\"");

						// COMMENTED: `case VALUE: { expr; expr; }` <--- curly brackets are not allowed.
						//tk = &tokenizer->peek();
						//if (tk->type == Token::BRACKET_LCUR) {
						//	tokenizer->next(); // eat "{"
						//	_case.body = _parse_block(block_node);
						//	if (tokenizer->next().type != Token::BRACKET_RCUR) throw UNEXP_TOKEN("symbol \"}\"");
						//} else {
						_case.body = _parse_block(block_node, false, { Token::KWORD_CASE, Token::KWORD_DEFAULT, Token::BRACKET_RCUR });
						//}
						switch_block->switch_cases.push_back(_case);

					} else if (tk->type == Token::KWORD_DEFAULT) {
						ControlFlowNode::SwitchCase _case;
						_case.default_case = true;
						if (tokenizer->next().type != Token::SYM_COLLON) throw UNEXP_TOKEN_ERROR("symbol \":\"");
						_case.body = _parse_block(block_node, false);
					} else if (tk->type == Token::BRACKET_RCUR) {
						break;
					} else {
						throw UNEXP_TOKEN_ERROR("keyword \"case\" or symbol \"}\"");
					}
				}
				block_node->statements.push_back(switch_block);
				parser_context.current_break = outer_break;
			} break;

			case Token::KWORD_WHILE: {
				tk = &tokenizer->next(); // eat "while"
				ptr<ControlFlowNode> while_block = new_node<ControlFlowNode>(ControlFlowNode::WHILE);

				ControlFlowNode* outer_break = parser_context.current_break;
				ControlFlowNode* outer_continue = parser_context.current_continue;
				parser_context.current_break = while_block.get();
				parser_context.current_continue = while_block.get();

				while_block->parernt_node = p_parent;
				while_block->args.push_back(_parse_expression(block_node, false));
				tk = &tokenizer->peek();
				if (tk->type == Token::BRACKET_LCUR) {
					tokenizer->next(); // eat "{"
					while_block->body = _parse_block(block_node);
					if (tokenizer->next().type != Token::BRACKET_RCUR) throw UNEXP_TOKEN_ERROR("symbol \"}\"");
				} else {
					while_block->body = _parse_block(block_node, true);
				}
				block_node->statements.push_back(while_block);
				parser_context.current_break = outer_break;
				parser_context.current_continue = outer_continue;
			} break;

			case Token::KWORD_FOR: {
				tk = &tokenizer->next(); // eat "for"
				ptr<ControlFlowNode> for_block = new_node<ControlFlowNode>(ControlFlowNode::FOR);
				ControlFlowNode* outer_break = parser_context.current_break;
				ControlFlowNode* outer_continue = parser_context.current_continue;
				parser_context.current_break = for_block.get();
				parser_context.current_continue = for_block.get();

				for_block->parernt_node = p_parent;
				if (tokenizer->next().type != Token::BRACKET_LPARAN) throw UNEXP_TOKEN_ERROR("symbol \"(\"");

				if (tokenizer->peek().type == Token::SYM_SEMI_COLLON) {
					tokenizer->next(); // eat ";"
					for_block->args.push_back(nullptr);
				} else {
					if (tokenizer->peek().type == Token::KWORD_VAR) {
						tokenizer->next(); // eat "var"

						tk = &tokenizer->next();
						if (tk->type != Token::IDENTIFIER) throw UNEXP_TOKEN_ERROR("an identifier");
						_check_identifier_predefinition(tk->identifier, block_node.get());

						ptr<VarNode> var_node = new_node<VarNode>();
						var_node->parernt_node = p_parent;
						var_node->name = tk->identifier;

						tk = &tokenizer->next();
						if (tk->type == Token::OP_EQ) {
							parser_context.current_var = var_node.get();
							ptr<Node> expr = _parse_expression(p_parent, false);
							parser_context.current_var = nullptr;
							var_node->assignment = expr;
							if (tokenizer->next().type != Token::SYM_SEMI_COLLON) throw UNEXP_TOKEN_ERROR("symbol \";\"");
						} else if (tk->type == Token::SYM_SEMI_COLLON) {

						} else if (tk->type == Token::SYM_COLLON) {
							for_block->cf_type = ControlFlowNode::CfType::FOREACH;
						}
						for_block->args.push_back(var_node);
					} else {
						for_block->args.push_back(_parse_expression(block_node, true));
						if (tokenizer->next().type != Token::SYM_SEMI_COLLON) throw UNEXP_TOKEN_ERROR("symbol \";\"");
					}
				}

				if (for_block->cf_type == ControlFlowNode::CfType::FOREACH) {
					for_block->args.push_back(_parse_expression(block_node, false));
					if (tokenizer->next().type != Token::BRACKET_RPARAN) throw UNEXP_TOKEN_ERROR("symbol \")\"");
				} else {
					if (tokenizer->peek().type == Token::SYM_SEMI_COLLON) {
						tokenizer->next(); // eat ";"
						for_block->args.push_back(nullptr);
					} else {
						for_block->args.push_back(_parse_expression(block_node, false));
						if (tokenizer->next().type != Token::SYM_SEMI_COLLON) throw UNEXP_TOKEN_ERROR("symbol \";\"");
					}

					if (tokenizer->peek().type == Token::BRACKET_RPARAN) {
						tokenizer->next(); // eat ")"
						for_block->args.push_back(nullptr);
					} else {
						for_block->args.push_back(_parse_expression(block_node, true));
						if (tokenizer->next().type != Token::BRACKET_RPARAN) throw UNEXP_TOKEN_ERROR("symbol \")\"");
					}
				}

				if (tokenizer->peek().type == Token::BRACKET_LCUR) {
					tokenizer->next(); // eat "{"
					for_block->body = _parse_block(block_node);
					if (tokenizer->next().type != Token::BRACKET_RCUR) throw UNEXP_TOKEN_ERROR("symbol \"}\"");
				} else {
					for_block->body = _parse_block(block_node, true);
				}

				// add loop counter initialization to local vars.
				if (for_block->args[0] != nullptr && for_block->args[0]->type == Node::Type::VAR) {
					for_block->body->local_vars.insert(for_block->body->local_vars.begin(), ptrcast<VarNode>(for_block->args[0]));
				}

				block_node->statements.push_back(for_block);
				parser_context.current_break = outer_break;
				parser_context.current_continue = outer_continue;
			} break;

			case Token::KWORD_BREAK: {
				tk = &tokenizer->next(); // eat "break"
				if (!parser_context.current_break) throw PARSER_ERROR(Error::SYNTAX_ERROR, "can't use break outside a loop/switch.", tk->get_pos());
				ptr<ControlFlowNode> _break = new_node<ControlFlowNode>(ControlFlowNode::BREAK);
				_break->break_continue = parser_context.current_break;
				parser_context.current_break->has_break = true;
				_break->parernt_node = p_parent;
				block_node->statements.push_back(_break);
			} break;

			case Token::KWORD_CONTINUE: {
				tk = &tokenizer->next(); // eat "continue"
				if (!parser_context.current_continue) throw PARSER_ERROR(Error::SYNTAX_ERROR, "can't use continue outside a loop.", tk->get_pos());
				ptr<ControlFlowNode> _continue = new_node<ControlFlowNode>(ControlFlowNode::CONTINUE);
				_continue->break_continue = parser_context.current_continue;
				parser_context.current_continue->has_continue = true;
				_continue->parernt_node = p_parent;
				block_node->statements.push_back(_continue);
			} break;

			case Token::KWORD_RETURN: {
				tk = &tokenizer->next(); // eat "return"
				if (!parser_context.current_func) throw PARSER_ERROR(Error::SYNTAX_ERROR, "can't use return outside a function.", tk->get_pos());
				if (parser_context.current_class && parser_context.current_class->constructor) {
					if (parser_context.current_class->constructor == parser_context.current_func) {
						throw PARSER_ERROR(Error::SYNTAX_ERROR, "constructor can't return anything.", tk->get_pos());
					}
				}
				ptr<ControlFlowNode> _return = new_node<ControlFlowNode>(ControlFlowNode::RETURN);
				if (tokenizer->peek().type != Token::SYM_SEMI_COLLON)  _return->args.push_back(_parse_expression(block_node, false));
				_return->parernt_node = p_parent;
				_return->_return = parser_context.current_func;
				if (tokenizer->next().type != Token::SYM_SEMI_COLLON) throw UNEXP_TOKEN_ERROR("symbol \";\"");
				parser_context.current_func->has_return = true;
				block_node->statements.push_back(_return);
			} break;

			default: {
				for (Token termination : p_termination) {
					if (tk->type == termination) {
						return block_node;
					}
				}
				ptr<Node> expr = _parse_expression(block_node, true);
				if (tokenizer->next().type != Token::SYM_SEMI_COLLON) throw UNEXP_TOKEN_ERROR("symbol \";\"");
				block_node->statements.push_back(expr);
			}
		}

		if (p_single_statement)
			break;
	}

	return block_node;
}


ptr<Parser::ControlFlowNode> Parser::_parse_if_block(const ptr<BlockNode>& p_parent) {
	ASSERT(tokenizer->peek(-1).type == Token::KWORD_IF);

	ptr<ControlFlowNode> if_block = new_node<ControlFlowNode>(ControlFlowNode::IF);
	if_block->parernt_node = p_parent;
	ptr<Node> cond = _parse_expression(p_parent, false);
	if_block->args.push_back(cond);

	const TokenData* tk = &tokenizer->peek();
	if (tk->type == Token::BRACKET_LCUR) {
		tokenizer->next(); // eat "{"
		if_block->body = _parse_block(p_parent);
		if (tokenizer->next().type != Token::BRACKET_RCUR) throw UNEXP_TOKEN_ERROR("symbol \"}\"");
	} else {
		if_block->body = _parse_block(p_parent, true);
	}

	tk = &tokenizer->peek(0);
	while (tk->type == Token::KWORD_ELSE) {
		tokenizer->next(); // eat "else"
		tk = &tokenizer->peek(0);
		switch (tk->type) {
			case Token::KWORD_IF: {
				tokenizer->next(); // eat "if"
				if_block->body_else = new_node<BlockNode>();
				if_block->body_else->parernt_node = p_parent;
				if_block->body_else->statements.push_back(_parse_if_block(p_parent));
			} break;
			case Token::BRACKET_LCUR: {
				tokenizer->next(); // eat "{"
				if_block->body_else = _parse_block(p_parent);
				if (tokenizer->next().type != Token::BRACKET_RCUR) throw UNEXP_TOKEN_ERROR("symbol \"}\"");
			} break;
			default: {
				if_block->body_else = _parse_block(p_parent, true);
			}
		}
		tk = &tokenizer->peek();
	}

	return if_block;
}

} // namespace carbon

/******************************************************************************************************************/
/*                                         EXPRESSION                                                             */
/******************************************************************************************************************/

namespace carbon {

ptr<Parser::Node> Parser::_parse_expression(const ptr<Node>& p_parent, bool p_allow_assign) {
	p_allow_assign = true; // all expressions suport assignment now (test for any bugs)
	ASSERT(p_parent != nullptr);

	stdvec<Expr> expressions;

	while (true) {

		const TokenData* tk = &tokenizer->next();
		ptr<Node> expr = nullptr;

		if (tk->type == Token::BRACKET_LPARAN) {
			expr = _parse_expression(p_parent, false);

			tk = &tokenizer->next();
			if (tk->type != Token::BRACKET_RPARAN) {
				throw UNEXP_TOKEN_ERROR("symbol \")\"");
			}

		} else if (tk->type == Token::KWORD_THIS) {
			if (parser_context.current_class == nullptr || (parser_context.current_func && parser_context.current_func->is_static) ||
				(parser_context.current_var && parser_context.current_var->is_static))
				throw PARSER_ERROR(Error::SYNTAX_ERROR, "keyword \"this\" only be used in non-static member function.", Vect2i());
			if (tokenizer->peek().type == Token::BRACKET_LPARAN) { // super();
				tk = &tokenizer->next(); // eat "("
				ptr<CallNode> call = new_node<CallNode>();
				call->base = new_node<ThisNode>();
				call->method = nullptr;
				call->args = _parse_arguments(p_parent);
				expr = call;
			} else {
				expr = new_node<ThisNode>();
			}
		} else if (tk->type == Token::KWORD_SUPER) {
			// if super is inside class function, it calls the same function in it's base.
			if (parser_context.current_class == nullptr || (parser_context.current_func == nullptr))
				throw PARSER_ERROR(Error::SYNTAX_ERROR, "keyword \"super\" can only be used in class function.", Vect2i());
			if (parser_context.current_class->base_type == ClassNode::NO_BASE) {
				throw PARSER_ERROR(Error::SYNTAX_ERROR, "invalid use of \"super\". Can only used inside classes with a base type.", Vect2i());
			}
			if (tokenizer->peek().type == Token::BRACKET_LPARAN) { // super();
				tk = &tokenizer->next(); // eat "("
				ptr<CallNode> call = new_node<CallNode>();
				call->base = new_node<SuperNode>();
				call->method = nullptr;
				call->args = _parse_arguments(p_parent);
				expr = call;
			} else {
				expr = new_node<SuperNode>();
			}

		} else if (tk->type == Token::VALUE_FLOAT || tk->type == Token::VALUE_INT || tk->type == Token::VALUE_STRING || tk->type == Token::VALUE_BOOL || tk->type == Token::VALUE_NULL) {
			expr = new_node<ConstValueNode>(tk->constant);

		} else if (tk->type == Token::OP_PLUS || tk->type == Token::OP_MINUS || tk->type == Token::OP_NOT || tk->type == Token::OP_BIT_NOT) {
			switch (tk->type) {
				case Token::OP_PLUS:
					expressions.push_back(Expr(OperatorNode::OpType::OP_POSITIVE, tokenizer->get_pos()));
					break;
				case Token::OP_MINUS:
					expressions.push_back(Expr(OperatorNode::OpType::OP_NEGATIVE, tokenizer->get_pos()));
					break;
				case Token::OP_NOT:
					expressions.push_back(Expr(OperatorNode::OpType::OP_NOT, tokenizer->get_pos()));
					break;
				case Token::OP_BIT_NOT:
					expressions.push_back(Expr(OperatorNode::OpType::OP_BIT_NOT, tokenizer->get_pos()));
					break;
			}
			continue;
		} else if ((tk->type == Token::IDENTIFIER || tk->type == Token::BUILTIN_TYPE) && tokenizer->peek().type == Token::BRACKET_LPARAN) {
			ptr<CallNode> call = new_node<CallNode>();

			if (tk->type == Token::IDENTIFIER) {
				BuiltinFunctions::Type builtin_func = BuiltinFunctions::get_func_type(tk->identifier);
				if (builtin_func != BuiltinFunctions::UNKNOWN) {
					call->is_compilttime = BuiltinFunctions::is_compiletime(builtin_func);
					call->base = new_node<BuiltinFunctionNode>(builtin_func);
					call->method = nullptr;
				} else {
					// Identifier node could be builtin class like File(), another static method, ...
					// will know when reducing.
					call->base = new_node<Node>(); // UNKNOWN on may/may-not be self
					call->method = new_node<IdentifierNode>(tk->identifier);
				}
			} else {
				call->base = new_node<BuiltinTypeNode>(tk->builtin_type);
				call->method = nullptr;
			}

			tk = &tokenizer->next();
			if (tk->type != Token::BRACKET_LPARAN) throw UNEXP_TOKEN_ERROR("symbol \"(\"");
			call->args = _parse_arguments(p_parent);
			expr = call;

		} else if (tk->type == Token::IDENTIFIER) {
			BuiltinFunctions::Type bif_type = BuiltinFunctions::get_func_type(tk->identifier);
			if (bif_type != BuiltinFunctions::UNKNOWN) {
				ptr<BuiltinFunctionNode> bif = new_node<BuiltinFunctionNode>(bif_type);
				expr = bif;
			} else {
				ptr<IdentifierNode> id = new_node<IdentifierNode>(tk->identifier);
				id->declared_block = parser_context.current_block;
				expr = id;
			}

		} else if (tk->type == Token::BUILTIN_TYPE) { // String.format(...);
			ptr<BuiltinTypeNode> bt = new_node<BuiltinTypeNode>(tk->builtin_type);
			expr = bt;

		} else if (tk->type == Token::BRACKET_LSQ) {
			ptr<ArrayNode> arr = new_node<ArrayNode>();
			bool done = false;
			bool comma_valid = false;
			while (!done) {
				tk = &tokenizer->peek();
				switch (tk->type) {
					case Token::_EOF:
						tk = &tokenizer->next(); // eat eof
						throw UNEXP_TOKEN_ERROR(nullptr);
						break;
					case Token::SYM_COMMA:
						tk = &tokenizer->next(); // eat comma
						if (!comma_valid) {
							throw UNEXP_TOKEN_ERROR(nullptr);
						}
						comma_valid = false;
						break;
					case Token::BRACKET_RSQ:
						tk = &tokenizer->next(); // eat ']'
						done = true;
						break;
					default:
						if (comma_valid) throw UNEXP_TOKEN_ERROR("symbol \",\"");

						ptr<Node> subexpr = _parse_expression(p_parent, false);
						arr->elements.push_back(subexpr);
						comma_valid = true;
				}
			}
			expr = arr;

		} else if (tk->type == Token::BRACKET_LCUR) {
			ptr<MapNode> map = new_node<MapNode>();
			bool done = false;
			bool comma_valid = false;
			while (!done) {
				tk = &tokenizer->peek();
				switch (tk->type) {
					case Token::_EOF:
						tk = &tokenizer->next(); // eat eof
						throw UNEXP_TOKEN_ERROR(nullptr);
						break;
					case Token::SYM_COMMA:
						tk = &tokenizer->next(); // eat comma
						if (!comma_valid) throw UNEXP_TOKEN_ERROR(nullptr);
						comma_valid = false;
						break;
					case Token::BRACKET_RCUR:
						tk = &tokenizer->next(); // eat '}'
						done = true;
						break;
					default:
						if (comma_valid) throw UNEXP_TOKEN_ERROR("symbol \",\"");

						ptr<Node> key = _parse_expression(p_parent, false);
						tk = &tokenizer->next();
						if (tk->type != Token::SYM_COLLON) throw UNEXP_TOKEN_ERROR("symbol \":\"");
						ptr<Node> value = _parse_expression(p_parent, false);
						map->elements.push_back(Parser::MapNode::Pair(key, value));
						comma_valid = true;
				}
			}
			expr = map;
		} else {
			throw UNEXP_TOKEN_ERROR(nullptr);
		}

		// -- PARSE INDEXING -------------------------------------------------------

		while (true) {

			tk = &tokenizer->peek();
			// .named_index
			if (tk->type == Token::SYM_DOT) {
				tk = &tokenizer->next(1);

				if (tk->type != Token::IDENTIFIER) throw UNEXP_TOKEN_ERROR(nullptr);

				// call
				if (tokenizer->peek().type == Token::BRACKET_LPARAN) {
					ptr<CallNode> call = new_node<CallNode>();

					call->base = expr;
					call->method = new_node<IdentifierNode>(tk->identifier);
					tk = &tokenizer->next(); // eat "("
					call->args = _parse_arguments(p_parent);
					expr = call;

					// Just indexing.
				} else {
					ptr<IndexNode> ind = new_node<IndexNode>();
					ind->base = expr;
					ind->member = new_node<IdentifierNode>(tk->identifier);
					expr = ind;
				}


				// [mapped_index]
			} else if (tk->type == Token::BRACKET_LSQ) {
				ptr<MappedIndexNode> ind_mapped = new_node<MappedIndexNode>();

				tk = &tokenizer->next(); // eat "["
				ptr<Node> key = _parse_expression(p_parent, false);
				tk = &tokenizer->next();
				if (tk->type != Token::BRACKET_RSQ) {
					throw UNEXP_TOKEN_ERROR("symbol \"]\"");
				}

				ind_mapped->base = expr;
				ind_mapped->key = key;
				expr = ind_mapped;

				// get_func()(...);
			} else if (tk->type == Token::BRACKET_LPARAN) {
				ptr<CallNode> call = new_node<CallNode>();

				call->base = expr;
				call->method = nullptr;
				tk = &tokenizer->next(); // eat "("
				call->args = _parse_arguments(p_parent);
				expr = call;

			} else {
				break;
			}

		}

		expressions.push_back(Expr(expr));

		// -- PARSE OPERATOR -------------------------------------------------------
		tk = &tokenizer->peek();

		OperatorNode::OpType op;
		bool valid = true;

		switch (tk->type) {
		#define OP_CASE(m_op) case Token::m_op: op = OperatorNode::OpType::m_op; break
			OP_CASE(OP_EQ);
			OP_CASE(OP_EQEQ);
			OP_CASE(OP_PLUS);
			OP_CASE(OP_PLUSEQ);
			OP_CASE(OP_MINUS);
			OP_CASE(OP_MINUSEQ);
			OP_CASE(OP_MUL);
			OP_CASE(OP_MULEQ);
			OP_CASE(OP_DIV);
			OP_CASE(OP_DIVEQ);
			OP_CASE(OP_MOD);
			OP_CASE(OP_MOD_EQ);
			OP_CASE(OP_LT);
			OP_CASE(OP_LTEQ);
			OP_CASE(OP_GT);
			OP_CASE(OP_GTEQ);
			OP_CASE(OP_AND);
			OP_CASE(OP_OR);
			OP_CASE(OP_NOT);
			OP_CASE(OP_NOTEQ);
			OP_CASE(OP_BIT_NOT);
			OP_CASE(OP_BIT_LSHIFT);
			OP_CASE(OP_BIT_LSHIFT_EQ);
			OP_CASE(OP_BIT_RSHIFT);
			OP_CASE(OP_BIT_RSHIFT_EQ);
			OP_CASE(OP_BIT_OR);
			OP_CASE(OP_BIT_OR_EQ);
			OP_CASE(OP_BIT_AND);
			OP_CASE(OP_BIT_AND_EQ);
			OP_CASE(OP_BIT_XOR);
			OP_CASE(OP_BIT_XOR_EQ);
		#undef OP_CASE

			default: valid = false;
		}
		MISSED_ENUM_CHECK(OperatorNode::OpType::_OP_MAX_, 33);

		if (valid) {
			tokenizer->next(); // Eat peeked token.

			expressions.push_back(Expr(op, tokenizer->get_pos()));
		} else {
			break;
		}
	}

	ptr<Node> op_tree = _build_operator_tree(expressions);
	if (op_tree->type == Node::Type::OPERATOR) {
		if (!p_allow_assign && OperatorNode::is_assignment(ptrcast<OperatorNode>(op_tree)->op_type)) {
			throw PARSER_ERROR(Error::SYNTAX_ERROR, "assignment is not allowed inside expression.", op_tree->pos);
		}
	}
	return op_tree;

}

stdvec<ptr<Parser::Node>> Parser::_parse_arguments(const ptr<Node>& p_parent) {
	ASSERT(tokenizer->peek(-1).type == Token::BRACKET_LPARAN);

	const TokenData* tk = &tokenizer->peek();
	stdvec<ptr<Node>> args;

	if (tk->type == Token::BRACKET_RPARAN) {
		tokenizer->next(); // eat BRACKET_RPARAN
	} else {
		while (true) {

			ptr<Node> arg = _parse_expression(p_parent, false);
			args.push_back(arg);

			tk = &tokenizer->next();
			if (tk->type == Token::SYM_COMMA) {
				// pass
			} else if (tk->type == Token::BRACKET_RPARAN) {
				break;
			} else {
				throw UNEXP_TOKEN_ERROR(nullptr);
			}
		}
	}

	return args;
}

int Parser::_get_operator_precedence(OperatorNode::OpType p_op) {
	switch (p_op) {
		case OperatorNode::OpType::OP_NOT:
		case OperatorNode::OpType::OP_BIT_NOT:
		case OperatorNode::OpType::OP_POSITIVE:
		case OperatorNode::OpType::OP_NEGATIVE:
			return 0;
		case OperatorNode::OpType::OP_MUL:
		case OperatorNode::OpType::OP_DIV:
		case OperatorNode::OpType::OP_MOD:
			return 1;
		case OperatorNode::OpType::OP_PLUS:
		case OperatorNode::OpType::OP_MINUS:
			return 2;
		case OperatorNode::OpType::OP_BIT_LSHIFT:
		case OperatorNode::OpType::OP_BIT_RSHIFT:
			return 3;
		case OperatorNode::OpType::OP_LT:
		case OperatorNode::OpType::OP_LTEQ:
		case OperatorNode::OpType::OP_GT:
		case OperatorNode::OpType::OP_GTEQ:
			return 4;
		case OperatorNode::OpType::OP_EQEQ:
		case OperatorNode::OpType::OP_NOTEQ:
			return 5;
		case OperatorNode::OpType::OP_BIT_AND:
			return 6;
		case OperatorNode::OpType::OP_BIT_XOR:
			return 7;
		case OperatorNode::OpType::OP_BIT_OR:
			return 8;
		case OperatorNode::OpType::OP_AND:
			return 9;
		case OperatorNode::OpType::OP_OR:
			return 10;
		case OperatorNode::OpType::OP_EQ:
		case OperatorNode::OpType::OP_PLUSEQ:
		case OperatorNode::OpType::OP_MINUSEQ:
		case OperatorNode::OpType::OP_MULEQ:
		case OperatorNode::OpType::OP_DIVEQ:
		case OperatorNode::OpType::OP_MOD_EQ:
		case OperatorNode::OpType::OP_BIT_LSHIFT_EQ:
		case OperatorNode::OpType::OP_BIT_RSHIFT_EQ:
		case OperatorNode::OpType::OP_BIT_AND_EQ:
		case OperatorNode::OpType::OP_BIT_XOR_EQ:
		case OperatorNode::OpType::OP_BIT_OR_EQ:
			return 11;
		default:
			ASSERT(false);
			return -1;
	}
	MISSED_ENUM_CHECK(OperatorNode::OpType::_OP_MAX_, 33);
}

ptr<Parser::Node> Parser::_build_operator_tree(stdvec<Expr>& p_expr) {
	ASSERT(p_expr.size() > 0);

	while (p_expr.size() > 1) {

		int next_op = -1;
		int min_precedence = 0xFFFFF;
		bool unary = false;

		for (int i = 0; i < (int)p_expr.size(); i++) {
			if (!p_expr[i].is_op()) {
				continue;
			}

			int precedence = _get_operator_precedence(p_expr[i].get_op());
			if (precedence < min_precedence) {
				min_precedence = precedence;
				next_op = i;
				OperatorNode::OpType op = p_expr[i].get_op();
				unary = (
					op == OperatorNode::OpType::OP_NOT ||
					op == OperatorNode::OpType::OP_BIT_NOT ||
					op == OperatorNode::OpType::OP_POSITIVE ||
					op == OperatorNode::OpType::OP_NEGATIVE);
			}
		}

		ASSERT(next_op >= 0);

		if (unary) {

			int next_expr = next_op;
			while (p_expr[next_expr].is_op()) {
				if (++next_expr == p_expr.size()) {
					throw PARSER_ERROR(Error::SYNTAX_ERROR, "expected an expression.", Vect2i());
				}
			}

			for (int i = next_expr - 1; i >= next_op; i--) {
				ptr<OperatorNode> op_node = new_node<OperatorNode>(p_expr[i].get_op());
				op_node->pos = p_expr[i].get_pos();
				op_node->args.push_back(p_expr[(size_t)i + 1].get_expr());
				p_expr.at(i) = Expr(op_node);
				p_expr.erase(p_expr.begin() + i + 1);
			}

		} else {
			ASSERT(next_op >= 1 && next_op < (int)p_expr.size() - 1);
			ASSERT(!p_expr[(size_t)next_op - 1].is_op() && !p_expr[(size_t)next_op + 1].is_op());

			ptr<OperatorNode> op_node = new_node<OperatorNode>(p_expr[(size_t)next_op].get_op());
			op_node->pos = p_expr[next_op].get_pos();

			if (p_expr[(size_t)next_op - 1].get_expr()->type == Node::Type::OPERATOR) {
				if (OperatorNode::is_assignment(ptrcast<OperatorNode>(p_expr[(size_t)next_op - 1].get_expr())->op_type)) {
					Vect2i pos = ptrcast<OperatorNode>(p_expr[(size_t)next_op - 1].get_expr())->pos;
					throw PARSER_ERROR(Error::SYNTAX_ERROR, "unexpected assignment.", Vect2i(pos.x, pos.y));
				}
			}

			if (p_expr[(size_t)next_op + 1].get_expr()->type == Node::Type::OPERATOR) {
				if (OperatorNode::is_assignment(ptrcast<OperatorNode>(p_expr[(size_t)next_op + 1].get_expr())->op_type)) {
					Vect2i pos = ptrcast<OperatorNode>(p_expr[(size_t)next_op + 1].get_expr())->pos;
					throw PARSER_ERROR(Error::SYNTAX_ERROR, "unexpected assignment.", Vect2i(pos.x, pos.y));
				}
			}

			op_node->args.push_back(p_expr[(size_t)next_op - 1].get_expr());
			op_node->args.push_back(p_expr[(size_t)next_op + 1].get_expr());

			p_expr.at((size_t)next_op - 1) = Expr(op_node);
			p_expr.erase(p_expr.begin() + next_op);
			p_expr.erase(p_expr.begin() + next_op);
		}
	}
	ASSERT(!p_expr[0].is_op());
	return p_expr[0].get_expr();
}

} // namespace carbon


/******************************************************************************************************************/
/*                                         STRINGS                                                               */
/******************************************************************************************************************/

namespace carbon {
	
String TokenData::to_string() const {
	switch (type) {
		case Token::UNKNOWN: return "<unknown>";
		case Token::_EOF:    return "<eof>";

		case Token::SYM_DOT:         return ".";
		case Token::SYM_COMMA:       return ",";
		case Token::SYM_COLLON:      return ":";
		case Token::SYM_SEMI_COLLON: return ";";
		case Token::SYM_AT:          return "@";
		case Token::SYM_HASH:        return "#";
		case Token::SYM_DOLLAR:      return "$";
		case Token::SYM_QUESTION:    return "?";
		case Token::BRACKET_LPARAN:  return "(";
		case Token::BRACKET_RPARAN:  return ")";
		case Token::BRACKET_LCUR:    return "{";
		case Token::BRACKET_RCUR:    return "}";
		case Token::BRACKET_RSQ:     return "[";
		case Token::BRACKET_LSQ:     return "]";

		case Token::OP_EQ:      return "=";
		case Token::OP_EQEQ:    return "==";
		case Token::OP_PLUS:    return "+";
		case Token::OP_PLUSEQ:  return "+=";
		case Token::OP_MINUS:   return "-";
		case Token::OP_MINUSEQ: return "-=";
		case Token::OP_MUL:     return "*";
		case Token::OP_MULEQ:   return "*=";
		case Token::OP_DIV:     return "/";
		case Token::OP_DIVEQ:   return "/=";
		case Token::OP_MOD:     return "%";
		case Token::OP_MOD_EQ:  return "%=";
		case Token::OP_LT:      return "<";
		case Token::OP_LTEQ:    return "<=";
		case Token::OP_GT:      return ">";
		case Token::OP_GTEQ:    return ">=";
		case Token::OP_AND:     return "&&";
		case Token::OP_OR:      return "||";
		case Token::OP_NOT:     return "!";
		case Token::OP_NOTEQ:   return "!=";

		case Token::OP_BIT_NOT:       return "~";
		case Token::OP_BIT_LSHIFT:    return "<<";
		case Token::OP_BIT_LSHIFT_EQ: return "<<=";
		case Token::OP_BIT_RSHIFT:    return ">>";
		case Token::OP_BIT_RSHIFT_EQ: return ">>=";
		case Token::OP_BIT_OR:        return "|";
		case Token::OP_BIT_OR_EQ:     return "|=";
		case Token::OP_BIT_AND:       return "&";
		case Token::OP_BIT_AND_EQ:    return "&=";
		case Token::OP_BIT_XOR:       return "^";
		case Token::OP_BIT_XOR_EQ:    return "^=";

		case Token::IDENTIFIER:     return identifier;
		case Token::BUILTIN_TYPE:   return BuiltinTypes::get_type_name(builtin_type);

		case Token::KWORD_IMPORT:   return "import";
		case Token::KWORD_CLASS:    return "class";
		case Token::KWORD_ENUM:     return "enum";
		case Token::KWORD_FUNC:     return "func";
		case Token::KWORD_VAR:      return "var";
		case Token::KWORD_CONST:    return "const";
		case Token::KWORD_NULL:     return "null";
		case Token::KWORD_TRUE:     return "true";
		case Token::KWORD_FALSE:    return "false";
		case Token::KWORD_IF:       return "if";
		case Token::KWORD_ELSE:     return "else";
		case Token::KWORD_WHILE:    return "while";
		case Token::KWORD_FOR:      return "for";
		case Token::KWORD_SWITCH:   return "switch";
		case Token::KWORD_CASE:     return "case";
		case Token::KWORD_DEFAULT:  return "default";
		case Token::KWORD_BREAK:    return "break";
		case Token::KWORD_CONTINUE: return "continue";
		case Token::KWORD_STATIC:   return "static";
		case Token::KWORD_THIS:     return "this";
		case Token::KWORD_SUPER:    return "super";
		case Token::KWORD_RETURN:   return "return";
		case Token::KWORD_AND:      return "and";
		case Token::KWORD_OR:       return "or";
		case Token::KWORD_NOT:      return "not";
			
		case Token::VALUE_NULL:     return "null";
		case Token::VALUE_STRING: 
			return String("\"") + constant.operator String() + "\"";
		case Token::VALUE_INT: 
		case Token::VALUE_FLOAT: 
			return constant.to_string();
		case Token::VALUE_BOOL:
			return (constant.operator bool()) ? "true" : "false";

		case Token::_TK_MAX_: return "<_TK_MAX_>";
	}
	THROW_BUG(String::format("missed enum in switch case."));
MISSED_ENUM_CHECK(Token::_TK_MAX_, 79);
}


const char* Tokenizer::get_token_name(Token p_tk) {
	static const char* token_names[] = {
		"UNKNOWN",
		"_EOF",

		"SYM_DOT",
		"SYM_COMMA",
		"SYM_COLLON",
		"SYM_SEMI_COLLON",
		"SYM_AT",
		"SYM_HASH",
		"SYM_DOLLAR",
		"SYM_QUESTION",
		"BRACKET_LPARAN",
		"BRACKET_RPARAN",
		"BRACKET_LCUR",
		"BRACKET_RCUR",
		"BRACKET_RSQ",
		"BRACKET_LSQ",

		"OP_EQ",
		"OP_EQEQ",
		"OP_PLUS",
		"OP_PLUSEQ",
		"OP_MINUS",
		"OP_MINUSEQ",
		"OP_MUL",
		"OP_MULEQ",
		"OP_DIV",
		"OP_DIVEQ",
		"OP_MOD",
		"OP_MOD_EQ",
		"OP_LT",
		"OP_LTEQ",
		"OP_GT",
		"OP_GTEQ",
		"OP_AND",
		"OP_OR",
		"OP_NOT",
		"OP_NOTEQ",

		"OP_BIT_NOT",
		"OP_BIT_LSHIFT",
		"OP_BIT_LSHIFT_EQ",
		"OP_BIT_RSHIFT",
		"OP_BIT_RSHIFT_EQ",
		"OP_BIT_OR",
		"OP_BIT_OR_EQ",
		"OP_BIT_AND",
		"OP_BIT_AND_EQ",
		"OP_BIT_XOR",
		"OP_BIT_XOR_EQ",

		"IDENTIFIER",
		"BUILTIN_TYPE",

		"KWORD_IMPORT",
		"KWORD_CLASS",
		"KWORD_ENUM",
		"KWORD_FUNC",
		"KWORD_VAR",
		"KWORD_CONST",
		"KWORD_NULL",
		"KWORD_TRUE",
		"KWORD_FALSE",
		"KWORD_IF",
		"KWORD_ELSE",
		"KWORD_WHILE",
		"KWORD_FOR",
		"KWORD_SWITCH",
		"KWORD_CASE",
		"KWORD_DEFAULT",
		"KWORD_BREAK",
		"KWORD_CONTINUE",
		"KWORD_STATIC",
		"KWORD_THIS",
		"KWORD_SUPER",
		"KWORD_RETURN",
		"KWORD_AND",
		"KWORD_OR",
		"KWORD_NOT",

		"VALUE_NULL",
		"VALUE_STRING",
		"VALUE_INT",
		"VALUE_FLOAT",
		"VALUE_BOOL",
		nullptr, //_TK_MAX_
	};
	return token_names[(int)p_tk];
MISSED_ENUM_CHECK(Token::_TK_MAX_, 79);
}

const char* Parser::Node::get_node_type_name(Type p_type) {
	static const char* type_names[] = {
		"UNKNOWN",
		"FILE",
		"CLASS",
		"ENUM",
		"FUNCTION",
		"BLOCK",
		"IDENTIFIER",
		"VAR",
		"CONST",
		"CONST_VALUE",
		"ARRAY",
		"MAP",
		"THIS",
		"SUPER",
		"BUILTIN_FUNCTION",
		"BUILTIN_TYPE",
		"CALL",
		"INDEX",
		"MAPPED_INDEX",
		"OPERATOR",
		"CONTROL_FLOW",
		nullptr, // _NODE_MAX_
	};
	return type_names[(int)p_type];
MISSED_ENUM_CHECK(Parser::Node::Type::_NODE_MAX_, 22);
}

const char* Parser::OperatorNode::get_op_name(OpType p_op) {
	static const char* op_names[] = {

		"OP_EQ",
		"OP_EQEQ",
		"OP_PLUS",
		"OP_PLUSEQ",
		"OP_MINUS",
		"OP_MINUSEQ",
		"OP_MUL",
		"OP_MULEQ",
		"OP_DIV",
		"OP_DIVEQ",
		"OP_MOD",
		"OP_MOD_EQ",
		"OP_LT",
		"OP_LTEQ",
		"OP_GT",
		"OP_GTEQ",
		"OP_AND",
		"OP_OR",
		"OP_NOT",
		"OP_NOTEQ",

		"OP_BIT_NOT",
		"OP_BIT_LSHIFT",
		"OP_BIT_LSHIFT_EQ",
		"OP_BIT_RSHIFT",
		"OP_BIT_RSHIFT_EQ",
		"OP_BIT_OR",
		"OP_BIT_OR_EQ",
		"OP_BIT_AND",
		"OP_BIT_AND_EQ",
		"OP_BIT_XOR",
		"OP_BIT_XOR_EQ",

		"OP_POSITIVE",
		"OP_NEGATIVE",

		nullptr, // _OP_MAX_
	};
	return op_names[p_op];
MISSED_ENUM_CHECK(Parser::OperatorNode::OpType::_OP_MAX_, 33);
}

const char* Parser::ControlFlowNode::get_cftype_name(CfType p_type) {
	static const char* cf_names[] = {
		"IF",
		"SWITCH",
		"WHILE",
		"FOR",
		"FOREACH",
		"BREAK",
		"CONTINUE",
		"RETURN",
		nullptr, // _CF_MAX_
	};
	return cf_names[p_type];

MISSED_ENUM_CHECK(Parser::ControlFlowNode::_CF_MAX_, 8);
}


stdmap<BuiltinFunctions::Type, String> BuiltinFunctions::_func_list = {

	{ BuiltinFunctions::__ASSERT,  "__assert" },
	{ BuiltinFunctions::__FUNC,    "__func"   },
	{ BuiltinFunctions::__LINE,    "__line"   },
	{ BuiltinFunctions::__FILE,    "__file"   },

	{ BuiltinFunctions::PRINT,     "print"    },
	{ BuiltinFunctions::PRINTLN,   "println"  },
	{ BuiltinFunctions::INPUT,     "input"    },
	{ BuiltinFunctions::HEX,       "hex"      },
	{ BuiltinFunctions::BIN,       "bin"      },
	{ BuiltinFunctions::MATH_MIN,  "min"      },
	{ BuiltinFunctions::MATH_MAX,  "max"      },
	{ BuiltinFunctions::MATH_POW,  "pow"      },

};
MISSED_ENUM_CHECK(BuiltinFunctions::Type::_FUNC_MAX_, 13);

stdmap<BuiltinTypes::Type, String> BuiltinTypes::_type_list = {
	//{ "", BuiltinTypes::UNKNOWN    },
	//{ "", BuiltinTypes::_TYPE_MAX_ },

	{ BuiltinTypes::_NULL,  "null",      },
	{ BuiltinTypes::BOOL,   "bool",      },
	{ BuiltinTypes::INT,    "int",       },
	{ BuiltinTypes::FLOAT,  "float",     },
	{ BuiltinTypes::STRING, "String",    },
	{ BuiltinTypes::STR,    "str",       },
	{ BuiltinTypes::ARRAY,  "Array",     },
	{ BuiltinTypes::MAP,    "Map",       },

};
MISSED_ENUM_CHECK(BuiltinTypes::Type::_TYPE_MAX_, 9);

} // namespace carbon

#endif //_FILE_SRC_COMPILER_PARSER_CPP_

#ifndef _FILE_SRC_COMPILER_TOKENIZER_CPP_


namespace carbon {

#define TOKENIZER_ERROR(m_type, m_msg) _tokenize_error(m_type, m_msg, _DBG_SOURCE)

#define GET_CHAR(m_off)  \
( ((size_t)char_ptr + m_off >= source.size())? '\0': source[(size_t)char_ptr + m_off] )

#define EAT_CHAR(m_num)  \
{	char_ptr += m_num;   \
	cur_col += m_num;    \
}

#define EAT_LINE()       \
{	char_ptr++;          \
	cur_col = 1;         \
	cur_line++;          \
}

#define IS_NUM(c)        \
( ('0' <= c && c <= '9') )

#define IS_HEX_CHAR(c)   \
( IS_NUM(c) || ('a' <= c && c <= 'f') || ('A' <= c && c <= 'F') )

#define IS_TEXT(c)       \
( (c == '_') || ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') )


struct KeywordName { const char* name; Token tk; };
static KeywordName _keyword_name_list[] = {
	{ "import",   Token::KWORD_IMPORT	     },
	{ "class",    Token::KWORD_CLASS	     },
	{ "enum",     Token::KWORD_ENUM	         },
	{ "func",     Token::KWORD_FUNC	         },
	{ "var",      Token::KWORD_VAR		     },
	{ "const",    Token::KWORD_CONST	     },
	{ "null",     Token::KWORD_NULL	         },
	{ "true",     Token::KWORD_TRUE	         },
	{ "false",    Token::KWORD_FALSE	     },
	{ "if",       Token::KWORD_IF		     },
	{ "else",     Token::KWORD_ELSE	         },
	{ "while",    Token::KWORD_WHILE	     },
	{ "for",      Token::KWORD_FOR		     },
	{ "switch",   Token::KWORD_SWITCH		 },
	{ "case",     Token::KWORD_CASE		     },
	{ "default",  Token::KWORD_DEFAULT       },
	{ "break",    Token::KWORD_BREAK	     },
	{ "continue", Token::KWORD_CONTINUE      },
	{ "static",   Token::KWORD_STATIC        },
	{ "this",     Token::KWORD_THIS          },
	{ "super",    Token::KWORD_SUPER         },
	{ "return",   Token::KWORD_RETURN	     },
	{ "and",      Token::KWORD_AND	         },
	{ "or",       Token::KWORD_OR	         },
	{ "not",      Token::KWORD_NOT	         },

};
MISSED_ENUM_CHECK(Token::_TK_MAX_, 79);

void Tokenizer::_eat_escape(String& p_str) {
	char c = GET_CHAR(0);
	ASSERT(c == '\\');
	c = GET_CHAR(1);
	switch (c) {
		case 0:
			throw TOKENIZER_ERROR(Error::UNEXPECTED_EOF, "");
			break;
		case '\\': p_str += '\\'; EAT_CHAR(2); break;
		case '\'': p_str += '\''; EAT_CHAR(2); break;
		case 't':  p_str += '\t'; EAT_CHAR(2); break;
		case 'n':  p_str += '\n'; EAT_CHAR(2); break;
		case '"':  p_str += '"';  EAT_CHAR(2); break;
		case 'r':  p_str += '\r'; EAT_CHAR(2); break;
		case '\n': EAT_CHAR(1); EAT_LINE(); break;
		default: 
			throw TOKENIZER_ERROR(Error::SYNTAX_ERROR, "invalid escape character");
	}
}

CompileTimeError Tokenizer::_tokenize_error(Error::Type m_err_type, const String& m_msg, const DBGSourceInfo& p_dbg_info) const {
	uint32_t err_len = 1;
	String token_str = peek(-1, true).to_string();
	if (token_str.size() > 1 && token_str[0] == '<' && token_str[token_str.size() - 1] == '>') err_len = 1;
	else err_len = (uint32_t)token_str.size();

	return CompileTimeError(m_err_type, m_msg,  DBGSourceInfo(source_path, source, std::pair<int, int>(cur_line, cur_col), err_len),p_dbg_info);
}

const TokenData& Tokenizer::next(int p_offset) {
	if (token_ptr + p_offset >= (int)tokens.size()) { THROW_BUG("invalid index."); }
	token_ptr += p_offset;
	cur_line = tokens[token_ptr].line; cur_col = tokens[token_ptr].col;
	return tokens[token_ptr++];
}

const TokenData& Tokenizer::peek(int p_offset, bool p_safe) const {
	static TokenData tmp = { Token::_EOF };
	if (token_ptr + p_offset < 0 || token_ptr + p_offset >= (int)tokens.size()) {
		if (p_safe) return tmp;
		else throw TOKENIZER_ERROR(Error::INVALID_INDEX, "Internal Bug: TokenData::peek() index out of bounds");
	}
	return tokens[token_ptr + p_offset];
}

Vect2i Tokenizer::get_pos() const {
	return Vect2i(cur_line, cur_col);
}

uint32_t Tokenizer::get_width() const {
	int last_token_ind = token_ptr - 1;
	if (last_token_ind < 0 || tokens.size() <= last_token_ind) return 1;
	return tokens[last_token_ind].get_width();
}

const TokenData& Tokenizer::get_token_at(const Vect2i& p_pos, bool p_safe) const {
	static TokenData tmp = { Token::_EOF };
	for (size_t i = 0; i < tokens.size(); i++) {
		if (tokens[i].line == p_pos.x && tokens[i].col == p_pos.y) {
			return tokens[i];
		}
	}
	if (p_safe) return tmp;
	throw TOKENIZER_ERROR(Error::BUG, "TokenData::get_token_at() called with invalid position.");
}

const String& Tokenizer::get_source() const {
	return source;
}

const String& Tokenizer::get_source_path() const {
	return source_path;
}

void Tokenizer::_eat_token(Token p_tk, int p_eat_size) {
	TokenData tk;
	tk.type = p_tk;
	tk.line = cur_line;
	tk.col = cur_col;
	tokens.push_back(tk);
	EAT_CHAR(p_eat_size);
}

void Tokenizer::_eat_eof() {
	TokenData tk;
	tk.type = Token::_EOF;
	tk.line = cur_line;
	tk.col = cur_col;
	tokens.push_back(tk);
	EAT_CHAR(1);
}

// TODO: eat const value, ... cur_line, cur_col are not at the end of the token
// make the position to be at the start
void Tokenizer::_eat_const_value(const var& p_value, int p_eat_size) {
	TokenData tk;
	tk.line = cur_line;
	tk.col = cur_col - __const_val_token_len;
	tk.constant = p_value;

	switch (p_value.get_type()) {
		case var::STRING:
			tk.type = Token::VALUE_STRING;
			break;
		case var::INT:
			tk.type = Token::VALUE_INT;
			break;
		case var::FLOAT:
			tk.type = Token::VALUE_FLOAT;
			break;
		default:
			THROW_BUG("invalid switch case.");
			break;
	}

	tokens.push_back(tk);
	EAT_CHAR(p_eat_size);
}

void Tokenizer::_eat_identifier(const String& p_idf, int p_eat_size) {
	
	TokenData tk;
	tk.type = Token::IDENTIFIER;
	tk.identifier = p_idf; // method name may be builtin func
	tk.col = cur_col - (int)p_idf.size();
	tk.line = cur_line;

	for (const KeywordName& kw : _keyword_name_list) {
		if (kw.name == p_idf) {
			tk.type = kw.tk;

			// remap tokens.
			switch (tk.type) {
				case Token::KWORD_TRUE:
					tk.type = Token::VALUE_BOOL;
					tk.constant = var(true);
					break;
				case Token::KWORD_FALSE:
					tk.type = Token::VALUE_BOOL;
					tk.constant = var(false);
					break;
				case Token::KWORD_NULL:
					tk.type = Token::VALUE_NULL;
					tk.constant = var();
					break;
				case Token::KWORD_AND:
					tk.type = Token::OP_AND;
					break;
				case Token::KWORD_OR:
					tk.type = Token::OP_OR;
					break;
				case Token::KWORD_NOT:
					tk.type = Token::OP_NOT;
					break;
			}

			break;
		}
	}

	if (tk.type == Token::IDENTIFIER) {
		BuiltinTypes::Type bf_type = BuiltinTypes::get_type_type(tk.identifier);
		if (bf_type != BuiltinTypes::UNKNOWN) {
			tk.type = Token::BUILTIN_TYPE;
			tk.builtin_type = bf_type;
		}
	}

	tokens.push_back(tk);
	EAT_CHAR(p_eat_size);
}

void Tokenizer::_clear() {
	source = "";
	source_path = "";
	tokens.clear();
	cur_line = 1, cur_col = 1;
	char_ptr = 0;
	token_ptr = 0;
	__const_val_token_len = 0;
}

void Tokenizer::tokenize(ptr<File> p_file) {
	tokenize(p_file->read_text(), Path(p_file->get_path()).absolute());
}

void Tokenizer::tokenize(const String& p_source, const String& p_source_path) {

	_clear();
	source = p_source;
	source_path = p_source_path;

	while (char_ptr < (int)source.size()) {

		switch (GET_CHAR(0)) {
			case 0:
				_eat_eof();
				break;
			case ' ':
			case '\t':
				EAT_CHAR(1);
				break;
			case '\n':
				EAT_LINE();
				break;
			case '/':
			{
				if (GET_CHAR(1) == '/') { // comment

					while (GET_CHAR(0) != '\n' && GET_CHAR(0) != 0 ) {
						EAT_CHAR(1);
					}
					if (GET_CHAR(0) == '\n') {
						EAT_LINE();
					} else if (GET_CHAR(0) == 0) {
						_eat_eof();
					}
					
				} else if (GET_CHAR(1) == '*') { // multi line comment

					EAT_CHAR(2);
					while (true) {
						if (GET_CHAR(0) == '*' && GET_CHAR(1) == '/') {
							EAT_CHAR(2);
							break;
						} else if (GET_CHAR(0) == 0) {
							throw TOKENIZER_ERROR(Error::UNEXPECTED_EOF, "");
						} else if (GET_CHAR(0) == '\n') {
							EAT_LINE();
						} else {
							EAT_CHAR(1);
						}
					}

				} else {
					if (GET_CHAR(1) == '=') _eat_token(Token::OP_DIVEQ, 2);
					else _eat_token(Token::OP_DIV);
				}
				break;
			}
			// symbols
			case ',': _eat_token(Token::SYM_COMMA); break;
			case ':': _eat_token(Token::SYM_COLLON); break;
			case ';': _eat_token(Token::SYM_SEMI_COLLON); break;
			case '@': _eat_token(Token::SYM_AT); break;
			case '#': _eat_token(Token::SYM_HASH); break;
			case '$': _eat_token(Token::SYM_DOLLAR); break;
			case '?': _eat_token(Token::SYM_QUESTION); break;

			// brackets
			case '(': _eat_token(Token::BRACKET_LPARAN); break;
			case ')': _eat_token(Token::BRACKET_RPARAN); break;
			case '{': _eat_token(Token::BRACKET_LCUR); break;
			case '}': _eat_token(Token::BRACKET_RCUR); break;
			case '[': _eat_token(Token::BRACKET_LSQ); break;
			case ']': _eat_token(Token::BRACKET_RSQ); break;

			// op
			case '=': {
				if (GET_CHAR(1) == '=') _eat_token(Token::OP_EQEQ, 2);
				else _eat_token(Token::OP_EQ);
				break;
			}
			case '+': {
				if (GET_CHAR(1) == '=') _eat_token(Token::OP_PLUSEQ, 2);
				else _eat_token(Token::OP_PLUS);
				break;
			}
			case '-': {
				if (GET_CHAR(1) == '=') _eat_token(Token::OP_MINUSEQ, 2);
				else _eat_token(Token::OP_MINUS);
				break;
			}
			case '*': {
				if (GET_CHAR(1) == '=') _eat_token(Token::OP_MULEQ, 2);
				else _eat_token(Token::OP_MUL);
				break;
			}
			// case '/': { } // already hadled
			case '\\':
				throw TOKENIZER_ERROR(Error::SYNTAX_ERROR, "invalid character '\\'.");
				break;
			case '%': {
				if (GET_CHAR(1) == '=') _eat_token(Token::OP_MOD_EQ, 2);
				else _eat_token(Token::OP_MOD);
				break;
			}
			case '<': {
				if (GET_CHAR(1) == '=') _eat_token(Token::OP_LTEQ, 2);
				else if (GET_CHAR(1) == '<') {
					if (GET_CHAR(2) == '=') _eat_token(Token::OP_BIT_LSHIFT_EQ, 3);
					else _eat_token(Token::OP_BIT_LSHIFT, 2);
				}
				else _eat_token(Token::OP_LT);
				break;
			}
			case '>': {
				if (GET_CHAR(1) == '=') _eat_token(Token::OP_GTEQ, 2);
				else if (GET_CHAR(1) == '>') {
					if (GET_CHAR(2) == '=') _eat_token(Token::OP_BIT_RSHIFT_EQ, 3);
					else _eat_token(Token::OP_BIT_RSHIFT, 2);
				}
				else _eat_token(Token::OP_GT);
				break;
			}
			case '!': {
				if (GET_CHAR(1) == '=') _eat_token(Token::OP_NOTEQ, 2);
				else _eat_token(Token::OP_NOT);
				break;
			}
			case '~':
				_eat_token(Token::OP_BIT_NOT);
				break;
			case '|': {
				if (GET_CHAR(1) == '=') _eat_token(Token::OP_BIT_OR_EQ, 2);
				else if (GET_CHAR(1) == '|') _eat_token(Token::OP_OR, 2);
				else _eat_token(Token::OP_BIT_OR);
				break;
			}
			case '&': {
				if (GET_CHAR(1) == '=') _eat_token(Token::OP_BIT_AND_EQ, 2);
				else if (GET_CHAR(1) == '&') _eat_token(Token::OP_AND, 2);
				else _eat_token(Token::OP_BIT_AND);
				break;
			}
			case '^': {
				if (GET_CHAR(1) == '=') _eat_token(Token::OP_BIT_XOR_EQ, 2);
				else _eat_token(Token::OP_BIT_XOR);
				break;
			}

			// double quote string value (single quote not supported)
			case '"': {
				EAT_CHAR(1);
				String str;
				while (GET_CHAR(0) != '"') {
					if (GET_CHAR(0) == '\\') {
						_eat_escape(str);
					} else if (GET_CHAR(0) == 0) {
						throw TOKENIZER_ERROR(Error::UNEXPECTED_EOF, "unexpected EOF while parsing String.");
						break;
					// NO MORE EOL 
					//} else if(GET_CHAR(0) == '\n') {
					//	throw TOKENIZER_ERROR(Error::SYNTAX_ERROR, "unexpected EOL while parsing String.");
					//	break;
					} else {
						str += GET_CHAR(0);
						if (GET_CHAR(0) == '\n') { EAT_LINE(); }
						else { EAT_CHAR(1); }
					}
				}
				EAT_CHAR(1);
				__const_val_token_len = (int)str.size() + 2;
				_eat_const_value(str);
				break;
			}
			case '\'':
				throw TOKENIZER_ERROR(Error::SYNTAX_ERROR, "invalid character '\\''.");
				break;
			default: {
				
				// TODO: 1.2e3 => is a valid float number

				// float value begins with '.'
				if (GET_CHAR(0) == '.' && IS_NUM(GET_CHAR(1)) ) {
					String float_str = '.';
					EAT_CHAR(1);
					while (IS_NUM(GET_CHAR(0))) {
						float_str += GET_CHAR(0);
						EAT_CHAR(1);
					}
					double float_val = float_str.to_float();
					__const_val_token_len = (int)float_str.size();
					_eat_const_value(float_val);
					break;
				}
				// integer/float value
				if (IS_NUM(GET_CHAR(0))) {
					String num = GET_CHAR(0);
					
					enum _ReadMode { INT, FLOAT, BIN, HEX };
					_ReadMode mode = INT;
					if (GET_CHAR(0) == '0') {
						if (GET_CHAR(1) == 'b' || GET_CHAR(1) == 'B') mode = BIN;
						if (GET_CHAR(1) == 'x' || GET_CHAR(1) == 'X') mode = HEX;
					}
					EAT_CHAR(1);
					switch (mode) {
						case INT: {
							while (IS_NUM(GET_CHAR(0)) || GET_CHAR(0) == '.') {
								if (GET_CHAR(0) == '.' && mode == FLOAT)
									throw TOKENIZER_ERROR(Error::SYNTAX_ERROR, "invalid numeric value.");
								if (GET_CHAR(0) == '.')
									mode = FLOAT;
								num += GET_CHAR(0);
								EAT_CHAR(1);
							}
						} break;
						case BIN: {
							num += GET_CHAR(0); EAT_CHAR(1); // eat 'b';
							while (GET_CHAR(0) == '0' || GET_CHAR(0) == '1') {
								num += GET_CHAR(0);
								EAT_CHAR(1);
							}
						} break;
						case HEX: {
							num += GET_CHAR(0); EAT_CHAR(1); // eat 'x';
							while (IS_HEX_CHAR(GET_CHAR(0))) {
								num += GET_CHAR(0);
								EAT_CHAR(1);
							}
						} break;
					}

					// "1." parsed as 1.0 which should be error.
					if (num[num.size() - 1] == '.') throw TOKENIZER_ERROR(Error::SYNTAX_ERROR, "invalid numeric value.");

					__const_val_token_len = (int)num.size();
					if (mode == FLOAT)
						_eat_const_value(num.to_float());
					else
						_eat_const_value(num.to_int());
					break;
				}
				// identifier
				if (IS_TEXT(GET_CHAR(0))) {
					String identifier = GET_CHAR(0);
					EAT_CHAR(1);
					while (IS_TEXT(GET_CHAR(0)) || IS_NUM(GET_CHAR(0))) {
						identifier += GET_CHAR(0);
						EAT_CHAR(1);
					}
					_eat_identifier(identifier);
					break;
				}

				if (GET_CHAR(0) == '.') {
					_eat_token(Token::SYM_DOT);
					break;
				}

				throw TOKENIZER_ERROR(Error::SYNTAX_ERROR, String::format("unknown character '%c' in parsing.", GET_CHAR(0)));

			} // default case

		} // switch
	} // while

	_eat_eof();
}

}

#endif //_FILE_SRC_COMPILER_TOKENIZER_CPP_

#ifndef _FILE_SRC_COMPILER_VM_CPP_


namespace carbon {

VM* VM::_singleton = nullptr;
VM* VM::singleton() {
	if (_singleton == nullptr) _singleton = new VM();
	return _singleton;
}

void VM::cleanup() {
	if (_singleton != nullptr) delete _singleton;
}

VMStack::VMStack(uint32_t p_max_size) {
	_stack = newptr<stdvec<var>>(p_max_size);
}
var* VMStack::get_at(uint32_t p_pos) {
	THROW_INVALID_INDEX(_stack->size(), p_pos);
	return &(*_stack)[p_pos];
}

var VM::call_function(const String& p_func_name, Bytecode* p_bytecode, ptr<Instance> p_self, stdvec<var*>& p_args) {
	THROW_IF_NULLPTR(p_bytecode);
	Function* p_func = p_bytecode->get_function(p_func_name).get();
	return call_function(p_func, p_bytecode, p_self, p_args);
}

var VM::call_function(const Function* p_func, Bytecode* p_bytecode, ptr<Instance> p_self, stdvec<var*>& p_args, int __stack) {

	THROW_IF_NULLPTR(p_func);
	THROW_IF_NULLPTR(p_bytecode);
	if (__stack >= STACK_MAX) THROW_ERROR(Error::STACK_OVERFLOW, "stack was overflowed.");

	// check argc and add default args
	stdvec<var> default_args_copy;
	if (p_args.size() > p_func->get_arg_count()) {
		THROW_ERROR(Error::INVALID_ARG_COUNT,
			String::format("too many arguments were provided, expected at most %i argument(s).", p_func->get_arg_count()));
	} else if (p_args.size() < p_func->get_arg_count()) {
		const stdvec<var>& defaults = p_func->get_default_args();
		if (p_args.size() + defaults.size() < p_func->get_arg_count()) {
			THROW_ERROR(Error::INVALID_ARG_COUNT,
				String::format("too few arguments were provided, expected at least %i argument(s).", p_func->get_arg_count() - (int)defaults.size()));
		}

		int args_needed = p_func->get_arg_count() - (int)p_args.size();
		while (args_needed > 0) {
			default_args_copy.push_back(defaults[defaults.size() - args_needed--]);
		}
		for (var& v : default_args_copy) p_args.push_back(&v);
	}

	p_bytecode->initialize();

	VMStack stack(p_func->get_stack_size());
	RuntimeContext context;
	context.vm = this;
	context.stack = &stack;
	context.args = &p_args;
	if (p_self != nullptr) context.self = p_self;
	context.curr_fn = p_func;
	for (int i = 0; i < p_func->get_is_args_ref().size(); i++) {
		if (!p_func->get_is_args_ref()[i]) {
			context.value_args.push_back(*(p_args[i]));
		}
	}
	if (p_bytecode->is_class()) {
		context.bytecode_class = p_bytecode;
		context.bytecode_file = p_bytecode->get_file().get();
	} else {
		context.bytecode_file = p_bytecode;
	}

	uint32_t ip = 0; // instruction pointer
	const stdvec<uint32_t>& opcodes = p_func->get_opcodes();

#define CHECK_OPCODE_SIZE(m_size) ASSERT(ip + m_size < opcodes.size())
#define DISPATCH() goto L_loop

	L_loop:
	while (ip < opcodes.size()) {
		ASSERT(opcodes[ip] <= Opcode::END);
		uint32_t last_ip = ip;
		try {
		switch (opcodes[ip]) {
			case Opcode::GET: {
				CHECK_OPCODE_SIZE(4);
				var* on = context.get_var_at(opcodes[++ip]);
				const String& name = context.get_name_at(opcodes[++ip]);
				var* dst = context.get_var_at(opcodes[++ip]);
				ip++;

				*dst = on->get_member(name);
			} DISPATCH();

			case Opcode::SET: {
				CHECK_OPCODE_SIZE(4);
				var* on = context.get_var_at(opcodes[++ip]);
				const String& name = context.get_name_at(opcodes[++ip]);
				var* value = context.get_var_at(opcodes[++ip]);
				ip++;

				on->set_member(name, *value);
			} DISPATCH();

			case Opcode::GET_MAPPED: {
				CHECK_OPCODE_SIZE(4);
				var* on = context.get_var_at(opcodes[++ip]);
				var* key = context.get_var_at(opcodes[++ip]);
				var* dst = context.get_var_at(opcodes[++ip]);
				ip++;

				*dst = on->__get_mapped(*key);
			} DISPATCH();

			case Opcode::SET_MAPPED: {
				CHECK_OPCODE_SIZE(4);
				var* on = context.get_var_at(opcodes[++ip]);
				var* key = context.get_var_at(opcodes[++ip]);
				var* value = context.get_var_at(opcodes[++ip]);
				ip++;

				on->__set_mapped(*key, *value);
			} DISPATCH();

			case Opcode::SET_TRUE: {
				CHECK_OPCODE_SIZE(2);
				var* dst = context.get_var_at(opcodes[++ip]);
				ip++;

				*dst = true;
			} DISPATCH();

			case Opcode::SET_FALSE: {
				CHECK_OPCODE_SIZE(2);
				var* dst = context.get_var_at(opcodes[++ip]);
				ip++;

				*dst = false;
			} break;

			case Opcode::OPERATOR: {
				CHECK_OPCODE_SIZE(5);
				ASSERT(opcodes[ip + 1] < var::_OP_MAX_);
				var::Operator op = (var::Operator)opcodes[++ip];
				var* left = context.get_var_at(opcodes[++ip]);
				var* right = context.get_var_at(opcodes[++ip]);
				var* dst = context.get_var_at(opcodes[++ip]);
				ip++;

				switch (op) {
					case var::OP_ASSIGNMENT: {
						THROW_BUG("assignment operations should be under ASSIGN opcode");
					};
					case var::OP_ADDITION: { *dst = *left + *right; } break;
					case var::OP_SUBTRACTION: { *dst = *left - *right; } break;
					case var::OP_MULTIPLICATION: { *dst = *left * *right; } break;
					case var::OP_DIVISION: { *dst = *left / *right; } break;
					case var::OP_MODULO: { *dst = *left % *right; } break;
					case var::OP_POSITIVE: { *dst = *left; /* is it okey? */ } break;
					case var::OP_NEGATIVE: {
						if (left->get_type() == var::INT) {
							*dst = -left->operator int64_t();
						} else if (left->get_type() == var::FLOAT) {
							*dst = -left->operator double();
						} else {
							THROW_ERROR(Error::OPERATOR_NOT_SUPPORTED,
								String::format("operator (-) not supported on base %s.", left->get_type_name().c_str()));
						}
					} break;
					case var::OP_EQ_CHECK: { *dst = *left == *right; } break;
					case var::OP_NOT_EQ_CHECK: { *dst = *left != *right; } break;
					case var::OP_LT: { *dst = *left < *right; } break;
					case var::OP_LTEQ: { *dst = *left <= *right; } break;
					case var::OP_GT: { *dst = *left > * right; } break;
					case var::OP_GTEQ: { *dst = *left >= *right; } break;
					case var::OP_AND: { *dst = *left && *right; } break;
					case var::OP_OR: { *dst = *left || *right; } break;
					case var::OP_NOT: { *dst = !*left; } break;
					case var::OP_BIT_LSHIFT: { *dst = left->operator int64_t() << right->operator int64_t(); } break;
					case var::OP_BIT_RSHIFT: { *dst = left->operator int64_t() >> right->operator int64_t(); } break;
					case var::OP_BIT_AND: { *dst = left->operator int64_t() & right->operator int64_t(); } break;
					case var::OP_BIT_OR: { *dst = left->operator int64_t() | right->operator int64_t(); } break;
					case var::OP_BIT_XOR: { *dst = left->operator int64_t() ^ right->operator int64_t(); } break;
					case var::OP_BIT_NOT: { *dst = ~left->operator int64_t(); } break;
				}
			} break;

			case Opcode::ASSIGN: {
				CHECK_OPCODE_SIZE(3);
				var* dst = context.get_var_at(opcodes[++ip]);
				var* src = context.get_var_at(opcodes[++ip]);

				*dst = *src;
				ip++;
			} break;

			case Opcode::CONSTRUCT_BUILTIN: {
				CHECK_OPCODE_SIZE(4);
				uint32_t b_type = opcodes[++ip];
				uint32_t argc = opcodes[++ip];
				stdvec<var*> args(argc);
				for (int i = 0; i < (int)argc; i++) {
					var* arg = context.get_var_at(opcodes[++ip]);
					args[i] = arg;
				}
				var* dst = context.get_var_at(opcodes[++ip]);
				ip++;

				ASSERT(b_type < BuiltinTypes::_TYPE_MAX_);
				*dst = BuiltinTypes::construct((BuiltinTypes::Type)b_type, args);
			} break;

			case Opcode::CONSTRUCT_NATIVE: {
				CHECK_OPCODE_SIZE(4);
				const String& class_name = context.get_name_at(opcodes[++ip]);
				uint32_t argc = opcodes[++ip];
				stdvec<var*> args(argc);
				for (int i = 0; i < (int)argc; i++) {
					var* arg = context.get_var_at(opcodes[++ip]);
					args[i] = arg;
				}
				var* dst = context.get_var_at(opcodes[++ip]);
				ip++;

				*dst = NativeClasses::singleton()->construct(class_name, args);
			} DISPATCH();

			case Opcode::CONSTRUCT_CARBON: {
				CHECK_OPCODE_SIZE(4);
				const String& name = context.get_name_at(opcodes[++ip]);
				uint32_t argc = opcodes[++ip];
				stdvec<var*> args(argc);
				for (int i = 0; i < (int)argc; i++) {
					var* arg = context.get_var_at(opcodes[++ip]);
					args[i] = arg;
				}
				var* dst = context.get_var_at(opcodes[++ip]);
				ip++;

				stdmap<String, ptr<Bytecode>>* classes;
				if (p_bytecode->is_class()) classes = &p_bytecode->get_file()->get_classes();
				else classes = &p_bytecode->get_classes();
				if (classes->find(name) == classes->end()) THROW_BUG("cannot find classes the class");

				ptr<Bytecode> blueprint = classes->at(name);
				ptr<Instance> instance = newptr<Instance>(blueprint);

				const Function* member_initializer = blueprint->get_member_initializer();
				stdvec<var*> _args;
				if (member_initializer) call_function(member_initializer, blueprint.get(), instance, _args, __stack + 1);

				const Function* constructor = blueprint->get_constructor();
				if (constructor) call_function(constructor, blueprint.get(), instance, args, __stack + 1);

				*dst = instance;
			} DISPATCH();

			case Opcode::CONSTRUCT_LITERAL_ARRAY: {
				CHECK_OPCODE_SIZE(3);
				uint32_t size = opcodes[++ip];

				Array arr;
				for (int i = 0; i < (int)size; i++) {
					var* value = context.get_var_at(opcodes[++ip]);
					arr.push_back(*value);
				}
				var* dst = context.get_var_at(opcodes[++ip]);
				ip++;

				*dst = arr;
			} DISPATCH();

			case Opcode::CONSTRUCT_LITERAL_MAP: {
				CHECK_OPCODE_SIZE(3);
				uint32_t size = opcodes[++ip];

				Map map;
				for (int i = 0; i < (int)size; i++) {
					var* key = context.get_var_at(opcodes[++ip]);
					var* value = context.get_var_at(opcodes[++ip]);
					map[*key] = *value;
				}
				var* dst = context.get_var_at(opcodes[++ip]);
				ip++;

				*dst = map;
			} DISPATCH();

			case Opcode::CALL: {
				CHECK_OPCODE_SIZE(4);
				var* on = context.get_var_at(opcodes[++ip]);
				uint32_t argc = opcodes[++ip];
				stdvec<var*> args(argc);
				for (int i = 0; i < (int)argc; i++) {
					var* arg = context.get_var_at(opcodes[++ip]);
					args[i] = arg;
				}
				var* ret_value = context.get_var_at(opcodes[++ip]);
				ip++;

				*ret_value = on->__call(args);
			} DISPATCH();

			case Opcode::CALL_FUNC: {
				CHECK_OPCODE_SIZE(4);

				const String& func = context.get_name_at(opcodes[++ip]);
				uint32_t argc = opcodes[++ip];
				stdvec<var*> args(argc);
				for (int i = 0; i < (int)argc; i++) {
					var* arg = context.get_var_at(opcodes[++ip]);
					args[i] = arg;
				}
				var* ret_value = context.get_var_at(opcodes[++ip]);
				ip++;

				Bytecode* call_base;
				ptr<Function> func_ptr;

				// first search through inheritance
				if (p_self != nullptr) {
					call_base = p_self->blueprint.get();
					while (call_base != nullptr) {
						auto it = call_base->get_functions().find(func);
						if (it != call_base->get_functions().end()) {
							func_ptr = it->second;
							break;
						}
						call_base = call_base->get_base_binary().get();
					}

				// search in static class functions
				} else if (p_bytecode->is_class()) {
					call_base = p_bytecode;
					func_ptr = call_base->get_function(func);
					if (func_ptr != nullptr && !func_ptr->is_static() && p_func->is_static()) {
						THROW_BUG("can't call a non static function from static function");
					}
				}

				// search in the file
				if (func_ptr == nullptr) {
					if (p_bytecode->is_class()) call_base = p_bytecode->get_file().get();
					else call_base = p_bytecode;
				
					auto& functions = call_base->get_functions();
					auto it = functions.find(func);
					if (it != functions.end()) func_ptr = it->second;
				}

				//if (func_ptr == nullptr) THROW_BUG(String::format("can't find the function \"%s\"", func.c_str()));
				if (func_ptr == nullptr) {
					//*ret_value = NativeClasses::singleton()->call_method_on(p_self->native_instance, func, args);
					*ret_value = Object::call_method_s(p_self->native_instance, func, args);
				} else {
					*ret_value = call_function(func_ptr.get(), call_base, (func_ptr->is_static()) ? nullptr : p_self, args, __stack + 1);
				}


			} DISPATCH();

			case Opcode::CALL_METHOD: {
				CHECK_OPCODE_SIZE(5);

				var* on = context.get_var_at(opcodes[++ip]);
				const String& method = context.get_name_at(opcodes[++ip]);
				uint32_t argc = opcodes[++ip];
				stdvec<var*> args(argc);
				for (int i = 0; i < (int)argc; i++) {
					var* arg = context.get_var_at(opcodes[++ip]);
					args[i] = arg;
				}
				var* ret_value = context.get_var_at(opcodes[++ip]);
				ip++;

				*ret_value = on->call_method(method, args);

			} DISPATCH();

			case Opcode::CALL_BUILTIN: {
				CHECK_OPCODE_SIZE(4);

				uint32_t func = opcodes[++ip];
				uint32_t argc = opcodes[++ip];
				stdvec<var*> args(argc);
				for (int i = 0; i < (int)argc; i++) {
					var* arg = context.get_var_at(opcodes[++ip]);
					args[i] = arg;
				}
				var* ret = context.get_var_at(opcodes[++ip]);

				ASSERT(func < BuiltinFunctions::_FUNC_MAX_);
				BuiltinFunctions::call((BuiltinFunctions::Type)func, args, *ret);
				ip++;
			} DISPATCH();

			case Opcode::CALL_SUPER_CTOR: {
				CHECK_OPCODE_SIZE(2);
				uint32_t argc = opcodes[++ip];
				stdvec<var*> args(argc);

				for (int i = 0; i < (int)argc; i++) {
					var* arg = context.get_var_at(opcodes[++ip]);
					args[i] = arg;
				}
				ip++;

				ASSERT(p_bytecode->is_class());
				if (p_bytecode->is_base_native()) {
					p_self->native_instance = NativeClasses::singleton()->construct(p_bytecode->get_base_native(), args);
				} else {

					const Function* member_initializer = p_bytecode->get_base_binary()->get_member_initializer();
					if (member_initializer) {
						stdvec<var*> _args;
						call_function(member_initializer, p_bytecode->get_base_binary().get(), p_self, _args, __stack + 1);
					}

					const Function* ctor = p_bytecode->get_base_binary()->get_constructor();
					if (ctor) call_function(ctor, p_bytecode->get_base_binary().get(), p_self, args, __stack + 1);
				}

			} DISPATCH();

			case Opcode::CALL_SUPER_METHOD: {
				CHECK_OPCODE_SIZE(4);

				const String& method = context.get_name_at(opcodes[++ip]);
				uint32_t argc = opcodes[++ip];
				stdvec<var*> args(argc);
				for (int i = 0; i < (int)argc; i++) {
					var* arg = context.get_var_at(opcodes[++ip]);
					args[i] = arg;
				}
				var* ret_value = context.get_var_at(opcodes[++ip]);
				ip++;

				ASSERT(p_self->blueprint->has_base());

				if (p_self->blueprint->is_base_native()) {
					//ptr<BindData> bd = NativeClasses::singleton()->find_bind_data(p_self->blueprint->get_base_native(), method);
					*ret_value = NativeClasses::singleton()->call_method_on(p_self->native_instance, method, args);
				} else {
					ptr<Function> fn = p_self->blueprint->get_base_binary()->get_function(method);
					var* sv = nullptr; if (fn == nullptr) sv = p_self->blueprint->get_base_binary()->get_static_var(method);

					if (fn != nullptr) {
						if (fn->is_static()) {
							*ret_value = call_function(fn.get(), p_self->blueprint.get(), p_self, args, __stack + 1);
						} else {
							if (p_self == nullptr) THROW_BUG("can't call non-static method statically");
							*ret_value = call_function(fn.get(), p_self->blueprint.get(), nullptr, args, __stack + 1);
						}
					} else if (sv != nullptr) {
						*ret_value = sv->__call(args);
					} else {
						THROW_BUG("attribute not found in super");
					}
				}

			} DISPATCH();

			case Opcode::JUMP: {
				CHECK_OPCODE_SIZE(2);
				uint32_t addr = opcodes[++ip];
				ip = addr;
			} DISPATCH();

			case Opcode::JUMP_IF: {
				CHECK_OPCODE_SIZE(3);
				var* cond = context.get_var_at(opcodes[++ip]);
				uint32_t addr = opcodes[++ip];
				if (cond->operator bool()) ip = addr;
				else ip++;
			} DISPATCH();

			case Opcode::JUMP_IF_NOT: {
				CHECK_OPCODE_SIZE(3);
				var* cond = context.get_var_at(opcodes[++ip]);
				uint32_t addr = opcodes[++ip];
				if (!cond->operator bool()) ip = addr;
				else ip++;
			} DISPATCH();

			case Opcode::RETURN: {
				CHECK_OPCODE_SIZE(2);
				var* val = context.get_var_at(opcodes[++ip]);
				return *val;
			} DISPATCH();

			case Opcode::ITER_BEGIN: {
				CHECK_OPCODE_SIZE(3);
				var* iterator = context.get_var_at(opcodes[++ip]);
				var* on = context.get_var_at(opcodes[++ip]);
				ip++;

				*iterator = on->__iter_begin();
			} DISPATCH();

			case Opcode::ITER_NEXT: {
				CHECK_OPCODE_SIZE(4);
				var* iter_value = context.get_var_at(opcodes[++ip]);
				var* iterator = context.get_var_at(opcodes[++ip]);
				uint32_t addr = opcodes[++ip];

				if (iterator->__iter_has_next()) {
					*iter_value = iterator->__iter_next();
					ip++;
				} else {
					ip = addr;
				}

			} DISPATCH();

			case Opcode::END: {
				return var();
			} DISPATCH();

			MISSED_ENUM_CHECK(Opcode::END, 25);

		}} catch (Throwable& err) {
			ptr<Throwable> nested;
			switch (err.get_kind()) {
				case Throwable::ERROR:
					nested = newptr<Error>(static_cast<Error&>(err));
					break;
				case Throwable::COMPILE_TIME:
					nested = newptr<CompileTimeError>(static_cast<CompileTimeError&>(err));
					break;
				case Throwable::WARNING:
					nested = newptr<Warning>(static_cast<Warning&>(err));
					break;
				case Throwable::TRACEBACK:
					nested = newptr<TraceBack>(static_cast<TraceBack&>(err));
					break;
			}

			auto it = p_func->get_op_dbg().lower_bound(last_ip);
			uint32_t line = (it != p_func->get_op_dbg().end()) ? line = (uint32_t)it->second : 0;
			String func;
			if (p_func->get_owner() != nullptr && p_func->get_owner()->is_class()) {
				func = p_func->get_owner()->get_name() + "." + p_func->get_name();
			} else {
				func = p_func->get_name();
			}

			throw TraceBack(nested, DBGSourceInfo(context.bytecode_file->get_name(), line, func), _DBG_SOURCE);
		}

	}
	THROW_BUG("can't reach here");
}

int VM::run(ptr<Bytecode> bytecode, stdvec<String> args) {

	const Function* main = bytecode->get_main();
	if (main == nullptr) {
		THROW_ERROR(Error::NULL_POINTER, "entry point was null");
	}

	ASSERT(main->get_arg_count() <= 1); // main() or main(args)

	stdvec<var*> call_args; var argv = Array();
	if (main->get_arg_count() == 1) {
		for (const String& str : args) argv.operator Array().push_back(str);
		call_args.push_back(&argv);
	}
	var main_ret = call_function(main, bytecode.get(), nullptr, call_args);

	if (main_ret.get_type() == var::_NULL) return 0;
	if (main_ret.get_type() != var::INT) THROW_ERROR(Error::TYPE_ERROR, "main function returned a non integer value");
	return main_ret.operator int();
}

var* RuntimeContext::get_var_at(const Address& p_addr) {
	static var _null;

	uint32_t index = p_addr.get_index();
	switch (p_addr.get_type()) {
		case Address::_NULL: {
			_null.clear();
			return &_null;
		} break;

		case Address::STACK: {
			return stack->get_at(index);
		} break;

		case Address::PARAMETER: {
			ASSERT(curr_fn != nullptr && curr_fn->get_is_args_ref().size() > index);
			int value_index = get_arg_value_index(index);
			if (value_index < 0) { // argument is reference
				return (*args)[index];
			} else { // argument is reference
				return &value_args[value_index];
			}
		} break;

		case Address::THIS: {
			return &self;
		} break;
		case Address::EXTERN: {
			const String& name = get_name_at(index);
			ASSERT(bytecode_file->get_externs().find(name) != bytecode_file->get_externs().end());
			return bytecode_file->_get_member_var_ptr(name);
		} break;
		case Address::NATIVE_CLASS: {
			const String& name = get_name_at(index);
			return vm->_get_native_ref(name);
		} break;
		case Address::BUILTIN_FUNC: {
			return vm->_get_builtin_func_ref(index);
		} break;
		case Address::BUILTIN_TYPE: {
			return vm->_get_builtin_type_ref(index);
		} break;
		case Address::MEMBER_VAR: {
			stdvec<var>& members = self.operator ptr<Instance>()->members;
			THROW_INVALID_INDEX(members.size(), index);
			return &members[index];
		} break;
		case Address::STATIC_MEMBER: {
			const String& name = get_name_at(index);
			var* member = nullptr;
			if (self.get_type() != var::_NULL) member = self.operator ptr<Instance>()->blueprint->_get_member_var_ptr(name);
			if (!member && bytecode_class) member = bytecode_class->_get_member_var_ptr(name);
			if (!member) member = bytecode_file->_get_member_var_ptr(name);
			return member;
		} break;
		case Address::CONST_VALUE: {
			return bytecode_file->get_global_const_value(index);
		} break;

		MISSED_ENUM_CHECK(Address::CONST_VALUE, 10);
	}
	THROW_BUG("can't reach here");
}

int RuntimeContext::get_arg_value_index(int p_arg_ind) const {
	auto& is_ref = curr_fn->get_is_args_ref();
	int value_index = -1;
	for (int i = 0; i < is_ref.size(); i++) {
		if (!is_ref[i]) value_index++;
		if (i == p_arg_ind) return (is_ref[i]) ? -1 : value_index;
	}
	return -1;
}

const String& RuntimeContext::get_name_at(uint32_t p_pos) {
	return bytecode_file->get_global_name(p_pos);
}

var* VM::_get_native_ref(const String& p_name) {
	auto it = _native_ref.find(p_name);
	if (it != _native_ref.end()) { return &it->second; }

	ASSERT(NativeClasses::singleton()->is_class_registered(p_name));
	var new_ref = newptr<NativeClassRef>(p_name);
	_native_ref[p_name] = new_ref;
	return &_native_ref[p_name];
}

var* VM::_get_builtin_func_ref(uint32_t p_type) {
	ASSERT(p_type < BuiltinFunctions::Type::_FUNC_MAX_);

	auto it = _builtin_func_ref.find(p_type);
	if (it != _builtin_func_ref.end()) { return &it->second; }

	var new_ref = newptr<BuiltinFuncRef>((BuiltinFunctions::Type)p_type);
	_builtin_func_ref[p_type] = new_ref;
	return &_builtin_func_ref[p_type];
}

var* VM::_get_builtin_type_ref(uint32_t p_type) {
	ASSERT(p_type < BuiltinTypes::Type::_TYPE_MAX_);

	auto it = _builtin_type_ref.find(p_type);
	if (it != _builtin_type_ref.end()) { return &it->second; }

	var new_ref = newptr<BuiltinTypeRef>((BuiltinTypes::Type)p_type);
	_builtin_type_ref[p_type] = new_ref;
	return &_builtin_type_ref[p_type];
}

}

#endif //_FILE_SRC_COMPILER_VM_CPP_

#ifndef _FILE_SRC_MAIN_MAIN_CPP_

#ifndef CARBON_NO_MAIN

using namespace carbon;

#define CARBON_INCLUDE_CRASH_HANDLER_MAIN
#define CARBON_CRASH_HANDLER_IMPLEMENTATION

/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2020 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2020 Godot Engine contributors (cf. AUTHORS.md).   */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

// This file is heavily modified version of the Godot's crahs_handler
// Usage:
//   #define CARBON_INCLUDE_CRASH_HANDLER_MAIN
//   #define CARBON_CRASH_HANDLER_IMPLEMENTATION
//   #include "crash_handler.h"

#if defined(PLATFORM_WINDOWS)

#include <Windows.h>
// #pragma comment(lib, "psapi.lib")
// #pragma comment(lib, "dbghelp.lib")

// Crash handler exception only enabled with MSVC
#if (defined(_DEBUG) || defined(DEBUG_BUILD)) && defined(_MSC_VER)
#define CRASH_HANDLER_EXCEPTION 1
extern DWORD CrashHandlerException(EXCEPTION_POINTERS* ep);
#endif

#elif defined(PLATFORM_LINUX)

#ifndef CRASH_HANDLER_X11_H
#define CRASH_HANDLER_X11_H

// LINK: 'dl'

class CrashHandler {

	bool disabled;

public:
	void initialize();

	void disable();
	bool is_disabled() const { return disabled; };

	CrashHandler();
	~CrashHandler();
};

#endif // CRASH_HANDLER_X11_H


#endif

#ifdef CARBON_INCLUDE_CRASH_HANDLER_MAIN

/***************************************************************************************************************************/
/*                                                CRASH HANDLER MAIN                                                       */
/***************************************************************************************************************************/

int _main(int, char**);

#if defined(PLATFORM_WINDOWS)

	int main(int argc, char** argv) {
	#ifdef CRASH_HANDLER_EXCEPTION
		__try {
			return _main(argc, argv);
		} __except (CrashHandlerException(GetExceptionInformation())) {
			return 1;
		}
	#else
		return _main(argc, argv);
	#endif
	}

#elif defined(PLATOFRM_LINUX)

	int main(int argc, char** argv) {
		CrashHandler crash_handler;
		crash_handler.initialize();
	
		_main(argc, argv);
	
		return 0;
	}

#else
	#define _main main

#endif

#endif // CARBON_INCLUDE_CRASH_HANDLER_MAIN

#if defined(CARBON_CRASH_HANDLER_IMPLEMENTATION) || (defined(CARBON_INCLUDE_CRASH_HANDLER_MAIN) && defined(CARBON_IMPLEMENTATION))
/***************************************************************************************************************************/
/*                                                CRASH HANDLER WINDOWS                                                    */
/***************************************************************************************************************************/

#if defined(PLATFORM_WINDOWS)

#ifdef CRASH_HANDLER_EXCEPTION

#include <windows.h>

// Backtrace code code based on: https://stackoverflow.com/questions/6205981/windows-c-stack-trace-from-a-running-app

#include <psapi.h>
#include <algorithm>
#include <iterator>
#include <vector>

// Some versions of imagehlp.dll lack the proper packing directives themselves
// so we need to do it.
#pragma pack(push, before_imagehlp, 8)
#include <imagehlp.h>
#pragma pack(pop, before_imagehlp)

struct module_data {
	std::string image_name;
	std::string module_name;
	void* base_address;
	DWORD load_size;
};

class symbol {
	typedef IMAGEHLP_SYMBOL64 sym_type;
	sym_type* sym;
	static const int max_name_len = 1024;

public:
	symbol(HANDLE process, DWORD64 address) :
		sym((sym_type*)::operator new(sizeof(*sym) + max_name_len)) {
		memset(sym, '\0', sizeof(*sym) + max_name_len);
		sym->SizeOfStruct = sizeof(*sym);
		sym->MaxNameLength = max_name_len;
		DWORD64 displacement;

		SymGetSymFromAddr64(process, address, &displacement, sym);
	}

	std::string name() { return std::string(sym->Name); }
	std::string undecorated_name() {
		if (*sym->Name == '\0')
			return "<couldn't map PC to fn name>";
		std::vector<char> und_name(max_name_len);
		UnDecorateSymbolName(sym->Name, &und_name[0], max_name_len, UNDNAME_COMPLETE);
		return std::string(&und_name[0], strlen(&und_name[0]));
	}
};

class get_mod_info {
	HANDLE process;

public:
	get_mod_info(HANDLE h) :
		process(h) {}

	module_data operator()(HMODULE module) {
		module_data ret;
		char temp[4096];
		MODULEINFO mi;

		GetModuleInformation(process, module, &mi, sizeof(mi));
		ret.base_address = mi.lpBaseOfDll;
		ret.load_size = mi.SizeOfImage;

		GetModuleFileNameEx(process, module, temp, sizeof(temp));
		ret.image_name = temp;
		GetModuleBaseName(process, module, temp, sizeof(temp));
		ret.module_name = temp;
		std::vector<char> img(ret.image_name.begin(), ret.image_name.end());
		std::vector<char> mod(ret.module_name.begin(), ret.module_name.end());
		SymLoadModule64(process, 0, &img[0], &mod[0], (DWORD64)ret.base_address, ret.load_size);
		return ret;
	}
};

DWORD CrashHandlerException(EXCEPTION_POINTERS* ep) {
	HANDLE process = GetCurrentProcess();
	HANDLE hThread = GetCurrentThread();
	DWORD offset_from_symbol = 0;
	IMAGEHLP_LINE64 line = { 0 };
	std::vector<module_data> modules;
	DWORD cbNeeded;
	std::vector<HMODULE> module_handles(1);

	if (IsDebuggerPresent()) {
		return EXCEPTION_CONTINUE_SEARCH;
	}

	fprintf(stderr, "%s: Program crashed\n", __FUNCTION__);

	// Load the symbols:
	if (!SymInitialize(process, nullptr, false))
		return EXCEPTION_CONTINUE_SEARCH;

	SymSetOptions(SymGetOptions() | SYMOPT_LOAD_LINES | SYMOPT_UNDNAME);
	EnumProcessModules(process, &module_handles[0], (DWORD)module_handles.size() * sizeof(HMODULE), &cbNeeded);
	module_handles.resize(cbNeeded / sizeof(HMODULE));
	EnumProcessModules(process, &module_handles[0], (DWORD)module_handles.size() * sizeof(HMODULE), &cbNeeded);
	std::transform(module_handles.begin(), module_handles.end(), std::back_inserter(modules), get_mod_info(process));
	void* base = modules[0].base_address;

	// Setup stuff:
	CONTEXT* context = ep->ContextRecord;
	STACKFRAME64 frame;
	bool skip_first = false;

	frame.AddrPC.Mode = AddrModeFlat;
	frame.AddrStack.Mode = AddrModeFlat;
	frame.AddrFrame.Mode = AddrModeFlat;

#ifdef _M_X64
	frame.AddrPC.Offset = context->Rip;
	frame.AddrStack.Offset = context->Rsp;
	frame.AddrFrame.Offset = context->Rbp;
#else
	frame.AddrPC.Offset = context->Eip;
	frame.AddrStack.Offset = context->Esp;
	frame.AddrFrame.Offset = context->Ebp;

	// Skip the first one to avoid a duplicate on 32-bit mode
	skip_first = true;
#endif

	line.SizeOfStruct = sizeof(line);
	IMAGE_NT_HEADERS* h = ImageNtHeader(base);
	DWORD image_type = h->FileHeader.Machine;

	fprintf(stderr, "Dumping the backtrace.\n");

	int n = 0;
	do {
		if (skip_first) {
			skip_first = false;
		} else {
			if (frame.AddrPC.Offset != 0) {
				std::string fnName = symbol(process, frame.AddrPC.Offset).undecorated_name();

				if (SymGetLineFromAddr64(process, frame.AddrPC.Offset, &offset_from_symbol, &line))
					fprintf(stderr, "[%d] %s (%s:%d)\n", n, fnName.c_str(), line.FileName, line.LineNumber);
				else
					fprintf(stderr, "[%d] %s\n", n, fnName.c_str());
			} else
				fprintf(stderr, "[%d] ???\n", n);

			n++;
		}

		if (!StackWalk64(image_type, process, hThread, &frame, context, nullptr, SymFunctionTableAccess64, SymGetModuleBase64, nullptr))
			break;
	} while (frame.AddrReturn.Offset != 0 && n < 256);

	fprintf(stderr, "-- END OF BACKTRACE --\n");

	SymCleanup(process);

	// Pass the exception to the OS
	return EXCEPTION_CONTINUE_SEARCH;
}
#endif

/***************************************************************************************************************************/
/*                                                CRASH HANDLER LINUX                                                      */
/***************************************************************************************************************************/

#elif defined(PLATFORM_LINUX)


#ifdef DEBUG_BUILD
#include <cxxabi.h>
#include <dlfcn.h>
#include <execinfo.h>
#include <signal.h>
#include <stdlib.h>

#include <string>
#include <vector>
#include <unistd.h>
#include <sys/wait.h>

// TODO: move this to os related place
std::string _get_exec_path() {
	int len = 1024;
	char pBuf[len];
#ifdef _WIN32
	int bytes = GetModuleFileName(NULL, pBuf, sizeof pBuf);
	// TODO:  assert bytes > 0
#elif __linux__
	char szTmp[32];
	sprintf(szTmp, "/proc/%d/exe", getpid());

	// int bytes = min(readlink(szTmp, pBuf, len), len - 1);
	int bytes = readlink(szTmp, pBuf, len);
	if (bytes > len - 1) bytes = len - 1;

	if (bytes >= 0)
		pBuf[bytes] = '\0';
#endif
	return pBuf;
}

int _execute(const std::string& p_path, const std::vector<std::string>& p_arguments, bool p_blocking,
	int* r_child_id, std::string* r_pipe, int* r_exitcode, bool read_stderr = true/*,Mutex *p_pipe_mutex*/) {

#ifdef __EMSCRIPTEN__
	// Don't compile this code at all to avoid undefined references.
	// Actual virtual call goes to OS_JavaScript.
	ERR_FAIL_V(ERR_BUG);
#else
	if (p_blocking && r_pipe) {

		std::string argss;
		argss = "\"" + p_path + "\"";

		for (int i = 0; i < p_arguments.size(); i++) {

			argss += std::string(" \"") + p_arguments[i] + "\"";
		}

		if (read_stderr) {
			argss += " 2>&1"; // Read stderr too
		} else {
			argss += " 2>/dev/null"; //silence stderr
		}
		FILE* f = popen(argss.c_str(), "r");

		if (!f) {
			printf("ERR_CANT_OPEN, Cannot pipe stream from process running with following arguments\n\t%s.\n", argss.c_str());
			return -1;
		}

		char buf[65535];

		while (fgets(buf, 65535, f)) {
			(*r_pipe) += std::string(buf);
		}
		int rv = pclose(f);
		if (r_exitcode)
			*r_exitcode = rv;

		return 0;
	}

	pid_t pid = fork();
	if (pid < 0) {
		printf("ERR_CANT_FORK\n");
		return -1;
	}

	if (pid == 0) {
		// is child

		if (!p_blocking) {
			// For non blocking calls, create a new session-ID so parent won't wait for it.
			// This ensures the process won't go zombie at end.
			setsid();
		}

		std::vector<std::string> cs;

		cs.push_back(p_path);
		for (int i = 0; i < p_arguments.size(); i++)
			cs.push_back(p_arguments[i]);

		std::vector<char*> args;
		for (int i = 0; i < cs.size(); i++)
			args.push_back((char*)cs[i].c_str());
		args.push_back(0);

		execvp(p_path.c_str(), &args[0]);
		// still alive? something failed..
		fprintf(stderr, "**ERROR** OS_Unix::execute - Could not create child process while executing: %s\n", p_path.c_str());
		abort();
	}

	if (p_blocking) {

		int status;
		waitpid(pid, &status, 0);
		if (r_exitcode)
			*r_exitcode = status;

	} else {

		if (r_child_id)
			*r_child_id = pid;
	}

	return 0;
#endif
}

// FIXME: chage it to string split
std::string _func_offset(const char* string_symbol) {

	// the backtrace_symbol output:
	// /home/thakeenathees/dev/carbon/bin/carbon.x11.debug.64(+0x2801) [0x55c5aa0a2801]
	// from that it'll extract the offset (0x2801) and feed to addr2line

	size_t i = 0;
	bool copy = false;
	std::string offset;
	while (char c = string_symbol[i++]) {
		if (c == ')') break;
		if (copy) offset += c;
		if (c == '+') copy = true;
	}
	return offset;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////

static void handle_crash(int sig) {

	void* bt_buffer[256];
	size_t size = backtrace(bt_buffer, 256);
	std::string _execpath = _get_exec_path();

	// Dump the backtrace to stderr with a message to the user
	fprintf(stderr, "%s: Program crashed with signal %d\n", __FUNCTION__, sig);

	fprintf(stderr, "Dumping the backtrace.\n");
	char** strings = backtrace_symbols(bt_buffer, size);
	if (strings) {
		for (size_t i = 1; i < size; i++) {

			/* fname not working like it works in godot!! using backtrace_symbol string instead to get method
						char fname[1024];
						Dl_info info;
						snprintf(fname, 1024, "%s", strings[i]);
						// Try to demangle the function name to provide a more readable one
						if (dladdr(bt_buffer[i], &info) && info.dli_sname) {
							if (info.dli_sname[0] == '_') {
								int status;
								char *demangled = abi::__cxa_demangle(info.dli_sname, nullptr, nullptr, &status);
								if (status == 0 && demangled) {
									snprintf(fname, 1024, "%s", demangled);
								}
								if (demangled)
									free(demangled);
							}
						}
			*/
			std::vector<std::string> args;

			//char str[1024];
			//snprintf(str, 1024, "%p", bt_buffer[i]);
			// godot using this but It's not working for some reason
			// but the offset from the backtrace_symbols working, why?
			// using args.push_back(_func_offset(strings[i])); instead

			args.push_back(_func_offset(strings[i]));
			args.push_back("-e");
			args.push_back(_execpath);
			args.push_back("-f");
			args.push_back("--demangle");
			args.push_back("-p");

			std::string output = "";

			// Try to get the file/line number using addr2line
			int ret;
			int err = _execute("addr2line", args, true, nullptr, &output, &ret);
			if (err == 0) {
				output.erase(output.length() - 1, 1);
			}

			if (output.find(" ??:") != std::string::npos) { // _start at ??:0 no symbol found
				fprintf(stderr, "[%ld] <<unresolved symbols>> at %s\n", (long int)i, /*fname,*/ strings[i]);
			} else {
				fprintf(stderr, "[%ld] %s\n", (long int)i, /*fname,*/ output.c_str());
			}

		}

		free(strings);
	}
	fprintf(stderr, "-- END OF BACKTRACE --\n");

	// Abort to pass the error to the OS
	abort();
}
#endif

CrashHandler::CrashHandler() {
	disabled = false;
}

CrashHandler::~CrashHandler() {
	disable();
}

void CrashHandler::disable() {
	if (disabled)
		return;

#ifdef DEBUG_BUILD
	signal(SIGSEGV, nullptr);
	signal(SIGFPE, nullptr);
	signal(SIGILL, nullptr);
#endif

	disabled = true;
}

void CrashHandler::initialize() {
#ifdef DEBUG_BUILD
	signal(SIGSEGV, handle_crash);
	signal(SIGFPE, handle_crash);
	signal(SIGILL, handle_crash);
#endif
}


#endif
#endif // CARBON_CRASH_HANDLER_IMPLEMENTATION




inline void log_copyright_and_license() {
	Logger::log(1 + R"(
Carbon 1.0.0 (https://github.com/ThakeeNathees/carbon/)
Copyright (c) 2020-2021 ThakeeNathees.
Free and open source software under the terms of the MIT license.

)");
}

inline void log_help() {
	Logger::log(1 + R"(
TODO: THESE HELP TEXT ARE DUMMY
usage: carbon [options] file ...
Options:
    -h, --help          : Display this help message.
    -v, --version       : Display the version.
    -o                  : Output path.
    -w                  : Warnings are treated as errors.
    -I(path)            : Import search path.
)");
}

int _main(int argc, char** argv) {

	carbon_initialize();
	//log_copyright_and_license();

	try {
		if (argc < 2) {
			log_copyright_and_license();
			log_help();
		} else {
			// TODO: properly parse command line args
			stdvec<String> args;
			for (int i = 1; i < argc; i++) args.push_back(argv[i]);

			ptr<Bytecode> bytecode = Compiler::singleton()->compile(argv[1]);
			VM::singleton()->run(bytecode, args);
		}
	} catch (Throwable& err) {
		err.console_log();
	}

	carbon_cleanup();
	return 0;
}

#endif // CARBON_NO_MAIN

#endif //_FILE_SRC_MAIN_MAIN_CPP_

#ifndef _FILE_SRC_CORE_PLATFORM_WINDOWS_CPP_


#ifdef PLATFORM_WINDOWS

#ifndef NOMINMAX // mingw already has defined for us.
	#define NOMINMAX
#endif
#include <Windows.h>
#undef ERROR
#include <tchar.h> 
#include <stdio.h>
#include <strsafe.h>

#include <direct.h>


namespace carbon {

static void _set_console_color(Console::Color p_forground, Console::Color p_background = Console::Color::DEFAULT) {
	if (p_background == Console::Color::DEFAULT) p_background = Console::Color::BLACK;
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), (int)p_background << 4 | (int)p_forground);
}

static std::string get_last_error_as_string() {
	DWORD errorMessageID = ::GetLastError();
	if (errorMessageID == 0) {
		return ""; //No error message has been recorded
	} else {
		return std::system_category().message(errorMessageID);
	}
}

// ------------------------------------------------------------------------

void _Platform::console_get_cursor(int* p_line, int* p_column) {
	CONSOLE_SCREEN_BUFFER_INFO cbsi;
	COORD pos;
	if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cbsi)) {
		pos = cbsi.dwCursorPosition;
	} else {
		THROW_ERROR(Error::IO_ERROR, get_last_error_as_string());
	}
	*p_line = pos.X;
	*p_column = pos.Y;
}

void _Platform::console_set_cursor(int p_line, int p_column) {
	COORD coord;
	coord.X = p_line;
	coord.Y = p_column;

	if (!SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord)) {
		THROW_ERROR(Error::IO_ERROR, get_last_error_as_string());
	}
}

void _Platform::console_logf(const char* p_fmt, va_list p_args, bool p_stderr, Console::Color p_forground, Console::Color p_background) {
	if (p_forground == Console::Color::DEFAULT) p_forground = Console::Color::L_WHITE;
	if (p_background == Console::Color::DEFAULT) p_background = Console::Color::BLACK;
	static const unsigned int BUFFER_SIZE = VSNPRINTF_BUFF_SIZE;
	char buf[BUFFER_SIZE + 1]; // +1 for the terminating character
	int len = vsnprintf(buf, BUFFER_SIZE, p_fmt, p_args);

	if (len <= 0) return;
	if ((unsigned int)len >= BUFFER_SIZE) len = BUFFER_SIZE; // Output is too big, will be truncated
	buf[len] = 0;

	_set_console_color(p_forground, p_background);
	if (p_stderr) fprintf(stderr, "%s", buf);
	else fprintf(stdout, "%s", buf);
	_set_console_color(Console::Color::L_WHITE);
}

void _Platform::console_log(const char* p_message, bool p_stderr, Console::Color p_forground, Console::Color p_background) {
	if (p_forground == Console::Color::DEFAULT) p_forground = Console::Color::L_WHITE;
	if (p_background == Console::Color::DEFAULT) p_background = Console::Color::BLACK;
	_set_console_color(p_forground, p_background);
	if (p_stderr) {
		fprintf(stderr, "%s", p_message);
	} else {
		fprintf(stdout, "%s", p_message);
	}
	_set_console_color(Console::Color::L_WHITE);
}


std::string _Platform::os_getcwd() {
	char* buffer = _getcwd(NULL, 0);
	if (buffer == NULL) {
		THROW_ERROR(Error::IO_ERROR, "_getcwd returned NULL");
	}
	String cwd = buffer;
	free(buffer);
	return cwd;
}

void _Platform::os_chdir(const std::string& p_path) {
	if (_chdir(p_path.c_str())) {
		switch (errno) {
			case ENOENT:
				THROW_ERROR(Error::IO_ERROR, String::format("Unable to locate the directory: %s.", p_path.c_str()));
			case EINVAL:
				THROW_ERROR(Error::IO_ERROR, "Invalid buffer.");
			default:
				THROW_ERROR(Error::IO_ERROR, "Unknown error.");
		}
	}
}

std::string _Platform::path_absolute(const std::string& p_path) {
	char buffer[VSNPRINTF_BUFF_SIZE];
	if (GetFullPathNameA(p_path.c_str(), VSNPRINTF_BUFF_SIZE, buffer, NULL) == 0) {
		THROW_ERROR(Error::IO_ERROR, get_last_error_as_string());
	}
	return buffer;
}

bool _Platform::path_exists(const std::string& p_path) {
	DWORD ftyp = GetFileAttributesA(p_path.c_str());
	if (ftyp == INVALID_FILE_ATTRIBUTES) return false;  //something is wrong with your path!

	if (ftyp & FILE_ATTRIBUTE_DIRECTORY) return true;   // this is a directory!
	else return true;
}

bool _Platform::path_isdir(const std::string& p_path) {
	DWORD ftyp = GetFileAttributesA(p_path.c_str());
	if (ftyp == INVALID_FILE_ATTRIBUTES) return false;  //something is wrong with your path!

	if (ftyp & FILE_ATTRIBUTE_DIRECTORY) return true;   // this is a directory!
	else return false;									// this is not a directory!
}

stdvec<std::string> _Platform::path_listdir(const std::string& p_path) {
	// reference: https://docs.microsoft.com/en-us/windows/win32/fileio/listing-the-files-in-a-directory
	WIN32_FIND_DATA ffd;
	//LARGE_INTEGER filesize;
	TCHAR szDir[MAX_PATH];
	size_t path_len;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	DWORD dwError = 0;

	StringCchLength(p_path.c_str(), MAX_PATH, &path_len);
	if (path_len > (MAX_PATH - 3)) {
		THROW_ERROR(Error::IO_ERROR, String::format("Directory path is too long (%s)", p_path.c_str()));
	}

	// Prepare string for use with FindFile functions.  First, copy the
   // string to a buffer, then append '\*' to the directory name.

	StringCchCopy(szDir, MAX_PATH, p_path.c_str());
	StringCchCat(szDir, MAX_PATH, TEXT("\\*"));

	// Find the first file in the directory.

	hFind = FindFirstFile(szDir, &ffd);

	if (INVALID_HANDLE_VALUE == hFind) {
		THROW_ERROR(Error::IO_ERROR, "Invalid handle");
	}

	stdvec<std::string> ret;

	do {
		//if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
		//	_tprintf(TEXT("  %s   <DIR>\n"), ffd.cFileName);
		//} else {
		//	filesize.LowPart = ffd.nFileSizeLow;
		//	filesize.HighPart = ffd.nFileSizeHigh;
		//	_tprintf(TEXT("  %s   %ld bytes\n"), ffd.cFileName, filesize.QuadPart);
		//}
		ret.push_back(ffd.cFileName);
	} while (FindNextFile(hFind, &ffd) != 0);

	dwError = GetLastError();
	if (dwError != ERROR_NO_MORE_FILES) {
		THROW_ERROR(Error::IO_ERROR, "Invalid handle");
	}
	FindClose(hFind);

	return ret;
}

}

#endif // PLATFORM_WINDOWS

#endif //_FILE_SRC_CORE_PLATFORM_WINDOWS_CPP_

#ifndef _FILE_SRC_CORE_PLATFORM_X11_CPP_
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


#ifdef PLATFORM_LINUX

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

namespace carbon {

void _Platform::console_get_cursor(int* p_line, int* p_column) {
	THROW_BUG("TODO:");
}

void _Platform::console_set_cursor(int p_line, int p_column) {
	printf("\033[%d;%dH", p_column + 1, p_line + 1);
}

void _Platform::console_logf(const char* p_fmt, va_list p_args, bool p_stderr, Console::Color p_forground, Console::Color p_background) {
	static const unsigned int BUFFER_SIZE = VSNPRINTF_BUFF_SIZE;
		char buf[BUFFER_SIZE + 1]; // +1 for the terminating character
		int len = vsnprintf(buf, BUFFER_SIZE, p_fmt, p_args);
		
		if (len <= 0) return;
		if ((unsigned int)len >= BUFFER_SIZE) len = BUFFER_SIZE; // Output is too big, will be truncated
		buf[len] = 0;

		console_log(buf, p_stderr, p_forground, p_background);
}

void _Platform::console_log(const char* p_msg, bool p_stderr, Console::Color p_forground, Console::Color p_background) {

    enum ANSI_Codes {
		FG_OFFSET      = 0,
		BG_OFFSET      = 10, // fg, bg offset = 10.

		FMT_DEFAULT    = 0,
		FMT_BOLD       = 1,
		FMT_DIM        = 2,
		FMT_UNDERLINED = 3,
		FMT_BLINK      = 5,
		FMT_REVERSE    = 7,
		FMT_HIDDEN     = 8,

		DEFAULT        = 39,

		BLACK          = 30,
		RED            = 31,
		GREEN          = 32,
		YELLOW         = 33,
		BLUE           = 34,
		MAGENTA        = 35,
		WHITE          = 37,
		GRAY           = 90,
		BRIGHT_RED     = 91,
		BRIGHT_GREEN   = 92,
		BRIGHT_YELLOW  = 93,
		BRIGHT_BLUE    = 94,
		BRIGHT_MEGENTA = 95,
		CYAN           = 96,
		BRIGHT_WHITE   = 97,
	};

	#define MAP_COLOR(m_target, m_offset)                                                         \
	switch (p_##m_target) {                                                                       \
		case Console::Color::DEFAULT:   /*[[fallthrought]]*/                                      \
		case Console::Color::BLACK:     m_target = ANSI_Codes::DEFAULT + m_offset;        break;  \
		case Console::Color::L_BLUE:    m_target = ANSI_Codes::BLUE + m_offset;           break;  \
		case Console::Color::L_GREEN:   m_target = ANSI_Codes::GREEN + m_offset;          break;  \
		case Console::Color::L_SKYBLUE: m_target = ANSI_Codes::CYAN + m_offset;           break;  \
		case Console::Color::L_RED:     m_target = ANSI_Codes::RED + m_offset;            break;  \
		case Console::Color::L_PINK:    m_target = ANSI_Codes::MAGENTA + m_offset;        break;  \
		case Console::Color::L_YELLOW:  m_target = ANSI_Codes::YELLOW + m_offset;         break;  \
		case Console::Color::L_WHITE:   m_target = ANSI_Codes::WHITE + m_offset;          break;  \
		case Console::Color::L_GRAY:    m_target = ANSI_Codes::GRAY + m_offset;           break;  \
		case Console::Color::D_BLUE:    m_target = ANSI_Codes::BRIGHT_BLUE + m_offset;    break;  \
		case Console::Color::D_GREEN:   m_target = ANSI_Codes::BRIGHT_GREEN + m_offset;   break;  \
		case Console::Color::D_SKYBLUE: m_target = ANSI_Codes::CYAN + m_offset;           break;  \
		case Console::Color::D_RED:     m_target = ANSI_Codes::BRIGHT_RED + m_offset;     break;  \
		case Console::Color::D_PINK:    m_target = ANSI_Codes::BRIGHT_MEGENTA + m_offset; break;  \
		case Console::Color::D_YELLOW:  m_target = ANSI_Codes::BRIGHT_YELLOW + m_offset;  break;  \
		case Console::Color::D_WHITE:   m_target = ANSI_Codes::BRIGHT_WHITE + m_offset;   break;  \
	}
	MISSED_ENUM_CHECK(Console::Color::__COLOR_MAX__, 16);

		int forground = ANSI_Codes::WHITE + ANSI_Codes::FG_OFFSET;
		int background = ANSI_Codes::BLACK + ANSI_Codes::BG_OFFSET;
		MAP_COLOR(forground, ANSI_Codes::FG_OFFSET);
		MAP_COLOR(background, ANSI_Codes::BG_OFFSET);
		if (!p_stderr) {
			fprintf(stdout, "\033[%i;%i;%im%s\033[%im", ANSI_Codes::FMT_DEFAULT, forground, background, p_msg, ANSI_Codes::FMT_DEFAULT);
		} else {
			fprintf(stderr, "\033[%i;%i;%im%s\033[%im", ANSI_Codes::FMT_DEFAULT, forground, background, p_msg, ANSI_Codes::FMT_DEFAULT);
		}
}


std::string _Platform::os_getcwd() {
	char buffer[VSNPRINTF_BUFF_SIZE];
    if (getcwd(buffer, sizeof(buffer)) == NULL) {
        THROW_ERROR(Error::IO_ERROR, "getcwd() returned NULL.");
    }
    return buffer;
}

void _Platform::os_chdir(const std::string& p_path) {
	if (chdir(p_path.c_str()) != 0){
        THROW_ERROR(Error::IO_ERROR, "chdir() failed.");
    }
}

std::string _Platform::path_absolute(const std::string& p_path) {
	char* path = realpath(p_path.c_str(), NULL);
    String ret;
    if (path != NULL) {
        ret = path;
        free(path);
    }
    return ret;
}

bool _Platform::path_exists(const std::string& p_path) {
	struct stat _stat;
	return stat(p_path.c_str(), &_stat) == 0;
}

bool _Platform::path_isdir(const std::string& p_path) {
	struct stat _stat;
	if (stat(p_path.c_str(), &_stat) == 0 && (_stat.st_mode & S_IFMT == S_IFDIR))
        return true;
    return false;
}

} // namespace carbon

#endif // PLATFORM_WINDOWS

#endif //_FILE_SRC_CORE_PLATFORM_X11_CPP_


#endif // CARBON_IMPLEMENTATION

