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
};

Texture2D<float4> 	image_texture 	: register(t0);
SamplerState 		texture_sampler	: register(s0);

#define PI 3.14159265359

struct VS_Output
{
	float4 Pos 	: SV_POSITION;  // This is still essential for rasterization, even if the PS doesn't use it.
	float2 UV 	: TEXCOORD0;    
};

float2 rotate(float2 p, float angle) {
	float2 result;
	result.x = p.x * cos(angle)  - p.y * sin(angle);
	result.y = p.x * sin(angle)  + p.y * cos(angle);
	return result;
}

VS_Output vs_main(uint id : SV_VertexID)
{
	uint x = id % 2;
	uint y = id / 2;
	float2 UV = float2(x, 1.0 -y);
	float2 P = float2(x, y) * 2.0 - 1.0;


	float aspect = aspect_img;
	if (aspect < aspect_wnd)
		P.x *= aspect / aspect_wnd;
	else
		P.y *= aspect_wnd / aspect;


	P = rotate(P * scale, PI/2 * rotation);

	VS_Output output;
	output.UV = UV;
	output.Pos = float4(P + position,0,1);
	return output;
} 

float4 adjust_hue(float4 color, float h)
{
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

float4 adjust_brightness(float4 color, float b)
{
	if (b == 0) return color;
	return float4(color.rgb + b, color.a);
}

float4 adjust_contrast(float4 color, float c)
{
	if (c == 1) return color;
	float3 mid = float3(0.5, 0.5, 0.5);
	color.rgb = mid + c * (color.rgb - mid);
	return color;
}

float4 adjust_saturation(float4 color, float s)
{
	if (s == 1) return color;
	const float3 luminance = float3(0.2125, 0.7154, 0.0721);
	float gray = dot(color.rgb, luminance);
	return float4(lerp(gray, color.rgb, s), color.a);
}

float4 adjust_gamma(float4 color, float g)
{
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

float4 modify_color(float4 color)
{
	color = adjust_hue(color, hue);
	color = adjust_saturation(color, saturation);
	color = adjust_contrast(color, contrast);
	color = adjust_brightness(color, brightness);
	color = adjust_gamma(color, gamma);
	if (srgb == 1)
		color = to_linear_4(color);
	return color;
}

float4 ps_main(VS_Output input) : SV_TARGET
{   
	if (pixel_grid == 1)
	{
		float2 uvFraction = 1.0 / image_dim;
		if (any(input.UV % uvFraction < uvFraction / true_scale)) {
			float4 color = float4(1,1,1,1) - image_texture.Sample(texture_sampler, input.UV );
			color = modify_color(color);
			return float4(color.rgb, 1);
		}
		else
		{  
			float4 color = image_texture.Sample(texture_sampler, input.UV) * float4(RGBAflags.rgb, 1);
			if (RGBAflags.a == 0.0)
				color.a = 1.0;
			return color;
		}
	}
	else
	{
		float4 color = image_texture.Sample(texture_sampler, input.UV) * float4(RGBAflags.rgb, 1);
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

struct VS_Output
{
	float4 Pos 	: SV_POSITION;
	float2 UV 	: TEXCOORD0;    
};

VS_Output vs_bg(uint id : SV_VertexID)
{
	uint x = id % 2;
	uint y = id / 2;
	float2 P = float2((float)x, (float)y) * 2.0 - 1.0;
	VS_Output output;
	output.UV = P;
	output.Pos = float4(P, 0, 1);
	return output;
} 

float4 ps_bg(VS_Output input) : SV_TARGET
{  
	float2 pixel = input.UV * window_dim / size;
	if ((frac(pixel.x) < 0.5) == (frac(pixel.y) < 0.5))
		return float4(lerp(color1, bg.rgb, bg.a),1);
	else
		return float4(lerp(color2, bg.rgb, bg.a),1);
}
)###";
