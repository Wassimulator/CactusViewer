#pragma once
#define UI_MAX_STRING_LEN 256
#include "wassentials.h"
#ifndef STB_RECT_PACK_IMPLEMENTATION
#define STB_RECT_PACK_IMPLEMENTATION
#endif
#ifndef STB_TRUETYPE_IMPLEMENTATION
#define STB_TRUETYPE_IMPLEMENTATION
#endif
#include <stb_rect_pack.h>
#include <stb_truetype.h>

#include <ft2build.h>
#include <freetype/ftlcdfil.h>
#include FT_FREETYPE_H

#define UI_MAX_VERTICES 10000
#define UI_MAX_TEXTURES 5

#define M_PI 3.14159265358979323846

#define UI_assert(argument) assert(argument);
#define ARRAY_SIZE(x)  (sizeof(x) / sizeof((x)[0]))
#define UI_clamp(value, min, max) (((value) < (min)) ? (min) : (((value) > (max)) ? (max) : (value)))
#define UI_swap(_type_, a, b) { _type_ tmp = a; a = b; b = tmp; }


#include "ui_shapes.cpp"

//TODO: uncomment these in case we ship this seperately
typedef uint8_t     u8;
typedef uint16_t    u16;
typedef uint32_t    u32;
typedef uint64_t    u64;
typedef int8_t      i8;
typedef int16_t     i16;
typedef int32_t     i32;
typedef int64_t     i64;
typedef float       f32;
typedef double      f64;


enum struct UI_Render_Backend_Type {
    None,
    D3D11,
};
enum struct UI_Platform_Type {
    None,
    Win32,
};

struct UI_Font_Glyph_Info {
	int x0, y0, x1, y1;	// uv int atlas
	int x_off, y_off;   // bearing
    int ybearing, xbearing;
	int advance_x;       
};

struct UI_Font {
    i32                 texture_handle;
    int                 *sizes;
    int                 *true_sizes;
    UI_Font_Glyph_Info  *char_info;
    int                 char_count;
    int                 sizes_count;
    int                 first;
    int                 w, h;
};

struct UI_Color4 {
    u32  c[4] = {0};
	UI_Color4 operator&(const u32 value) const {
		UI_Color4 result;
		result.c[0] = c[0] & value;
		result.c[1] = c[1] & value;
		result.c[2] = c[2] & value;
		result.c[3] = c[3] & value;
		return result;
	}
};

struct UI_Rect {
    union {
        struct {
            f32 x, y, w, h;
        };
        struct {
            v2 pos;
            v2 size;    
        };
    };
    // default constructor for compilation reasons:
    UI_Rect() : pos(0.0f, 0.0f), size(0.0f, 0.0f) {}
	UI_Rect(f32 pos_x, f32 pos_y, f32 size_w, f32 size_h) {
		x = pos_x;
		y = pos_y;
		w = size_w;
		h = size_h;
	}
    UI_Rect(v2 rect_pos, v2 rect_size) {
        pos =  rect_pos;
        size = rect_size;
    }
};

#define    UI_Vertex_Flags_srgb      (1<<1)
#define    UI_Vertex_Flags_lcd       (1<<2)

struct UI_Vertex {
    v2      dst_p0;
    v2      dst_p1;
    v2      src_p0;
    v2      src_p1;
    v2      clp_p0;
    v2      clp_p1;
    f32     depth;
    v4      colors[4];
    v4      roundness;
    f32     softness;
    f32     border_size;
    i32     texture_id;
    u32     ui_block;
    f32     rotation;
	i32		flags;
};

#define    UI_Block_Flags_none              0
#define    UI_Block_Flags_draw_image        (1<<1)
#define    UI_Block_Flags_draw_text         (1<<2)
#define    UI_Block_Flags_draw_border       (1<<3)
#define    UI_Block_Flags_draw_background   (1<<4)
#define    UI_Block_Flags_render_srgb       (1<<5)
#define    UI_Block_Flags_no_clip       	(1<<6)
#define    UI_Block_Flags_hit_test       	(1<<7)


enum UI_Edge {
    UI_Edge_none = -1,
    UI_Edge_left,
    UI_Edge_right,
    UI_Edge_top,
    UI_Edge_bottom,

    UI_Edge_Count
};

struct UI_Input {
    u32      block_key;
    bool     hovered;
    UI_Edge  on_edge         = UI_Edge_none;
    bool     mouse_L_double_click = false;
    bool     mouse_L_triple_click = false;
	int      mouse_L_dn      = false;
	int      mouse_L_up      = false;
	int      mouse_L_press   = false;
	int      mouse_R_dn      = false;
	int      mouse_R_up      = false;
	int      mouse_R_press   = false;
};

