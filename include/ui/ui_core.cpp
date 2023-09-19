#pragma once
#include "ui_core.h"

// helpers /////////////////////////////////////
char *UI_tprintf(UI_Context *ctx, char* format, ...) {
	va_list args;
	va_start(args, format);
	size_t bytes = vsnprintf(nullptr, 0, format, args) + 1;
	va_end(args);

	if (ctx->string_storage_size + bytes > ctx->string_storage_capacity) {
		UI_assert(!"Out of string storage memory.");
	}
	char *at = ctx->string_storage + ctx->string_storage_size;
	//char *at = (char*)malloc(bytes);

	va_start(args, format);
	vsnprintf(at, bytes, format, args);
	va_end(args);

	ctx->string_storage_size += bytes;
	return at;
}

bool UI_handle_signal(bool *signal) {
	bool result = *signal;
	*signal = false;
	return result;
}

Dynarray<UI_Block>* UI_get_current_frame_buffer(UI_Context *ctx) {
	return &ctx->buffers[ctx->buffer_index];
}

Dynarray<UI_Block>* UI_get_previous_frame_buffer(UI_Context *ctx) {
	return &ctx->buffers[1 - ctx->buffer_index];
}
#define UI_CURRENT 	0
#define UI_PREVIOUS 1

UI_Block *UI_find_block(UI_Context *ctx, u32 hash, int frame) {
	UI_assert(frame == UI_CURRENT || frame == UI_PREVIOUS);
	UI_Block *result = 0;
	Dynarray<UI_Block>* buffer;
	if 		(frame == UI_PREVIOUS) buffer = UI_get_previous_frame_buffer(ctx);
	else if (frame == UI_CURRENT)  buffer = UI_get_current_frame_buffer(ctx);
	if (hash != 0) {
		for (u32 i = 0; i < buffer->count; ++i) {
			UI_Block *block = &buffer->data[i];
			if (block->hash == hash) {
				result = block;
				break;
			}
		}
	}
	return result;
}




inline UI_Block *UI_find_block(UI_Context *ctx, u32 hash, bool previous = true) {
	auto buffer = UI_get_previous_frame_buffer(ctx);
	if (!previous)
		buffer = UI_get_current_frame_buffer(ctx);
	for (u32 i = 0; i < buffer->count; i++) {
		if (buffer->data[i].hash == hash) {
			return &buffer->data[i];
        }
    }
    return nullptr;
}

//inline UI_Block *UI_find_first_unused_block(UI_Context *ctx) {
//    for (u32 i = 0; i < ctx->blocks.count; i++) {
//        if (ctx->blocks[i].used == false) {
//			if (ctx->blocks[i].data.buffer != nullptr) 
//				free(ctx->blocks[i].data.buffer);
//			memset(&ctx->blocks[i], 0, sizeof(UI_Block));
//            return &ctx->blocks[i];
//        }
//    }
//    return nullptr;
//}

inline u32 UI_v4_to_u32(v4 color) {
	u32 r = (u32)(color.r * 255.0f + 0.5f);
	u32 g = (u32)(color.g * 255.0f + 0.5f);
	u32 b = (u32)(color.b * 255.0f + 0.5f);
	u32 a = (u32)(color.a * 255.0f + 0.5f);
	return (r << 24) | (g << 16) | (b << 8) | a;
}

inline v4 UI_u32_to_v4(u32 color) {
	v4 result;
	result.e[0] = ((color & 0xff000000) >> 24) / 255.f;
	result.e[1] = ((color & 0x00ff0000) >> 16) / 255.f;
	result.e[2] = ((color & 0x0000ff00) >> 8 ) / 255.f;
	result.e[3] = ((color & 0x000000ff) >> 0 ) / 255.f;
	return result;
}

inline u32 UI_invert_u32_color(u32 color) {
	v4 inter = UI_u32_to_v4(color);
	inter.r = 1 - inter.r;
	inter.g = 1 - inter.g;
	inter.b = 1 - inter.b;
	return UI_v4_to_u32(inter);
}

UI_Color4 UI_color4_sld_u32(u32 color) 						{ return { color, color, color, color }; }
UI_Color4 UI_color4_sld_v4(v4 color) 						{ return UI_color4_sld_u32(UI_v4_to_u32(color)); }
UI_Color4 UI_color4_sld_v3(v3 color) 						{ return UI_color4_sld_u32(UI_v4_to_u32(v4(color.r, color.g, color.b, 1.0))); }
UI_Color4 UI_color4_vrt(u32 color_top,  u32 color_bottom) 	{ return { color_bottom, color_top, color_bottom, color_top }; }
UI_Color4 UI_color4_hrz(u32 color_left, u32 color_right) 	{ return { color_left, color_left, color_right, color_right }; }
UI_Color4 UI_color4_vrt(v4  color_top,  v4  color_bottom)	{ return UI_color4_vrt(UI_v4_to_u32(color_top), UI_v4_to_u32(color_bottom)); }
UI_Color4 UI_color4_hrz(v4  color_left, v4  color_right) 	{ return UI_color4_hrz(UI_v4_to_u32(color_left), UI_v4_to_u32(color_right)); }

UI_Color4 UI_mix_gradients(UI_Color4 a, UI_Color4 b, float a_portion) {
    UI_Color4 result;
    if      (a_portion == 0) return a;
    else if (a_portion == 1) return b;
	for (int i = 0; i < 4; i++) {
		u32 col_a = a.c[i], col_b = b.c[i];
		u32 r_a = (col_a >> 24) & 0xFF, g_a = (col_a >> 16) & 0xFF, b_a = (col_a >> 8) & 0xFF, a_a = (col_a >> 0) & 0xFF;
		u32 r_b = (col_b >> 24) & 0xFF, g_b = (col_b >> 19) & 0xFF, b_b = (col_b >> 8) & 0xFF, a_b = (col_b >> 0) & 0xFF;
		result.c[i] = ((u32)(r_a * (1.0f - a_portion) + r_b * a_portion) << 24) |
					  ((u32)(g_a * (1.0f - a_portion) + g_b * a_portion) << 16) |
					  ((u32)(b_a * (1.0f - a_portion) + b_b * a_portion) << 8)  |
					  ((u32)(a_a * (1.0f - a_portion) + a_b * a_portion) << 0);
	}
    return result;
}

