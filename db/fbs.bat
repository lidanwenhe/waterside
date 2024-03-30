@echo off
set path=%path%;../bin/Release
flatc -c -o ../code/waterside/stmtc StmtConfig.fbs
@echo on
pause