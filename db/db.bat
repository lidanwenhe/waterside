@echo off
set path=%path%;../bin/Debug
stmtc db_login.json ../code/waterside/LoginServer/mysql/
@echo on
pause