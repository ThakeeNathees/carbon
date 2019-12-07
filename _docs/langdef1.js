
// types (no unsigned types) 'c', "str"
bool ( true, false ), char, short, int, long, float, double, void

// other types
list<type>, map<key, value>, string, // func<int,string><int>, functions are not type, no more array<dtype, size>
list<int> my_list = { 1, 2, 3, 4 };

// operators
+ - * / // %	<< >> |(or) & ^(xor), 
.(dot operator) []

// keywords
null, if, else, else if, while, or, and, not, 
break, continue, return, static (static for function and field), import, 

// number litterls
0x12, 12, 
'12s'// short
'12l' //long 
'12d', '12.d', '.12d', '12.12d' // double, f- float

// build in
print, rand, min, max,
/***********************************************/

//structures

if (bool_cond) {}
if (cond){} else {}
if (cond){} else if(cond){} else if(cond){}
if (cond){} else if(cond){} else if(cond){} else{}

for ( expr; expr; expr ){ expr; ... }
for ( expr : idf_iter ) { expr; ... }

while (bool_cond) {}

class ClassName(parent) {
	// fields
	int id;
	static string name;

	//methods
	function getId() : int {  return id; }
	static function someStaticFunc() 

}

// TODO: DEFAULT arguments, keyword arguments
function function_name(type arg, type_arg) : return type; // function without a body
function name(){} // this means args and return type are void
