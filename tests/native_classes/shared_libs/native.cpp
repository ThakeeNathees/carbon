
#define VAR_IMPLEMENTATION
#include "var.h"
using namespace varh;

// debug compiler flag for MSVC : '/bigobj', '/std:c++17', '/W3', '/GR', '/EHsc', '/MDd', '/ZI'
// release flags: TODO:
// cl.exe /LD {COMPILE_FLAGS} native_lib.cpp

namespace pe {
	
class Aclass : public Object {
	REGISTER_CLASS(Aclass, Object) {
		BIND_METHOD("a_method",  &Aclass::a_method);
	}
	public:
	void a_method() {
		printf("a method called();\n");
	}
	~Aclass(){
		std::cout << "~Aclass()" << std::endl;
	}
};

} // namespace pe

#ifdef __cplusplus
extern "C" {
#endif

__declspec(dllexport)
void carbon_register_classes(NativeClasses* p_native) {
	NativeClasses::_set_singleton(p_native);
	NativeClasses::singleton()->register_class<pe::Aclass>();
	printf("register classes called\n");
}

__declspec(dllexport)
void carbon_unregister_classes(NativeClasses* p_native) {
	printf("unregister classes called\n");
}

#ifdef __cplusplus
}
#endif