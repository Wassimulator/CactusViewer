#pragma once
#include "main.h"
#include "ui_core.h"
#include "structs.h"
#include "shaders.hlsl"
#include "events.cpp"

#include "ui_platform_win32.cpp"
#include "ui_d3d11.cpp"
#include "ui_core.cpp"
#include "web_anim.cpp"

#include "gui.cpp"

void d3d11_get_last_error(bool message = false) {
	DWORD errorCode = GetLastError();
	if (errorCode == 0) return;
	LPSTR errorString = NULL;
	FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		errorCode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPSTR)&errorString,
		0,
		NULL
	);
	if (errorString != NULL) {
		if (message)
		{
			char errorMessage[256];
			sprintf(errorMessage, "Error: [%d] %s", errorCode,  errorString);
			MessageBoxA(NULL, errorMessage, "Error", MB_OK | MB_ICONERROR);
		}
		else printf("Error: [%d] %s\n", errorCode, errorString);
		LocalFree(errorString);
	} else {
		if (message)
		{
			char errorMessage[256];
			sprintf(errorMessage, "Error code: [%d]", errorCode);
			MessageBoxA(NULL, errorMessage, "Error", MB_OK | MB_ICONERROR);
		}
		else printf("Error code: [%d]\n", errorCode);
	}
}

bool win32_get_error(HRESULT hresult, bool message = false) {
	if (FAILED(hresult)) {
		LPWSTR buffer = NULL;
		FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		               NULL, hresult, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&buffer, 0, NULL);
		if (buffer) {
			if (message)
				MessageBoxW(NULL, buffer, L"ERROR", MB_OK | MB_ICONERROR);
			else
				fwprintf(stderr, L"Error: %s\n", buffer);
			LocalFree(buffer);
		}
		return false;
	}
	return true;
}

#define err(x) win32_get_error(x)

static ID3DBlob *compile_shader_memory(
	char *shader_code,
	size_t shader_code_size,
	const char *entry_point,
	const char *target) {

	ID3DBlob* blob;
	ID3DBlob* errors;

	HRESULT result = D3DCompile(
		shader_code, shader_code_size,
		nullptr,
		nullptr,   // defines
		nullptr,   // includes
		entry_point,
		target,
		0, 0,
		&blob,
		&errors
	);

	int error_count = 0;
	if (errors) {
		error_count = int(errors->GetBufferSize());
		printf("%*s\n", error_count, (char *)errors->GetBufferPointer());
		fflush(stdout);
	}

	if (FAILED(result)) {
		blob = nullptr;
	}
	return blob;
}

static ID3D11Buffer * create_constants_buffer(Graphics *ctx, size_t bytes) {
	D3D11_BUFFER_DESC desc = {};

	// round constant buffer size to 16 byte boundary
	desc.ByteWidth = (bytes + 0xf) & 0xfffffff0;
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	ID3D11Buffer *buffer;
	ctx->device->CreateBuffer(&desc, nullptr, &buffer);

	return buffer;
}

static Shader_Program create_shader_program(
	Graphics *ctx, char *shader_string, size_t shader_string_size,
	char * vs_entry_point, char * ps_entry_point, size_t constants_size,
	int input_elements, D3D11_INPUT_ELEMENT_DESC *input_desc) {

	Shader_Program shader = { 0 };

	ID3DBlob *vs_blob = compile_shader_memory(shader_string, shader_string_size, vs_entry_point, "vs_5_0");
	ctx->device->CreateVertexShader(vs_blob->GetBufferPointer(), vs_blob->GetBufferSize(), nullptr, &shader.vertex_shader);

	ID3DBlob *ps_blob = compile_shader_memory(shader_string, shader_string_size, ps_entry_point, "ps_5_0");
	ctx->device->CreatePixelShader(ps_blob->GetBufferPointer(), ps_blob->GetBufferSize(), nullptr, &shader.pixel_shader);

	if (input_desc) {
		ctx->device->CreateInputLayout(
			input_desc, input_elements,
			vs_blob->GetBufferPointer(), vs_blob->GetBufferSize(),
			&shader.input_layout);
	}

	if (constants_size) {
		shader.constants_buffer = create_constants_buffer(ctx, constants_size);
	}

	shader.constants_buffer_size = constants_size;

	return shader;
}

static void upload_constants(Shader_Program* program, void* data) {
	D3D11_MAPPED_SUBRESOURCE mapped;
	G->graphics.device_ctx->Map(program->constants_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
	assert(program->constants_buffer_size > 0);
	memcpy(mapped.pData, data, program->constants_buffer_size);
	G->graphics.device_ctx->Unmap(program->constants_buffer, 0);
}

static void upload_texture(Texture* texture, void* data, u64 size) {
	D3D11_MAPPED_SUBRESOURCE mapped;
	ZeroMemory(&mapped, sizeof(D3D11_MAPPED_SUBRESOURCE));
	HRESULT hr = G->graphics.device_ctx->Map(texture->d3d_texture, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);

	BYTE* dest = static_cast<BYTE*>(mapped.pData);
	BYTE* src = G->anim_buffer + G->anim_index * G->graphics.main_image.w * G->graphics.main_image.h * 4;

	int row_pitch = mapped.RowPitch;
	int row_size = G->graphics.main_image.w * 4; // 4 bytes per pixel as it's RGBA

	for (int y = 0; y < G->graphics.main_image.h; ++y) {
		memcpy(dest, src, row_size);
		dest += row_pitch;
		src += row_size;
	}
	G->graphics.device_ctx->Unmap(texture->d3d_texture, 0);
}

static void set_framebuffer_size(Graphics *ctx, iv2 size, bool set_dpi) {
	if (ctx == nullptr) 				return;
	if (ctx->viewport_size == size) 	return;
	if (ctx->frame_buffer == nullptr) 	return;
	if (size.x <= 0 || size.y <= 0) 	return;

	// resize swapchain

	float dpi_scale_factor = 1;
	if (set_dpi) {
		UINT dpi = GetDpiForWindow(hwnd);
		dpi_scale_factor = dpi / 96.0f;
	}

	ctx->frame_buffer_view->Release();
	ctx->frame_buffer->Release();
	ctx->swap_chain->ResizeBuffers(0, size.x / dpi_scale_factor, size.y / dpi_scale_factor, DXGI_FORMAT_UNKNOWN, 0);
	ctx->swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&ctx->frame_buffer);
	ctx->device->CreateRenderTargetView(ctx->frame_buffer, nullptr, &ctx->frame_buffer_view);

	//resize depthbuffer
	D3D11_TEXTURE2D_DESC depth_buffer_desc;
	ctx->depth_buffer->GetDesc(&depth_buffer_desc);

	ctx->depth_buffer_view->Release();
	ctx->depth_buffer->Release();

	depth_buffer_desc.Width = size.x;
	depth_buffer_desc.Height = size.y;

	ctx->device->CreateTexture2D(&depth_buffer_desc, nullptr, &ctx->depth_buffer);
	ctx->device->CreateDepthStencilView(ctx->depth_buffer, nullptr, &ctx->depth_buffer_view);

	ctx->viewport_size = size;
}


static void init_d3d11(HWND window_handle, int ww, int wh) {

	Graphics *ctx = &G->graphics;

	RECT rect;
	GetClientRect(hwnd, &rect);
	ww = rect.right - rect.left;
	wh = rect.bottom - rect.top;

	D3D_FEATURE_LEVEL feature_levels[] = {
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
	};
	ID3D11Device* base_device;
	ID3D11DeviceContext* base_device_ctx;
	UINT createDeviceFlags = 0;
#if DEBUG_MODE
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D11CreateDevice(
		nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags,
		feature_levels, array_size(feature_levels),
		D3D11_SDK_VERSION, &base_device,
		nullptr, &base_device_ctx
	);

	base_device->QueryInterface(__uuidof(ID3D11Device1), (void**)&ctx->device);
	base_device_ctx->QueryInterface(__uuidof(ID3D11DeviceContext1), (void**)&ctx->device_ctx);

	IDXGIDevice1* dxgi_device;
	(ctx->device)->QueryInterface(__uuidof(IDXGIDevice1), (void**)&dxgi_device);
	IDXGIAdapter* dxgi_adapter;
	dxgi_device->GetAdapter(&dxgi_adapter);
	IDXGIFactory2* dxgi_factory;
	dxgi_adapter->GetParent(__uuidof(IDXGIFactory2), (void**)&dxgi_factory);

	DXGI_SWAP_CHAIN_DESC1 swap_chain_desc = { 0 };
	swap_chain_desc.Width = 0;
	swap_chain_desc.Height = 0;
	swap_chain_desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	swap_chain_desc.Stereo = FALSE;
	swap_chain_desc.SampleDesc.Count = 1;
	swap_chain_desc.SampleDesc.Quality = 0;
	swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swap_chain_desc.BufferCount = 2;
	swap_chain_desc.Scaling = DXGI_SCALING_STRETCH;
	swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swap_chain_desc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	swap_chain_desc.Flags = 0;
	dxgi_factory->CreateSwapChainForHwnd(ctx->device, window_handle, &swap_chain_desc, nullptr, nullptr, &ctx->swap_chain);
	ctx->swap_chain->GetDesc1(&swap_chain_desc);

	D3D11_TEXTURE2D_DESC frame_buffer_desc = { 0 };;
	frame_buffer_desc.Width = ww ;
	frame_buffer_desc.Height = wh ;
	frame_buffer_desc.SampleDesc.Count = 1;
	frame_buffer_desc.MipLevels = 1;
	frame_buffer_desc.ArraySize = 1;
	frame_buffer_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	frame_buffer_desc.Usage = D3D11_USAGE_DEFAULT;
	frame_buffer_desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

	D3D11_TEXTURE2D_DESC depth_buffer_desc = frame_buffer_desc;
	depth_buffer_desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depth_buffer_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

	err(ctx->swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&ctx->frame_buffer)); // substitutes needing to call createtexture since we're using the swap chain's texture
	err(ctx->device->CreateRenderTargetView(ctx->frame_buffer, nullptr, &ctx->frame_buffer_view));

	err(ctx->device->CreateTexture2D(&depth_buffer_desc, nullptr, &ctx->depth_buffer));
	err(ctx->device->CreateDepthStencilView(ctx->depth_buffer, nullptr, &ctx->depth_buffer_view));


	ctx->main_program = create_shader_program(ctx, shader_text_main, strlen(shader_text_main),
	                                          "vs_main", "ps_main", sizeof(Shader_Constants_Main), 0, 0);
	ctx->bg_program   = create_shader_program(ctx, shader_text_bg, strlen(shader_text_bg),
	                                          "vs_bg", "ps_bg", sizeof(Shader_Constants_BG), 0, 0);
	ctx->crop_program = create_shader_program(ctx, shader_text_crop, strlen(shader_text_crop),
	                                          "vs_crop", "ps_crop", sizeof(Shader_Constants_Crop), 0, 0);

	D3D11_INPUT_ELEMENT_DESC lines_layout[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	ctx->lines_program= create_shader_program(ctx, shader_text_lines, strlen(shader_text_lines),
	                                          "vs_lines", "ps_lines", sizeof(Shader_Constants_Lines), 
	                                          1, lines_layout);

	D3D11_BUFFER_DESC lines_vertex_desc = {};
	lines_vertex_desc.Usage = D3D11_USAGE_DYNAMIC;
	lines_vertex_desc.ByteWidth = sizeof(v2) * 256;
	lines_vertex_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	lines_vertex_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	ctx->device->CreateBuffer(&lines_vertex_desc, 0, &G->graphics.lines_vertex_buffer);

	D3D11_BLEND_DESC blend_desc = { 0 };
	blend_desc.AlphaToCoverageEnable = false;
	blend_desc.RenderTarget[0].BlendEnable = true;
	blend_desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blend_desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blend_desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blend_desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blend_desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
	blend_desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blend_desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	err(ctx->device->CreateBlendState(&blend_desc, &ctx->blend_state));

	D3D11_RASTERIZER_DESC rast_desc = { 0 };
	rast_desc.CullMode = D3D11_CULL_NONE;
	rast_desc.FillMode = D3D11_FILL_SOLID;
	rast_desc.ScissorEnable = false;
	rast_desc.DepthClipEnable = false;
	rast_desc.FrontCounterClockwise = true;
	err(ctx->device->CreateRasterizerState(&rast_desc, &ctx->raster_state));

	D3D11_DEPTH_STENCIL_DESC depth_desc = { 0 };
	depth_desc.DepthEnable = false;
	depth_desc.StencilEnable = false;
	depth_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depth_desc.DepthFunc = D3D11_COMPARISON_ALWAYS;
	depth_desc.FrontFace.StencilFailOp = depth_desc.FrontFace.StencilDepthFailOp = depth_desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depth_desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	depth_desc.BackFace = depth_desc.FrontFace;
	err(ctx->device->CreateDepthStencilState(&depth_desc, &ctx->depth_stencil_state));

	D3D11_SAMPLER_DESC sampler_desc = { 0 };
	sampler_desc.Filter = 			D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampler_desc.AddressU = 		D3D11_TEXTURE_ADDRESS_CLAMP;
	sampler_desc.AddressV = 		D3D11_TEXTURE_ADDRESS_CLAMP;
	sampler_desc.AddressW = 		D3D11_TEXTURE_ADDRESS_CLAMP;
	sampler_desc.ComparisonFunc = 	D3D11_COMPARISON_NEVER;
	sampler_desc.MinLOD = 			0;
	sampler_desc.MaxLOD = 			D3D11_FLOAT32_MAX;
	err(ctx->device->CreateSamplerState(&sampler_desc, &ctx->sampler_linear));

	sampler_desc.Filter = 			D3D11_FILTER_MIN_MAG_MIP_POINT;
	err(ctx->device->CreateSamplerState(&sampler_desc, &ctx->sampler_nearest));

	set_framebuffer_size(ctx, iv2(ww, wh));

	G->graphics.MAX_GPU = D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION;

	ID3D10Multithread* multi_thread = nullptr;
    HRESULT hr = ctx->device->QueryInterface(__uuidof(ID3D10Multithread), reinterpret_cast<void**>(&multi_thread));
    if (SUCCEEDED(hr) && multi_thread)
    {
        multi_thread->SetMultithreadProtected(TRUE);
        multi_thread->Release();
    }
}

static Shader_Constants_Main set_main_shader_constants() {
	Shader_Constants_Main result = { 0 };
	result.aspect_img = G->graphics.aspect_img;
	result.aspect_wnd = G->graphics.aspect_wnd;
	result.scale = G->scale;
	result.position = G->position / v2(WW, -WH) * 2;
	result.pixel_grid = (G->nearest_filtering && (G->truescale > 5) && G->pixel_grid) ? 1 : 0;
	result.true_scale = G->truescale;
	result.image_dim = v2(G->graphics.main_image.w, G->graphics.main_image.h);
	result.window = v2(WW, WH);
	result.rgba_flags = v4((float)RGBAflags[0], (float)RGBAflags[1], (float)RGBAflags[2], (float)RGBAflags[3]);
	result.rotation = G->graphics.main_image.orientation;
	result.hue = G->hue;
	result.saturation = G->saturation;
	result.contrast = G->contrast;
	result.brightness = G->brightness;
	result.srgb = G->srgb;
	result.gamma = G->gamma;
	result.render_mode = RENDER_MODE_VEIWER;
	result.do_blur = G->do_blur;
	result.blur_lod = G->blur_lod;
	result.blur_samples = G->blur_samples;
	result.blur_scale = G->blur_scale;
	result.crop_a = _v2(G->crop_a);
	result.crop_b = _v2(G->crop_b);
	result.crop_mode = G->crop_mode;
	return result;
}

static Texture create_texture(u8 *data, int w, int h, bool dynamic) {
	Graphics *d3d_ctx = &G->graphics;
	D3D11_TEXTURE2D_DESC texture_desc = {};
	texture_desc.Width             = w;
	texture_desc.Height            = h;
	texture_desc.MipLevels         = dynamic ? 1 : 0; 
	texture_desc.ArraySize         = 1;
	texture_desc.SampleDesc.Count  = 1;
	texture_desc.Format            = DXGI_FORMAT_R8G8B8A8_UNORM;
	texture_desc.BindFlags         = D3D11_BIND_SHADER_RESOURCE;
	texture_desc.Usage             = dynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
	if (!dynamic) texture_desc.BindFlags |= D3D11_BIND_RENDER_TARGET;
	if (!dynamic) texture_desc.MiscFlags  = D3D11_RESOURCE_MISC_GENERATE_MIPS;
	if (dynamic)  texture_desc.CPUAccessFlags  = D3D11_CPU_ACCESS_WRITE;

	D3D11_SUBRESOURCE_DATA texture_SRD = {};
	texture_SRD.pSysMem     = data;
	texture_SRD.SysMemPitch = w * 4;

	D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
	srv_desc.Format = texture_desc.Format;
	srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srv_desc.Texture2D.MipLevels = UINT_MAX;

	Texture result;
	result.size = v2(w, h);
	err(d3d_ctx->device->CreateTexture2D(&texture_desc, 0, &result.d3d_texture));
	err(d3d_ctx->device->CreateShaderResourceView(result.d3d_texture, &srv_desc, &result.srv));
	

	if (!dynamic && data) {
		d3d_ctx->device_ctx->UpdateSubresource(result.d3d_texture, 0, NULL, data, w * 4, w * h * 4);
		d3d_ctx->device_ctx->GenerateMips(result.srv);
	}

	return result;
}

#include <windows.h>

RECT g_original_window_rect;
LONG g_original_window_style;

void enter_fullscreen(HWND hwnd) {
	GetWindowRect(hwnd, &g_original_window_rect);
	g_original_window_style = GetWindowLong(hwnd, GWL_STYLE);
    
	LONG newWindowStyle = g_original_window_style & ~(WS_CAPTION | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_SYSMENU | WS_THICKFRAME);
	SetWindowLong(hwnd, GWL_STYLE, newWindowStyle);

	int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	int screenHeight = GetSystemMetrics(SM_CYSCREEN);

	SetWindowPos(hwnd, HWND_TOP, 0, 0, screenWidth, screenHeight, SWP_FRAMECHANGED);
}

void exit_fullscreen(HWND hwnd) {
	SetWindowLong(hwnd, GWL_STYLE, g_original_window_style);
    
	SetWindowPos(hwnd, HWND_NOTOPMOST,
	             g_original_window_rect.left, g_original_window_rect.top,
	             g_original_window_rect.right - g_original_window_rect.left,
	             g_original_window_rect.bottom - g_original_window_rect.top,
	             SWP_FRAMECHANGED);
}

bool is_fullscreen(HWND hwnd) {
	return !(GetWindowLong(hwnd, GWL_STYLE) & WS_CAPTION);
}

void toggle_fullscreen(HWND hwnd) {
	if (!is_fullscreen(hwnd))
		enter_fullscreen(hwnd);
	else
		exit_fullscreen(hwnd);
}


static void init_logo_image() {
	G->graphics.logo_image = create_texture(minilogo, 50, 53, false);
}

static void save_settings() {
    char buffer[0x400];
    snprintf(buffer, sizeof(buffer), "%s\\config.json", APPDATA_FOLDER);
    FILE *F = fopen(buffer, "w");
    cJSON *config_file = cJSON_CreateObject();

    if (F) {
	cJSON *j_checkboard_color_1 = cJSON_CreateArray();
	cJSON_AddItemToArray(j_checkboard_color_1, cJSON_CreateNumber(checkerboard_color_1[0]));
	cJSON_AddItemToArray(j_checkboard_color_1, cJSON_CreateNumber(checkerboard_color_1[1]));
	cJSON_AddItemToArray(j_checkboard_color_1, cJSON_CreateNumber(checkerboard_color_1[2]));
	cJSON *j_checkboard_color_2 = cJSON_CreateArray();
	cJSON_AddItemToArray(j_checkboard_color_2, cJSON_CreateNumber(checkerboard_color_2[0]));
	cJSON_AddItemToArray(j_checkboard_color_2, cJSON_CreateNumber(checkerboard_color_2[1]));
	cJSON_AddItemToArray(j_checkboard_color_2, cJSON_CreateNumber(checkerboard_color_2[2]));
	cJSON *j_bg_color = cJSON_CreateArray();
	cJSON_AddItemToArray(j_bg_color, cJSON_CreateNumber(bg_color[0]));
	cJSON_AddItemToArray(j_bg_color, cJSON_CreateNumber(bg_color[1]));
	cJSON_AddItemToArray(j_bg_color, cJSON_CreateNumber(bg_color[2]));
	cJSON_AddItemToArray(j_bg_color, cJSON_CreateNumber(bg_color[3]));

	cJSON_AddItemToObject(config_file, "settings_resetpos", cJSON_CreateNumber(G->settings_resetpos));
    cJSON_AddItemToObject(config_file, "settings_resetzoom", cJSON_CreateNumber(G->settings_resetzoom));
	cJSON_AddItemToObject(config_file, "checkerboard_color_1", j_checkboard_color_1);
	cJSON_AddItemToObject(config_file, "checkerboard_color_2", j_checkboard_color_2);
	cJSON_AddItemToObject(config_file, "bg_color", j_bg_color);
    cJSON_AddItemToObject(config_file, "settings_autoplayGIFs", cJSON_CreateBool(G->settings_autoplayGIFs));
    cJSON_AddItemToObject(config_file, "settings_movementmag", cJSON_CreateNumber(G->settings_movementmag));
    cJSON_AddItemToObject(config_file, "settings_shiftslowmag", cJSON_CreateNumber(G->settings_shiftslowmag));
    cJSON_AddItemToObject(config_file, "settings_movementinvert", cJSON_CreateBool(G->settings_movementinvert));
    cJSON_AddItemToObject(config_file, "nearest_filtering", cJSON_CreateBool(G->nearest_filtering));
    cJSON_AddItemToObject(config_file, "pixel_grid", cJSON_CreateBool(G->pixel_grid));
    cJSON_AddItemToObject(config_file, "settings_sort", cJSON_CreateBool(G->settings_sort));
    cJSON_AddItemToObject(config_file, "settings_exif", cJSON_CreateBool(G->settings_exif));
    cJSON_AddItemToObject(config_file, "settings_hide_status_fullscreen", cJSON_CreateBool(G->settings_hide_status_fullscreen));
    cJSON_AddItemToObject(config_file, "settings_start_in_fullscreen", cJSON_CreateBool(G->settings_start_in_fullscreen));
    cJSON_AddItemToObject(config_file, "settings_dont_resize", cJSON_CreateBool(G->settings_dont_resize));
    cJSON_AddItemToObject(config_file, "settings_selected_theme", cJSON_CreateNumber(G->settings_selected_theme));
    cJSON_AddItemToObject(config_file, "settings_calculate_histograms", cJSON_CreateBool(G->settings_calculate_histograms));
    cJSON_AddItemToObject(config_file, "settings_preview_thumbs", cJSON_CreateBool(G->settings_preview_thumbs));
    cJSON_AddItemToObject(config_file, "settings_hide_status_with_gui", cJSON_CreateBool(G->settings_hide_status_with_gui));
    cJSON_AddItemToObject(config_file, "settings_always_show_gui", cJSON_CreateBool(G->settings_always_show_gui));
    cJSON_AddItemToObject(config_file, "settings_newfilezoom", cJSON_CreateNumber(G->settings_newfilezoom));
    cJSON_AddItemToObject(config_file, "settings_copy_color_format", cJSON_CreateNumber(G->settings_copy_color_format));
    cJSON_AddItemToObject(config_file, "settings_copy_color_enclose_type", cJSON_CreateNumber(G->settings_copy_color_enclose_type));
    cJSON_AddItemToObject(config_file, "settings_copy_color_include_alpha", cJSON_CreateBool(G->settings_copy_color_include_alpha));
    cJSON_AddItemToObject(config_file, "settings_copy_color_normalize_rgb", cJSON_CreateBool(G->settings_copy_color_normalize_rgb));
    fprintf(F, cJSON_Print(config_file));
	fclose(F);
    }
}

