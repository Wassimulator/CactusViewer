cd ..
pushd bin
cl /c ..\src\imgui_all.cpp /Od /Zi /I ..\include /I ..\include\SDL
cd ..
cd src
