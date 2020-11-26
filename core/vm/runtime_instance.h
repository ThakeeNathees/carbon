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

#ifndef RUNTIME_INSTANCE_H
#define RUNTIME_INSTANCE_H

#include "core.h"
#include "binary/bytecode.h"

namespace carbon {

class RuntimeInstance : public Object {
	REGISTER_CLASS(RuntimeInstance, Object) {}

	var call_method(const String& p_method_name, stdvec<var*>& p_args) override;
	var __call(stdvec<var*>& p_args) override;
	var get_member(const String& p_name) override {
		uint32_t pos = blueprint->get_member_index(p_name);
		return members[pos];
	}
	void set_member(const String& p_name, var& p_value) override {
		uint32_t pos = blueprint->get_member_index(p_name);
		members[pos] = p_value;
	}

	RuntimeInstance() {}
	RuntimeInstance(ptr<Bytecode>& p_blueprint) {
		blueprint = p_blueprint;
		members.resize(blueprint->get_member_count());
	}

	// TODO: implement all the operator methods here.

private:
	friend class VM;
	friend struct RuntimeContext;
	ptr<Bytecode> blueprint;
	ptr<Object> native_instance;
	stdvec<var> members;
	
	ptr<RuntimeInstance>* _self_ptr = nullptr; // not sure if it's a good idea.

};

}

#endif // RUNTIME_INSTANCE_H