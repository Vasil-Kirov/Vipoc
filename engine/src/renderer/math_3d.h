#pragma once

#include <math.h>
#include "platform/platform.h"
#include "renderer/load_gl_black_magic.h"


#pragma pack(push, 1)
typedef struct v2
{
	GLfloat x;
	GLfloat y;
} v2;
typedef struct v3
{
	GLfloat x;
	GLfloat y;
	GLfloat z;
} v3;
typedef struct v4
{
	GLfloat x;
	GLfloat y;
	GLfloat z;
	GLfloat w;
} v4;
typedef struct m2
{
	f32 x1;
	f32 y1;
	f32 x2;
	f32 y2;
} m2;
typedef struct m4
{
	f32 m[4][4];
} m4;
typedef struct vertex
{
	v4 position;
	v4 texture;
	v4 color;
} vertex;
#pragma pack(pop)

typedef struct quaternion
{
	f32 x;
	f32 y;
	f32 z;
	f32 w;
} quaternion;

typedef struct camera
{
	v3 position;
	v3 look_dir;
	float yaw;
	float pitch;
	double mouse_x;
	double mouse_y;
} camera;

inline void
set_shader_uniform_mat4(char *str, m4 mat);

inline double
DegToRad(f32 degrees)
{
	return (double)(degrees * (PI / 180));
}
inline double
RadToDeg(f32 Rad)
{
	return (double)(Rad * (180.0f/PI));
}

inline m4
x_rotation(f32 Angle)
{
	f32 c = cos(Angle);
	f32 s = sin(Angle);
	m4 R =
	{
		{{1, 0, 0, 0},
			{0, c,-s, 0},
			{0, s, c, 0},
			{0, 0, 0, 1}}
	};
	return R;
}

inline m4
y_rotation(f32 Angle)
{
	f32 c = cos(Angle);
	f32 s = sin(Angle);
	m4 R =
	{
		{{ c, 0, s, 0},
			{ 0, 1, 0, 0},
			{-s, 0, c, 0},
			{ 0, 0, 0, 1}}
	};
	return R;
}

inline m4
z_rotation(f32 Angle)
{
	f32 c = cos(Angle);
	f32 s = sin(Angle);
	m4 R =
	{
		{{c,-s, 0, 0},
			{s, c, 0, 0},
			{0, 0, 1, 0},
			{0, 0, 0, 1}}
	};
	return R;
}

inline m4
transpose(m4 A)
{
	m4 R = {};
	for (int j = 0; j <= 3; ++j) 
	{
		for(int i = 0; i <= 3; ++i)
		{
			R.m[j][i] = A.m[i][j];
		}
	}
	return R;
}

inline m4
projection(f32 WidthOverHeight, f32 FOV)
{
	f32 Near = 1.0f;
	f32 Far = 100.0f;
	
	f32 f = 1.0f/(f32)tan(DegToRad(FOV / 2.0f));
	f32 fn = 1.0f / (Near - Far);
	
	
	f32 a = f / WidthOverHeight;
	f32 b = f;
	f32 c = Far * fn;
	f32 d = Near * Far * fn;
	
	m4 Result = 
	{
		{{a, 0, 0, 0},
			{0, b, 0, 0},
			{0, 0, c, -1},
			{0, 0, d, 0}}
	};
	return Result;
}

inline m4
mat4_multiply(m4 A, m4 B)
{
	m4 Ret;
	
	for (unsigned int i = 0; i < 4; i++) {
		for (unsigned int j = 0; j < 4; j++) {
			Ret.m[i][j] = A.m[i][0] * B.m[0][j] +
				A.m[i][1] * B.m[1][j] +
				A.m[i][2] * B.m[2][j] +
				A.m[i][3] * B.m[3][j];
		}
	}
	
	return Ret;
}

inline m4
mat4_multiply_multiple(int number, m4 A, m4 B, ...)
{
	va_list args;
	va_start(args, B);
	
	m4 Result = mat4_multiply(A, B);
	if(number <= 2)
	{
		return Result;
	}
	for(int i = 2; i < number; ++i)
	{
		m4 Right = va_arg(args, m4);
		Result = mat4_multiply(Result, Right);
	}
	return Result;
}

/* REQUIRES TRANSPOSED MATRIX!!! */
inline v3
mat4_v3_multiply(m4 A, v3 B)
{
	v3 Ret;
	float w = 1.0f;
	
	Ret.x = B.x * A.m[0][0] + B.y * A.m[1][0] + B.z * A.m[2][0] + w * A.m[3][0];
	Ret.y = B.x * A.m[0][1] + B.y * A.m[1][1] + B.z * A.m[2][1] + w * A.m[3][1];
	Ret.z = B.x * A.m[0][2] + B.y * A.m[1][2] + B.z * A.m[2][2] + w * A.m[3][2];
	
	return Ret;
}

