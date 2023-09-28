#define UI_MAX_STRING_LEN 256
#define ARRAY_SIZE(x)  (sizeof(x) / sizeof((x)[0]))
#define UI_hash_formatted(__ctx__, ...) UI_hash_djb2(__ctx__, UI_sprintf(&__ctx__->strings, __VA_ARGS__))
#define UI_push_parent_defer(__ctx__, __parent__) for (int _i_ = 1, j = UI_push_parent(__ctx__, __parent__); _i_; _i_ = 0, UI_pop_parent(__ctx__))

UI_Theme *UI_get_theme() {
	static UI_Theme themes[UI_Theme_Count];
	static bool init = false;

	if (!init) {
		themes[UI_Theme_Cactus_Green].bg_main_0 = 		UI_color4_sld_u32(0x222920ff);
		themes[UI_Theme_Cactus_Green].bg_main_1 = 		UI_color4_sld_u32(0x374234ff);
		themes[UI_Theme_Cactus_Green].bg_main_1_d = 	UI_color4_sld_u32(0x444444FF);
		themes[UI_Theme_Cactus_Green].bg_main_2 = 		UI_color4_sld_u32(0x666e64ff);
		themes[UI_Theme_Cactus_Green].bg_main_2_d = 	UI_color4_sld_u32(0x555555ff);
		themes[UI_Theme_Cactus_Green].bg_main_3 = 		UI_color4_sld_u32(0x9ca19aff);
		themes[UI_Theme_Cactus_Green].bg_main_4 = 		UI_color4_sld_u32(0x2d362bff);
		themes[UI_Theme_Cactus_Green].bg_sub = 			UI_color4_sld_u32(0x222222FF);
		themes[UI_Theme_Cactus_Green].border_main = 	UI_color4_sld_u32(0x444444FF);
		themes[UI_Theme_Cactus_Green].separator = 		UI_color4_sld_v3(v3(.5));
		themes[UI_Theme_Cactus_Green].text_reg_main = 	UI_color4_sld_u32(0xFFFFFFFF);
		themes[UI_Theme_Cactus_Green].text_reg_main_d = UI_color4_sld_u32(0x888888FF);
		themes[UI_Theme_Cactus_Green].text_reg_light = 	UI_color4_sld_v3(v3(.95));
		themes[UI_Theme_Cactus_Green].text_reg_mid = 	UI_color4_sld_v3(v3(.7));
		themes[UI_Theme_Cactus_Green].text_error = 		UI_color4_sld_v3(v3(1, 0.3, 0));
		themes[UI_Theme_Cactus_Green].text_info = 		UI_color4_sld_v3(v3(1, 0.9, 0));
		themes[UI_Theme_Cactus_Green].text_header_0 = 	UI_color4_sld_v3(v3(1, 1, 0));
		themes[UI_Theme_Cactus_Green].text_header_1 = 	UI_color4_sld_v3(v3(1, .7, 0));
		themes[UI_Theme_Cactus_Green].text_header_2 = 	UI_color4_sld_v3(v3(.7, .7, 1));
		themes[UI_Theme_Cactus_Green].text_slider_0 = 	UI_color4_sld_u32(0xFFFFFFBB);
		themes[UI_Theme_Cactus_Green].text_slider_1 = 	UI_color4_sld_u32(0xFFFF00BB);
		themes[UI_Theme_Cactus_Green].text_slider_2 = 	UI_color4_sld_u32(0xFFFF00FF);
		themes[UI_Theme_Cactus_Green].pos_btn_0 = 		UI_color4_sld_u32(0x389c3cff);
		themes[UI_Theme_Cactus_Green].pos_btn_1 = 		UI_color4_sld_u32(0x4ec652ff);
		themes[UI_Theme_Cactus_Green].pos_btn_2 = 		UI_color4_sld_u32(0x2d7b2fff);
		themes[UI_Theme_Cactus_Green].neg_btn_0 = 		UI_color4_sld_u32(0x8a4e2eff);
		themes[UI_Theme_Cactus_Green].neg_btn_1 = 		UI_color4_sld_u32(0xaf643aff);
		themes[UI_Theme_Cactus_Green].neg_btn_2 = 		UI_color4_sld_u32(0x512e1bff);

		themes[UI_Theme_Dark_Blue].bg_main_0 = 			UI_color4_sld_u32(0x251c27ff);
		themes[UI_Theme_Dark_Blue].bg_main_1 = 			UI_color4_sld_u32(0x3e2f41ff);
		themes[UI_Theme_Dark_Blue].bg_main_1_d = 		UI_color4_sld_u32(0x444444FF);
		themes[UI_Theme_Dark_Blue].bg_main_2 = 			UI_color4_sld_u32(0x6c5d6fff);
		themes[UI_Theme_Dark_Blue].bg_main_2_d = 		UI_color4_sld_u32(0x555555ff);
		themes[UI_Theme_Dark_Blue].bg_main_3 = 			UI_color4_sld_u32(0x9e93a3ff);
		themes[UI_Theme_Dark_Blue].bg_main_4 = 			UI_color4_sld_u32(0x302432ff);
		themes[UI_Theme_Dark_Blue].bg_sub = 			UI_color4_sld_u32(0x222222FF);
		themes[UI_Theme_Dark_Blue].border_main = 		UI_color4_sld_u32(0x444444FF);
		themes[UI_Theme_Dark_Blue].separator = 			UI_color4_sld_v3(v3(.5));
		themes[UI_Theme_Dark_Blue].text_reg_main = 		UI_color4_sld_u32(0xFFFFFFFF);
		themes[UI_Theme_Dark_Blue].text_reg_main_d =	UI_color4_sld_u32(0x888888FF);
		themes[UI_Theme_Dark_Blue].text_reg_light = 	UI_color4_sld_v3(v3(.95));
		themes[UI_Theme_Dark_Blue].text_reg_mid = 		UI_color4_sld_v3(v3(.7));
		themes[UI_Theme_Dark_Blue].text_error = 		UI_color4_sld_v3(v3(1, 0.3, 0));
		themes[UI_Theme_Dark_Blue].text_info = 			UI_color4_sld_v3(v3(1, 0.9, 0));
		themes[UI_Theme_Dark_Blue].text_header_0 = 		UI_color4_sld_v3(v3(1, 1, 0));
		themes[UI_Theme_Dark_Blue].text_header_1 = 		UI_color4_sld_v3(v3(1, .7, 0));
		themes[UI_Theme_Dark_Blue].text_header_2 = 		UI_color4_sld_v3(v3(.7, .7, 1));
		themes[UI_Theme_Dark_Blue].text_slider_0 = 		UI_color4_sld_u32(0xFFFFFFBB);
		themes[UI_Theme_Dark_Blue].text_slider_1 = 		UI_color4_sld_u32(0xFFFF00BB);
		themes[UI_Theme_Dark_Blue].text_slider_2 = 		UI_color4_sld_u32(0xFFFF00FF);
		themes[UI_Theme_Dark_Blue].pos_btn_0 = 			UI_color4_sld_u32(0x2d7b2fff);
		themes[UI_Theme_Dark_Blue].pos_btn_1 = 			UI_color4_sld_u32(0x389c3cff);
		themes[UI_Theme_Dark_Blue].pos_btn_2 = 			UI_color4_sld_u32(0x1a481cff);
		themes[UI_Theme_Dark_Blue].neg_btn_0 = 			UI_color4_sld_u32(0x8a4e2eff);
		themes[UI_Theme_Dark_Blue].neg_btn_1 = 			UI_color4_sld_u32(0xaf643aff);
		themes[UI_Theme_Dark_Blue].neg_btn_2 = 			UI_color4_sld_u32(0x512e1bff);

		themes[UI_Theme_Light].bg_main_0 = 				UI_color4_sld_u32(0xffffffff);
		themes[UI_Theme_Light].bg_main_1 = 				UI_color4_sld_u32(0xe1d8e3ff);
		themes[UI_Theme_Light].bg_main_2 = 				UI_color4_sld_u32(0xcdbed0ff);
		themes[UI_Theme_Light].bg_main_3 = 				UI_color4_sld_u32(0x9f90a2ff);
		themes[UI_Theme_Light].bg_main_4 = 				UI_color4_sld_u32(0x675c6cff);
		themes[UI_Theme_Light].bg_main_1_d = 			UI_color4_sld_u32(0xbdbdbdFF);
		themes[UI_Theme_Light].bg_main_2_d = 			UI_color4_sld_u32(0x919191ff);
		themes[UI_Theme_Light].bg_sub = 				UI_color4_sld_u32(0xeeeeeeFF);
		themes[UI_Theme_Light].border_main = 			UI_color4_sld_u32(0xf1edf2FF);
		themes[UI_Theme_Light].separator = 				UI_color4_sld_v3(v3(.5));
		themes[UI_Theme_Light].text_reg_main = 			UI_color4_sld_u32(0x000000FF);
		themes[UI_Theme_Light].text_reg_main_d =		UI_color4_sld_u32(0x3d3d3dFF);
		themes[UI_Theme_Light].text_reg_light = 		UI_color4_sld_v3(v3(.05));
		themes[UI_Theme_Light].text_reg_mid = 			UI_color4_sld_v3(v3(.3));
		themes[UI_Theme_Light].text_error = 			UI_color4_sld_v3(v3(1, 0.3, 0));
		themes[UI_Theme_Light].text_info = 				UI_color4_sld_v3(v3(.5, .5, 0));
		themes[UI_Theme_Light].text_header_0 = 			UI_color4_sld_v3(v3(.5, .5, 0));
		themes[UI_Theme_Light].text_header_1 = 			UI_color4_sld_v3(v3(.8, .5, 0));
		themes[UI_Theme_Light].text_header_2 = 			UI_color4_sld_v3(v3(.3, .3, .6));
		themes[UI_Theme_Light].text_slider_0 = 			UI_color4_sld_u32(0x000000BB);
		themes[UI_Theme_Light].text_slider_1 = 			UI_color4_sld_u32(0x363d00BB);
		themes[UI_Theme_Light].text_slider_2 = 			UI_color4_sld_u32(0xFFFF00FF);
		themes[UI_Theme_Light].pos_btn_0 = 				UI_color4_sld_u32(0x56d55cff);
		themes[UI_Theme_Light].pos_btn_1 = 				UI_color4_sld_u32(0x49ab4bff);
		themes[UI_Theme_Light].pos_btn_2 = 				UI_color4_sld_u32(0x306b33ff);
		themes[UI_Theme_Light].neg_btn_0 = 				UI_color4_sld_u32(0xff9c6fff);
		themes[UI_Theme_Light].neg_btn_1 = 				UI_color4_sld_u32(0xfe7245ff);
		themes[UI_Theme_Light].neg_btn_2 = 				UI_color4_sld_u32(0x9b4b2eff);

		init = true;
	}
	return &themes[G->settings_selected_theme];
}
char *themes_str[] {"Cactus Green", "Dark Blue", "Light Blue"};

