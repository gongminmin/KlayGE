// Math3DNowEx.cpp
// KlayGE 数学函数库3DNow!优化 实现文件
// Ver 1.3.8.2
// 版权所有(C) 龚敏敏, 2003
// Homepage: http://www.enginedev.com
//
// 1.3.8.2
// 初次建立 (2003.1.2)
//
// 修改记录
///////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Math.hpp>

#include "MathImpl.hpp"

namespace KlayGE
{
	namespace KlayGEImpl
	{
		const long	mabs		= 0x7FFFFFFF;			// mask for absolute value (~sgn)
		const float one			= 1;

		const float ones[2]		= { 1, 1 };

		// sincos专用
		const float xmax		= 25735.9f;
		const float fouropi		= 1.27324f;							// 4 / PI
		const float pio4ht[2]	= { -0.785156f, -0.000241913f };
		const float pio4s[2]	= { 0.785398f, 0.785398f };			// PI / 4
		const float mo56_42[2]	= { -0.0178571f, -0.0238095f };		// 1 / 56 | 1 / 42
		const float mo30_20[2]	= { -0.0333333f, -0.05f };			// 1 / 30 | 1 / 20
		const float mo12_6[2]	= { -0.0833333f, -0.166667f };		// 1 / 12 | 1 / 6
		const float mo2s[2]		= { -0.5f, -0.5f };
		const int	iones[2]	= { 1, 1 };


		static const float CONST_1_0[2] = { 1, 0 };
		static const float CONST_0_1[2] = { 0, 1 };
		static const float CONST_1_1[2] = { 1, 1 };

		// 计算sin的值
		/////////////////////////////////////////////////////////////////////////////////
		float Math3DNowExLib::Sin(float x) const
		{
			float s, c;
			this->SinCos(x, s, c);
			return s;
		}

		// 计算cos的值
		/////////////////////////////////////////////////////////////////////////////////
		float Math3DNowExLib::Cos(float x) const
		{
			float s, c;
			this->SinCos(x, s, c);
			return c;
		}

		// 一次计算sin和cos的值
		/////////////////////////////////////////////////////////////////////////////////
		void Math3DNowExLib::SinCos(float x, float& s, float& c) const
		{
			_asm
			{
				femms

				movd		mm0, x
				movd		eax, mm0
				movq		mm1, mm0
				movd		mm3, mabs
				mov			ebx, eax
				mov			edx, eax
				pand		mm0, mm3				// m0 = fabs(x)
				and			ebx, 0x80000000			// get sign bit
				shr			edx, 31
				xor			eax, ebx				// sign(ebx) = sign(eax)
				cmp			eax, xmax
				movd		mm2, fouropi
				jl			X2
				movd		mm0, one
				jmp			ENDING

			X2:
				movq		mm1, mm0
				pfmul		mm0, mm2				// mm0 = fabs(x) * 4 / PI
				movq		mm3, pio4ht
				pf2id		mm0, mm0
				movq		mm7, mo56_42
				movd		ecx, mm0
				pi2fd		mm0, mm0
				mov			esi, ecx
				movq		mm6, mo30_20
				punpckldq	mm0, mm0
				movq		mm5, ones
				pfmul		mm0, mm3
				movq		mm3, pio4s
				pfadd		mm1, mm0
				shr			esi, 2
				punpckhdq	mm0, mm0
				xor			edx, esi
				pfadd		mm1, mm0
				test		ecx, 1
				punpckldq	mm1, mm1
				jz			X5
				pfsubr		mm1, mm3

			X5:
				movq		mm2, mm5
				shl			edx, 31
				punpckldq	mm2, mm1
				pfmul		mm1, mm1
				mov			esi, ecx
				movq		mm4, mo12_6
				shr			esi, 1
				pfmul		mm7, mm1
				xor			ecx, esi
				pfmul		mm6, mm1
				shl			esi, 31
				pfadd		mm7, mm5
				xor			ebx, esi
				pfmul		mm4, mm1
				pfmul		mm7, mm6
				movq		mm6, mo2s
				pfadd		mm7, mm5
				pfmul		mm6, mm1
				pfmul		mm4, mm7
				movd		mm0, edx
				pfadd		mm4, mm5
				punpckldq	mm6, mm5
				psrlq		mm5, 32
				pfmul		mm4, mm6
				punpckldq	mm0, mm0
				movd		mm1, ebx
				pfadd		mm4, mm5
				test		ecx, 1
				pfmul		mm4, mm2
				jz			X7
				punpckldq	mm5, mm4
				punpckhdq	mm4, mm5

			X7:
				pxor		mm4, mm1
				pxor		mm0, mm4

			ENDING:
				mov			eax, c
				mov			ebx, s
				movd		[eax], mm0
				psrlq		mm0, 32
				movd		[ebx], mm0

				femms
			}
		}

		// 浮点数平方根
		/////////////////////////////////////////////////////////////////////////////////
		float Math3DNowExLib::Sqrt(float x) const
		{
			float fRet;

			_asm
			{
				movd		mm0, x
				pfrsqrt		mm1, mm0
				movq		mm2, mm1
				pfmul		mm1, mm1
				pfrsqit1	mm1, mm0
				pfrcpit2	mm1, mm2
				pfmul		mm0, mm1
				movd		fRet, mm0
				femms
			}

			return fRet;
		}

		// 浮点数平方根的倒数
		/////////////////////////////////////////////////////////////////////////////////
		float Math3DNowExLib::RecipSqrt(float x) const
		{
			float fRet;

			_asm
			{
				movd		mm2, x
				pfrsqrt		mm0, mm2
				movq		mm1, mm0
				pfmul		mm1, mm0
				pfrsqit1	mm2, mm1
				pfrcpit2	mm2, mm0
				movd		fRet, mm2
				femms
			}

			return fRet;
		}


		// 2D 向量
		///////////////////////////////////////////////////////////////////////////////

		Vector2& Math3DNowExLib::Normalize(Vector2& vOut, const Vector2& rhs) const
		{
			_asm
			{
				femms

				mov			eax, vOut
				mov			ecx, rhs

				movq		mm2, [ecx]		// r.x | r.y
				movq		mm7, mm2
				pfmul		mm2, mm2		// r.x * r.x | r.y * r.y
				pfacc		mm2, mm2		// r.x * r.x + r.y * r.y

				pfrsqrt		mm0, mm2
				movq		mm1, mm0
				pfmul		mm1, mm0
				pfrsqit1	mm2, mm1
				pfrcpit2	mm2, mm0

				punpckldq	mm2, mm2

				pfmul		mm7, mm2

				movq		[eax], mm7

				femms
			}
		}

