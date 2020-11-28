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

#include "_map.h"
#include "var.h"
#include "error.h"
#include "type_info.h"
#include "var_private.h"

namespace carbon {
	
const stdmap<size_t, ptr<MemberInfo>>& TypeInfo::get_member_info_list_map() {
	static stdmap<size_t, ptr<MemberInfo>> member_info = {
		_NEW_METHOD_INFO("size",                                          var::INT   ),
		_NEW_METHOD_INFO("empty",                                         var::BOOL  ),
		_NEW_METHOD_INFO("insert",  _PARAMS("what"),  _TYPES(var::VAR),   var::_NULL ),
		_NEW_METHOD_INFO("clear",                                         var::_NULL ),
		_NEW_METHOD_INFO("has",     _PARAMS("what"),  _TYPES(var::VAR),   var::BOOL  ),
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
		ss << it->second.key.to_string() << " : " << it->second.value.to_string();
	}
	ss << " }";
	return ss.str();
}

Map Map::copy(bool p_deep) const {
	Map ret;
	for (_map_internal_t::iterator it = (*_data).begin(); it != (*_data).end(); it++) {
		if (p_deep) {
			ret[(int64_t)it->first] = it->second.value.copy(true);
		} else {
			ret[(int64_t)it->first] = it->second.value;
		}
	}
	return ret;
}

// TODO: error message.
#define _INSERT_KEY_IF_HAVENT(m_key)                            \
	_map_internal_t::iterator it = (*_data).find(m_key.hash()); \
	if (it == _data->end()) (*_data)[m_key.hash()].key = m_key

var Map::operator[](const var& p_key) const { _INSERT_KEY_IF_HAVENT(p_key); return (*_data).operator[](p_key.hash()).value;  }
var& Map::operator[](const var& p_key) { _INSERT_KEY_IF_HAVENT(p_key); return (*_data).operator[](p_key.hash()).value; }
var Map::operator[](const char* p_key) const { _INSERT_KEY_IF_HAVENT(var(p_key)); return (*_data).operator[](var(p_key).hash()).value; }
var& Map::operator[](const char* p_key) { _INSERT_KEY_IF_HAVENT(var(p_key)); return (*_data).operator[](var(p_key).hash()).value; }

Map::_map_internal_t::iterator Map::begin() const { return (*_data).begin(); }
Map::_map_internal_t::iterator Map::end() const { return (*_data).end(); }
Map::_map_internal_t::iterator Map::find(const var& p_key) const { return (*_data).find(p_key.hash()); }

void Map::insert(const var& p_key, const var& p_value) {
	(*_data).insert(std::pair<size_t, _KeyValue>(p_key.hash(), _KeyValue(p_key, p_value)));
}

bool Map::has(const var& p_key) const { return find(p_key) != end(); }
void Map::clear() { _data->clear(); }

size_t Map::size() const { return _data->size(); }
bool Map::empty() const { return _data->empty(); }

bool Map::operator ==(const Map& p_other) const {
	if (size() != p_other.size())
		return false;
	for (_map_internal_t::iterator it_other = p_other.begin(); it_other != p_other.end(); it_other++) {
		_map_internal_t::iterator it_self = find(it_other->second.key);
		if (it_self == end()) return false;
		if (it_self->second.value != it_other->second.value) return false;

	}
	return true;
}

Map::operator bool() const { return empty(); }

Map& Map::operator=(const Map& p_other) {
	_data = p_other._data;
	return *this;
}

} // namespace carbon