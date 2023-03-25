@echo off
RC /nologo res.rc
cd ..
pushd bin

set OUT=           /OUT:CactusViewer.exe 
set Libs=          /LIBPATH:..\lib shell32.lib imgui_all.obj gdi32.lib opengl32.lib glu32.lib ole32.lib /LIBPATH:..\src res.res


cl /nologo ..\src\main.cpp  /EHsc /Zi /Od /utf-8 /D DEBUG_MODE=1 /I ..\include  /link %OUT% /debug /subsystem:Windows %Libs%
cd ..
cd src