		Vector4& Math3DNowExLib::Transform(Vector4& vOut, const Vector2& v, const Matrix4& mat) const
		{
			_asm
			{
				femms

				mov			eax, vOut
				mov			ecx, v
				mov			edx, mat

				movq		mm0, [ecx]		// v.x | v.y
				pshufw      mm1, mm0, 0xEE	// v.y | v.y
				punpckldq	mm0, mm0		// v.x | v.x

				movq		mm2, [edx]		// m.11 | m.12
				movq		mm3, [edx + 8]	// m.13 | m.14
				movq		mm4, [edx + 16]	// m.21 | m.22
				movq		mm5, [edx + 24]	// m.23 | m.24

				pfmul		mm2, mm0		// v.x * m.11 | v.x * m.12
				pfmul		mm3, mm0		// v.x * m.13 | v.x * m.14
				pfmul		mm4, mm1		// v.y * m.21 | v.y * m.22
				pfmul		mm5, mm1		// v.y * m.23 | v.y * m.24

				pfadd		mm2, mm4		// v.x * m.11 + v.y * m.21 | v.x * m.12 + v.y * m.22
				pfadd		mm3, mm5		// v.x * m.13 + v.y * m.23 | v.x * m.14 + v.y * m.24

				pfadd		mm2, [edx + 48]	// v.x * m.11 + v.y * m.21 + m.41 | v.x * m.12 + v.y * m.22 + m.42
				pfadd		mm3, [edx + 56]	// v.x * m.13 + v.y * m.23 + m.43 | v.x * m.14 + v.y * m.24 + m.44

				movq		[eax], mm2		// r.x | r.y
				movq		[eax + 8], mm3	// r.z | r.w

				femms
			}
		}

		Vector2& Math3DNowExLib::TransformCoord(Vector2& vOut, const Vector2& v, const Matrix4& mat) const
		{
			_asm
			{
				femms

				mov			eax, vOut
				mov			ecx, v
				mov			edx, mat

				movq		mm0, [ecx]		// v.x | v.y
				pshufw      mm1, mm0, 0xEE	// v.y | v.y
				punpckldq	mm0, mm0		// v.x | v.x

				pfmul		mm0, [edx]		// v.x * m.11 | v.x * m.12
				pfmul		mm1, [edx + 16]	// v.y * m.21 | v.y * m.22

				pfadd		mm0, mm1		// v.x * m.11 + v.y * m.21 | v.x * m.12 + v.y * m.22
				pfadd		mm0, [edx + 48]	// v.x * m.11 + v.y * m.21 + m.41 | v.x * m.12 + v.y * m.22 + m.42

				movq		[eax], mm0

				femms
			}
		}

		Vector2& Math3DNowExLib::TransformNormal(Vector2& vOut, const Vector2& v, const Matrix4& mat) const
		{
			_asm
			{
				femms

				mov			eax, vOut
				mov			ecx, v
				mov			edx, mat

				movq		mm0, [ecx]		// v.x | v.y
				pshufw      mm1, mm0, 0xEE	// v.y | v.y
				punpckldq	mm0, mm0		// v.x | v.x

				pfmul		mm0, [edx]		// v.x * m.11 | v.x * m.12
				pfmul		mm1, [edx + 16]	// v.y * m.21 | v.y * m.22

				pfadd		mm0, mm1		// v.x * m.11 + v.y * m.21 | v.x * m.12 + v.y * m.22

				movq		[eax], mm0

				femms
			}
		}


		// 3D 向量
		///////////////////////////////////////////////////////////////////////////////

		Vector3& Math3DNowExLib::Cross(Vector3& vOut, const Vector3& lhs, const Vector3& rhs) const
		{
			_asm
			{
				femms

				mov			eax, vOut
				mov			ecx, lhs
				mov			edx, rhs

				movq		mm0, [ecx + 4]	// l.y | l.z
				movd		mm1, [edx + 8]	// r.z
				punpckldq	mm1, [edx]		// r.z | r.x
				movd		mm2, [ecx + 8]	// l.z
				punpckldq	mm2, [ecx]		// l.z | l.x
				movq		mm3, [edx + 4]	// r.y | r.z
				pfmul		mm0, mm1		// l.y * r.z | l.z * r.x
				pfmul		mm2, mm3		// l.z * r.y | l.x * r.z
				pfsub		mm0, mm2		// l.y * r.z - l.z * r.y | l.z * r.x - l.x * r.z

				movq		mm4, [ecx]		// l.x | l.y
				pswapd		mm5, [edx]		// r.y | r.x
				pfmul		mm4, mm5		// l.x * r.y | l.y * r.x
				pfnacc		mm4, mm4		// l.x * r.y - l.y * r.x

				movq		[eax], mm0
				movd		[eax + 8], mm4

				femms
			}
		}

		Vector3& Math3DNowExLib::Normalize(Vector3& vOut, const Vector3& rhs) const
		{
			_asm
			{
				femms

				mov			eax, vOut
				mov			ecx, rhs

				movq		mm2, [ecx]		// r.x | r.y
				movq		mm6, mm2
				movd		mm3, [ecx + 8]	// r.z
				pfmul		mm2, mm2		// r.x * r.x | r.y * r.y
				movq		mm7, mm3
				pfacc		mm2, mm2		// r.x * r.x + r.y * r.y
				pfmul		mm3, mm3		// r.z * r.z
				pfadd		mm2, mm3		// r.x * r.x + r.y * r.y + r.z * r.z

				pfrsqrt		mm0, mm2
				movq		mm1, mm0
				pfmul		mm1, mm0
				pfrsqit1	mm2, mm1
				pfrcpit2	mm2, mm0

				punpckldq	mm2, mm2

				pfmul		mm6, mm2
				pfmul		mm7, mm2

				movq		[eax], mm6
				movd		[eax + 8], mm7

				femms
			}
		}

		Vector4& Math3DNowExLib::Transform(Vector4& vOut, const Vector3& v, const Matrix4& mat) const
		{
			_asm
			{
				femms

				mov			eax, vOut
				mov			ecx, v
				mov			edx, mat

				movq		mm0, [ecx]		// v.x | v.y
				pshufw		mm1, mm0, 0xEE	// v.y | v.y
				punpckldq	mm0, mm0		// v.x | v.x

				movq		mm7, [edx + 24]	// m.23 | m.24
				movq		mm6, [edx + 16]	// m.21 | m.22
				movq		mm5, [edx + 8]	// m.13 | m.14
				movq		mm4, [edx]		// m.11 | m.12

				pfmul		mm4, mm0		// v.x * m.11 | v.x * m.12
				pfmul		mm6, mm1		// v.y * m.21 | v.y * m.22
				pfmul		mm5, mm0		// v.x * m.13 | v.x * m.14
				pfmul		mm7, mm1		// v.y * m.23 | v.y * m.24
				
				pfadd		mm6, mm4		// v.x * m.11 + v.y * m.21 | v.x * m.12 + v.y * m.22
				pfadd		mm7, mm5		// v.x * m.13 + v.y * m.23 | v.x * m.14 + v.y * m.24

				movd		mm0, [ecx + 8]	// v.z
				punpckldq	mm0, mm0		// v.z | v.z
				
				movq		mm2, [edx + 32]	// m.31 | m.32
				movq		mm3, [edx + 40]	// m.33 | m.34

				pfmul		mm2, mm0		// v.z * m.31 | v.z * m.32
				pfmul		mm3, mm0		// v.z * m.33 | v.z * m.34
				pfadd		mm6, mm2		// v.x * m.11 + v.y * m.21 + v.z * m.31 | v.x * m.12 + v.y * m.22 + v.z * m.32
				pfadd		mm7, mm3		// v.x * m.13 + v.y * m.23 + v.z * m.33 | v.x * m.14 + v.y * m.24 + v.z * m.34
				pfadd		mm6, [edx + 48]	// v.x * m.11 + v.y * m.21 + v.z * m.31 + m.41 | v.x * m.12 + v.y * m.22 + v.z * m.32 + m.42
				pfadd		mm7, [edx + 56]	// v.x * m.13 + v.y * m.23 + v.z * m.33 + m.43 | v.x * m.14 + v.y * m.24 + v.z * m.34 + m.44

				movq		[eax], mm6
				movq		[eax + 8], mm7

				femms
			}
		}

