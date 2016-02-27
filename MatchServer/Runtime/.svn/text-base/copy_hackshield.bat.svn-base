@echo off

set TARGET=.\
if "%1" == "" goto :no_target
set TARGET=%1
:no_target

if exist %TARGET%\HackShield.crc del %TARGET%\HackShield.crc
if exist %TARGET%\AntiCpSvr.dll del %TARGET%\AntiCpSvr.dll

xcopy ..\HShield\HackShield.crc %TARGET% /r /y
xcopy ..\HShield\AntiCpSvr.dll %TARGET% /r /y
