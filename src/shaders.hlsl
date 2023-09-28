char *shader_text_main = R"###(
cbuffer Main_Constants : register(b0) {
	float aspect_img;
	float aspect_wnd;
	float scale;
	float _padding0;

	float2 position;
	int pixel_grid;
	float true_scale;
	
	float2 image_dim;
	float2 window;

	float4 RGBAflags;

	int rotation;
	float hue;
	float saturation;
	float contrast;

	float brightness;
	float gamma;
	int srgb;
	int render_mode;

	int do_blur;
	uint blur_samples;
	uint blur_lod;
	float blur_scale;

	float2 crop_a;
	float2 crop_b;
	int crop_mode;

};

Texture2D<float4> 	image_texture 	: register(t0);
SamplerState 		texture_sampler	: register(s0);

#define PI 3.14159265359
#define RENDER_MODE_VEIWER	0
#define RENDER_MODE_ENCODER	1

struct VS_Output {
	float4 pos 	  		: SV_POSITION;  // This is still essential for rasterization, even if the PS doesn't use it.
	float2 uv 	  		: TEXCOORD0;    
	float2 crop_a 		: DATA0;
	float2 crop_b 		: DATA1;
	float2 uv_original 	: DATA2;    

};

float2 rotate(float2 p, float angle) {
	float2 result;
	result.x = p.x * cos(angle)  - p.y * sin(angle);
	result.y = p.x * sin(angle)  + p.y * cos(angle);
	return result;
}

float2 rotate_uv(float2 uv) {  
    float2 center = uv - float2(0.5, 0.5);
    float2 rot = center;
    switch (rotation) {
        case 0:	rot = center;                          break;
        case 1: rot = float2(-center.y,  center.x);    break;
        case 2: rot = float2(-center.x, -center.y);    break;
        case 3:	rot = float2( center.y, -center.x);    break;
    }
    return rot + float2(0.5, 0.5);
}

uint get_corner_id(float2 uv) {
	if ((uint)uv.x == 1 && (uint)uv.y == 0) return 1;
	if ((uint)uv.x == 0 && (uint)uv.y == 1) return 2;
	if ((uint)uv.x == 1 && (uint)uv.y == 1) return 3;
	return 0;
}

float2 set_uv_as_cropped(float2 uv) {
	float2 res = uv;
	switch (get_corner_id(uv)) {
		case 0:	res = crop_a / image_dim;						break;
		case 1:	res = float2(crop_b.x, crop_a.y) / image_dim;	break;
		case 2:	res = float2(crop_a.x, crop_b.y) / image_dim;	break;
		case 3:	res = crop_b / image_dim;						break;
	}
	return rotate_uv(res);
}

VS_Output vs_main(uint id : SV_VertexID) {
	uint x = id % 2;
	uint y = id / 2;
	float2 uv_original = float2(x, 1.0 -y);
	float2 uv_rot = rotate_uv(uv_original);
	float2 pos = float2(x, y) * 2.0 - 1.0;

	VS_Output output;
	output.uv = uv_rot;
	output.uv_original = uv_original;
	output.crop_a = crop_a;
	output.crop_b = crop_b;
	output.pos = float4(pos, 0, 1);

	if (render_mode == RENDER_MODE_VEIWER) {
		if (aspect_img < aspect_wnd)	pos.x *= aspect_img / aspect_wnd;
		else							pos.y *= aspect_wnd / aspect_img;
		output.pos = float4(pos *scale + position, 0, 1);
	} else if (render_mode == RENDER_MODE_ENCODER) {
		output.crop_a = float2(0,0);
		output.crop_b = image_dim;
		output.uv = set_uv_as_cropped(uv_original);
		output.pos = float4(pos, 0, 1);
	}

	return output;
} 

bool point_in_rect(float2 p0, float2 p1, float2 q) {
	float2 rect_min = float2(min(p0.x, p1.x), min(p0.y, p1.y));
	float2 rect_max = float2(max(p0.x, p1.x), max(p0.y, p1.y));
	return (q.x >= rect_min.x && 
		q.x <= rect_max.x && 
		q.y >= rect_min.y && 
		q.y <= rect_max.y);
}