		Vector3& Math3DNowExLib::TransformCoord(Vector3& vOut, const Vector3& v, const Matrix4& mat) const
		{
			_asm
			{
				femms

				mov			eax, vOut
				mov			ecx, v
				mov			edx, mat

				movq		mm0, [ecx]		// v.x | v.y
				pshufw      mm1, mm0, 0xEE	// v.y | v.y
				punpckldq	mm0, mm0		// v.x | v.x

				movq		mm4, [edx]		// m.11 | m.12
				movq		mm5, [edx + 8]	// m.13 | m.14
				movq		mm6, [edx + 16]	// m.21 | m.22
				movq		mm7, [edx + 24]	// m.23 | m.24

				pfmul		mm4, mm0		// v.x * m.11 | v.x * m.12
				pfmul		mm5, mm0		// v.x * m.13 | v.x * m.14
				pfmul		mm6, mm1		// v.y * m.21 | v.y * m.22
				pfmul		mm7, mm1		// v.y * m.23 | v.y * m.24
				pfadd		mm6, mm4		// v.x * m.11 + v.y * m.21 | v.x * m.12 + v.y * m.22
				pfadd		mm7, mm5		// v.x * m.13 + v.y * m.23 | v.x * m.14 + v.y * m.24

				movd		mm0, [ecx + 8]	// v.z
				punpckldq	mm0, mm0		// v.z | v.z

				movq		mm3, [edx + 40]	// m.33 | m.34
				movq		mm2, [edx + 32]	// m.31 | m.32

				pfmul		mm2, mm0		// v.z * m.31 | v.z * m.32
				pfmul		mm3, mm0		// v.z * m.33 | v.z * m.34
				pfadd		mm6, mm2		// v.x * m.11 + v.y * m.21 + v.z * m.31 | v.x * m.12 + v.y * m.22 + v.z * m.32
				pfadd		mm7, mm3		// v.x * m.13 + v.y * m.23 + v.z * m.33 | v.x * m.14 + v.y * m.24 + v.z * m.34
				pfadd		mm7, [edx + 56]	// v.x * m.13 + v.y * m.23 + v.z * m.33 + m.43 | v.x * m.14 + v.y * m.24 + v.z * m.34 + m.44
				pfadd		mm6, [edx + 48]	// v.x * m.11 + v.y * m.21 + v.z * m.31 + m.41 | v.x * m.12 + v.y * m.22 + v.z * m.32 + m.42

				movd		[eax + 8], mm7
				movq		[eax], mm6

				femms
			}
		}

		Vector3& Math3DNowExLib::TransformNormal(Vector3& vOut, const Vector3& v, const Matrix4& mat) const
		{
			_asm
			{
				femms

				mov			eax, vOut
				mov			ecx, v
				mov			edx, mat

				movq		mm0, [ecx]		// v.x | v.y
				pshufw      mm1, mm0, 0xEE	// v.y | v.y
				punpckldq	mm0, mm0		// v.x | v.x

				movq		mm7, [edx + 24]	// m.23 | m.24
				movq		mm6, [edx + 16]	// m.21 | m.22
				movq		mm5, [edx + 8]	// m.13 | m.14
				movq		mm4, [edx]		// m.11 | m.12

				pfmul		mm4, mm0		// v.x * m.11 | v.x * m.12
				pfmul		mm5, mm0		// v.x * m.13 | v.x * m.14
				pfmul		mm6, mm1		// v.y * m.21 | v.y * m.22
				pfmul		mm7, mm1		// v.y * m.23 | v.y * m.24
				pfadd		mm6, mm4		// v.x * m.11 + v.y * m.21 | v.x * m.12 + v.y * m.22
				pfadd		mm7, mm5		// v.x * m.13 + v.y * m.23 | v.x * m.14 + v.y * m.24

				movd		mm0, [ecx + 8]	// v.z
				punpckldq	mm0, mm0		// v.z | v.z

				movq		mm3, [edx + 40]	// m.33 | m.34
				movq		mm2, [edx + 32]	// m.31 | m.32

				pfmul		mm2, mm0		// v.z * m.31 | v.z * m.32
				pfmul		mm3, mm0		// v.z * m.33 | v.z * m.34
				pfadd		mm6, mm2		// v.x * m.11 + v.y * m.21 + v.z * m.31 | v.x * m.12 + v.y * m.22 + v.z * m.32
				pfadd		mm7, mm3		// v.x * m.13 + v.y * m.23 + v.z * m.33 | v.x * m.14 + v.y * m.24 + v.z * m.34

				movq		[eax], mm6
				movd		[eax + 8], mm7

				femms
			}
		}

		Vector3& Math3DNowExLib::TransQuat(Vector3& vOut, const Vector3& v, const Quaternion& q) const
		{
			// result = av + bq + c(q.v CROSS v)
			// where
			//  a = q.w^2 - (q.v DOT q.v)
			//  b = 2 * (q.v DOT v)
			//  c = 2q.w
			const float a = q.w() * q.w() - this->Dot(q.v(), q.v());
			const float b = 2 * this->Dot(q.v(), v);
			const float c = q.w() + q.w();

			// Must store this, because result may alias v
			Vector3 cross;
			this->Cross(cross, q.v(), v);		// q.v CROSS v

			_asm
			{
				femms

				mov			eax, vOut
				mov			ebx, v
				mov			ecx, q
				lea			edx, cross

				movd		mm0, a			// a
				punpckldq	mm0, mm0		// a | a

				movd		mm1, b			// b
				punpckldq	mm1, mm1		// b | b

				movd		mm2, c			// c
				punpckldq	mm2, mm2		// c | c

				movd		mm5, [ebx + 8]	// v.z
				movq		mm4, [ebx]		// v.x | v.y
				movd		mm7, [ecx + 8]	// q.z
				movq		mm6, [ecx]		// q.x | q.y

				pfmul		mm4, mm0		// a * v.x | a * v.y
				pfmul		mm5, mm0		// a * v.z
				pfmul		mm6, mm1		// b * q.x | b * q.y
				pfadd		mm6, mm4		// a * v.x + b * q.x | a * v.y + b * q.y
				pfmul		mm7, mm1		// b * q.z
				pfadd		mm7, mm5		// a * v.z + b * q.z

				movd		mm5, [edx + 8]	// vCross.z
				movq		mm4, [edx]		// vCross.x | vCross.y

				pfmul		mm4, mm2		// c * vCross.x | c * vCross.y
				pfadd		mm6, mm4		// a * v.x + b * q.x + c * vCross.x | a * v.y + b * q.y + c * vCross.y
				pfmul		mm5, mm2		// c * vCross.z
				pfadd		mm7, mm5		// a * v.z + b * q.z + c * vCross.z

				movq		[eax], mm6
				movd		[eax + 8], mm7

				femms
			}
		}


