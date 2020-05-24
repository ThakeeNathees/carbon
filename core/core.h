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

#ifndef CORE_H
#define CORE_H

#include "var.h/_var.h"

#ifndef STR
	#define STR(m_) #m_
#endif
#ifndef STRINGIFY
	#define STRINGIFY(m_) STR(m_)
#endif
#define NOEFFECT(m_) m_

#define STRCAT2(m_1, m_2) m_1##m_2
#define STRCAT3(m_1, m_2, m_3) m_1##m_2##m_3
#define STRCAT4(m_1, m_2, m_3, m_4) m_1##m_2##m_3##m_4

// for windows dll define CARBON_DLL, CARBON_DLL_EXPORT
#if defined(CARBON_DLL)
	#if defined(CARBON_DLL_EXPORT)
		#define CARBON_API __declspec(dllexport)
	#else
		#define CARBON_API __declspec(dllimport)
	#endif
#else
	#define CARBON_API
#endif



#endif // CORE_H