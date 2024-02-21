@echo off
set path=%path%;../../../tools
flatc -c -o network -I . rpc_protocol.fbs
@echo on
pause