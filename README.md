![logo-export](https://user-images.githubusercontent.com/75145262/200181192-35c8ea4d-e864-4ac7-89d0-8deda4901699.png)
# Cactus Image Viewer
#### by Wassim Alhajomar (@wassimulator)
A single-file handmade image viewer for Windows, written in C/C++.
### VIDEO: Watch it in action
Featured on the Handmade Network's Wheel Reinvention Jam.

[![Video](https://img.youtube.com/vi/FofjjYWCeX8/maxresdefault.jpg)](https://youtu.be/FofjjYWCeX8)
-----
![F7HthCRWYAAnZQH](https://github.com/Wassimulator/CactusViewer/assets/75145262/a67dd51c-a61c-4c74-9bbf-0fd07c33594f)
-----

[Consider donating if you like this project](https://www.paypal.com/donate/?hosted_button_id=NYKGNB8VZG84Y)
#### Used libraries
Homemade immediate mode UI, a custom D3D11 renderer, [Windows Imaging Component](https://learn.microsoft.com/en-us/windows/win32/wic/-wic-about-windows-imaging-codec), [stb_image](https://github.com/nothings/stb) and [libwebp](https://chromium.googlesource.com/webm/libwebp) for decoding, [cute_files.h](https://github.com/RandyGaul/cute_headers/blob/master/cute_files.h) to manage directories on different platforms, [FreeType](https://freetype.org/) for font loading, and [easyexif](https://github.com/mayanklahiri/easyexif) for fetching image metadata.
#### Supported codecs
**BMP, GIF, ICO, JPEG, JPEG XR, PNG, TIFF, DDS, WEBP**.
It also supports installed codecs for WIC, check Microsoft Store to get codecs like HEIF, RAW, or AVIF. Here:
[AV1 Video Extension](https://www.microsoft.com/store/productid/9MVZQVXJBQ9V?ocid=pdpshare)
[HEVC Video Extensions](https://www.microsoft.com/store/productid/9NMZLZ57R3T7?ocid=pdpshare)
[HEIF Image Extensions](https://www.microsoft.com/store/productid/9PMMSR1CGPWG?ocid=pdpshare)
[Raw Image Extension](https://www.microsoft.com/store/productid/9NCTDW2W1BH8?ocid=pdpshare)

## Features

### Core
- **Standalone executable**: No installation required. Just download and run, all dependencies are built-in.
- **Crisp font rendering**: Custom text rendering with subpixel antialiasing for sharp, easy-to-read UI text on any display.
- **Fullscreen mode**: Press F11 to toggle distraction-free fullscreen viewing.
- **Drag and drop**: Open images or entire folders by dropping them onto the window.
- **Unicode support**: Full support for international characters and symbols in file and folder names.

### Folder Navigation
- **Folder scanning**: Automatically scans the folder of any opened image and lets you flip through all supported images with arrow keys or on-screen controls.
- **Smart folder sorting**: Automatically detects and matches the sort order from Windows Explorer or [FilePilot](https://filepilot.tech/) file manager. Supports sorting by name (with natural sorting so "file2" comes before "file10"), date modified, file size, and file type—both ascending and descending. Can be disabled to default to alphabetical order.
- **Thumbnail preview bar**: Quick visual navigation through all images in the current folder. Click any thumbnail to jump directly to that image.
- **Multithreaded loading**: Images load in the background so you can keep browsing while large files decode.

### Viewing & Navigation
- **Pan and zoom**: Smooth image panning with mouse drag, and zoom with scroll wheel. Zoom centers on your cursor position for precise control.
- **Fitting modes**: Quickly fit images to window width, height, or view at 1:1 actual pixels. Cycle through modes or set a default in settings.
- **Rotation**: Rotate the view in 90° increments without modifying the source file. EXIF rotation data is automatically applied on load.
- **Pixel-perfect mode**: Toggle between smooth linear filtering and crisp nearest-neighbor sampling—perfect for pixel art, sprites, and retro graphics.
- **Pixel grid**: Overlay a grid at high zoom levels to see individual pixel boundaries clearly.

### Image Analysis
- **RGB Histograms**: Real-time histogram display showing the distribution of red, green, and blue color values across the image.
- **Pixel color inspector**: Hover over any pixel to see its exact RGB values and coordinates. Click to copy the HEX color code to clipboard instantly.
- **EXIF metadata**: View embedded camera and image data from JPEG files including camera model, exposure settings, GPS coordinates, and more.
- **Channel isolation**: Toggle individual RGB channels on/off to analyze color composition. Switch between premultiplied alpha and straight RGB blending modes.

### Image Editing
- **Paint/Draw mode**: Draw directly on images with a circular brush. Adjust brush size with scroll wheel, pick any color, and toggle anti-aliasing for smooth or hard edges. Includes an eraser mode. Hold Shift or press B to toggle paint mode on/off.
- **Color adjustments**: Fine-tune hue, saturation, gamma, and brightness with real-time preview. All adjustments are non-destructive until you save.
- **Image cropping**: Select and crop to any rectangular region of the image with pixel-precise control.
- **Gaussian blur**: Apply adjustable blur filters for effects or to obscure sensitive information.
- **Save with adjustments**: Export images with all your current adjustments (color, crop, rotation, drawings) baked in.

### Animation Support
- **Animated GIF playback**: Full support for animated GIFs with smooth frame timing.
- **Animated WebP playback**: Play animated WebP files with proper frame delays.
- **Playback controls**: Play, pause, and scrub through animation frames. Jump to any frame or step through frame-by-frame.

### Customization
- **Multiple UI themes**: Choose from several built-in color themes to match your preference or reduce eye strain.
- **Custom backgrounds**: Set any solid color as the image background, or use a customizable checkerboard pattern (great for viewing transparency). Configure both checkerboard colors.
- **Persistent settings**: Optionally remember zoom level and pan position for each image during a session, or reset to default view on every image change.
- **Keyboard shortcuts**: Full keyboard control for navigation, zooming, rotation, animation playback, and more. See the config panel for the complete list.

## Installation

### [Option 1] Standalone EXE from GitHub releases

Download and run the latest standalone `CactusViewer.exe` file from [releases](https://github.com/Wassimulator/CactusViewer/releases).

### [Option 2] Using Scoop

This will install the standalone `CactusViewer.exe` using [Scoop](https://scoop.sh/), which will also add a shortcut to the start menu and add `CactusViewer.exe` to the PATH environment variable.

1. Install Scoop if you don't already have it.
   - Instructions: <https://scoop.sh/>
2. Add the Scoop Extras bucket if you don't already have it.
   - `scoop bucket add extras`
3. Install CactusViewer with Scoop.
   - `scoop install extras/cactusviewer`

CactusViewer can later be updated with `scoop upgrade cactusviewer` or `scoop upgrade --all` (updates all apps installed with Scoop).

## Usage
- Opening a file:
  - Drag and drop image file into the viewer, if the file is supported it should load the file and read the folder contents of the file's directory.
  - CactusViewer also supports "Open With..." by choosing it as the program to open an image with.
  - You can also use the command line to open a file by typing: `> CactusViewer.exe "path_to_file"` (make sure you write the full path of the file!).
- Move the mouse to the bottom of the screen to view controls, click on "config" to see controls and change settings.

## Building
### Windows
All dependencies are within the repo for your, my, and everyone's convenience.
Using an MSVC 64 bit developer terminal:
- run `b.bat` to build the project in debug mode, output in `\bin`
  (the debug version expects the font file in src, so terminal calls need to have bin as the working directory to run
   also make sure you have D3D11 debug layers from [here](https://learn.microsoft.com/en-us/windows/uwp/gaming/use-the-directx-runtime-and-visual-studio-graphics-diagnostic-features) installed or remove that flag in `init_d3d11()` when building in debug mode).
- run `b.bat release` to build the project in release mode, output in `\bin`

## Remarks
- Update 2.0 ditches ImGui and OpenGL, the executable runs now on D3D11 with a handmade immediate mode UI library. If you have Windows, it should work, so please report any bugs!
- For the sake of everyone's sanity, this repo has prebuilt binaries of all the above for Windows x64, I will provide other builds if I make any.

## Troubleshooting
If you encounter any issues or crashes, short of trying to debug it yourself, CactusViewer writes a debug log to:
```
%APPDATA%\CactusViewer\debug.log
```
If you need help please [create a GitHub issue](https://github.com/Wassimulator/CactusViewer/issues) describing the problem and attach your log file or the relevant entries.

## Possible improvements
Possible things to expand on:
- Unlock the supported image resolutions sizes beyond maximum GPU texture sizes, to support images larger than 60MP or 268MP respectively.
- Decode images at lower resolutions at first and only decode full image upon zooming, to improve display speed of large files.
- Providing an optional software renderer backend.