static void load_settings() {
    char buffer[0x400];
    snprintf(buffer, sizeof(buffer), "%s\\config.json", APPDATA_FOLDER);
    FILE *F = fopen(buffer, "r");
    if (F) {
		fseek(F, 0, SEEK_END);
		size_t size = ftell(F);
		fseek(F, 0, SEEK_SET);
		char *data = (char *)malloc(size);
		fread(data, 1, size, F);
		fclose(F);
		cJSON *config_file = cJSON_ParseWithLength(data, size);
		cJSON *item = 0;
		
		cJSON *j_checkboard_color_1 = cJSON_GetObjectItemCaseSensitive(config_file, "checkerboard_color_1");
		if (j_checkboard_color_1) {
			checkerboard_color_1[0] = cJSON_GetArrayItem(j_checkboard_color_1, 0)->valuedouble;
			checkerboard_color_1[1] = cJSON_GetArrayItem(j_checkboard_color_1, 1)->valuedouble;
			checkerboard_color_1[2] = cJSON_GetArrayItem(j_checkboard_color_1, 2)->valuedouble;
		}
		cJSON *j_checkboard_color_2 = cJSON_GetObjectItemCaseSensitive(config_file, "checkerboard_color_2");
		if (j_checkboard_color_2) {
			checkerboard_color_2[0] = cJSON_GetArrayItem(j_checkboard_color_2, 0)->valuedouble;
			checkerboard_color_2[1] = cJSON_GetArrayItem(j_checkboard_color_2, 1)->valuedouble;
			checkerboard_color_2[2] = cJSON_GetArrayItem(j_checkboard_color_2, 2)->valuedouble;
		}
		cJSON *j_bg_color = cJSON_GetObjectItemCaseSensitive(config_file, "bg_color");
		if (j_bg_color) {
			bg_color[0] = cJSON_GetArrayItem(j_bg_color, 0)->valuedouble;
			bg_color[1] = cJSON_GetArrayItem(j_bg_color, 1)->valuedouble;
			bg_color[2] = cJSON_GetArrayItem(j_bg_color, 2)->valuedouble;
			bg_color[3] = cJSON_GetArrayItem(j_bg_color, 3)->valuedouble;
		}
		item = cJSON_GetObjectItemCaseSensitive(config_file, "settings_resetpos"); 					if (item) G->settings_resetpos = item->valueint;
		item = cJSON_GetObjectItemCaseSensitive(config_file, "settings_resetzoom"); 				if (item) G->settings_resetzoom = item->valueint;
		item = cJSON_GetObjectItemCaseSensitive(config_file, "settings_autoplayGIFs"); 				if (item) G->settings_autoplayGIFs = item->valueint;
		item = cJSON_GetObjectItemCaseSensitive(config_file, "settings_movementmag"); 				if (item) G->settings_movementmag = item->valuedouble;
		item = cJSON_GetObjectItemCaseSensitive(config_file, "settings_shiftslowmag"); 				if (item) G->settings_shiftslowmag = item->valuedouble;
		item = cJSON_GetObjectItemCaseSensitive(config_file, "settings_movementinvert"); 			if (item) G->settings_movementinvert = item->valueint;
		item = cJSON_GetObjectItemCaseSensitive(config_file, "nearest_filtering"); 					if (item) G->nearest_filtering = item->valueint;
		item = cJSON_GetObjectItemCaseSensitive(config_file, "pixel_grid"); 						if (item) G->pixel_grid = item->valueint;
		item = cJSON_GetObjectItemCaseSensitive(config_file, "settings_sort"); 						if (item) G->settings_sort = item->valueint;
		item = cJSON_GetObjectItemCaseSensitive(config_file, "settings_exif"); 						if (item) G->settings_exif = item->valueint;
		item = cJSON_GetObjectItemCaseSensitive(config_file, "settings_hide_status_fullscreen"); 	if (item) G->settings_hide_status_fullscreen = item->valueint;
		item = cJSON_GetObjectItemCaseSensitive(config_file, "settings_start_in_fullscreen"); 		if (item) G->settings_start_in_fullscreen = item->valueint;
		item = cJSON_GetObjectItemCaseSensitive(config_file, "settings_dont_resize"); 				if (item) G->settings_dont_resize = item->valueint;
		item = cJSON_GetObjectItemCaseSensitive(config_file, "settings_selected_theme"); 			if (item) G->settings_selected_theme = item->valueint;
		item = cJSON_GetObjectItemCaseSensitive(config_file, "settings_calculate_histograms"); 		if (item) G->settings_calculate_histograms = item->valueint;
		item = cJSON_GetObjectItemCaseSensitive(config_file, "settings_preview_thumbs"); 			if (item) G->settings_preview_thumbs = item->valueint;
		item = cJSON_GetObjectItemCaseSensitive(config_file, "settings_hide_status_with_gui"); 		if (item) G->settings_hide_status_with_gui = item->valueint;
		item = cJSON_GetObjectItemCaseSensitive(config_file, "settings_always_show_gui"); 			if (item) G->settings_always_show_gui = item->valueint;
		item = cJSON_GetObjectItemCaseSensitive(config_file, "settings_newfilezoom"); 				if (item) G->settings_newfilezoom = item->valueint;
		item = cJSON_GetObjectItemCaseSensitive(config_file, "settings_copy_color_format"); 		if (item) G->settings_copy_color_format = item->valueint;
		item = cJSON_GetObjectItemCaseSensitive(config_file, "settings_copy_color_enclose_type"); 	if (item) G->settings_copy_color_enclose_type = item->valueint;
		item = cJSON_GetObjectItemCaseSensitive(config_file, "settings_copy_color_include_alpha"); 	if (item) G->settings_copy_color_include_alpha = item->valueint;
		item = cJSON_GetObjectItemCaseSensitive(config_file, "settings_copy_color_normalize_rgb"); 	if (item) G->settings_copy_color_normalize_rgb = item->valueint;
        fclose(F);
		free(data);
    }
}

static uint32_t get_ticks() {
    static LARGE_INTEGER frequency;
    static bool n = QueryPerformanceFrequency(&frequency);
    LARGE_INTEGER  result;
    QueryPerformanceCounter(&result);
    return 1000 * result.QuadPart / frequency.QuadPart;
}
static void center_window() {
    v2 display_size = v2(GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
    RECT rect;
    GetClientRect (hwnd, &rect);
    int w = rect.right - rect.left;
    int h = rect.bottom - rect.top;
    int x = (display_size.x - w) / 2;
    int y = (display_size.y - h) / 2;
    SetWindowPos(hwnd, NULL, x, y, w, h, SWP_NOZORDER | SWP_NOACTIVATE);
}

bool get_font_file_from_system(char* output, int output_size, char* font) {
    HKEY hkey;
    CHAR value[2048];
    DWORD value_length = sizeof(value);
    const CHAR* sub_key = "Software\\Microsoft\\Windows NT\\CurrentVersion\\Fonts";
    CHAR fonts_dir[MAX_PATH];
    bool result = false;

    if (SHGetFolderPathA(NULL, CSIDL_FONTS, NULL, SHGFP_TYPE_CURRENT, fonts_dir) == S_OK) {
        if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, sub_key, 0, KEY_READ, &hkey) == ERROR_SUCCESS) {
            if (RegQueryValueExA(hkey, font, NULL, NULL, (LPBYTE)value, &value_length) == ERROR_SUCCESS) {
                if (PathIsRelativeA(value)) {
                    snprintf(output, output_size, "%s\\%.*s", fonts_dir, value_length, value);
                } else {
                    strncpy(output, value, output_size);
                }
                output[output_size - 1] = '\0'; 
                if (value[0] != '\0') {
                    result = true;
                }
            } 
            RegCloseKey(hkey);
        }
    }
    return result;
}

static void init_all() {
    WW  = 700;
    WH = 800;

    HINSTANCE hInstance = GetModuleHandle(NULL);
    const char* szTitle = "CactusViewer";
   
    WNDCLASSEX wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS ;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hbrBackground = NULL;
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = L"CactusViewer" ;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(1));
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(1));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    RegisterClassEx(&wcex);
    RECT rc  { 0, 0,  WW, WH };
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, 0);
    hwnd = CreateWindowEx(WS_EX_APPWINDOW, L"CactusViewer", L"CactusViewer", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInstance, nullptr);
    hdc = GetDC(hwnd);
    center_window();
    PIXELFORMATDESCRIPTOR pfd;
    ZeroMemory(&pfd, sizeof(PIXELFORMATDESCRIPTOR));
    pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cDepthBits = 24;
    pfd.cStencilBits = 8;

	SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);

	Gdiplus::GdiplusStartupInput gdipsi;
	ULONG_PTR gdipt;
	Gdiplus::GdiplusStartup(&gdipt, &gdipsi, NULL);

	init_d3d11(hwnd, WW, WH);
	init_logo_image();

	bg_color[0] = 0.15;
	bg_color[1] = 0.15;
	bg_color[2] = 0.15;
	bg_color[3] = 0;
	float c1 = 0.16;
	float c2 = 0.1;
	checkerboard_color_1[0] = c1;
	checkerboard_color_1[1] = c1;
	checkerboard_color_1[2] = c1;
	checkerboard_color_2[0] = c2;
	checkerboard_color_2[1] = c2;
	checkerboard_color_2[2] = c2;

	Checkerboard_size = 250;

	RGBAflags[0] = 1;
	RGBAflags[1] = 1;
	RGBAflags[2] = 1;
	RGBAflags[3] = 1;
	G->alert.timer = 0;
	G->settings_autoplayGIFs = true;
	G->settings_movementmag = 2;
	G->settings_shiftslowmag = 9;

	G->hue = 0;
	G->saturation = 1;
	G->contrast = 1;
	G->brightness = 0;
	G->gamma = 1;
	G->do_blur = false;
	G->blur_lod = 1;
	G->blur_samples = 32;
	G->blur_scale = 0.001;

	G->graphics.main_image.has_exif = false;
	G->graphics.main_image.orientation = 0;

	G->graphics.thumbs = create_texture(0, 4000, 4000, false);

	load_settings();

//	BOOL USE_DARK_MODE = G->settings_selected_theme != UI_Theme_Light;
//	BOOL SET_IMMERSIVE_DARK_MODE_SUCCESS = SUCCEEDED(DwmSetWindowAttribute(
//		hwnd, DWMWINDOWATTRIBUTE::DWMWA_USE_IMMERSIVE_DARK_MODE,
//		&USE_DARK_MODE, sizeof(USE_DARK_MODE)));

    DragAcceptFiles(hwnd, TRUE);
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);
    SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)wcex.hIcon);
    SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)wcex.hIcon);

	G->hcursor[(int)Cursor_Type_arrow] =     LoadCursor(nullptr, IDC_ARROW);
	G->hcursor[(int)Cursor_Type_text] =      LoadCursor(nullptr, IDC_IBEAM);
	G->hcursor[(int)Cursor_Type_resize_h] =  LoadCursor(nullptr, IDC_SIZEWE);
	G->hcursor[(int)Cursor_Type_resize_v] =  LoadCursor(nullptr, IDC_SIZENS);
	G->hcursor[(int)Cursor_Type_resize_dr] = LoadCursor(nullptr, IDC_SIZENWSE);
	G->hcursor[(int)Cursor_Type_resize_dl] = LoadCursor(nullptr, IDC_SIZENESW);
    G->graphics.main_image.texture.d3d_texture = 0;
	
    InitializeCriticalSection(&G->mutex);
    InitializeCriticalSection(&G->sort_mutex);
    InitializeCriticalSection(&G->thumbs_mutex);
    InitializeCriticalSection(&G->id_mutex);

	G->ui = UI_init_context();
	UI_d3d11_init(G->ui, G->graphics.device, G->graphics.device_ctx);
	UI_init_platform_win32(G->ui);

	int sizes[] = { 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 20 };
	int ascii_start = 32;
	int ascii_end = 126;

//#if	DEBUG_MODE
//	G->ui_font = UI_load_font_file(G->ui, "../src/FiraSans-Regular.ttf", 
//	                                 ascii_start, ascii_end, sizes, array_size(sizes));
//#else
//	u8 font_file[] = {
//		#include "FiraSans-Regular.ttf.cpp"
//	};
//	G->ui_font = UI_load_font_memory(G->ui, font_file, array_size(font_file), 
//	                                 ascii_start, ascii_end, sizes, array_size(sizes));
//	#endif

	// just getting the font from the system now:
	char system_font[512] = {};
	if (!get_font_file_from_system(system_font, 512, "Segoe UI (TrueType)"))
		get_font_file_from_system(system_font, 512, "Arial (TrueType)");
	G->ui_font = UI_load_font_file(G->ui, system_font, 
								   ascii_start, ascii_end, sizes, array_size(sizes));

//	G->shapes_texture_id = UI_create_texture(G->ui, 
//	                                         (u8*)UI_asset_shape_arrow,
//	                                         UI_ASSET_SHAPE_ARROW_WIDTH,
//	                                         UI_ASSET_SHAPE_ARROW_HEIGHT);


    // CreateThread(NULL, 0, FontLoadThread, NULL, 0, NULL);

	G->force_loop_frames = 2;
	G->loader_event = CreateEvent(
		NULL,               // default security attributes
		TRUE,               // manual-reset event; false = auto-reset
		FALSE,              // initial state is nonsignaled
		TEXT("loader event")     
	);



}

static void push_alert(char *string, Alert_Type type = Alert_Error) {
    int l = strlen(string) + 1;
    memcpy(G->alert.string, string, min(l, 256));
    G->alert.timer = 1;
	G->alert.type = type;
	G->force_loop_frames += 310;

#if DEBUG_MODE
	printf("%s\n",string);
#endif
};
struct Loader_Thread_Inputs {
    wchar_t *path;
    u32 id;
	File_Data* file_data;
    bool dropped;
};

static void reset_to_no_folder() {
    if (G->loading_dropped_file)
        G->loading_dropped_file = false;
    G->files.reset_count();
}

static void set_to_no_file() {
    G->graphics.main_image.w = 50;
    G->graphics.main_image.h = 53;
    G->req_truescale = 4;
    G->position.x = 0;
    G->position.y = 0;
    send_signal(G->signals.update_truescale);
}

void calculate_histogram(unsigned char* data, u64 size) {
	G->graphics.main_image.has_histo = false;
	if (!G->settings_calculate_histograms) return;
	assert(size % 4 == 0);
	memset(G->histo_r, 0, 256 * sizeof(u64));
	memset(G->histo_g, 0, 256 * sizeof(u64));
	memset(G->histo_b, 0, 256 * sizeof(u64));
	memset(G->histo_t, 0, 256 * sizeof(u64));
	G->histo_max = 0;
	for (int i = 0; i < size - 4; i+=4) {
		G->histo_r[data[i + 0]]++; G->histo_t[data[i + 0]]++; 
		G->histo_g[data[i + 1]]++; G->histo_t[data[i + 1]]++;
		G->histo_b[data[i + 2]]++; G->histo_t[data[i + 2]]++;
	}
	for (int i = 0; i < 256; i++) {
		G->histo_max = max(G->histo_max, G->histo_t[i]);
	}
	G->histo_max_edit = G->histo_max;
	G->graphics.main_image.has_histo = true;
}

static int load_image_pre(wchar_t *path, u32 id, bool dropped) {
    int w, h, n;
    int result = 0;
    G->files[id].loading = true;

    int size = stbi_convert_wchar_to_utf8(0, 0, path);
	char *filename_utf8 = (char *)malloc(size);
	stbi_convert_wchar_to_utf8(filename_utf8, size, path);
    unsigned char *data = stbi_load(filename_utf8, &w, &h, &n, 4);
    G->files[id].loading = false;
    if (data == nullptr) {
        push_alert("Loading the file failed");
        G->files[G->current_file_index].failed = true;
        G->loaded = true;
        
        //if (dropped)
        //    reset_to_no_folder();
        set_to_no_file();
    } else {
        if (G->graphics.MAX_GPU < w || G->graphics.MAX_GPU < h) {
            push_alert("Image is too large.");
        }
        else {
            EnterCriticalSection(&G->mutex);
            if (id == G->current_file_index) {
                G->graphics.main_image.w = w;
                G->graphics.main_image.h = h;
                G->graphics.main_image.n = n;
				if (G->graphics.main_image.data) {
					wfree(G->graphics.main_image.data);
					G->graphics.main_image.data = 0;
				}
                G->graphics.main_image.data = data;
                send_signal(G->signals.init_step_2);
            }
            else {
                free(data);
            }
            LeaveCriticalSection(&G->mutex);
            result = 1;
        }
    }
    return result;
}

static int load_image_wic_pre(wchar_t *path, u32 id, bool dropped, File_Data* file_data) {
    u32 w, h;
    int result = 0;
    G->files[id].loading = true;
	IWICBitmapDecoder* decoder = NULL;
	IWICBitmapFrameDecode* frame = NULL;
	IWICFormatConverter* converter = NULL;

	// Initialize the COM library
	CoInitialize(NULL);

	// Create WIC factory
	err(CoCreateInstance(CLSID_WICImagingFactory,
	                     NULL,
	                     CLSCTX_INPROC_SERVER,
	                     IID_IWICImagingFactory,
	                     (LPVOID*) & G->wic_factory));

	unsigned char *data = 0;
	HRESULT hr = G->wic_factory->CreateDecoderFromFilename(path, NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &decoder);
	if(SUCCEEDED(hr)) hr = decoder->GetFrame(0, &frame);
	if(SUCCEEDED(hr)) hr =G->wic_factory->CreateFormatConverter(&converter);
	if(SUCCEEDED(hr)) hr = converter->Initialize(frame, GUID_WICPixelFormat32bppRGBA, WICBitmapDitherTypeNone, NULL, 0.0, WICBitmapPaletteTypeCustom);
	if (SUCCEEDED(hr)) {
		converter->GetSize(&w, &h);
		data = (unsigned char *)walloc(w * h * 4);
		converter->CopyPixels(NULL, w * 4, w * h * 4, data);

		// expensive, but requested:
		if (G->settings_exif) {
			FILE* temp_file = _wfopen(path, L"rb");
			if (temp_file) {
				fseek(temp_file, 0, SEEK_END);
				size_t file_size = ftell(temp_file);
				fseek(temp_file, 0, SEEK_SET);
				uint8_t* file_data = (uint8_t*) malloc(file_size);
				fread(file_data, 1, file_size, temp_file);
				fclose(temp_file);
				int exif_result = G->graphics.main_image.exif_info.parseFrom(file_data, file_size);
				free(file_data);
				G->graphics.main_image.has_exif = exif_result == PARSE_EXIF_SUCCESS;
				if (G->graphics.main_image.has_exif) {
					switch (G->graphics.main_image.exif_info.Orientation) {
						case 3:
							G->graphics.main_image.orientation = 2; break;
						case 6:
							G->graphics.main_image.orientation = 3; break;
						case 8:
							G->graphics.main_image.orientation = 1; break;
					}
					send_signal(G->signals.update_orientation_step_2);
				}
			}
		}
	}
	G->files[id].loading = false;
	if (FAILED(hr) || data == 0) {
		if (hr == WINCODEC_ERR_COMPONENTNOTFOUND) {
			push_alert(UI_sprintf(&G->ui->strings, "Component not found: File type '%s' not supported.", UI_sprintf(&G->ui->strings, "%S", file_data->file.ext)));
		} else if (hr == WINCODEC_ERR_COMPONENTINITIALIZEFAILURE) {
			push_alert(UI_sprintf(&G->ui->strings, "Component initialization failed: Codec of '%s' is likely not installed.", UI_sprintf(&G->ui->strings, "%S", file_data->file.ext)));
		} else {
			LPVOID lpMsgBuf;
			DWORD bufLen = FormatMessageA(
				FORMAT_MESSAGE_ALLOCATE_BUFFER |
				FORMAT_MESSAGE_FROM_SYSTEM |
				FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL,
				hr,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				(LPSTR) & lpMsgBuf,
				0, NULL);
			push_alert((char*)lpMsgBuf);
			LocalFree(lpMsgBuf);
		}
        G->files[G->current_file_index].failed = true;
        G->loaded = true;
        
        //if (dropped)
            //reset_to_no_folder();
        set_to_no_file();
    } else {
        if (G->graphics.MAX_GPU < w || G->graphics.MAX_GPU < h) {
            push_alert("Image is too large.");
        } else {
            EnterCriticalSection(&G->mutex);
            if (id == G->current_file_index) {
                G->graphics.main_image.w = w;
                G->graphics.main_image.h = h;
                G->graphics.main_image.n = 0;
			//	if (G->graphics.main_image.data) {
			//		wfree(G->graphics.main_image.data);
			//		G->graphics.main_image.data = 0;
			//	}
				G->graphics.main_image.data = data;
				calculate_histogram(data, w * h * 4);
                send_signal(G->signals.init_step_2);
            } else {
                wfree(data);
            }
            LeaveCriticalSection(&G->mutex);
			result = 1;
        }
    }

	cleanup:

	if (frame) frame->Release();
	if (decoder) decoder->Release();
	if (converter) converter->Release();
	CoUninitialize();

    return result;
}

static void unload_anim_image() {
	G->anim_frames = 0;
	free(G->anim_buffer);
	G->anim_buffer = nullptr;
	free(G->anim_frame_delays);
	G->anim_frame_delays = nullptr;
}

