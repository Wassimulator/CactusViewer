@echo off
set Mode=%1
if [%1]==[] (
    set Mode=debug
)
RC /nologo src/res.rc
pushd bin

set Libs= /LIBPATH:..\lib shell32.lib d3d11.lib d3dcompiler.lib shell32.lib Ole32.lib User32.lib freetype.lib libwebp.lib libwebpdecoder.lib windowscodecs.lib dwmapi.lib /LIBPATH:..\src res.res
set CompileFlags=/nologo /utf-8 /I ..\include /I ..\include\ui /I ..\include\webp /I ..\include\easyexif
set LinkFlags=/OUT:CactusViewer.exe /IGNORE:4099 /debug /subsystem:Windows %Libs%

if %Mode%==release (
    set CompileFlags=/EHsc /O2 /D DEBUG_MODE=0 %CompileFlags%
) else if %Mode%==debug (
    set CompileFlags=/EHsc /Zi /Od /D DEBUG_MODE=1 %CompileFlags%
    set LinkFlags=/debug %LinkFlags%
) else (
    echo Unknown build mode %Mode%.
	popd
    exit /b 2
)

cl ..\src\main.cpp %CompileFlags% /link %LinkFlags%
popd
