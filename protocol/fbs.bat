@echo off
set path=%path%;../bin/Release
for /r %%s in (*.fbs) do (
	flatc -c -o ../code/waterside/fbs --gen-object-api %%s
)
@echo on
pause