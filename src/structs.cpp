#pragma once
#include "main.h"
#include <emaths.h>
using namespace Emaths;

struct keys
{
    bool disconnect = false;

    bool RightButton = false;
    bool LeftButton = false;
    bool UpButton = false;
    bool DownButton = false;

    bool Enter = false;

    bool A_Key = false;
    bool B_Key = false;
    bool C_Key = false;
    bool D_Key = false;
    bool E_Key = false;
    bool F_Key = false;
    bool G_Key = false;
    bool H_Key = false;
    bool I_Key = false;
    bool J_Key = false;
    bool K_Key = false;
    bool L_Key = false;
    bool M_Key = false;
    bool N_Key = false;
    bool O_Key = false;
    bool P_Key = false;
    bool Q_Key = false;
    bool R_Key = false;
    bool S_Key = false;
    bool T_Key = false;
    bool U_Key = false;
    bool V_Key = false;
    bool W_Key = false;
    bool X_Key = false;
    bool Y_Key = false;
    bool Z_Key = false;

    bool _0_Key = false;
    bool _1_Key = false;
    bool _2_Key = false;
    bool _3_Key = false;
    bool _4_Key = false;
    bool _5_Key = false;
    bool _6_Key = false;
    bool _7_Key = false;
    bool _8_Key = false;
    bool _9_Key = false;

    bool LBracket_Key = false;
    bool RBracket_Key = false;

    bool Quote_Key = false;
    bool Colon_Key = false;
    bool Comma_Key = false;
    bool Dot_Key = false;
    bool FSlash_Key = false;
    bool BSlash_Key = false;
    bool Return_Key = false;
    bool Minus_Key = false;
    bool Plus_Key = false;
    bool Equals_Key = false;
    bool Backspace_Key = false;
    bool Delete_Key = false;

    bool F1_Key = false;
    bool F2_Key = false;
    bool F3_Key = false;
    bool F4_Key = false;
    bool F5_Key = false;
    bool F6_Key = false;
    bool F7_Key = false;
    bool F8_Key = false;
    bool F9_Key = false;
    bool F10_Key = false;
    bool F11_Key = false;
    bool F12_Key = false;

    bool Esc_Key = false;

    bool Backquote_Key = false;

    bool Shift = false;
    bool Tab_Key = false;
    bool Space_Key = false;
    bool Ctrl_Key = false;
    bool Alt_Key = false;

    bool MouseLeft = false;
    bool MouseLeftUp = false;
    bool MouseLeftAlready = false;
    bool MouseLeftReleased = false;
    bool MouseLeftOnce = false;
    bool MouseLeft_Click = false;

    bool MouseRight = false;
    bool MouseRightUp = false;
    bool MouseRightAlready = false;
    bool MouseRightOnce = false;

    bool MouseMiddle = false;
    bool MouseMiddleOnce = false;

    SDL_Point Mouse;
    SDL_Point Logical_Mouse;
    int xrel;
    int yrel;
    int ScrollY = 0;
    int ScrollYdiff = 0;
};

struct color
{
    union
    {
        float E[4];
        struct
        {
            float r, g, b, a;
        };
    };
    color() : E{0, 0, 0, 255} {}
    color(float r, float g, float b, float a) : E{r, g, b, a} {}
};

struct vertex
{
    v2 P;
    v2 UV;
    color C;
    float Z;
};

struct glrect
{
    vertex V[6];
};
struct image
{
    int w;
    int h;
    int n;
    unsigned char *data;
    GLuint TextureID;
    int frac1, frac2;
    float aspectratio;
};

struct graphics
{
    GLuint VAO = 0;
    GLint MAX_GPU = 0;
    GLuint MainProgram;
    GLuint BGProgram;

    image MainImage;

    float aspect_wnd;
    float aspect_img;
};

struct signals
{
    bool Initstep2 = false;
    bool update_blending = false;
    bool update_filtering = false;
    bool nextimage = false;
    bool previmage = false;
    bool update_truescale = false;
};

struct global
{
    graphics Graphics;
    keys Keys;
    cf_file_t *files = nullptr;
    bool *files_loading = nullptr;
    bool *files_GIF = nullptr;
    uint allocated_files;
    uint max_files;
    uint CurrentFileIndex;
    SDL_mutex *Mutex;
    signals signals;

    bool loaded = false;
    float scale = 1;
    float truescale = 1;
    float req_truescale = 1;
    bool nearest_filtering = false;
    bool pixelgrid = false;
    v2 Position = v2(0, 0);
    cf_file_t file;
    v2 PixelMouse;
    float InspectColors[4];

    gd_GIF *GIF = nullptr;
    char *GIF_buffer[1024];
    uint16_t GIF_FrameDelays[1024];
    GLuint GIFTextureID;
    int GIF_frames;
    int GIF_index;
    bool GIF_Loaded;
    bool GIF_Play;
};

global Global = {0};
global *G = &Global;
