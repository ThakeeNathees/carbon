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

#include "core_internal.h"

namespace carbon {

class var;
class Object;
class MemberInfo;
class Array;

class String {
public:

	// Constructors.
	String() : _data(new std::string(""))       {}
	String(const std::string& p_copy)  { _data = new std::string(p_copy); }
	String(const char* p_copy)         { _data = new std::string(p_copy); }
	String(const String& p_copy)       {
		if (p_copy._data) _data = new std::string(*p_copy._data);
	}
	String(char p_char)                         { _data = new std::string(1, p_char); }
	explicit String(int p_i)                    { _data = new std::string(std::to_string(p_i).c_str()); }
	explicit String(int64_t p_i)                { _data = new std::string(std::to_string(p_i).c_str()); }
	explicit String(size_t p_i)                 { _data = new std::string(std::to_string(p_i).c_str()); }
	explicit String(double p_d)                 { _data = new std::string(std::to_string(p_d).c_str()); }
	~String() { delete _data; }
	constexpr static  const char* get_type_name_s() { return "String"; }

	// reflection methods.
	var call_method(const String& p_method, const stdvec<var*>& p_args);
	static const stdmap<size_t, ptr<MemberInfo>>& get_member_info_list();
	static bool has_member(const String& p_member);
	static const ptr<MemberInfo> get_member_info(const String& p_member);

	static String format(const char* p_format, ...);
	int64_t to_int() const;
	double to_float() const { return std::stod(*_data); }
	String get_line(uint64_t p_line) const;
	size_t hash() const { return std::hash<std::string>{}(*_data); }
	size_t const_hash() const { return __const_hash(_data->c_str()); }

	String substr(size_t p_start, size_t p_end) const;
	bool endswith(const String& p_str) const;
	bool startswith(const String& p_str) const;
	Array split(const String& p_delimiter = "") const;
	// String strip(p_delemeter = " "); lstrip(); rstrip();

	// operators.
	char operator[](int64_t p_index) const;
	char& operator[](int64_t p_index);

	operator std::string() const                   { return *_data; }
	// operator bool() {} don't implement this don't even delete
	
	bool operator==(const String & p_other) const  { return *_data == *p_other._data; }
	bool operator!=(const String & p_other) const  { return *_data != *p_other._data; }
	bool operator<(const String& p_other) const    { return *_data < *p_other._data; }

	String operator+(char p_c) const               { return *_data + p_c; }
	String operator+(int p_i) const                { return *_data + std::to_string(p_i); }
	String operator+(double p_d) const             { return *_data + std::to_string(p_d); }
	String operator+(const char* p_cstr) const     { return *_data + p_cstr; }
	String operator+(const String& p_other) const  { return *_data + *p_other._data; }
	// String operator+(var& p_obj) const          { TODO: }

	String& operator+=(char p_c)                   { *_data += p_c;                 return *this; }
	String& operator+=(int p_i)                    { *_data += std::to_string(p_i); return *this; }
	String& operator+=(double p_d)                 { *_data += std::to_string(p_d); return *this; }
	String& operator+=(const char* p_cstr)         { *_data += p_cstr;              return *this; }
	String& operator+=(const String& p_other)      { *_data += *p_other._data;      return *this; }
	// String& operator+(var& p_obj)               { TODO: }

	String& operator=(char p_c)                   { *_data = p_c;                 return *this; }
	String& operator=(int p_i)                    { *_data = std::to_string(p_i); return *this; }
	String& operator=(double p_d)                 { *_data = std::to_string(p_d); return *this; }
	String& operator=(const char* p_cstr)         { *_data = p_cstr;              return *this; }
	String& operator=(const String& p_other)      { *_data = *p_other._data;      return *this; }
	// String& operator=(var& p_obj)               { TODO: }

	// Wrappers.
	size_t size() const                   { return _data->size(); }
	const char* c_str() const             { return _data->c_str(); }
	String& append(const String& p_other) { _data->append(p_other); return *this; }

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