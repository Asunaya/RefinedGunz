@echo off
rem publish.bat [언어] [대상폴더]

set TARGETLANG=KOR
if "%1" == "" goto :no_target_lang
set TARGETLANG=%1
:no_target_lang

del vssver.scc /s /q /f

mkdir \\Databank\TeamWorks\Gunz\BuildServer\lastest
set TARGET=\\Databank\TeamWorks\Gunz\BuildServer\lastest\MatchAgent\


del %TARGET%*.* /s /q /f
mkdir %TARGET%

copy MatchAgent.exe %TARGET%
copy MatchAgent.pdb %TARGET%
copy dbghelp.dll %TARGET%
copy AgentConfig.xml %TARGET%_AgentConfig.xml

rem 익스플로러 창을 띄운다.
explorer \\databank\TeamWorks\Gunz\BuildServer\lastest\
