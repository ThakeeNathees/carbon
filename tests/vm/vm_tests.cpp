
#include "../carbon_tests.h"

TEST_CASE("[vm_tests]:vm_tests") {
	ptr<Tokenizer> tokenizer = newptr<Tokenizer>();
	ptr<Parser> parser = newptr<Parser>();
	ptr<Analyzer> analyzer = newptr<Analyzer>();
	ptr<CodeGen> codegen = newptr<CodeGen>();
	ptr<Bytecode> bytecode;
	stdvec<String> argv;

	CHECK_NOTHROW__VM(R"(
	class A { var x = "testing"; }
	func main() {
		var x = A().x;

		// TODO: implement assert()
		// assert(x == "testing");
	}
)");
	
}