void UI_set_gradient_alpha(UI_Color4 *col, u8 alpha) {
	col->c[0] = col->c[0] & 0xFFFFFF00 | alpha;
	col->c[1] = col->c[1] & 0xFFFFFF00 | alpha;
	col->c[2] = col->c[2] & 0xFFFFFF00 | alpha;
	col->c[3] = col->c[3] & 0xFFFFFF00 | alpha;
}
void UI_set_gradient_alpha(UI_Color4 *col, float alpha) {
	UI_set_gradient_alpha(col, (u8)(alpha * 255.0f + 0.5f));
}

inline f32 UI_round_away_from_zero(f32 value) {
	return copysign(ceilf(abs(value)), value);
}

u32 UI_lerp_u32(u32 col_a, u32 col_b, f32 t) {
	u8 r1 = (col_a >> 24) & 0xFF, r2 = (col_b >> 24) & 0xFF;
	u8 g1 = (col_a >> 16) & 0xFF, g2 = (col_b >> 16) & 0xFF;
	u8 b1 = (col_a >> 8)  & 0xFF, b2 = (col_b >> 8)  & 0xFF;
	u8 a1 = (col_a >> 0)  & 0xFF, a2 = (col_b >> 0)  & 0xFF;

	u8 r = (u8)(r1 + UI_round_away_from_zero((r2 - r1) * t));
	u8 g = (u8)(g1 + UI_round_away_from_zero((g2 - g1) * t));
	u8 b = (u8)(b1 + UI_round_away_from_zero((b2 - b1) * t));
	u8 a = (u8)(a1 + UI_round_away_from_zero((a2 - a1) * t));

	return (r << 24) | (g << 16) | (b << 8) | a;
}

UI_Color4 UI_lerp_color4(UI_Color4 a, UI_Color4 b, f32 t) {
	UI_Color4 res;
	res.c[0] = UI_lerp_u32(a.c[0], b.c[0], t);
	res.c[1] = UI_lerp_u32(a.c[1], b.c[1], t);
	res.c[2] = UI_lerp_u32(a.c[2], b.c[2], t);
	res.c[3] = UI_lerp_u32(a.c[3], b.c[3], t);
	return res;
}

UI_Color4 UI_animate_color_3(
	UI_Color4 current_color,
	bool hot, bool active,
	UI_Color4 color_base, UI_Color4 color_hot, UI_Color4 color_active) {
	UI_Color4 target_color = active ? color_active 	: hot ? color_hot : color_base;
	f32 	  speed = 	   	 active ? 0.6f     	 	: hot ? 0.2f 	   : 0.2f;
	UI_Color4 result = UI_lerp_color4(current_color, target_color, speed);
	return result;
}
UI_Color4 UI_animate_color_4(
	UI_Color4 current_color,
	bool hot, bool active, bool disabled,
	UI_Color4 color_base, UI_Color4 color_hot, UI_Color4 color_active, UI_Color4 color_disabled) {
	UI_Color4 target_color = disabled ? color_disabled : active ? color_active 	: hot ? color_hot : color_base;
	f32 	  speed = 	   	 disabled ? 0.8f 		   : active ? 0.6f     	 	: hot ? 0.2f 	  : 0.2f;
	UI_Color4 result = UI_lerp_color4(current_color, target_color, speed);
	return result;
}


char* UI_sprintf(UI_string_array* string_array, const char *format, ...) {

	va_list va;
	va_start(va, format);
	int size = vsnprintf(0, 0, format, va);
	char* result = 0;

	UI_assert(string_array->count + size + 1 <  string_array->capacity)
	if (size >= 0) { 
		result = &string_array->buffer[string_array->count];
		vsnprintf(&string_array->buffer[string_array->count], size + 1, format, va);
		string_array->count += size + 1;
	}
	va_end(va);

	return result;
}

UI_Block *UI_get_current_parent(UI_Context *ctx) {
	if (ctx->parents.count)
		return ctx->parents.back();
	return nullptr;
}

//////////////////////////////////////////////


u32 UI_hash_djb2(UI_Context* ctx, char *str, u32 hash = 5381) {
    int c = *str;
	u32 count = 0;
	if (ctx->hashes.count > 0)
		hash = ctx->hashes.back();
	while ((c = *str++)) { // assumes str is null-terminated!
	//	if (len > 0 && count >= len)
	//		break;
		hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
		count++;
	}
    return hash;
}
/**
* @return returns -1 if there is no free texture slot available, otherwise returns a valid handle that is >= 0
**/
i32 UI_create_texture(UI_Context *ctx, u8 *data, int w, int h) {
    // check if we have a free texture slot available
    i32 handle = -1;
    for (int i = 0; i < UI_MAX_TEXTURES; i++) {
        if (ctx->textures[i] == false) {
            handle = i;
            ctx->textures[i] = true;
            break;
        }
    }
    // if we don't have a free slot, return -1 (error)
    if (handle == -1) {
        printf("UI Error: A maximum of 5 textures can be created for the UI context!\n");
        return handle;
    }

    switch (ctx->backend)
    {
        case UI_Render_Backend_Type::D3D11: 
            UI_d3d11_create_texture(ctx, handle, data, w, h); break;
    }

    return handle;
}

