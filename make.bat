@echo off

rem ------------------------------------------------------------------------------
rem  MIT License
rem ------------------------------------------------------------------------------
rem  
rem  Copyright (c) 2020 Thakee Nathees
rem  
rem  Permission is hereby granted, free of charge, to any person obtaining a copy
rem  of this software and associated documentation files (the "Software"), to deal
rem  in the Software without restriction, including without limitation the rights
rem  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
rem  copies of the Software, and to permit persons to whom the Software is
rem  furnished to do so, subject to the following conditions:
rem  
rem  The above copyright notice and this permission notice shall be included in all
rem  copies or substantial portions of the Software.
rem  
rem  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
rem  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
rem  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
rem  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
rem  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
rem  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
rem  SOFTWARE.
rem ------------------------------------------------------------------------------

:: msvc x86_64
if "%MSVC_INITIALIZED%" neq "" goto :L_COMPILATION_START

:: Find vswhere
if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" (
	set VSWHERE_PATH="%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
) else ( if exist "%ProgramFiles%\Microsoft Visual Studio\Installer\vswhere.exe" (
	set VSWHERE_PATH="%ProgramFiles%\Microsoft Visual Studio\Installer\vswhere.exe"
) else (
	echo Error: can't find vswhere.exe required
	exit /B 1
))

:: Get the VC installation path
%VSWHERE_PATH% -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -latest -property installationPath > temp.txt
set /p VSWHERE_PATH= < temp.txt
del temp.txt
if not exist "%VSWHERE_PATH%" (
	echo Error: can't find VisualStudio installation directory
	exit /B 1
)
echo [make.bat] VS path found : %VSWHERE_PATH%

:: Initialize VC for X86_64
call "%VSWHERE_PATH%\VC\Auxiliary\Build\vcvars64.bat"

echo [make.bat] Initialized msvc x86_64
set MSVC_INITIALIZED=true

:L_COMPILATION_START

if not exist "bin" md "./bin"

set SOURCE_DIR="./src"
set ADDNL_INCLUDE=/Iinclude
set ADDNL_CPPFLAGS=/EHsc /MDd
set ADDNL_LIBS=psapi.lib dbghelp.lib User32.lib

set object_files=
:: loop all files in src matching *.cpp and compile
for /r %SOURCE_DIR% %%f in (*.c, *.cpp) do (
	for %%i in ("%%f") do (
		rem echo filedrive=%%~di
		rem echo filepath=%%~pi
		rem echo filename=%%~ni
		rem echo fileextension=%%~xi
		call set "object_files=%%object_files%% bin\%%~ni.obj"		
		cl /c %ADDNL_INCLUDE% %ADDNL_CPPFLAGS% %%f /Fobin/%%~ni.obj
	)
)

:: link all the object files
cl %ADDNL_CPPFLAGS% %object_files% %ADDNL_LIBS%  /Febin/carbon.exe

for %%o in (%object_files%) do (
	del "%%o"
)

:L_EXIT
