
#include "../carbon_tests.h"


TEST_CASE("[analyzer_tests]:analyze_files") {
	ptr<Tokenizer> tokenizer = newptr<Tokenizer>();
	ptr<Parser> parser = newptr<Parser>();
	Analyzer analyzer;
	//String path, source;

	Array files = Array(
		"tests/test_files/z_function.cb",
		"tests/test_files/gcd_subset.cb",
		"tests/test_files/newman_conway.cb",
		"" // end with comma above
	);

	for (int i = 0; i < (int)files.size() - 1; i++) {
		try {
			tokenizer->tokenize(newptr<File>(files[i], File::READ));
			parser->parse(tokenizer);
			analyzer.analyze(parser);
		} catch (Throwable& err) {
			CHECK_MESSAGE(false, err.what());
		}
	}

}