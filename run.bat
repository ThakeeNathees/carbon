echo off

set platform=Win64
set configuration=Debug
set carbon_src=tests/main.cb

:loop
"_bin/%platform%/%configuration%/carbon" "%carbon_src%"
pause
echo .
echo .
echo .
goto loop