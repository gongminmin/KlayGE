// Math.hpp
// KlayGE 数学函数库 实现文件
// Ver 1.4.8.5
// 版权所有(C) 龚敏敏, 2003
// Homepage: http://www.enginedev.com
//
// 1.3.8.2
// 初次建立 (2003.1.2)
//
// 1.3.8.3
// 优化了结构 (2003.1.7)
//
// 1.4.8.5
// 增加了MathInterface (2003.4.20)
//
// 修改记录
///////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>

#include <KlayGE/Math.hpp>
#include "MathImpl.hpp"

namespace KlayGE
{
	MathLib* MathLib::Create(CPUOptimal cpu)
	{
		switch (cpu)
		{
		case CPU_Standard:
		case CPU_MMX:
			return new KlayGEImpl::MathStandardLib;

		case CPU_AMD3DNowEx:
			return new KlayGEImpl::Math3DNowExLib;
		}

		return NULL;
	}

	Matrix4& MathLib::Scaling(Matrix4& out, const Vector3& vPos) const
	{
		return this->Scaling(out, vPos.x(), vPos.y(), vPos.z());
	}

	Matrix4& MathLib::Translation(Matrix4& out, const Vector3& vPos) const
	{
		return this->Translation(out, vPos.x(), vPos.y(), vPos.z());
	}

	Matrix4& MathLib::LookAtLH(Matrix4& out, const Vector3& vEye, const Vector3& vAt) const
	{
		return this->LookAtLH(out, vEye, vAt, MakeVector(0.0f, 1.0f, 0.0f));
	}

	Matrix4& MathLib::LookAtRH(Matrix4& out, const Vector3& vEye, const Vector3& vAt) const
	{
		return this->LookAtRH(out, vEye, vAt, MakeVector(0.0f, 1.0f, 0.0f));
	}

	Matrix4& MathLib::OrthoRH(Matrix4& out, float w, float h, float fNear, float fFar) const
	{
		this->OrthoLH(out, w, h, fNear, fFar);
		return this->LHToRH(out, out);
	}

	Matrix4& MathLib::OrthoOffCenterRH(Matrix4& out, float l, float r, float b, float t, 
													float fNear, float fFar) const
	{
		this->OrthoOffCenterLH(out, l, r, b, t, fNear, fFar);
		return this->LHToRH(out, out);
	}

	Matrix4& MathLib::PerspectiveRH(Matrix4& out, float w, float h, float fNear, float fFar) const
	{
		this->PerspectiveLH(out, w, h, fNear, fFar);
		return this->LHToRH(out, out);
	}

	Matrix4& MathLib::PerspectiveFovRH(Matrix4& out, float fFOV, float fAspect, float fNear, float fFar) const
	{
		this->PerspectiveFovLH(out, fFOV, fAspect, fNear, fFar);
		return this->LHToRH(out, out);
	}

	Matrix4& MathLib::PerspectiveOffCenterRH(Matrix4& out, float l, float r, float b, float t, 
													float fNear, float fFar) const
	{
		this->PerspectiveOffCenterLH(out, l, r, b, t, fNear, fFar);
		return this->LHToRH(out, out);
	}

	Matrix4& MathLib::RHToLH(Matrix4& out, const Matrix4& rhs) const
	{
		return this->LHToRH(out, rhs);
	}

	Quaternion& MathLib::RotationYawPitchRoll(Quaternion& out, const Vector3& vAng) const
	{
		return this->RotationYawPitchRoll(out, vAng.x(), vAng.y(), vAng.z());
	}
}