UI_Font *UI_load_font_memory(UI_Context *ctx, unsigned char *data, size_t file_size, int ASCII_start, int ASCII_end, int *sizes, int sizes_count)
{
    UI_assert(ctx != nullptr && "UI context *ctx is a null pointer!");
    UI_assert(ctx->initialized && "UI context wasn't initialized!");
    UI_assert(ctx->backend != UI_Render_Backend_Type::None && "No render backend is initialized!");

    UI_Font newfont = {0};
    ctx->fonts.push_back(newfont);
    UI_Font *font = &ctx->fonts.back();

    FT_Face face;
    FT_New_Memory_Face(ctx->ft_lib, data, file_size, 0, &face);

    int char_count = ASCII_end - ASCII_start + 1;
    font->char_count = char_count;

    font->sizes = (int *)malloc(sizeof(int) * sizes_count);
    font->true_sizes = (int *)malloc(sizeof(int) * sizes_count);
    for (int i = 0; i < sizes_count; i++) {
        font->sizes[i] = sizes[i];
        font->true_sizes[i] = 0;
    }

    int w = 2000, h = 2000; // TODO(Wassim): fixed for now but we need a better way to handle this.
    font->w = w;
    font->h = h;
    u8 *pixels_RGB  = (u8 *)calloc(w * h * 3, sizeof(u8));
    u8 *pixels_RGBA = (u8 *)calloc(w * h * 4, sizeof(u8));
    int pen_x = 0, pen_y = 0;
    font->char_info = (UI_Font_Glyph_Info *)malloc(sizeof(UI_Font_Glyph_Info) * char_count * sizes_count);
    font->sizes_count = sizes_count;
    int max_height_row = 0;
    for (int size_index = 0; size_index < sizes_count; size_index++) {
        int size_h = sizes[size_index] * 64 * 72 / 69.0f;
        FT_Set_Char_Size(face, 0, size_h, 0, 0);
        FT_BBox bbox = face->bbox;
        for (int ASCII_code = ASCII_start; ASCII_code <= ASCII_end; ASCII_code++) {
            FT_Load_Char(face, ASCII_code, FT_LOAD_RENDER | FT_LOAD_TARGET_LCD);
            FT_GlyphSlot glyph_slot = face->glyph;
            size_t offset = char_count * size_index;
            UI_Font_Glyph_Info* target_char = &font->char_info[offset + ASCII_code - ASCII_start];
            FT_Bitmap* bitmap = &face->glyph->bitmap;
            if(pen_x + bitmap->width >= w) {
			    pen_x = 0;
			    pen_y += max_height_row;
                max_height_row = 0;
		    }
            for(int row = 0; row < bitmap->rows; ++row){
		    	for(int col = 0; col < bitmap->width; col += 3){
		    		int x = pen_x + col / 3;
		    		int y = pen_y + row;
		    		pixels_RGB[y*w*3 + x*3 + 0] = bitmap->buffer[row * bitmap->pitch + col + 0];
		    		pixels_RGB[y*w*3 + x*3 + 1] = bitmap->buffer[row * bitmap->pitch + col + 1];
		    		pixels_RGB[y*w*3 + x*3 + 2] = bitmap->buffer[row * bitmap->pitch + col + 2];
		    	}
		    }
            target_char->x0         = pen_x;
		    target_char->y0         = pen_y;
		    target_char->x1         = pen_x + bitmap->width / 3;
		    target_char->y1         = pen_y + bitmap->rows;
		    target_char->x_off      = face->glyph->bitmap_left;
		    target_char->y_off      = face->glyph->bitmap_top;
		    target_char->advance_x  = (face->glyph->advance.x >> 6);
            target_char->ybearing   = (face->glyph->metrics.vertBearingY) >> 6;
            target_char->xbearing   = (face->glyph->metrics.vertBearingX) >> 6;

            int char_h = (face->glyph->metrics.horiBearingY) >> 6;
            font->true_sizes[size_index] = max(font->true_sizes[size_index], char_h);
            max_height_row = max(max_height_row, char_h);

		    pen_x += bitmap->width / 3 + 1;
        }
    }
    font->first = ASCII_start;

    for (int i = 0; i < w * h; ++i) {
        int r = pixels_RGB[i * 3 + 0];
        int g = pixels_RGB[i * 3 + 1];
        int b = pixels_RGB[i * 3 + 2];

        pixels_RGBA[i * 4 + 0] = r;
        pixels_RGBA[i * 4 + 1] = g;
        pixels_RGBA[i * 4 + 2] = b;

        pixels_RGBA[i * 4 + 3] = (r + g + b) / 3;
    }

    font->texture_handle = UI_create_texture(ctx, pixels_RGBA, w, h);
    free(pixels_RGB);
    free(pixels_RGBA);
    FT_Done_Face(face);
    return font;
}

UI_Font *UI_load_font_file(UI_Context *ctx, char *path, int ASCII_start, int ASCII_end, int *sizes, int sizes_count)
{
    int size = 0;
    unsigned char *buffer = nullptr;

    FILE *file = fopen(path, "rb");
    fseek(file, 0, SEEK_END);
    size = ftell(file);
    fseek(file, 0, SEEK_SET);
    buffer = (unsigned char *)malloc(size);
    fread(buffer, size, 1, file);
    fclose(file);

    UI_Font *F = UI_load_font_memory(ctx, buffer, size, ASCII_start, ASCII_end, sizes, sizes_count);

    free(buffer);
    return F;
}

void UI_push_vertex(UI_Context *ctx, UI_Vertex vertex) {
    UI_assert(ctx != nullptr && "UI context *ctx is a null pointer!");
    UI_assert(ctx->initialized && "UI context wasn't initialized!");
    switch (ctx->backend)
    {
        case UI_Render_Backend_Type::D3D11: 
            UI_d3d11_push_vertex(ctx, vertex); break;
    }
}

int UI_find_font_size_index(UI_Font *font, u16 size)
{
    int target = size;
    bool found = false;
    int result = 0;
    while (!found) {
        for (int i = 0; i < font->sizes_count; i++) {
            if (font->sizes[i] == size) {
                found = true;
                result = i;
            }
        }
        size--;
        size = clamp(size, font->sizes[0], 100);
    }
    return result;
}

bool UI_font_size_exists(UI_Font *font, u16 size, int *size_index) {
    for (int i = 0; i < font->sizes_count; i++) {
        if (font->sizes[i] == size) {
            *size_index = i;
            return true;
        }
    }
    return false;
}

UI_Block_Data UI_find_else_allocate_data(UI_Context *ctx, u32 hash, size_t size, bool *created_new = 0) {
	for (int i = 0; i < ctx->data_chunks.count; i++) {
		if (ctx->data_chunks[i].hash == hash)
			return ctx->data_chunks[i];
	}

    UI_Block_Data chunk;
	chunk.buffer = (u8 *)UI_malloc(ctx, size);
	chunk.hash = hash;
	chunk.size = size;
	memset(chunk.buffer, 0, size);
	ctx->data_chunks.push_back(chunk);
	if (created_new) *created_new = true;

	return chunk;
}
//void UI_free_block_data(UI_Context *ctx, UI_Block* block) {
//	if (block->data.buffer != nullptr)
//		UI_free(ctx, block->data.size, block->data.buffer);
//	block->data.buffer = 0;
//	block->data.size = 0;
//	block->data.block_key = 0;
//}