		// 4D 向量
		///////////////////////////////////////////////////////////////////////////////

		Vector4& Math3DNowExLib::Cross(Vector4& vOut, const Vector4& v1, const Vector4& v2, const Vector4& v3) const
		{
			float A, B, C, D, E, F;

			_asm
			{
				femms

				mov         eax, vOut
				mov			ebx, v1
				mov			ecx, v2
				mov			edx, v3

				movq		mm0, [ecx]
				pswapd		mm1, [edx]		// v3.y | v3.x
				pfmul		mm0, mm1		// v2.x * v3.y | v2.y * v3.x
				pfnacc		mm0, mm0		// v2.x * v3.y - v2.y * v3.x
				movd		A, mm0

				movd		mm1, [ecx]
				punpckldq	mm1, [ecx + 8]	// v2.x | v2.z
				movd		mm2, [edx + 8]
				punpckldq	mm2, [edx]		// v3.z | v3.x
				pfmul		mm1, mm2		// v2.x * v3.z | v2.z * v3.x
				pfnacc		mm1, mm1		// v2.x * v3.z - v2.z * v3.x
				movd		B, mm1

				movd		mm2, [ecx]
				punpckldq	mm2, [ecx + 12]	// v2.x | v2.w
				movd		mm3, [edx + 12]
				punpckldq	mm3, [edx]		// v3.w | v3.x
				pfmul		mm2, mm3		// v2.x * v3.w | v2.w * v3.x
				pfnacc		mm2, mm2		// v2.x * v3.w - v2.w * v3.x
				movd		C, mm2

				movq		mm3, [ecx + 4]
				pswapd		mm4, [edx + 4]	// v3.z | v3.y
				pfmul		mm3, mm4		// v2.y * v3.z | v2.z * v3.y
				pfnacc		mm3, mm3		// v2.y * v3.z - v2.z * v3.y
				movd		D, mm3

				movd		mm4, [ecx + 4]
				punpckldq	mm4, [ecx + 12]	// v2.y | v2.w
				movd		mm5, [edx + 12]
				punpckldq	mm5, [edx + 4]	// v3.w | v3.y
				pfmul		mm4, mm5		// v2.y * v3.w | v2.w * v3.y
				pfnacc		mm4, mm5		// v2.y * v3.w - v2.w * v3.y
				movd		E, mm4

				movq		mm5, [ecx + 8]
				pswapd		mm6, [edx + 8]	// v3.w | v3.z
				pfmul		mm5, mm6		// v2.z * v3.w | v2.w * v3.z
				pfnacc		mm5, mm5		// v2.z * v3.w - v2.w * v3.z
				movd		F, mm5

				movq		mm0, [ebx + 4]
				movd		mm1, F
				punpckldq	mm1, E			// F | E
				pfmul		mm0, mm1		// v1.y * F | v1.z * E
				movd		mm2, [ebx + 12]
				movd		mm3, D
				pfmul		mm2, mm3		// v1.w * D
				pfnacc		mm0, mm0		// v1.y * F - v1.z * E
				pfadd		mm0, mm2		// v1.y * F - v1.z * E + v1.w * D

				movq		mm1, [ebx + 8]
				movd		mm2, C
				punpckldq	mm2, B			// C | B
				pfmul		mm1, mm2		// v1.z * C | v1.w * B
				movd		mm3, [ebx]
				movd		mm4, F
				pfmul		mm3, mm4		// v1.x * F
				pfnacc		mm1, mm1		// v1.z * C - v1.w * B
				pfsub		mm1, mm3		// v1.z * C - v1.w * B - v1.x * F

				movq		mm2, [ebx]
				movd		mm3, E
				punpckldq	mm3, C			// E | C
				pfmul		mm2, mm3		// v1.x * E | v1.y * C
				movd		mm4, [ebx + 12]
				movd		mm5, A
				pfmul		mm4, mm5		// v1.w * A
				pfnacc		mm2, mm2		// v1.x * E - v1.y * C
				pfadd		mm2, mm4		// v1.x * E - v1.y * C + v1.w * A

				movq		mm3, [ebx + 4]
				movd		mm4, B
				punpckldq	mm4, A			// B | A
				pfmul		mm3, mm4		// v1.y * B | v1.z * A
				movd		mm5, [ebx]
				movd		mm6, D
				pfmul		mm5, mm6		// v1.x * D
				pfnacc		mm3, mm3		// v1.y * B - v1.z * A
				pfsub		mm3, mm5		// v1.y * B - v1.z * A - v1.x * D

				movd		[eax],		mm0
				movd		[eax + 4],	mm1
				movd		[eax + 8],	mm2
				movd		[eax + 12], mm3

				femms
			}
		}

		Vector4& Math3DNowExLib::Normalize(Vector4& vOut, const Vector4& rhs) const
		{
			_asm
			{
				femms

				mov			eax, vOut
				mov			ecx, rhs

				movq		mm2, [ecx]		// r.x | r.y
				movq		mm3, [ecx + 8]	// r.z | r.w
				movq		mm6, mm2
				movq		mm7, mm3
				pfmul		mm2, mm2		// r.x * r.x | r.y * r.y
				pfmul		mm3, mm3		// r.z * r.z | r.w * r.w
				pfacc		mm2, mm2		// r.x * r.x + r.y * r.y
				pfacc		mm3, mm3		// r.z * r.z + r.w * r.w
				pfadd		mm2, mm3		// r.x * r.x + r.y * r.y + r.z * r.z + r.w * r.w

				pfrsqrt		mm0, mm2
				movq		mm1, mm0
				pfmul		mm1, mm0
				pfrsqit1	mm2, mm1
				pfrcpit2	mm2, mm0

				punpckldq	mm2, mm2

				pfmul		mm6, mm2
				pfmul		mm7, mm2

				movq		[eax], mm6
				movq		[eax + 8], mm7

				femms
			}
		}

