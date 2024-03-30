@echo off
set path=%path%;../../../bin/Release
flatc -c -o network --gen-mutable rpc_protocol.fbs
@echo on
pause