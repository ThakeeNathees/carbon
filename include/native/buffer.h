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

#include "core/native.h"

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