int UI_measure_text_upto_pixels(UI_Context *ctx, UI_Font *font, u16 size, char *buff, i16 up_to, i16 length = -1) {
    UI_assert(ctx != nullptr && "UI context *ctx is a null pointer!");
    UI_assert(ctx->initialized && "UI context wasn't initialized!");
    UI_assert(font != nullptr && "UI_Font *font is a null pointer!");
    if (length == 0) return 0;

    int size_index = 0;
    assert(UI_font_size_exists(font, size, &size_index) && "Requested font size is not initialized!");

    int w = 0;
    int h = 0;
    int y0 = 0;
    int y1 = 0;
    if (length < 0)
        length = strlen(buff);
    size_t offset = font->char_count * size_index;
    int i = 0;
    for (i = 0; i < length; i++) {
        if (w >= up_to) break;
        if (buff[i] == '#') break;
        UI_Font_Glyph_Info* target_char = &font->char_info[offset + buff[i] - font->first];
        int local_y1 = target_char->y_off;
        int local_y0 = local_y1 - abs(target_char->y1 - target_char->y0);
        UI_assert(local_y0 <= local_y1);
        y0 = min(y0, local_y0);
        y1 = max(y1, local_y1);
        // h = max(h, target_char->ybearing);

        w += target_char->advance_x;
    }
    h =  font->true_sizes[size_index];
    // h = y1 - y0;
    UI_assert(h >= 0);
    return i;
}

v2 UI_measure_text(UI_Context *ctx, UI_Font *font, u16 size, char *buff, i16 length = -1) {
    UI_assert(ctx != nullptr && "UI context *ctx is a null pointer!");
    UI_assert(ctx->initialized && "UI context wasn't initialized!");
    UI_assert(font != nullptr && "UI_Font *font is a null pointer!");
    if (length == 0) return v2(0);

    int size_index = 0;
    assert(UI_font_size_exists(font, size, &size_index) && "Requested font size is not initialized!");

    int w = 0;
    int h = 0;
    int y0 = 0;
    int y1 = 0;
    if (length < 0)
        length = strlen(buff);
    size_t offset = font->char_count * size_index;
    for (int i = 0; i < length; i++) {
        if (buff[i] == '#') break;
        if (buff[i] == 0) break;
        UI_Font_Glyph_Info* target_char = &font->char_info[offset + buff[i] - font->first];
        int local_y1 = target_char->y_off;
        int local_y0 = local_y1 - abs(target_char->y1 - target_char->y0);
        UI_assert(local_y0 <= local_y1);
        y0 = min(y0, local_y0);
        y1 = max(y1, local_y1);
        // h = max(h, target_char->ybearing);

        w += target_char->advance_x;
    }
    h =  font->true_sizes[size_index];
    // h = y1 - y0;
    UI_assert(h >= 0);
    return v2(w, h);
}

v2 UI_push_text(
    UI_Context *ctx, 
    UI_Font *font, 
    UI_Color4 color, 
    u16 size, 
    v2 dest_point, 
    char *buff,
    f32 depth, 
    f32 h_offset,
    v2 clip_0 = v2(-1,-1),
    v2 clip_1 = v2(-1,-1),
    u16 length = 0){
        
    UI_assert(ctx != nullptr && "UI context *ctx is a null pointer!");
    UI_assert(ctx->initialized && "UI context wasn't initialized!");
    UI_assert(font != nullptr && "UI_Font *font is a null pointer!");

    int size_index = 0;
    assert(UI_font_size_exists(font, size, &size_index) && "Requested font size is not initialized!");

    int x = dest_point.x, y = dest_point.y;
    int start_x = x;
    int w = 0;
    int h = 0;
    int l = 0;
    if (length == 0)
        length = strlen(buff);
    size_t offset = font->char_count * size_index;
    for (int i = 0; i < length; i++) {
        //if (buff[i] == '#') break;
        UI_Font_Glyph_Info* target_char = &font->char_info[offset + buff[i] - font->first];
        UI_Rect src;
        src.x = target_char->x0;
        src.y = target_char->y0;
        src.w = target_char->x1 - target_char->x0;
        src.h = target_char->y1 - target_char->y0;
        
        UI_Rect dst;
        dst.x = x + target_char->x_off;
        dst.y = y - target_char->y_off + h_offset;
        dst.w = target_char->x1 - target_char->x0;
        dst.h = target_char->y1 - target_char->y0;
        h =  font->true_sizes[size_index];

		UI_Vertex vertex = { 0 };
        vertex.dst_p0 = v2(dst.x, dst.y);
        vertex.dst_p1 = vertex.dst_p0 + v2(dst.w, dst.h);
        vertex.src_p0 = v2(src.x, src.y);
        vertex.src_p1 = vertex.src_p0 + v2(src.w, src.h);
        vertex.texture_id = font->texture_handle;
        vertex.depth = depth;
        vertex.softness = 0;
        vertex.rotation = 0;
        vertex.border_size = 0;
        vertex.roundness = v4(0.0f);
        vertex.colors[0] = UI_u32_to_v4(color.c[0]);
        vertex.colors[1] = UI_u32_to_v4(color.c[1]);
        vertex.colors[2] = UI_u32_to_v4(color.c[2]);
        vertex.colors[3] = UI_u32_to_v4(color.c[3]);
        vertex.clp_p0 = clip_0;
        vertex.clp_p1 = clip_1;
        vertex.ui_block = -1;
		vertex.flags |= UI_Vertex_Flags_lcd;

        UI_push_vertex(ctx, vertex);

        x += target_char->advance_x;
        l += target_char->advance_x;
    }
    w = l;
    return v2(w, h);
}

/**
 * @param UI_Context *ctx : pointer to a UI_Context struct allocated by the user in memory.
 * **/
UI_Context * UI_init_context() {
    UI_Context *ctx = (UI_Context *)malloc(sizeof(UI_Context));
    UI_assert(ctx != nullptr);
    // TODO(Wassim): We REALLY need an arena allocator, all this binary tree is run by pointers
    // this is just a placeholder hack that should be changed later!
	ctx->buffers[0].init_null();        ctx->buffers[0].reserve(1000);
	ctx->buffers[1].init_null();        ctx->buffers[1].reserve(1000);
    ctx->parents.init_null();       	ctx->parents.reserve(1000);
    ctx->fonts.init_null();         	ctx->fonts.reserve(10);
	ctx->data_chunks.init_null();		ctx->data_chunks.reserve(1000);
	ctx->blocks_hit_test.init_null();	ctx->blocks_hit_test.reserve(1000);
    ctx->hashes.init_null();           
    ctx->inputs.init_null();
    ctx->vertices.init_null();
    for (int i = 0; i < UI_MAX_TEXTURES; i++)
        ctx->textures[i] = 0;
    
    FT_Init_FreeType(&ctx->ft_lib);
    FT_Library_SetLcdFilter(ctx->ft_lib, FT_LCD_FILTER_LIGHT);

	ctx->strings.buffer = (char*)malloc(1024 * 10);
	ctx->strings.count = 0;
	ctx->strings.capacity = 1024 * 10;
	ctx->buffer_index = 0;
    ctx->frame_id = 0;
    ctx->backend = UI_Render_Backend_Type::None;
    ctx->viewport = v2(0,0);
    ctx->backend_context = nullptr;
    ctx->vertices.init_null();
	ctx->debug.last_block_count = 0;
	ctx->debug.allocated_bytes = 0;
	ctx->debug.last_vertex_count = 0;
	ctx->string_storage_size = 0;
	ctx->string_storage_capacity = 1 << 20;
	ctx->string_storage = (char*)malloc(ctx->string_storage_capacity);

    ctx->initialized = true;
    return ctx;
}

