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

#ifndef INSTANCE_H
#define INSTANCE_H

#include "core/core.h"
#include "bytecode.h"

namespace carbon {

class Instance : public Object, public std::enable_shared_from_this<Instance> {
	REGISTER_CLASS(Instance, Object) {}
	friend class VM;
	friend struct RuntimeContext;

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

	// TODO: implement all the operator methods here.
	var __call(stdvec<var*>& p_args) override;
	String to_string() override;

};

}

#endif // INSTANCE_H