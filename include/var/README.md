## var.h

A single header runtime type system library. It uses a type safe union of primitive and derived types. Basically it's the reinvention of the wheel [std::variant](https://en.cppreference.com/w/cpp/utility/variant), inspired from the [Godot Variants](https://github.com/godotengine/godot/blob/master/core/variant.h). This mainly used as the dynamic type system of the [carbon language](https://github.com/ThakeeNathees/carbon).

The library uses shared pointers to prevent memory leaks and manage memory and ownership of Objects. The primitive types and String passed by value and other types are passed by reference. A user defined type could be added to the dynamic type system by inheriting the Object and overriding it's methods.

### Usage
```c++

#define VAR_IMPLEMENTATION
#include "var.h"
using namespace varh;

#include <iostream>
#define print(x) std::cout << (x) << std::endl

int main() {
	var v;
	v = 3.14;           print(v); // prints 3.14 float
	v = "hello world!"; print(v); // prints the String

	v = String("string"); v = Vect2f(1.2, 3.4); v = Vect2i(1, 2);
	v = Map(); v = Array(1, 2.3, "hello world!", Array(4, 5, 6));

	class Aclass : public Object {
		public: String to_string() const { return "Aclass"; }
	};
	v = newptr<Aclass>(); print(v); // prints Aclass
}
```
Note that you must define `VAR_IMPLEMENTATION` in exactly one source file. You can `#include var.h` in as many files as you like. Define `UNDEF_VAR_DEFINES` to undef all the var.h internal macro like functions it you don't want them.

For operator overloading you have to override the "magic methods" (like in python) such as `__add`, `__sub`, `__lt`, `__eq`, `__call`, ... Operation on Basic types are python like ex:
```c++
var str = "abc";
str *= 3;           // str = "abcabcabc"
var arr = Array(1);
arr += Array("2");  // arr = [1, "2"]
arr *= 2;           // arr = [1, "2", 1, "2"]
```

Calling a methods/getting attributes by it's name (basically reflection) is possible on basic types, but Objects needs to be implemented with `_VAR_H_EXTERN_IMPLEMENTATIONS` using [data binding](https://github.com/ThakeeNathees/carbon/blob/master/core/native/native_bind.gen.h) (see: [carbon](https://github.com/ThakeeNathees/carbon))
```c++
var arr = Array();
arr.call_method("append", 42); // arr = [42]
var str = "var.h";
bool b = str.call_method("endswith", ".h");
```
```c++
// CARBON IMPLEMENTATION OF DATA BINDING (should be implemented externally var.h doesn't support out of the box)
class File : public Object {
	REGISTER_CLASS(File, Object) {
		BIND_STATIC_FUNC("File", &File::_File, PARAMS("self", "path", "mode"), DEFVALUES("", DEFAULT));

		BIND_ENUM_VALUE("READ",   READ);
		BIND_ENUM_VALUE("WRITE",  WRITE);
		BIND_ENUM_VALUE("APPEND", APPEND);
		BIND_ENUM_VALUE("BINARY", BINARY);
		BIND_ENUM_VALUE("EXTRA",  EXTRA);

		BIND_METHOD("open",  &File::open,  PARAMS("path", "mode"), DEFVALUES(DEFAULT));
		BIND_METHOD("read",  &File::read);
		BIND_METHOD("write", &File::write, PARAMS("what"));
		BIND_METHOD("close", &File::close);
	}
	...
};
```