//void UI_push_ID_(UI_Context *ctx, char* label) {
//	UI_assert(ctx != nullptr);
//	ctx->hashes.push_back((i64)UI_hash_djb2(ctx, label));
//}
//void UI_push_ID_(UI_Context *ctx, void* pointer) {
//	UI_assert(ctx != nullptr);
//	ctx->hashes.push_back((i64)pointer);
//}
void UI_push_hash(UI_Context *ctx, u32 key) {
	UI_assert(ctx != nullptr);
	ctx->hashes.push_back(key);
}
void UI_pop_hash(UI_Context *ctx) {
	UI_assert(ctx != nullptr);
	UI_assert(ctx->hashes.count > 0 && "UI popped hash one too many times!"); 
	ctx->hashes.pop_back();
}

int UI_push_parent(UI_Context *ctx, UI_Block *parent) {
    UI_assert(ctx != nullptr);
    UI_assert(parent != nullptr);
    ctx->parents.push_back(parent);
    return 0;
}

int UI_pop_parent(UI_Context *ctx) {
    UI_assert(ctx != nullptr);
    // you shouldn't be able to pop the root!
    UI_assert(ctx->parents.count > 0 && "UI popped parent one too many times!"); 
    ctx->parents.pop_back();
    return 0;
}

//u32 UI_key_from_string(String8 string) {
//    return UI_hash_djb2(string.data);
//}
//
//u32 UI_key_from_cstring(char *string) {
//    char* new_string = string;
//    int len = strlen(string);
//    int i = 0;
//    while(string[i] != '#') {
//        i++;
//        if (i == len) {
//            i = 0;
//            break;
//        }
//    }
//    return UI_hash_djb2(string + i);
//}

UI_Input *UI_get_input_of_block(UI_Context *ctx, UI_Block *block) {
    for (int i = 0; i < ctx->inputs.count; i++) {
        if (ctx->inputs[i].block_key == block->hash)
        return &ctx->inputs[i];
    }
    return nullptr;
}

i64 UI_generate_unique_id(UI_Context *ctx) {
    i64 hash_value = 0;
    for (int i = 0; i < ctx->hashes.count; i++) {
        hash_value = hash_value * 3.1 + ctx->hashes[i];
    }
    return hash_value;
}


bool UI_point_in_rect(v2 p0, v2 p1, v2 point) {
    v2 rect_min = v2(min(p0.x, p1.x), min(p0.y, p1.y));
    v2 rect_max = v2(max(p0.x, p1.x), max(p0.y, p1.y));
    return (point.x >= rect_min.x && 
            point.x <= rect_max.x && 
            point.y >= rect_min.y && 
            point.y <= rect_max.y);
}

UI_Edge UI_point_on_rect(v2 p0, v2 p1, v2 q, float md) {
    v2 r_min = v2(min(p0.x, p1.x), min(p0.y, p1.y));
    v2 r_max = v2(max(p0.x, p1.x), max(p0.y, p1.y));
    float dl = q.x - r_min.x;
    float dr = r_max.x - q.x;
    float dt = q.y - r_min.y;
    float db = r_max.y - q.y;
    return dl >= -md && dl <= md && q.y >= r_min.y && q.y <= r_max.y ? UI_Edge_left   :
           dr >= -md && dr <= md && q.y >= r_min.y && q.y <= r_max.y ? UI_Edge_right  :
           dt >= -md && dt <= md && q.x >= r_min.x && q.x <= r_max.x ? UI_Edge_top    :
           db >= -md && db <= md && q.x >= r_min.x && q.x <= r_max.x ? UI_Edge_bottom : UI_Edge_none;
}


UI_Block *UI_push_block(UI_Context *ctx, UI_Block* parent) {
    UI_assert(ctx != nullptr);
    UI_Block *result = nullptr;
	auto frame = UI_get_current_frame_buffer(ctx);

	UI_Block new_block = { 0 };
	result = frame->push_back(new_block);

	result->parent = parent;
	result->frame_created = ctx->frame_id;
	result->style.softness = 1.5; // defaults to 1 because everything looks better with it.

    // family matters
	if (parent) {
        UI_assert(result->parent != nullptr);   // must not be null if it's on that list!
        result->prev = result->parent->last;    // set the last child of parent as prev sibling
        result->parent->last = result;          // make this the parent's last child
        if (result->parent->first == nullptr) {
            result->parent->first = result;     // if parent has no first child, set this as first
        }
		result->depth_level = result->parent->depth_level + 1;
    }
    if (result->prev != nullptr)
        result->prev->next = result;        // if it has a prev, link em: set that prev's next to this.

    return result;
}

UI_Block *UI_push_block(UI_Context *ctx) {
	UI_assert(ctx != nullptr);
	UI_Block *parent = 0;
	if (ctx->parents.count > 0)
		parent = ctx->parents.back();

	return UI_push_block(ctx, parent);
}

