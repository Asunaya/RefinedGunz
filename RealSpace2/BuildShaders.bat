SET fxc="C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\Utilities\bin\x64\fxc.exe"
SET options=/O3
%fxc% /Tvs_3_0 /FhInclude/ShadowVS.h -VnShadowVSData Source/ShadowVS.hlsl %options%
%fxc% /Tps_3_0 /FhInclude/ShadowPS.h -VnShadowPSData Source/ShadowPS.hlsl %options%
%fxc% /Tvs_3_0 /FhInclude/MergeShadowMapsVS.h -VnMergeShadowMapsVSData Source/MergeShadowMapsVS.hlsl %options%
%fxc% /Tps_3_0 /FhInclude/MergeShadowMapsPS.h -VnMergeShadowMapsPSData Source/MergeShadowMapsPS.hlsl %options%
%fxc% /Tvs_1_1 /FhInclude/skin.h -VnskinData Source/skin.hlsl %options%