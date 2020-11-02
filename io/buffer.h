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

#ifndef BUFFER_H
#define BUFFER_H

#include "core.h"

namespace carbon {

class Buffer : public Object {
	REGISTER_CLASS(Buffer, Object) {
		BIND_STATIC_FUNC("Buffer", &Buffer::_Buffer, PARAMS("self", "size"), DEFVALUES(0));
		BIND_METHOD("alloc", &Buffer::alloc, PARAMS("size"));
		BIND_METHOD("size", &Buffer::size);
	}

public:

	Buffer(size_t p_size = 0) { alloc(p_size); }
	static void _Buffer(ptr<Object> self, int64_t p_size = 0) { ptrcast<Buffer>(self)->alloc(p_size); }

	// Methods.
	void alloc(size_t p_size) {
		_buffer = ptr<byte_t[]>(new byte_t[p_size]);
		_size = p_size;
	}

	byte_t* front() { return _buffer.get(); }

	size_t size() const { return _size; }

	byte_t& operator[](size_t p_index) {
		THROW_INVALID_INDEX(_size, p_index);
		return _buffer.get()[p_index];
	}
	const byte_t& operator[](size_t p_index) const {
		THROW_INVALID_INDEX(_size, p_index);
		return _buffer.get()[p_index];
	}

	// operators
	virtual var __get_mapped(const var& p_key) const override {
		if (p_key.get_type() != var::INT) THROW_ERROR(Error::TYPE_ERROR, "expected an integer as key.");
		int64_t key = p_key.operator int64_t();
		THROW_INVALID_INDEX(_size, key);
		return (int64_t)_buffer.get()[key];
	}
	virtual void __set_mapped(const var& p_key, const var& p_value) override {
		if (p_key.get_type() != var::INT) THROW_ERROR(Error::TYPE_ERROR, "expected an integer as key.");
		if (p_value.get_type() != var::INT) THROW_ERROR(Error::TYPE_ERROR, "expected an integer as value.");
		int64_t key = p_key.operator int64_t();
		int64_t value = p_value.operator int64_t();
		if (value < 0 || 0xff < value) THROW_ERROR(Error::TYPE_ERROR, String::format("integer value %lli is invalid for a buffer byte of range 0 - 0xff", value));
		THROW_INVALID_INDEX(_size, key);
		_buffer.get()[key] = (byte_t)value;
	}

protected:

private:
	// Members.
	ptr<byte_t[]> _buffer;
	size_t _size = 0;
};

}

#endif // BUFFER_H