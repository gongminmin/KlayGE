// Camera.hpp
// KlayGE 摄像机类 头文件
// Ver 2.0.0
// 版权所有(C) 龚敏敏, 2003
// Homepage: http://www.enginedev.com
//
// 2.0.0
// 初次建立 (2003.5.31)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#ifndef _CAMERA_HPP
#define _CAMERA_HPP

#include <KlayGE/Vector.hpp>
#include <KlayGE/Matrix.hpp>

#pragma comment(lib, "KlayGE_Core.lib")

namespace KlayGE
{
	// 3D摄像机操作
	//////////////////////////////////////////////////////////////////////////////////
	class Camera
	{
	public:
		const Vector3& EyePos() const
			{ return eyePos_; }
		const Vector3& LookAt() const
			{ return lookat_; }
		const Vector3& UpVec() const
			{ return upVec_; }

		float FOV() const
			{ return FOV_; }
		float Aspect() const
			{ return aspect_; }
		float NearPlane() const
			{ return nearPlane_; }
		float FarPlane() const
			{ return farPlane_; }

		const Matrix4& ViewMatrix() const
			{ return this->viewMat_; }
		const Matrix4& BillboardMatrix();
		const Matrix4& ProjMatrix() const
			{ return this->projMat_; }

		void ViewParams(const Vector3& eyePos, const Vector3& lookat,
			const Vector3& upVec = Vector3(0, 1, 0));
		void ProjParams(float FOV, float aspect, float nearPlane, float farPlane);

		Camera();

	private:
		Vector3		eyePos_;			// 观察矩阵的属性
		Vector3		lookat_;
		Vector3		upVec_;
		Vector3		viewVec_;
		Matrix4		viewMat_;

		bool		reEvalBillboard_;
		Matrix4		billboardMat_;

		float		FOV_;			// 投射矩阵的属性
		float		aspect_;
		float		nearPlane_;
		float		farPlane_;
		Matrix4		projMat_;
	};
}

#endif		// _CAMERA_HPP