static int load_webp_pre(wchar_t *path, u32 id, bool dropped, int* type) {
    int w, h;
    int result = 0;
    int size = stbi_convert_wchar_to_utf8(0, 0, path);
	char *filename_utf8 = (char *)malloc(size);
	stbi_convert_wchar_to_utf8(filename_utf8, size, path);
	// Read the WebP file into memory.
	FILE* file = _wfopen(path, L"rb");
	if (!file) {
		perror("Failed to open file");
		return result;
	}
	fseek(file, 0, SEEK_END);
	size_t file_size = ftell(file);
	fseek(file, 0, SEEK_SET);
	uint8_t* file_data = (uint8_t*)malloc(file_size);
	fread(file_data, 1, file_size, file);
	fclose(file);

	WebPBitstreamFeatures features;
	WebPGetFeatures(file_data, file_size, &features);

	if (!features.has_animation) {
		G->files[id].loading = true;
		unsigned char *data = WebPDecodeRGBA(file_data, file_size, &w, &h);
		G->files[id].loading = false;
		if (data == nullptr) {
			push_alert("Loading the file failed");
			G->files[G->current_file_index].failed = true;
			G->loaded = true;
			
			//if (dropped)
				//reset_to_no_folder();
			set_to_no_file();
		} else {
			if (G->graphics.MAX_GPU < w || G->graphics.MAX_GPU < h) {
				push_alert("Image is too large.");
			}
			else {
				EnterCriticalSection(&G->mutex);
				if (id == G->current_file_index) {
					G->graphics.main_image.w = w;
					G->graphics.main_image.h = h;
					G->graphics.main_image.n = 0;
					//if (G->graphics.main_image.data) {
					//	wfree(G->graphics.main_image.data);
					//	G->graphics.main_image.data = 0;
					//}
					G->graphics.main_image.data = data;
					calculate_histogram(data, w * h * 4);
					send_signal(G->signals.init_step_2);
				} else {
					wfree(data);
				}
				LeaveCriticalSection(&G->mutex);
				result = 1;
			}
		}
	} else {
		unload_anim_image();
		WebPData webp_data = { file_data, file_size };
		Animated_Image webp_anim_image;
		*type = TYPE_WEBP_ANIM;

		if (webp_anim_read_file(&webp_data, &webp_anim_image)) {
			EnterCriticalSection(&G->mutex);
			G->graphics.main_image.w = webp_anim_image.canvas_width;
			G->graphics.main_image.h = webp_anim_image.canvas_height;
			G->anim_frames = webp_anim_image.num_frames;
			G->anim_frame_delays = webp_anim_image.durations;
			G->anim_buffer = (unsigned char *)webp_anim_image.raw_mem;

			G->anim_index = 0;
			G->anim_play = G->settings_autoplayGIFs;

			if (id != G->current_file_index)
				unload_anim_image();
			else
				send_signal(G->signals.init_step_2);
			result = 1;
		} else {
			push_alert("Loading animated WebP file failed");
			G->files[G->current_file_index].failed = true;
			G->loaded = true;
			//if (dropped)
				//reset_to_no_folder();
			set_to_no_file();
		}
		LeaveCriticalSection(&G->mutex);

	}
	free(file_data);
	free(filename_utf8);
    return result;
}

static int load_ppm_pre(wchar_t *path, u32 id, bool dropped, int* type) {
	int result = 0;
	unsigned int width = 0;
	unsigned int height = 0;
	unsigned int depth = 0;
	unsigned char* data = NULL;
	char header[4] = { 0 };
	uint8_t block[63]; // @hkva: greatest multiple of 3 in a cache line
	size_t block_read = 0;
	size_t cur_write = 0;
	FILE* f = NULL;
	G->files[id].loading = true;
	if (!(f = _wfopen(path, L"rb"))) {
		goto cleanup;
	}
	if (!fgets(header, sizeof(header), f) || strncmp(header, "P6\n", 3)) {
		goto cleanup;
	}
	// Skip header comments
	while (fgetc(f) == '#') {
		fscanf(f, "%*[^\n]");
	}
	fseek(f, -1, SEEK_CUR);
	if (fscanf(f, "%u %u\n", &width, &height) < 2) {
		goto cleanup;
	}
	if (fscanf(f, "%u", &depth) < 1 || depth != 255) {
		goto cleanup;
	}
	fseek(f, 1, SEEK_CUR);
	data = (unsigned char*)walloc(width * height * 4);
	for (size_t i = 0; i < width * height * 4; ++i) {
		data[i] = 0xFF;
	}
	// RRGGBB -> RRGGBBAA
	while ((block_read = fread(block, 1, sizeof(block), f)) > 0) {
		if (block_read % 3 != 0 || cur_write + (block_read / 3 * 4) > width * height * 4) {
			goto cleanup;
		}
		for (size_t i = 0; i < block_read / 3; ++i) {
			data[cur_write++] = block[i*3+0];
			data[cur_write++] = block[i*3+1];
			data[cur_write++] = block[i*3+2];
			data[cur_write++] = 0xFF;
		}
	}
	G->files[id].loading = false;
	if (width > G->graphics.MAX_GPU || height > G->graphics.MAX_GPU) {
		push_alert("Image is too large.");
		goto cleanup;
	}
	EnterCriticalSection(&G->mutex);
	if (id == G->current_file_index) {
		G->graphics.main_image.w = width;
		G->graphics.main_image.h = height;
		G->graphics.main_image.n = 4;
		// @hkva: freeing here causes corrupted image data to be displayed
		// same with load_webp_pre()
#if 0
		if (G->graphics.main_image.data) {
			wfree(G->graphics.main_image.data);
			G->graphics.main_image.data = 0;
		}
#endif
		G->graphics.main_image.data = data;
		send_signal(G->signals.init_step_2);
	} else {
		wfree(data);
	}
	LeaveCriticalSection(&G->mutex);
	result = 1;
cleanup:
	// if (data) free(data);
	if (f) fclose(f);
	return result;
}

void save_PPM() {
    const int dimx = 800, dimy = 800;
    int i, j;
    FILE *fp = fopen("save.ppm", "wb"); /* b - binary mode */
    (void)fprintf(fp, "P6\n%d %d\n255\n", G->graphics.main_image.w, G->graphics.main_image.h);
    for (i = 0; i < G->graphics.main_image.w * G->graphics.main_image.h * 4; i += 4) {
        static unsigned char color[3];
        color[0] = G->graphics.main_image.data[i + 0]; /* red */
        color[1] = G->graphics.main_image.data[i + 1]; /* green */
        color[2] = G->graphics.main_image.data[i + 2]; /* blue */
        (void)fwrite(color, 1, 3, fp);
    }
    (void)fclose(fp);
}

static int load_GIF_pre(wchar_t *File, u32 id, bool dropped) {
    int w, h;
    int frames;
    int *delays;
    int result = 0;

    unload_anim_image();

    G->files[id].loading = true;
    unsigned char *data = stbi_xload_file(File, &w, &h, &frames, &delays);
    G->files[id].loading = false;

    if (data != nullptr) {
        G->graphics.main_image.w = w;
        G->graphics.main_image.h = h;
        G->anim_frames = frames;
        G->anim_frame_delays = delays;
        G->anim_buffer = data;

        G->anim_index = 0;
        G->anim_play = G->settings_autoplayGIFs;

        if (id != G->current_file_index)
            unload_anim_image();
        else
            send_signal(G->signals.init_step_2);
        result = 1;
    } else {
        push_alert("Loading GIF file failed");
        G->files[G->current_file_index].failed = true;
        G->loaded = true;
        //if (dropped)
            //reset_to_no_folder();
        set_to_no_file();
    }
    return result;
}

struct Reduced_Frac {
    int n1, n2;
};

int gcd(int n, int m) {
    int gcd, rem;

    while (n != 0) {
        rem = m % n;
        m = n;
        n = rem;
    }
    gcd = m;

    return gcd;
}

Reduced_Frac reduced_fraction(int n1, int n2) {
    Reduced_Frac result;

    result.n1 = n1 / gcd(n1, n2);
    result.n2 = n2 / gcd(n1, n2);
    return result;
}

static void set_window_size(v2 display_size, int w, int h) {
    RECT rc  { 0, 0,  w, h };
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, 0);
    int x = (display_size.x - w) / 2;
    int y = (display_size.y - h) / 2;
    SetWindowPos(hwnd, NULL, x, y, w, h, SWP_NOZORDER | SWP_NOACTIVATE);
}

static void get_window_size() {
    RECT rect;
    GetClientRect (hwnd, &rect);
    WW = rect.right - rect.left;
    WH = rect.bottom - rect.top;
}

static void apply_scale(float true_scale)
{
	float TS = true_scale;
	TS = clamp(TS, 0.1, 500.f);

	if (G->graphics.aspect_img < G->graphics.aspect_wnd)
		G->scale = TS * (float)G->graphics.main_image.h / WH;
	else
		G->scale = TS * (float)G->graphics.main_image.w / WW;

	//if (!G->signals.setting_applied) G->position *= G->scale / prev_scale;
	send_signal(G->signals.update_truescale);
}

static void refresh_display() {
	if (is_fullscreen(hwnd) || G->settings_dont_resize) return;
    v2 display_size = v2(GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
    float aspect = (float)G->graphics.main_image.w / G->graphics.main_image.h;
    // printf("%i %i", Display.w, Display.h);
    if (G->graphics.main_image.w > display_size.x * 0.85 || G->graphics.main_image.h > display_size.y* 0.85) {
        if (aspect < 1)
            set_window_size(display_size, display_size.y * 0.85 * aspect, display_size.y * 0.85);
        else
            set_window_size(display_size, display_size.x * 0.85, display_size.y * 0.85);
    }
    else
        set_window_size(display_size, max(G->graphics.main_image.w, 700), max(G->graphics.main_image.h, 700));

    get_window_size();
}

static void fit_image_in()
{
	G->position = v2(0, 0);
	if (G->graphics.main_image.w > G->graphics.main_image.h)
		G->req_truescale = (float)WW / G->graphics.main_image.w;
	else 
		G->req_truescale = (float)WH / G->graphics.main_image.h;
	G->files[G->current_file_index].scaled = true;
}
static void fit_image_w()
{
	G->position = v2(0, 0);
	G->req_truescale = (float)WW / G->graphics.main_image.w;
	G->files[G->current_file_index].scaled = true;
}
static void fit_image_h()
{
	G->position = v2(0, 0);
	G->req_truescale = (float)WH / G->graphics.main_image.h;
	G->files[G->current_file_index].scaled = true;
}
static void fit_image_1()
{
	G->position = v2(0, 0);
	G->req_truescale = 1;
	G->files[G->current_file_index].scaled = true;
}

static void apply_settings() {
	bool dont_apply_scale = false;
    switch (G->settings_resetzoom) {
    case 0: dont_apply_scale = true; break;
    case 1: // Save zoom for each file
		if (G->files[G->current_file_index].scaled)
        	G->req_truescale = G->files[G->current_file_index].scale;
		else // use new file setting
		{
			switch (G->settings_newfilezoom)
			{
			case 0: fit_image_in(); break;
			case 1: fit_image_1(); break;
			}
		}
        break;
    case 2: fit_image_w(); break;
    case 3: fit_image_h(); break;
    case 4: fit_image_1();break;
    default: break;
    }
    switch (G->settings_resetpos) {
    case 0: // Persistent position across all files
        break;
    case 1: // Save position for each file
        G->position = G->files[G->current_file_index].pos;
        break;
    case 2: // Reset to center
        G->position = v2(0, 0);
        break;
    default:
        break;
    }
	if (!dont_apply_scale)
		apply_scale(G->req_truescale);
}
#include <d3d11.h>
static void load_image_post() {
    if (G->files[G->current_file_index].type == TYPE_GIF || G->files[G->current_file_index].type == TYPE_WEBP_ANIM) {
		if (G->anim_texture.d3d_texture != 0)
			G->anim_texture.d3d_texture->Release();
		if (G->anim_texture.d3d_texture == 0)
            refresh_display();

		G->anim_texture = create_texture(G->anim_buffer + G->anim_index * G->graphics.main_image.w * G->graphics.main_image.h * 4, 
		                                G->graphics.main_image.w, G->graphics.main_image.h, true);
    } else {
		if (G->graphics.main_image.texture.d3d_texture != 0)
			G->graphics.main_image.texture.d3d_texture->Release();

		if (G->graphics.main_image.texture.d3d_texture == 0)
            refresh_display();

		G->graphics.main_image.texture = create_texture(G->graphics.main_image.data, G->graphics.main_image.w, G->graphics.main_image.h, false);
		if (G->graphics.main_image.data) {
			wfree(G->graphics.main_image.data);
			//SetProcessWorkingSetSize(GetCurrentProcess(), -1, -1);
			G->graphics.main_image.data = 0;
		}
    }

    Reduced_Frac frac = reduced_fraction(G->graphics.main_image.w, G->graphics.main_image.h);
    G->graphics.main_image.frac1 = frac.n1;
    G->graphics.main_image.frac2 = frac.n2;
    G->graphics.main_image.aspect_ratio = (float)frac.n1 / frac.n2;

    wchar_t title[512];
    swprintf(title, array_size(title), L"CactusViewer %hs - %ws", VERSION, G->files[G->current_file_index].file.name);
    SetWindowTextW(hwnd, title);

    apply_settings();
}

DWORD WINAPI loader_thread(LPVOID lpParam) {
	EnterCriticalSection(&G->id_mutex);
    Loader_Thread_Inputs *inputs = (Loader_Thread_Inputs *)lpParam;
	G->alert.timer = 0;
	G->graphics.main_image.has_exif = 0;
	G->graphics.main_image.orientation = 0;
    if (!G->files[inputs->id].loading) {
		switch (inputs->file_data->type) {
			case TYPE_STB_IMAGE:  	load_image_pre(inputs->file_data->file.path, inputs->id, inputs->dropped); 							break;
			case TYPE_GIF: 			load_GIF_pre(inputs->file_data->file.path, inputs->id, inputs->dropped); 							break;
			case TYPE_WEBP: 		load_webp_pre(inputs->file_data->file.path, inputs->id, inputs->dropped, &inputs->file_data->type); break;
			case TYPE_PPM: 			load_ppm_pre(inputs->file_data->file.path, inputs->id, inputs->dropped, &inputs->file_data->type); break;
			case TYPE_MISC: 		load_image_wic_pre(inputs->file_data->file.path, inputs->id, inputs->dropped, inputs->file_data); 	break;
		}
    }
	LeaveCriticalSection(&G->id_mutex);
	SetEvent(G->loader_event);
    return 0;
}

static int check_valid_extention(wchar_t *EXT) {
    const int length = wcslen(EXT);
    wchar_t *ext = (wchar_t *)malloc((length + 1) * sizeof(wchar_t));
    ext[0] = '.';
    for (int i = 1; i < length + 1; i++)
        ext[i] = tolower(EXT[i]);

    int result = TYPE_UNKNOWN;
//    if      (wcscmp(ext, L".png")  == 0)	result = TYPE_STB_IMAGE;
//    else if (wcscmp(ext, L".jpg")  == 0)	result = TYPE_STB_IMAGE;
//    else if (wcscmp(ext, L".jpeg") == 0)	result = TYPE_STB_IMAGE;
//    else if (wcscmp(ext, L".bmp")  == 0)	result = TYPE_STB_IMAGE;
	if (wcscmp(ext, L".gif")  == 0)	result = TYPE_GIF;
    else if (wcscmp(ext, L".webp") == 0)	result = TYPE_WEBP;
    // Portable PixMap
    else if (wcscmp(ext, L".ppm") == 0)		result = TYPE_PPM;
	// WIC formats:
	else if (wcscmp(ext, L".3fr")   == 0)   result = TYPE_MISC;
	else if (wcscmp(ext, L".ari") 	== 0)   result = TYPE_MISC;
	else if (wcscmp(ext, L".arw")   == 0)   result = TYPE_MISC;
	else if (wcscmp(ext, L".avci")  == 0)   result = TYPE_MISC;
	else if (wcscmp(ext, L".avcs")  == 0)   result = TYPE_MISC;
	else if (wcscmp(ext, L".avif")  == 0)   result = TYPE_MISC;
	else if (wcscmp(ext, L".avifs") == 0)   result = TYPE_MISC;
	else if (wcscmp(ext, L".bay")   == 0)   result = TYPE_MISC;
	else if (wcscmp(ext, L".bmp")   == 0)   result = TYPE_MISC;
	else if (wcscmp(ext, L".cap")   == 0)   result = TYPE_MISC;
	else if (wcscmp(ext, L".cr2")   == 0)   result = TYPE_MISC;
	else if (wcscmp(ext, L".cr3")   == 0)   result = TYPE_MISC;
	else if (wcscmp(ext, L".crw")   == 0)   result = TYPE_MISC;
	else if (wcscmp(ext, L".cur")   == 0)   result = TYPE_MISC;
	else if (wcscmp(ext, L".dcr")   == 0)   result = TYPE_MISC;
	else if (wcscmp(ext, L".dcs")   == 0)   result = TYPE_MISC;
	else if (wcscmp(ext, L".dds")   == 0)   result = TYPE_MISC;
	else if (wcscmp(ext, L".dib")   == 0)   result = TYPE_MISC;
	else if (wcscmp(ext, L".dng")   == 0)   result = TYPE_MISC;
	else if (wcscmp(ext, L".drf")   == 0)   result = TYPE_MISC;
	else if (wcscmp(ext, L".eip")   == 0)   result = TYPE_MISC;
	else if (wcscmp(ext, L".erf")   == 0)   result = TYPE_MISC;
	else if (wcscmp(ext, L".exif")  == 0)   result = TYPE_MISC;
	else if (wcscmp(ext, L".fff")   == 0)   result = TYPE_MISC;
	else if (wcscmp(ext, L".gif")   == 0)   result = TYPE_MISC;
	else if (wcscmp(ext, L".heic")  == 0)   result = TYPE_MISC;
	else if (wcscmp(ext, L".heics") == 0)   result = TYPE_MISC;
	else if (wcscmp(ext, L".heif")  == 0)   result = TYPE_MISC;
	else if (wcscmp(ext, L".heifs") == 0)   result = TYPE_MISC;
	else if (wcscmp(ext, L".hif")   == 0)   result = TYPE_MISC;
	else if (wcscmp(ext, L".ico")   == 0)   result = TYPE_MISC;
	else if (wcscmp(ext, L".icon")  == 0)   result = TYPE_MISC;
	else if (wcscmp(ext, L".iiq")   == 0)   result = TYPE_MISC;
	else if (wcscmp(ext, L".jfif")  == 0)   result = TYPE_MISC;
	else if (wcscmp(ext, L".jpe")   == 0)   result = TYPE_MISC;
	else if (wcscmp(ext, L".jpeg")  == 0)   result = TYPE_MISC;
	else if (wcscmp(ext, L".jpg")   == 0)   result = TYPE_MISC;
	else if (wcscmp(ext, L".jxr")   == 0)   result = TYPE_MISC;
	else if (wcscmp(ext, L".k25")   == 0)   result = TYPE_MISC;
	else if (wcscmp(ext, L".kdc")   == 0)   result = TYPE_MISC;
	else if (wcscmp(ext, L".mef")   == 0)   result = TYPE_MISC;
	else if (wcscmp(ext, L".mos")   == 0)   result = TYPE_MISC;
	else if (wcscmp(ext, L".mrw")   == 0)   result = TYPE_MISC;
	else if (wcscmp(ext, L".nef")   == 0)   result = TYPE_MISC;
	else if (wcscmp(ext, L".nrw")   == 0)   result = TYPE_MISC;
	else if (wcscmp(ext, L".orf")   == 0)   result = TYPE_MISC;
	else if (wcscmp(ext, L".ori")   == 0)   result = TYPE_MISC;
	else if (wcscmp(ext, L".pef")   == 0)   result = TYPE_MISC;
	else if (wcscmp(ext, L".png")   == 0)   result = TYPE_MISC;
	else if (wcscmp(ext, L".ptx")   == 0)   result = TYPE_MISC;
	else if (wcscmp(ext, L".pxn")   == 0)   result = TYPE_MISC;
	else if (wcscmp(ext, L".raf")   == 0)   result = TYPE_MISC;
	else if (wcscmp(ext, L".raw")   == 0)   result = TYPE_MISC;
	else if (wcscmp(ext, L".rle")   == 0)   result = TYPE_MISC;
	else if (wcscmp(ext, L".rw2")   == 0)   result = TYPE_MISC;
	else if (wcscmp(ext, L".rwl")   == 0)   result = TYPE_MISC;
	else if (wcscmp(ext, L".sr2")   == 0)   result = TYPE_MISC;
	else if (wcscmp(ext, L".srf")   == 0)   result = TYPE_MISC;
	else if (wcscmp(ext, L".srw")   == 0)   result = TYPE_MISC;
	//else if (wcscmp(ext, L".svg")   == 0)   result = TYPE_MISC;
	//else if (wcscmp(ext, L".svgz")  == 0)   result = TYPE_MISC;
	else if (wcscmp(ext, L".tif")   == 0)   result = TYPE_MISC;
	else if (wcscmp(ext, L".tiff")  == 0)   result = TYPE_MISC;
	else if (wcscmp(ext, L".wdp")   == 0)   result = TYPE_MISC;
	else if (wcscmp(ext, L".webp")  == 0)   result = TYPE_MISC;
	else if (wcscmp(ext, L".x3f")   == 0)   result = TYPE_MISC;

    free(ext);
    return result;
}
static void remove_char(wchar_t *str, wchar_t ch) {
    int len = wcslen(str);

    for (int i = 0; i < len; i++) {
        if (str[i] == ch) {
            for (int j = i; j < len; j++) {
                str[j] = str[j + 1];
            }
            len--;
            i--;
        }
    }
}

bool is_valid_windows_path(wchar_t* path)  {
    bool isValid = false;
    
    if (PathFileExistsW(path))  {
        if (PathIsRelativeW(path) == FALSE)  {
            wchar_t drive[3];
            wcsncpy(drive, path, 2);
            drive[2] = '\0';
            UINT driveType = GetDriveTypeW(drive);
            if (driveType == DRIVE_FIXED || driveType == DRIVE_CDROM || driveType == DRIVE_RAMDISK ||
                driveType == DRIVE_REMOTE || driveType == DRIVE_REMOVABLE)  {
                isValid = true;
            }
        }
    }
    
    return isValid;
}

struct Folder_Entry  { 
    wchar_t wpath[MAX_PATH];
	char path[MAX_PATH];
};
Folder_Entry *files_in_folder;

bool string_equal(const wchar_t *a, const wchar_t *b)  {
	while (true)  {
		if (*a != *b) return false;
		if (*a == 0) return true;
		a++, b++;
	}
}

void string_copy(wchar_t *d, const wchar_t *s)  {
	while (true)  {
		wchar_t c = *s++;
		*d++ = c;
		if (!c) break;
	}
}

void string_append(wchar_t *d, const wchar_t *s)  {
	while (*d) d++;
	string_copy(d, s);
}
wchar_t *get_wc(char *c) {
    const size_t cSize = strlen(c) + 1;
    wchar_t* wc = new wchar_t[cSize];
    mbstowcs(wc, c, cSize);

    return wc;
}
void get_c(wchar_t* wc, char* target) {
    const size_t wcSize = wcslen(wc) + 1;
	if (wcSize >= MAX_PATH) assert(!"max path length!");
	wcstombs(target, wc, wcSize);
}
int items_in_folder;

int cmp(const void* a, const void* b)  {
    File_Data* A = (File_Data*)a;
    File_Data* B = (File_Data*)b;
    if      (A->index > B->index)  return  1; 
    else if (A->index < B->index)  return -1; 
    else                           return  0; 
}

static void sort_folder() {
    for (int i =0; i < G->files.Count; i++)
        for (int j =0; j < items_in_folder; j++)
            if (wcscmp(G->files[i].file.path, files_in_folder[j].wpath) == 0) {
                G->files[i].index = j;
                break;
            }
    qsort(G->files.Data, G->files.Count, sizeof(File_Data), cmp);
}

struct Folder_Sort_Thread_data {
    wchar_t *path;
    wchar_t *FileName;
};

DWORD WINAPI folder_sort_thread(LPVOID lpParam) {
    EnterCriticalSection(&G->sort_mutex);
    G->sorting = true;

    Folder_Sort_Thread_data *data = (Folder_Sort_Thread_data *)lpParam;

    wchar_t *file_path = data->path;
    wchar_t *file_name = data->FileName;
	wchar_t path_buffer[MAX_PATH + 4];
    static int index_in_folder;
    
	IShellWindows *shellWindows = NULL;
	CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
	if (S_OK != CoCreateInstance(CLSID_ShellWindows, NULL, CLSCTX_ALL, IID_IShellWindows, (void **) &shellWindows)) return 0;
	
	IDispatch *dispatch = NULL;
	VARIANT v  {};
	V_VT(&v) = VT_I4;
	
	for (V_I4(&v) = 0; S_OK == shellWindows->Item(v, &dispatch); V_I4(&v)++)  {
		bool success = false;
		
		IFolderView *folderView = NULL;	
		IWebBrowserApp *webBrowserApp = NULL;
		IServiceProvider *serviceProvider = NULL;
		IShellBrowser *shellBrowser = NULL;
		IShellView *shellView = NULL;
		IPersistFolder2 *persistFolder = NULL;
		ITEMIDLIST *folderPIDL = NULL;
		ITEMIDLIST *itemPIDL = NULL;
		PIDLIST_ABSOLUTE fullPIDL = NULL;
		
		int itemCount = 0, focusedItem = 0;
		path_buffer[0] = 0;
		
		if (S_OK != dispatch->QueryInterface(IID_IWebBrowserApp, (void **) &webBrowserApp)) goto Alert;
		if (S_OK != webBrowserApp->QueryInterface(IID_IServiceProvider, (void **) &serviceProvider)) goto Alert;
		if (S_OK != serviceProvider->QueryService(SID_STopLevelBrowser, IID_IShellBrowser, (void **) &shellBrowser)) goto Alert;
		if (S_OK != shellBrowser->QueryActiveShellView(&shellView)) goto Alert;
		if (S_OK != shellView->QueryInterface(IID_IFolderView, (void **) &folderView)) goto Alert;
		if (S_OK != folderView->GetFolder(IID_IPersistFolder2, (void **) &persistFolder)) goto Alert;
		if (S_OK != persistFolder->GetCurFolder(&folderPIDL)) goto Alert;
		if (S_OK != folderView->GetFocusedItem(&focusedItem)) goto Alert;
		if (S_OK != folderView->Item(focusedItem, &itemPIDL)) goto Alert;
		fullPIDL = ILCombine(folderPIDL, itemPIDL);
		if (!SHGetPathFromIDListW(fullPIDL, path_buffer)) goto Alert;
		if (!string_equal(file_path, path_buffer)) goto Alert;
		if (S_OK != folderView->ItemCount(SVGIO_ALLVIEW, &itemCount)) goto Alert;

		if (!(files_in_folder = (Folder_Entry *) malloc(itemCount * sizeof(Folder_Entry)))) goto Alert;
		
		for (int i = 0; i < itemCount; i++)  {
			files_in_folder[i].wpath[0] = 0;
			ITEMIDLIST *itemPIDL = NULL;
			if (S_OK != folderView->Item(i, &itemPIDL)) continue;
			PIDLIST_ABSOLUTE fullPIDL = ILCombine(folderPIDL, itemPIDL);
			SHGetPathFromIDListW(fullPIDL, files_in_folder[i].wpath);
			CoTaskMemFree(fullPIDL);
			CoTaskMemFree(itemPIDL);
		}
		
		items_in_folder = itemCount;
		index_in_folder = focusedItem;
	
		success = true;
		Alert:;
		
		if (fullPIDL) CoTaskMemFree(fullPIDL);
		if (folderPIDL) CoTaskMemFree(folderPIDL);
		if (itemPIDL) CoTaskMemFree(itemPIDL);
		if (persistFolder) persistFolder->Release();
		if (folderView) folderView->Release();
		if (shellView) shellView->Release();
		if (shellBrowser) shellBrowser->Release();
		if (serviceProvider) serviceProvider->Release();
		if (webBrowserApp) webBrowserApp->Release();
		if (dispatch) dispatch->Release();
		
		if (success) break;
	}
	
	shellWindows->Release();

	if (files_in_folder) {
    
        for(int i = 0; i < items_in_folder; i++) {
			get_c(files_in_folder[i].wpath, files_in_folder[i].path);
        }

        sort_folder();

        for (int i = 0; i < G->files.Count; i++) {
            if (wcscmp(file_name, G->files[i].file.name) == 0) {
				EnterCriticalSection(&G->id_mutex);
                G->current_file_index = i;
				LeaveCriticalSection(&G->id_mutex);
			}
            G->files[i].loading = false;
            G->files[i].failed = false;
        }
    } 

	free(files_in_folder);

    free(data->FileName);
    free(data->path);
    G->sorting = false;
    LeaveCriticalSection(&G->sort_mutex);
	CoUninitialize();
    return 0;
}

DWORD WINAPI thumbs_thread(LPVOID lpParam) {

	IWICImagingFactory* pFactory = nullptr;
	IWICBitmapScaler* pScaler = nullptr;
	IWICBitmapClipper* pClipper = nullptr;
	IWICFormatConverter* pConverter = nullptr;
	CoInitialize(NULL);
	CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pFactory));
	pFactory->CreateBitmapScaler(&pScaler);
	pFactory->CreateBitmapClipper(&pClipper);
	pFactory->CreateFormatConverter(&pConverter);

	i32 index_pro = clamp(G->current_file_index, 0, G->files.Count - 1);
	i32 index_retro = clamp(G->current_file_index - 1, 0, G->files.Count - 1);

	//for (int i = 0; i < G->files.Count; i++)
	bool turn_pro = 0;
	while(true)
	{

		i32 i = turn_pro ? index_pro : index_retro;
		if (turn_pro && index_pro > G->files.Count - 1) {
			i = index_retro;
			turn_pro = false;
		} else if (!turn_pro && index_retro < 0) {
			i = index_pro;
			turn_pro = true;
		}

		cf_file_t *file = &G->files[i].file;

		EnterCriticalSection(&G->thumbs_mutex);
		i32 thumb_dim = THUMBS_DIM;

		IWICBitmapDecoder* pDecoder = nullptr;
		IWICBitmapSource* pThumbnail = nullptr;
		IWICBitmapSource* pSource = nullptr;
		IWICBitmapFrameDecode* pFrame = nullptr;
		IWICFormatConverter* pConverter = nullptr;
		IWICBitmapClipper* pClipper = nullptr;
		IWICBitmapScaler* pScaler = nullptr;

		HRESULT hr = pFactory->CreateDecoderFromFilename(file->path, nullptr, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pDecoder);
		if (SUCCEEDED(hr)) hr = pDecoder->GetThumbnail(&pThumbnail);

		if (FAILED(hr) && pDecoder) hr = pDecoder->GetFrame(0, &pFrame);
		if (SUCCEEDED(hr)) pSource = pThumbnail ? pThumbnail : pFrame;

		if (SUCCEEDED(hr)) hr = pFactory->CreateFormatConverter(&pConverter);
		if (SUCCEEDED(hr)) hr = pConverter->Initialize(pFrame, GUID_WICPixelFormat32bppRGBA, WICBitmapDitherTypeNone, NULL, 0.0, WICBitmapPaletteTypeCustom);
		if (SUCCEEDED(hr)) pSource = pConverter;

		UINT o_w, o_h;
		if (SUCCEEDED(hr)) hr = pSource->GetSize(&o_w, &o_h);

		UINT sq_l = min(o_w, o_h);
		UINT offsetX = (o_w - sq_l) / 2;
		UINT offsetY = (o_h - sq_l) / 2;
		WICRect rcClip = { (int)(offsetX), (int)(offsetY), (int)(sq_l), (int)(sq_l) };

		if (SUCCEEDED(hr)) hr = pFactory->CreateBitmapClipper(&pClipper);
		if (SUCCEEDED(hr)) hr = pClipper->Initialize(pSource, &rcClip);
		if (SUCCEEDED(hr)) hr = pFactory->CreateBitmapScaler(&pScaler);
		if (SUCCEEDED(hr)) hr = pScaler->Initialize(pClipper, thumb_dim, thumb_dim, WICBitmapInterpolationModeFant);

		UINT stride = thumb_dim * 4;
		UINT bufferSize = stride * thumb_dim;
		BYTE* buffer = nullptr;
		if (SUCCEEDED(hr)) {
			buffer = (BYTE*)(malloc(bufferSize));
			memset(buffer, 0, bufferSize);
			hr = pScaler->CopyPixels(nullptr, stride, bufferSize, buffer);
		}

		if (SUCCEEDED(hr)) {
			int n = i;
			const UINT texture_size = 4000;
			const UINT max_thumbnails_per_row = texture_size / thumb_dim;

			UINT start_x = (n % max_thumbnails_per_row) * thumb_dim;
			UINT start_y = (n / max_thumbnails_per_row) * thumb_dim;
			UINT end_x = start_x + thumb_dim;
			UINT end_y = start_y + thumb_dim;

			D3D11_BOX dst_box = { start_x, start_y, 0, end_x, end_y, 1 };
			G->graphics.device_ctx->UpdateSubresource(G->graphics.thumbs.d3d_texture, 0, &dst_box, buffer, stride, bufferSize);
		}

		if (pSource && pSource != pThumbnail && pSource != pConverter) pSource->Release();
		if (buffer) 	free(buffer);
		if (pThumbnail) pThumbnail->Release();
		if (pFrame)		pFrame->Release();
		if (pConverter) pConverter->Release();
		if (pClipper) 	pClipper->Release();
		if (pScaler) 	pScaler->Release();
		if (pDecoder) 	pDecoder->Release();

		if (turn_pro)
			index_pro++;
		else
			index_retro--;

		turn_pro = !turn_pro;

		G->files[i].thumb_loaded = true;
		LeaveCriticalSection(&G->thumbs_mutex);

		if (index_pro > G->files.Count - 1 && index_retro < 0)
			break;
		if(G->signals.new_folder)
			break;
	}

	if (pClipper) pClipper->Release();
	if (pScaler) pScaler->Release();
	if (pFactory) pFactory->Release();
	CoUninitialize();

    return 0;
}


