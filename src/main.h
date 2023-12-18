#pragma once

#define UNICODE
#include <d3d11.h>
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#define STBI_WINDOWS_UTF8
#include "stb.c"
#include <windows.h> 
#include <windowsx.h> 
#include <commctrl.h> 
#include <shellscalingapi.h>
#include <stdint.h>
#include <shlwapi.h>
#include <shlobj.h>
#include <exdisp.h>
#include <GdiPlus.h>
#include <shobjidl_core.h>
#pragma comment(lib, "shlwapi.lib")
#include <stb_truetype.h>
#define CUTE_FILES_IMPLEMENTATION
#include <cute_files.h>
#include "logo.cpp"
#define WGL_WGLEXT_PROTOTYPES
#include <dynarray.h>
#include <emaths.h>
#include <webp/webp/decode.h>
#include <webp/demux/demux.c>
#include <wincodec.h>
#include <propidl.h>
#include <dwmapi.h>
#include <ShellScalingAPI.h>
#pragma comment(lib, "Shcore.lib")

#define _MAX_FPS 250;

#define VERSION_N "2.1.3"
#define VERSION VERSION_N" - 18/12/23"

#define handle_signal(signal) for(int _i_ = (signal); _i_; _i_ = 0, (signal = false))
#define send_signal(signal) signal = true
#define array_size(a) (sizeof(a) / sizeof(*(a)))
#define couti(_i_)  printf("%-20s = %10i\n", #_i_, _i_);
#define coutb(_i_)  printf("%-20s = %10s\n", #_i_, _i_ ? "TRUE" : "FALSE");
#define coutf(_i_)  printf("%-20s = %10f\n", #_i_, _i_);
#define coutv2(_i_) printf("%-20s = %10f, %10f\n", #_i_, _i_.x, _i_.y);
#define coutv4(_i_) printf("%-20s = %10f, %10f, %10f, %10f\n", #_i_, _i_.x, _i_.y, _i_.z, _i_.w);
#define coutiv2(_i_) printf("%-20s = %10i, %10i\n", #_i_, _i_.x, _i_.y);
#define couts(_i_)  printf("%-20s\n", _i_);
#define swap(_type_, a, b) { _type_ tmp = a; a = b; b = tmp; }

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float f32;
typedef double f64;

typedef uint32_t b32;

int MAX_FPS = _MAX_FPS;

bool Running = true;
int WW, WH;
bool firsttime = true;
int FPS = MAX_FPS;
bool VSYNC_ON = false;
int mouse_detection = WH - 180;
bool keepUI = false;
float bg_color[4];
float checkerboard_color_1[3];
float checkerboard_color_2[3];
float Checkerboard_size;
int RGBAflags[4];
wchar_t *global_temp_path;

HWND hwnd;
HGLRC hRC;
HDC hdc;

char *APPDATA_FOLDER;
