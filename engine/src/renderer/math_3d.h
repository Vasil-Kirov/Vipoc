#pragma once

#if defined(__cplusplus)
extern "C"
{
#endif

#include <immintrin.h>
#include <math.h>
#include "platform/platform.h"
#include <stdarg.h>

#pragma pack(push, 1)
	typedef struct _v2
	{
		f32 x;
		f32 y;
	} v2;
	typedef struct _v3
	{
		f32 x;
		f32 y;
		f32 z;
	} v3;
	typedef struct _v4
	{
		f32 x;
		f32 y;
		f32 z;
		f32 w;
	} v4;
	typedef struct _m2
	{
		f32 x1;
		f32 y1;
		f32 x2;
		f32 y2;
	} m2;
	typedef struct _m3
	{
		f32 m[6];
	} m3;
	typedef struct _m4
	{
		f32 m[4][4];
	} m4;
	typedef struct vertex
	{
		v4 position;
		v4 texture;
		v4 color;
		v4 normal;
		v3 world_pos;
		float is_affected_by_light;
	} vertex;
	typedef struct mesh_vertex
	{
		v3 position;
		v3 texture;
		v3 normal;
	} mesh_vertex;
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
		bool32 is_locked;
	} camera;

	typedef struct vp_light
	{
		v3 position;

		v3 ambient;
		v3 diffuse;
		v3 specular;
	} vp_light;

	typedef __m128 simd_v4;

	inline double
	DegToRad(f32 degrees)
	{
		return (double)(degrees * (PI / 180));
	}
	inline double
	RadToDeg(f32 Rad)
	{
		return (double)(Rad * (180.0f / PI));
	}

	inline m4
	x_rotation(f32 Angle)
	{
		f32 c = cos(Angle);
		f32 s = sin(Angle);
		m4 R =
			{
				{{1, 0, 0, 0},
				 {0, c, -s, 0},
				 {0, s, c, 0},
				 {0, 0, 0, 1}}};
		return R;
	}

	inline m4
	y_rotation(f32 Angle)
	{
		f32 c = cos(Angle);
		f32 s = sin(Angle);
		m4 R =
			{
				{{c, 0, s, 0},
				 {0, 1, 0, 0},
				 {-s, 0, c, 0},
				 {0, 0, 0, 1}}};
		return R;
	}

	inline m4
	z_rotation(f32 Angle)
	{
		f32 c = cosf(Angle);
		f32 s = sinf(Angle);
		m4 R =
			{
				{{c, -s, 0, 0},
				 {s, c, 0, 0},
				 {0, 0, 1, 0},
				 {0, 0, 0, 1}}};
		return R;
	}

	inline m4
	transpose(m4 A)
	{
		m4 R = {};
		for (int j = 0; j <= 3; ++j)
		{
			for (int i = 0; i <= 3; ++i)
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

		f32 f = 1.0f / tanf(DegToRad(FOV / 2.0f));
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
				 {0, 0, d, 0}}};
		return Result;
	}

	inline m4
	mat4_multiply(m4 A, m4 B)
	{
		m4 Ret;

		for (unsigned int i = 0; i < 4; i++)
		{
			for (unsigned int j = 0; j < 4; j++)
			{
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
		if (number <= 2)
		{
			return Result;
		}
		for (int i = 2; i < number; ++i)
		{
			m4 Right = va_arg(args, m4);
			Result = mat4_multiply(Result, Right);
		}
		return Result;
	}

	// NOTE(Vasko): I wrote this comment bellow a couple of months ago and I don't know what it means
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

	inline f32
	normalize_between(f32 x, f32 minx, f32 maxx, f32 a, f32 b)
	{
		f32 Result = ((b - a) * ((x - minx) / (maxx - minx)) + a);
		return Result;
	}

	inline float
	normalize_coordinate(float x, float maxx, float minx)
	{
		float Result;
		Result = (2.0f * ((x - minx) / (maxx - minx))) - 1.0f;
		return Result;
	}

	inline float
	normalize_tex_coordinate(float x, float maxx, float minx)
	{
		float Result;
		Result = ((x - minx) / (maxx - minx));
		return Result;
	}

	inline v3
	normalize_v3(v3 target, float minx, float maxx, float from, float to)
	{
		v3 result = {};
		result.x = normalize_between(target.x, minx, maxx, from, to);
		result.y = normalize_between(target.y, minx, maxx, from, to);
		result.z = normalize_between(target.z, minx, maxx, from, to);
		return result;
	}

	inline v4
	normalize_v4(v4 target, float minx, float maxx, float from, float to)
	{
		v4 result = {};
		result.x = normalize_between(target.x, minx, maxx, from, to);
		result.y = normalize_between(target.y, minx, maxx, from, to);
		result.z = normalize_between(target.z, minx, maxx, from, to);
		result.w = normalize_between(target.w, minx, maxx, from, to);
		return result;
	}

	inline v4
	mat4_v4_multiply(m4 A, v4 B)
	{
		v4 Ret;

		Ret.x = B.x * A.m[0][0] + B.y * A.m[1][0] + B.z * A.m[2][0] + B.w * A.m[3][0];
		Ret.y = B.x * A.m[0][1] + B.y * A.m[1][1] + B.z * A.m[2][1] + B.w * A.m[3][1];
		Ret.z = B.x * A.m[0][2] + B.y * A.m[1][2] + B.z * A.m[2][2] + B.w * A.m[3][2];
		Ret.w = B.x * A.m[0][3] + B.y * A.m[1][3] + B.z * A.m[2][3] + B.w * A.m[3][3];

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
		matrix.m[0][0] = num1;
		matrix.m[0][1] = num2;
		matrix.m[0][2] = num3;
		matrix.m[0][3] = num4;
		matrix.m[1][0] = num5;
		matrix.m[1][1] = num6;
		matrix.m[1][2] = num7;
		matrix.m[1][3] = num8;
		matrix.m[2][0] = num9;
		matrix.m[2][1] = num10;
		matrix.m[2][2] = num11;
		matrix.m[2][3] = num12;
		matrix.m[3][0] = num13;
		matrix.m[3][1] = num14;
		matrix.m[3][2] = num15;
		matrix.m[3][3] = num16;
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

	inline f64
	double_normalize_between(f64 x, f64 minx, f64 maxx, f64 a, f64 b)
	{
		f64 Result = ((b - a) * ((x - minx) / (maxx - minx)) + a);
		return Result;
	}

	inline float
	get_distance_3d(v3 pa, v3 pb)
	{
		f32 maxx, minx;
		if (pa.x > pb.x)
		{
			maxx = pa.x;
			minx = pb.x;
		}
		else
		{
			maxx = pb.x;
			minx = pa.x;
		}

		f32 maxy, miny;
		if (pa.y > pb.y)
		{
			maxy = pa.y;
			miny = pb.y;
		}
		else
		{
			maxy = pb.y;
			miny = pa.y;
		}

		f32 a = maxx - minx;
		f32 b = maxy - miny;
		f32 c = sqrtf(a * a + b * b);

		return c;
	}

	inline bool32
	v3_check_equality(v3 A, v3 B)
	{
		return (A.x == B.x && A.y == B.y && A.z == B.z);
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
		return (sqrtf(A.x * A.x + A.y * A.y + A.z * A.z));
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

	inline v3
	v3_f32_add(v3 A, f32 B)
	{
		v3 R = {};
		R.x = A.x + B;
		R.y = A.y + B;
		R.z = A.z + B;
		return R;
	}

	inline v3
	v3_negate(v3 A)
	{
		return (v3){-A.x, -A.y, -A.z};
	}

	inline v3
	v3_v3_scale(v3 A, v3 B)
	{
		return (v3){A.x * B.x, A.y * B.y, A.z * B.z};
	}

	inline f32
	v3_dot(v3 A, v3 B)
	{
		return (A.x * B.x + A.y * B.y + A.z * B.z);
	}

	inline simd_v4
	v4_dot(simd_v4 A, simd_v4 B)
	{
		simd_v4 const mul0 = _mm_mul_ps(A, B);
		simd_v4 const swp0 = _mm_shuffle_ps(mul0, mul0, _MM_SHUFFLE(2, 3, 0, 1));
		simd_v4 const add0 = _mm_add_ps(mul0, swp0);
		simd_v4 const swp1 = _mm_shuffle_ps(add0, add0, _MM_SHUFFLE(0, 1, 2, 3));
		simd_v4 const add1 = _mm_add_ps(add0, swp1);

		return add1;
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
			new_right.x, new_right.y, new_right.z, 0.0f,
			new_up.x, new_up.y, new_up.z, 0.0f,
			new_forward.x, new_forward.y, new_forward.z, 0.0f,
			pos.x, pos.y, pos.z, 0.0f);
		return matrix;
	}

	inline m4
	translate(v3 position)
	{
		return create_mat4(
			1.0f, 0.0f, 0.0f, position.x,
			0.0f, 1.0f, 0.0f, position.y,
			0.0f, 0.0f, 1.0f, position.z,
			0.0f, 0.0f, 0.0f, 1.0f);
	}

	/* I have no idea what this does so it' just copied */
	inline m4
	quick_inverse(m4 m)
	{
		m4 matrix;
		matrix.m[0][0] = m.m[0][0];
		matrix.m[0][1] = m.m[1][0];
		matrix.m[0][2] = m.m[2][0];
		matrix.m[0][3] = 0.0f;
		matrix.m[1][0] = m.m[0][1];
		matrix.m[1][1] = m.m[1][1];
		matrix.m[1][2] = m.m[2][1];
		matrix.m[1][3] = 0.0f;
		matrix.m[2][0] = m.m[0][2];
		matrix.m[2][1] = m.m[1][2];
		matrix.m[2][2] = m.m[2][2];
		matrix.m[2][3] = 0.0f;
		matrix.m[3][0] = -(m.m[3][0] * matrix.m[0][0] + m.m[3][1] * matrix.m[1][0] + m.m[3][2] * matrix.m[2][0]);
		matrix.m[3][1] = -(m.m[3][0] * matrix.m[0][1] + m.m[3][1] * matrix.m[1][1] + m.m[3][2] * matrix.m[2][1]);
		matrix.m[3][2] = -(m.m[3][0] * matrix.m[0][2] + m.m[3][1] * matrix.m[1][2] + m.m[3][2] * matrix.m[2][2]);
		matrix.m[3][3] = 1.0f;
		return matrix;
	}

	/* Calculates the view matrix and gives it back in column-major order */
	inline m4
	calculate_view_matrix(camera *cm)
	{
		v3 up = {0.0f, 1.0f, 0.0f};
		v3 target = {0.0f, 0.0f, 1.0f};
		m4 camera_y_rotation = y_rotation(cm->yaw);
		m4 camera_x_rotation = x_rotation(cm->pitch);
		m4 camera_rotation = mat4_multiply(camera_x_rotation, camera_y_rotation);
		cm->look_dir = mat4_v3_multiply(camera_rotation, target);
		target = v3_add(cm->position, cm->look_dir);

		m4 camera = point_at(cm->position, target, up);
		return transpose(quick_inverse(camera));
	}

	inline v4
	create_v4(f32 a, f32 b, f32 c, f32 d)
	{
		v4 result = {a, b, c, d};
		return result;
	}

	// NOTE(Vasko): row counted from 0
	inline void
	add_row_from_v4(m4 *matrix, v4 vector, int row)
	{
		matrix->m[row][0] = vector.x;
		matrix->m[row][1] = vector.y;
		matrix->m[row][2] = vector.z;
		matrix->m[row][3] = vector.w;
	}
	inline void
	add_row_from_f32_arr(m4 *matrix, f32 *vector, int row)
	{
		matrix->m[row][0] = vector[0];
		matrix->m[row][1] = vector[1];
		matrix->m[row][2] = vector[2];
		matrix->m[row][3] = vector[3];
	}

	// NOTE(Vasko): Stolen from glm
	inline m4
	good_inverse_m4(m4 input)
	{
		v4 in_tmp[4] = {};
		in_tmp[0] = create_v4(input.m[0][0], input.m[0][1], input.m[0][2], input.m[0][3]);
		in_tmp[1] = create_v4(input.m[1][0], input.m[1][1], input.m[1][2], input.m[1][3]);
		in_tmp[2] = create_v4(input.m[2][0], input.m[2][1], input.m[2][2], input.m[2][3]);
		in_tmp[3] = create_v4(input.m[3][0], input.m[3][1], input.m[3][2], input.m[3][3]);

		simd_v4 in[4] = {_mm_loadu_ps((f32 *)&(in_tmp[0])), _mm_loadu_ps((f32 *)&(in_tmp[1])),
						 _mm_loadu_ps((f32 *)&(in_tmp[2])), _mm_loadu_ps((f32 *)&(in_tmp[3]))};

		__m128 Fac0;
		{
			//	valType SubFactor00 = m[2][2] * m[3][3] - m[3][2] * m[2][3];
			//	valType SubFactor00 = m[2][2] * m[3][3] - m[3][2] * m[2][3];
			//	valType SubFactor06 = m[1][2] * m[3][3] - m[3][2] * m[1][3];
			//	valType SubFactor13 = m[1][2] * m[2][3] - m[2][2] * m[1][3];

			__m128 Swp0a = _mm_shuffle_ps(in[3], in[2], _MM_SHUFFLE(3, 3, 3, 3));
			__m128 Swp0b = _mm_shuffle_ps(in[3], in[2], _MM_SHUFFLE(2, 2, 2, 2));

			__m128 Swp00 = _mm_shuffle_ps(in[2], in[1], _MM_SHUFFLE(2, 2, 2, 2));
			__m128 Swp01 = _mm_shuffle_ps(Swp0a, Swp0a, _MM_SHUFFLE(2, 0, 0, 0));
			__m128 Swp02 = _mm_shuffle_ps(Swp0b, Swp0b, _MM_SHUFFLE(2, 0, 0, 0));
			__m128 Swp03 = _mm_shuffle_ps(in[2], in[1], _MM_SHUFFLE(3, 3, 3, 3));

			__m128 Mul00 = _mm_mul_ps(Swp00, Swp01);
			__m128 Mul01 = _mm_mul_ps(Swp02, Swp03);
			Fac0 = _mm_sub_ps(Mul00, Mul01);
		}

		__m128 Fac1;
		{
			//	valType SubFactor01 = m[2][1] * m[3][3] - m[3][1] * m[2][3];
			//	valType SubFactor01 = m[2][1] * m[3][3] - m[3][1] * m[2][3];
			//	valType SubFactor07 = m[1][1] * m[3][3] - m[3][1] * m[1][3];
			//	valType SubFactor14 = m[1][1] * m[2][3] - m[2][1] * m[1][3];

			__m128 Swp0a = _mm_shuffle_ps(in[3], in[2], _MM_SHUFFLE(3, 3, 3, 3));
			__m128 Swp0b = _mm_shuffle_ps(in[3], in[2], _MM_SHUFFLE(1, 1, 1, 1));

			__m128 Swp00 = _mm_shuffle_ps(in[2], in[1], _MM_SHUFFLE(1, 1, 1, 1));
			__m128 Swp01 = _mm_shuffle_ps(Swp0a, Swp0a, _MM_SHUFFLE(2, 0, 0, 0));
			__m128 Swp02 = _mm_shuffle_ps(Swp0b, Swp0b, _MM_SHUFFLE(2, 0, 0, 0));
			__m128 Swp03 = _mm_shuffle_ps(in[2], in[1], _MM_SHUFFLE(3, 3, 3, 3));

			__m128 Mul00 = _mm_mul_ps(Swp00, Swp01);
			__m128 Mul01 = _mm_mul_ps(Swp02, Swp03);
			Fac1 = _mm_sub_ps(Mul00, Mul01);
		}

		__m128 Fac2;
		{
			//	valType SubFactor02 = m[2][1] * m[3][2] - m[3][1] * m[2][2];
			//	valType SubFactor02 = m[2][1] * m[3][2] - m[3][1] * m[2][2];
			//	valType SubFactor08 = m[1][1] * m[3][2] - m[3][1] * m[1][2];
			//	valType SubFactor15 = m[1][1] * m[2][2] - m[2][1] * m[1][2];

			__m128 Swp0a = _mm_shuffle_ps(in[3], in[2], _MM_SHUFFLE(2, 2, 2, 2));
			__m128 Swp0b = _mm_shuffle_ps(in[3], in[2], _MM_SHUFFLE(1, 1, 1, 1));

			__m128 Swp00 = _mm_shuffle_ps(in[2], in[1], _MM_SHUFFLE(1, 1, 1, 1));
			__m128 Swp01 = _mm_shuffle_ps(Swp0a, Swp0a, _MM_SHUFFLE(2, 0, 0, 0));
			__m128 Swp02 = _mm_shuffle_ps(Swp0b, Swp0b, _MM_SHUFFLE(2, 0, 0, 0));
			__m128 Swp03 = _mm_shuffle_ps(in[2], in[1], _MM_SHUFFLE(2, 2, 2, 2));

			__m128 Mul00 = _mm_mul_ps(Swp00, Swp01);
			__m128 Mul01 = _mm_mul_ps(Swp02, Swp03);
			Fac2 = _mm_sub_ps(Mul00, Mul01);
		}

		__m128 Fac3;
		{
			//	valType SubFactor03 = m[2][0] * m[3][3] - m[3][0] * m[2][3];
			//	valType SubFactor03 = m[2][0] * m[3][3] - m[3][0] * m[2][3];
			//	valType SubFactor09 = m[1][0] * m[3][3] - m[3][0] * m[1][3];
			//	valType SubFactor16 = m[1][0] * m[2][3] - m[2][0] * m[1][3];

			__m128 Swp0a = _mm_shuffle_ps(in[3], in[2], _MM_SHUFFLE(3, 3, 3, 3));
			__m128 Swp0b = _mm_shuffle_ps(in[3], in[2], _MM_SHUFFLE(0, 0, 0, 0));

			__m128 Swp00 = _mm_shuffle_ps(in[2], in[1], _MM_SHUFFLE(0, 0, 0, 0));
			__m128 Swp01 = _mm_shuffle_ps(Swp0a, Swp0a, _MM_SHUFFLE(2, 0, 0, 0));
			__m128 Swp02 = _mm_shuffle_ps(Swp0b, Swp0b, _MM_SHUFFLE(2, 0, 0, 0));
			__m128 Swp03 = _mm_shuffle_ps(in[2], in[1], _MM_SHUFFLE(3, 3, 3, 3));

			__m128 Mul00 = _mm_mul_ps(Swp00, Swp01);
			__m128 Mul01 = _mm_mul_ps(Swp02, Swp03);
			Fac3 = _mm_sub_ps(Mul00, Mul01);
		}

		__m128 Fac4;
		{
			//	valType SubFactor04 = m[2][0] * m[3][2] - m[3][0] * m[2][2];
			//	valType SubFactor04 = m[2][0] * m[3][2] - m[3][0] * m[2][2];
			//	valType SubFactor10 = m[1][0] * m[3][2] - m[3][0] * m[1][2];
			//	valType SubFactor17 = m[1][0] * m[2][2] - m[2][0] * m[1][2];

			__m128 Swp0a = _mm_shuffle_ps(in[3], in[2], _MM_SHUFFLE(2, 2, 2, 2));
			__m128 Swp0b = _mm_shuffle_ps(in[3], in[2], _MM_SHUFFLE(0, 0, 0, 0));

			__m128 Swp00 = _mm_shuffle_ps(in[2], in[1], _MM_SHUFFLE(0, 0, 0, 0));
			__m128 Swp01 = _mm_shuffle_ps(Swp0a, Swp0a, _MM_SHUFFLE(2, 0, 0, 0));
			__m128 Swp02 = _mm_shuffle_ps(Swp0b, Swp0b, _MM_SHUFFLE(2, 0, 0, 0));
			__m128 Swp03 = _mm_shuffle_ps(in[2], in[1], _MM_SHUFFLE(2, 2, 2, 2));

			__m128 Mul00 = _mm_mul_ps(Swp00, Swp01);
			__m128 Mul01 = _mm_mul_ps(Swp02, Swp03);
			Fac4 = _mm_sub_ps(Mul00, Mul01);
		}

		__m128 Fac5;
		{
			//	valType SubFactor05 = m[2][0] * m[3][1] - m[3][0] * m[2][1];
			//	valType SubFactor05 = m[2][0] * m[3][1] - m[3][0] * m[2][1];
			//	valType SubFactor12 = m[1][0] * m[3][1] - m[3][0] * m[1][1];
			//	valType SubFactor18 = m[1][0] * m[2][1] - m[2][0] * m[1][1];

			__m128 Swp0a = _mm_shuffle_ps(in[3], in[2], _MM_SHUFFLE(1, 1, 1, 1));
			__m128 Swp0b = _mm_shuffle_ps(in[3], in[2], _MM_SHUFFLE(0, 0, 0, 0));

			__m128 Swp00 = _mm_shuffle_ps(in[2], in[1], _MM_SHUFFLE(0, 0, 0, 0));
			__m128 Swp01 = _mm_shuffle_ps(Swp0a, Swp0a, _MM_SHUFFLE(2, 0, 0, 0));
			__m128 Swp02 = _mm_shuffle_ps(Swp0b, Swp0b, _MM_SHUFFLE(2, 0, 0, 0));
			__m128 Swp03 = _mm_shuffle_ps(in[2], in[1], _MM_SHUFFLE(1, 1, 1, 1));

			__m128 Mul00 = _mm_mul_ps(Swp00, Swp01);
			__m128 Mul01 = _mm_mul_ps(Swp02, Swp03);
			Fac5 = _mm_sub_ps(Mul00, Mul01);
		}

		__m128 SignA = _mm_set_ps(1.0f, -1.0f, 1.0f, -1.0f);
		__m128 SignB = _mm_set_ps(-1.0f, 1.0f, -1.0f, 1.0f);

		// m[1][0]
		// m[0][0]
		// m[0][0]
		// m[0][0]
		__m128 Temp0 = _mm_shuffle_ps(in[1], in[0], _MM_SHUFFLE(0, 0, 0, 0));
		__m128 Vec0 = _mm_shuffle_ps(Temp0, Temp0, _MM_SHUFFLE(2, 2, 2, 0));

		// m[1][1]
		// m[0][1]
		// m[0][1]
		// m[0][1]
		__m128 Temp1 = _mm_shuffle_ps(in[1], in[0], _MM_SHUFFLE(1, 1, 1, 1));
		__m128 Vec1 = _mm_shuffle_ps(Temp1, Temp1, _MM_SHUFFLE(2, 2, 2, 0));

		// m[1][2]
		// m[0][2]
		// m[0][2]
		// m[0][2]
		__m128 Temp2 = _mm_shuffle_ps(in[1], in[0], _MM_SHUFFLE(2, 2, 2, 2));
		__m128 Vec2 = _mm_shuffle_ps(Temp2, Temp2, _MM_SHUFFLE(2, 2, 2, 0));

		// m[1][3]
		// m[0][3]
		// m[0][3]
		// m[0][3]
		__m128 Temp3 = _mm_shuffle_ps(in[1], in[0], _MM_SHUFFLE(3, 3, 3, 3));
		__m128 Vec3 = _mm_shuffle_ps(Temp3, Temp3, _MM_SHUFFLE(2, 2, 2, 0));

		// col0
		// + (Vec1[0] * Fac0[0] - Vec2[0] * Fac1[0] + Vec3[0] * Fac2[0]),
		// - (Vec1[1] * Fac0[1] - Vec2[1] * Fac1[1] + Vec3[1] * Fac2[1]),
		// + (Vec1[2] * Fac0[2] - Vec2[2] * Fac1[2] + Vec3[2] * Fac2[2]),
		// - (Vec1[3] * Fac0[3] - Vec2[3] * Fac1[3] + Vec3[3] * Fac2[3]),
		__m128 Mul00 = _mm_mul_ps(Vec1, Fac0);
		__m128 Mul01 = _mm_mul_ps(Vec2, Fac1);
		__m128 Mul02 = _mm_mul_ps(Vec3, Fac2);
		__m128 Sub00 = _mm_sub_ps(Mul00, Mul01);
		__m128 Add00 = _mm_add_ps(Sub00, Mul02);
		__m128 Inv0 = _mm_mul_ps(SignB, Add00);

		// col1
		// - (Vec0[0] * Fac0[0] - Vec2[0] * Fac3[0] + Vec3[0] * Fac4[0]),
		// + (Vec0[0] * Fac0[1] - Vec2[1] * Fac3[1] + Vec3[1] * Fac4[1]),
		// - (Vec0[0] * Fac0[2] - Vec2[2] * Fac3[2] + Vec3[2] * Fac4[2]),
		// + (Vec0[0] * Fac0[3] - Vec2[3] * Fac3[3] + Vec3[3] * Fac4[3]),
		__m128 Mul03 = _mm_mul_ps(Vec0, Fac0);
		__m128 Mul04 = _mm_mul_ps(Vec2, Fac3);
		__m128 Mul05 = _mm_mul_ps(Vec3, Fac4);
		__m128 Sub01 = _mm_sub_ps(Mul03, Mul04);
		__m128 Add01 = _mm_add_ps(Sub01, Mul05);
		__m128 Inv1 = _mm_mul_ps(SignA, Add01);

		// col2
		// + (Vec0[0] * Fac1[0] - Vec1[0] * Fac3[0] + Vec3[0] * Fac5[0]),
		// - (Vec0[0] * Fac1[1] - Vec1[1] * Fac3[1] + Vec3[1] * Fac5[1]),
		// + (Vec0[0] * Fac1[2] - Vec1[2] * Fac3[2] + Vec3[2] * Fac5[2]),
		// - (Vec0[0] * Fac1[3] - Vec1[3] * Fac3[3] + Vec3[3] * Fac5[3]),
		__m128 Mul06 = _mm_mul_ps(Vec0, Fac1);
		__m128 Mul07 = _mm_mul_ps(Vec1, Fac3);
		__m128 Mul08 = _mm_mul_ps(Vec3, Fac5);
		__m128 Sub02 = _mm_sub_ps(Mul06, Mul07);
		__m128 Add02 = _mm_add_ps(Sub02, Mul08);
		__m128 Inv2 = _mm_mul_ps(SignB, Add02);

		// col3
		// - (Vec1[0] * Fac2[0] - Vec1[0] * Fac4[0] + Vec2[0] * Fac5[0]),
		// + (Vec1[0] * Fac2[1] - Vec1[1] * Fac4[1] + Vec2[1] * Fac5[1]),
		// - (Vec1[0] * Fac2[2] - Vec1[2] * Fac4[2] + Vec2[2] * Fac5[2]),
		// + (Vec1[0] * Fac2[3] - Vec1[3] * Fac4[3] + Vec2[3] * Fac5[3]));
		__m128 Mul09 = _mm_mul_ps(Vec0, Fac2);
		__m128 Mul10 = _mm_mul_ps(Vec1, Fac4);
		__m128 Mul11 = _mm_mul_ps(Vec2, Fac5);
		__m128 Sub03 = _mm_sub_ps(Mul09, Mul10);
		__m128 Add03 = _mm_add_ps(Sub03, Mul11);
		__m128 Inv3 = _mm_mul_ps(SignA, Add03);

		__m128 Row0 = _mm_shuffle_ps(Inv0, Inv1, _MM_SHUFFLE(0, 0, 0, 0));
		__m128 Row1 = _mm_shuffle_ps(Inv2, Inv3, _MM_SHUFFLE(0, 0, 0, 0));
		__m128 Row2 = _mm_shuffle_ps(Row0, Row1, _MM_SHUFFLE(2, 0, 2, 0));

		//	valType Determinant = m[0][0] * Inverse[0][0]
		//						+ m[0][1] * Inverse[1][0]
		//						+ m[0][2] * Inverse[2][0]
		//						+ m[0][3] * Inverse[3][0];
		__m128 Det0 = v4_dot(in[0], Row2);
		__m128 Rcp0 = _mm_div_ps(_mm_set1_ps(1.0f), Det0);
		//__m128 Rcp0 = _mm_rcp_ps(Det0);

		f32 row_1[4] = {};
		f32 row_2[4] = {};
		f32 row_3[4] = {};
		f32 row_4[4] = {};
		_mm_store_ps(row_1, _mm_mul_ps(Inv0, Rcp0));
		_mm_store_ps(row_2, _mm_mul_ps(Inv1, Rcp0));
		_mm_store_ps(row_3, _mm_mul_ps(Inv2, Rcp0));
		_mm_store_ps(row_4, _mm_mul_ps(Inv3, Rcp0));

		m4 out = identity();
		//	Inverse /= Determinant;
		add_row_from_f32_arr(&out, row_1, 0);
		add_row_from_f32_arr(&out, row_2, 1);
		add_row_from_f32_arr(&out, row_3, 2);
		add_row_from_f32_arr(&out, row_4, 3);
		return out;
	}

	inline b32
	inverse_matrix(m4 input, m4 *output)
	{
		double det, inv[16];
		float *m = &(input.m[0][0]);

		inv[0] = m[5] * m[10] * m[15] -
				 m[5] * m[11] * m[14] -
				 m[9] * m[6] * m[15] +
				 m[9] * m[7] * m[14] +
				 m[13] * m[6] * m[11] -
				 m[13] * m[7] * m[10];

		inv[4] = -m[4] * m[10] * m[15] +
				 m[4] * m[11] * m[14] +
				 m[8] * m[6] * m[15] -
				 m[8] * m[7] * m[14] -
				 m[12] * m[6] * m[11] +
				 m[12] * m[7] * m[10];

		inv[8] = m[4] * m[9] * m[15] -
				 m[4] * m[11] * m[13] -
				 m[8] * m[5] * m[15] +
				 m[8] * m[7] * m[13] +
				 m[12] * m[5] * m[11] -
				 m[12] * m[7] * m[9];

		inv[12] = -m[4] * m[9] * m[14] +
				  m[4] * m[10] * m[13] +
				  m[8] * m[5] * m[14] -
				  m[8] * m[6] * m[13] -
				  m[12] * m[5] * m[10] +
				  m[12] * m[6] * m[9];

		inv[1] = -m[1] * m[10] * m[15] +
				 m[1] * m[11] * m[14] +
				 m[9] * m[2] * m[15] -
				 m[9] * m[3] * m[14] -
				 m[13] * m[2] * m[11] +
				 m[13] * m[3] * m[10];

		inv[5] = m[0] * m[10] * m[15] -
				 m[0] * m[11] * m[14] -
				 m[8] * m[2] * m[15] +
				 m[8] * m[3] * m[14] +
				 m[12] * m[2] * m[11] -
				 m[12] * m[3] * m[10];

		inv[9] = -m[0] * m[9] * m[15] +
				 m[0] * m[11] * m[13] +
				 m[8] * m[1] * m[15] -
				 m[8] * m[3] * m[13] -
				 m[12] * m[1] * m[11] +
				 m[12] * m[3] * m[9];

		inv[13] = m[0] * m[9] * m[14] -
				  m[0] * m[10] * m[13] -
				  m[8] * m[1] * m[14] +
				  m[8] * m[2] * m[13] +
				  m[12] * m[1] * m[10] -
				  m[12] * m[2] * m[9];

		inv[2] = m[1] * m[6] * m[15] -
				 m[1] * m[7] * m[14] -
				 m[5] * m[2] * m[15] +
				 m[5] * m[3] * m[14] +
				 m[13] * m[2] * m[7] -
				 m[13] * m[3] * m[6];

		inv[6] = -m[0] * m[6] * m[15] +
				 m[0] * m[7] * m[14] +
				 m[4] * m[2] * m[15] -
				 m[4] * m[3] * m[14] -
				 m[12] * m[2] * m[7] +
				 m[12] * m[3] * m[6];

		inv[10] = m[0] * m[5] * m[15] -
				  m[0] * m[7] * m[13] -
				  m[4] * m[1] * m[15] +
				  m[4] * m[3] * m[13] +
				  m[12] * m[1] * m[7] -
				  m[12] * m[3] * m[5];

		inv[14] = -m[0] * m[5] * m[14] +
				  m[0] * m[6] * m[13] +
				  m[4] * m[1] * m[14] -
				  m[4] * m[2] * m[13] -
				  m[12] * m[1] * m[6] +
				  m[12] * m[2] * m[5];

		inv[3] = -m[1] * m[6] * m[11] +
				 m[1] * m[7] * m[10] +
				 m[5] * m[2] * m[11] -
				 m[5] * m[3] * m[10] -
				 m[9] * m[2] * m[7] +
				 m[9] * m[3] * m[6];

		inv[7] = m[0] * m[6] * m[11] -
				 m[0] * m[7] * m[10] -
				 m[4] * m[2] * m[11] +
				 m[4] * m[3] * m[10] +
				 m[8] * m[2] * m[7] -
				 m[8] * m[3] * m[6];

		inv[11] = -m[0] * m[5] * m[11] +
				  m[0] * m[7] * m[9] +
				  m[4] * m[1] * m[11] -
				  m[4] * m[3] * m[9] -
				  m[8] * m[1] * m[7] +
				  m[8] * m[3] * m[5];

		inv[15] = m[0] * m[5] * m[10] -
				  m[0] * m[6] * m[9] -
				  m[4] * m[1] * m[10] +
				  m[4] * m[2] * m[9] +
				  m[8] * m[1] * m[6] -
				  m[8] * m[2] * m[5];

		det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];

		if (det == 0)
			return false;

		det = 1.0 / det;

		int iterator = 0;
		for (int x = 0; x < 4; x++)
		{
			for (int y = 0; y < 4; y++)
			{
				output->m[x][y] = inv[iterator] * det;
				iterator++;
			}
		}
		return true;
	}

#if defined(__cplusplus)
}
#endif