Folder_Sort_Thread_data sort_data;

#define SCAN_FAILED 0
#define SCAN_FILE 1
#define SCAN_DIR 2

static int scan_folder(wchar_t *path) {


	int result = SCAN_FILE;
    if (path == nullptr) {
        G->files.reset_count();
		return SCAN_DIR;
    }


    int len = wcslen(path);
    wchar_t *BasePath = nullptr;
    wchar_t *FileName = nullptr;
    wchar_t *FullPath = nullptr;
    int newlen = len;

	if (PathIsDirectoryW(path))  {
		BasePath = (wchar_t*)malloc((len + 2) * sizeof(wchar_t));
		memcpy(BasePath, path, (len + 1) * sizeof(wchar_t));
        BasePath[len] = '\\';
        BasePath[len + 1] = 0;
        FileName = (wchar_t*)malloc(8 * sizeof(wchar_t));
        swprintf(FileName, 8, L"none");
		result = SCAN_DIR;
	} else {
		remove_char(path, '/"');

		for (int i = len - 1; i > 0; i--) {
			if (path[i] == '/' || path[i] == '\\') {
				newlen = i + 1;
				break;
			}
		}

		BasePath = (wchar_t *)malloc((newlen + 1) * sizeof(wchar_t));
		FileName = (wchar_t *)malloc((len - newlen + 1) * sizeof(wchar_t));
		memcpy(FileName, &path[newlen], (len - newlen) * sizeof(wchar_t));
		memcpy(BasePath, path, newlen * sizeof(wchar_t));
		BasePath[newlen] = '\0';
		FileName[len - newlen] = '\0';
        
        if (BasePath[0] && !FileName[0]) {
            // When user executes program with something like: "C:\path\CactusViewer.exe" SomeImage.png
            // FileName will be empty and BasePath will be the image name... swap and set BasePath to current working directory.
            free(FileName);
            FileName = BasePath;
            const size_t name_len = wcslen(CURRENT_FOLDER) + 2;
            BasePath = (wchar_t *)malloc(name_len * sizeof(wchar_t));
            swprintf(BasePath, name_len, L"%ls%lc", CURRENT_FOLDER, L'\\');
        } else if (PathFileExistsW(BasePath) && PathIsRelativeW(BasePath)) {
            // If BasePath is relative, append it to current working directory.
            wchar_t *Tmp = BasePath;
            const size_t name_len = wcslen(Tmp) + wcslen(CURRENT_FOLDER) + 2;
            BasePath = (wchar_t *)malloc(name_len * sizeof(wchar_t));
            swprintf(BasePath, name_len, L"%ls\\%ls%lc", CURRENT_FOLDER, Tmp, L'\\');
            free(Tmp);
        }
		
		const size_t name_len = wcslen(BasePath) + wcslen(FileName) + 1;
        FullPath = (wchar_t *)malloc(name_len * sizeof(wchar_t));
        swprintf(FullPath, name_len, L"%ls%ls%lc", BasePath, FileName, L'\0');
	}
    if (!is_valid_windows_path(BasePath))  {
        G->files.reset_count();
		return SCAN_DIR;
    }

    cf_dir_t dir;
    cf_dir_open(&dir, BasePath);

	send_signal(G->signals.new_folder);

	EnterCriticalSection(&G->thumbs_mutex);

    G->files.reset_count();

    while (dir.has_next) {
        cf_file_t file_0;
        cf_read_file(&dir, &file_0);

        if (file_0.is_dir) {
            cf_dir_next(&dir);
            continue;
        }
        remove_char(file_0.path, '/');

		int type = check_valid_extention(file_0.ext); 
		if (type == TYPE_UNKNOWN) {
			if (wcscmp(FileName, file_0.name) == 0) {
				char err[128] = { 0 };
				sprintf(err, "Cannot open files of the type '%S'.", file_0.ext);
				push_alert(err, Alert_Error);
				result = SCAN_FAILED;
				break;
			}
            cf_dir_next(&dir);
            continue;
        }
		File_Data new_file;
        G->files.push_back(new_file);
		G->files.back().type = type;

        cf_file_t *file = &G->files.back().file;
        *file = file_0;
	    stbi_convert_wchar_to_utf8(file->name_utf8, 1024, file->name);

        cf_dir_next(&dir);
    }
    cf_dir_close(&dir);

	if (result == SCAN_FAILED)
		goto cleanup;

    if (!G->sorting && G->settings_sort) {   
        sort_data.FileName = (wchar_t*)malloc((wcslen(FileName) + 1) * sizeof(wchar_t));
        memcpy(sort_data.FileName, FileName,  (wcslen(FileName) + 1) * sizeof(wchar_t));
        sort_data.path =     (wchar_t*)malloc((wcslen(FullPath) + 1) * sizeof(wchar_t));
        memcpy(sort_data.path, FullPath,      (wcslen(FullPath) + 1) * sizeof(wchar_t));
        //CreateThread(NULL, 0, folder_sort_thread, (LPVOID)&sort_data, 0, NULL);
		folder_sort_thread((LPVOID)&sort_data); // seems to be performant and not need to be multithreadded
	} else {
		G->current_file_index = 0;
	}

	for (int i = 0; i < G->files.Count; i++) {
		if (wcscmp(FileName, G->files[i].file.name) == 0) {
			G->current_file_index = i;
			break;
		}
	}
	cleanup:
    free(BasePath);    
    free(FileName);  
	
	LeaveCriticalSection(&G->thumbs_mutex);

	G->signals.new_folder = false;

	if (G->settings_preview_thumbs)
		CreateThread(NULL, 0, thumbs_thread, 0, 0, NULL);
	
    return result;
}



unsigned long create_RBG(int r, int g, int b) {
    return (r << 16) | (g << 8) | b;
}


static GUID get_GUID(Encoder_Format encoder_format) {

	switch (encoder_format) {
		case Format_Bmp:	return GUID_ContainerFormatBmp;
		case Format_Png:	return GUID_ContainerFormatPng;
		//case Format_Ico:	return GUID_ContainerFormatIco;
		//case Format_Jpeg:	return GUID_ContainerFormatJpeg;
		case Format_Tiff:	return GUID_ContainerFormatTiff;
		//case Format_Gif:	return GUID_ContainerFormatGif;
		//case Format_Wmp:	return GUID_ContainerFormatWmp;
		case Format_Dds:	return GUID_ContainerFormatDds;
		//case Format_Adng:	return GUID_ContainerFormatAdng;
		case Format_Heif:	return GUID_ContainerFormatHeif;
		//case Format_Webp:	return GUID_ContainerFormatWebp;
		//case Format_Raw:	return GUID_ContainerFormatRaw;

		default:  			return GUID_ContainerFormatJpeg;
	}
}

