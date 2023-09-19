#pragma once
#define _USE_MATH_DEFINES
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef sign
#define sign(x) ((x > 0) ? 1 : ((x < 0) ? -1 : 0))
#endif

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif


int rand_range(int min, int max);

template <typename T> struct vec2 {
	union {
		T e[2];
		struct { T x, y; };
	};

	vec2() : e { 0, 0 } {}
	vec2(T e0, T e1) : e{e0, e1} {}
	vec2(T e) : e{e, e} {}

	inline vec2 operator-() const 			{ return vec2(-x, -y); }
	inline T operator[](int i) const 		{ return e[i]; }
	inline T &operator[](int i) 			{ return e[i]; }
	inline vec2 &operator+=(const vec2 &v) 	{ x += v.x; y += v.y; return *this; }
	inline vec2 &operator-=(const vec2 &v) 	{ x -= v.x; y -= v.y; return *this; }
	inline vec2 &operator*=(const T t) 		{ x *= t; y *= t; return *this; }
	inline vec2 &operator/=(const T t) 		{ return *this *= static_cast<T>(1) / t; }
	inline T length_squared() const 		{ return x * x + y * y; }
	inline T length() const 				{ return sqrt(length_squared()); }
	inline T dot(vec2 V) 					{ return x * V.x + y * V.y; }
	inline vec2 perpendicular() 			{ return vec2(y, -x); }
	inline T perpdot(vec2 V) 				{ return x * V.y - y * V.x; }
	inline T cross(vec2 V) 					{ return x * V.y - y * V.x; }
	inline vec2 hadamard(vec2 V) 			{ return vec2(x * V.x, y * V.y); }
	inline T angle(vec2 V) 					{ return atan2(x * V.y - y * V.x, x * V.x + y * V.y); } // returns signed angle in radians
	inline vec2 normalize() { 
		T L = length(); 
		return (L > static_cast<T>(0)) ? vec2(x / L, y / L) : vec2(0, 0);
	}
};

template<typename T> struct vec3 {
	union {
		T e[3];
		struct { T x, y, z; };
		struct { T r, g, b; };
		struct { T h, s, v; };
	};

	vec3() : e { 0, 0, 0 } {}
	vec3(T e0, T e1, T e2) : e{e0, e1, e2} {}
	vec3(T e0) : e{e0, e0, e0} {}
	vec3(const T e[3]) : e{e[0], e[1], e[2]} {}

	inline vec3 operator-() const 			{ return vec3(-x, -y, -z); }
	inline T operator[](int i) const 		{ return e[i]; }
	inline T &operator[](int i) 			{ return e[i]; }
	inline vec3 &operator+=(const vec3 &v) 	{ x += v.x; y += v.y; z += v.z; return *this; }
	inline vec3 &operator-=(const vec3 &v)	{ x -= v.x; y -= v.y; z -= v.z; return *this; }
	inline vec3 &operator*=(const T t) 		{ x *= t; y *= t; z *= t; return *this; }
	inline vec3 &operator/=(const T t) 		{ return *this *= static_cast<T>(1) / t; }
	inline T length_squared() const 		{ return x * x + y * y + z * z; }
	inline T length() const 				{ return sqrt(length_squared()); }
	inline vec3 floor() 					{ return vec3(floor(x), floor(y), floor(z)); }
	inline vec3 normalize() {
		T L = length();
		return (L > static_cast<T>(0)) ? vec3(x / L, y / L, z / L) : vec3(0, 0, 0);
	}
};

