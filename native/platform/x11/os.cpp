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

#include "os.h"

// TODO: move this header to a platform abstraction layer
#include <unistd.h>

namespace carbon {

String OS::getcwd() {
    char buffer[VSNPRINTF_BUFF_SIZE];
    if (::getcwd(buffer, sizeof(buffer)) == NULL) {
        THROW_ERROR(Error::IO_ERROR, "getcwd() returned NULL.");
    }
    return buffer;
}

void OS::chdir(const String& p_path) {
	if (::chdir(p_path.c_str()) != 0){
        THROW_ERROR(Error::IO_ERROR, "chdir() failed.");
    }
}


//void Path::mkdir(const String& p_path, bool p_recursive) {
//	//if (p_recursive) PATH_TRY(fs::create_directories(p_path.operator std::string()));
//	//else PATH_TRY(fs::create_directory(p_path.operator std::string()));
//	THROW_BUG("Not imlpemented");
//}
//
//void Path::copy_file(const String& p_from, const String& p_to) {
//	//PATH_TRY(fs::copy_file(p_from.operator std::string(), p_to.operator std::string()));
//	THROW_BUG("Not imlpemented");
//}
//
//void Path::copy_tree(const String& p_from, const String& p_to) {
//	//PATH_TRY(fs::copy(p_from.operator std::string(), p_to.operator std::string(), fs::copy_options::recursive));
//	THROW_BUG("Not imlpemented");
//}
//
//void Path::remove(const String& p_path, bool p_recursive) {
//	//if (p_recursive) PATH_TRY(fs::remove_all(p_path.operator std::string()));
//	//else PATH_TRY(fs::remove(p_path.operator std::string()));
//	THROW_BUG("Not imlpemented");
//}
//
//void Path::rename(const String& p_path, const String& p_new) {
//	//PATH_TRY(fs::rename(p_path.operator std::string(), p_new.operator std::string()));
//	THROW_BUG("Not imlpemented");
//}
//
//Array Path::listdir(const String& p_path, bool p_recursive) {
//	THROW_BUG("Not imlpemented");
//	Array ret;
//	//try {
//	//	if (p_recursive) {
//	//		for (auto entry : fs::recursive_directory_iterator(p_path.operator std::string()))
//	//			ret.append(String(entry.path().string()));
//	//	} else {
//	//		for (auto entry : fs::directory_iterator(p_path.operator std::string()))
//	//			ret.append(String(entry.path().string()));
//	//	}
//	//} catch (std::exception& err) {
//	//	THROW_ERROR(Error::IO_ERROR, err.what());
//	//}
//	return ret;
//}



}