// TODO(Wassim): we now just take the ms frame time from the game but this should
//               be part of platform code at some point and the UI lib should take
//               its own measurements.
void UI_begin_frame(UI_Context *ctx, u64 ms_since_last_frame) {
    UI_assert(ctx != nullptr && "UI context *ctx is a null pointer!");
    UI_assert(ctx->initialized && "UI context wasn't initialized!");
    UI_assert(ctx->parents.count == 0);
    // Manage previous frame://///////////////////
    ctx->inputs.reset_count();
    v2 mouse = UI_get_mouse();
    v2 mouse_delta = UI_get_mouse_delta();
    ctx->want_capture_keyboard = false;
    ctx->want_capture_mouse = false;
    ctx->last_frame_time = ms_since_last_frame;
    bool touched_a_block = false;
    i32 current_focused_block_id = 0;
	int freed = 0;


    // New frame://////////////////////////////////
    //UI_set_cursor(Cursor_Type_arrow);
	ctx->buffer_index = 1 - ctx->buffer_index;
    ctx->frame_id++;
    ctx->unique_counter = 0;
	ctx->strings.count = 0;
	UI_get_current_frame_buffer(ctx)->count = 0;

    // Reset the tree
    ctx->viewport = _v2(get_client_size());
    ctx->vertices.reset_count();
}

void UI_preorder_traversal(
        UI_Block* block, 
        Axis2 axis, 
        void (*callback)(UI_Block*, Axis2)) {
    if (block == nullptr) return; 
    // visit the current node and do stuff to it
    callback(block, axis);
    // traverse all nodes in the subtrees rooted at the siblings
    UI_Block* sibling = block->first;
    while (sibling != nullptr) {
        UI_preorder_traversal(sibling, axis, callback);
        sibling = sibling->next;
    }
}

void UI_postorder_traversal(
        UI_Block* block, 
        Axis2 axis, 
        void (*callback)(UI_Block*, Axis2)) {
    if (block == nullptr) return; 
    // traverse all nodes in the subtrees rooted at the siblings
    UI_Block* child = block->first;
    while (child != nullptr) {
        UI_postorder_traversal(child, axis, callback);
        child = child->next;
    }
    // visit the current node and do stuff to it
    callback(block, axis);
}

void UI_layout_solve_standalone_sizes(UI_Context* ctx, Axis2 axis) {
    //order is irrelevant here, so we are going lineary
	auto buffer = UI_get_current_frame_buffer(ctx);
	for (int i = 0; i < buffer->count; i++) {
		UI_Block *block = &buffer->data[i];
        UI_Size* semantic_size = &block->style.size[axis];
        switch (semantic_size->type)
        {
        case UI_Size_t::pixels:
            block->size.e[axis] = semantic_size->value;
            break;
        case UI_Size_t::text_content:
            UI_Font *font = block->style.font;
            u32     size =  block->style.font_size;
            UI_assert(font != nullptr);
            block->size.e[axis] = UI_measure_text(ctx, font, size, block->string.data).e[axis];
            break;
        }
    }
}

void UI_callback_bring_to_front(UI_Block* block, Axis2 axis/*redundant*/) {
	UI_assert(block != nullptr);
	block->depth_level += 100;
	block->clip_rect = UI_Rect(0,0,-1,-1);
}

void UI_callback_solve_upwards_sizes(UI_Block* block, Axis2 axis) {
    UI_assert(block != nullptr);
    if (block->style.size[axis].type != UI_Size_t::percent_of_parent) return;
    if (block->style.size[axis].value == -1) return; //skip those because they are custom-handled by the frontend
    UI_Size* semantic_size = &block->style.size[axis];
    UI_Block *parent = block->parent;
    if (parent == nullptr) return;
    while (parent->style.size[axis].type == UI_Size_t::sum_of_children) {
        parent = parent->parent;
        UI_assert(parent != 0);
    }
    UI_assert(semantic_size->value >= 0 && semantic_size->value <= 1 && "percentage in 0 to 1 flaoting point! or -1 for autofill");
    block->size.e[axis] = 
        (parent->size.e[axis] - parent->style.layout.padding.e[axis] * 2) * semantic_size->value;
    parent->layout_data[axis] += semantic_size->value;
}

void UI_callback_solve_downwards_sizes(UI_Block* block, Axis2 axis) {
    UI_assert(block != nullptr);
    if (block->style.size[axis].type != UI_Size_t::sum_of_children) return;
    UI_Block* sibling = block->first;
    if (sibling == 0) return; // has no children
    // UI_assert(sibling != 0);
    f32 sum = 0;
    while(sibling != nullptr) {
		// this assert might trigger if parent is sum of children 
		// and child is percent of parent, those conflict
		// NOTE: skipping children that have absolute positions
		if (sibling->style.position[axis].type == UI_Position_t::automatic)
		{
			UI_assert(sibling->size.e[axis] >= 0);
			if (axis == block->style.layout.axis)
				sum += sibling->size.e[axis] + block->style.layout.spacing.e[axis];
			else
				sum = max(sum, sibling->size.e[axis]);
		}
        sibling = sibling->next;
    }
	if (axis == block->style.layout.axis)
		sum -= block->style.layout.spacing.e[axis];
    sum += block->style.layout.padding.e[axis] * 2;
    block->size.e[axis] = sum;
}

void UI_callback_solve_violations(UI_Block* block, Axis2 axis) {
    UI_assert(block != nullptr);
    if (block->style.layout.axis != axis) return;
    // if (block->computed_size.e[axis] <= parent->computed_size.e[axis]) return;
    // TODO(Wassim): return here also if parent is scrollable
    // Calculate the total size of all children blocks in the given axis
    f32 total_child_size = 0;
    f32 proportional_count = 0;
    for (UI_Block* child = block->first; child != nullptr; child = child->next) {
        total_child_size += child->size.e[axis];
        child->interim_size.e[axis] = child->size.e[axis];
        proportional_count += (1 - child->style.size[axis].strictness);
    }
    f32 diff = total_child_size - block->size.e[axis] + block->style.layout.padding.e[axis];
    if (diff <= 0) return;
    int n = 100;
    // Adjust the size of each child block proportionally
    for (int iteration = 0; iteration < n; iteration++) {
        total_child_size = 0;
        for (UI_Block* child = block->first; child != nullptr; child = child->next) {
            if (child->size.e[axis] > 
                child->style.size[axis].strictness * child->interim_size.e[axis])
                child->size.e[axis] -= diff / proportional_count / 10;
            total_child_size += child->size.e[axis] + block->style.layout.spacing.e[axis];
        }
        diff = total_child_size - block->size.e[axis] + block->style.layout.padding.e[axis];
        if (diff < 0) break;
    }
    for (UI_Block* child = block->first; child != nullptr; child = child->next) {
        for(Axis2 axis = (Axis2)0; axis < axis_count; axis = (Axis2)((int)axis + 1)) {
            UI_Block* sibling = child->first;
            while (sibling != nullptr) {
                UI_preorder_traversal(sibling, axis, UI_callback_solve_upwards_sizes);
                sibling = sibling->next;
            }
        }
    }
}

