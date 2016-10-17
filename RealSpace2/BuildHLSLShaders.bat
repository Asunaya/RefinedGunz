@echo off
SET fxc="C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\Utilities\bin\x64\fxc.exe"
SET options=/O3

call:CompileVSPS Shadow
call:CompileVSPS MergeShadowMaps
call:CompileShader skin /Tvs_1_1
goto:eof

:CompileVSPS
call:CompileVS %~1VS
call:CompilePS %~1PS
goto:eof

:CompileVS
call:CompileShader %~1 /Tvs_3_0
goto:eof

:CompilePS
call:CompileShader %~1 /Tps_3_0
goto:eof

:CompileShader
%fxc% %~2 /FhInclude/%~1.h -Vn%~1Data Source/%~1.hlsl %options%
goto:eof