inline m4
create_mat4(
			float num1, float num2, float num3, float num4, 
			float num5, float num6, float num7, float num8, 
			float num9, float num10, float num11, float num12, 
			float num13, float num14, float num15, float num16)
{
	m4 matrix = {};
	matrix.m[0][0] = num1;		matrix.m[0][1] = num2;		matrix.m[0][2] = num3;		matrix.m[0][3] = num4;
	matrix.m[1][0] = num5;		matrix.m[1][1] = num6;		matrix.m[1][2] = num7;		matrix.m[1][3] = num8;
	matrix.m[2][0] = num9;		matrix.m[2][1] = num10;		matrix.m[2][2] = num11;		matrix.m[2][3] = num12;
	matrix.m[3][0] = num13;		matrix.m[3][1] = num14;		matrix.m[3][2] = num15;		matrix.m[3][3] = num16;
	return matrix;
}

inline m4
identity()
{
	m4 Result = {};
	Result = create_mat4(
						 1.0f, 0.0f, 0.0f, 0.0f,
						 0.0f, 1.0f, 0.0f, 0.0f,
						 0.0f, 0.0f, 1.0f, 0.0f,
						 0.0f, 0.0f, 0.0f, 1.0f);
	return Result;
}

inline f32
normalize_between(f32 x, f32 minx, f32 maxx, f32 a, f32 b)
{
	f32 Result = ( (b - a) * ( (x-minx) / (maxx-minx) ) + a );
	return Result;
}

inline v3
v3_cross(v3 A, v3 B)
{
	v3 R = {};
	R.x = A.y * B.z - A.z * B.y;
	R.y = A.z * B.x - A.x * B.z;
	R.z = A.x * B.y - A.y * B.x;
	return R;
}


inline f32
v3_get_length(v3 A)
{
	return(sqrtf(A.x * A.x + A.y * A.y + A.z * A.z));
}

inline v3
v3_normalize(v3 A)
{
	f32 length = v3_get_length(A);
	
	A.x /= length;
	A.y /= length;
	A.z /= length;
	
	return A;
}


inline v3
v3_add(v3 A, v3 B)
{
	v3 R = {};
	R.x = A.x + B.x;
	R.y = A.y + B.y;
	R.z = A.z + B.z;
	return R;
}


inline v3
v3_sub(v3 A, v3 B)
{
	v3 R = {};
	R.x = A.x - B.x;
	R.y = A.y - B.y;
	R.z = A.z - B.z;
	return R;
}

inline v3
v3_scale(v3 A, f32 B)
{
	v3 R = {};
	R.x = A.x * B;
	R.y = A.y * B;
	R.z = A.z * B;
	return R;
}


inline f32
v3_dot(v3 A, v3 B)
{
	return (A.x * B.x + A.y * B.y + A.z * B.z);
}

inline v3 
v3_negate(v3 A)
{
	return (v3){-A.x, -A.y, -A.z};
}

inline m4
point_at(v3 pos, v3 target, v3 up)
{
	v3 new_forward = v3_sub(target, pos);
	new_forward = v3_normalize(new_forward);

	v3 a = v3_scale(new_forward, v3_dot(up, new_forward));
	v3 new_up = v3_sub(up, a);
	new_up = v3_normalize(new_up);

	v3 new_right = v3_cross(new_up, new_forward);

	m4 matrix = create_mat4(
		new_right.x, 	new_right.y, 	new_right.z, 	0.0f,
		new_up.x,		new_up.y,		new_up.z,		0.0f,
		new_forward.x,	new_forward.y,	new_forward.z,	0.0f,
		pos.x,			pos.y,			pos.z,			0.0f
	);
	return matrix;
}

/* I have no idea what this does so it' just copied */
inline m4
quick_inverse(m4 m)
{
	m4 matrix;
	matrix.m[0][0] = m.m[0][0]; matrix.m[0][1] = m.m[1][0]; matrix.m[0][2] = m.m[2][0]; matrix.m[0][3] = 0.0f;
	matrix.m[1][0] = m.m[0][1]; matrix.m[1][1] = m.m[1][1]; matrix.m[1][2] = m.m[2][1]; matrix.m[1][3] = 0.0f;
	matrix.m[2][0] = m.m[0][2]; matrix.m[2][1] = m.m[1][2]; matrix.m[2][2] = m.m[2][2]; matrix.m[2][3] = 0.0f;
	matrix.m[3][0] = -(m.m[3][0] * matrix.m[0][0] + m.m[3][1] * matrix.m[1][0] + m.m[3][2] * matrix.m[2][0]);
	matrix.m[3][1] = -(m.m[3][0] * matrix.m[0][1] + m.m[3][1] * matrix.m[1][1] + m.m[3][2] * matrix.m[2][1]);
	matrix.m[3][2] = -(m.m[3][0] * matrix.m[0][2] + m.m[3][1] * matrix.m[1][2] + m.m[3][2] * matrix.m[2][2]);
	matrix.m[3][3] = 1.0f;
	return matrix;
}