template <typename T> struct vec4 {
	union {
		T e[4];
		struct { T x, y, z, w; };
		struct { T r, g, b, a; };
	};

	vec4() : e { 0, 0, 0, 0 } {}
	vec4(T e0, T e1, T e2, T e3) : e{e0, e1, e2, e3} {}
	vec4(T e0) : e { e0, e0, e0, e0 } {}
	vec4(const T e[4]) : e{e[0], e[1], e[2], e[3]} {}

	inline vec4 operator-() const 			{ return vec4(-x, -y, -z, -w); }
	inline T operator[](int i) const 		{ return e[i]; }
	inline T &operator[](int i) 			{ return e[i]; }
	inline vec4 &operator+=(const vec4 &v) 	{ x += v.x; y += v.y; z += v.z; w += v.w; return *this; }
	inline vec4 &operator-=(const vec4 &v) 	{ x -= v.x; y -= v.y; z -= v.z; w -= v.w; return *this; }
	inline vec4 &operator*=(const T t)		{ x *= t; y *= t; z *= t; w *= t; return *this; }
	inline vec4 &operator/=(const T t) 		{ return *this *= static_cast<T>(1) / t; }
	inline T length_squared() const 		{ return x * x + y * y + z * z + w * w; }
	inline T length() const 				{ return sqrt(length_squared()); }
	inline vec4 floor() 					{ return vec4(static_cast<int>(x), static_cast<int>(y), static_cast<int>(z), static_cast<int>(w)); }
	inline vec4 normalize() {
		T L = length();
		return (L > static_cast<T>(0)) ? vec4(x / L, y / L, z / L, w / L) : vec4(0, 0, 0, 0);
	}
};

typedef vec2<float> v2;
typedef vec3<float> v3;
typedef vec4<float> v4;
typedef vec2<int> 	iv2;
typedef vec3<int> 	iv3;
typedef vec4<int> 	iv4;

#define operations_vec2(T)\
inline T operator-(T a, T b)			{ return T(a.x - b.x, a.y - b.y); }\
inline T operator+(T a, T b)			{ return T(a.x + b.x, a.y + b.y); }\
inline T operator-(T a, float b)		{ return T(a.x - b  , a.y - b  ); }\
inline T operator+(T a, float b)		{ return T(a.x + b  , a.y + b  ); }\
inline T operator*(T a, float b)		{ return T(a.x * b  , a.y * b  ); }\
inline T operator*(T a, T b)			{ return T(a.x * b.x, a.y * b.y); }\
inline T operator*(float b, T a)		{ return T(a.x * b  , a.y * b  ); }\
inline T operator/(T a, float b)		{ return T(a.x / b  , a.y / b  ); }\
inline T operator/(T a, T b)			{ return T(a.x / b.x, a.y / b.y); }\
inline static float dot(T A, T B)		{ return A.x * B.x + A.y * B.y; }\
inline static float perpdot(T A, T B)	{ return A.x * B.y - A.y * B.x; }\
inline static bool operator==(T A , T B){ return A.x == B.x && A.y == B.y; }\
inline static bool operator!=(T A , T B){ return A.x != B.x || A.y != B.y; }

#define operations_vec3(T)\
inline T operator-(T a, T b) 			{ return T(a.x - b.x, a.y - b.y, a.z - b.z); }\
inline T operator+(T a, T b) 			{ return T(a.x + b.x, a.y + b.y, a.z + b.z); }\
inline T operator-(T a, float b) 		{ return T(a.x - b  , a.y - b  , a.z - b); }\
inline T operator+(T a, float b) 		{ return T(a.x + b  , a.y + b  , a.z + b); }\
inline T operator*(T a, float b) 		{ return T(a.x * b  , a.y * b  , a.z * b); }\
inline T operator*(T a, int b) 			{ return T(a.x * b  , a.y * b  , a.z * b); }\
inline T operator*(T a, T b) 			{ return T(a.x * b.x, a.y * b.y, a.z * b.z); }\
inline T operator*(float b     , T a) 	{ return T(a.x * b  , a.y * b  , a.z * b); }\
inline T operator/(T a, float b) 		{ return T(a.x / b  , a.y / b  , a.z / b); }\
inline T operator/(T a, T b) 			{ return T(a.x / b.x, a.y / b.y, a.z / b.z); }\
inline static bool operator==(T A , T B){ return A.x == B.x && A.y == B.y && A.z == B.z; }\
inline static bool operator!=(T A , T B){ return A.x != B.x || A.y != B.y || A.z != B.z; }\
inline static float dot(T A , T B)		{ return A.x *  B.x + A.y *  B.y + A.z *  B.z; }