		Vector4& Math3DNowExLib::Transform(Vector4& vOut, const Vector4& v, const Matrix4& mat) const
		{
			_asm
			{
				femms

				mov			eax, vOut
				mov			ecx, v
				mov			edx, mat

				movq		mm0, [ecx]		// v.x | v.y
				pshufw      mm1, mm0, 0xEE	// v.y | v.y
				punpckldq	mm0, mm0		// v.x | v.x

				movq		mm7, [edx + 24]	// m.23 | m.24
				movq		mm6, [edx + 16]	// m.21 | m.22
				movq		mm5, [edx + 8]	// m.13 | m.14
				movq		mm4, [edx]		// m.11 | m.12

				pfmul		mm4, mm0		// v.x * m.11 | v.x * m.12
				pfmul		mm5, mm0		// v.x * m.13 | v.x * m.14
				pfmul		mm6, mm1		// v.y * m.21 | v.y * m.22
				pfadd		mm6, mm4		// v.x * m.11 + v.y * m.21 | v.x * m.12 + v.y * m.22
				pfmul		mm7, mm1		// v.y * m.23 | v.y * m.24
				pfadd		mm7, mm5		// v.x * m.13 + v.y * m.23 | v.x * m.14 + v.y * m.24

				movq		mm0, [ecx + 8]	// v.z | v.w
				pshufw      mm1, mm0, 0xEE	// v.w | v.w
				punpckldq	mm0, mm0		// v.z | v.z

				movq		mm5, [edx + 56]	// m.43 | m.44
				movq		mm4, [edx + 48]	// m.41 | m.42
				movq		mm3, [edx + 40]	// m.33 | m.34
				movq		mm2, [edx + 32]	// m.31 | m.32

				pfmul		mm2, mm0		// v.z * m.31 | v.z * m.32
				pfadd		mm6, mm2		// v.x * m.11 + v.y * m.21 + v.z * m.31 | v.x * m.12 + v.y * m.22 + v.z * m.32
				pfmul		mm3, mm0		// v.z * m.33 | v.z * m.34
				pfadd		mm7, mm3		// v.x * m.13 + v.y * m.23 + v.z * m.33 | v.x * m.14 + v.y * m.24 + v.z * m.34
				pfmul		mm4, mm1		// v.w * m.41 | v.w * m.42
				pfadd		mm6, mm4		// v.x * m.11 + v.y * m.21 + v.z * m.31 + v.w * m.41 | v.x * m.12 + v.y * m.22 + v.z * m.32 + v.w * m.42
				pfmul		mm5, mm1		// v.w * m.43 | v.w * m.44
				pfadd		mm7, mm5		// v.x * m.13 + v.y * m.23 + v.z * m.33 + v.w * m.43 | v.x * m.14 + v.y * m.24 + v.z * m.34 + v.w * m.44

				movq		[eax + 8], mm7
				movq		[eax], mm6

				femms
			}
		}


		// 4D 矩阵
		///////////////////////////////////////////////////////////////////////////////

