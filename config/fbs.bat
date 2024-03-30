@echo off
set path=%path%;../bin/Release
flatc -c -o ../code/waterside/NameRegServer CommonConfig.fbs
flatc -c -o ../code/waterside/NameRegServer NameRegConfig.fbs
flatc -c -o ../code/waterside/LoginServer CommonConfig.fbs
flatc -c -o ../code/waterside/LoginServer LoginConfig.fbs
flatc -c -o ../code/waterside/GameServer CommonConfig.fbs
flatc -c -o ../code/waterside/GameServer GameConfig.fbs
flatc -c -o ../code/waterside/GateServer CommonConfig.fbs
flatc -c -o ../code/waterside/GateServer GateConfig.fbs
@echo on
pause