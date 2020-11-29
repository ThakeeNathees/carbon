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

#include "error.h"
#include "logger.h"

namespace carbon {

static const char* _error_names[Error::_ERROR_MAX_] = {
	"OK",
	"BUG",
	"NULL_POINTER",
	"OPERATOR_NOT_SUPPORTED",
	"NOT_IMPLEMENTED",
	"ZERO_DIVISION",
	"TYPE_ERROR",
	"ATTRIBUTE_ERROR",
	"INVALID_ARG_COUNT",
	"INVALID_INDEX",
	"IO_ERROR",
	"SYNTAX_ERROR",
	"ASSERTION",
	"UNEXPECTED_EOF",
	"NAME_ERROR",
	"VARIABLE_SHADOWING",
	"MISSED_ENUM_IN_SWITCH",
	"NON_TERMINATING_LOOP",
	"UNREACHABLE_CODE",
	"STAND_ALONE_EXPRESSION",
	//_ERROR_MAX_
};
MISSED_ENUM_CHECK(Error::Type::_ERROR_MAX_, 20);

String Throwable::get_err_name(Throwable::Type p_type) {
	THROW_INVALID_INDEX(_ERROR_MAX_, (int)p_type);
	return _error_names[p_type];
}

DBGSourceInfo::DBGSourceInfo() {}
DBGSourceInfo::DBGSourceInfo(const String& p_file, uint32_t p_line, const String& p_func) : func(p_func), file(p_file), line(p_line) {}
DBGSourceInfo::DBGSourceInfo(const String& p_file, const String& p_line_str, Vect2i& p_pos, uint32_t p_width, const String& p_func)
	: func(p_func), file(p_file), pos(p_pos), line((uint32_t)p_pos.x), width(p_width) {

	const char* source = p_line_str.c_str();
	uint64_t cur_line = 1;
	std::stringstream ss_line;
	
	while (char c = *source) {
		if (c == '\n') {
			if (cur_line >= line + 2) break;
			cur_line++;
		} else if (cur_line == line - 1) {
			line_before += c;
		} else if (cur_line == line) {
			line_str += c;
		} else if (cur_line == line + 1) {
			line_after += c;
		}
		source++;
	}
}

Throwable::Throwable(Type p_type, const String& p_what, const DBGSourceInfo& p_source_info)
	: _type(p_type), _what(p_what), source_info(p_source_info) {
}
void Throwable::set_source_info(const DBGSourceInfo& p_source_info) { source_info = p_source_info; }

Error::Error(Type p_type, const String& p_what, const DBGSourceInfo& p_dbg_info)
	: Throwable(p_type, p_what, p_dbg_info) {}

CompileTimeError::CompileTimeError(Type p_type, const String& p_what, const DBGSourceInfo& p_cb_dbg, const DBGSourceInfo& p_dbg_info)
	: _cb_dbg_info(p_cb_dbg), Throwable(p_type, p_what, p_dbg_info) {}

Warning::Warning(Type p_type, const String& p_what, const DBGSourceInfo& p_cb_dbg, const DBGSourceInfo& p_dbg_info)
	: _cb_dbg_info(p_cb_dbg), Throwable(p_type, p_what, p_dbg_info) {}



// TODO: log levels may prevent from logging. just log independently of the log level.
void Error::console_log() const {
	Logger::logf_error("Error(%s) : %s\n", Error::get_err_name(get_type()).c_str(), what());
	Logger::log(String::format(  "    source : %s (%s:%i)\n", source_info.func.c_str(), source_info.file.c_str(), source_info.line).c_str(),
		Logger::ERROR, Logger::Color::L_SKYBLUE
	);
}

void CompileTimeError::console_log() const {
	Logger::logf_error("Error(%s) : %s\n",      Error::get_err_name(get_type()).c_str(), what());
	Logger::log( String::format("    source : %s (%s:%i)\n", source_info.func.c_str(), source_info.file.c_str(), source_info.line).c_str(),
		Logger::ERROR, Logger::Color::L_SKYBLUE);
	Logger::log(String::format("    at     : (%s:%i)\n", _cb_dbg_info.file.c_str(), _cb_dbg_info.line).c_str(),
		Logger::ERROR, Logger::Color::L_WHITE);

	if (_cb_dbg_info.line - 1 >= 1) // first line may not be available to log
	Logger::logf_info("%3i | %s\n",           _cb_dbg_info.line - 1,  _cb_dbg_info.line_before.c_str());
	Logger::logf_info("%3i | %s\n    | %s\n", _cb_dbg_info.line, _cb_dbg_info.line_str.c_str(), _cb_dbg_info.get_pos_str().c_str());
	Logger::logf_info("%3i | %s\n",           _cb_dbg_info.line + 1,  _cb_dbg_info.line_after.c_str());
}

void Warning::console_log() const {
	Logger::logf_warning("Warning(%s) : %s\n", Error::get_err_name(get_type()).c_str(), what());
	Logger::log(String::format("    source : %s (%s:%i)\n", source_info.func.c_str(), source_info.file.c_str(), source_info.line).c_str(),
		Logger::ERROR, Logger::Color::L_SKYBLUE);
	Logger::log(String::format("    at     : (%s:%i)\n", _cb_dbg_info.file.c_str(), _cb_dbg_info.line).c_str(),
		Logger::ERROR, Logger::Color::L_WHITE);

	if (_cb_dbg_info.line - 1 >= 1) // first line may not be available to log
		Logger::logf_info("%3i | %s\n", _cb_dbg_info.line - 1, _cb_dbg_info.line_before.c_str());
	Logger::logf_info("%3i | %s\n    | %s\n", _cb_dbg_info.line, _cb_dbg_info.line_str.c_str(), _cb_dbg_info.get_pos_str().c_str());
	Logger::logf_info("%3i | %s\n", _cb_dbg_info.line + 1, _cb_dbg_info.line_after.c_str());
}

//-----------------------------------------------

String DBGSourceInfo::get_pos_str() const {
	// var x = blabla;
	//         ^^^^^^

	std::stringstream ss_pos;
	size_t cur_col = 0;
	bool done = false;
	for (size_t i = 0; i < line_str.size(); i++) {
		cur_col++;
		if (cur_col == pos.y) {
			for (uint32_t i = 0; i < width; i++) {
				ss_pos << '^';
			}
			done = true;
			break;
		} else if (line_str[i] != '\t') {
			ss_pos << ' ';
		} else {
			ss_pos << '\t';
		}
	}
	if (!done) ss_pos << '^';
	return ss_pos.str();
}



}