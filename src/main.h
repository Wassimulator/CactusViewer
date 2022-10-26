#pragma once
#include <stdlib.h>
#include <stdio.h>
#include <SDL/SDL.h>
#include <iostream>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <stb_truetype.h>
#define CUTE_FILES_IMPLEMENTATION
#include <cute_files.h>
#include "glad.c"
#include "gifdec.c"

#define IMGUI 1

#if IMGUI
#include "imgui/imconfig.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_opengl3.h"
#include "imgui/imgui_impl_sdl.h"
#include "imgui/imgui_impl_sdlrenderer.h"
#endif

typedef unsigned int uint;

bool Running = true;
SDL_Window *Window;
int WindowWidth, WindowHeight;
SDL_GLContext GLContext;
    bool firsttime = true;
int FPS = 250;
bool VSYNC_ON = false;
int MouseDetection = WindowHeight - 180;
bool keepUI = false;
float BGcolor[4];
float Checkerboard_color1[3];
float Checkerboard_color2[3];
float Checkerboard_size;