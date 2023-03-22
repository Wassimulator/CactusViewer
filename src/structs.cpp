#pragma once
#include "main.h"
#include <emaths.h>
using namespace Emaths;


enum keys_
{
    Key_Right,
    Key_Left,
    Key_Up,
    Key_Down,
    Key_Enter,
    Key_A,
    Key_B,
    Key_C,
    Key_D,
    Key_E,
    Key_F,
    Key_G,
    Key_H,
    Key_I,
    Key_J,
    Key_K,
    Key_L,
    Key_M,
    Key_N,
    Key_O,
    Key_P,
    Key_Q,
    Key_R,
    Key_S,
    Key_T,
    Key_U,
    Key_V,
    Key_W,
    Key_X,
    Key_Y,
    Key_Z,
    Key_0,
    Key_1,
    Key_2,
    Key_3,
    Key_4,
    Key_5,
    Key_6,
    Key_7,
    Key_8,
    Key_9,
    Key_num_0,
    Key_num_1,
    Key_num_2,
    Key_num_3,
    Key_num_4,
    Key_num_5,
    Key_num_6,
    Key_num_7,
    Key_num_8,
    Key_num_9,
    Key_LBracket,
    Key_RBracket,
    Key_Quote,
    Key_Colon,
    Key_Comma,
    Key_Dot,
    Key_FSlash,
    Key_BSlash,
    Key_Return,
    Key_Minus,
    Key_Plus,
    Key_Multiply,
    Key_Equals,
    Key_Backspace,
    Key_Delete,
    Key_F1,
    Key_F2,
    Key_F3,
    Key_F4,
    Key_F5,
    Key_F6,
    Key_F7,
    Key_F8,
    Key_F9,
    Key_F10,
    Key_F11,
    Key_F12,
    Key_Esc,
    Key_Backquote,
    MouseL,
    MouseR,
    MouseM,
    MouseX1,
    MouseX2,
    Key_Shift,
    Key_LShift,
    Key_RShift,
    Key_Tab,
    Key_Space,
    Key_Ctrl,
    Key_LCtrl,
    Key_RCtrl,
    Key_Alt,
    Key_LAlt,
    Key_RAlt,
    Key_Pause,
    Key_CapsLock,
    Key_PgUp,
    Key_PgDn,
    Key_Home,
    Key_End,
    Key_Select,
    Key_Print,
    Key_Insert,
    Key_LWIN,
    Key_RWIN,
    Key_NumLock,
    Key_ScrollLock,

    key_COUNT
};

struct key
{
    bool up;
    bool dn;
    bool on;
};


struct keys
{
    key K[key_COUNT];
    bool disconnect = false;

    v2 Mouse;
    v2 Mouse_rel;
    int ScrollY = 0;
    int ScrollYdiff = 0;
};

enum filetypes
{
    filetype_Regular,
    filetype_GIF,
    filetype_HEIC
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
    GLuint LogoTextureID;

    float aspect_wnd;
    float aspect_img;
};

struct error
{
    int timer;
    char string[265];
};

struct signals
{
    bool UpdatePass = false;
    bool Initstep2 = false;
    bool update_blending = false;
    bool update_filtering = false;
    bool nextimage = false;
    bool previmage = false;
    bool update_truescale = false;
};

struct file_data
{
    cf_file_t file;
    bool loading = false;
    int type = 0;
    bool failed = false;
    v2 pos = v2();
    float scale = 0;
    int index;
};

struct global
{
    graphics Graphics;
    keys Keys;
    dynarray <file_data> Files;
    uint CurrentFileIndex;
    CRITICAL_SECTION Mutex;
    CRITICAL_SECTION SortMutex;
    signals signals;
    error Error;

    bool keep_menu = false;
    bool loaded = false;
    bool sorting = false;
    float scale = 1;
    float truescale = 1;
    float req_truescale = 1;

    v2 Position = v2(0, 0);
    cf_file_t file;
    v2 PixelMouse;
    float InspectColors[4];

    unsigned char *GIF_buffer;
    int *GIF_FrameDelays;
    GLuint GIFTextureID;
    int GIF_frames;
    int GIF_index;
    bool GIF_Loaded;
    bool GIF_Play;
    bool settings_visible;
    bool Droppedfile;
    bool Loading_Droppedfile;

    int32_t settings_resetpos;
    int32_t settings_resetzoom;
    float settings_movementmag;
    float settings_shiftslowmag;
    bool settings_movementinvert;
    bool settings_autoplayGIFs;
    bool settings_Sort = false;
    bool nearest_filtering = false;
    bool pixelgrid = false;
};

global Global = {0};
global *G = &Global;