static HRESULT save_image(Encoder_Format encoder_format, wchar_t* path, bool clipboard = false) {
	Graphics* ctx = &G->graphics;

	IWICBitmapEncoder* encoder = 0;
	IWICStream* stream = 0;
	IWICBitmapFrameEncode* frame = 0;

	HRESULT hr = 0;
	if (!clipboard && path == 0) {
		push_alert("Failed to fetch file path from `save as` dialogue.");
		return -1;
	}

	DXGI_FORMAT image_format = DXGI_FORMAT_B8G8R8A8_UNORM;

	v2 new_size = _v2(G->crop_b - G->crop_a);

	// create an offscreen texture
	D3D11_TEXTURE2D_DESC tex_desc = { };
	tex_desc.Width =  new_size.x; 
	tex_desc.Height =  new_size.y;
	tex_desc.MipLevels = 1;
	tex_desc.ArraySize = 1;
	tex_desc.Format = image_format;
	tex_desc.SampleDesc.Count = 1; 
	tex_desc.SampleDesc.Quality = 0;
	tex_desc.Usage = D3D11_USAGE_DEFAULT;
	tex_desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	tex_desc.CPUAccessFlags = 0;
	tex_desc.MiscFlags = 0;
	ID3D11Texture2D* offscreen_texture = nullptr;
	ID3D11RenderTargetView* offscreen_texture_rtv = nullptr;
	hr = ctx->device->CreateTexture2D(&tex_desc, NULL, &offscreen_texture);
	hr = ctx->device->CreateRenderTargetView(offscreen_texture, NULL, &offscreen_texture_rtv);

	// run the shaders on it
	ctx->device_ctx->ClearState();
	Shader_Constants_Main constants_main = set_main_shader_constants();
	constants_main.render_mode = RENDER_MODE_ENCODER;
	constants_main.aspect_img = new_size.x / new_size.y;
	upload_constants(&ctx->main_program, &constants_main);
	D3D11_VIEWPORT vp;
	vp.Width = new_size.x;
	vp.Height = new_size.y;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	ctx->device_ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	ctx->device_ctx->IASetInputLayout(nullptr);
	ctx->device_ctx->VSSetShader(ctx->main_program.vertex_shader, nullptr, 0);
	ctx->device_ctx->VSSetConstantBuffers(0, 1, &ctx->main_program.constants_buffer);
	ctx->device_ctx->PSSetShader(ctx->main_program.pixel_shader, nullptr, 0);
	ctx->device_ctx->PSSetConstantBuffers(0, 1, &ctx->main_program.constants_buffer);
	ctx->device_ctx->PSSetShaderResources(0, 1, &G->graphics.main_image.texture.srv);
	ctx->device_ctx->PSSetSamplers(0, 1, &ctx->sampler_nearest);
	ctx->device_ctx->RSSetState(ctx->raster_state);
	ctx->device_ctx->RSSetViewports(1, &vp);
	ctx->device_ctx->OMSetRenderTargets(1, &offscreen_texture_rtv, nullptr); // we don't need a depth/stencil view here
	ctx->device_ctx->Draw(4, 0);

	// Create a staging texture with the same format as the render target
	D3D11_TEXTURE2D_DESC staging_desc = {};
	staging_desc.Width = new_size.x;
	staging_desc.Height = new_size.y;
	staging_desc.MipLevels = 1;
	staging_desc.ArraySize = 1;
	staging_desc.SampleDesc.Count = 1;
	staging_desc.Format = image_format;
	staging_desc.Usage = D3D11_USAGE_STAGING;
	staging_desc.BindFlags = 0;
	staging_desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;

	ID3D11Texture2D* staging_texture = nullptr;
	hr = ctx->device->CreateTexture2D(&staging_desc, nullptr, &staging_texture);
	if (FAILED(hr)) {
		goto cleanup;
	}

	// Copy the whole texture from the render target to the staging texture
	ctx->device_ctx->CopySubresourceRegion(staging_texture, 0, 0, 0, 0, offscreen_texture, 0, 0);

	// Map the staging texture
	D3D11_MAPPED_SUBRESOURCE mapped_resource;
	hr = ctx->device_ctx->Map(staging_texture, 0, D3D11_MAP_READ, 0, &mapped_resource);
	if (FAILED(hr)) {
		goto cleanup;
	}

	u8* data = reinterpret_cast<u8*>(mapped_resource.pData);
	if (data == nullptr) {
		printf("invalid data ptr\n");
		goto cleanup;
	}

	if (clipboard) {
		// @hkva: GdiPlus required here as Gdi32 does not easily support bitmaps with alpha channels.
		//	      First create a device-independent bitmap (GdiPlus::Bitmap), then convert to a
		//		  device-compatible bitmap for WinAPI use.
		// References:
		//        https://github.com/nakst/imgview/blob/d3e918b7f65cfb5a595b1b756c69cd1c1a30b13e/imgview.cpp#L574

		Gdiplus::Bitmap dib(new_size.x, new_size.y, mapped_resource.RowPitch, PixelFormat32bppARGB, data);
		if (dib.GetLastStatus() != Gdiplus::Ok) {
			printf("Failed to create GDI+ bitmap\n");
			return 1;
		}

		HBITMAP hdib;
		dib.GetHBITMAP(0, &hdib);

		DIBSECTION hdibs;
		GetObject(hdib, sizeof(hdibs), &hdibs);

		HBITMAP ddb = CreateDIBitmap(hdc, &hdibs.dsBmih, CBM_INIT, hdibs.dsBm.bmBits, (const BITMAPINFO*)&hdibs.dsBmih, DIB_RGB_COLORS);
		if (!ddb) {
			printf("Failed to to convert GDI+ bitmap to a device-compatible bitmap\n");
			return 1;
		}

		if (!OpenClipboard(NULL)) {
			printf("Failed to open clipboard\n");
			return 1;
		}
		EmptyClipboard();
		SetClipboardData(CF_BITMAP, ddb);
		CloseClipboard();

		DeleteObject(ddb);

		goto cleanup;
	}

	// Read the pixel value BGRA (!)
	CoInitialize(NULL);
	IPropertyBag2 *property_bag = NULL;
	WICPixelFormatGUID req_pixel_format = GUID_WICPixelFormat32bppBGRA; // Direct3D defaults to BGRA for some reason
	WICPixelFormatGUID pixel_format = req_pixel_format; 
	hr = CoCreateInstance(CLSID_WICImagingFactory,
	                      NULL,
	                      CLSCTX_INPROC_SERVER,
	                      IID_IWICImagingFactory,
	                      (LPVOID*) & G->wic_factory);
	UINT width = new_size.x;
	UINT height = new_size.y;
	UINT stride = mapped_resource.RowPitch;
	UINT buffer_size = stride * height;
	if (SUCCEEDED(hr))	hr = G->wic_factory->CreateStream(&stream);
	if (SUCCEEDED(hr))	hr = stream->InitializeFromFilename(path, GENERIC_WRITE);
	if (SUCCEEDED(hr))	hr = G->wic_factory->CreateEncoder(get_GUID(encoder_format), nullptr, &encoder);
	if (SUCCEEDED(hr))	hr = encoder->Initialize(stream, WICBitmapEncoderNoCache);
	if (SUCCEEDED(hr))	hr = encoder->CreateNewFrame(&frame, &property_bag);
	if (SUCCEEDED(hr))	hr = frame->Initialize(property_bag);  // No encoder parameters
	if (SUCCEEDED(hr))	hr = frame->SetSize(width, height);
	if (SUCCEEDED(hr))	hr = frame->SetPixelFormat(&pixel_format);
	//	if (SUCCEEDED(hr)) // We're expecting to write out 32bppBGRA. Fail if the encoder cannot do it.
	//		hr = IsEqualGUID(req_pixel_format, pixel_format) ? S_OK : E_FAIL;
	if (SUCCEEDED(hr)) 	hr = frame->WritePixels(height, stride, buffer_size, data);
	if (SUCCEEDED(hr)) 	hr = frame->Commit();
	if (SUCCEEDED(hr)) 	hr = encoder->Commit();
	if (SUCCEEDED(hr)) {
		printf("saved!\n");
	} else {
		printf("failed!\n");
	}

	cleanup:

	if (frame) frame->Release();
	if (stream) stream->Release();
	if (encoder) encoder->Release();

	if (offscreen_texture_rtv) offscreen_texture_rtv->Release();
	if (offscreen_texture) offscreen_texture->Release();

	if (staging_texture) ctx->device_ctx->Unmap(staging_texture, 0);
	if (staging_texture) staging_texture->Release();

	CoUninitialize();

	return hr;
}


HRESULT save_as_dialogue() {
	IFileSaveDialog *dialogue = 0;
	IShellItem *item = 0;
	PWSTR file_path = 0;
	UINT selected_encoder_index = 0;

	CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
	HRESULT hr = CoCreateInstance(CLSID_FileSaveDialog, NULL, CLSCTX_ALL, 
	                              IID_IFileSaveDialog, reinterpret_cast<void**>(&dialogue));	

	if (dialogue == 0)
		goto cleanup;
	if (SUCCEEDED(hr))
		hr = dialogue->SetDefaultExtension(L"");
	if (SUCCEEDED(hr))
		hr = dialogue->SetFileTypes(array_size(encoder_formats_filter), encoder_formats_filter);
	if (SUCCEEDED(hr))
		hr = dialogue->Show(NULL);
	if (SUCCEEDED(hr))
		hr = dialogue->GetResult(&item);
	if (SUCCEEDED(hr))
		hr = dialogue->GetFileTypeIndex(&selected_encoder_index);
	if (SUCCEEDED(hr))
		hr = item->GetDisplayName(SIGDN_FILESYSPATH, &file_path);
	if (SUCCEEDED(hr)) {
		hr = save_image((Encoder_Format)(selected_encoder_index - 1), file_path);
	}

	cleanup:

	if (item) 		item->Release();
	if (dialogue) 	dialogue->Release();
	if (file_path)	free(file_path);

	CoUninitialize();

	return hr;
}


void file_open_dialogue() {
    IFileOpenDialog *dialogue = 0;
	IShellItem *item = 0;
	PWSTR file_path = 0;

	CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, 
            IID_IFileOpenDialog, reinterpret_cast<void**>(&dialogue));
	COMDLG_FILTERSPEC extensions[] = {  { L"Images", L"*.3fr;*.ari;*.arw;*.avci;*.avcs;*.avif;*.avifs;*.bay;*.bmp;*.cap;*.cr2;*.cr3;*.crw;*.cur;*.dcr;*.dcs;*.dds;*.dib;*.dng;*.drf;*.eip;*.erf;*.exif;*.fff;*.gif;*.heic;*.heics;*.heif;*.heifs;*.hif;*.ico;*.icon;*.iiq;*.jfif;*.jpe;*.jpeg;*.jpg;*.jxr;*.k25;*.kdc;*.mef;*.mos;*.mrw;*.nef;*.nrw;*.orf;*.ori;*.pef;*.png;*.ptx;*.pxn;*.raf;*.raw;*.rle;*.rw2;*.rwl;*.sr2;*.srf;*.srw;*.tif;*.tiff;*.wdp;*.webp;*.x3f" },
    };

	if (dialogue == 0) 
		goto cleanup;
	if (SUCCEEDED(hr))
		hr = dialogue->SetFileTypes(1, extensions);
	if (SUCCEEDED(hr))
		hr = dialogue->Show(NULL);
	if (SUCCEEDED(hr))
		hr = dialogue->GetResult(&item);
	if (SUCCEEDED(hr))
		hr = item->GetDisplayName(SIGDN_FILESYSPATH, &file_path);
	if (SUCCEEDED(hr)) {
		if (!G->loading_dropped_file) {
			size_t len = wcslen(file_path);
			global_temp_path = (wchar_t *)malloc((len + 1) * sizeof(wchar_t));
			memcpy(global_temp_path, file_path, (len + 1) * sizeof(wchar_t));
			global_temp_path[len] = 0;
		}
		G->dropped_file = true;
	}

	cleanup:
		
	if (item) 		item->Release();
	if (dialogue) 	dialogue->Release();

    CoUninitialize();
}
bool should_show_gui() {
    POINT cursorPos;
    GetCursorPos(&cursorPos);
    HWND hWnd = WindowFromPoint(cursorPos);
    bool inside_window = !(hWnd == NULL || hWnd != hwnd);
    return (G->keep_menu || (G->keys.Mouse.y > mouse_detection && inside_window && !G->settings_visible && !G->mouse_dragging));
}

static v4 read_texture_pixel(ID3D11Texture2D* renderTargetTexture, iv2 pixel) {
	Graphics* ctx = &G->graphics;
	// Create a staging texture with a single pixel size and the same format as the render target
	D3D11_TEXTURE2D_DESC staging_desc;
	renderTargetTexture->GetDesc(&staging_desc);
	staging_desc.Width = 1;
	staging_desc.Height = 1;
	staging_desc.MipLevels = 1;
	staging_desc.ArraySize = 1;
	staging_desc.SampleDesc.Count = 1;
	staging_desc.Usage = D3D11_USAGE_STAGING;
	staging_desc.BindFlags = 0;
	staging_desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;

	ID3D11Texture2D* staging_texture = nullptr;
	HRESULT hr = ctx->device->CreateTexture2D(&staging_desc, nullptr, &staging_texture);
	if (FAILED(hr))
		return v4(0, 0, 0, 0);

	// Copy the specific pixel from the render target to the staging texture
	pixel.x = clamp(pixel.x, 0, WW);
	pixel.y = clamp(pixel.y, 0, WH);
	D3D11_BOX source_region;
	source_region.left = pixel.x;
	source_region.top = pixel.y;
	source_region.right = pixel.x + 1;
	source_region.bottom = pixel.y + 1;
	source_region.front = 0;
	source_region.back = 1;
	ctx->device_ctx->CopySubresourceRegion(staging_texture, 0, 0, 0, 0, renderTargetTexture, 0, &source_region);

	// Map the staging texture
	D3D11_MAPPED_SUBRESOURCE mapped_resource;
	hr = ctx->device_ctx->Map(staging_texture, 0, D3D11_MAP_READ, 0, &mapped_resource);
	if (FAILED(hr)) {
		staging_texture->Release();
		return v4(0, 0, 0, 0);
	}

	u8* data = reinterpret_cast<u8*>(mapped_resource.pData);
	if (data == nullptr) {
		ctx->device_ctx->Unmap(staging_texture, 0);
		staging_texture->Release();
		printf("invalid data ptr\n");
		return v4(0, 0, 0, 0);
	}

	// Read the pixel value (BGRA)!
	u8 b = data[0];
	u8 g = data[1];
	u8 r = data[2];
	u8 a = data[3];
	v4 result = v4(r, g, b, a) / 255.f;

	// Unmap and release the staging texture
	ctx->device_ctx->Unmap(staging_texture, 0);
	staging_texture->Release();
	return result;
}

void set_clipboard_text(const char* text, int len_raw = -1) {
	OpenClipboard(NULL);
	EmptyClipboard();
	if (len_raw < 0)
		len_raw = strlen(text);
	const size_t len = (len_raw + 1) * sizeof(char);
	HGLOBAL h_memory = GlobalAlloc(GMEM_MOVEABLE, len);
	if (h_memory == NULL) {
		CloseClipboard();
		return;
	}
	char* clipboard_text = static_cast<char*>(GlobalLock(h_memory));
	clipboard_text[len_raw] = 0;
	memcpy(clipboard_text, text, len);
	GlobalUnlock(h_memory);
	SetClipboardData(CF_TEXT, h_memory);
	CloseClipboard();
	GlobalFree(h_memory);
}
char* get_clipboard_text() {
	OpenClipboard(NULL);
	HANDLE h_data = GetClipboardData(CF_TEXT);
	if (h_data == NULL) {
		CloseClipboard();
		return NULL;
	}
	char* data = static_cast<char*>(GlobalLock(h_data));
	const size_t len = strlen(data) + 1;
	char* pText = static_cast<char*>(malloc(len));
	if (pText != NULL) {
		memcpy(pText, data, len);
	}
	GlobalUnlock(h_data);
	CloseClipboard();
	return pText;
}

static void reset_image_edit() {
	G->crop_a = iv2(0);
	G->crop_b = iv2(G->graphics.main_image.w, G->graphics.main_image.h);
	G->hue = 0;
	G->saturation = 1;
	G->contrast = 1;
	G->brightness = 0;
	G->gamma = 1;
	G->crop_mode = false;
	G->do_blur = false;
	G->blur_samples = 32;
	G->blur_scale = 0.001;
}