int UI_set_disabled(bool value) {
	G->gui_disabled_backup = G->gui_disabled;
	G->gui_disabled = value;
	return G->gui_disabled_backup;
}
void UI_reset_disabled() {
	G->gui_disabled = G->gui_disabled_backup;
}

#define UI_set_disabled_defer(__value__) for (int _i_ = 1, _j_ = UI_set_disabled((__value__)); _i_; _i_ = 0, UI_reset_disabled())

UI_Block* UI_bar(Axis2 axis) {
	UI_Context *ctx = G->ui;
	UI_Block *bar = UI_push_block(ctx);                             
	bar->style.layout.axis = axis;								  
	bar->style.size[axis_x] = { UI_Size_t::sum_of_children, 0, 1 }; 
	bar->style.size[axis_y] = { UI_Size_t::sum_of_children, 0, 1 }; 
	return bar;
}

UI_Block* UI_text(UI_Color4 color, UI_Font* font, u32 size, char* text, ...) {
	UI_Context *ctx = G->ui;
	char string[UI_MAX_STRING_LEN] = {0};
	va_list args;   
	va_start(args, text);   
	vsnprintf(string, ARRAY_SIZE(string), text, args); 
	va_end(args); 

	UI_Block* content = UI_push_block(ctx);
	content->style.size[axis_x] = { UI_Size_t::text_content, 0, 1 };
	content->style.size[axis_y] = { UI_Size_t::text_content, 0, 1 };
	content->style.position[axis_x] = { UI_Position_t::automatic, 0};
	content->style.position[axis_y] = { UI_Position_t::automatic, 0};
	content->style.font = font;
	content->style.color[c_text] = color;
	content->style.font_size = size;
	content->string = string;
	content->flags |= UI_Block_Flags_draw_text;
	content->hash = UI_hash_formatted(ctx, "%s#__UI_TEXT__", string);

	return content;
}

struct UI_Color4_Set {
	UI_Color4 base;
	UI_Color4 hot;
	UI_Color4 active;
	UI_Color4 disabled;
};

struct UI_Button_Style {
	UI_Color4_Set color_bg;
	UI_Color4_Set color_text;
	UI_Font* font;
	u32 font_size;
	v4 roundness;
	v2 size; //ignored if zero
};

UI_Color4 UI_animate_color_4(
	UI_Color4 current_color,
	bool hot, bool active, bool disabled,
	UI_Color4_Set colors) {
	return UI_animate_color_4(
		current_color, hot, active, disabled,
		colors.base,
		colors.hot,
		colors.active,
		colors.disabled);
}

bool UI_mouse_in_block(UI_Block* block) {
	UI_Context *ctx = G->ui;
	if (G->mouse_dragging) return false;
	bool result = ctx->hit_test_result.hash == block->hash;
	return result;
}

bool UI_mouse_in_block_force(UI_Block* block) {
	if (G->mouse_dragging) return false;
	return UI_point_in_rect(block->position, block->position + block->size, UI_get_mouse());
}

bool UI_button(UI_Button_Style *style, char* name, ...) {
	UI_Context *ctx = G->ui;
	char formatted_name[UI_MAX_STRING_LEN] = {0};
	va_list args;   
	va_start(args, name);   
	vsnprintf(formatted_name, ARRAY_SIZE(formatted_name), name, args); 
	va_end(args); 

	UI_Block *box = 0;
	UI_Block *text_block = 0;

	box = UI_push_block(ctx);
	UI_push_parent(ctx, box);
	text_block = UI_push_block(ctx);
	UI_pop_parent(ctx);

	box->hash = UI_hash_formatted(ctx, "%s#__BTN_BOX__", formatted_name);
	box->style.size[axis_x] = {UI_Size_t::sum_of_children, 0, 1};
	box->style.size[axis_y] = {UI_Size_t::pixels, f32(style->font_size * 2), 1};
	UI_Color4 color_background = box->style.color[c_background] = style->color_bg.base;
	box->style.layout.align[axis_x] = align_center;
	box->style.layout.align[axis_y] = align_center;
	//box->style.layout.padding = v2(5, 2);
	box->style.roundness = style->roundness;
	box->flags |= UI_Block_Flags_draw_background | UI_Block_Flags_hit_test;

	if (style->size.x > 0) box->style.size[axis_x] = {UI_Size_t::pixels, style->size.x, 1};
	if (style->size.y > 0) box->style.size[axis_y] = {UI_Size_t::pixels, style->size.y, 1};

//	if (strlen(formatted_name) < 2) 
//		box->style.size[axis_x] = {UI_Size_t::pixels, style->font_size * 2 - box->style.layout.padding.x, 1};

	text_block->string = formatted_name;
	text_block->flags |= UI_Block_Flags_draw_text;
	text_block->hash =  UI_hash_formatted(ctx, "%s__TEXT__", formatted_name);
	text_block->style.font = style->font;
	UI_Color4 color_text = text_block->style.color[c_text] = style->color_text.base;
	text_block->style.font_size = style->font_size;
	text_block->style.size[axis_x] = {UI_Size_t::text_content, 0, 1};
	text_block->style.size[axis_y] = {UI_Size_t::text_content, 0, 1};

	bool result = false;
	bool hot = false;
	bool active = false;
	bool disabled = G->gui_disabled;
	UI_Block* prev = UI_find_block(ctx, box->hash, UI_PREVIOUS);
	UI_Block* prev_txt = UI_find_block(ctx, text_block->hash, UI_PREVIOUS);

	if (prev && prev_txt) {
		color_background = prev->style.color[c_background];
		color_text = prev_txt->style.color[c_text];
		if (UI_mouse_in_block(prev) && !disabled) {
			active = keypress(MouseL);
			if (keydn(MouseL)) {
				G->mouse_dn_hash = prev->hash;
			}
			if (keyup(MouseL)) {
				int i = 0;
			}
			hot = true;
			if (G->mouse_dn_hash == prev->hash && keyup(MouseL))
				result = true;
		}
	}

	box->style.color[c_background] = UI_animate_color_4(color_background, hot, active, disabled, style->color_bg);
	text_block->style.color[c_text] = UI_animate_color_4(color_text, hot, active, disabled, style->color_text);
	G->tooltip_block = box;

	return result;
}

struct UI_Checkbox_Style {
	u32 box_dim;
	u32 roundness;
	u32 line_height;
	u32 font_size;
	UI_Font* font;
	UI_Color4_Set color_text;
	UI_Color4_Set color;
	f32 border;
};

bool UI_checkbox(UI_Checkbox_Style *style, bool *data, char* name, ...) {
	UI_Context *ctx = G->ui;
	char formatted_name[UI_MAX_STRING_LEN] = {0};
	u32 off_mask = 0xFFFFFF00;
    
	va_list args;   
	va_start(args, name);   
	vsnprintf(formatted_name, ARRAY_SIZE(formatted_name), name, args); 
	va_end(args); 

	i32 box_padding = 4;

	UI_Block *main_box = UI_push_block(ctx);
	main_box->hash = UI_hash_formatted(ctx, "%s__MAIN__", formatted_name);
	main_box->style.size[axis_x] = {UI_Size_t::sum_of_children, 0, 1};
	main_box->style.size[axis_y] = {UI_Size_t::pixels, f32(style->line_height), 1};
	main_box->style.layout.align[axis_y] = align_center;
	main_box->style.layout.spacing = v2(5);
	main_box->style.layout.axis = axis_x;
	main_box->flags |= UI_Block_Flags_hit_test;
	UI_push_parent(ctx, main_box);

	UI_Block *check_outer_box = UI_push_block(ctx);
	check_outer_box->hash = UI_hash_formatted(ctx, "%s__OUT__", formatted_name);
	check_outer_box->style.size[axis_x] = {UI_Size_t::pixels, f32(style->box_dim), 1};
	check_outer_box->style.size[axis_y] = {UI_Size_t::pixels,  f32(style->box_dim), 1};
	check_outer_box->flags |= UI_Block_Flags_draw_border;
	check_outer_box->style.border_size = style->border;
	UI_Color4 col_check_outer_box = check_outer_box->style.color[c_border] = style->color.base;
	check_outer_box->style.layout.align[axis_y] = align_center;
	check_outer_box->style.layout.align[axis_x] = align_center;
	check_outer_box->style.roundness = v4(style->roundness);
	UI_push_parent(ctx, check_outer_box);

	UI_Block *dot = UI_push_block(ctx);
	dot->hash = UI_hash_formatted(ctx, "%s__INN__", formatted_name);
	dot->style.size[axis_x] = {UI_Size_t::percent_of_parent, 0.6, 1};
	dot->style.size[axis_y] = {UI_Size_t::percent_of_parent, 0.6, 1};
	UI_Color4 col_dot = dot->style.color[c_background] = !(*data) ? (style->color.base & off_mask) : style->color.base;
	dot->flags |= UI_Block_Flags_draw_background;
	dot->style.roundness = v4(2);
	UI_pop_parent(ctx);

	UI_Block *label = UI_text(style->color_text.base, style->font, style->font_size, formatted_name);
	UI_Color4 col_label = label->style.color[c_text];
	UI_pop_parent(ctx);

	bool result = false;
	bool hot = false;
	bool active = false;
	bool disabled = G->gui_disabled;
	UI_Block* prv_main_box = UI_find_block(ctx, main_box->hash, UI_PREVIOUS);
	UI_Block* prv_check_outer_box = UI_find_block(ctx, check_outer_box->hash, UI_PREVIOUS);
	UI_Block* prv_dot = UI_find_block(ctx, dot->hash, UI_PREVIOUS);
	UI_Block* prv_label = UI_find_block(ctx, label->hash, UI_PREVIOUS);

	if (prv_main_box && prv_check_outer_box && prv_dot && prv_label) {
		col_check_outer_box = prv_check_outer_box->style.color[c_border];
		col_dot = prv_dot->style.color[c_background];
		col_label = prv_label->style.color[c_text];
		if (UI_mouse_in_block(prv_main_box)) {
			active = keypress(MouseL) && !G->gui_disabled;
			hot = true;
			if (keydn(MouseL))
				G->mouse_dn_hash = prv_main_box->hash;
			if (keyup(MouseL) && !G->gui_disabled && G->mouse_dn_hash == prv_main_box->hash) {
				result = true;
				*data = !(*data);
			}
		}
	}

	bool check_on = *data;

	check_outer_box->style.color[c_border] = UI_animate_color_4(col_check_outer_box, hot, active, disabled,style->color);
	label->style.color[c_text] = UI_animate_color_4(col_label, hot, active, disabled, style->color_text);
	dot->style.color[c_background] = 
		UI_animate_color_4(
			col_dot, hot, active, disabled,
			!(*data) ? (style->color.base 		& off_mask) : style->color.base,
			!(*data) ? (style->color.hot 		& off_mask) : style->color.hot,
			!(*data) ? (style->color.hot 		& off_mask) : style->color.hot,
			!(*data) ? (style->color.disabled 	& off_mask) : style->color.disabled);

	G->tooltip_block = main_box;

	return result;
}