#define operations_vec4(T)\
inline T operator-(T a, T b) 			{ return T(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w); }\
inline T operator+(T a, T b) 			{ return T(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w); }\
inline T operator-(T a, float b) 		{ return T(a.x - b  , a.y - b  , a.z - b  , a.w - b); }\
inline T operator+(T a, float b) 		{ return T(a.x + b  , a.y + b  , a.z + b  , a.w + b); }\
inline T operator*(T a, float b) 		{ return T(a.x * b  , a.y * b  , a.z * b  , a.w * b); }\
inline T operator*(T a, int b) 			{ return T(a.x * b  , a.y * b  , a.z * b  , a.w * b); }\
inline T operator*(T a, T b) 			{ return T(a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w); }\
inline T operator*(float b     , T a) 	{ return T(a.x * b  , a.y * b  , a.z * b  , a.w * b); }\
inline T operator/(T a, float b) 		{ return T(a.x / b  , a.y / b  , a.z / b  , a.w / b); }\
inline T operator/(T a, T b) 			{ return T(a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w); }\
inline static bool operator==(T A , T B){ return A.x == B.x && A.y == B.y && A.z == B.z && A.w == B.w; }\
inline static bool operator!=(T A , T B){ return A.x != B.x || A.y != B.y || A.z != B.z || A.w != B.w; }\
inline static float dot(T A , T B) 		{ return A.x *  B.x + A.y *  B.y + A.z *  B.z + A.w *  B.w; }

operations_vec2(v2)
operations_vec2(iv2)
operations_vec3(v3)
operations_vec3(iv3)
operations_vec4(v4)
operations_vec4(iv4)

inline static v2 rand_vector(float length) 	{ return v2(rand_range(-100, 100) *0.01f *length, rand_range(-100, 100) *0.01f *length); }

inline iv2 _iv2(v2 A ) { iv2 B; B.x = A.x; B.y = A.y;            return B;} 
inline v2  _v2 (iv2 A) { v2  B; B.x = A.x; B.y = A.y;            return B;}
inline iv3 _iv3(v3 A ) { iv3 B; B.x = A.x; B.y = A.y; B.z = A.z; return B;}
inline v3  _v3 (iv3 A) { v3  B; B.x = A.x; B.y = A.y; B.z = A.z; return B;}

inline float lerp(float a, float b, float t) { return a + t * (b - a); }
inline v2 lerp_v2(v2 a, v2 b, float t) 
{
	v2 result;
	result.x = lerp(a.x, b.x, t);
	result.y = lerp(a.y, b.y, t);
	return result;
}
inline v3 lerp_v3(v3 a, v3 b, float t) 
{
	v3 result;
	result.x = lerp(a.x, b.x, t);
	result.y = lerp(a.y, b.y, t);
	result.z = lerp(a.z, b.z, t);
	return result;
}
inline v4 lerp_v4(v4 a, v4 b, float t) 
{
	v4 result;
	result.x = lerp(a.x, b.x, t);
	result.y = lerp(a.y, b.y, t);
	result.z = lerp(a.z, b.z, t);
	result.w = lerp(a.w, b.w, t);
	return result;
}