static void update_gui() {
	UI_Context* ctx = G->ui;

	bool disabled_prv = false;

	UI_Theme* theme = UI_get_theme();

	u32 font_size_status = 13;
	u32 font_size_btn = 12;

	u32 status_bar_height = 0;
	bool fullscreen = is_fullscreen(hwnd);
	bool draw_status = !fullscreen || (fullscreen && !G->settings_hide_status_fullscreen);
	if (!G->show_gui && G->settings_hide_status_with_gui)
		draw_status = false;
	if (G->settings_always_show_gui)
		draw_status = true;

	if (draw_status) {
		UI_Block *status_bar = UI_push_block(ctx, 0);
		status_bar->flags |= UI_Block_Flags_draw_background;
		status_bar->style.size[axis_x] = { UI_Size_t::pixels, f32(WW) + 2, 1 };
		status_bar->style.size[axis_y] = { UI_Size_t::pixels, f32(32), 1 };
		status_bar->style.color[c_background] = theme->bg_main_0;
		status_bar->style.position[axis_x] = { UI_Position_t::absolute, -1.f };
		status_bar->style.position[axis_y] = { UI_Position_t::absolute, WH - f32(30) };
		status_bar->style.layout.padding = v2(3);
		status_bar->style.layout.align[axis_y] = align_center;
		status_bar->style.layout.axis = axis_x;
		status_bar_height = 30;

		UI_push_parent(ctx, status_bar);
		if (G->alert.timer > 0) {
			if (G->alert.type == Alert_Error)
				UI_text(theme->text_error, G->ui_font, font_size_status, "Error: %s", G->alert.string);
			else if (G->alert.type == Alert_Info)
				UI_text(theme->text_info, G->ui_font, font_size_status, "Info: %s", G->alert.string);
		} else {
			if (G->files.Count > 0) {
				UI_text(theme->text_reg_light, G->ui_font, font_size_status, "%i / %i | ", G->current_file_index + 1, G->files.Count);

				if (G->files[G->current_file_index].type == TYPE_GIF || G->files[G->current_file_index].type == TYPE_WEBP_ANIM)
					UI_text(theme->text_reg_light, G->ui_font, font_size_status, "%d x %d - frames: %i - ", G->graphics.main_image.w, G->graphics.main_image.h, G->anim_frames);
				else
					UI_text(theme->text_reg_light, G->ui_font, font_size_status, "%d x %d - ", G->graphics.main_image.w, G->graphics.main_image.h);
				UI_text(theme->text_reg_light, G->ui_font, font_size_status, "%i:%i = %.3f - zoom: %.0f%% - Mouse: %i , %i",
				        G->graphics.main_image.frac1, G->graphics.main_image.frac2, G->graphics.main_image.aspect_ratio, G->truescale * 100,
				        (int)G->pixel_mouse.x, (int)G->pixel_mouse.y);
			} else {
				UI_text(theme->text_reg_light, G->ui_font, font_size_status, "No file open. Click \"Open\" or drag and drop an image file to view it.");
			}
		}
		UI_pop_parent(ctx);
	}

	if (!G->loaded && G->files.Count > 0) {
		UI_Block *frame = UI_push_block(ctx, 0);
		frame->style.size[axis_x] = { UI_Size_t::pixels, f32(WW), 1 };
		frame->style.size[axis_y] = { UI_Size_t::pixels, f32(WH), 1 };
		frame->style.layout.align[axis_y] = align_start;
		frame->style.layout.align[axis_x] = align_start;

		UI_push_parent(ctx, frame);

		UI_Block *loading = UI_push_block(ctx);
		loading->style.size[axis_x] = { UI_Size_t::sum_of_children, 0, 1 };
		loading->style.size[axis_y] = { UI_Size_t::sum_of_children, 0, 1 };
		loading->style.layout.padding = v2(5);
		loading->style.roundness = v4(6.f);
		loading->style.color[c_background] = theme->bg_main_0;
		loading->flags |= UI_Block_Flags_draw_background;

		UI_push_parent(ctx, loading);
		UI_text(theme->text_info, G->ui_font, font_size_status, "Loading image...");
		UI_pop_parent(ctx);

		UI_pop_parent(ctx);
	}
	if (G->crop_mode) {
		UI_Block *poup = UI_push_block(ctx, 0);
		poup->style.size[axis_x] = { UI_Size_t::pixels, 200, 1 };
		poup->style.size[axis_y] = { UI_Size_t::sum_of_children, 0, 1 };
		poup->style.position[axis_x] = { UI_Position_t::absolute, WW - 200 - 5.f };
		poup->style.position[axis_y] = { UI_Position_t::absolute, 5};
		poup->style.layout.padding = v2(5);
		poup->style.roundness = v4(6.f);
		poup->style.color[c_background] = theme->bg_main_0;
		poup->flags |= UI_Block_Flags_draw_background;
		poup->style.layout.spacing = v2(4);

		UI_push_parent(ctx, poup);
		iv2 dim = G->crop_b - G->crop_a;
		UI_Color4 col_0 = theme->text_header_2;
		UI_Color4 col_1 = theme->text_reg_main;
		UI_text(theme->text_header_1, G->ui_font, 14, "Crop:");
		UI_push_parent_defer(ctx, UI_bar(axis_x)) {
			UI_text(col_0, G->ui_font, 12, "Dimenstions: ");
			UI_text(col_1, G->ui_font, 12, "%i x %i", dim.x, dim.y);
		}
		UI_push_parent_defer(ctx, UI_bar(axis_x)) {
			UI_text(col_0, G->ui_font, 12, "Offset top-left: ");
			UI_text(col_1, G->ui_font, 12, "%i, %i", G->crop_a.x, G->crop_a.y);
		}
		UI_push_parent_defer(ctx, UI_bar(axis_x)) {
			UI_text(col_0, G->ui_font, 12, "Offset bottom-right: ");
			UI_text(col_1, G->ui_font, 12, "%i, %i", G->crop_b.x, G->crop_b.y);
		}
		UI_pop_parent(ctx);

	}
	
	if (keypress(Key_Ctrl) || keypress(MouseM)) {
		v4 px = G->read_px;
		v2 mouse = UI_get_mouse();
		UI_Block *inspector_menu = UI_push_block(ctx, 0);
		inspector_menu->depth_level += 100;
		inspector_menu->hash = UI_hash_djb2(ctx, "inspector menu");
		inspector_menu->style.position[axis_x] = { UI_Position_t::absolute, mouse.x };
		inspector_menu->style.position[axis_y] = { UI_Position_t::absolute, mouse.y };
		inspector_menu->style.size[axis_x] = { UI_Size_t::pixels, 220, 1 };
		inspector_menu->style.size[axis_y] = { UI_Size_t::pixels, 155, 1 };
		inspector_menu->style.color[c_background] = theme->bg_main_0;
		inspector_menu->style.layout.spacing = v2(5, 3);
		inspector_menu->style.layout.axis = axis_y;
		inspector_menu->style.roundness = v4(8);
		inspector_menu->flags |= UI_Block_Flags_draw_background;
		UI_push_parent_defer(ctx, inspector_menu) {
			UI_Block* color_box = UI_push_block(ctx);
			color_box->style.color[c_background] = UI_color4_sld_v4(px);
			color_box->flags |= UI_Block_Flags_draw_background;
			color_box->style.size[axis_x] = { UI_Size_t::pixels, 220, 1 };
			color_box->style.size[axis_y] = { UI_Size_t::pixels, 50, 1 };
			color_box->style.roundness[UI_Corner_tl] = 8;
			color_box->style.roundness[UI_Corner_tr] = 8;
			color_box->style.softness = 0;
			UI_push_parent_defer(ctx, UI_bar(axis_y)) {
				UI_Block* bar = UI_get_current_parent(ctx);
				bar->style.layout.spacing = v2(3);
				bar->style.layout.padding = v2(10, 5);
				unsigned char r = px[0] * 255;
				unsigned char g = px[1] * 255;
				unsigned char b = px[2] * 255;
				unsigned char a = px[3] * 255;
				UI_text(theme->text_reg_light, G->ui_font, 13, "R : %i", r);
				UI_text(theme->text_reg_light, G->ui_font, 13, "G : %i", g);
				UI_text(theme->text_reg_light, G->ui_font, 13, "B : %i", b);
				UI_text(theme->text_reg_light, G->ui_font, 13, "A : %i", a);
                char color_str[64];
                bool copy_alpha = G->settings_copy_color_include_alpha;
                if (G->settings_copy_color_format == 0) {
                    // Hex
                    if (copy_alpha) sprintf(color_str, "#%02X%02X%02X%02X", r, g, b, a);
                    else            sprintf(color_str, "#%02X%02X%02X", r, g, b);
                } else {
                    int32_t t = G->settings_copy_color_enclose_type;
                    char e0 = t == 0 ? '(' : t == 1 ? '[' : t == 2 ? '{' : ' ';
                    char e1 = t == 0 ? ')' : t == 1 ? ']' : t == 2 ? '}' : '\0';
                    char alpha_str[16] = "";
                    if (G->settings_copy_color_format == 1) {
                        // RGB
                        if (copy_alpha) {
                            if (G->settings_copy_color_normalize_rgb) sprintf(alpha_str, ", %.4f", px.a);
                            else                                      sprintf(alpha_str, ", %d", a);
                        }
                        if (G->settings_copy_color_normalize_rgb)
                            sprintf(color_str, "%c%.4f, %.4f, %.4f%s%c", e0, px.r, px.g, px.b, alpha_str, e1);
                        else
                            sprintf(color_str, "%c%d, %d, %d%s%c", e0, r, g, b, alpha_str, e1);
                        
                    } else if (G->settings_copy_color_format == 2) {
                        // HSV
                        if (copy_alpha) sprintf(alpha_str, ", %.4f", px.a);
                        u32 rgb = UI_v4_to_u32(px);
                        v3 hsv  = UI_rgb_to_hsv(rgb);
                        sprintf(color_str, "%c%.4f, %.4f, %.4f%s%c", e0, hsv[0], hsv[1], hsv[2], alpha_str, e1);
                    }
                }
		if (keyup(MouseR)) set_clipboard_text(color_str);
                UI_text(theme->text_info, G->ui_font, 13, color_str);
		UI_text(theme->text_reg_mid, G->ui_font, 9, "Right click to copy");
			}
		}

	}

	UI_Button_Style btn_default;
	btn_default.dots = false;
	btn_default.color_bg = {
		theme->bg_main_2,
		theme->bg_main_3,
		theme->bg_main_4,
		theme->bg_main_2_d
	};
	btn_default.color_inner = {
		theme->text_reg_main,
		theme->text_reg_main,
		theme->text_slider_2,
		theme->text_reg_main_d
	};
	btn_default.font = G->ui_font;
	btn_default.font_size = font_size_btn;
	btn_default.roundness = v4(4);

	UI_Checkbox_Style checkbox_default;
	checkbox_default.color = {
		theme->bg_main_2,
		theme->bg_main_3,
		theme->bg_main_4,
		theme->bg_main_2_d
	};
	checkbox_default.color_text = {
		theme->text_reg_main,
		theme->text_reg_main,
		theme->text_reg_main,
		theme->bg_main_2_d
	};
	checkbox_default.border = G->settings_selected_theme == UI_Theme_Polar_White ? 0.5 : 2;
	checkbox_default.font = G->ui_font;
	checkbox_default.font_size = font_size_btn;
	checkbox_default.roundness = 4;
	checkbox_default.line_height = 20;
	checkbox_default.box_dim = 18;

	UI_Color_Picker_Style picker_style = { 0 };
	picker_style.roundness = 4;
	picker_style.font_size = btn_default.font_size;
	sprintf(picker_style.label, "BG");
	picker_style.col_border = {
		theme->border_main,
		theme->text_reg_main,
		theme->bg_main_0,
		theme->bg_main_2_d
	};
	picker_style.col_bg = theme->bg_sub;
	UI_Slider_Style slider_style;
	slider_style.bar_long_axis = 155;
	slider_style.bar_short_axis = slider_style.pad_min_size = 20;
	slider_style.roundness = 4;
	slider_style.font = G->ui_font;
	slider_style.font_size = font_size_btn - 1;
	slider_style.pad_style = true;
	slider_style.logarithmic = false;
	slider_style.snap = false;
	slider_style.col_button = {
		theme->bg_main_2,
		theme->bg_main_3,
		theme->bg_main_4,
		theme->bg_main_2_d
	};
	slider_style.col_background = {
		theme->bg_main_1,
		theme->bg_main_2,
		theme->bg_main_2,
		theme->bg_main_1_d
	};
	slider_style.col_text = {
		theme->text_slider_0,
		theme->text_slider_1,
		theme->text_slider_2,
		theme->text_reg_main_d
	};
	UI_Combo_Style default_combo_style;
	default_combo_style.btn_size = default_combo_style.item_size = v2(237.5, 25);
	default_combo_style.show_selected_item = true;
	default_combo_style.font_size = font_size_btn;
	default_combo_style.col_box = {
		theme->bg_main_2,
		theme->bg_main_3,
		theme->bg_main_4,
		theme->bg_main_2_d
	};
	default_combo_style.col_item = {
		theme->bg_main_1,
		theme->bg_main_3,
		theme->bg_main_4,
		theme->bg_main_2_d
	};
	default_combo_style.col_text = {
		theme->text_reg_main,
		theme->text_reg_main,
		theme->text_reg_main,
		theme->text_reg_main_d
	};
	default_combo_style.roundness = 4;

	if (G->files.Count == 0)
		G->gui_disabled = true;
	static bool popup_open = false;

	if ((G->show_gui || popup_open) || G->settings_always_show_gui) {
		popup_open = false;
		//if (G->files.Count && G->files[G->current_file_index].failed)
		//G->gui_disabled = true;

		UI_Block *main_bar = UI_push_block(ctx, 0);
		main_bar->style.position[axis_x] = { UI_Position_t::absolute, 0 };
		main_bar->style.position[axis_y] = { UI_Position_t::absolute, 0 };
		main_bar->style.size[axis_x] = { UI_Size_t::pixels, f32(WW), 1 };
		main_bar->style.size[axis_y] = { UI_Size_t::pixels, f32(WH - status_bar_height - 2), 1 };
		main_bar->style.layout.spacing = v2(2);
		main_bar->style.layout.align[axis_y] = align_end;
		main_bar->style.layout.axis = axis_y;
		UI_push_parent_defer(ctx, main_bar)
		{
			UI_Block *main_h_bar = UI_push_block(ctx);
			main_h_bar->style.size[axis_x] = { UI_Size_t::pixels, f32(WW), 1 };
			main_h_bar->style.size[axis_y] = { UI_Size_t::pixels, 100, 1 };
			main_h_bar->style.layout.axis = axis_x;
			main_h_bar->style.layout.align[axis_x] = align_center;
			main_h_bar->style.layout.align[axis_y] = align_end;
			main_h_bar->style.layout.spacing = v2(5);
			UI_push_parent_defer(ctx, main_h_bar)
			{
				UI_Block *left_menu = UI_push_block(ctx);
				left_menu->style.size[axis_x] = { UI_Size_t::sum_of_children, 0, 1 };
				left_menu->style.size[axis_y] = { UI_Size_t::sum_of_children, 0, 1 };
				left_menu->style.color[c_background] = theme->bg_main_0;
				left_menu->style.layout.padding = v2(8);
				left_menu->style.layout.spacing = v2(5, 3);
				left_menu->style.layout.axis = axis_x;
				left_menu->style.roundness = v4(8);
				left_menu->flags |= UI_Block_Flags_draw_background;
				left_menu->hash = UI_hash_djb2(ctx, "left_menu");
				G->check_mouse_hashes.push_back(left_menu->hash);
				UI_push_parent_defer(ctx, left_menu)
				{
					UI_push_parent_defer(ctx, UI_bar(axis_y))
					{
						v2 btn_size = v2(75, 27);
						UI_get_current_parent(ctx)->style.layout.spacing = v2(5);
						UI_Image_Edit_Style image_value_style;
						image_value_style.checkbox_style = checkbox_default;
						image_value_style.button_style = btn_default;
						image_value_style.button_style.size = btn_size;
						image_value_style.slider_style = slider_style;
						image_value_style.slider_style.pad_style = false;
						image_value_style.slider_style.logarithmic = false;
						image_value_style.color_text = theme->text_reg_main;
						image_value_style.color_frame_bg = theme->bg_sub;
						popup_open |= UI_image_edit(&image_value_style, "edit");

						UI_Histogram_Style histogram_style;
						histogram_style.button_style = btn_default;
						histogram_style.button_style.size = btn_size;
						histogram_style.color_text = theme->text_reg_main;
						histogram_style.checkbox_style = checkbox_default;
						histogram_style.color_frame_bg = theme->bg_sub;

						UI_set_disabled_defer((!G->settings_calculate_histograms || !G->graphics.main_image.has_histo))
						{
							popup_open |= UI_histogram(&histogram_style, "histogram");
						}

						UI_Button_Style style = btn_default;
						style.color_bg.base = theme->pos_btn_0;
						style.color_bg.hot = theme->pos_btn_1;
						style.color_bg.active = theme->pos_btn_2;
						style.size = btn_size;
						if (UI_button(&style, "save as")) {
							if (SUCCEEDED(save_as_dialogue())) {
								push_alert("Image saved successfully!", Alert_Info);
							}
						}
					}
					UI_push_parent_defer(ctx, UI_bar(axis_y))
					{
						UI_get_current_parent(ctx)->style.layout.spacing = v2(3);
						UI_checkbox(&checkbox_default, (bool *) & RGBAflags[0], "R"); UI_tooltip("Toggle red channel");
						UI_checkbox(&checkbox_default, (bool *) & RGBAflags[1], "G"); UI_tooltip("Toggle green channel");
						UI_checkbox(&checkbox_default, (bool *) & RGBAflags[2], "B"); UI_tooltip("Toggle blue channel");
						UI_checkbox(&checkbox_default, (bool *) & RGBAflags[3], "A"); UI_tooltip("Toggle alpha channel");
					}
					UI_push_parent_defer(ctx, UI_bar(axis_y))
					{
						UI_get_current_parent(ctx)->style.layout.spacing = v2(5);
						btn_default.size = v2(50, 19);
						if (UI_button(&btn_default, "fit in")) {
							G->position = v2(0, 0);
							fit_image_in();
							send_signal(G->signals.update_truescale);
						}
						UI_tooltip("Fit image within window size");
						if (UI_button(&btn_default, "fit W")) {
							fit_image_w();
							send_signal(G->signals.update_truescale);
						}
						UI_tooltip("Zoom image to fit its width to the window width");
						if (UI_button(&btn_default, "fit H")) {
							fit_image_h();
							send_signal(G->signals.update_truescale);
						}
						UI_tooltip("Zoom image to fit its height to the window height");
						if (UI_button(&btn_default, "1:1")) {
							fit_image_1();
							send_signal(G->signals.update_truescale);
						}
					}
				}

				UI_Block *control_menu = UI_push_block(ctx);
				control_menu->style.size[axis_x] = { UI_Size_t::sum_of_children, 0, 1 };
				control_menu->style.size[axis_y] = { UI_Size_t::sum_of_children, 0, 1 };
				control_menu->style.color[c_background] = theme->bg_main_0;
				control_menu->style.layout.padding = v2(8);
				control_menu->style.layout.spacing = v2(5);
				control_menu->style.layout.axis = axis_y;
				control_menu->style.roundness = v4(8);
				control_menu->flags |= UI_Block_Flags_draw_background;
				control_menu->hash = UI_hash_djb2(ctx, "control_menu");
				G->check_mouse_hashes.push_back(control_menu->hash);
				UI_push_parent_defer(ctx, control_menu)
				{
					if (G->files.Count && (G->files[G->current_file_index].type == TYPE_GIF || G->files[G->current_file_index].type == TYPE_WEBP_ANIM)) {
						UI_Button_Style style_main = btn_default;
						UI_Button_Style style_side = btn_default;
						style_side.size = v2(35, 30);
						style_main.size = v2(75, 30);
						char *str_play = "Play";
						char *str_pause = "Pause";
						char *str_button = G->anim_play ? str_pause : str_play;
						f32 req_index_f = G->anim_index;
						sprintf(slider_style.string, "frame: %i \\ %i", G->anim_index, G->anim_frames - 1);
						UI_slider(&slider_style, axis_x,  &req_index_f, 0, G->anim_frames - 1, UI_hash_djb2(ctx, "gif_slider"));
						G->anim_index = req_index_f;
						int req_index_i = G->anim_index;
						UI_push_parent_defer(ctx, UI_bar(axis_x))
						{
							UI_get_current_parent(ctx)->style.layout.spacing = v2(5);
							UI_set_disabled_defer((G->anim_play || G->anim_index == 0)) {
								if (UI_button(&style_side, "<<")) req_index_i--;
							}
							if (UI_button(&style_main, str_button)) G->anim_play = !G->anim_play;
							UI_set_disabled_defer((G->anim_play || G->anim_index == 0)) {
								if (UI_button(&style_side, ">>")) req_index_i++;
							}
							G->anim_index = clamp(req_index_i, 0, G->anim_frames - 1);
						}
					}
					static float testnr = 0.1;
					slider_style.bar_short_axis = slider_style.pad_min_size = 25;
					slider_style.logarithmic = true;
					sprintf(slider_style.string, "zoom: %.0f%%%%", G->truescale * 100);

					if (UI_slider(&slider_style, axis_x, &G->truescale_edit, 0.1, 500, UI_hash_djb2(ctx, "zoom slider"))) {
						send_signal(G->signals.update_scale_ui);
					}
	
					slider_style.logarithmic = false;
					UI_push_parent_defer(ctx, UI_bar(axis_x))
					{
						UI_get_current_parent(ctx)->style.layout.spacing = v2(5);
						UI_Button_Style style = btn_default;
						style.size = v2(75, 42);
						UI_set_disabled_defer((G->files.Count == 0) || G->sorting || !(G->current_file_index > 0)) {
							if (UI_button(&style, "<< Prev")) {
								send_signal(G->signals.prev_image);
							}
						}
						UI_set_disabled_defer((G->files.Count == 0) || G->sorting || !(G->current_file_index < G->files.Count - 1)) {
							if (UI_button(&style, "Next >>")) {
								send_signal(G->signals.next_image);
							}
						}

					}
					float file = G->current_file_index;
					slider_style.string[0] = 0;
					slider_style.col_text = {
						theme->text_reg_main_d,
						theme->text_reg_main_d,
						theme->text_reg_main_d,
						theme->text_reg_main_d
					};
					slider_style.bar_long_axis -= 30;
					sprintf(slider_style.string, "file: %i \\ %i", G->current_file_index + 1, G->files.Count);
					slider_style.bar_short_axis = slider_style.pad_min_size = 15;
					UI_push_parent_defer(ctx, UI_bar(axis_x))
					{
						UI_set_disabled_defer(true)
						{
							UI_slider(&slider_style, axis_x, &file, 0, G->files.Count - 1, UI_hash_djb2(ctx, "file slider"));
						}
						UI_Button_Style bs = btn_default;
						bs.size = v2(30, 15);
						bs.dots = true;
						UI_Button_Style bsi = btn_default;
						bsi.size = v2(125, 25);
						UI_set_disabled_defer(G->files.Count == 0)
						{
							popup_open |= UI_files_reload_menu(&bs, &bsi, "files order");
						}
					}
				}

				UI_Block *right_menu = UI_push_block(ctx);
				right_menu->style.size[axis_x] = { UI_Size_t::sum_of_children, 0, 1 };
				right_menu->style.size[axis_y] = { UI_Size_t::sum_of_children, 0, 1 };
				right_menu->style.color[c_background] = theme->bg_main_0;
				right_menu->style.layout.padding = v2(8);
				right_menu->style.layout.spacing = v2(5, 3);
				right_menu->style.layout.axis = axis_y;
				right_menu->style.roundness = v4(8);
				right_menu->flags |= UI_Block_Flags_draw_background;
				right_menu->hash = UI_hash_djb2(ctx, "right_menu");
				G->check_mouse_hashes.push_back(right_menu->hash);
				UI_push_parent_defer(ctx, right_menu)
				{
					UI_push_parent_defer(ctx, UI_bar(axis_x))
					{
						UI_get_current_parent(ctx)->style.layout.spacing = v2(5);
						UI_Button_Style style = btn_default;
						style.size = v2(62.5, 20);
						style.font_size -= 2;
						if (UI_button(&style, "Rotate L")) {
							G->graphics.main_image.orientation++;
							iv2 prev_a = G->crop_a;
							iv2 prev_b = G->crop_b;
							G->crop_a = iv2(prev_a.y, G->graphics.main_image.w - prev_b.x);
							G->crop_b = iv2(prev_b.y, G->graphics.main_image.w - prev_a.x);
							swap(int, G->graphics.main_image.w, G->graphics.main_image.h);
						}
						UI_tooltip("rotate image 90 degrees anticlockwise (doesn't change original file)");
						if (UI_button(&style, "Rotate R")) {
							G->graphics.main_image.orientation--;
							iv2 prev_a = G->crop_a;
							iv2 prev_b = G->crop_b;
							G->crop_a = iv2(G->graphics.main_image.h - prev_b.y, prev_a.x);
							G->crop_b = iv2(G->graphics.main_image.h - prev_a.y, prev_b.x);
							swap(int, G->graphics.main_image.w, G->graphics.main_image.h);
						}
						UI_tooltip("rotate image 90 degrees clockwise (doesn't change original file)");
						G->graphics.main_image.orientation = clamp_circular(G->graphics.main_image.orientation, 0, 3);
					}
					UI_push_parent_defer(ctx, UI_bar(axis_y))
					{
						UI_get_current_parent(ctx)->style.layout.spacing = v2(5);
						UI_push_parent_defer(ctx, UI_bar(axis_x))
						{
							UI_get_current_parent(ctx)->style.layout.spacing = v2(19);
							UI_push_parent_defer(ctx, UI_bar(axis_y))
							{
								UI_checkbox(&checkbox_default, &G->nearest_filtering, "nrst");
								UI_tooltip("Toggle between nearest-neighbor and linear filtering");
								if (!G->nearest_filtering) UI_set_disabled(true);
								UI_checkbox(&checkbox_default, &G->pixel_grid, "grid");
								UI_tooltip("Show pixel grid");
								UI_reset_disabled();
							}

							UI_set_disabled_defer(false) {
								static u32 tmp_bg_color = UI_v4_to_u32(v4(bg_color));
								UI_push_parent_defer(ctx, UI_bar(axis_y))
								{
									UI_Block* bar = UI_get_current_parent(ctx);
									bar->style.layout.spacing = v2(3);
									picker_style.button_size = v2(65, 40);
									popup_open |= UI_color_picker(&picker_style, &tmp_bg_color, true, 15);
									UI_tooltip("Set background color");
									v4 new_bg_color = UI_u32_to_v4(tmp_bg_color);
									bg_color[0] = new_bg_color[0];
									bg_color[1] = new_bg_color[1];
									bg_color[2] = new_bg_color[2];
									bg_color[3] = new_bg_color[3];
								}
							}
						}
						UI_push_parent_defer(ctx, UI_bar(axis_x))
						{
							UI_get_current_parent(ctx)->style.layout.spacing = v2(5);
							UI_Button_Style style = btn_default;
							style.color_bg.base = theme->pos_btn_0;
							style.color_bg.hot = theme->pos_btn_1;
							style.color_bg.active = theme->pos_btn_2;
							v2 btn_size = v2(50, 25);
							UI_set_disabled_defer(false) {
								style.size = btn_size;
								if (UI_button(&style, "open")) {
									file_open_dialogue();
								}
								UI_tooltip("Open image file");
								style = btn_default;
								style.size = btn_size;
								if (UI_button(&style, "config")) {
									G->settings_visible = true;
									G->exif_data_visible = false;
								}
								UI_tooltip("Open settings menu");
							}
							style.size = v2(20, 25);
							UI_set_disabled_defer(!G->graphics.main_image.has_exif) {
								if (UI_button(&style, "i")) {
									G->exif_data_visible = true;
									G->settings_visible = false;
								}
								if (!G->graphics.main_image.has_exif)
									UI_tooltip("No EXIF metadata found on this image!");
								else
									UI_tooltip("Open image EXIF metadata");
							}

						}
					}
				}
			}
			if (G->files.Count && G->settings_preview_thumbs) {
				i32 thumb_dim = THUMBS_DIM;
				UI_Block *thumbs_bar = UI_push_block(ctx);
				thumbs_bar->style.size[axis_x] = { UI_Size_t::pixels, f32(WW), 1 };
				thumbs_bar->style.size[axis_y] = { UI_Size_t::pixels, 50, 1 };
				thumbs_bar->style.layout.axis = axis_x;
				thumbs_bar->style.layout.align[axis_x] = align_center;
				thumbs_bar->style.softness = 0;
				thumbs_bar->hash = UI_hash_djb2(ctx, "thumbs_bar");
				G->check_mouse_hashes.push_back(thumbs_bar->hash);
				UI_push_parent_defer(ctx, thumbs_bar)
				{
					static f32 begin = 0;
					f32 begin_should = WW / 2.f - thumb_dim / 2.f - G->current_file_index * thumb_dim + (thumb_dim / 2.f);
					if (WW)
						begin = UI_lerp_f32(begin, begin_should, 0.2);

					int thumbs_that_fit = (WW / thumb_dim) + 2;
					int start_index = G->current_file_index - (min(G->current_file_index, thumbs_that_fit / 2));
					f32 offset_begin = begin + start_index * thumb_dim;
					int end_index = min(start_index + thumbs_that_fit, G->files.Count);

					UI_Block *thumbs_scroll = UI_push_block(ctx);
					thumbs_scroll->style.position[axis_x] = { UI_Position_t::absolute, f32(offset_begin) };
					thumbs_scroll->style.layout.axis = axis_x;
					thumbs_bar->style.color[c_background] = theme->bg_sub;
					thumbs_bar->flags |= UI_Block_Flags_draw_background;
					UI_push_parent_defer(ctx, thumbs_scroll)
					{
						UI_push_hash(ctx, UI_hash_djb2(ctx, "thumbs"));
						for (int i = start_index; i < end_index; i++) {
							UI_Button_Style thumb_style;
							thumb_style.color_bg.base = theme->bg_main_2;
							thumb_style.color_bg.hot = theme->bg_main_3;
							thumb_style.color_bg.active = theme->bg_main_4;
							thumb_style.color_inner.base = UI_color4_sld_u32(0);
							thumb_style.color_inner.hot = theme->pos_btn_1;
							thumb_style.color_inner.active = theme->pos_btn_2;
							thumb_style.roundness = v4(0.f);
							thumb_style.size = v2(thumb_dim);
							thumb_style.thumb_button = true;
							thumb_style.thumb_index = i;
							if (UI_button(&thumb_style, "%i", i)) {
								send_signal(G->signals.reload_file);
								G->req_file_index = i;
							}
						}
						UI_pop_hash(ctx);
					}
					UI_Block *selector = UI_push_block(ctx);
					selector->flags |= UI_Block_Flags_draw_border;
					selector->style.size[axis_x] = { UI_Size_t::pixels, (f32)thumb_dim, 1 };
					selector->style.size[axis_y] = { UI_Size_t::pixels, (f32)thumb_dim, 1 };
					selector->style.position[axis_x] = { UI_Position_t::absolute, (WW - thumb_dim) * 0.5f + 25.f };
					selector->style.position[axis_y] = { UI_Position_t::percent_of_parent, 0 };
					selector->style.border_size = 2;
					selector->style.softness = 0;
					selector->style.color[c_border] = theme->text_reg_light;
					selector->depth_level += 5;
				}
			}
		}
	}
	static bool exif_popup_open = false;
	if (G->exif_data_visible || exif_popup_open) {
		UI_set_disabled(false);

		UI_Block *frame = UI_push_block(ctx, 0);
		frame->style.size[axis_x] = { UI_Size_t::pixels, f32(WW), 1 };
		frame->style.size[axis_y] = { UI_Size_t::pixels, f32(WH), 1 };
		frame->style.layout.align[axis_y] = align_center;
		frame->style.layout.align[axis_x] = align_center;

		UI_Block *exif_menu = UI_push_block(ctx, frame);
		exif_menu->style.size[axis_x] = { UI_Size_t::pixels, 350, 1 };
		exif_menu->style.size[axis_y] = { UI_Size_t::sum_of_children, 0, 1 };
		exif_menu->style.color[c_background] = theme->bg_main_0;
		exif_menu->style.layout.padding = v2(8);
		exif_menu->style.layout.spacing = v2(5);
		exif_menu->style.layout.axis = axis_y;
		exif_menu->style.roundness = v4(8);
		exif_menu->flags |= UI_Block_Flags_draw_background;
		exif_menu->hash = UI_hash_djb2(ctx, "settings_menu");
		exif_menu->depth_level += 200;
		G->check_mouse_hashes.push_back(exif_menu->hash);
		UI_Block *menu_ref = UI_find_block(ctx, exif_menu->hash, UI_PREVIOUS);
		if (menu_ref) {
			if (!UI_point_in_rect(menu_ref->position, menu_ref->position + menu_ref->size, UI_get_mouse())) {
				if (keydn(MouseL))
					G->exif_data_visible = false;
			}
		}
		UI_Color4 col_0 = theme->text_header_2;
		UI_Color4 col_1 = theme->text_reg_main;
		UI_push_parent_defer(ctx, exif_menu) {
			UI_text(theme->text_header_1, G->ui_font, 14, "Image Metadata");
			UI_separator(2.5, theme->separator);
			UI_push_parent_defer(ctx, UI_bar(axis_x)) {
				UI_text(col_0, G->ui_font, 12, "Camera make: ");
				UI_text(col_1, G->ui_font, 12, "%s", G->graphics.main_image.exif_info.Make.c_str());
			}
			UI_push_parent_defer(ctx, UI_bar(axis_x)) {
				UI_text(col_0, G->ui_font, 12, "Camera model: ");
				UI_text(col_1, G->ui_font, 12, "%s", G->graphics.main_image.exif_info.Model.c_str());
			}
			UI_push_parent_defer(ctx, UI_bar(axis_x)) {
				UI_text(col_0, G->ui_font, 12, "Software: ");
				UI_text(col_1, G->ui_font, 12, "%s", G->graphics.main_image.exif_info.Software.c_str());
			}
			UI_push_parent_defer(ctx, UI_bar(axis_x)) {
				UI_text(col_0, G->ui_font, 12, "Bits per sample: ");
				UI_text(col_1, G->ui_font, 12, "%d", G->graphics.main_image.exif_info.BitsPerSample);
			}
			UI_push_parent_defer(ctx, UI_bar(axis_x)) {
				UI_text(col_0, G->ui_font, 12, "Image width: ");
				UI_text(col_1, G->ui_font, 12, "%d", G->graphics.main_image.exif_info.ImageWidth);
			}
			UI_push_parent_defer(ctx, UI_bar(axis_x)) {
				UI_text(col_0, G->ui_font, 12, "Image height: ");
				UI_text(col_1, G->ui_font, 12, "%d", G->graphics.main_image.exif_info.ImageHeight);
			}
			UI_push_parent_defer(ctx, UI_bar(axis_x)) {
				UI_text(col_0, G->ui_font, 12, "Image description: ");
				UI_text(col_1, G->ui_font, 12, "%s", G->graphics.main_image.exif_info.ImageDescription.c_str());
			}
			UI_push_parent_defer(ctx, UI_bar(axis_x)) {
				UI_text(col_0, G->ui_font, 12, "Image orientation: ");
				UI_text(col_1, G->ui_font, 12, "%d", G->graphics.main_image.exif_info.Orientation);
			}
			UI_push_parent_defer(ctx, UI_bar(axis_x)) {
				UI_text(col_0, G->ui_font, 12, "Image copyright: ");
				UI_text(col_1, G->ui_font, 12, "%s", G->graphics.main_image.exif_info.Copyright.c_str());
			}
			UI_push_parent_defer(ctx, UI_bar(axis_x)) {
				UI_text(col_0, G->ui_font, 12, "Image date/time: ");
				UI_text(col_1, G->ui_font, 12, "%s", G->graphics.main_image.exif_info.DateTime.c_str());
			}
			UI_push_parent_defer(ctx, UI_bar(axis_x)) {
				UI_text(col_0, G->ui_font, 12, "Original date/time: ");
				UI_text(col_1, G->ui_font, 12, "%s", G->graphics.main_image.exif_info.DateTimeOriginal.c_str());
			}
			UI_push_parent_defer(ctx, UI_bar(axis_x)) {
				UI_text(col_0, G->ui_font, 12, "Digitize date/time: ");
				UI_text(col_1, G->ui_font, 12, "%s", G->graphics.main_image.exif_info.DateTimeDigitized.c_str());
			}
			UI_push_parent_defer(ctx, UI_bar(axis_x)) {
				UI_text(col_0, G->ui_font, 12, "Subsecond time: ");
				UI_text(col_1, G->ui_font, 12, "%s", G->graphics.main_image.exif_info.SubSecTimeOriginal.c_str());
			}
			UI_push_parent_defer(ctx, UI_bar(axis_x)) {
				UI_text(col_0, G->ui_font, 12, "Exposure time: ");
				UI_text(col_1, G->ui_font, 12, "1/%d s", (unsigned)(1.0 / G->graphics.main_image.exif_info.ExposureTime));
			}
			UI_push_parent_defer(ctx, UI_bar(axis_x)) {
				UI_text(col_0, G->ui_font, 12, "F-stop: ");
				UI_text(col_1, G->ui_font, 12, "f/%.1f", G->graphics.main_image.exif_info.FNumber);
			}
			UI_push_parent_defer(ctx, UI_bar(axis_x)) {
				UI_text(col_0, G->ui_font, 12, "ISO speed: ");
				UI_text(col_1, G->ui_font, 12, "%d", G->graphics.main_image.exif_info.ISOSpeedRatings);
			}
			UI_push_parent_defer(ctx, UI_bar(axis_x)) {
				UI_text(col_0, G->ui_font, 12, "Subject distance: ");
				UI_text(col_1, G->ui_font, 12, "%f m", G->graphics.main_image.exif_info.SubjectDistance);
			}
			UI_push_parent_defer(ctx, UI_bar(axis_x)) {
				UI_text(col_0, G->ui_font, 12, "Exposure bias: ");
				UI_text(col_1, G->ui_font, 12, "%f EV", G->graphics.main_image.exif_info.ExposureBiasValue);
			}
			UI_push_parent_defer(ctx, UI_bar(axis_x)) {
				UI_text(col_0, G->ui_font, 12, "Flash used?: ");
				UI_text(col_1, G->ui_font, 12, "%d", G->graphics.main_image.exif_info.Flash);
			}
			UI_push_parent_defer(ctx, UI_bar(axis_x)) {
				UI_text(col_0, G->ui_font, 12, "Metering mode: ");
				UI_text(col_1, G->ui_font, 12, "%d", G->graphics.main_image.exif_info.MeteringMode);
			}
			UI_push_parent_defer(ctx, UI_bar(axis_x)) {
				UI_text(col_0, G->ui_font, 12, "Lens focal length: ");
				UI_text(col_1, G->ui_font, 12, "%f mm", G->graphics.main_image.exif_info.FocalLength);
			}
			UI_push_parent_defer(ctx, UI_bar(axis_x)) {
				UI_text(col_0, G->ui_font, 12, "35mm focal length: ");
				UI_text(col_1, G->ui_font, 12, "%u mm", G->graphics.main_image.exif_info.FocalLengthIn35mm);
			}
			UI_push_parent_defer(ctx, UI_bar(axis_x)) {
				UI_text(col_0, G->ui_font, 12, "GPS Latitude: ");
				UI_text(col_1, G->ui_font, 12, "%f deg", G->graphics.main_image.exif_info.GeoLocation.Latitude);
			}
			UI_push_parent_defer(ctx, UI_bar(axis_x)) {
				UI_text(col_0, G->ui_font, 12, "GPS Longitude: ");
				UI_text(col_1, G->ui_font, 12, "%f deg", G->graphics.main_image.exif_info.GeoLocation.Longitude);
			}
			UI_push_parent_defer(ctx, UI_bar(axis_x)) {
				UI_text(col_0, G->ui_font, 12, "GPS Altitude: ");
				UI_text(col_1, G->ui_font, 12, "%f m", G->graphics.main_image.exif_info.GeoLocation.Altitude);
			}
		}
		UI_reset_disabled();
	}
	static bool save_as_popup_open = false;

	static bool settings_popup_open = false;
	if (G->settings_visible || settings_popup_open) {
		UI_set_disabled(false);

		UI_Block *frame = UI_push_block(ctx, 0);
		frame->style.size[axis_x] = {UI_Size_t::pixels, f32(WW), 1};
		frame->style.size[axis_y] = {UI_Size_t::pixels, f32(WH), 1};
		frame->style.layout.align[axis_y] = align_center;
		frame->style.layout.align[axis_x] = align_center;

		UI_Block *settings_menu = UI_push_block(ctx, frame);
		settings_menu->style.size[axis_x] = { UI_Size_t::pixels, 500, 1 };
		settings_menu->style.size[axis_y] = { UI_Size_t::sum_of_children, 0, 1 };
		settings_menu->style.color[c_background] = theme->bg_main_0;
		settings_menu->style.layout.padding = v2(8);
		settings_menu->style.layout.spacing = v2(5);
		settings_menu->style.layout.axis = axis_y;
		settings_menu->style.roundness = v4(8);
		settings_menu->flags |= UI_Block_Flags_draw_background;
		settings_menu->hash = UI_hash_djb2(ctx, "settings_menu");
		settings_menu->depth_level += 200;
		G->check_mouse_hashes.push_back(settings_menu->hash);
		UI_Block *menu_ref = UI_find_block(ctx, settings_menu->hash, UI_PREVIOUS);
		if (menu_ref) {
			if (!UI_point_in_rect(menu_ref->position, menu_ref->position + menu_ref->size, UI_get_mouse())) {
				if (keydn(MouseL))
					G->settings_visible = false;
			}
		}
		UI_push_parent_defer(ctx, settings_menu) {
			UI_push_parent_defer(ctx, UI_bar(axis_x)) {
				UI_push_parent_defer(ctx, UI_bar(axis_x)) {
					UI_get_current_parent(ctx)->style.size[axis_x] = { UI_Size_t::percent_of_parent, 0.5, 0 };
					UI_text(theme->text_header_0, G->ui_font, 20, "Cactus Image Viewer");
				}
				UI_push_parent_defer(ctx, UI_bar(axis_x)) {
					UI_get_current_parent(ctx)->style.size[axis_x] = { UI_Size_t::percent_of_parent, 0.5, 0 };
					UI_get_current_parent(ctx)->style.layout.align[axis_x] = align_end;
					btn_default.size = v2(20);
					if (UI_button(&btn_default, "X")) {
						G->settings_visible = false;
					}
				}
			}
			UI_text(theme->text_header_1, G->ui_font, 14,"version %s", VERSION);
			UI_text(theme->text_header_2, G->ui_font, 13,"by Wassim Alhajomar @wassimulator");
			UI_separator(2.5, theme->separator);
			UI_push_parent_defer(ctx, UI_bar(axis_x)) {
				UI_text(theme->text_header_1, G->ui_font, 12,"Supported codecs: ");
				UI_text(theme->text_reg_main, G->ui_font, 12,"BMP, GIF, ICO, JPEG, JPEG XR, PNG, TIFF, DDS, WEBP");	
			}
			UI_text(theme->text_reg_main, G->ui_font, 10,"(Supports installed codecs for WIC as well, check Microsoft Store to get codecs like HEIF, RAW, or AVIF)");	
			UI_separator(2, theme->separator);
			UI_text(theme->text_header_1, G->ui_font, 13,"Controls:");
			UI_push_parent_defer(ctx, UI_bar(axis_x)) {
				UI_text(theme->text_header_2, G->ui_font, 12,"Left Mouse button / WASD keys: ");
				UI_text(theme->text_reg_main, G->ui_font, 12,"pan image.");	
			}
			UI_push_parent_defer(ctx, UI_bar(axis_x)) {
				UI_text(theme->text_header_2, G->ui_font, 12,"Mouse wheel / Q/E keys: ");
				UI_text(theme->text_reg_main, G->ui_font, 12,"zoom image.");	
			}
			UI_push_parent_defer(ctx, UI_bar(axis_x)) {
				UI_text(theme->text_header_2, G->ui_font, 12,"Shift key: ");
				UI_text(theme->text_reg_main, G->ui_font, 12,"fine pan and zoom mode.");	
			}
			UI_push_parent_defer(ctx, UI_bar(axis_x)) {
				UI_text(theme->text_header_2, G->ui_font, 12,"Middle Mouse button/ Ctrl key (hold): ");
				UI_text(theme->text_reg_main, G->ui_font, 12,"pixel inspector (right click to copy).");	
			}
			UI_push_parent_defer(ctx, UI_bar(axis_x)) {
				UI_text(theme->text_header_2, G->ui_font, 12,"Left/Right Arrow keys: ");
				UI_text(theme->text_reg_main, G->ui_font, 12,"next/ previous image.");	
			}
			UI_push_parent_defer(ctx, UI_bar(axis_x)) {
				UI_text(theme->text_header_2, G->ui_font, 12,"Up/Down Arrow keys: ");
				UI_text(theme->text_reg_main, G->ui_font, 12,"GIF control: next/ previous image.");	
			}
			UI_push_parent_defer(ctx, UI_bar(axis_x)) {
				UI_text(theme->text_header_2, G->ui_font, 12,"Space key: ");
				UI_text(theme->text_reg_main, G->ui_font, 12,"GIF control: play/ pause.");	
			}
			UI_push_parent_defer(ctx, UI_bar(axis_x)) {
				UI_text(theme->text_header_2, G->ui_font, 12,"F key: ");
				UI_text(theme->text_reg_main, G->ui_font, 12,"Toggle between linear and nearest neighbor filtering.");	
			}
			UI_push_parent_defer(ctx, UI_bar(axis_x)) {
				UI_text(theme->text_header_2, G->ui_font, 12,"G key: ");
				UI_text(theme->text_reg_main, G->ui_font, 12,"Toggle pixel grid (if in nearest neighbor filtering is on).");	
			}
			UI_push_parent_defer(ctx, UI_bar(axis_x)) {
				UI_text(theme->text_header_2, G->ui_font, 12,"F11 key: ");
				UI_text(theme->text_reg_main, G->ui_font, 12,"Toggle fullscreen mode.");	
			}
			UI_push_parent_defer(ctx, UI_bar(axis_x)) {
				UI_text(theme->text_header_2, G->ui_font, 12,"R key: ");
				UI_text(theme->text_reg_main, G->ui_font, 12,"Reload current folder, and scan for new files in it.");	
			}
			UI_separator(2, theme->separator);
			UI_text(theme->text_header_1, G->ui_font, 13,"Settings:");

			char *resetzoom_options[]  {"Do not reset zoom", "Save zoom for each file", "Fit Width", "Fit Height", "Zoom to 1:1"};
			char *resetpos_options[] { "Do not reset position", "Save position for each file", "Reset to center" };
			char *new_file_zoom_options[] { "Fill window", "zoom to 1:1" };
			UI_text(theme->text_reg_main, G->ui_font, 12,"Persistent zoom and position settings upon file change: ");
			UI_push_parent_defer(ctx, UI_bar(axis_x)) {
				UI_Block* bar = UI_get_current_parent(ctx);
				bar->style.layout.spacing = v2(5);
				bar->style.layout.align[axis_x] = align_center;
				UI_combo(&default_combo_style, "reset position options",  &G->settings_resetpos, resetpos_options, array_size(resetpos_options));
				UI_combo(&default_combo_style, "reset zoom options", &G->settings_resetzoom, resetzoom_options, array_size(resetzoom_options));
			}
			UI_push_parent_defer(ctx, UI_bar(axis_x)) {
				UI_Block* bar = UI_get_current_parent(ctx);
				bar->style.size[axis_x].type = UI_Size_t::pixels;
				bar->style.size[axis_x].value = 482;
				bar->style.layout.align[axis_y] = align_center;
				UI_text(theme->text_reg_main, G->ui_font, 12,"Upon opening a new file: ");
				UI_push_parent_defer(ctx, UI_bar(axis_x)) {
					bar = UI_get_current_parent(ctx);
					bar->style.size[axis_x].type = UI_Size_t::percent_of_parent;
					bar->style.size[axis_x].value = 1;
					bar->style.size[axis_x].strictness = 0;
					bar->style.layout.align[axis_x] = align_end;
					UI_combo(&default_combo_style, "new file zoom options", &G->settings_newfilezoom, new_file_zoom_options, array_size(new_file_zoom_options));
				}
			}
            char *copy_color_format_options[] {"Hex", "RGB", "HSV"};
            char *copy_color_enclose_options[] {"(Parenthesis)", "[Brackets]", "{Braces}", "Don't enclose"};
			UI_text(theme->text_reg_main, G->ui_font, 12,"Copied color format settings: ");
			UI_push_parent_defer(ctx, UI_bar(axis_x)) {
				UI_Block* bar = UI_get_current_parent(ctx);
				bar->style.layout.spacing = v2(5);
                UI_Combo_Style combo_style = default_combo_style;
                combo_style.btn_size.x *= 0.5;
                combo_style.item_size.x *= 0.5;
				UI_combo(&combo_style, "copy color format options", &G->settings_copy_color_format, copy_color_format_options, array_size(copy_color_format_options));
                UI_checkbox(&checkbox_default, &G->settings_copy_color_include_alpha, "Include alpha");
                if (G->settings_copy_color_format != 0) // Only show if format is NOT hex
                    UI_combo(&combo_style, "copy color enclose options", &G->settings_copy_color_enclose_type, copy_color_enclose_options, array_size(copy_color_enclose_options));
                if (G->settings_copy_color_format == 1) // Only show if format is RGB.
                    UI_checkbox(&checkbox_default, &G->settings_copy_color_normalize_rgb, "Normalize RGB");
			}
			f32 line_h = 20;
			UI_push_parent_defer(ctx, UI_bar(axis_x)) {
				UI_push_parent_defer(ctx, UI_bar(axis_x)) {
					UI_Block* bar = UI_get_current_parent(ctx);
					bar->style.size[axis_x] = { UI_Size_t::percent_of_parent, 0.5, 1 };
					bar->style.size[axis_y] = { UI_Size_t::pixels, line_h, 1 };
					bar->style.layout.align[axis_y] = align_center;
					bar->style.layout.spacing = v2(5);
					UI_text(theme->text_reg_main, G->ui_font, 12, "Checkerboard colors: ");
				}
				UI_push_parent_defer(ctx, UI_bar(axis_x)) {
					UI_Block* bar = UI_get_current_parent(ctx);
					bar->style.size[axis_x] = { UI_Size_t::percent_of_parent, 0.5, 1 };
					bar->style.layout.spacing = v2(5);
					u32 c1 = UI_v4_to_u32(v4(checkerboard_color_1[0], checkerboard_color_1[1], checkerboard_color_1[2], 1));
					u32 c2 = UI_v4_to_u32(v4(checkerboard_color_2[0], checkerboard_color_2[1], checkerboard_color_2[2], 1));
					picker_style.button_size = v2(115, line_h);
					picker_style.label[0] = 0;
					settings_popup_open = false;
					if (UI_color_picker(&picker_style, &c1, false, UI_hash_djb2(ctx, "checkbrd first"))) settings_popup_open = true;
					if (UI_color_picker(&picker_style, &c2, false, UI_hash_djb2(ctx, "checkbrd second"))) settings_popup_open = true;
					v4 vc1 = UI_u32_to_v4(c1);
					v4 vc2 = UI_u32_to_v4(c2);
					for (int i = 0; i < 3; i++) {
						checkerboard_color_1[i] = vc1[i];
						checkerboard_color_2[i] = vc2[i];
					}
				}
			}
			UI_push_parent_defer(ctx, UI_bar(axis_x)) {
				UI_push_parent_defer(ctx, UI_bar(axis_x)) {
					UI_Block* bar = UI_get_current_parent(ctx);
					bar->style.size[axis_x] = { UI_Size_t::percent_of_parent, 0.5, 1 };
					bar->style.size[axis_y] = { UI_Size_t::pixels, line_h, 1 };
					bar->style.layout.align[axis_y] = align_center;
					bar->style.layout.spacing = v2(5);
					UI_text(theme->text_reg_main, G->ui_font, 12, "Theme: ");
				}
				UI_push_parent_defer(ctx, UI_bar(axis_x)) {
					UI_Block* bar = UI_get_current_parent(ctx);
					bar->style.size[axis_x] = { UI_Size_t::percent_of_parent, 0.5, 1 };
					bar->style.layout.spacing = v2(5);
					default_combo_style.btn_size = v2(235, line_h);
					UI_combo(&default_combo_style, "Theme selector", &G->settings_selected_theme, themes_str, array_size(themes_str));
				}
			}
			UI_push_parent_defer(ctx, UI_bar(axis_y)) {
				UI_checkbox(&checkbox_default, &G->settings_autoplayGIFs, "Autoplay GIF files upon loading");
				UI_checkbox(&checkbox_default, &G->settings_sort, "Sort files according to folder's sorting order (otherwise it's alphabetical)");
				UI_checkbox(&checkbox_default, &G->settings_movementinvert, "Inverted pan movement with WASD");
				UI_checkbox(&checkbox_default, &G->settings_exif, "Parse EXIF data from JPEGs");
				UI_tooltip("Parses image orientation, disablable for optional performance improvement");
				UI_checkbox(&checkbox_default, &G->settings_hide_status_fullscreen, "Hide status bar in fullscreen mode");
				UI_checkbox(&checkbox_default, &G->settings_start_in_fullscreen, "Start Cactus Viewer in fullscreen mode");
				UI_checkbox(&checkbox_default, &G->settings_hide_status_with_gui, "Only show status bar on hover");
				UI_checkbox(&checkbox_default, &G->settings_always_show_gui, "Always show GUI");
				UI_checkbox(&checkbox_default, &G->settings_dont_resize, "Don't resize window on image change");
				UI_checkbox(&checkbox_default, &G->settings_calculate_histograms, "Calculate image histograms (relatively performance intensive on load)");
				UI_checkbox(&checkbox_default, &G->settings_preview_thumbs, "Show thumbnail bar of images in folder.");
				UI_tooltip("Generates thumbnails for images in the folder (can be performance intensive with large folders and is limited to 25.600 images.)");

			}
			UI_push_parent_defer(ctx, UI_bar(axis_x)) {
				UI_push_parent_defer(ctx, UI_bar(axis_x)) {
					UI_Block*bar = UI_get_current_parent(ctx);
					bar->style.size[axis_x] = { UI_Size_t::percent_of_parent, 0.5, 1 };
					bar->style.size[axis_y] = { UI_Size_t::pixels, line_h, 1 };
					bar->style.layout.align[axis_y] = align_center;
					UI_text(theme->text_reg_main, G->ui_font, 12, "Keyboard (WASD) pan speed: ");
				}
				UI_push_parent_defer(ctx, UI_bar(axis_x)) {
					UI_get_current_parent(ctx)->style.size[axis_x] = { UI_Size_t::percent_of_parent, 0.5, 1 };
					slider_style.string[0] = 0;
					slider_style.bar_long_axis = 235;
					slider_style.bar_short_axis = line_h;
					UI_slider(&slider_style, axis_x, &G->settings_movementmag, 0.1, 20, UI_hash_djb2(ctx, "pan speed slider"));
				}
			}
			UI_push_parent_defer(ctx, UI_bar(axis_x)) {
				UI_push_parent_defer(ctx, UI_bar(axis_x)) {
					UI_Block*bar = UI_get_current_parent(ctx);
					bar->style.size[axis_x] = { UI_Size_t::percent_of_parent, 0.5, 1 };
					bar->style.size[axis_y] = { UI_Size_t::pixels, 25, 1 };
					bar->style.layout.align[axis_y] = align_center;
					UI_text(theme->text_reg_main, G->ui_font, 12, "Shift slow mode magnitude: ");
				}
				UI_push_parent_defer(ctx, UI_bar(axis_x)) {
					UI_get_current_parent(ctx)->style.size[axis_x] = { UI_Size_t::percent_of_parent, 0.5, 1 };
					slider_style.string[0] = 0;
					slider_style.bar_long_axis = 235;
					slider_style.bar_short_axis = line_h;
					UI_slider(&slider_style, axis_x, &G->settings_shiftslowmag, 0.1, 20, UI_hash_djb2(ctx, " slow mode slider"));
					UI_tooltip("adjusts how much slower the pan and zoom speed is when holding SHIFT");
				}
			}
			UI_reset_disabled();
		}
	}
	
	G->gui_disabled = false;
}

