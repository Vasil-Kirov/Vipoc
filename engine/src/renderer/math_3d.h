#pragma once

#include <math.h>
#include "renderer/load_gl_black_magic.h"

typedef float f32;

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

inline void
set_shader_uniform_mat4(char *str, m4 mat);

inline double
DegToRad(f32 degrees)
{
	return (double)(degrees * (PI / 180));
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
//typedef __declspec(align(32)) v4 m4[4];