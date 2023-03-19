#pragma once
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include "stb.c"
#include <windows.h>
#include <stb_truetype.h>
#define CUTE_FILES_IMPLEMENTATION
#include <cute_files.h>
#include "logo.cpp"
#include "glad.c"
#define WGL_WGLEXT_PROTOTYPES
#include <wglext.h>

#define IMGUI 1

#if IMGUI
#include "imgui/imconfig.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_opengl3.h"
#include "imgui/imgui_impl_win32.h"
#endif


#define _MAX_FPS 250;

#define VERSION "1.2.0 - 03/2023"

typedef unsigned int uint;

int MAX_FPS = _MAX_FPS;

bool Running = true;
int WindowWidth, WindowHeight;
bool firsttime = true;
int FPS = MAX_FPS;
bool VSYNC_ON = false;
int MouseDetection = WindowHeight - 180;
bool keepUI = false;
float BGcolor[4];
float Checkerboard_color1[3];
float Checkerboard_color2[3];
float Checkerboard_size;
int RGBAflags[4];
char *TempPath;

HWND hwnd;
HGLRC hRC;
HDC hdc;

char *APPDATA_FOLDER;

