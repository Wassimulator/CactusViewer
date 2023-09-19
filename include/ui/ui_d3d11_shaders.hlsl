char* UI_d3d11_shader_string = R"###(
#define MAX_TEXTURES 5

cbuffer Constants : register(b0) {
    float2 res : packoffset(c0); //frame resolution
    float2 texture_size[MAX_TEXTURES] : packoffset(c1);
};

#define    UI_Vertex_Flags_srgb      (1<<1)
#define    UI_Vertex_Flags_lcd       (1<<2)

struct VS_Input {
    float2  dst_p0;
    float2  dst_p1;
    float2  src_p0;
    float2  src_p1;
    float2  clp_p0;
    float2  clp_p1;
    float   depth;
    float4  colors[4];
    float   corner_radius[4];
    float   edge_softness;
    float   border_thickness;
    int     texture_id;
    int     ui_block;
    float   rotation;
    int     flags;
};

//note: Semantics are required on all variables passed between shader stages, even arbitrary ones.
struct PS_Input {
    float4  vertex              : SV_POSITION;
    float2  clp_p0              : POSITION0;
    float2  clp_p1              : POSITION1;
    float2  uv                  : TEXCOORD0;
    float2  dst_pos             : DATA0; 
    float2  dst_center          : DATA1;
    float2  dst_half_size       : DATA2;
    float   corner_radius       : DATA3;
    float   edge_softness       : DATA4;
    float   border_thickness    : DATA5;
    float4  color               : COLOR0;
    int     texture_id          : DATA6;
    int     ui_block            : DATA7;
	int		flags				: DATA8;
};

float rounded_rect_SDF(float2 sample_pos, float2 rect_center, float2 rect_half_size, float r) {
	r = r < 0 ? rect_half_size : r;
	float2 d2 = (abs(rect_center - sample_pos) - rect_half_size + float2(r, r));
	return min(max(d2.x, d2.y), 0.0) + length(max(d2, 0.0)) - r;
}

bool point_in_rect(float2 p0, float2 p1, float4 position) {
    float2 p = position.xy;
    float2 rect_min = min(p0, p1);
    float2 rect_max = max(p0, p1);
    return (p.x >= rect_min.x && 
            p.x <= rect_max.x && 
            p.y >= rect_min.y && 
            p.y <= rect_max.y);
} 

float2 rotate(float2 p, float angle) {
    float2 result;
    result.x = p.x * cos(angle) - p.y * sin(angle);
    result.y = p.x * sin(angle) + p.y * cos(angle);
    return result;
}

StructuredBuffer<VS_Input> vertex_buffer            : register(t0);
Texture2D<float4>          textures[MAX_TEXTURES]   : register(t1);
SamplerState               texture_sampler          : register(s0);

PS_Input VSMain(uint rect_id : SV_INSTANCEID, uint vertex_id : SV_VertexID) {
    VS_Input input = vertex_buffer[rect_id];
    static float2 vertices[] =
    { //reverse N configuration
      {-1, +1}, //A
      {-1, -1}, //D
      {+1, +1}, //B
      {+1, -1}, //C
    };

    // "dst" => "destination" (on screen)
    float2 dst_half_size = (input.dst_p1 - input.dst_p0) / 2;
    float2 dst_center = input.dst_p0 + (input.dst_p1 - input.dst_p0) / 2;
    float2 dst_pos = (vertices[vertex_id] * dst_half_size + dst_center);

    if (input.rotation != 0) {
        float2 vertex_pos = vertices[vertex_id] * dst_half_size;
        float2 rotated_vertex_pos = rotate(vertex_pos, input.rotation);
        dst_pos = rotated_vertex_pos + dst_center;
    }

    // "src" => "source" (on texture)
    float2 src_half_size = (input.src_p1 - input.src_p0) / 2;
    float2 src_center = (input.src_p1 + input.src_p0) / 2;
    float2 src_pos = (vertices[vertex_id] * src_half_size + src_center);

    float2 current_texture_size = float2(1,1);
    switch (input.texture_id) {
        case -1: break;
        case  0: current_texture_size = texture_size[0]; break;
        case  1: current_texture_size = texture_size[1]; break;
        case  2: current_texture_size = texture_size[2]; break;
        case  3: current_texture_size = texture_size[3]; break;
        case  4: current_texture_size = texture_size[4]; break;
    }

    // package output
    PS_Input output;
    output.vertex = float4(2 * dst_pos.x / res.x - 1,
                           2 * (res.y - dst_pos.y) / res.y - 1,
                           input.depth,
                           1);
    output.uv = float2(src_pos.x / current_texture_size.x,
                       src_pos.y / current_texture_size.y);
    output.color = input.colors[vertex_id];
	output.flags = input.flags;

    // calculated earlier
    output.dst_pos          = dst_pos;
    output.dst_center       = dst_center;
    output.dst_half_size    = dst_half_size;

    // pass-through, no vertex shader work to do
    output.clp_p0           = input.clp_p0;
    output.clp_p1           = input.clp_p1;
    output.corner_radius    = input.corner_radius[vertex_id];
    output.edge_softness    = input.edge_softness;
    output.border_thickness = input.border_thickness;
    output.texture_id       = input.texture_id;
    output.ui_block         = input.ui_block;

    return output;
}

