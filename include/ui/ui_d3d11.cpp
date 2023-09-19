#pragma once
#include "ui_core.h"
#include "d3d11.h"
#include "ui_d3d11.h"
#include "ui_d3d11_shaders.hlsl"

void UI_d3d11_get_last_error(bool message = false) {
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

bool UI_d3d11_get_error(HRESULT hresult, bool message = false) {
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

#define err(x) UI_d3d11_get_error(x)

static ID3DBlob * UI_d3d11_compile_memory(char *shader_code,
                                        size_t shader_code_size, 
                                        const char *entry_point,
                                        const char *target, 
                                        bool free_upon_finish = false) {

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

    if (free_upon_finish)
        free(shader_code);

    if (FAILED(result)) {
        printf("%*s\n", int(errors->GetBufferSize()), (char *)errors->GetBufferPointer());
        fflush(stdout);
        blob = nullptr;
    }
    return blob;
}
static ID3DBlob * UI_d3d11_compile_file(const char *path,
                                        const char *entry_point,
                                        const char *target) {
    FILE *file = fopen(path, "rb");
    if (file == nullptr) {
        printf("Could not open file %s\n", path);
        fflush(stdout);
        return false;
    }
    fseek(file, 0, SEEK_END);
    size_t shader_code_size = ftell(file);
    fseek(file, 0, SEEK_SET);

	char *shader_code = (char *)malloc(shader_code_size);
    if (fread(shader_code, shader_code_size, 1, file) != 1) {
        printf("Could not read file %s\n", path);
        fflush(stdout);
        fclose(file);
        return false;
    }
    fclose(file);

    return UI_d3d11_compile_memory(shader_code, shader_code_size, entry_point, target, true);
}
/**
 * @param UI_Context *ctx : the exisitng and already initialized context where this render backend should be added to.
 * **/
void UI_d3d11_init(UI_Context *ctx, ID3D11Device1* device, ID3D11DeviceContext1* device_context) {

    UI_assert(ctx != nullptr && "UI context *ctx is a null pointer!");
    UI_assert(ctx->initialized && "UI context wasn't initialized!");
    UI_assert(ctx->backend == UI_Render_Backend_Type::None && "A different backend was already initalized!");
    UI_assert(device != nullptr && "D3D11 device is null! check if device is correctly initialized.");
    UI_assert(device_context != nullptr && "D3D11 device context is null! check if device is correctly initialized.");
    UI_D3D11_Context *d3d_ctx = (UI_D3D11_Context *)malloc(sizeof(UI_D3D11_Context));
    d3d_ctx->device = device;
    d3d_ctx->device_ctx = device_context;
    ctx->backend = UI_Render_Backend_Type::D3D11;

    // Get factory from device
    IDXGIDevice1* DXGIDevice = NULL;
    IDXGIAdapter* DXGIAdapter = NULL;
    IDXGIFactory* factory = NULL;

    if (device->QueryInterface(IID_PPV_ARGS(&DXGIDevice)) == S_OK)
        if (DXGIDevice->GetParent(IID_PPV_ARGS(&DXGIAdapter)) == S_OK)
            if (DXGIAdapter->GetParent(IID_PPV_ARGS(&factory)) == S_OK)
            {
                d3d_ctx->device = device;
                d3d_ctx->device_ctx = device_context;
                d3d_ctx->factory = factory;
            }
    if (DXGIDevice)   DXGIDevice->Release();
    if (DXGIAdapter)  DXGIAdapter->Release();

	size_t shader_len = strlen(UI_d3d11_shader_string);

	ID3DBlob *vs_blob   = UI_d3d11_compile_memory(UI_d3d11_shader_string, shader_len, "VSMain", "vs_5_0");
    err(d3d_ctx->device->CreateVertexShader(vs_blob->GetBufferPointer(), vs_blob->GetBufferSize(), nullptr, &d3d_ctx->vertex_shader));
    
	ID3DBlob *ps_blob = UI_d3d11_compile_memory(UI_d3d11_shader_string, shader_len, "PSMain", "ps_5_0");
    err(d3d_ctx->device->CreatePixelShader(ps_blob->GetBufferPointer(), ps_blob->GetBufferSize(), nullptr, &d3d_ctx->pixel_shader));

    D3D11_BUFFER_DESC constant_buffer_desc = {};
    constant_buffer_desc.ByteWidth      = sizeof(UI_D3D11_Constants);
    constant_buffer_desc.Usage          = D3D11_USAGE_DYNAMIC;
    constant_buffer_desc.BindFlags      = D3D11_BIND_CONSTANT_BUFFER;
    constant_buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    err(d3d_ctx->device->CreateBuffer(&constant_buffer_desc, nullptr, &d3d_ctx->constant_buffer));

    D3D11_BUFFER_DESC vertex_buffer_desc = {};
    vertex_buffer_desc.ByteWidth = sizeof(UI_Vertex) * UI_MAX_VERTICES; 
    vertex_buffer_desc.Usage = D3D11_USAGE_DYNAMIC; 
    vertex_buffer_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    vertex_buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    vertex_buffer_desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    vertex_buffer_desc.StructureByteStride = sizeof(UI_Vertex);
    err(d3d_ctx->device->CreateBuffer(&vertex_buffer_desc, nullptr, &d3d_ctx->vertex_buffer));

    D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
    SRVDesc.Format             = DXGI_FORMAT_UNKNOWN;
    SRVDesc.ViewDimension      = D3D11_SRV_DIMENSION_BUFFER;
    SRVDesc.Buffer.NumElements = UI_MAX_VERTICES;
    err(d3d_ctx->device->CreateShaderResourceView(d3d_ctx->vertex_buffer, &SRVDesc, &d3d_ctx->vertex_SRV));

    D3D11_BLEND_DESC blend_desc = {};
    blend_desc.AlphaToCoverageEnable = false;
    blend_desc.RenderTarget[0].BlendEnable = true;
	blend_desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC1_COLOR;
    blend_desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC1_ALPHA;
    blend_desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blend_desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_SRC1_ALPHA;
	blend_desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC1_ALPHA;
    blend_desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blend_desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    d3d_ctx->device->CreateBlendState(&blend_desc, &d3d_ctx->blend_state);

    D3D11_RASTERIZER_DESC rast_desc = {};
    rast_desc.FillMode = D3D11_FILL_SOLID;
    rast_desc.CullMode = D3D11_CULL_NONE;
    rast_desc.ScissorEnable = false;
    rast_desc.DepthClipEnable = true;
    rast_desc.FrontCounterClockwise = true;
    d3d_ctx->device->CreateRasterizerState(&rast_desc, &d3d_ctx->raster_state);

    D3D11_DEPTH_STENCIL_DESC depth_desc = {};
    depth_desc.DepthEnable = false;
    depth_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    depth_desc.DepthFunc = D3D11_COMPARISON_ALWAYS;
    depth_desc.StencilEnable = false;
    depth_desc.FrontFace.StencilFailOp = depth_desc.FrontFace.StencilDepthFailOp = depth_desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    depth_desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
    depth_desc.BackFace = depth_desc.FrontFace;
    d3d_ctx->device->CreateDepthStencilState(&depth_desc, &d3d_ctx->depth_stencil_state);

    D3D11_SAMPLER_DESC sampler_desc = {};
    sampler_desc.Filter         = D3D11_FILTER_MIN_MAG_MIP_POINT;
    sampler_desc.AddressU       = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampler_desc.AddressV       = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampler_desc.AddressW       = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampler_desc.ComparisonFunc = D3D11_COMPARISON_NEVER;

    device->CreateSamplerState(&sampler_desc, &d3d_ctx->sampler_state);

    for (int i = 0; i < UI_MAX_TEXTURES; i++) {
        d3d_ctx->textures[i].texture =  nullptr;
        d3d_ctx->textures[i].srv =      nullptr;
    }

    ctx->backend_context = d3d_ctx;
}

void UI_d3d11_push_vertex(UI_Context *ctx, UI_Vertex vertex)  {
    UI_assert(ctx != nullptr && "UI context *ctx is a null pointer!");
    UI_assert(ctx->initialized && "UI context wasn't initialized!");
    UI_assert(ctx->backend == UI_Render_Backend_Type::D3D11 && "Wrong UI render backend initialized.");

    // auto-sorts the vertices by depth so that all can be sent in one draw call
    if (ctx->vertices.count == 0) {
        ctx->vertices.push_back(vertex);
        return;
    }
    for (int i = 0; i < ctx->vertices.count; i++) {
        if (vertex.depth <= ctx->vertices[i].depth) {
            ctx->vertices.insert(&ctx->vertices[i], vertex);
            return;
        }
    }
    ctx->vertices.push_front(vertex);
}

void UI_d3d11_render(UI_Context *ctx) {
    UI_assert(ctx != nullptr && "UI context *ctx is a null pointer!");
    UI_assert(ctx->initialized && "UI context wasn't initialized!");
    UI_D3D11_Context *d3d_ctx = (UI_D3D11_Context *)ctx->backend_context;
    UI_assert(d3d_ctx != nullptr && "d3d11 UI context not initialized!");
    if (ctx->vertices.count == 0) return;

    UI_D3D11_Constants constants;
    constants.resolution.x = ctx->viewport.x;
    constants.resolution.y = ctx->viewport.y;
    for (int i = 0; i < UI_MAX_TEXTURES; i++) {
        constants.texture_size[i].x = d3d_ctx->textures[i].size.x; 
        constants.texture_size[i].y = d3d_ctx->textures[i].size.y; 
    }
    D3D11_MAPPED_SUBRESOURCE mapped;
    d3d_ctx->device_ctx->Map(d3d_ctx->constant_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
    memcpy(mapped.pData, &constants, sizeof(UI_D3D11_Constants));
    d3d_ctx->device_ctx->Unmap(d3d_ctx->constant_buffer, 0);

    // upload vertex data
    ctx->vertices.reverse_order();
    {
        D3D11_MAPPED_SUBRESOURCE mappedResource = {};
        HRESULT hr = d3d_ctx->device_ctx->Map(d3d_ctx->vertex_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
        UI_Vertex* vertexData = (UI_Vertex*)mappedResource.pData;
        memcpy(vertexData, ctx->vertices.data, sizeof(UI_Vertex) * ctx->vertices.count);
        d3d_ctx->device_ctx->Unmap(d3d_ctx->vertex_buffer, 0);
    }

    // Input Assembler
    uint32_t stride = sizeof(UI_Vertex);
    uint32_t offset = 0;
    d3d_ctx->device_ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    d3d_ctx->device_ctx->IASetInputLayout(nullptr);

    // Vertex Shader
    d3d_ctx->device_ctx->VSSetShader(d3d_ctx->vertex_shader, nullptr, 0);
    d3d_ctx->device_ctx->VSSetShaderResources(0, 1, &d3d_ctx->vertex_SRV);
    d3d_ctx->device_ctx->VSSetConstantBuffers(0, 1, &d3d_ctx->constant_buffer);

    // Rasterizing stage
    D3D11_VIEWPORT viewport = { 0.0f, 0.0f, ctx->viewport.x, ctx->viewport.y, 0.0f, 1.0f };
    d3d_ctx->device_ctx->RSSetViewports(1, &viewport);
    d3d_ctx->device_ctx->RSSetState(d3d_ctx->raster_state);

    // Pixel shader stage
    d3d_ctx->device_ctx->PSSetShader(d3d_ctx->pixel_shader, nullptr, 0);
    d3d_ctx->device_ctx->PSSetConstantBuffers(0, 1, &d3d_ctx->constant_buffer);
    ID3D11ShaderResourceView* texture_array[UI_MAX_TEXTURES];
    for (int i = 0; i < UI_MAX_TEXTURES; i++) {
        texture_array[i] = d3d_ctx->textures[i].srv;
    }
    d3d_ctx->device_ctx->PSSetShaderResources(1, UI_MAX_TEXTURES, texture_array); 
    d3d_ctx->device_ctx->PSSetSamplers(0, 1, &d3d_ctx->sampler_state);
    d3d_ctx->device_ctx->OMSetBlendState(d3d_ctx->blend_state, nullptr, 0xffffffff);

    // draw `count` number of instances and 4 vertices per instance, using a triangle strip.
    d3d_ctx->device_ctx->DrawInstanced(4, ctx->vertices.count, 0, 0);
}

void UI_d3d11_create_texture(UI_Context *ctx, i32 handle, u8 *data, int w, int h) {
    UI_assert(ctx != nullptr && "UI context *ctx is a null pointer!");
    UI_assert(ctx->initialized && "UI context wasn't initialized!");
    UI_D3D11_Context *d3d_ctx = (UI_D3D11_Context *)ctx->backend_context;
    UI_assert(d3d_ctx != nullptr && "d3d11 UI context not initialized!");
    UI_assert(handle >= 0 && handle < UI_MAX_TEXTURES); 
    
    D3D11_TEXTURE2D_DESC texture_desc = {};
    texture_desc.Width             = w;
    texture_desc.Height            = h;
    texture_desc.MipLevels         = 1;
    texture_desc.ArraySize         = 1;
    texture_desc.Format            = DXGI_FORMAT_R8G8B8A8_UNORM;
    texture_desc.SampleDesc.Count  = 1;
    texture_desc.Usage             = D3D11_USAGE_IMMUTABLE;
    texture_desc.BindFlags         = D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA texture_SRD = {};
    texture_SRD.pSysMem     = data;
    texture_SRD.SysMemPitch = w * 4;

    UI_D3D11_Texture *texture = &d3d_ctx->textures[handle];
    texture->size = v2(w, h);
    err(d3d_ctx->device->CreateTexture2D(&texture_desc, &texture_SRD, &texture->texture));
    err(d3d_ctx->device->CreateShaderResourceView(texture->texture, nullptr, &texture->srv));
}

#undef err