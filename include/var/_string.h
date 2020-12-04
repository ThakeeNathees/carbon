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

#ifndef STRING_H
#define STRING_H

#include "internal.h"

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
	String substr(size_t p_start, size_t p_end) const;
	bool endswith(const String& p_str) const;
	bool startswith(const String& p_str) const;
	Array split(const String& p_delimiter = "") const;
	// String strip(p_delemeter = " "); lstrip(); rstrip();
	String join(const Array& p_elements) const;

	// operators.
	char operator[](int64_t p_index) const;
	char& operator[](int64_t p_index);

	// operator bool() {} don't implement this don't even delete
	operator std::string() const                   { return *_data; } // <-- TODO: remove this and find alternative.
	
	bool operator==(const String& p_other) const;
	bool operator!=(const String& p_other) const;
	bool operator<(const String& p_other) const;
	bool operator<=(const String& p_other) const;
	bool operator>(const String& p_other) const;
	bool operator>=(const String& p_other) const;

	String operator+(char p_c) const;
	String operator+(int p_i) const;
	String operator+(double p_d) const;
	String operator+(const char* p_cstr) const;
	String operator+(const String& p_other) const;
	// String operator+(var& p_obj) const          { TODO: }

	String& operator+=(char p_c);
	String& operator+=(int p_i);
	String& operator+=(double p_d);
	String& operator+=(const char* p_cstr);
	String& operator+=(const String& p_other);
	// String& operator+(var& p_obj)               { TODO: }

	String& operator=(char p_c);
	String& operator=(int p_i);
	String& operator=(double p_d);
	String& operator=(const char* p_cstr);
	String& operator=(const String& p_other);
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