inline v4 slerp_v4(v4 previousQuat, v4 nextQuat, double interpolationValue) {
	double dotProduct = dot(previousQuat, nextQuat);

	//make sure we take the shortest path in case dot Product is negative
	if (dotProduct < 0.0) {
		nextQuat = -nextQuat ;
		dotProduct = -dotProduct;
	}

	//if the two quaternions are too close to each other, just linear interpolate between the 4D vector
	if (dotProduct > 0.9995) {
		v4 result = previousQuat + interpolationValue * (nextQuat - previousQuat);
		return result.normalize();
	}

	//perform the spherical linear interpolation
	double theta_0 = acos(dotProduct);
	double theta = interpolationValue * theta_0;
	double sin_theta = sin(theta);
	double sin_theta_0 = sin(theta_0);

	double scalePreviousQuat = cos(theta) - dotProduct * sin_theta / sin_theta_0;
	double scaleNextQuat = sin_theta / sin_theta_0;

	v4 result = scalePreviousQuat * previousQuat + scaleNextQuat * nextQuat;

	return result.normalize();
}

#define clamp(input, min, max)			((input) < (min) ? (min) : ((input) > (max) ? (max) : (input)))
#define clamp_circular(input, min, max) ((input) < (min) ? (max) : ((input) > (max) ? (min) : (input)))


struct RandState
{
	uint64_t seed;
	bool initialized = false;
};
RandState RANDSTATE;

uint64_t rand_XOR()
{
	if (!RANDSTATE.initialized)
	{
		RANDSTATE.seed = time(NULL);
		RANDSTATE.initialized = true;
	}
	uint64_t x = RANDSTATE.seed;
	x ^= x << 9;
	x ^= x >> 5;
	x ^= x << 15;
	return RANDSTATE.seed = x;
}

int *rand_array(int count)
{
	int* ptr = (int*)malloc(count * sizeof(int));
	srand(time(NULL));
	for (int i = 0; i < count; i++) ptr[i] = rand();
	return ptr;
}
int           rand_range(int min, int max)          { return min + rand_XOR() % (max - min + 1); } // Returns a random real in [min,max].
inline double random_double()                       { return rand() / (RAND_MAX + 1.0);                  } // Returns a random real in [0,1].
inline double random_double(double min, double max) { return min + (max - min) * random_double();        } // Returns a random real in [min,max].

float v2_distance_2Points(v2 A, v2 B)
{
	return sqrt((A.x - B.x) * (A.x - B.x) + (A.y - B.y) * (A.y - B.y));
}

v2 unitvec_AtoB(v2 A, v2 B)
{
	float n = v2_distance_2Points(A, B);
	return ((B - A) / n);
}

float signed_angle_v2(v2 A, v2 B)
{
	return atan2(A.x * B.y - A.y * B.x, A.x * B.x + A.y * B.y);
}

v2 Rotate2D(v2 P, float sine, float cosine)
{
	return v2(v2(cosine, -sine).dot(P), v2(sine, cosine).dot(P));
}

v2 Rotate2D(v2 P, float Angle)
{
	return Rotate2D(P, sin(Angle), cos(Angle));
}
v2 Rotate2D(v2 p, v2 o, float angle)
{
	// Demonstration: https://www.desmos.com/calculator/8aaegifsba
	float s = sin(angle);
	float c = cos(angle);

	float x = (p.x - o.x) * c - (p.y - o.y) * s + o.x;
	float y = (p.x - o.x) * s + (p.y - o.y) * c + o.y;

	return v2(x, y);
}

v2 Reflection2D(v2 P, v2 N)
{
	return P - 2 * N.dot(P) * N;
}

bool PointInRectangle(v2 P, v2 A, v2 B, v2 C)
{
	v2 M = P;
	v2 AB = B - A;
	v2 BC = C - B;
	v2 AM = M - A;
	v2 BM = M - B;
	if (0 <= AB.dot(AM) && AB.dot(AM) <= AB.dot(AB) &&
		0 <= BC.dot(BM) && BC.dot(BM) <= BC.dot(BC))
	return true;
	else
	return false;
}

static float abso(float F) { return F > 0 ? F : -F; };

uint32_t hash_djb2(char *str)
{
	uint32_t hash = 5381;
	int c;
	while (c = *str++)
	hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
	return hash;
}