		Matrix4& Math3DNowExLib::Multiply(Matrix4& mOut, const Matrix4& lhs, const Matrix4& rhs) const
		{
			static Matrix4 tmp;
			this->Transpose(tmp, rhs);

			_asm
			{
				lea				ebx, tmp

				mov				eax, mOut
				mov				ecx, lhs

				movq			mm0, [ecx]			// l.11 | l.12
				movq			mm1, [ecx + 8]		// l.13 | l.14
				movq			mm2, mm0			// l.11 | l.12
				movq			mm3, mm1			// l.13 | l.14
				movq			mm4, mm0			// l.11 | l.12
				movq			mm5, mm1			// l.13 | l.14
				pfmul			mm0, [ebx]			// l.11 * m.11 | l.12 * m.12
				pfmul			mm1, [ebx + 8]		// l.13 * m.13 | l.14 * m.13
				movq			mm6, mm2			// l.11 | l.12
				movq			mm7, mm3			// l.13 | l.14
				pfmul			mm2, [ebx + 16]		// l.11 * m.21 | l.12 * m.22
				pfmul			mm3, [ebx + 24]		// l.13 * m.23 | l.14 * m.24
				pfmul			mm4, [ebx + 32]		// l.11 * m.31 | l.12 * m.32
				pfmul			mm5, [ebx + 40]		// l.13 * m.33 | l.14 * m.34
				pfmul			mm6, [ebx + 48]		// l.11 * m.41 | l.12 * m.42
				pfmul			mm7, [ebx + 56]		// l.13 * m.43 | l.14 * m.44

				pfacc			mm0, mm2			// l.11 * m.11 + l.12 * m.12 | l.11 * m.21 + l.12 * m.22
				pfacc			mm1, mm3			// l.13 * m.13 + l.14 * m.14 | l.13 * m.23 + l.14 * m.24
				pfacc			mm4, mm6			// l.11 * m.31 + l.12 * m.32 | l.11 * m.41 + l.12 * m.42
				pfacc			mm5, mm7			// l.13 * m.33 + l.14 * m.34 | l.13 * m.43 + l.14 * m.44
				pfadd			mm0, mm1			// l.11 * m.11 + l.12 * m.12 + l.13 * m.13 + l.14 * m.13 | l.11 * m.21 + l.12 * m.22 + l.13 * m.23 + l.14 * m.24
				pfadd			mm4, mm5			// l.11 * m.31 + l.12 * m.32 + l.13 * m.33 + l.14 * m.34 | l.11 * m.41 + l.12 * m.42 + l.13 * m.43 + l.14 * m.44
				movq			[eax], mm0			// o.11 = l.11 * m.11 + l.12 * m.12 + l.13 * m.13 + l.14 * m.14 | o.12 = l.11 * m.21 + l.12 * m.22 + l.13 * m.23 + l.14 * m.24
				movq			[eax + 8], mm4		// o.13 = l.11 * m.31 + l.12 * m.32 + l.13 * m.33 + l.14 * m.34 | o.14 = l.11 * m.41 + l.12 * m.42 + l.13 * m.43 + l.14 * m.44

				movq			mm0, [ecx + 16]		// l.21 | l.22
				movq			mm1, [ecx + 24]		// l.23 | l.24
				movq			mm2, mm0			// l.21 | l.22
				movq			mm3, mm1			// l.23 | l.24
				movq			mm4, mm0			// l.21 | l.22
				movq			mm5, mm1			// l.23 | l.24
				pfmul			mm0, [ebx]			// l.21 * m.11 | l.22 * m.12
				pfmul			mm1, [ebx + 8]		// l.23 * m.13 | l.24 * m.14
				movq			mm6, mm2			// l.21 | l.22
				movq			mm7, mm3			// l.23 | l.24
				pfmul			mm2, [ebx + 16]		// l.21 * m.21 | l.22 * m.22
				pfmul			mm3, [ebx + 24]		// l.23 * m.23 | l.24 * m.24
				pfmul			mm4, [ebx + 32]		// l.21 * m.31 | l.22 * m.32
				pfmul			mm5, [ebx + 40]		// l.23 * m.33 | l.24 * m.34
				pfmul			mm6, [ebx + 48]		// l.21 * m.41 | l.22 * m.42
				pfmul			mm7, [ebx + 56]		// l.23 * m.43 | l.24 * m.44
				pfacc			mm0, mm2			// l.21 * m.11 + l.22 * m.12 | l.21 * m.21 + l.22 * m.22
				pfacc			mm1, mm3			// l.23 * m.13 + l.24 * m.14 | l.23 * m.23 + l.24 * m.24
				pfacc			mm4, mm6			// l.21 * m.31 + l.22 * m.32 | l.21 * m.41 + l.22 * m.42
				pfacc			mm5, mm7			// l.23 * m.33 + l.24 * m.34 | l.23 * m.43 + l.24 * m.44
				pfadd			mm0, mm1			// l.21 * m.11 + l.22 * m.12 + l.23 * m.13 + l.24 * m.14 | l.21 * m.21 + l.22 * m.22 + l.23 * m.23 + l.24 * m.24
				pfadd			mm4, mm5			// l.21 * m.31 + l.22 * m.32 + l.23 * m.33 + l.24 * m.34 | l.21 * m.41 + l.22 * m.42 + l.23 * m.43 + l.24 * m.44
				movq			[eax + 16], mm0		// o.21 = l.21 * m.11 + l.22 * m.12 + l.23 * m.13 + l.24 * m.14 | o.22 = l.21 * m.21 + l.22 * m.22 + l.23 * m.23 + l.24 * m.24
				movq			[eax + 24], mm4		// o.23 = l.21 * m.31 + l.22 * m.32 + l.23 * m.33 + l.24 * m.34 | o.24 = l.21 * m.41 + l.22 * m.42 + l.21 * m.41 + l.22 * m.42

				movq			mm0, [ecx + 32]		// l.31 | l.32
				movq			mm1, [ecx + 40]		// l.33 | l.34
				movq			mm2, mm0			// l.31 | l.32
				movq			mm3, mm1			// l.33 | l.34
				movq			mm4, mm0			// l.31 | l.32
				movq			mm5, mm1			// l.33 | l.34
				pfmul			mm0, [ebx]			// l.31 * m.11 | l.32 * m.12
				pfmul			mm1, [ebx + 8]		// l.33 * m.13 | l.34 * m.14
				movq			mm6, mm2			// l.31 | l.32
				movq			mm7, mm3			// l.33 | l.34
				pfmul			mm2, [ebx + 16]		// l.31 * m.21 | l.32 * m.22
				pfmul			mm3, [ebx + 24]		// l.33 * m.23 | l.34 * m.24
				pfmul			mm4, [ebx + 32]		// l.31 * m.31 | l.32 * m.32
				pfmul			mm5, [ebx + 40]		// l.33 * m.33 | l.34 * m.34
				pfmul			mm6, [ebx + 48]		// l.31 * m.41 | l.32 * m.42
				pfmul			mm7, [ebx + 56]		// l.33 * m.43 | l.34 * m.44
				pfacc			mm0, mm2			// l.31 * m.11 + l.32 * m.12 | l.31 * m.21 + l.32 * m.22
				pfacc			mm1, mm3			// l.33 * m.13 + l.34 * m.14 | l.33 * m.23 + l.34 * m.24
				pfacc			mm4, mm6			// l.31 * m.31 + l.32 * m.32 | l.31 * m.41 + l.32 * m.42
				pfacc			mm5, mm7			// l.33 * m.33 + l.34 * m.34 | l.33 * m.43 + l.34 * m.44
				pfadd			mm0, mm1			// l.31 * m.11 + l.32 * m.12 + l.33 * m.13 + l.34 * m.14 | l.31 * m.21 + l.32 * m.22 + l.33 * m.23 + l.34 * m.24
				pfadd			mm4, mm5			// l.31 * m.31 + l.32 * m.32 + l.33 * m.33 + l.34 * m.34 | l.31 * m.41 + l.32 * m.42 + l.33 * m.43 + l.34 * m.44
				movq			[eax + 32], mm0		// o.31 = l.31 * m.11 + l.32 * m.12 + l.33 * m.13 + l.34 * m.14 | o.32 = l.31 * m.21 + l.32 * m.22 + l.33 * m.23 + l.34 * m.24
				movq			[eax + 40], mm4		// o.33 = l.31 * m.31 + l.32 * m.32 + l.33 * m.33 + l.34 * m.34 | o.34 = l.31 * m.41 + l.32 * m.42 + l.31 * m.41 + l.32 * m.42

				movq			mm0, [ecx + 48]		// l.41 | l.42
				movq			mm1, [ecx + 56]		// l.43 | l.44
				movq			mm2, mm0			// l.41 | l.42
				movq			mm3, mm1			// l.43 | l.44
				movq			mm4, mm0			// l.41 | l.42
				movq			mm5, mm1			// l.43 | l.44
				pfmul			mm0, [ebx]			// l.41 * m.11 | l.42 * m.12
				pfmul			mm1, [ebx + 8]		// l.43 * m.13 | l.44 * m.14
				movq			mm6, mm2			// l.41 | l.42
				movq			mm7, mm3			// l.43 | l.44
				pfmul			mm2, [ebx + 16]		// l.41 * m.21 | l.42 * m.22
				pfmul			mm3, [ebx + 24]		// l.43 * m.23 | l.44 * m.24
				pfmul			mm4, [ebx + 32]		// l.41 * m.31 | l.42 * m.32
				pfmul			mm5, [ebx + 40]		// l.43 * m.33 | l.44 * m.34
				pfmul			mm6, [ebx + 48]		// l.41 * m.41 | l.42 * m.42
				pfmul			mm7, [ebx + 56]		// l.43 * m.43 | l.44 * m.44
				pfacc			mm0, mm2			// l.41 * m.11 + l.42 * m.12 | l.41 * m.21 + l.42 * m.22
				pfacc			mm1, mm3			// l.43 * m.13 + l.44 * m.14 | l.43 * m.23 + l.44 * m.24
				pfacc			mm4, mm6			// l.41 * m.31 + l.42 * m.32 | l.41 * m.41 + l.42 * m.42
				pfacc			mm5, mm7			// l.43 * m.33 + l.44 * m.34 | l.43 * m.43 + l.44 * m.44
				pfadd			mm0, mm1			// l.41 * m.11 + l.42 * m.12 + l.43 * m.13 + l.44 * m.14 | l.41 * m.21 + l.42 * m.22 + l.43 * m.23 + l.44 * m.24
				pfadd			mm4, mm5			// l.41 * m.31 + l.42 * m.32 + l.43 * m.33 + l.44 * m.34 | l.41 * m.41 + l.42 * m.42 + l.43 * m.43 + l.44 * m.44
				movq			[eax + 48], mm0		// o.41 = l.41 * m.11 + l.42 * m.12 + l.43 * m.13 + l.44 * m.14 | o.42 = l.41 * m.21 + l.42 * m.22 + l.43 * m.23 + l.44 * m.24
				movq			[eax + 56], mm4		// o.43 = l.41 * m.31 + l.42 * m.32 + l.43 * m.33 + l.44 * m.34 | o.44 = l.41 * m.41 + l.42 * m.42 + l.43 * m.43 + l.44 * m.44

				femms
			}
		}