u32 UI_hsv_to_rgb(f32 h, f32 s, f32 v, f32 a) {
	f32 r, g, b;

	if (s == 0.0f) { // gray
		r = g = b = v;
		return UI_v4_to_u32(v4(r, g, b, a));
	}

	h = fmod(h, 1.0f) / (60.0f / 360.0f);
	i32 i = (i32)h;
	f32 f = h - (f32)i;
	f32 p = v * (1.0f - s);
	f32 q = v * (1.0f - s * f);
	f32 t = v * (1.0f - s * (1.0f - f));

	switch (i) {
		case 0: 		 r = v; g = t; b = p; break;
		case 1: 		 r = q; g = v; b = p; break;
		case 2: 		 r = p; g = v; b = t; break;
		case 3: 		 r = p; g = q; b = v; break;
		case 4: 		 r = t; g = p; b = v; break;
		case 5: default: r = v; g = p; b = q; break;
	}
	return UI_v4_to_u32(v4(r, g, b, a));
}

v3 UI_rgb_to_hsv(u32 rgb) {
	v3 output;
	v4 input = UI_u32_to_v4(rgb);
	f32 r = input.r;
	f32 g = input.g;
	f32 b = input.b;
	f32 K = 0.f;
	if (g < b) {
		swap(f32, g, b);
		K = -1.f;
	}
	if (r < g) {
		swap(f32, r, g);
		K = -2.f / 6.f - K;
	}
	const f32 chroma = r - (g < b ? g : b);
	output.h = fabsf(K + (g - b) / (6.f * chroma + 1e-20f)); //h
	output.s = chroma / (r + 1e-20f); //s
	output.v = r; //v
	return output;
}

struct UI_Color_Picker_Data {
	bool open;
	u32 value;
	f32 h, s, v;
	f32 a;
};
struct UI_Color_Picker_Style {
	v2 button_size;
	char label[16];
	f32 roundness;
	UI_Color4_Set col_border;
	UI_Color4 col_bg;
	i32 font_size;
};

