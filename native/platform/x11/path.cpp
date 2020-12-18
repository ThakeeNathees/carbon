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


#include "path.h"

// TODO: move these headers to platform abstraction
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

namespace carbon {

String Path::absolute() {
	char* path = realpath(_path.c_str(), NULL);
    String ret;
    if (path != NULL) {
        ret = path;
        free(path);
    }
    return ret;
}

bool Path::exists() {
	struct stat _stat;
	return stat(_path.c_str(), &_stat) == 0;
}

bool Path::isdir() {
	struct stat _stat;
	if (stat(_path.c_str(), &_stat) == 0 && (_stat.st_mode & S_IFMT == S_IFDIR))
        return true;
    return false;
}

}