		Matrix4& Math3DNowExLib::Shadow(Matrix4& mOut, const Vector4& L, const Plane& p) const
		{
			Vector4 v(-L);
			Plane P;
			this->Normalize(P, p);
			const float d(-this->Dot(P, v));

			_asm
			{
				femms

				mov			eax, mOut
				lea			ecx, v
				lea			edx, P

				movd		mm0, [edx]
				punpckldq	mm0, mm0		// P.a | P.a
				movq		mm1, mm0
				movd		mm2, [edx + 4]
				punpckldq	mm2, mm2		// P.b | P.b
				movq		mm3, mm2
				movq		mm4, [ecx]		// L.x | L.y
				movq		mm5, [ecx + 8]	// L.z | L.w
				pxor		mm6, mm6
				pxor		mm7, mm7
				movd		mm6, d			// d | 0
				punpckldq	mm7, mm6		// 0 | d

				pfmul		mm0, mm4		// P.a * L.x | P.a * L.y
				pfmul		mm1, mm5		// P.a * L.z | P.a * L.w
				pfmul		mm2, mm4		// P.b * L.x | P.b * L.y
				pfmul		mm3, mm5		// P.b * L.z | P.b * L.w

				pfadd		mm0, mm6		// P.a * L.x + d | P.a * L.y
				pfadd		mm2, mm7		// P.b * L.x | P.b * L.y + d

				movq		[eax], mm0
				movq		[eax + 8], mm1
				movq		[eax + 16], mm2
				movq		[eax + 24], mm3

				movd		mm0, [edx + 8]
				punpckldq	mm0, mm0		// P.c | P.c
				movq		mm1, mm0
				movd		mm2, [edx + 12]
				punpckldq	mm2, mm2		// P.d | P.d
				movq		mm3, mm2

				pfmul		mm0, mm4		// P.c * L.x | P.c * L.y
				pfmul		mm1, mm5		// P.c * L.z | P.c * L.w
				pfmul		mm2, mm4		// P.d * L.x | P.d * L.y
				pfmul		mm3, mm5		// P.d * L.z | P.d * L.w

				pfadd		mm1, mm6		// P.c * L.z + d | P.c * L.w
				pfadd		mm3, mm7		// P.d * L.z | P.d * L.w + d

				movq		[eax + 32], mm0
				movq		[eax + 40], mm1
				movq		[eax + 48], mm2
				movq		[eax + 56], mm3

				femms
			}
		}

		Matrix4& Math3DNowExLib::Transpose(Matrix4& mOut, const Matrix4& m) const	
		{
			_asm
			{
				mov				eax, mOut
				mov				edx, m

				movq			mm0, [edx]			// r11 | r12
				movq			mm1, [edx + 16]		// r21 | r22
				movq			mm3, [edx + 40]		// r33 | r34
				movq			mm4, [edx + 56]		// r43 | r44
				movq			mm2, mm0			// r11 | r12
				punpckldq		mm0, mm1			// r11 | r21
				punpckhdq		mm2, mm1			// r12 | r22
				movq			mm5, mm3			// r33 | r34
				movq			[eax], mm0			// m11 = r11 | m12 = r21
				movq			[eax + 16], mm2		// m21 = r12 | m22 = r22

				movq			mm6, [edx + 8]		// r13 | r14
				punpckldq		mm3, mm4			// r33 | r43
				movq			mm7, [edx + 24]		// r23 | r24
				movq			mm0, [edx + 32]		// r31 | r32
				punpckhdq		mm5, mm4			// r34 | r44
				movq			mm1, mm6			// r13 | r14
				movq			[eax + 40], mm3		// m33 = r33 | m34 = r43
				movq			[eax + 56], mm5		// m43 = r34 | m44 = r44

  				punpckldq		mm6, mm7			// r13 | r23
				movq			mm4, [edx + 48]		// r41 | r42
				movq			mm2, mm0			// r31 | r32
				punpckhdq		mm1, mm7			// r14 | r24
				movq			[eax + 32], mm6		// m31 = r13 | m32 = r23
				punpckhdq		mm0, mm4			// r32 | r42
				punpckldq		mm2, mm4			// r31 | r41
				movq			[eax + 48], mm1		// m41 = r14 | m42 = r24
				movq			[eax + 24], mm0		// m23 = r32 | m24 = r42
				movq			[eax + 8], mm2		// m13 = r31 | m14 = r41

				femms
			}
		}


		// 四元数
		///////////////////////////////////////////////////////////////////////////////

		Quaternion& Math3DNowExLib::Multiply(Quaternion& qOut, const Quaternion& lhs,
													const Quaternion& rhs) const
		{
			_asm
			{
				mov			eax, qOut
				mov			ecx, lhs
				mov			edx, rhs

				movq		mm0, [ecx]		// l.x | l.y
				pswapd		mm1, [edx + 8]	// r.w | r.z
				pfmul		mm0, mm1		// l.x * r.w | l.y * r.z

				movq		mm1, [ecx]		// l.x | l.y
				pfmul		mm1, [edx + 8]	// l.x * r.z | l.y * r.w

				pfpnacc		mm0, mm1		// l.x * r.w - l.y * r.z | l.x * r.z + l.y * r.w

				movq		mm2, [ecx + 8]	// l.z | l.w
				pswapd		mm3, [edx]		// r.y | r.x
				pfmul		mm2, mm3		// l.z * r.y | l.w * r.x
				pfacc		mm2, mm3		// l.z * r.y + l.w * r.x | l.z * r.y + l.w * r.x

				movq		mm3, [ecx + 8]	// l.z | l.w
				pfmul		mm3, [edx]		// l.z * r.x | l.w * r.y
				pswapd		mm3, mm3		// l.w * r.y | l.z * r.x
				pfnacc		mm3, mm3		// l.w * r.y - l.z * r.x | l.w * r.y - l.z * r.x

				punpckldq	mm2, mm3		// l.z * r.y + l.w * r.x | l.w * r.y - l.z * r.x

				pfadd		mm0, mm2		// l.x * r.w - l.y * r.z + l.z * r.y + l.w * r.x | l.x * r.z + l.y * r.w - l.z * r.x + l.w * r.y


				movd		mm1, [ecx + 12]	// l.w
				punpckldq	mm1, [ecx]		// l.w | l.x
				movd		mm2, [edx + 12]	// r.w
				punpckldq	mm2, [edx]		// r.w | r.x
				pfmul		mm1, mm2		// l.w * r.w | l.x * r.x

				movq		mm3, [ecx + 4]	// l.y | l.z
				movq		mm4, [edx + 4]	// r.y | r.z
				pfmul		mm3, mm4		// l.y * r.y | l.z * r.z

				pfpnacc		mm1, mm3		// l.w * r.w - l.x * r.x | l.y * r.y + l.z * r.z

				pswapd		mm3, [ecx]		// l.y | l.x
				movq		mm4, [edx]		// r.x | r.y
				pfmul		mm3, mm4		// l.y * r.x | l.x * r.y

				movq		mm5, [ecx + 8]	// l.z | l.w
				pswapd		mm6, [edx + 8]	// r.w | r.z
				pfmul		mm5, mm6		// l.z * r.w | l.w * r.z

				pfpnacc		mm3, mm5		// l.y * r.x - l.x * r.y | l.z * r.w + l.w * r.z

				pfpnacc		mm1, mm3		// l.w * r.w - l.x * r.x - l.y * r.y - l.z * r.z | l.y * r.x - l.x * r.y + l.z * r.w + l.w * r.z
				pswapd		mm1, mm1		// l.y * r.x - l.x * r.y + l.z * r.w + l.w * r.z | l.w * r.w - l.x * r.x - l.y * r.y - l.z * r.z


				movq		[eax], mm0
				movq		[eax + 8], mm1

				femms
			}
		}