void UI_callback_set_cursors(UI_Block* block, Axis2 axis) {
    UI_assert(block != nullptr);
    if (block->style.layout.align[axis] == align_start) {
        block->cursor.e[axis] = block->style.layout.padding.e[axis];
        return;
    } else if (block->style.layout.align[axis] == align_end) {
        block->cursor.e[axis] = 
            block->size.e[axis] - block->style.layout.padding.e[axis];
        return;
    }

    f32 total_child_size = 0;
    for (UI_Block* child = block->first; child != nullptr; child = child->next) {
        if (child->style.position[axis].type != UI_Position_t::automatic) continue;
        if (axis == block->style.layout.axis) 
            total_child_size += child->size.e[axis];
    }
    if (block->style.layout.align[axis] == align_center) {
        if (axis == block->style.layout.axis) 
            block->cursor.e[axis] = (block->size.e[axis] - total_child_size) * 0.5;
        else
			block->cursor.e[axis] = (block->size.e[axis]) * 0.5;
    }

}

struct UI_Intersection_Rect {
    v2 p[2];


    UI_Intersection_Rect() {
        p[0] = v2(0);
        p[1] = v2(0);
    }
    UI_Intersection_Rect(v2 p_0, v2 p_1) {
        p[0] = p_0;
        p[1] = p_1;
    }
};
inline static bool operator==(UI_Intersection_Rect A, UI_Intersection_Rect B) {
    return A.p[0].x == B.p[0].x &&  A.p[1].x == B.p[1].x && 
           A.p[0].y == B.p[0].y &&  A.p[1].y == B.p[1].y;
}

// UI_Intersection_Rect UI_intersect_rects(v2 A_0, v2 A_1, v2 B_0, v2 B_1) {
UI_Intersection_Rect UI_intersect_rects(UI_Intersection_Rect A, UI_Intersection_Rect B) {
    UI_Intersection_Rect result;

    v2 max_tl = v2(max(A.p[0].x, B.p[0].x), max(A.p[0].y, B.p[0].y));
    v2 min_br = v2(min(A.p[1].x, B.p[1].x), min(A.p[1].y, B.p[1].y));
    v2 size = min_br - max_tl;

    if (size.x > 0 && size.y > 0) {
        result.p[0] = max_tl;
        result.p[1] = max_tl + size;
    }

    return result;
}

void UI_set_clip_rects(UI_Context * ctx) {
	auto buffer = UI_get_current_frame_buffer(ctx);
	for (int i = 0; i < buffer->count; i++) {
		UI_Block* block = &buffer->data[i];
        UI_Block* current_parent = block;
        UI_Intersection_Rect main_rect = UI_Intersection_Rect(block->position, block->position + block->size);
        UI_Intersection_Rect result_rect = main_rect;
        block->clip_rect = UI_Rect(v2(-1), v2(-1)); 
        while(current_parent) {
            UI_Block* intersector = current_parent->style.clip_block;
            if (intersector != nullptr) {
                UI_Intersection_Rect current_intersection = 
                    UI_Intersection_Rect(intersector->position, intersector->position + intersector->size);
                result_rect = UI_intersect_rects(result_rect, current_intersection);
            }
            current_parent = current_parent->parent;
        }
        if (!(result_rect == main_rect)) {
            //UI_assert(result_rect.p[1].x >= 0 && result_rect.p[1].y >= 0)
            block->clip_rect = UI_Rect(result_rect.p[0], result_rect.p[1] - result_rect.p[0]);
        }
    }
}

void UI_callback_finalize_auto_positions(UI_Block* block, Axis2 axis) {
    UI_assert(block != nullptr);
    UI_Block* parent = block->parent;
    if (parent == nullptr) return; // account for the case of root which has no parent
    if (block->style.position[axis].type != UI_Position_t::automatic) return;
    // position is parents position plus the current cursor position
    if (parent->style.layout.align[axis] == align_end) {
        block->position.e[axis] = 
            parent->position.e[axis] + parent->cursor.e[axis] - block->size.e[axis];
    } else if (parent->style.layout.align[axis] == align_center) {
        if (axis == parent->style.layout.axis) 
            block->position.e[axis] = parent->position.e[axis] + parent->cursor.e[axis];
        else
            block->position.e[axis] = 
                parent->position.e[axis] + parent->cursor.e[axis] - block->size.e[axis] * 0.5;
    } else {
        block->position.e[axis] = parent->position.e[axis] + parent->cursor.e[axis];
    }
    // increment the cursor by the size of the block
    if (parent->style.layout.axis == axis)
        parent->cursor.e[axis] += 
            block->size.e[axis] + parent->style.layout.spacing.e[axis];
}

void UI_callback_finalize_custom_positions(UI_Block* block, Axis2 axis) {
    UI_assert(block != nullptr);
    if (block->style.position[axis].type == UI_Position_t::automatic) return;
    UI_Position* semantic_pos = &block->style.position[axis];
    switch (block->style.position[axis].type)
    {
    case UI_Position_t::absolute: {
        block->position.e[axis] = semantic_pos->value; 
        break;
        }
    case UI_Position_t::percent_of_parent: {
        UI_Block* parent = block->parent;
        UI_assert(parent != nullptr);
        block->position.e[axis] = 
            parent->position.e[axis] + parent->size.e[axis] * semantic_pos->value; 
        break; 
        }
    case UI_Position_t::pixels_from_parent: {
        UI_Block* parent = block->parent;
        UI_assert(parent != nullptr);
        block->position.e[axis] = 
            parent->position.e[axis] + semantic_pos->value; 
        break;
        }
    }
}

void UI_apply_layout(UI_Context *ctx) {
    UI_assert(ctx != nullptr && "UI context *ctx is a null pointer!");
    UI_assert(ctx->initialized && "UI context wasn't initialized!");
	auto buffer = UI_get_current_frame_buffer(ctx);
	for (int i = 0; i < buffer->count; i++) {
		UI_Block* root = &buffer->data[i];
		if (root->parent != nullptr) 
			continue;
		for (Axis2 axis = (Axis2)0; axis < axis_count; axis = (Axis2)((int)axis + 1)) {
			UI_layout_solve_standalone_sizes(ctx, axis);
			UI_preorder_traversal(root, axis, UI_callback_solve_upwards_sizes);
			UI_postorder_traversal(root, axis, UI_callback_solve_downwards_sizes);
			UI_preorder_traversal (root, axis, UI_callback_solve_violations);
			UI_preorder_traversal (root, axis, UI_callback_set_cursors);
			UI_postorder_traversal(root, axis, UI_callback_finalize_custom_positions);
			UI_preorder_traversal (root, axis, UI_callback_finalize_auto_positions);
			UI_postorder_traversal(root, axis, UI_callback_finalize_custom_positions);
		}
	}
	UI_set_clip_rects(ctx);
//	if (ctx->popup) {
//		UI_preorder_traversal(ctx->popup, (Axis2)0, UI_callback_bring_to_front);
//	}
}

