![logo-export](https://user-images.githubusercontent.com/75145262/200181192-35c8ea4d-e864-4ac7-89d0-8deda4901699.png)
# Cactus Image Viewer
#### by Wassim Alhajomar (@wassimulator)
A single-file handmade image viewer for Windows, written in C/C++.       
### VIDEO: Watch it in action: 
Featured on the Handmade Network's Wheel Reinvention Jam.

[![Video](https://img.youtube.com/vi/FofjjYWCeX8/maxresdefault.jpg)](https://youtu.be/FofjjYWCeX8)
-----
![F7HthCRWYAAnZQH](https://github.com/Wassimulator/CactusViewer/assets/75145262/a67dd51c-a61c-4c74-9bbf-0fd07c33594f)
-----

[Consider donating if you like this project](https://www.paypal.com/donate/?hosted_button_id=NYKGNB8VZG84Y)
#### Used libraries:
Homemade immediate mode UI, a custom D3D11 renderer, [Windows Imaging Component](https://learn.microsoft.com/en-us/windows/win32/wic/-wic-about-windows-imaging-codec), [stb_image](https://github.com/nothings/stb) and [libwebp](https://chromium.googlesource.com/webm/libwebp) for decoding, [cute_files.h](https://github.com/RandyGaul/cute_headers/blob/master/cute_files.h) to manage directories on different platforms, [FreeType](https://freetype.org/) for font loading, and [easyexif](https://github.com/mayanklahiri/easyexif) for fetching image metadata.             
#### Supported codecs: 
**BMP, GIF, ICO, JPEG, JPEG XR, PNG, TIFF, DDS, WEBP**.         
It also supports installed codecs for WIC, check Microsoft Store to get codecs like HEIF, RAW, or AVIF. Here:     
[AV1 Video Extension](https://www.microsoft.com/store/productid/9MVZQVXJBQ9V?ocid=pdpshare)      
[HEVC Video Extensions](https://www.microsoft.com/store/productid/9NMZLZ57R3T7?ocid=pdpshare)      
[HEIF Image Extensions](https://www.microsoft.com/store/productid/9PMMSR1CGPWG?ocid=pdpshare)       
[Raw Image Extension](https://www.microsoft.com/store/productid/9NCTDW2W1BH8?ocid=pdpshare)      

## Features:
- Standalone executable.
- Crisp font rendering with subpixel antialiasing.
- Scans folder of opened image and allows flipping through supported images within folder, and rotating the view without changing the source files.
- Supports loading image files in the sorted order of the folder, can be optionally disabled to default to alphabetic.
- Support for unicode file names.
- Multithreaded file loading.
- Fullscreen mode with F11.
- Image pan and zoom with fitting modes; to width or height.
- RGB Histograms.
- Adjustable hue, saturation, gamma, and brightness values.
- Image cropping
- Gaussian blur filters
- Ability to save images with current adjustments
- Support for reading and displaying EXIF metadata off of JPEG images and rotation info.
- Pixel color inspector and a quick copy of HEX value to clipboard.
- Animated GIF and WebP support with play/pause and seek controls.
- Quick toggle between nearest-neighbor and linear filtering for pixelart images.
- Pixel grid.
- 3 UI themes.
- Customizable background color with support for checkerboard with custom colors.
- Drag and drop support for opening image files and folders.
- Switches to turn individual RGB channels off and switch between premultiplied alpha and straight RGB.
- Keyboard controls to pan, zoom, and flip through files and GIF controls.
- Customizable settings to save each image's zoom and pan locations within a session, or to reset image zoom and pan upon file change.

## Usage:
- Opening a file:
  - Drag and drop image file into the viewer, if the file is supported it should load the file and read the folder contents of the file's directory.
  - CactusViewer also supports "Open With..." by choosing it as the program to open an image with.
  - You can also use the command line to open a file by typing: `> CactusViewer.exe "path_to_file"` (make sure you write the full path of the file!).
- Move the mouse to the bottom of the screen to view controls, click on "config" to see controls and change settings. 

## Building:
### Windows:
All dependencies are within the repo for your, my, and everyone's convenience. 
Using an MSVC 64 bit developer terminal:
- run `b.bat` to build the project in debug mode, output in `\bin`
  (the debug version expects the font file in src, so terminal calls need to have bin as the working directory to run
   also make sure you have D3D11 debug layers from [here](https://learn.microsoft.com/en-us/windows/uwp/gaming/use-the-directx-runtime-and-visual-studio-graphics-diagnostic-features) installed or remove that flag in `init_d3d11()` when building in debug mode).
- run `b.bat release` to build the project in release mode, output in `\bin`

## Remarks:
- Update 2.0 ditches ImGui and OpenGL, the executable runs now on D3D11 with a handmade immediate mode UI library. If you have Windows, it should work, so please report any bugs!
- For the sake of everyone's sanity, this repo has prebuilt binaries of all the above for Windows x64, I will provide other builds if I make any.

## Possible improvements:
Possible things to expand on:
- Unlock the supported image resolutions sizes beyond maximum GPU texture sizes, to support images larger than 60MP or 268MP respectively.
- Decode images at lower resolutions at first and only decode full image upon zooming, to improve display speed of large files.
- Rectangle selection and viewing of x,y,w,h values.
- Crop and rotate image, stb_image_write.h is already included and should be able to save the files, only the editing algorithms and UI need to be written.
- Providing an optional software renderer backend.