enum Drag_index {
	drag_free,
	drag_crop_a_x,
	drag_crop_a_y,
	drag_crop_b_x,
	drag_crop_b_y,

	drag_crop_a_x_a_y,
	drag_crop_a_x_b_y,
	drag_crop_b_x_a_y,
	drag_crop_b_x_b_y,

	drag_count,
};

static void shuffle_folder() {

	for (int i = G->files.Count - 1; i > 0; i--) {
		int j = rand() % (i + 1);
		swap(File_Data, G->files[i], G->files[j]);
		G->current_file_index = 0;
		G->signals.reload_file = true;
	}
}

static void update_logic() {
	bool WantCaptureMouse = G->ui_want_capture_mouse;

    if (G->alert.timer > 0)
        G->alert.timer++;
    if (G->alert.timer == 300)
        G->alert.timer = 0;

	if (keyup(Key_F11))
		toggle_fullscreen(hwnd);
	if (keyup(Key_Esc) && is_fullscreen(hwnd)) 
		exit_fullscreen(hwnd);

	if (keyup(Key_F)) {
        //send_signal(G->signals.update_filtering);
        G->nearest_filtering = !G->nearest_filtering;
    }
    if (keyup(Key_G)) {
        G->pixel_grid = !G->pixel_grid;
    }
	if (G->files.Count > 0 && keyup(Key_R)) {
		scan_folder(G->files[G->current_file_index].file.path);
	}

	G->mouse_dragging = false;
	static int drag_index = drag_free;
	bool hovers[drag_count] = { 0 };
	if (!WantCaptureMouse) {
		if (G->crop_mode) {
			f32 threshold = 5 / G->truescale;
			SetCursor(G->hcursor[Cursor_Type_arrow]);

			if 		  (abso(G->pixel_mouse.x - (G->crop_a.x)) < threshold) {
				if (keydn(MouseL)) drag_index = drag_crop_a_x;
				hovers[drag_crop_a_x] = true;
				SetCursor(G->hcursor[Cursor_Type_resize_h]);
			} else if (abso(G->pixel_mouse.x - (G->crop_b.x)) < threshold) {
				if (keydn(MouseL)) drag_index = drag_crop_b_x;
				hovers[drag_crop_b_x] = true;
				SetCursor(G->hcursor[Cursor_Type_resize_h]);
			}

			if 		  (abso(G->pixel_mouse.y - (G->crop_a.y)) < threshold) {
				if (keydn(MouseL)) drag_index = drag_crop_a_y;
				hovers[drag_crop_a_y] = true;
				SetCursor(G->hcursor[Cursor_Type_resize_v]);
			} else if (abso(G->pixel_mouse.y - (G->crop_b.y)) < threshold) {
				if (keydn(MouseL)) drag_index = drag_crop_b_y;
				hovers[drag_crop_b_y] = true;
				SetCursor(G->hcursor[Cursor_Type_resize_v]);
			}

			if 		  (hovers[drag_crop_a_x] && hovers[drag_crop_a_y]) {
				SetCursor(G->hcursor[Cursor_Type_resize_dr]);
				if (keydn(MouseL)) drag_index = drag_crop_a_x_a_y;
			} else if (hovers[drag_crop_a_x] && hovers[drag_crop_b_y]) {
				SetCursor(G->hcursor[Cursor_Type_resize_dl]);
				if (keydn(MouseL)) drag_index = drag_crop_a_x_b_y;
			} else if (hovers[drag_crop_b_x] && hovers[drag_crop_a_y]) {
				SetCursor(G->hcursor[Cursor_Type_resize_dl]);
				if (keydn(MouseL)) drag_index = drag_crop_b_x_a_y;
			} else if (hovers[drag_crop_b_x] && hovers[drag_crop_b_y]) {
				SetCursor(G->hcursor[Cursor_Type_resize_dr]);
				if (keydn(MouseL)) drag_index = drag_crop_b_x_b_y;
			}
		}
		if (keypress(MouseL)) {
			if (drag_index == drag_free)
				G->position += G->keys.Mouse_rel;
			G->mouse_dragging = true;
		}
	}
	if (G->crop_mode) {
		v2 new_pixel = G->pixel_mouse + 0.5f;
		switch (drag_index) {
			case drag_crop_a_x: 
				SetCursor(G->hcursor[Cursor_Type_resize_h]);
				G->crop_a.x = new_pixel.x; 
				break;
			case drag_crop_b_x: 
				SetCursor(G->hcursor[Cursor_Type_resize_h]);
				G->crop_b.x = new_pixel.x;
				break;
			case drag_crop_a_y: 
				SetCursor(G->hcursor[Cursor_Type_resize_v]);
				G->crop_a.y = new_pixel.y;
				break;
			case drag_crop_b_y: 
				SetCursor(G->hcursor[Cursor_Type_resize_v]);
				G->crop_b.y = new_pixel.y;
				break;
			case drag_crop_a_x_a_y: 
				SetCursor(G->hcursor[Cursor_Type_resize_dr]);
				G->crop_a.x = new_pixel.x; 
				G->crop_a.y = new_pixel.y;
				break;
			case drag_crop_a_x_b_y: 
				SetCursor(G->hcursor[Cursor_Type_resize_dl]);
				G->crop_a.x = new_pixel.x; 
				G->crop_b.y = new_pixel.y;
				break;
			case drag_crop_b_x_a_y: 
				SetCursor(G->hcursor[Cursor_Type_resize_dl]);
				G->crop_b.x = new_pixel.x;
				G->crop_a.y = new_pixel.y;
				break;
			case drag_crop_b_x_b_y: 
				SetCursor(G->hcursor[Cursor_Type_resize_dr]);
				G->crop_b.x = new_pixel.x;
				G->crop_b.y = new_pixel.y;
				break;
			default:
				if (keypress(MouseR) && !WantCaptureMouse) {
					G->crop_a += _iv2(G->keys.Mouse_rel / G->truescale);
					G->crop_b += _iv2(G->keys.Mouse_rel / G->truescale);
					G->mouse_dragging = true;
				}
				break;
		}
		if (drag_index != drag_free) 
			G->mouse_dragging = true;
	}
	v2 img_dim = v2(G->graphics.main_image.w, G->graphics.main_image.h);
	G->crop_a.x = clamp(G->crop_a.x, 0, G->crop_b.x - 1);
	G->crop_b.x = clamp(G->crop_b.x, G->crop_a.x + 1, img_dim.x);
	G->crop_a.y = clamp(G->crop_a.y, 0, G->crop_b.y - 1);
	G->crop_b.y = clamp(G->crop_b.y, G->crop_a.y + 1, img_dim.y);

	if (!keypress(MouseL)) {
		drag_index = drag_free;
	}

	if (G->keys.double_click) {
		G->crop_mode = false;
		G->force_loop_frames++;
	}
	if (G->files.Count > 0 && keyup(Key_C)) {
		if (keypress(Key_Ctrl)) {
			if (SUCCEEDED(save_image(Format_Bmp, NULL, true))) {
				push_alert("Image copied successfully!", Alert_Info);
			} else {
				push_alert("Failed to copy image to clipboard");
			}
		} else {
			G->crop_mode = !G->crop_mode;
			G->force_loop_frames++;
		}
	}

    {
        v2 diff = v2(keypress(Key_D) - keypress(Key_A), keypress(Key_S) - keypress(Key_W)) *
                  G->settings_movementmag / (1 + G->settings_shiftslowmag * keypress(Key_Shift));
        if (G->settings_movementinvert)
            diff = -diff;
        G->position += diff;
    }
	if (G->files.Count > 0 && (G->files[G->current_file_index].type == TYPE_GIF|| G->files[G->current_file_index].type == TYPE_WEBP_ANIM)) {
        int reqindex = G->anim_index;
        if (keypress(Key_Up)) {
            reqindex--;
            keyrelease(Key_Up);
        }
        if (keypress(Key_Down)) {
            reqindex++;
            keyrelease(Key_Down);
        }
        if (keypress(Key_Space)) {
            keyrelease(Key_Space);
            G->anim_play = !G->anim_play;
        }
        G->anim_index = clamp(reqindex, 0, G->anim_frames - 1);
    }

    G->graphics.aspect_wnd = (float)WW / WH;
    G->graphics.aspect_img = (float)G->graphics.main_image.w / G->graphics.main_image.h;

    float prev_scale = G->scale;
    v2 prev_Position = G->position;
    bool updatescalebar = false;
	handle_signal(G->signals.update_scale_ui) {
		updatescalebar = true;
	}

    if (G->loaded && G->files.Count > 0) {
        if (!WantCaptureMouse)
            G->scale *= 1 + G->keys.scroll_y_diff * 0.1 / (1 + G->settings_shiftslowmag * keypress(Key_Shift));
        if (G->files[G->current_file_index].type != 1) {
            G->files[G->current_file_index].pos = G->position;
            G->files[G->current_file_index].scale = G->truescale;
        }
		if (G->keys.scroll_y_diff > 0)
			G->files[G->current_file_index].scaled = true;
    }
    
    if (G->graphics.main_image.w > 0) {
        if (G->graphics.aspect_img < G->graphics.aspect_wnd) {
            G->truescale = (float)WH / G->graphics.main_image.h * G->scale;
        } else {
            G->truescale = (float)WW / G->graphics.main_image.w * G->scale;
        }
        v2 M = G->keys.Mouse;
		v2 img_dim = v2(G->graphics.main_image.w, G->graphics.main_image.h);
		v2 wnd = v2(WW, WH);
		G->pixel_mouse = (M - (wnd - img_dim * G->truescale) * 0.5 - G->position) / G->truescale;
    } 
    {
        v2 Mouse = G->keys.Mouse - v2(WW, WH) / 2;

        v2 M = G->keys.Mouse;

        float TS = G->truescale;
        if (!G->signals.setting_applied) {
            G->position -= Mouse;
            G->position *= G->scale / prev_scale;
            G->position += Mouse;
        }
        if ((G->show_gui) || G->settings_always_show_gui) {
			TS = G->truescale_edit;
        }
        if (G->files.Count > 0) {
            if (keypress(Key_Q)) {
                TS *= 1 + 0.05 / (1 + G->settings_shiftslowmag * keypress(Key_Shift));
                updatescalebar = true;
            }
            if (keypress(Key_E)) {
                TS *= 1 - 0.05 / (1 + G->settings_shiftslowmag * keypress(Key_Shift));
                updatescalebar = true;
            }
        }
        if (G->signals.update_truescale || updatescalebar) {
            handle_signal(G->signals.update_truescale) {
                TS = G->req_truescale;
            }

            TS = clamp(TS, 0.1, 500.f);

            if (G->graphics.aspect_img < G->graphics.aspect_wnd)
                G->scale = TS * (float)G->graphics.main_image.h / WH;
            else
                G->scale = TS * (float)G->graphics.main_image.w / WW;

            if (!G->signals.setting_applied) G->position *= G->scale / prev_scale;
        }
    }
    G->signals.setting_applied = false;
}