void UI_end_frame(UI_Context *ctx) {
    UI_assert(ctx != nullptr && "UI context *ctx is a null pointer!");
    UI_assert(ctx->initialized && "UI context wasn't initialized!");
    UI_apply_layout(ctx);
    //ctx->parents.pop_back();
	ctx->blocks_hit_test.reset_count();
	auto buffer = UI_get_current_frame_buffer(ctx);
	for (int i = 0; i < buffer->count; i++) {
		UI_Block* block = &buffer->data[i];
		if (block->hash == 0) continue;
		if (!(block->flags & UI_Block_Flags_hit_test)) continue;
		bool in_block = UI_point_in_rect(block->position, block->position + block->size, UI_get_mouse());
		UI_Hit_Test_Item item;
		item.hash = block->hash;
		item.depth_level = block->depth_level;
		if (in_block) {
			ctx->blocks_hit_test.push_back(item);
		}
	}
	u32 max_depth = 0;
    UI_Hit_Test_Item foremost = { 0 };
	ctx->hit_test_result = foremost;
	for (int i = 0; i < ctx->blocks_hit_test.count; i++) {
		UI_Hit_Test_Item *current = &ctx->blocks_hit_test[i];
		if (current->depth_level > max_depth) {
			max_depth = current->depth_level;
			foremost = *current;
		}
	}
	ctx->hit_test_result = foremost;


    UI_assert(ctx->parents.count == 0 && "UI parent stack must be empty at the end of the frame!");
    UI_assert(ctx->hashes.count == 0 && "UI ID stack must be empty at the end of the frame!");
    if (ctx->want_capture_keyboard)
        UI_release_char_keys();
	ctx->string_storage_size = 0;
}

void UI_render(UI_Context *ctx) {
    UI_assert(ctx != nullptr && "UI context *ctx is a null pointer!");
    UI_assert(ctx->initialized && "UI context wasn't initialized!");
    UI_assert(ctx->backend != UI_Render_Backend_Type::None && "No render backend is initialized!");
    // prepare render data
	auto buffer = UI_get_current_frame_buffer(ctx);
	for (int i = 0; i < buffer->count; i++) {
		UI_Vertex vertex = { 0 };
		UI_Block *block = &buffer->data[i];
        // TODO(Wassim): vertex.texture_id =
        vertex.rotation = 0; 
        vertex.dst_p0 = block->position;
        vertex.dst_p1 = vertex.dst_p0 + v2(block->size.e[axis_x], 
                                           block->size.e[axis_y]);
        vertex.texture_id = -1;
        if (block->flags & UI_Block_Flags_draw_image) {
            vertex.texture_id = block->style.texture_handle;
            vertex.src_p0 = block->style.texture_uv;
            vertex.src_p1 = vertex.src_p0 + block->style.texture_src_size;
            vertex.rotation = block->style.texture_rotation;
        }
        vertex.roundness = block->style.roundness;
		vertex.softness = block->style.softness; 
		//vertex.softness = 1.5; 
		if (block->flags & UI_Block_Flags_draw_background) {
			vertex.colors[0] = UI_u32_to_v4(block->style.color[c_background].c[0]);
			vertex.colors[1] = UI_u32_to_v4(block->style.color[c_background].c[1]);
			vertex.colors[2] = UI_u32_to_v4(block->style.color[c_background].c[2]);
			vertex.colors[3] = UI_u32_to_v4(block->style.color[c_background].c[3]);
		}
		vertex.flags = 0;
		if (block->flags & UI_Block_Flags_render_srgb)
			vertex.flags |= UI_Vertex_Flags_srgb;
        
        vertex.depth = 1 - (f32)block->depth_level / ctx->parents.capacity;
        vertex.clp_p0 = v2(-1,-1);
        vertex.clp_p1 = v2(-1,-1);
		if (!(block->flags & UI_Block_Flags_no_clip))
        	if (block->clip_rect.w >= 0 && block->clip_rect.h >= 0) {
        	    vertex.clp_p0 = block->clip_rect.pos;
        	    vertex.clp_p1 = block->clip_rect.pos + block->clip_rect.size;
        	}
        vertex.ui_block = block->hash;
		UI_push_vertex(ctx, vertex);
        if (block->flags & UI_Block_Flags_draw_border) {
            vertex.depth = 1 - (f32)(block->depth_level + 0.5) / ctx->parents.capacity;
            UI_assert(block->style.border_size > 0 && "Block set to have a border with a border size of 0!")
            vertex.border_size = block->style.border_size;
			vertex.colors[0] = UI_u32_to_v4(block->style.color[c_border].c[0]);
			vertex.colors[1] = UI_u32_to_v4(block->style.color[c_border].c[1]);
			vertex.colors[2] = UI_u32_to_v4(block->style.color[c_border].c[2]);
			vertex.colors[3] = UI_u32_to_v4(block->style.color[c_border].c[3]);
            UI_push_vertex(ctx, vertex);
        }
        if (block->flags & UI_Block_Flags_draw_text) {
            v2 dim = UI_measure_text(ctx, block->style.font, block->style.font_size, block->string.data);
            UI_push_text(ctx,
                block->style.font, 
                block->style.color[c_text],
                block->style.font_size,
                block->position,
                block->string.data,
                1 - ((f32)block->depth_level + 3) / ctx->parents.capacity,
                dim.y - 1,
                vertex.clp_p0,
                vertex.clp_p1);
        }
    }

    ctx->debug.last_block_count = buffer->count;
    ctx->debug.last_vertex_count = ctx->vertices.count;

    // dispatch the appropriate backend to draw the vertices
    switch (ctx->backend)
    {
        case UI_Render_Backend_Type::D3D11: UI_d3d11_render(ctx); break;
    }
}






