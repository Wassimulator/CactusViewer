![logo-export](https://user-images.githubusercontent.com/75145262/200181192-35c8ea4d-e864-4ac7-89d0-8deda4901699.png)
# Cactus Image Viewer
#### by Wassim Alhajomar (@wassimulator)
A small handmade image viewer written in C/C++.
#### Used libraries:
[SDL2](https://www.libsdl.org/) for Window and event management, [ImGui](https://github.com/ocornut/imgui) for the UI, [stb_image](https://github.com/nothings/stb) for decoding, [cute_files.h](https://github.com/RandyGaul/cute_headers/blob/master/cute_files.h) to manage directories on different platforms, and a custom OpenGL renderer.     
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

## Usage:
- Opening a file:
  - Drag and drop image file into the viewer, if the file is supported it should load the file and read the folder contents of the file's directory.
  - CactusViewer also supports "Open With..." by choosing it as the program to open an image with.
  - You can also use the command line to open a file by typing: `> CactusViewer.exe "path_to_file"` (make sure you write the full path of the file!).
- Move the mouse to the buttom of the screen to view controls, click on "config" to see controls and change settings. 

## Building:
### Windows:
using a MSVC developer terminal:
- run `bgui.bat` to compile ImGui to `imgui_all.obj`
- run `b.bat` to build the project, output in `\bin`

## Remarks:
- This project was written with portability in mind and uses no platform dependant libraries to ease said portability. I might port it to Linux or MacOS at some point if I find the time but it should be simple. 
- Make sure you compile `imgui_all.cpp` to `imgui_all.obj` and place that in `\bin` first.
- When compiling the main viewer make sure you link to `SDL2.lib`, `SDL2main.lib`, and `imgui_all.obj`.
- SDL2's DLL file or its platform specific equivalent is needed in the binary folder, you can get them here: https://github.com/libsdl-org/SDL/releases/tag/release-2.24.2
- For the sake of everyone's sanity, this repo has prebuilt binaries of all the above for Windows x64, I will provide other builds if I make any.

**Limitation:** *CactusViewer decodes image files into memory and uploads them to the GPU using a single texture, a limitation here is posed when of the image's dimensions is larger than the maximum texture size of the respective GPU. 
According to https://feedback.wildfiregames.com/report/opengl/feature/GL_MAX_TEXTURE_SIZE: 44% of users have GPUs that support up to **16384 x 16384** textures and 46% of users have GPUs that support up to **8192 x 8192**, the last 10% have **4096 x 4069** limits or less. These ranges cover regular image usage, including large files of up to 60 or 268 megapixel images respectively. So this likely won't be a problem to most use cases.*


## Possible imporovements:
Possible things to expand on, since I don't have that much time to spend on this project for now:
- Unlock the supported image resolutions sizes beyong maximum GPU texture sizes, to support images larger than 60MP or 268MP respectively.
- Decode images at lower resolutions at first and only decode full image upon zooming, to improve display speed of large files.
- Stream decoding GIF files to eliminate any load times (though it is already miniscule).
- Support for HEIF, TIFF, and some RAW formats, some of those require special codec.
- Packing all dlls into one binary loader to make the viewer into one standalone file. A save file is still required on the side.
- Rectangle selection and viewing of x,y,w,h values.
- Crop and rotate image, stb_image_write.h is already included and should be able to save the files, only the editing algorithms and UI need to be written.
- Providing an optional software renderer backend.
