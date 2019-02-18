# Compiles D3D shaders to headers - only needed if you change the hlsl.
# The headers are checked in....
# Need the path to fxc
@echo off
set PATH=%PATH%;"c:/program files (x86)/Windows Kits/8.1/bin/x86"
@echo on
fxc.exe /Fh D3D11PixelShader.h /T ps_4_0_level_9_3 /E D3D11PixelShader_Main D3D11PixelShader.hlsl
fxc.exe /Fh D3D11PixelShaderAA.h /T ps_4_0_level_9_3 /E D3D11PixelShaderAA_Main D3D11PixelShaderAA.hlsl
fxc.exe /Fh D3D11VertexShader.h /T vs_4_0_level_9_3 /E D3D11VertexShader_Main D3D11VertexShader.hlsl