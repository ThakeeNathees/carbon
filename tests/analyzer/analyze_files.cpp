
#include "tests/carbon_tests.h"


TEST_CASE("[analyzer_tests]:analyze_files") {
	ptr<Parser> parser = newptr<Parser>();
	Analyzer analyzer;
	//String path, source;

	Array files = Array(
		"tests/analyzer/test_files/z_function.cb",
		"tests/analyzer/test_files/gcd_subset.cb",
		"tests/analyzer/test_files/newman_conway.cb"
	);

	try {
		for (int i = 0; i < files.size(); i++) {
			parser->parse(File(files[i], File::READ).read_text(), files[i]);
			analyzer.analyze(parser);
		}

	} catch (Error& err) {
		// Logger::logf_error("ERROR(%s): %s ", Error::get_err_name(err.get_type()).c_str(), err.get_msg().c_str());
		// Logger::logf_info("at: (%s:%i)\n", err.get_file().c_str(), err.get_pos().x);
		// Logger::log(
		// 	String::format("    at: %s (%s:%i)\n", err.get_dbg_func().c_str(), err.get_dbg_file().c_str(), err.get_dbg_line()).c_str(),
		// 	Logger::ERROR, Logger::Color::L_SKYBLUE
		// );
		// Logger::logf_info("%s\n%s\n", err.get_line().c_str(), err.get_line_pos().c_str());

		throw err; // this will print formated (no-color) in the console
	}

}