#include "tests/carbon_tests.h"

#define TEST_FILE_PATH     "tests/test_files/file_test1"
#define TEST_BIN_FILE_PATH "tests/test_files/bin_file_test1"

// POSITIVE TESTS
TEST_CASE("[native_classes]:file_tests+") {

	// native file test
	File file1;
	file1.open(TEST_FILE_PATH, File::WRITE | File::EXTRA);
	file1.write("FILE1\nLINE2");
	String read1 = file1.read();
	file1.close();
	CHECK(read1 == "FILE1\nLINE2");

	file1.open(TEST_BIN_FILE_PATH, File::WRITE | File::BINARY | File::EXTRA);
	ptr<Buffer> buff = newptr<Buffer>();
	buff->alloc(5);
	for (int i = 0; i < 5; i++) {
		(*buff)[i] = i;
	}
	file1.write(buff);
	buff = nullptr;
	buff = file1.read().operator ptr<Buffer>();
	for (int i = 0; i < 5; i++) {
		CHECK((*buff)[i] == i);
	}
	file1.close();

	// bind method test
	stdvec<var*> args;
	var file2 = NativeClasses::singleton()->construct("File", args);
	call_method(file2, "open", TEST_FILE_PATH, File::APPEND);
	call_method(file2, "write", "\nappended by FILE2.");
	call_method(file2, "close");
	
	call_method(file2, "open", TEST_FILE_PATH, File::READ);
	String read2 = call_method(file2, "read");
	call_method(file2, "close");
	CHECK(read2 == "FILE1\nLINE2\nappended by FILE2.");
}

// NEGATIVE TESTS
TEST_CASE("[native_classes]:file_tests-") {
	File file;
	CHECK_THROWS_ERR(Error::IO_ERROR, file.open("./blah blah", File::READ));
}
