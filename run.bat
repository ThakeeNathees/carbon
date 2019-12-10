echo off

set platform=Win64
set configuration=Debug
set carbon_src=tests/main.cb

"%_bin/%platform%/%configuration%/carbon" "%carbon_src%"
pause