float to_linear(float x) {
    return lerp(pow((x + 0.055) / 1.055, 2.4), x / 12.92, step(x, 0.04045));
}

float alpha_to_linear(float alpha) {
        return 1.0 - (1.0 - alpha) * (1.0 - alpha);
}

float4 to_linear_4(float4 color) {
    return float4(to_linear(color.r), to_linear(color.g), to_linear(color.b), alpha_to_linear(color.a));
}

struct PS_Output {
	float4 color : SV_TARGET0;
	float4 blend : SV_TARGET1;
};

PS_Output PSMain(PS_Input input) {
    if (input.clp_p0.x >= 0 && !point_in_rect(input.clp_p0, input.clp_p1, input.vertex)) discard;

    float4 sampled = float4(1,1,1,1);
    
    switch (input.texture_id) {
        case -1: break;
        case 0: sampled = textures[0].Sample(texture_sampler, input.uv); break;
        case 1: sampled = textures[1].Sample(texture_sampler, input.uv); break;
        case 2: sampled = textures[2].Sample(texture_sampler, input.uv); break;
        case 3: sampled = textures[3].Sample(texture_sampler, input.uv); break;
        case 4: sampled = textures[4].Sample(texture_sampler, input.uv); break;
    }

    // we need to shrink the rectangle's half-size that is used for distance calculations with
    // the edge softness - otherwise the underlying primitive will cut off the falloff too early.
    float2 softness = input.edge_softness;
    float2 softness_padding = max(0.0, softness * 2 - 1);

    // sample distance
    float dist = rounded_rect_SDF(input.vertex,
                                  input.dst_center,
                                  input.dst_half_size - softness_padding,
                                  input.corner_radius);

    // map distance => a blend factor
    float sdf_factor = 1.f - smoothstep(0, 2 * input.edge_softness, dist);

    float border_factor = 1.f;
    if(input.border_thickness != 0) {
        float2 interior_half_size = input.dst_half_size - float2(input.border_thickness, input.border_thickness);

        // reduction factor for the internal corner radius
        float interior_radius_reduce_f = min(interior_half_size.x / input.dst_half_size.x, interior_half_size.y / input.dst_half_size.y);
        float interior_corner_radius =
            (input.corner_radius *
             interior_radius_reduce_f *
             interior_radius_reduce_f);

        // calculate sample distance from "interior"
        float inside_d = rounded_rect_SDF(input.vertex,
                                        input.dst_center,
                                        interior_half_size-
                                        softness_padding,
                                        interior_corner_radius);

        // map distance => factor
        float inside_f = smoothstep(0, 2 * input.edge_softness, inside_d);
        border_factor = inside_f;
    }
	
	float4 color = input.color;
	if (input.flags & UI_Vertex_Flags_srgb) {
		color = to_linear_4(input.color); 
	}
	PS_Output output;
	output.color = sampled * color * border_factor * sdf_factor;
	output.blend = output.color.aaaa;
	if (input.flags & UI_Vertex_Flags_lcd) {
		output.color = color;
		output.blend = sampled * output.color.a;
	}

	return output;
}


)###";