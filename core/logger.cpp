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

// singleton declaration is in <platform>/logger_<platform>.cpp
Logger::LogLevel Logger::level = Logger::LogLevel::VERBOSE;
Logger::LogLevel Logger::last_level = Logger::LogLevel::VERBOSE;

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

void Logger::log(const char* p_msg, LogLevel p_level, Color p_fg, Color p_bg) {
	if (!is_level(p_level)) return; singleton->log_impl(p_msg, p_fg, p_bg);
}

void Logger::log(const char* p_msg, Color p_fg, Color p_bg) {
	singleton->log_impl(p_msg, p_fg, p_bg); // JUST_LOG
}

void Logger::set_cursor(int p_line, int p_column) { singleton->set_cursor_impl(p_line, p_column); }

void Logger::log_verbose(const char* p_msg) { if (!is_level(LogLevel::VERBOSE)) return; singleton->log_verbose_impl(p_msg); }
void Logger::log_info(const char* p_msg)    { if (!is_level(LogLevel::INFO))    return; singleton->log_info_impl(p_msg); }
void Logger::log_success(const char* p_msg) { if (!is_level(LogLevel::SUCCESS)) return; singleton->log_success_impl(p_msg); }
void Logger::log_warning(const char* p_msg) { if (!is_level(LogLevel::WARNING)) return; singleton->log_warning_impl(p_msg); }
void Logger::log_error(const char* p_msg)   { if (!is_level(LogLevel::ERROR))   return; singleton->log_error_impl(p_msg); }

void Logger::logf_verbose(const char* p_fmt, ...) {
	if (!is_level(LogLevel::VERBOSE)) return;
	va_list args;
	va_start(args, p_fmt);
	singleton->logf_verbose_impl(p_fmt, args);
	va_end(args);
}

void Logger::logf_info(const char* p_fmt, ...) {
	if (!is_level(LogLevel::INFO)) return;
	va_list args;
	va_start(args, p_fmt);
	singleton->logf_info_impl(p_fmt, args);
	va_end(args);
}

void Logger::logf_success(const char* p_fmt, ...) {
	if (!is_level(LogLevel::SUCCESS)) return;
	va_list args;
	va_start(args, p_fmt);
	singleton->logf_success_impl(p_fmt, args);
	va_end(args);
}

void Logger::logf_warning(const char* p_fmt, ...) {
	if (!is_level(LogLevel::WARNING)) return;
	va_list args;
	va_start(args, p_fmt);
	singleton->logf_warning_impl(p_fmt, args);
	va_end(args);
}

void Logger::logf_error(const char* p_fmt, ...) {
	if (!is_level(LogLevel::ERROR)) return;
	va_list args;
	va_start(args, p_fmt);
	singleton->logf_error_impl(p_fmt, args);
	va_end(args);
}

}