struct m4 
{ 
	union
	{
		float e[16];
		float m[4][4];
		v4 columns[4];
	};

	m4(): e{1 , 0 , 0 , 0,
	0 , 1 , 0 , 0,
	0 , 0 , 1 , 0,
	0 , 0 , 0 , 1} {}

	m4(float E[16]) : e{E[16]} {}

	m4(float e0  ,float e1  ,float e2  ,float  e3 ,
	float e4  ,float e5  ,float e6  ,float  e7 ,
	float e8  ,float e9  ,float e10 ,float  e11,
	float e12 ,float e13 ,float e14 ,float  e15)  
	: e{e0  ,e1  ,e2  ,e3 ,
	e4  ,e5  ,e6  ,e7 ,
	e8  ,e9  ,e10 ,e11,
	e12 ,e13 ,e14 ,e15} {}
};

m4 m4_ident() {
	return m4(1 , 0 , 0 , 0,
			0 , 1 , 0 , 0,
			0 , 0 , 1 , 0,
			0 , 0 , 0 , 1);
}

v4 linear_combine_v4m4(v4 L, m4 R)
{
	v4 Result;

	Result.x =  L.e[0] * R.columns[0].x;
	Result.y =  L.e[0] * R.columns[0].y;
	Result.z =  L.e[0] * R.columns[0].z;
	Result.w =  L.e[0] * R.columns[0].w;

	Result.x += L.e[1] * R.columns[1].x;
	Result.y += L.e[1] * R.columns[1].y;
	Result.z += L.e[1] * R.columns[1].z;
	Result.w += L.e[1] * R.columns[1].w;

	Result.x += L.e[2] * R.columns[2].x;
	Result.y += L.e[2] * R.columns[2].y;
	Result.z += L.e[2] * R.columns[2].z;
	Result.w += L.e[2] * R.columns[2].w;

	Result.x += L.e[3] * R.columns[3].x;
	Result.y += L.e[3] * R.columns[3].y;
	Result.z += L.e[3] * R.columns[3].z;
	Result.w += L.e[3] * R.columns[3].w;

	return Result;
}

m4 operator*(const m4& L, const m4& R)
{
	m4 Result;

	Result.columns[0] = linear_combine_v4m4(R.columns[0], L);
	Result.columns[1] = linear_combine_v4m4(R.columns[1], L);
	Result.columns[2] = linear_combine_v4m4(R.columns[2], L);
	Result.columns[3] = linear_combine_v4m4(R.columns[3], L);

	return Result;

}

m4 m4_transpose(m4 input)
{
	m4 res = input;
	res.m[0][1] = input.m[1][0];
	res.m[0][2] = input.m[2][0];
	res.m[0][3] = input.m[3][0];

	res.m[1][0] = input.m[0][1];
	res.m[1][2] = input.m[2][1];
	res.m[1][3] = input.m[3][1];

	res.m[2][0] = input.m[0][2];
	res.m[2][1] = input.m[1][2];
	res.m[2][3] = input.m[3][2];

	res.m[3][0] = input.m[0][3];
	res.m[3][1] = input.m[1][3];
	res.m[3][2] = input.m[2][3];

	return res;
}

m4 m4_scale(v3 S)
{
	float M[4][4] = {0};
	M[0][0] = S.x;
	M[1][1] = S.y;
	M[2][2] = S.z;
	M[3][3] = 1.0;

	return *(m4*)M;
}

m4 m4_translation(v3 T)
{
	float M[4][4] = {0};
	M[3][0] = T.x;
	M[3][1] = T.y;
	M[3][2] = T.z;
	M[3][3] = 1;

	M[0][0] = 1.0f;
	M[1][1] = 1.0f;
	M[2][2] = 1.0f;

	return *(m4*)M;
}

