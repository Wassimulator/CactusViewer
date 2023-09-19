#pragma once
#include "ui_core.h"
void UI_init_platform_win32(UI_Context *ctx) {
	UI_assert(ctx != nullptr && "UI context was not initialized!");
	ctx->platform = UI_Platform_Type::Win32;
}
// placeholder for now until we write properly idependent platform code:
// #include "platform.cpp"
bool UI_is_keyup(int key) {
	return keyup(key);
}
bool UI_is_keydown(int key) {
	return keydn(key);
}
bool UI_is_keypressed(int key) {
	return keypress(key);
}
void UI_release_key(int key) {
	keyrelease(key);
}
inline v2 UI_get_mouse() {
	return G->keys.Mouse;
}
v2 UI_get_mouse_delta() {
	return G->keys.Mouse_rel;
}
char UI_get_char_input() {
	return get_char_input();
}
char UI_get_num_input() {
	return get_num_input();
}
void UI_release_char_keys() {
	release_char_keys();
}
void UI_set_cursor(int type) {
	SetCursor(G->hcursor[type]);
}

v2 UI_get_client_size() {
	return _v2(get_client_size());
}