struct UI_Popup_Return {
	bool update;
	bool popup;
};
bool UI_color_picker(UI_Color_Picker_Style *style, u32 *color, bool with_alpha, u32 hash) {
	UI_Context *ctx = G->ui;
	UI_Block *button = 0;
	UI_Color_Picker_Data *metadata = 0;
	iv2 mouse = _iv2(UI_get_mouse());
	bool hot = false;
	bool active = false;
	bool disabled = G->gui_disabled;
	bool clicked = false;
	bool clicked_outside = false;
	bool popup_open = false;
	bool update_color = false;
	UI_Theme* theme = UI_get_theme();

	button = UI_push_block(ctx);
	button->hash = hash;
	button->style.size[axis_x] = { UI_Size_t::pixels, style->button_size.x, 1.f };
	button->style.size[axis_y] = { UI_Size_t::pixels, style->button_size.y, 1.f };
	button->flags = UI_Block_Flags_draw_background | UI_Block_Flags_draw_border | UI_Block_Flags_draw_border | UI_Block_Flags_hit_test;
	button->style.border_size = 2;
	button->style.color[c_background] = UI_color4_sld_u32(with_alpha ? *color : (*color & 0xFFFFFF00) | 0x000000FF);
	UI_Color4 color_border = button->style.color[c_border] = style->col_border.base;
	button->style.roundness = v4(style->roundness);
	button->style.layout.align[axis_x] = align_center;
	button->style.layout.align[axis_y] = align_center;
	UI_push_parent(ctx, button);
	u32 color_inv = UI_invert_u32_color((*color & 0xFFFFFF00) | 0x000000FF);
	color_inv = UI_lerp_u32(0xFFFFFFFF, color_inv, (((*color) & 0x000000FF)) / 255.f, false);
	UI_text(UI_color4_sld_u32(color_inv), G->ui_font, style->font_size, style->label);
	UI_pop_parent(ctx);

	UI_Block *prv = UI_find_block(ctx, button->hash, UI_PREVIOUS);
	if (prv) {
		metadata = (UI_Color_Picker_Data *)
			UI_find_else_allocate_data(ctx, button->hash, sizeof(UI_Color_Picker_Data)).buffer;
		hot = UI_mouse_in_block(prv) && !disabled;
		active = hot && keypress(MouseL);
		if (keydn(MouseL) && hot)
			G->mouse_dn_hash = prv->hash;
		clicked = hot && keyup(MouseL) && G->mouse_dn_hash == prv->hash;
		color_border = prv->style.color[c_border];
	}

	if (metadata && metadata->open) {
		if (!with_alpha) metadata->a = 1;
		popup_open = true;
		UI_Block *popup = UI_push_block(ctx, 0);
		UI_Block *hue_box = 0;
		UI_Block *alpha_box = 0;
		UI_Block *sv_box_0= 0;
		UI_Block *sv_box_1= 0;

		UI_Block *hue_picker = 0;
		UI_Block *alpha_picker = 0;
		UI_Block *sv_picker = 0;

		f32 sv_picker_size = 16;
		f32 box_size = 200;
		f32 hue_box_width = 30;

		popup->depth_level = button->depth_level + 200;
		popup->hash = UI_hash_djb2(ctx, "popup", button->hash);
		popup->flags |= UI_Block_Flags_hit_test;
		G->check_mouse_hashes.push_back(popup->hash);

		UI_push_parent_defer(ctx, popup) {
			UI_push_parent_defer(ctx, UI_bar(axis_x)) {
				UI_get_current_parent(ctx)->style.layout.spacing = v2(7);
				UI_Block *pop_prv = UI_find_block(ctx, popup->hash, UI_PREVIOUS);
				if (pop_prv) {
					if (!UI_mouse_in_block(pop_prv) && keydn(MouseL)) {
						clicked_outside = true;
					}
				}

				const u32 col_hues[6 + 1] = {
					0xFF0000FF,
					0xFFFF00FF,
					0x00FF00FF,
					0x00FFFFFF,
					0x0000FFFF,
					0xFF00FFFF,
					0xFF0000FF
				};
				u32 hue_c = UI_hsv_to_rgb(metadata->h, 1, 1, 1);

				UI_push_parent_defer(ctx, UI_bar(axis_y))
				{
					sv_box_0 = UI_get_current_parent(ctx);
					sv_box_0->hash = UI_hash_djb2(ctx, "sv_box", popup->hash);
					sv_box_0->style.size[axis_x] = { UI_Size_t::pixels, box_size, 1.0f };
					sv_box_0->style.size[axis_y] = { UI_Size_t::pixels, box_size, 1.0f };
					sv_box_0->style.color[c_background].c[0] = 0xFFFFFFFF;
					sv_box_0->style.color[c_background].c[1] = 0xFFFFFFFF;
					sv_box_0->style.color[c_background].c[2] = hue_c;
					sv_box_0->style.color[c_background].c[3] = hue_c;
					sv_box_0->style.roundness = v4(4);
					sv_box_0->flags |= UI_Block_Flags_draw_background
									| UI_Block_Flags_render_srgb;

					sv_picker = UI_push_block(ctx);

					sv_box_1 = UI_push_block(ctx, sv_box_0);
					sv_box_1->style.size[axis_x] = { UI_Size_t::percent_of_parent, 1, 1.0f };
					sv_box_1->style.size[axis_y] = { UI_Size_t::percent_of_parent, 1, 1.0f };
					sv_box_1->style.color[c_background].c[0] = 0x000000FF;
					sv_box_1->style.color[c_background].c[1] = 0;
					sv_box_1->style.color[c_background].c[2] = 0x000000FF;
					sv_box_1->style.color[c_background].c[3] = 0;
					sv_box_1->style.roundness = v4(4);
					sv_box_1->flags |= UI_Block_Flags_draw_background
						| UI_Block_Flags_render_srgb;
				}
				UI_push_parent_defer(ctx, UI_bar(axis_y))
				{
					hue_box = UI_get_current_parent(ctx);
					hue_box->style.roundness = v4(4);
					hue_box->style.size[axis_x] = { UI_Size_t::pixels, hue_box_width, 1.0f };
					hue_box->style.size[axis_y] = { UI_Size_t::sum_of_children, 1, 1.0f };
					hue_box->hash = UI_hash_djb2(ctx, "hue_box", popup->hash);
					for (int i = 0; i < 6; i++) {
						UI_Block *slice = UI_push_block(ctx);
						slice->style.size[axis_x] = { UI_Size_t::percent_of_parent, 1, 1.0f };
						slice->style.size[axis_y] = { UI_Size_t::pixels, box_size / 6, 1.0f };
						slice->style.color[c_background].c[0] = col_hues[i + 1];
						slice->style.color[c_background].c[1] = col_hues[i];
						slice->style.color[c_background].c[2] = col_hues[i + 1];
						slice->style.color[c_background].c[3] = col_hues[i];
						slice->flags |= UI_Block_Flags_draw_background
							| UI_Block_Flags_render_srgb;
						slice->style.softness = 0;
						if (i == 0) {
							slice->style.roundness[UI_Corner_tl] = 4;
							slice->style.roundness[UI_Corner_tr] = 4;
						}
						if (i == 5) {
							slice->style.roundness[UI_Corner_bl] = 4;
							slice->style.roundness[UI_Corner_br] = 4;
						}
					}
					hue_picker = UI_push_block(ctx);
				}
				if (with_alpha) {
					UI_push_parent_defer(ctx, UI_bar(axis_y))
					{
						alpha_box = UI_get_current_parent(ctx);
						alpha_box->style.roundness = v4(4);
						alpha_box->style.size[axis_x] = { UI_Size_t::pixels, hue_box_width, 1.0f };
						alpha_box->style.size[axis_y] = { UI_Size_t::pixels, box_size, 1.0f };
						alpha_box->hash = UI_hash_djb2(ctx, "alpha_box", popup->hash);
						alpha_box->style.color[c_background].c[0] = 0;
						alpha_box->style.color[c_background].c[1] = 0xFFFFFFFF;
						alpha_box->style.color[c_background].c[2] = 0;
						alpha_box->style.color[c_background].c[3] = 0xFFFFFFFF;
						alpha_box->flags |= UI_Block_Flags_draw_background
										 | UI_Block_Flags_draw_border
										 | UI_Block_Flags_render_srgb;
						alpha_box->style.color[c_border] = UI_color4_sld_u32(0xFFFFFFFF);
						alpha_box->style.border_size = 1;
				
						alpha_picker = UI_push_block(ctx);
					}
				}


				sv_picker->hash = UI_hash_djb2(ctx, "sv_picker", popup->hash);
				sv_picker->depth_level = sv_box_0->depth_level + 5;
				sv_picker->style.size[axis_x] = { UI_Size_t::pixels, sv_picker_size, 1.0f };
				sv_picker->style.size[axis_y] = { UI_Size_t::pixels, sv_picker_size, 1.0f };
				sv_picker->style.color[c_background] =
					UI_color4_sld_u32(UI_hsv_to_rgb(metadata->h, metadata->s, metadata->v, metadata->a));
				sv_picker->style.color[c_border] = UI_color4_sld_u32(0xFFFFFFFF);
				sv_picker->style.border_size = 2;
				sv_picker->style.roundness = v4(sv_picker_size / 2);
				sv_picker->flags |= UI_Block_Flags_draw_background
					| UI_Block_Flags_draw_border
					| UI_Block_Flags_render_srgb;
				UI_Block *prv_sv = UI_find_block(ctx, sv_box_0->hash, UI_PREVIOUS);
				if (prv_sv) {
					sv_picker->style.position[axis_x] = { UI_Position_t::pixels_from_parent, box_size*(metadata->s) - sv_picker_size / 2 };
					sv_picker->style.position[axis_y] = { UI_Position_t::pixels_from_parent, box_size*(1 - metadata->v) - sv_picker_size / 2 };
					if (UI_mouse_in_block_force(prv_sv)) {
						if (keypress(MouseL)) {
							//v2 sv_pos_mouse = UI_get_mouse() /*- v2(sv_picker_size / 2)*/;
							v2 sv_pos_mouse = (UI_get_mouse() - prv_sv->position) / prv_sv->size;
							sv_picker->style.position[axis_x] = { UI_Position_t::pixels_from_parent, sv_pos_mouse.x * box_size - sv_picker_size / 2 };
							sv_picker->style.position[axis_y] = { UI_Position_t::pixels_from_parent, sv_pos_mouse.y * box_size - sv_picker_size / 2 };
							sv_picker->style.position[axis_x].value = clamp(sv_picker->style.position[axis_x].value, - sv_picker_size / 2, box_size - sv_picker_size / 2);
							sv_picker->style.position[axis_y].value = clamp(sv_picker->style.position[axis_y].value, - sv_picker_size / 2, box_size - sv_picker_size / 2);
							metadata->s = sv_pos_mouse.x;
							metadata->v = 1 - sv_pos_mouse.y;
							update_color = true;
						}
					}
				}

				hue_picker->hash = UI_hash_djb2(ctx, "hue_picker", popup->hash);
				hue_picker->depth_level = hue_box->depth_level + 2;
				hue_picker->style.size[axis_x] = { UI_Size_t::pixels, hue_box_width + 4, 1.0f };
				hue_picker->style.size[axis_y] = { UI_Size_t::pixels, 8, 1.0f };
				hue_picker->style.color[c_background] = UI_color4_sld_u32(hue_c);
				hue_picker->style.color[c_border] = UI_color4_sld_u32(0xFFFFFFFF);
				hue_picker->style.border_size = 2;
				hue_picker->style.roundness = v4(2);
				hue_picker->flags |= UI_Block_Flags_draw_background
					| UI_Block_Flags_draw_border
					| UI_Block_Flags_render_srgb;
				UI_Block *ref_hue = UI_find_block(ctx, hue_box->hash, UI_PREVIOUS);
				if (ref_hue) {
					hue_picker->style.position[axis_x] = { UI_Position_t::pixels_from_parent, -2 };
					hue_picker->style.position[axis_y] = { UI_Position_t::pixels_from_parent, metadata->h * box_size - 4.f };
					if (UI_mouse_in_block_force(ref_hue)) {
						if (keypress(MouseL)) {
							v2 hue_picker_pos_mouse = (UI_get_mouse() - ref_hue->position) / ref_hue->size;
							hue_picker->style.position[axis_y].value = hue_picker_pos_mouse.y * box_size - 4.f;
							hue_picker->style.position[axis_y].value = clamp(hue_picker->style.position[axis_y].value, 0 , box_size);
							metadata->h = hue_picker_pos_mouse.y;
							update_color = true;
						}
					}
				}
				if (with_alpha) {
					alpha_picker->hash = UI_hash_djb2(ctx, "alpha_picker", popup->hash);
					alpha_picker->depth_level = alpha_box->depth_level + 2;
					alpha_picker->style.size[axis_x] = { UI_Size_t::pixels, hue_box_width + 4, 1.0f };
					alpha_picker->style.size[axis_y] = { UI_Size_t::pixels, 8, 1.0f };
					alpha_picker->style.color[c_background] = UI_color4_sld_v4(v4(1, 1, 1, metadata->a));
					alpha_picker->style.color[c_border] = UI_color4_sld_u32(0xFFFFFFFF);
					alpha_picker->style.border_size = 2;
					alpha_picker->style.roundness = v4(2);
					alpha_picker->flags |= UI_Block_Flags_draw_background
						| UI_Block_Flags_draw_border
						| UI_Block_Flags_render_srgb;
					UI_Block *ref_alp = UI_find_block(ctx, alpha_box->hash, UI_PREVIOUS);
					if (ref_alp) {
						alpha_picker->style.position[axis_x] = { UI_Position_t::pixels_from_parent, -2 };
						alpha_picker->style.position[axis_y] = { UI_Position_t::pixels_from_parent, (1-metadata->a) * box_size - 4.f };
						if (UI_mouse_in_block_force(ref_alp)) {
							if (keypress(MouseL)) {
								v2 alpha_picker_pos_mouse = (UI_get_mouse() - ref_alp->position) / ref_alp->size;
								alpha_picker->style.position[axis_y].value = alpha_picker_pos_mouse.y * box_size - 4.f;
								alpha_picker->style.position[axis_y].value = clamp(alpha_picker->style.position[axis_y].value, 0 , box_size);
								metadata->a = 1 - alpha_picker_pos_mouse.y;
								update_color = true;
							}
						}
					}
				}
			}
			if (update_color)
				*color = UI_hsv_to_rgb(metadata->h, metadata->s, metadata->v, metadata->a);

			v4 px = UI_u32_to_v4(*color);
			unsigned char r = px[0] * 255;
			unsigned char g = px[1] * 255;
			unsigned char b = px[2] * 255;
			unsigned char a = px[3] * 255;
			UI_Theme *theme = UI_get_theme();
			UI_push_parent_defer(ctx, UI_bar(axis_x)) {
				if (with_alpha)
					UI_text(theme->text_reg_main, G->ui_font, 13, "%i, %i, %i, %i", r, g, b, a);
				else
					UI_text(theme->text_reg_main, G->ui_font, 13, "%i, %i, %i", r, g, b);
			}
			if (with_alpha)
				UI_text(theme->text_info, G->ui_font, 13, "#%02X%02X%02X%02X", r, g, b, a);
			else
				UI_text(theme->text_info, G->ui_font, 13, "#%02X%02X%02X", r, g, b);

		}

		//hue_box->flags |= UI_bf_clip_children;

		popup->style.size[axis_x] = { UI_Size_t::sum_of_children, 1, 1.0f };
		popup->style.size[axis_y] = { UI_Size_t::sum_of_children, 1, 1.f };
		popup->style.color[c_background] = style->col_bg;
		popup->style.color[c_border] = theme->bg_sub;
		popup->style.border_size = 1;
		popup->style.roundness = v4(8);
		popup->style.layout.padding = v2(5);
		popup->style.layout.spacing = v2(7);
		popup->style.layout.axis = axis_y;
		if (prv) {
			popup->style.position[axis_x] = { UI_Position_t::absolute, prv->position.x };
			popup->style.position[axis_y] = { UI_Position_t::absolute, prv->position.y - box_size};
			UI_Block *ref_popup = UI_find_block(ctx, popup->hash, UI_PREVIOUS);
			if (ref_popup) {
				popup->style.position[axis_x].value = clamp(popup->style.position[axis_x].value, 0, WW - ref_popup->size.x);
				popup->style.position[axis_y].value = clamp(popup->style.position[axis_y].value, 0, WH - ref_popup->size.y);
				popup->flags |= UI_Block_Flags_draw_background | UI_Block_Flags_draw_border;
			}

		}
	}
	if (metadata && !metadata->open) {
		v3 hsv = UI_rgb_to_hsv(*color);
		metadata->h = hsv[0];
		metadata->s = hsv[1];
		metadata->v = hsv[2];
		metadata->a = (((*color) & 0x000000ff) >> 0) / 255.f;
	}

	if (metadata) {
		if (!metadata->open)	metadata->value = *color;
		if (clicked)			{ metadata->open = true; G->force_loop_frames += 2; }
		if (clicked_outside)	metadata->open = false;
	}

	button->style.color[c_border] = UI_animate_color_4(color_border, hot, active, disabled, style->col_border);

	G->tooltip_block = button;
		
	return popup_open;
}

