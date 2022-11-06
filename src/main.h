#pragma once
#include <stdlib.h>
#include <stdio.h>
#include <SDL/SDL.h>
#include <iostream>
#include "stb.c"
#include <stb_truetype.h>
#define CUTE_FILES_IMPLEMENTATION
#include <cute_files.h>
#include "glad.c"
#include "logo.cpp"

#define IMGUI 1

#if IMGUI
#include "imgui/imconfig.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_opengl3.h"
#include "imgui/imgui_impl_sdl.h"
#include "imgui/imgui_impl_sdlrenderer.h"
#endif


#define _MAX_FPS 250;

#define VERSION "1.0.0 - 11/2022"

typedef unsigned int uint;

int MAX_FPS = _MAX_FPS;

bool Running = true;
SDL_Window *Window;
int WindowWidth, WindowHeight;
SDL_GLContext GLContext;
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
SDL_Event Ev = {0};
SDL_Event *Event = &Ev;
char *TempPath;

char *APPDATA_FOLDER;

