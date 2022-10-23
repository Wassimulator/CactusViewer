#pragma once
#include <stdio.h>
#include <SDL/SDL.h>
#include <iostream>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <stb_truetype.h>
#define CUTE_FILES_IMPLEMENTATION
#include <cute_files.h>
#include "glad.c"

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
