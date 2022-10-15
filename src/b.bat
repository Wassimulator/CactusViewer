@echo off
rc /nologo /fo r.res game.rc
cd ..
pushd bin

set OUT=           /OUT:AV_Racer_debug.exe 
set ENet_Libs= enet64.lib ws2_32.lib winmm.lib
set OpenAL_Libs= OpenAL32.lib alut.lib 
cl /nologo ..\src\main.cpp /EHsc /Zi /Od /D DEBUG_MODE=1 /I ..\include /link %OUT% /debug /subsystem:console /LIBPATH:..\lib SDL2.lib steam_api64.lib SDL2main.lib shell32.lib imgui_all.obj %OpenAL_Libs% /LIBPATH:..\src r.res
cd ..
cd src