m4 quaternion_to_m4(v4 Q)
{
	float S = 1.0f / Q.length_squared();
	float M[16];
	M[0] = 1.0f - 2.0f * S * (Q.y * Q.y + Q.z * Q.z);
	M[1] = 2.0f * S * (Q.x * Q.y + Q.w * Q.z);
	M[2] = 2.0f * S * (Q.x * Q.z - Q.w * Q.y);
	M[3] = 0.0f;

	M[4] = 2.0f * S * (Q.x * Q.y - Q.w * Q.z);
	M[5] = 1.0f - 2.0f * S * (Q.x * Q.x + Q.z * Q.z);
	M[6] = 2.0f * S * (Q.y * Q.z + Q.w * Q.x);
	M[7] = 0.0f;

	M[8] = 2.0f * S * (Q.x * Q.z + Q.w * Q.y);
	M[9] = 2.0f * S * (Q.y * Q.z - Q.w * Q.x);
	M[10] = 1.0f - 2.0f * S * (Q.x * Q.x + Q.y * Q.y);
	M[11] = 0.0f;

	M[12] = 0.0f;
	M[13] = 0.0f;
	M[14] = 0.0f;
	M[15] = 1.0f;

	return *(m4*)M;
}

inline m4 operator*  (m4 a, float b ) { for (int i = 0; i < 16; i++) a.e[i] *= b; return a; }

m4 m4_rotate_spherical(float theta, float phi) {
	m4 theta_matrix = m4(
		1,  0,				0, 				0,
		0,  cosf(theta),	-sinf(theta), 	0,
		0,  sinf(theta), 	cosf(theta), 	0,
		0,  0,				0, 				1
	);

	m4 phi_matrix = m4(
		cosf(phi), -sinf(phi),  0, 0,
		sinf(phi),  cosf(phi),  0, 0,
		0,			0,  		1, 0,
		0,			0,  		0, 1
	);

	return theta_matrix * phi_matrix;
}

m4 m4_no_translation(m4 in) {
	m4 out = in;
	out.e[12] = 0.0f;
	out.e[13] = 0.0f;
	out.e[14] = 0.0f;
	return out;
}

v3 m4_mul_v3(m4 m, v3 v, float w) {
	v3 r;
	r.x = m.e[0] * v.x + m.e[4] * v.y + m.e[ 8] * v.z + m.e[12] * w;
	r.y = m.e[1] * v.x + m.e[5] * v.y + m.e[ 9] * v.z + m.e[13] * w;
	r.z = m.e[2] * v.x + m.e[6] * v.y + m.e[10] * v.z + m.e[14] * w;
	return r;
}

v4 m4_mul_v4(m4 m, v4 v) {
	v4 r;
	r.x = m.e[0] * v.x + m.e[4] * v.y + m.e[ 8] * v.z + m.e[12] * v.w;
	r.y = m.e[1] * v.x + m.e[5] * v.y + m.e[ 9] * v.z + m.e[13] * v.w;
	r.z = m.e[2] * v.x + m.e[6] * v.y + m.e[10] * v.z + m.e[14] * v.w;
	r.w = m.e[3] * v.x + m.e[7] * v.y + m.e[11] * v.z + m.e[15] * v.w;
	return r;
}

v3 cross(v3 a, v3 b) {
	v3 r;
	r.x = a.y * b.z - a.z * b.y;
	r.y = a.z * b.x - a.x * b.z;
	r.z = a.x * b.y - a.y * b.x;
	return r;
}

v3 v3_spherical_to_cartesian(v3 p, float theta, float phi) {
	float x = p.x * sin(phi) * cos(theta);
	float y = p.y * cos(phi);
	float z = p.z * sin(phi) * sin(theta);
	return v3(x, y, z);
}

float logarithmic_scale(float x) {
	return (powf(10.0f, x) - 1.0f) / 9.0f;
}

float logarithmic_scale_inv(float x) {
	return log10f(x * 9.0f + 1.0f);
}