float4 adjust_hue(float4 color, float h) {
	if (h == 0) return color;
	float3 k = float3(0.57735, 0.57735, 0.57735); //normalized axis for RGB
	float cosAngle = cos(h);
	float sinAngle = sin(h);
	float3x3 hueRotation = 
	{
		cosAngle + (1.0 - cosAngle) * k.x * k.x,
		(1.0 - cosAngle) * k.x * k.y - sinAngle * k.z,
		(1.0 - cosAngle) * k.x * k.z + sinAngle * k.y,

		(1.0 - cosAngle) * k.x * k.y + sinAngle * k.z,
		cosAngle + (1.0 - cosAngle) * k.y * k.y,
		(1.0 - cosAngle) * k.y * k.z - sinAngle * k.x,

		(1.0 - cosAngle) * k.x * k.z - sinAngle * k.y,
		(1.0 - cosAngle) * k.y * k.z + sinAngle * k.x,
		cosAngle + (1.0 - cosAngle) * k.z * k.z
	};
	return float4(mul(color.rgb, hueRotation), color.a);
}

float4 adjust_brightness(float4 color, float b) {
	if (b == 0) return color;
	return float4(color.rgb + b, color.a);
}

float4 adjust_contrast(float4 color, float c) {
	if (c == 1) return color;
	float3 mid = float3(0.5, 0.5, 0.5);
	color.rgb = mid + c * (color.rgb - mid);
	return color;
}

float4 adjust_saturation(float4 color, float s) {
	if (s == 1) return color;
	const float3 luminance = float3(0.2125, 0.7154, 0.0721);
	float gray = dot(color.rgb, luminance);
	return float4(lerp(gray, color.rgb, s), color.a);
}

float4 adjust_gamma(float4 color, float g) {
	if (g == 0.0f)
		return float4(1.0f, 1.0f, 1.0f, color.a);  // Avoid division by zero

	color.rgb = pow(abs(color.rgb), float3(1.0f / g, 1.0f / g, 1.0f / g));
	return color;
}

float to_linear(float x) {
	return lerp(pow((abs(x) + 0.055) / 1.055, 2.4), x / 12.92, step(x, 0.04045));
}

float alpha_to_linear(float alpha) {
	return 1.0 - (1.0 - alpha) * (1.0 - alpha);
}

float4 to_linear_4(float4 color) {
	return float4(to_linear(color.r), to_linear(color.g), to_linear(color.b), alpha_to_linear(color.a));
}

static const uint sLOD = 1 << blur_lod;
static const float sigma = float(blur_samples) * 0.25;

float gaussian(float2 i) {
	i /= sigma;
	return exp(-0.5 * dot(i, i)) / (6.28 * sigma * sigma);
}

float4 modify_color(float4 color) {
	color = adjust_hue(color, hue);
	color = adjust_saturation(color, saturation);
	color = adjust_contrast(color, contrast);
	color = adjust_brightness(color, brightness);
	color = adjust_gamma(color, gamma);
	if (srgb == 1)
		color = to_linear_4(color);
	return color;
}

float4 blur(float2 uv, float2 scale) {
	float4 output = float4(0, 0, 0, 0);
	uint s = blur_samples / sLOD;
    
	for (uint i = 0; i < s * s; i++) {
		float2 d = float2(i % s, i / s) * float(sLOD) - float(blur_samples) * 0.5;
		output += gaussian(d) * image_texture.SampleLevel(texture_sampler, uv + scale * d, float(blur_lod));
	}
	return output / output.a;
}

float4 sample_texture(float2 uv) {
	if (do_blur == 1)
		return blur(uv, blur_scale);
	else 
		return image_texture.Sample(texture_sampler, uv);
}

