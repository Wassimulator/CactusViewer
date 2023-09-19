#pragma once
#include "ui_core.h"

enum struct UI_D3D11_Shader_Type {
    Vertex,
    Pixel,
};

struct UI_D3D11_Constants {
    // these are V4 to account for 16 byte alignment rules in HLSL constant buffers.
    v4 resolution;
    v4 texture_size[UI_MAX_TEXTURES]; 
};

struct UI_D3D11_Texture {
    v2                          size;
    ID3D11Texture2D*            texture;
    ID3D11ShaderResourceView*   srv;
};

struct UI_D3D11_Context {
    ID3D11Device1*              device;
    ID3D11DeviceContext1*       device_ctx;

    IDXGIFactory*               factory;
    
    ID3D11DepthStencilState*    depth_stencil_state;
    ID3D11RasterizerState*      raster_state;
    ID3D11BlendState*           blend_state;
    ID3D11SamplerState*         sampler_state;

    ID3D11InputLayout*          input_layout;
    
    ID3D11VertexShader*         vertex_shader;
    ID3D11PixelShader*          pixel_shader;

    ID3D11Buffer*               constant_buffer;
    ID3D11Buffer*               vertex_buffer;
    ID3D11ShaderResourceView*   vertex_SRV;

    UI_D3D11_Texture            textures[UI_MAX_TEXTURES];
};
