// Camera.cpp
// KlayGE 摄像机类 实现文件
// Ver 2.0.0
// 版权所有(C) 龚敏敏, 2003
// Homepage: http://www.enginedev.com
//
// 2.0.0
// 初次建立(2003.10.1)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Math.hpp>

#include <KlayGE/Camera.hpp>

namespace KlayGE
{
	// 构造函数
	//////////////////////////////////////////////////////////////////////////////////
	Camera::Camera()
	{
		// 设置观察矩阵的参数
		this->ViewParams(MakeVector(0.0f, 0.0f, 0.0f), MakeVector(0.0f, 0.0f, 1.0f), MakeVector(0.0f, 1.0f, 0.0f));

		// 设置投射矩阵的参数
		this->ProjParams(PI / 4, 1, 1, 1000);
	}

	// 设置摄像机的观察矩阵
	//////////////////////////////////////////////////////////////////////////////////
	void Camera::ViewParams(const Vector3& eyePt, const Vector3& lookatPt,
										const Vector3& upVec)
	{
		// 设置观察矩阵的参数
		eyePt_		= eyePt;
		lookatPt_	= lookatPt;
		upVec_		= upVec;
		
		MathLib::Normalize(viewVec_, lookatPt_ - eyePt_);
		MathLib::LookAtLH(viewMat_, eyePt_, lookatPt_, upVec);

		reEvalBillboard_ = true;
	}

	// 设置摄像机的投射矩阵
	//////////////////////////////////////////////////////////////////////////////////
	void Camera::ProjParams(float FOV, float aspect, float nearPlane,
											float farPlane)
	{
		// 设置投射矩阵的参数
		FOV_		= FOV;
		aspect_		= aspect;
		nearPlane_	= nearPlane;
		farPlane_	= farPlane;

		MathLib::PerspectiveFovLH(projMat_, FOV, aspect, nearPlane, farPlane);
	}

	// 公告牌技术所需要的矩阵
	//////////////////////////////////////////////////////////////////////////////////
	const Matrix4& Camera::BillboardMatrix()
	{
		if (reEvalBillboard_)
		{
			MathLib::Inverse(billboardMat_, viewMat_);

			billboardMat_(3, 0) = 0;
			billboardMat_(3, 1) = 0;
			billboardMat_(3, 2) = 0;
		}

		return billboardMat_;
	}
}
