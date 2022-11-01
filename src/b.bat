@echo off
rc /nologo /fo r.res res.rc
cd ..
pushd bin

set OUT=           /OUT:CactusViewer.exe 
set Libs=          /LIBPATH:..\lib SDL2.lib SDL2main.lib shell32.lib imgui_all.obj  /LIBPATH:..\src r.res


cl /nologo ..\src\main.cpp /EHsc /Zi /O2 /D DEBUG_MODE=1 /I ..\include  /link %OUT% /debug /subsystem:Windows %Libs%
cd ..
cd src
