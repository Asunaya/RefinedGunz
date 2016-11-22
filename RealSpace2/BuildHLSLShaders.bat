@echo off
SET fxc="C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\Utilities\bin\x64\fxc.exe"
SET options=/O3

call:CompileVSPS Shadow
call:CompileVSPS MergeShadowMaps
call:CompileSingle Deferred /Tvs_2_0 /Tps_2_0
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

:CompileSingle
call:CompileShaderOutput %~1 %~1VS %~2 /Evs_main
call:CompileShaderOutput %~1 %~1PS %~3 /Eps_main
goto:eof

:CompileShader
call:CompileShaderOutput %~1 %~1 %~2 %~3 %~4
goto:eof

:CompileShaderOutput
%fxc% %~3 %~4 %~5 /FhInclude/%~2.h /Vn%~2Data Source/%~1.hlsl %options%
goto:eof