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

#include "logger.h"

namespace carbon {

Logger::LogLevel Logger::level = Logger::LogLevel::VERBOSE;
Logger::LogLevel Logger::last_level = Logger::LogLevel::VERBOSE;


void Logger::initialize() {}
void Logger::cleanup() {}

void Logger::set_level(LogLevel p_level) {
	last_level = level;
	level = p_level;
}

void Logger::reset_level() {
	level = last_level;
}

Logger::LogLevel Logger::get_level() {
	return level;
}

bool Logger::is_level(LogLevel p_level) {
	return (int)p_level >= (int)level;
}

void Logger::log(const char* p_msg, Console::Color p_fg, Console::Color p_bg) {
	_log(p_msg, false, p_fg, p_bg);
}

void Logger::log(const char* p_msg, LogLevel p_level, Console::Color p_fg, Console::Color p_bg) {
	if (!is_level(p_level)) return; log(p_msg, p_fg, p_bg);
}

void Logger::log_verbose(const char* p_msg) { if (!is_level(LogLevel::VERBOSE)) return;  _log(p_msg, false);                          }
void Logger::log_info(const char* p_msg)    { if (!is_level(LogLevel::INFO))    return;  _log(p_msg, false, Console::Color::L_WHITE); }
void Logger::log_success(const char* p_msg) { if (!is_level(LogLevel::SUCCESS)) return;  _log(p_msg, false, Console::Color::L_GREEN); }
void Logger::log_warning(const char* p_msg) { if (!is_level(LogLevel::WARNING)) return;  _log(p_msg, true, Console::Color::D_YELLOW); }
void Logger::log_error(const char* p_msg)   { if (!is_level(LogLevel::ERROR))   return;  _log(p_msg, true, Console::Color::D_RED);    }

void Logger::logf_verbose(const char* p_fmt, ...) {
	if (!is_level(LogLevel::VERBOSE)) return;
	va_list args;
	va_start(args, p_fmt);
	_logf(p_fmt, args, false);
	va_end(args);
}

void Logger::logf_info(const char* p_fmt, ...) {
	if (!is_level(LogLevel::INFO)) return;
	va_list args;
	va_start(args, p_fmt);
	_logf(p_fmt, args, false, Console::Color::L_WHITE);
	va_end(args);
}

void Logger::logf_success(const char* p_fmt, ...) {
	if (!is_level(LogLevel::SUCCESS)) return;
	va_list args;
	va_start(args, p_fmt);
	_logf(p_fmt, args, false, Console::Color::L_GREEN);
	va_end(args);
}

void Logger::logf_warning(const char* p_fmt, ...) {
	if (!is_level(LogLevel::WARNING)) return;
	va_list args;
	va_start(args, p_fmt);
	_logf(p_fmt, args, true, Console::Color::D_YELLOW);
	va_end(args);
}

void Logger::logf_error(const char* p_fmt, ...) {
	if (!is_level(LogLevel::ERROR)) return;
	va_list args;
	va_start(args, p_fmt);
	_logf(p_fmt, args, true, Console::Color::D_RED);
	va_end(args);
}

void Logger::_log(const char* p_message, bool p_stderr, Console::Color p_forground, Console::Color p_background) {
	Console::log(p_message, p_stderr, p_forground, p_background);
}

void Logger::_logf(const char* p_fmt, va_list p_args, bool p_err, Console::Color p_forground, Console::Color p_background) {
	Console::logf(p_fmt, p_args, p_err, p_forground, p_background);
}

}