f32 lin_to_log(f32 x, f32 min_val, f32 max_val) {
	f32 log_min = log(min_val);
	f32 log_max = log(max_val);
    
	f32 log_val = x * (log_max - log_min) + log_min;

	return exp(log_val);
}

f32 log_to_lin(f32 y, f32 min_val, f32 max_val) {
	f32 log_min = log(min_val);
	f32 log_max = log(max_val);
    
	f32 log_val_y = log(y);

	return (log_val_y - log_min) / (log_max - log_min);
}

struct UI_Slider_Style {
	f32 bar_long_axis;
	f32 bar_short_axis;
	f32 pad_min_size;
	UI_Color4_Set col_button;
	UI_Color4_Set col_background;
	char string[64];
	u32 font_size;
	UI_Font* font;
	UI_Color4_Set col_text;
	f32 roundness;

	bool pad_style;
	bool logarithmic;
	
	bool snap;
	f32 snap_value;
	f32 snap_range;

	bool mouse_right_to_reset;
	f32 reset_value;
};


bool UI_slider(UI_Slider_Style* style, Axis2 axis, f32* data, f32 min, f32 max, u32 hash) {
	UI_Context *ctx = G->ui;
	UI_Block *box = 0;
	UI_Block *button = 0;
	UI_Block *text = 0;
	UI_Color4 color_text;

	UI_push_hash(ctx, hash);
	box = UI_push_block(ctx);
	UI_push_parent_defer(ctx, box) {
		button = UI_push_block(ctx);
		if (strlen(style->string) > 0) {
			text = UI_text(style->col_text.base, style->font, style->font_size, style->string);
			color_text = text->style.color[c_text];
		}
	}
	UI_pop_hash(ctx);
	u32 padding = 1;
	bool hot = false;
	bool active = false;
	bool disabled = G->gui_disabled;

	u32 axis_l = (axis == axis_x) ? axis_x : axis_y;
	u32 axis_s = (axis == axis_x) ? axis_y : axis_x;

	box->hash = hash;
	box->style.size[axis_l] = { UI_Size_t::pixels, style->bar_long_axis, 1};
	box->style.size[axis_s] = { UI_Size_t::pixels, style->bar_short_axis, 1};
	UI_Color4 color_background = box->style.color[c_background] = style->col_background.base;
	box->flags |= UI_Block_Flags_draw_background | UI_Block_Flags_hit_test;
	box->style.layout.padding = v2(padding);
	box->style.roundness = style->roundness;
	box->style.layout.align[axis_x] = align_center;
	box->style.layout.align[axis_y] = align_center;

	UI_Color4 color_button = button->style.color[c_background] = style->col_button.base;
	button->hash = UI_hash_djb2(ctx, "btn", box->hash);
	button->style.size[axis_l] = { UI_Size_t::pixels, style->bar_long_axis / (max - min), 1 };
	button->style.size[axis_l].value = max(button->style.size[axis_l].value, style->pad_min_size);
	button->style.size[axis_s] = { UI_Size_t::pixels, style->bar_short_axis - padding * 2, 1 };
	button->style.roundness = style->roundness;

	f32 size_fracture = (button->style.size[axis_l].value) / style->bar_long_axis;

	UI_Block* ref = UI_find_block(ctx, box->hash, UI_PREVIOUS);
	UI_Block* ref_btn = UI_find_block(ctx, button->hash, UI_PREVIOUS);
	UI_Block* ref_txt = 0;
	if (text)
		ref_txt = UI_find_block(ctx, text->hash, UI_PREVIOUS);
	if (ref && ref_btn) {
		color_button = ref_btn->style.color[c_background];
		color_background = ref->style.color[c_background];
		if (ref_txt) color_text = ref_txt->style.color[c_text];
		if (UI_mouse_in_block(ref)) {
			hot = true && !disabled;
			active = hot && keypress(MouseL);
		}
	}

	if (hot) {
		if (!style->logarithmic)
			(*data) -= G->keys.scroll_y_diff * ((0.06 - 0.05 * keypress(Key_Shift)) * (min - max));
		if (style->mouse_right_to_reset && keyup(MouseR))
			*data = style->reset_value;
		*data = clamp(*data, min, max);
	}

	if (active && ref) {
		if (style->pad_style) {
			f32 mouse_rel = (UI_get_mouse()[axis_l] - ref->position[axis_l] - button->style.size[axis_l].value / 2) / (ref->size[axis_l] - button->style.size[axis_l].value);
			mouse_rel = clamp(mouse_rel, 0 + style->logarithmic * 0.000001, 1 + size_fracture);
			if (style->logarithmic)
				*data = lin_to_log(mouse_rel, min, max);
			else
				*data = (max - min) * mouse_rel;
		} else {
			f32 mouse_rel = (UI_get_mouse()[axis_l] - ref->position[axis_l]) / (ref->size[axis_l]);
			mouse_rel = clamp(mouse_rel, 0, 1);
			*data = min + (max - min) * mouse_rel;
			if (style->snap && !keypress(Key_Shift)) {
				if (abs(*data - style->snap_value) < style->snap_range) {
					*data = style->snap_value;
				}
			} 
		}
	} 
	if (style->pad_style) {
		if (style->logarithmic)
			button->style.position[axis_l] = { UI_Position_t::percent_of_parent, log_to_lin(*data , min, max) * (1 - size_fracture) };
		else
			button->style.position[axis_l] = { UI_Position_t::percent_of_parent, (*data - min) / (max - min) * (1 - size_fracture) };
		button->style.position[axis_s] = { UI_Position_t::pixels_from_parent, f32(padding) };
		button->style.position[axis_l].value = clamp(button->style.position[axis_l].value, 0, (1 - size_fracture));
	} else {
		button->style.size[axis_l] = { UI_Size_t::percent_of_parent, f32(*data - min) / (max - min)};
		button->style.position[axis_l] = { UI_Position_t::pixels_from_parent, f32(padding) };
		button->style.position[axis_s] = { UI_Position_t::pixels_from_parent, f32(padding)};
		button->style.size[axis_l].value = clamp(button->style.size[axis_l].value, 0, 1);
	}

	button->flags |= UI_Block_Flags_draw_background;

	button->style.color[c_background] = UI_animate_color_4(color_button, hot, active, disabled, style->col_button);
	box->style.color[c_background] = UI_animate_color_4(color_background, hot, active, disabled, style->col_background);
	if (text) {
		text->style.color[c_text] = UI_animate_color_4(color_text, hot, active, disabled, style->col_text);
	}

	G->tooltip_block = box;
	return active;
}

bool UI_slider_fill(UI_Slider_Style* style, Axis2 axis,f32* data, f32 min, f32 max, u32 hash) {
	UI_Context *ctx = G->ui;
	UI_Block *box = 0;
	UI_Block *button = 0;
	UI_Block *text = 0;
	UI_Color4 color_text;

	box = UI_push_block(ctx);
	UI_push_parent_defer(ctx, box) {
		button = UI_push_block(ctx);
		if (strlen(style->string) > 0) {
			text = UI_text(style->col_text.base, style->font, style->font_size, style->string);
			color_text = text->style.color[c_text];
		}
	}
	u32 padding = 1;
	bool hot = false;
	bool active = false;
	bool disabled = G->gui_disabled;

	u32 axis_l = (axis == axis_x) ? axis_x : axis_y;
	u32 axis_s = (axis == axis_x) ? axis_y : axis_x;

	box->hash = hash;
	box->style.size[axis_l] = { UI_Size_t::pixels, style->bar_long_axis, 1};
	box->style.size[axis_s] = { UI_Size_t::pixels, style->bar_short_axis, 1};
	UI_Color4 color_background = box->style.color[c_background] = style->col_background.base;
	box->flags |= UI_Block_Flags_draw_background | UI_Block_Flags_hit_test;
	box->style.layout.padding = v2(padding);
	box->style.roundness = style->roundness;
	box->style.layout.align[axis_x] = align_center;
	box->style.layout.align[axis_y] = align_center;

	button->hash = UI_hash_djb2(ctx, "btn", box->hash);
	UI_Color4 color_button = button->style.color[c_background] = style->col_button.base;
	button->style.size[axis_l] = { UI_Size_t::pixels, style->bar_long_axis / (max - min), 1 };
	button->style.size[axis_l].value = max(button->style.size[axis_l].value, style->pad_min_size);
	button->style.size[axis_s] = { UI_Size_t::pixels, style->bar_short_axis - padding * 2, 1 };
	button->style.roundness = style->roundness;

	UI_Block* ref = UI_find_block(ctx, box->hash, UI_PREVIOUS);
	UI_Block* ref_btn = UI_find_block(ctx, button->hash, UI_PREVIOUS);
	UI_Block* ref_txt = 0;
	if (text)
		ref_txt = UI_find_block(ctx, text->hash, UI_PREVIOUS);
	if (ref && ref_btn) {
		color_button = ref_btn->style.color[c_background];
		color_background = ref->style.color[c_background];
		if (ref_txt) color_text = ref_txt->style.color[c_text];
		if (UI_mouse_in_block(ref)) {
			hot = true && !disabled;
			active = hot && keypress(MouseL);
		}
	}

	if (active && ref) {
		f32 mouse_rel = (UI_get_mouse()[axis_l] - ref->position[axis_l]) / (ref->size[axis_l]);
		mouse_rel = clamp(mouse_rel, 0, 1);
		*data = min + (max - min) * mouse_rel;
	} 

	button->style.size[axis_l] = { UI_Size_t::percent_of_parent, f32(*data - min) / (max - min)};
	button->style.position[axis_l] = { UI_Position_t::pixels_from_parent, f32(padding) };
	button->style.position[axis_s] = { UI_Position_t::pixels_from_parent, f32(padding)};
	button->style.size[axis_l].value = clamp(button->style.size[axis_l].value, 0, 1);
	button->flags |= UI_Block_Flags_draw_background;

	button->style.color[c_background] = UI_animate_color_4(color_button, hot, active, disabled, style->col_button);
	box->style.color[c_background] = UI_animate_color_4(color_background, hot, active, disabled, style->col_background);
	if (text) {
		text->style.color[c_text] = UI_animate_color_4(color_text, hot, active, disabled, style->col_text);
	}

	G->tooltip_block = box;
	return active;
}

