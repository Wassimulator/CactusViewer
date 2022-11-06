![logo - Copy](https://user-images.githubusercontent.com/75145262/199360352-9d8ff4c9-9e87-4f97-ae20-68671e69f919.png)
# Cactus Image Viewer
#### by Wassimulator
A small handmade image viewer written in C/C++ using SDL2 for Window and event management, ImGui for the UI, decoding with stb_image, and a custom OpenGL renderer.     
Supported file extensions: **PNG, JPG, JPEG, BMP, GIF**

## Features:
- Scans folder of opened image and allows flipping through supported images within folder.
- Multithreaded file loading.
- image pan and zoom with fitting modes; to width or height.
- Pixel color inspector and a quick copy of HEX value to clipboad.
- Animated GIF support with play/pause and seek controls.
- Quick toggle between nearest-neighbor and linear filtering for pixelart images.
- Pixel grid.
- Customizable background color with support for checkerboard with custom colors.
- Drag and drop support to opening image files.
- Switches to turn individual RGB channels off and switch between premultiplied alpha and straight RGB.
- Keyboard controls to pan, zoom, flip through files and GIF controls.
- Customizable settings to save each image's zoom and pan locations within a session, or to reset image zoom and pan upon file change.

## Building:
### Windows:
using a MSVC developer terminal:
- run `bgui.bat` to compile ImGui to `imgui_all.obj`
- run `b.bat` to build the project, output in `\bin`

## Remarks:
- This project was written with portability in mind and uses no platform dependant libraries to ease said portability. I might port it to Linux or MacOS at some point if I find the time but it should be simple. 
- Make sure you compile `imgui_all.cpp` to `imgui_all.obj` and place that in `\bin` first.
- When compiling the main viewer make sure you link to `SDL2.lib SDL2main.lib imgui_all.obj`
- SDL2's DLL file or its platform specific equivalent is needed in the binary folder, you can get them here: https://github.com/libsdl-org/SDL/releases/tag/release-2.24.2
- For the sake of everyone's sanity, this repo has prebuilt binaries of all the above for Windows x64, I will provide other builds if I make any.

**Limitation:** *CactusViewer decodes image files into memory and uploads them to the GPU using a single texture, a limitation here is posed when of the image's dimensions is larger than the maximum texture size of the respective GPU. 
According to https://feedback.wildfiregames.com/report/opengl/feature/GL_MAX_TEXTURE_SIZE: 44% of users have GPUs that support up to **16384 x 16384** textures and 46% of users have GPUs that support up to **8192 x 8192**, the last 10% have **4096 x 4069** limits or less. These ranges cover regular image usage, including large files of up to 60 megapixel images. So this likely won't be a problem to most.*