float4 ps_main(VS_Output input) : SV_TARGET { 
	if (crop_mode == 0 && render_mode == RENDER_MODE_VEIWER) {
		float2 pixel = input.uv_original * image_dim;
		if (!point_in_rect(input.crop_a, input.crop_b, pixel)) {
			discard;
		}
	}
	float4 sampled_color = sample_texture(input.uv);
	if (pixel_grid == 1) {
		float2 uvFraction = 1.0 / image_dim;
		if (any(input.uv % uvFraction < uvFraction / true_scale)) {
			float4 color = float4(1, 1, 1, 1) - sampled_color;
			color = modify_color(color);
			return float4(color.rgb, 1);
		} else {  
			float4 color = sampled_color * float4(RGBAflags.rgb, 1);
			if (RGBAflags.a == 0.0)
				color.a = 1.0;
			return color;
		}
	} else {
		float4 color = sampled_color * float4(RGBAflags.rgb, 1);
		color = modify_color(color);
		if (RGBAflags.a == 0.0)
			color.a = 1.0;
		return color;
	}
}
)###";

/////////////////////////////////

char *shader_text_bg = R"###(
cbuffer BG_Constants : register(b0) {
	float2 window_dim;
	float size;
	float _padding1;

	float3 color1;
	float _padding2;

	float3 color2;
	float _padding3;

	float4 bg;
};

struct VS_Output {
	float4 pos 	: SV_POSITION;
	float2 uv 	: TEXCOORD0;    
};

VS_Output vs_bg(uint id : SV_VertexID) {
	uint x = id % 2;
	uint y = id / 2;
	float2 P = float2((float)x, (float)y) * 2.0 - 1.0;
	VS_Output output;
	output.uv = P;
	output.pos = float4(P, 0, 1);
	return output;
} 

float4 ps_bg(VS_Output input) : SV_TARGET {  
	float2 pixel = input.uv * window_dim / size;
	if ((frac(pixel.x) < 0.5) == (frac(pixel.y) < 0.5))
		return float4(lerp(color1, bg.rgb, bg.a),1);
	else
		return float4(lerp(color2, bg.rgb, bg.a),1);
}
)###";

/////////////////////////////////

char *shader_text_crop = R"###(
cbuffer crop_Constants : register(b0) {
	float2 window_dim;
	float2 image_pos;

	float2 crop_a;
	float2 crop_b;

	float2 image_dim;
	float scale;
};

struct VS_Output {
	float4 pos	: SV_POSITION;
	float2 uv	: TEXCOORD0;    
};

VS_Output vs_crop(uint id : SV_VertexID) {
	uint x = id % 2;
	uint y = id / 2;
	float2 P = float2((float)x, (float)y) * 2.0 - 1.0;
	VS_Output output;
	output.uv = P;
	output.pos = float4(P, 0, 1);
	return output;
} 

bool point_in_rect(float2 p0, float2 p1, float2 q) {
	float2 rect_min = float2(min(p0.x, p1.x), min(p0.y, p1.y));
	float2 rect_max = float2(max(p0.x, p1.x), max(p0.y, p1.y));
	return (q.x >= rect_min.x && 
			q.x <= rect_max.x && 
			q.y >= rect_min.y && 
			q.y <= rect_max.y);
}

float4 ps_crop(VS_Output input) : SV_TARGET {  
	float2 pixel = input.uv * window_dim;
	float2 uvp = 1 / image_dim;
	if (abs(pixel.x - (image_pos.x + crop_a.x)) < 3 ||
		abs(pixel.x - (image_pos.x + crop_b.x)) < 3 ||
		abs(pixel.y - (image_pos.y + crop_a.y)) < 3 ||
		abs(pixel.y - (image_pos.y + crop_b.y)) < 3) {
		return float4(0, 0, 1, 1);
	}

	if (!point_in_rect(image_pos + crop_a, image_pos + crop_b, pixel))
		return float4(0, 0, 0, 0.5);
	
	return float4(0, 0, 0, 0);
}
)###";
/////////////////////////////////

char *shader_text_lines = R"###(
struct VS_Input {
    float2 pos : POSITION;
};

cbuffer Lines_Constants : register(b0) {
	float4 color;
    float2 viewport_size;
    float2 pos_offset;
};

float4 vs_lines(VS_Input input) : SV_POSITION {
	float4 position = float4((pos_offset + input.pos) / viewport_size * 2.0f - 1.0f, 0.0f, 1.0f);
	position.y = -position.y;
	return position;
}

float4 ps_lines() : SV_Target {
    return color;
}

)###";