void UI_spacer_ver(f32 pixels = 0) {
	UI_Context *ctx = G->ui;
	UI_Block* spacer = UI_push_block(ctx);
	if (pixels == 0)
		spacer->style.size[axis_y] = { UI_Size_t::percent_of_parent, 1, 0 };
	else
		spacer->style.size[axis_y] = { UI_Size_t::pixels, pixels, 1 };
	spacer->style.size[axis_x] = { UI_Size_t::pixels, 1, 1 };
}
void UI_spacer_hor(f32 pixels = 0) {
	UI_Context *ctx = G->ui;
	UI_Block* spacer = UI_push_block(ctx); 
	if (pixels == 0)
		spacer->style.size[axis_x] = { UI_Size_t::percent_of_parent, 1, 0 };
	else
		spacer->style.size[axis_x] = { UI_Size_t::pixels, pixels, 1 };
	spacer->style.size[axis_y] = { UI_Size_t::pixels, 1, 1 };
}
void UI_separator(f32 thickness, UI_Color4 color) {
	UI_Context *ctx = G->ui;
	UI_Block* separator = UI_push_block(ctx); 
	separator->style.size[axis_x] = { UI_Size_t::percent_of_parent, 1, 0 };
	separator->style.size[axis_y] = { UI_Size_t::pixels, thickness, 0 };
	separator->style.color[c_background] = color;
	separator->flags |= UI_Block_Flags_draw_background;
}

void UI_tooltip(char* string, u32 time = 30) {
	UI_Context *ctx = G->ui;
	static int timer = 0;
	static iv2 last_mouse = _iv2(G->keys.Mouse);

	if (G->tooltip_block == 0) return;
	UI_Block* prv = UI_find_block(ctx, G->tooltip_block->hash, UI_PREVIOUS);
	if (prv == 0) return;

	UI_Theme *theme = UI_get_theme();

	if (UI_mouse_in_block(prv)) {
		if (last_mouse != _iv2(G->keys.Mouse)) {
			timer = 0;
			G->force_loop_frames += time;
		}
		last_mouse = _iv2(G->keys.Mouse);
		timer++;
		if (timer > time) {
			UI_Block* box = UI_push_block(ctx, 0);
			box->hash = 9999;
			box->flags |= UI_Block_Flags_draw_background | UI_Block_Flags_draw_border;
			box->style.color[c_background] = theme->bg_sub;
			box->style.color[c_border] = theme->border_main;
			box->style.layout.padding = v2(5);
			box->style.roundness = v4(3.f);
			box->style.size[axis_x] = {UI_Size_t::sum_of_children, 0, 1};
			box->style.size[axis_y] = {UI_Size_t::sum_of_children, 0, 1};
			box->style.position[axis_x] = { UI_Position_t::absolute, f32(last_mouse.x) };
			box->style.position[axis_y] = { UI_Position_t::absolute, f32(last_mouse.y + 20) };
			box->style.border_size = 1;
			box->depth_level += 500;
			UI_push_parent(ctx, box);
			UI_text(theme->text_reg_main, G->ui_font, 13, string);
			UI_pop_parent(ctx);
			UI_Block* ref = UI_find_block(ctx, box->hash, UI_PREVIOUS);
			if (ref) {
				box->style.position[axis_x].value = clamp(box->style.position[axis_x].value, 0, WW - ref->size.x);
				box->style.position[axis_y].value = clamp(box->style.position[axis_y].value, 0, WH - ref->size.y);
			}
		}
	}
	
	G->tooltip_block = 0;
}

struct UI_Combo_Style {
	v2 btn_size; //also combo item size
	v2 item_size; //also combo item size
	bool show_selected_item; // else shows label
	u32 font_size;
	UI_Color4_Set col_box;
	UI_Color4_Set col_item;
	UI_Color4_Set col_text;
	f32 roundness;
};

struct UI_Combo_Data {
	bool open;
};

struct UI_Combo_Return {
	bool changed;
	bool popup;
	UI_Block* bounding_box;
};

UI_Combo_Return UI_combo(UI_Combo_Style* style, char* label,  i32 *index, char**items, u32 item_count) {
	UI_Context *ctx = G->ui;
	UI_Block *box = UI_push_block(ctx);
	//UI_Block* arrow = 0;
	UI_Block* label_block = 0;
	UI_Block* container = 0;
	UI_Block* bounding_box = 0;
	bool changed = false;
	bool popup = false;
	box->hash = UI_hash_formatted(ctx, label);
	box->style.roundness = v4(style->roundness);
	box->flags |= UI_Block_Flags_draw_background | UI_Block_Flags_hit_test;
	UI_Color4 col_box = box->style.color[c_background] = style->col_box.base;
	
	UI_push_parent_defer(ctx, box) {
		//UI_push_parent_defer(ctx, UI_bar(axis_x)) {
			label_block = UI_text(style->col_text.base, G->ui_font, style->font_size, style->show_selected_item ? items[*index] : label);
		//}
//		UI_push_parent_defer(ctx, UI_bar(axis_x)) {
//			UI_get_current_parent(ctx)->style.layout.align[axis_x] = align_end;
//			arrow = UI_push_block(ctx);
//		}
	}

	bool hot = false;
	bool active = false;
	bool disabled = G->gui_disabled;
	bool in_container = false;

	UI_Combo_Data* metadata = (UI_Combo_Data*)UI_find_else_allocate_data(ctx, box->hash, sizeof(UI_Combo_Data)).buffer;
	UI_Block* prv_box = UI_find_block(ctx, box->hash, UI_PREVIOUS);
	if (prv_box)
		col_box = prv_box->style.color[c_background];

	if (metadata->open && prv_box) {
		popup = true;
		box->style.roundness[UI_Corner_bl] = 0;
		box->style.roundness[UI_Corner_br] = 0;
		box->style.softness = 0;

		container = UI_push_block(ctx, 0);
		container->hash = UI_hash_formatted(ctx, "%s_container", label);
		container->depth_level = box->depth_level + 200;
		container->style.position[axis_x] = { UI_Position_t::absolute, prv_box->position.x };
		container->style.position[axis_y] = { UI_Position_t::absolute, prv_box->position.y + prv_box->size.y };
		container->flags |= UI_Block_Flags_draw_border;
		container->style.border_size = 1;
		container->style.color[c_border] = UI_color4_sld_u32(0xFFFFFFFF);
		UI_push_parent_defer(ctx, container) {
			for (int i = 0; i < item_count; i++) {
				UI_Block* item = UI_push_block(ctx);
				UI_Block* item_label = 0;
				item->style.softness = 0;
				item->hash = UI_hash_formatted(ctx, "%s_%i", label, i);
				item->style.size[axis_x] = { UI_Size_t::pixels, style->item_size.x, 1 };
				item->style.size[axis_y] = { UI_Size_t::pixels, style->item_size.y, 1 };
				item->flags |= UI_Block_Flags_draw_background | UI_Block_Flags_hit_test;
				UI_Color4 item_col = item->style.color[c_background] = style->col_box.base;
				item->style.layout.padding = v2(5);

				if (i == item_count - 1) {
					item->style.roundness[UI_Corner_bl] = style->roundness;
					item->style.roundness[UI_Corner_br] = style->roundness;
				}
				UI_push_parent_defer(ctx, item) {
					item_label = UI_text(style->col_text.base, G->ui_font, style->font_size, items[i]);
				}
				UI_Color4 item_text_col = item_label->style.color[c_text];
				UI_Block* prv_item = UI_find_block(ctx, item->hash, UI_PREVIOUS);
				UI_Block* prv_label = UI_find_block(ctx, item_label->hash, UI_PREVIOUS);
				bool item_hot = false;
				bool item_active = false;
				if (prv_item) {
					item_col = prv_item->style.color[c_background];
					item_hot = UI_mouse_in_block(prv_item) && !disabled;
					if (item_hot) in_container = true;
					item_active = item_hot && keypress(MouseL);
					if (item_hot && keydn(MouseL)) {
						G->mouse_dn_hash = prv_item->hash;
					}
					if (item_hot && G->mouse_dn_hash == prv_item->hash && keyup(MouseL)) {
						*index = i;
						metadata->open = false;
						changed = true;
					}
				}
				if (prv_label) item_text_col = prv_label->style.color[c_text];
				item->style.color[c_background] = UI_animate_color_4(item_col,   	item_hot, item_active, disabled, style->col_item);
				item_label->style.color[c_text] = UI_animate_color_4(item_text_col,	item_hot, item_active, disabled, style->col_text);
			}
		}
		UI_Block *frame = UI_push_block(ctx, container);
		frame->depth_level = container->depth_level + 10;
		frame->hash = UI_hash_formatted(ctx, "%s_c_frame", label);
		frame->style.position[axis_x] = { UI_Position_t::pixels_from_parent, 0 };
		frame->style.position[axis_y] = { UI_Position_t::pixels_from_parent, -style->btn_size.y };
		frame->style.size[axis_x] = { UI_Size_t::pixels, style->btn_size.x, 1 };
		frame->style.size[axis_y] = { UI_Size_t::pixels, style->item_size.y * (item_count) + style->btn_size.y, 1 };
		frame->flags |= UI_Block_Flags_draw_border;
		frame->style.border_size = 1;
		frame->style.color[c_border] = UI_color4_sld_u32(0xFFFFFFBB);
		frame->style.roundness = v4(style->roundness);
		bounding_box = frame;
	}

	if (prv_box) {
		hot = UI_mouse_in_block(prv_box) && !disabled;
		active = hot && keypress(MouseL);
		if (hot && keydn(MouseL)) {
			G->mouse_dn_hash = prv_box->hash;
		}
		if (hot && G->mouse_dn_hash == prv_box->hash && keydn(MouseL)) { 
			metadata->open = !metadata->open; 
			G->force_loop_frames += 2;
		}
		if (!hot && !in_container && keydn(MouseL)) {
			metadata->open = false;
		}
	}

//	arrow->style.texture_handle = G->shapes_texture_id;
//	arrow->style.texture_uv = v2(0);
//	arrow->style.texture_src_size = v2(UI_ASSET_SHAPE_ARROW_WIDTH, UI_ASSET_SHAPE_ARROW_HEIGHT);
//	arrow->style.texture_rotation = 0;
//	arrow->hash = UI_hash_formatted(ctx, "%s_combo_arrow", label);
//	UI_Color4 col_arrow = arrow->style.color[c_background] = style->col_text.base;
//	arrow->flags |= UI_Block_Flags_draw_image;
//	UI_Block* prv_arrow = UI_find_block(ctx, box->hash, UI_PREVIOUS);
//	if (prv_arrow) col_arrow = prv_arrow->style.color[c_background];

	box->style.size[axis_x] = {UI_Size_t::pixels, style->btn_size.x, 1};
	box->style.size[axis_y] = {UI_Size_t::pixels, style->btn_size.y, 1};
	box->style.layout.align[axis_x] = align_center;
	box->style.layout.align[axis_y] = align_center;
	
	box->style.layout.padding = v2(5);

	UI_Color4 col_text = label_block->style.color[c_text];
	UI_Block* prv_text = UI_find_block(ctx, label_block->hash, UI_PREVIOUS);
	if (prv_text) col_text = prv_text->style.color[c_text];

	//arrow->style.color[c_background] = 	UI_animate_color_4(col_arrow, hot, active, disabled, style->col_text);
	box->style.color[c_background] = 	UI_animate_color_4(col_box,   hot, active, disabled, style->col_box);
	label_block->style.color[c_text] = 	UI_animate_color_4(col_text,  hot, active, disabled, style->col_text);

	return { changed, popup, bounding_box };
}

