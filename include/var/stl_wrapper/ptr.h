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

#ifndef  PTR_H
#define  PTR_H

#include <memory>

namespace carbon {

template<typename T>
class _ptr {
	std::shared_ptr<T> _data;
public:
	_ptr() {}
	_ptr(std::shared_ptr<T>& other) { _data = other; }
	_ptr(std::nullptr_t other) { _data = other; }
	template<typename T2> _ptr(T* other) { _data = other; }

	template<typename T2> void operator =(const _ptr<T2> other) { _data = other; }
	template<typename T2> void operator =(const T* other) { _data = other; }

	void operator =(std::nullptr_t other) { _data = other; }
	bool operator == (std::nullptr_t other) { return _data == other; }
	bool operator != (std::nullptr_t other) { return _data != other; }
	operator bool() const { return _data.operator bool(); }

	template<typename T2> operator _ptr<T2>() const { return std::static_pointer_cast<T2>(_data); }
	template<typename T2> _ptr<T2> _to() { return std::static_pointer_cast<T2>(_data); }

	T* get() const { return _data.get(); }
	T* operator->() const { return _data.get(); }
	T& operator*() const { return *_data; }
};
template<typename T> bool operator ==(_ptr<T> p, std::nullptr_t) { return p.operator==(nullptr); }
template<typename T> bool operator !=(_ptr<T> p, std::nullptr_t) { return p.operator!=(nullptr); }

}

#endif // PTR_H