enum struct UI_Size_t {
    pixels,
    text_content,
    percent_of_parent,
    sum_of_children,
};

struct UI_Size {
    UI_Size_t  type;
    f32        value;
    f32        strictness;
};

enum struct UI_Position_t {
    automatic,
    absolute,
    percent_of_parent,
    pixels_from_parent,
};

struct UI_Position {
    UI_Position_t    type = UI_Position_t::automatic;
    f32              value;
};

enum  Axis2 {
	axis_both = -1,
    axis_x,
    axis_y,
    axis_count
};

enum UI_Align {
    align_start,
    align_center,
    align_end,
};

struct UI_Layout {
    Axis2           axis    = axis_y;
    v2              spacing = v2(0,0);
    v2              padding  = v2(0,0);
    UI_Align        align[axis_count];
};

enum UI_Color_Type {
    c_all = -1,
    c_background,
    c_border,
    c_text,
    color_COUNT
};

struct UI_Block;

struct UI_Style {
    UI_Size             size[axis_count];
    UI_Position         position[axis_count];
    UI_Layout           layout;
    UI_Color4           color[color_COUNT];
    UI_Font             *font;
    UI_Block*           clip_block;
    u32                 font_size;
    f32                 border_size;
    f32                 softness;
    v4                  roundness;
    f32                 anim_time_hot = 1;
    f32                 anim_time_active = 1;
    v2                  texture_uv;
    v2                  texture_src_size;
    f32                 texture_rotation;
    i32                 texture_handle = -1; // defaults to -1 so that the shader doesn't sample, TODO(Wassim): 
                                             // not clean, make the shader sample a blank white box instead?
};

struct UI_Block {
    // tree links
    UI_Block        *first;     // first child of this block
    UI_Block        *last;      // last child of this block's children
    UI_Block        *next;      // next sibling
    UI_Block        *prev;      // previous sibling
    UI_Block        *parent;    // this block's parent

    // hash + generation info
    u32             hash;
    u64             frame_created;

    // per-frame info provided by builders
    u32             flags;
    String8         string;
    u32             depth_level;
    UI_Style        style;

    // computed every frame
    v2              size;           // computed size based on the size requrested in `style`
    v2              position;       // computed position based on the position requrested in `style`
    UI_Rect         rect;
    UI_Rect         clip_rect;
    v2              cursor;
    u32             vertex_id;
    v2              interim_size;   // used internally
    f32             layout_data[axis_count];    // used internally

    // persistent data
    f32             hot_t;
    f32             active_t;
    i32             fade_dir;

	bool 			resizeable;
};

struct UI_Debug {
    int last_block_count = 0;
    int last_vertex_count = 0;
	size_t allocated_bytes = 0;
	int freed_blocks= 0;
};

enum UI_Corner {
	UI_Corner_bl,
	UI_Corner_tl,
	UI_Corner_br,
	UI_Corner_tr,
};

struct UI_Signals {
	bool nullthing = false;
};

struct UI_Block_Data {
	void *buffer;
	u64  size;
	u32	 hash;
};

struct UI_string_array {
	char *buffer;
	u32 capacity;
	u32 count;
};

struct UI_Hit_Test_Item {
	u32 hash;
	u32 depth_level;
};
struct UI_Context {
	u32							buffer_index;
	Dynarray <UI_Block>         buffers[2];
    Dynarray <UI_Block*>        parents;
    Dynarray <u32>              hashes;
    Dynarray <UI_Input>         inputs;
	Dynarray <UI_Block_Data>	data_chunks;
	Dynarray <UI_Hit_Test_Item> blocks_hit_test;

	UI_Hit_Test_Item			hit_test_result;

    FT_Library                  ft_lib;
	UI_string_array				strings;

    bool                        textures[UI_MAX_TEXTURES];
    Dynarray <UI_Font>          fonts;  
    u64                         frame_id; // starts at 0 and increments every frame
    u64                         last_frame_time; //in ms
    u64                         unique_counter; // resets to 0 every frame

    bool                        want_capture_mouse;
    bool                        want_capture_keyboard;

	char						*string_storage;
	u32							string_storage_size;
	u32							string_storage_capacity;

	UI_Signals					signals;

    UI_Debug                    debug;

    // Render   
    UI_Render_Backend_Type      backend;
    UI_Platform_Type            platform;
    void                        *backend_context;
    v2                          viewport;
    Dynarray <UI_Vertex>        vertices;

    bool                        initialized;
};


void *UI_malloc(UI_Context *ctx, size_t size) {
	ctx->debug.allocated_bytes += size;
	void * result = malloc(size);
	UI_assert(result);
	return result;
}

void UI_free(UI_Context *ctx, size_t size, void* address) {
	ctx->debug.allocated_bytes -= size;
	free(address);
}