// more Cactus specific widgets:

struct UI_Popup_Data {
	bool open;
};

struct UI_Image_Edit_Style {
	UI_Button_Style button_style;
	UI_Slider_Style slider_style;
	UI_Checkbox_Style checkbox_style;
	UI_Color4 color_text;
	UI_Color4 color_frame_bg;
};

static void reset_image_edit();

bool UI_image_edit(UI_Image_Edit_Style *style, char* label) {
	UI_Context *ctx = G->ui;
	UI_Popup_Data *metadata = 0;
	iv2 mouse = _iv2(UI_get_mouse());
	bool hot = false;
	bool active = false;
	bool disabled = G->gui_disabled;
	bool clicked = false;
	bool clicked_outside = false;
	bool popup_open = false;
	static v2 spawn_pos;
	u32 hash = UI_hash_djb2(ctx, label);
	UI_Theme* theme = UI_get_theme();

	metadata = (UI_Popup_Data *)
		UI_find_else_allocate_data(ctx, hash, sizeof(UI_Popup_Data)).buffer;

	if (UI_button(&style->button_style, label)) {
		metadata->open = !metadata->open; 
		G->force_loop_frames += 2;
		spawn_pos = UI_get_mouse();
	}

	if (metadata && metadata->open) {
		popup_open = true;
		UI_Block *popup = UI_push_block(ctx, 0);
		popup->depth_level = UI_get_current_parent(ctx)->depth_level + 200;
		popup->hash = UI_hash_djb2(ctx, "popup", hash);
		popup->flags |= UI_Block_Flags_hit_test;
		G->check_mouse_hashes.push_back(popup->hash);
		UI_Block *pop_prv = UI_find_block(ctx, popup->hash, UI_PREVIOUS);
		if (pop_prv) {
			if (!UI_mouse_in_block_force(pop_prv) && keydn(MouseL)) {
				clicked_outside = true;
			}
		}
		f32 text_bar_frac = 0.45;
		style->slider_style.snap = true;
		style->slider_style.mouse_right_to_reset = true;
		UI_push_parent_defer(ctx, popup) {
			UI_Button_Style btn_style = style->button_style;
			btn_style.size.x = 120;
			UI_push_parent_defer(ctx, UI_bar(axis_x)) {
				auto bar = UI_get_current_parent(ctx);
				bar->style.layout.align[axis_x] = align_center;
				bar->style.size[axis_x] = { UI_Size_t::percent_of_parent, 1, 1 };
				if (UI_button(&btn_style, G->crop_mode ? "Exit crop mode" : "Enter crop mode"))
					G->crop_mode = !G->crop_mode;
				UI_spacer_hor(20);
				btn_style.color_bg.base = theme->neg_btn_0;
				btn_style.color_bg.hot = theme->neg_btn_1;
				btn_style.color_bg.active = theme->neg_btn_2;
				if (UI_button(&btn_style, "Reset edits"))
					reset_image_edit();
			}
			UI_push_parent_defer(ctx, UI_bar(axis_x)) {
				UI_get_current_parent(ctx)->style.layout.align[axis_y] = align_center;
				UI_push_parent_defer(ctx, UI_bar(axis_x)) {
					UI_get_current_parent(ctx)->style.size[axis_x] = {UI_Size_t::percent_of_parent, text_bar_frac, 0};
					UI_text(style->color_text, G->ui_font, style->button_style.font_size, "Hue:");
				}
				UI_push_parent_defer(ctx, UI_bar(axis_x)) {
					UI_get_current_parent(ctx)->style.layout.align[axis_x] = align_end;
					sprintf(style->slider_style.string, "%.0f", G->hue * 180.f / M_PI);
					style->slider_style.snap_value = 0;
					style->slider_style.snap_range =  2 * M_PI * 0.05;
					style->slider_style.reset_value = 0;
					UI_slider(&style->slider_style, axis_x, &G->hue, - 2 * M_PI, 2 * M_PI, UI_hash_formatted(ctx, "%s_hue", label));
					UI_tooltip("right click to reset", 20);
				}
			}
			UI_push_parent_defer(ctx, UI_bar(axis_x)) {
				UI_get_current_parent(ctx)->style.layout.align[axis_y] = align_center;
				UI_push_parent_defer(ctx, UI_bar(axis_x)) {
					UI_get_current_parent(ctx)->style.size[axis_x] = {UI_Size_t::percent_of_parent, text_bar_frac, 0};
					UI_text(style->color_text, G->ui_font, style->button_style.font_size, "Saturation:");
				}
				UI_push_parent_defer(ctx, UI_bar(axis_x)) {
					UI_get_current_parent(ctx)->style.layout.align[axis_x] = align_end;
					sprintf(style->slider_style.string, "%.2f", G->saturation);
					style->slider_style.snap_value = 1;
					style->slider_style.snap_range = 0.07;
					style->slider_style.reset_value = 1;
					UI_slider(&style->slider_style, axis_x, &G->saturation, 0, 3, UI_hash_formatted(ctx, "%s_saturation", label));
					UI_tooltip("right click to reset", 20);
				}
			}
			UI_push_parent_defer(ctx, UI_bar(axis_x)) {
				UI_get_current_parent(ctx)->style.layout.align[axis_y] = align_center;
				UI_push_parent_defer(ctx, UI_bar(axis_x)) {
					UI_get_current_parent(ctx)->style.size[axis_x] = {UI_Size_t::percent_of_parent, text_bar_frac, 0};
					UI_text(style->color_text, G->ui_font, style->button_style.font_size, "Contrast:");
				}
				UI_push_parent_defer(ctx, UI_bar(axis_x)) {
					UI_get_current_parent(ctx)->style.layout.align[axis_x] = align_end;
					sprintf(style->slider_style.string, "%.2f", G->contrast);
					style->slider_style.snap_value = 1;
					style->slider_style.snap_range = 0.07;
					style->slider_style.reset_value = 1;
					UI_slider(&style->slider_style, axis_x, &G->contrast, 0, 2, UI_hash_formatted(ctx, "%s_contrast", label));
					UI_tooltip("right click to reset", 20);
				}
			}
			UI_push_parent_defer(ctx, UI_bar(axis_x)) {
				UI_get_current_parent(ctx)->style.layout.align[axis_y] = align_center;
				UI_push_parent_defer(ctx, UI_bar(axis_x)) {
					UI_get_current_parent(ctx)->style.size[axis_x] = {UI_Size_t::percent_of_parent, text_bar_frac, 0};
					UI_text(style->color_text, G->ui_font, style->button_style.font_size, "Brightness:");
				}
				UI_push_parent_defer(ctx, UI_bar(axis_x)) {
					UI_get_current_parent(ctx)->style.layout.align[axis_x] = align_end;
					sprintf(style->slider_style.string, "%.2f", G->brightness);
					style->slider_style.snap_value = 0;
					style->slider_style.snap_range = 0.07;
					style->slider_style.reset_value = 0;
					UI_slider(&style->slider_style, axis_x, &G->brightness, -1, 1, UI_hash_formatted(ctx, "%s_brightness", label));
					UI_tooltip("right click to reset", 20);
				}
			}
			UI_push_parent_defer(ctx, UI_bar(axis_x)) {
				UI_get_current_parent(ctx)->style.layout.align[axis_y] = align_center;
				UI_push_parent_defer(ctx, UI_bar(axis_x)) {
					UI_get_current_parent(ctx)->style.size[axis_x] = {UI_Size_t::percent_of_parent, text_bar_frac, 0};
					UI_text(style->color_text, G->ui_font, style->button_style.font_size, "Gamma:");
				}
				UI_push_parent_defer(ctx, UI_bar(axis_x)) {
					UI_get_current_parent(ctx)->style.layout.align[axis_x] = align_end;
					sprintf(style->slider_style.string, "%.2f", G->gamma);
					style->slider_style.snap_value = 1;
					style->slider_style.snap_range = 0.07;
					style->slider_style.reset_value = 1;
					UI_slider(&style->slider_style, axis_x, &G->gamma, 0, 2, UI_hash_formatted(ctx, "%s_gamma", label));
					UI_tooltip("right click to reset", 20);
				}
			}
			UI_checkbox(&style->checkbox_style, &G->srgb, "Render base colors in sRGB");
			UI_checkbox(&style->checkbox_style,(bool*)(&G->do_blur), "Gaussian blur (GPU intensive)");
			bool prev_disabled = G->gui_disabled;
			if (!G->do_blur)
				G->gui_disabled = true;
			UI_push_parent_defer(ctx, UI_bar(axis_x)) {
				UI_get_current_parent(ctx)->style.layout.align[axis_y] = align_center;
				UI_push_parent_defer(ctx, UI_bar(axis_x)) {
					UI_get_current_parent(ctx)->style.size[axis_x] = {UI_Size_t::percent_of_parent, text_bar_frac, 0};
					UI_text(style->color_text, G->ui_font, style->button_style.font_size, "Blur samples:");
				}
				UI_push_parent_defer(ctx, UI_bar(axis_x)) {
					UI_get_current_parent(ctx)->style.layout.align[axis_x] = align_end;
					sprintf(style->slider_style.string, "%i", G->blur_samples);
					style->slider_style.snap_value = 32;
					style->slider_style.snap_range = 4;
					style->slider_style.reset_value = 32;
					float t_samples = G->blur_samples;
					UI_slider(&style->slider_style, axis_x, &t_samples, 2, 64, UI_hash_formatted(ctx, "%s_blur_samples", label));
					G->blur_samples = t_samples;
					UI_tooltip("right click to reset", 20);
				}
			}
			UI_push_parent_defer(ctx, UI_bar(axis_x)) {
				UI_get_current_parent(ctx)->style.layout.align[axis_y] = align_center;
				UI_push_parent_defer(ctx, UI_bar(axis_x)) {
					UI_get_current_parent(ctx)->style.size[axis_x] = {UI_Size_t::percent_of_parent, text_bar_frac, 0};
					UI_text(style->color_text, G->ui_font, style->button_style.font_size, "Blur scale:");
				}
				UI_push_parent_defer(ctx, UI_bar(axis_x)) {
					UI_get_current_parent(ctx)->style.layout.align[axis_x] = align_end;
					sprintf(style->slider_style.string, "%.5f", G->blur_scale);
					style->slider_style.snap_value = 0.001;
					style->slider_style.snap_range = 0.0001;
					style->slider_style.reset_value = 0.001;
					style->slider_style.logarithmic = true;
					UI_slider(&style->slider_style, axis_x, &G->blur_scale, 0, 0.005, UI_hash_formatted(ctx, "%s_blur_scale", label));
					UI_tooltip("right click to reset", 20);
				}
			}
			G->gui_disabled = prev_disabled;
		}
		popup->style.size[axis_x] = { UI_Size_t::pixels, 300, 1.0f };
		popup->style.size[axis_y] = { UI_Size_t::sum_of_children, 1, 1.f };
		popup->style.color[c_background] = style->color_frame_bg;
		popup->style.color[c_border] = theme->bg_sub;
		popup->style.border_size = 1;
		popup->style.roundness = v4(8);
		popup->style.layout.padding = v2(10);
		popup->style.layout.spacing = v2(7);
		popup->style.layout.axis = axis_y;
		popup->style.position[axis_x] = { UI_Position_t::absolute, 5 };
		popup->style.position[axis_y] = { UI_Position_t::absolute, WH - 120.f };
		UI_Block *ref_popup = UI_find_block(ctx, popup->hash, UI_PREVIOUS);
		if (ref_popup) {
			popup->style.position[axis_x].value = clamp(popup->style.position[axis_x].value, 0, WW - ref_popup->size.x - 5);
			popup->style.position[axis_y].value = clamp(popup->style.position[axis_y].value, 0, WH - ref_popup->size.y - 35);
			popup->flags |= UI_Block_Flags_draw_background | UI_Block_Flags_draw_border;
		}

	}

	if (metadata) {
		if (clicked)			{ metadata->open = true; G->force_loop_frames += 2; }
		if (clicked_outside)	metadata->open = false;
	}

	return popup_open;
}
struct UI_Histogram_Style {
	UI_Button_Style button_style;
	UI_Checkbox_Style checkbox_style;
	UI_Color4 color_text;
	UI_Color4 color_frame_bg;
};

