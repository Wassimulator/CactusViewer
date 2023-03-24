cd ..
pushd bin
cl /c ..\src\imgui_all.cpp /O2 /Zi /I ..\include /I ..\include\SDL
cd ..
cd src
