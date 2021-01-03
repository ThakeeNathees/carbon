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

#include "var/_string.h"

#include "var/_array.h"
#include "var/var.h"
#include "core/type_info.h"
#include "var/var_internal.h"

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

						// TODO: support more formatings
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
		return (*_data)[p_index];
	if ((int64_t)size() * -1 <= p_index && p_index < 0)
		return (*_data)[size() + p_index];
	THROW_ERROR(Error::INVALID_INDEX, String::format("String index %i is invalid.", p_index));
}
char& String::operator[](int64_t p_index) {
	if (0 <= p_index && p_index < (int64_t)size())
		return (*_data)[p_index];
	if ((int64_t)size() * -1 <= p_index && p_index < 0)
		return (*_data)[size() + p_index];
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
	return _data->substr(p_start, p_end - p_start);
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
	size_t start = 0, end = 0;
	if (p_delimiter == "") { // split by white space
		THROW_BUG("TODO:"); // use isspace(c)
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
	for (int i = 0; i < p_elements.size(); i++) {
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

} // namespace carbon