static void UI_check_mouse() {
	UI_Context *ctx = G->ui;
	G->ui_want_capture_mouse = false;
	for (int i = 0; i < G->check_mouse_hashes.count; i++) {
		UI_Block *ref = UI_find_block(ctx, G->check_mouse_hashes[i], UI_PREVIOUS);
		if (ref) {
			if (UI_point_in_rect(ref->position, ref->position + ref->size, UI_get_mouse())) {
				G->ui_want_capture_mouse = true;
				break;
			}
		}
	}
	G->check_mouse_hashes.reset_count();
}

static void render_histogram() {
	if (!G->histo_block) return;
	Graphics* ctx = &G->graphics;
	UINT stride = sizeof(v2);
	UINT offset = 0;
	ctx->device_ctx->OMSetRenderTargets(1, &ctx->frame_buffer_view, ctx->depth_buffer_view);
	ctx->device_ctx->OMSetDepthStencilState(ctx->depth_stencil_state, 0);
	ctx->device_ctx->OMSetBlendState(ctx->blend_state, nullptr, 0xffffffff);
	D3D11_VIEWPORT viewport = { 0.0f, 0.0f, float(WW), float(WH), 0.0f, 1.0f };
	ctx->device_ctx->RSSetViewports(1, &viewport);
	ctx->device_ctx->IASetInputLayout(ctx->lines_program.input_layout);
	ctx->device_ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);
	ctx->device_ctx->IASetVertexBuffers(0, 1, &G->graphics.lines_vertex_buffer, &stride, &offset);
	ctx->device_ctx->VSSetShader(ctx->lines_program.vertex_shader, nullptr, 0);
	ctx->device_ctx->VSSetConstantBuffers(0, 1, &ctx->lines_program.constants_buffer);
	ctx->device_ctx->PSSetShader(ctx->lines_program.pixel_shader, nullptr, 0);
	ctx->device_ctx->PSSetConstantBuffers(0, 1, &ctx->lines_program.constants_buffer);
	ctx->device_ctx->ClearDepthStencilView(ctx->depth_buffer_view, D3D11_CLEAR_DEPTH, 1.0f, 0);

	for (int i = 0; i < 4; i++) {
		v2* target = G->p_histo_t;
		v4 color = v4(1, 1, 1, 1);
		switch (i) {
			case 0: if (!G->draw_histo_t) continue; break;
			case 1: if (!G->draw_histo_b) continue; target = G->p_histo_b; color = v4(0, 0, 1, 1); break;
			case 2: if (!G->draw_histo_g) continue; target = G->p_histo_g; color = v4(0, 1, 0, 1); break;
			case 3: if (!G->draw_histo_r) continue; target = G->p_histo_r; color = v4(1, 0, 0, 1); break;
		}

		D3D11_MAPPED_SUBRESOURCE mapped_data;
		ctx->device_ctx->Map(G->graphics.lines_vertex_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_data);
		Vertex* buffer_data = (Vertex*)mapped_data.pData;
		memcpy(buffer_data, target, sizeof(v2) * 256);
		ctx->device_ctx->Unmap(G->graphics.lines_vertex_buffer, 0);


		Shader_Constants_Lines constants_lines;
		constants_lines.window = v2(WW, WH);
		constants_lines.offset = G->histo_block->position + v2(2, 2);
		constants_lines.color = color;
		upload_constants(&ctx->lines_program, &constants_lines);
		ctx->device_ctx->Draw(256, 0);
	}
}

static void render() {
	Graphics* ctx = &G->graphics;
	ID3D11ShaderResourceView** target_srv = 0;
	bool force_nearest = false;

	if (G->signals.init_step_2 || G->loaded || G->files.Count == 0) {
		handle_signal(G->signals.init_step_2) {
			load_image_post();
			handle_signal(G->signals.update_orientation_step_2) {
				if (G->graphics.main_image.orientation % 2 == 1) {
					swap(int, G->graphics.main_image.w, G->graphics.main_image.h);
				}
			}
			reset_image_edit();
			
			G->loaded = true;
			if (G->loading_dropped_file) {
				G->loading_dropped_file = false;
				free(global_temp_path);
			}
		}

		if (G->files.Count > 0 && !G->files[G->current_file_index].failed) { // check if we have a folder open and no failed to load image 
			if ((G->files[G->current_file_index].type == TYPE_GIF || G->files[G->current_file_index].type == TYPE_WEBP_ANIM) && G->anim_frames > 0) {
				static uint32_t time = 0;
				uint32_t delta = get_ticks() - time;
				if (G->anim_play && !G->minimized)
					G->force_loop = true;
				if (delta >= G->anim_frame_delays[G->anim_index] && G->anim_play) {
					G->anim_index++;
					if (G->anim_index == G->anim_frames) {
						G->anim_index = 0;
					}
					time = get_ticks();
				}
				upload_texture(&G->anim_texture, G->anim_buffer + u32(G->anim_index * G->anim_texture.size.x * G->anim_texture.size.y * 4),
				               G->anim_texture.size.x * G->anim_texture.size.y * 4);
				target_srv = &G->anim_texture.srv;
			} else {
				target_srv = &G->graphics.main_image.texture.srv;
			}
		} else {
			target_srv = &G->graphics.logo_image.srv;
			force_nearest = true;
			set_to_no_file();
		}

		G->graphics.aspect_wnd = (float)WW / WH;
		G->graphics.aspect_img = (float)G->graphics.main_image.w / G->graphics.main_image.h;

		//clear
		f32 color[4] = { bg_color[0], bg_color[1], bg_color[2], 1.0f };
		ctx->device_ctx->ClearRenderTargetView(ctx->frame_buffer_view, color);
		ctx->device_ctx->ClearDepthStencilView(ctx->depth_buffer_view, D3D11_CLEAR_DEPTH, 1.0f, 0);

		//common stuff
		ctx->device_ctx->OMSetRenderTargets(1, &ctx->frame_buffer_view, ctx->depth_buffer_view);
		ctx->device_ctx->OMSetDepthStencilState(ctx->depth_stencil_state, 0);
		ctx->device_ctx->OMSetBlendState(ctx->blend_state, nullptr, 0xffffffff);
		D3D11_VIEWPORT viewport = { 0.0f, 0.0f, float(WW), float(WH), 0.0f, 1.0f };
		ctx->device_ctx->RSSetViewports(1, &viewport);
		ctx->device_ctx->RSSetState(ctx->raster_state);
		ctx->device_ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		ctx->device_ctx->IASetInputLayout(nullptr);
		if (G->nearest_filtering || force_nearest) 
			ctx->device_ctx->PSSetSamplers(0, 1, &ctx->sampler_nearest);
		else 
			ctx->device_ctx->PSSetSamplers(0, 1, &ctx->sampler_linear);

		//draw checkerboard
		Shader_Constants_BG constants_bg;
		constants_bg.window = v2(WW, WH);
		constants_bg.size = Checkerboard_size;
		constants_bg.color1 = v3(checkerboard_color_1[0], checkerboard_color_1[1], checkerboard_color_1[2]);
		constants_bg.color2 = v3(checkerboard_color_2[0], checkerboard_color_2[1], checkerboard_color_2[2]);
		constants_bg.bg = v4(bg_color[0], bg_color[1], bg_color[2], bg_color[3]);
		upload_constants(&ctx->bg_program, &constants_bg);
		ctx->device_ctx->VSSetShader(ctx->bg_program.vertex_shader, nullptr, 0);
		ctx->device_ctx->VSSetConstantBuffers(0, 1, &ctx->bg_program.constants_buffer);
		ctx->device_ctx->PSSetShader(ctx->bg_program.pixel_shader, nullptr, 0);
		ctx->device_ctx->PSSetConstantBuffers(0, 1, &ctx->bg_program.constants_buffer);
		ctx->device_ctx->Draw(4, 0);
	
		//draw image
		Shader_Constants_Main constants_main = set_main_shader_constants();
		if (force_nearest) { 
			constants_main.do_blur = false;
			constants_main.crop_mode = true; // hack
		}
		upload_constants(&ctx->main_program, &constants_main);
		ctx->device_ctx->VSSetShader(ctx->main_program.vertex_shader, nullptr, 0);
		ctx->device_ctx->VSSetConstantBuffers(0, 1, &ctx->main_program.constants_buffer);
		ctx->device_ctx->PSSetShader(ctx->main_program.pixel_shader, nullptr, 0);
		ctx->device_ctx->PSSetConstantBuffers(0, 1, &ctx->main_program.constants_buffer);
		if (target_srv)
			ctx->device_ctx->PSSetShaderResources(0, 1, target_srv); 	
		ctx->device_ctx->Draw(4, 0);

		if (G->crop_mode) {
			//draw crop overlay
			Shader_Constants_Crop constants_crop;
			v2 image_dim = v2(G->graphics.main_image.w, G->graphics.main_image.h);
			constants_crop.window = v2(WW, WH);
			constants_crop.image_pos = v2(G->position.x, -G->position.y) - G->truescale * image_dim.invert_y() / 2;
			constants_crop.crop_a = constants_crop.image_pos + _v2(G->crop_a.invert_y()) * 2 * G->truescale;
			constants_crop.crop_b = constants_crop.image_pos + _v2(G->crop_b.invert_y()) * 2 * G->truescale;
			constants_crop.scale = G->scale;
			constants_crop.image_dim = image_dim;
			upload_constants(&ctx->crop_program, &constants_crop);
			ctx->device_ctx->VSSetShader(ctx->crop_program.vertex_shader, nullptr, 0);
			ctx->device_ctx->VSSetConstantBuffers(0, 1, &ctx->crop_program.constants_buffer);
			ctx->device_ctx->PSSetShader(ctx->crop_program.pixel_shader, nullptr, 0);
			ctx->device_ctx->PSSetConstantBuffers(0, 1, &ctx->crop_program.constants_buffer);
			ctx->device_ctx->Draw(4, 0);
		}
	}

	G->imgui_in_frame = false;
	UI_end_frame(G->ui);
	UI_render(G->ui);

	render_histogram();

	ctx->swap_chain->Present(1, 0);

	if (keypress(Key_Ctrl) || keypress(MouseM)) {
		G->read_px = read_texture_pixel(G->graphics.frame_buffer, _iv2(G->keys.Mouse));
	}
}
