
<p align="center" >
<img src="https://raw.githubusercontent.com/ThakeeNathees/carbon/master/extra/logo.png" width="300" alt="Carbon Programming Language" title="Carbon Programming Language">
</p>

Carbon is a simple embeddable, object-oriented, dynamic-typed, bytecode-interpreted, scripting language written in C++11 with smart pointers for safe memory deallocation. Carbon is syntactically similar to C/C++, it's analyzer and backend written using <a href="https://docs.godotengine.org/en/stable/getting_started/scripting/gdscript/gdscript_basics.html">GDScript</a>  as a reference (it performance is as much as GDScript).

## What Carbon looks like
```swift

class Vector {
	// non-static members
	var x = 0, y = 0;
	
	// static members
	static var ZERO = Vector(0, 0);
	static var UNIT = Vector(1, 1);

	// constructor
	func Vector (x = 0, y = 0) {
		this.x = x;
		this.y = y;
	}

	// built-in operator overriding, parameter as reference
	func __add(v&) {
		return Vector(x + v.x, y + v.y);
	}

	// built-in String conversion overriding
	func to_string() {
		return "Vector(%s, %s)" % [x, y];
	}
}

func main() {
	// construct a new vector object
	var v1 = Vector(1,2);
	
	// construct a new vector object
	var v2 = Vector(3,4);
	
	// call '__add' function in the vector object
	var v3 = v1 + v2;
	
	// prints "Vector(4, 6)"
	println(v3);
}
```

## Features
* Minimal setup
* Embeddable
*  Dynamic typing
* Garbage collection
* Object-oriented
* Reference, default argument
* Operator overriding
* First class types, functions
* Callables and Iterables

## Building from source
For a non-development install/embedding with a single header use <a href="https://raw.githubusercontent.com/ThakeeNathees/carbonUI/master/include/carbon.h">carbon.h</a> header, you only need a C++11 compiler. For more details. read usage in `carbon.h`.

### Requirenment
* C++11 compiler (Compiler that supports C++11 are GCC version 4.6+ and <a href="https://visualstudio.microsoft.com/vs/community/">Microsoft Visual Studio 2017</a> or newer.)
* <a href="https://git-scm.com/downloads">git</a> (version control)
* <a href="https://www.scons.org/">scons 3.0</a> (python based build system)
* <a href="https://www.python.org/downloads/">python 3.6+</a> (3.6+ for f-string)

#### install scons
```
python -m pip install scons
```
In Linux if scons using python2 instead of 3  you'll have to edit `/usr/local/bin/scons` or `~/.local/bin/scons` to ensure that it points to `/usr/bin/env python3` and not `python`

#### building
```
git clone https://github.com/ThakeeNathees/carbon.git
cd carbon
scons
```
You can specify the number of jobs scons to use to speed up the building process using the `-j` flag (`-j6`, `-j8`). To generate Visual Studio project files add `vsproj=true` argument when building. If your build failed feel free to open <a href="https://github.com/ThakeeNathees/carbon/issues">an issue</a>. Once a successful compile the bytecode-interpreter and unit-test binaries are found in `bin/` directory of the carbon root directory.