		Quaternion& Math3DNowExLib::Normalize(Quaternion& qOut, const Quaternion& rhs) const
		{
			_asm
			{
				femms

				mov			eax, qOut
				mov			ecx, rhs

				movq		mm2, [ecx]		// r.x | r.y
				movq		mm3, [ecx + 8]	// r.z | r.w
				movq		mm6, mm2
				movq		mm7, mm3
				pfmul		mm2, mm2		// r.x * r.x | r.y * r.y
				pfmul		mm3, mm3		// r.z * r.z | r.w * r.w
				pfacc		mm2, mm2		// r.x * r.x + r.y * r.y
				pfacc		mm3, mm3		// r.z * r.z + r.w * r.w
				pfadd		mm2, mm3		// r.x * r.x + r.y * r.y + r.z * r.z + r.w * r.w

				pfrsqrt		mm0, mm2
				movq		mm1, mm0
				pfmul		mm1, mm0
				pfrsqit1	mm2, mm1
				pfrcpit2	mm2, mm0

				punpckldq	mm2, mm2

				pfmul		mm6, mm2		// r.x * inv | r.y * inv
				pfmul		mm7, mm2		// r.z * inv | r.w * inv

				movq		[eax], mm6
				movq		[eax + 8], mm7

				femms
			}
		}

		Quaternion& Math3DNowExLib::RotationYawPitchRoll(Quaternion& qOut, float fYaw,
																float fPitch, float fRoll) const
		{
			const float angX(fPitch * 0.5f), angY(fYaw * 0.5f), angZ(fRoll * 0.5f);
			float _x[2], _y[2], _z[2];
			this->SinCos(angX, _x[0], _x[1]);
			this->SinCos(angY, _y[0], _y[1]);
			this->SinCos(angZ, _z[0], _z[1]);

			_asm
			{
				mov			eax, qOut
				lea			ebx, _x
				lea			ecx, _y
				lea			edx, _z

				movq		mm0, [ebx]
				movq		mm3, mm0		// sx | cx
				punpckldq	mm0, mm0		// sx | sx
				punpckhdq	mm3, mm3		// cx | cx
				movq		mm5, mm0
				movq		mm4, mm3

				movq		mm6, [ecx]		// sy | cy
				movq		mm7, [edx]		// sz | cz
				pswapd		mm1, mm6		// cy | sy
				pswapd		mm2, mm7		// cz | sz

				pfmul		mm0, mm1		// sx * cy | sx * sy
				pfmul		mm3, mm6		// cx * sy | cx * cy
				pfmul		mm0, mm2		// sx * cy * cz | sx * sy * sz
				pfmul		mm3, mm7		// cx * sy * sz | cx * cy * cz
				pfadd		mm0, mm3		// sx * cy * cz + cx * sy * sz | sx * sy * sz + cx * cy * cz

				pfmul		mm4, mm6		// cx * sy | cx * cy
				pfmul		mm5, mm1		// sx * cy | sx * sy
				pfmul		mm4, mm2		// cx * sy * cz | cx * cy * sz
				pfmul		mm5, mm7		// sx * cy * sz | sx * sy * cz
				pfsub		mm4, mm5		// cx * sy * cz - sx * cy * sz | cx * cy * sz - sx * sy * cz

				movd		[eax], mm0
				movq		[eax + 4], mm4
				punpckhdq	mm0, mm0		// sx * sy * sz + cx * cy * cz
				movd		[eax + 12], mm0

				femms
			}
		}


		// 平面
		///////////////////////////////////////////////////////////////////////////////

		Plane& Math3DNowExLib::Normalize(Plane& pOut, const Plane& rhs) const
		{
			_asm
			{
				femms

				mov			eax, pOut
				mov			ecx, rhs

				movq		mm2, [ecx]		// r.a | r.b
				movq		mm3, [ecx + 8]	// r.c | r.d
				movq		mm6, mm2
				movq		mm7, mm3
				pfmul		mm2, mm2		// r.a * r.a | r.b * r.b
				pfmul		mm3, mm3		// r.c * r.c | r.d * r.d
				pfacc		mm2, mm2		// r.a * r.a + r.b * r.b
				pfadd		mm2, mm3		// r.a * r.a + r.b * r.b + r.c * r.c

				pfrsqrt		mm0, mm2
				movq		mm1, mm0
				pfmul		mm1, mm0
				pfrsqit1	mm2, mm1
				pfrcpit2	mm2, mm0

				punpckldq	mm2, mm2

				pfmul		mm6, mm2		// r.a * inv | r.b * inv
				pfmul		mm7, mm2		// r.c * inv | r.d * inv

				movq		[eax], mm6
				movq		[eax + 8], mm7

				femms
			}
		}

		// plane * Matrix4
		Plane& Math3DNowExLib::Transform(Plane& pOut, const Plane& P, const Matrix4& mat) const
		{
			_asm
			{
				mov			eax, pOut
				mov			ecx, P
				mov			edx, mat

				movd		mm0, [ecx]
				movd		mm1, [ecx + 4]
				movd		mm2, [ecx + 8]
				movd		mm3, [ecx + 12]
				punpckldq	mm0, mm0		// P.a | P.a
				punpckldq	mm1, mm1		// P.b | P.b
				punpckldq	mm2, mm2		// P.c | P.c
				punpckldq	mm3, mm3		// P.d | P.d
				movq		mm4, mm0
				movq		mm5, mm1
				movq		mm6, mm2
				movq		mm7, mm3

				pfmul		mm0, [edx]		// P.a * m.11 | P.a * m.12
				pfmul		mm4, [edx + 8]	// P.a * m.13 | P.a * m.14
				pfmul		mm1, [edx + 16]	// P.b * m.21 | P.b * m.22
				pfmul		mm5, [edx + 24]	// P.b * m.23 | P.b * m.24
				pfmul		mm2, [edx + 32]	// P.c * m.31 | P.c * m.32
				pfmul		mm6, [edx + 40]	// P.c * m.33 | P.c * m.34
				pfmul		mm3, [edx + 48]	// P.d * m.41 | P.d * m.42
				pfmul		mm7, [edx + 56]	// P.d * m.43 | P.d * m.44

				pfadd		mm0, mm1
				pfadd		mm4, mm5
				pfadd		mm0, mm2
				pfadd		mm4, mm6
				pfadd		mm0, mm3
				pfadd		mm4, mm7

				movq		[eax], mm0
				movq		[eax + 8], mm4

				femms
			}
		}
	}
}