struct UI_Histogram_Data {
	bool open;
};

bool UI_histogram(UI_Histogram_Style *style, char* label) {
	UI_Context *ctx = G->ui;
	UI_Histogram_Data *metadata = 0;
	iv2 mouse = _iv2(UI_get_mouse());
	bool hot = false;
	bool active = false;
	bool disabled = G->gui_disabled;
	bool clicked = false;
	bool clicked_outside = false;
	bool popup_open = false;
	static v2 spawn_pos;
	u32 hash = UI_hash_djb2(ctx, label);

	UI_Theme* theme = UI_get_theme();

	metadata = (UI_Histogram_Data *)UI_find_else_allocate_data(ctx, hash, sizeof(UI_Histogram_Data)).buffer;

	if (UI_button(&style->button_style, label)) {
		metadata->open = !metadata->open;
		G->force_loop_frames += 2;
		spawn_pos = UI_get_mouse();
	}

	if (G->settings_calculate_histograms) {
		if (!G->graphics.main_image.has_histo)
			UI_tooltip("no histogram loaded for this image, reload to calculate");
		else
			UI_tooltip("show original image's histogram");
	} else {
		UI_tooltip("this is disabled from settings. you can change it in 'config'");
	}
	G->histo_block = 0;
	if (metadata && metadata->open) {
		popup_open = true;
		UI_Block *popup = UI_push_block(ctx, 0);
		popup->depth_level = UI_get_current_parent(ctx)->depth_level + 200;
		popup->hash = UI_hash_djb2(ctx, "popup", hash);
		popup->flags |= UI_Block_Flags_hit_test;
		G->check_mouse_hashes.push_back(popup->hash);
		UI_Block *pop_prv = UI_find_block(ctx, popup->hash, UI_PREVIOUS);
		if (pop_prv) {
			if (!UI_mouse_in_block_force(pop_prv) && keydn(MouseL)) {
				clicked_outside = true;
			}
		}
		UI_push_parent_defer(ctx, popup) {
			UI_text(style->color_text, style->button_style.font, 10, 
			        "Use mouse wheel to adjust scale, right click to reset");
			UI_Block* hframe = UI_push_block(ctx);
			hframe->style.size[axis_x] = { UI_Size_t::pixels, 258, 1 };
			hframe->style.size[axis_y] = { UI_Size_t::pixels, 150, 1 };
			hframe->flags |= UI_Block_Flags_draw_border | UI_Block_Flags_draw_background;
			hframe->style.color[c_border] = style->color_text;
			hframe->style.color[c_background] = UI_color4_sld_u32(0x111111FF);
			hframe->style.border_size = 1;
			hframe->style.layout.align[axis_y] = align_end;
			hframe->style.layout.axis = axis_x;
			hframe->style.layout.spacing = v2(5);
			hframe->hash = UI_hash_djb2(ctx, label, hash);
			UI_Color4 col_r = UI_color4_sld_u32(0xFF0000DD);
			UI_Color4 col_g = UI_color4_sld_u32(0x00FF00DD);
			UI_Color4 col_b = UI_color4_sld_u32(0x0000FFDD);
			UI_Color4 col_t = UI_color4_sld_u32(0xFFFFFFDD);
			UI_Block* br = 0;
			UI_Block *ref_frame = UI_find_block(ctx, hframe->hash, UI_PREVIOUS);
			if (ref_frame) {
				if (UI_mouse_in_block_force(ref_frame)) {
					G->histo_max_edit *= 1 + G->keys.scroll_y_diff * 0.1 / (1 + G->settings_shiftslowmag * keypress(Key_Shift));
					G->histo_max_edit = max(G->histo_max_edit, 5);
					if (keyup(MouseR)) {
						G->histo_max_edit = G->histo_max;
					}
				}
			}
			G->histo_block = ref_frame;

			f32 height = 150 - 4;
			for (int i = 0; i < 256; i++) {
				G->p_histo_r[i] = v2(i, height - min(((f32)G->histo_r[i] / G->histo_max_edit) * height, height));
				G->p_histo_g[i] = v2(i, height - min(((f32)G->histo_g[i] / G->histo_max_edit) * height, height));
				G->p_histo_b[i] = v2(i, height - min(((f32)G->histo_b[i] / G->histo_max_edit) * height, height));
				G->p_histo_t[i] = v2(i, height - min(((f32)G->histo_t[i] / G->histo_max_edit) * height, height));
			}

			UI_push_parent_defer(ctx, UI_bar(axis_x)) {
				UI_get_current_parent(ctx)->style.layout.spacing = v2(5);
				UI_checkbox(&style->checkbox_style, &G->draw_histo_r, "red");
				UI_checkbox(&style->checkbox_style, &G->draw_histo_g, "green");
				UI_checkbox(&style->checkbox_style, &G->draw_histo_b, "blue");
				UI_checkbox(&style->checkbox_style, &G->draw_histo_t, "luminosity");
			}

			popup->style.size[axis_x] = { UI_Size_t::sum_of_children, 1, 1.0f };
			popup->style.size[axis_y] = { UI_Size_t::sum_of_children, 1, 1.f };
			popup->style.color[c_background] = style->color_frame_bg;
			popup->style.color[c_border] = theme->bg_sub;
			popup->style.border_size = 1;
			popup->style.roundness = v4(8);
			popup->style.layout.padding = v2(10);
			popup->style.layout.spacing = v2(7);
			popup->style.layout.axis = axis_y;
			popup->style.position[axis_x] = { UI_Position_t::absolute, 5 };
			popup->style.position[axis_y] = { UI_Position_t::absolute, f32(WH)};
			UI_Block *ref_popup = UI_find_block(ctx, popup->hash, UI_PREVIOUS);
			if (ref_popup) {
				popup->style.position[axis_x].value = clamp(popup->style.position[axis_x].value, 0, WW - ref_popup->size.x - 5);
				popup->style.position[axis_y].value = clamp(popup->style.position[axis_y].value, 0, WH - ref_popup->size.y - 35);
				popup->flags |= UI_Block_Flags_draw_background | UI_Block_Flags_draw_border;
			}
		}

	}

	if (metadata) {
		if (clicked)			{ metadata->open = true; G->force_loop_frames += 2; }
		if (clicked_outside)	metadata->open = false;
	}



	return popup_open;
}


