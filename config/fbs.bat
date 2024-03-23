@echo off
set path=%path%;../tools
flatc -c -o ../code/waterside/NameRegServer CommonConfig.fbs
flatc -c -o ../code/waterside/NameRegServer NameRegConfig.fbs
flatc -c -o ../code/waterside/LoginServer CommonConfig.fbs
flatc -c -o ../code/waterside/LoginServer LoginConfig.fbs
flatc -c -o ../code/waterside/GameServer CommonConfig.fbs
flatc -c -o ../code/waterside/GameServer -I . GameConfig.fbs
@echo on
pause