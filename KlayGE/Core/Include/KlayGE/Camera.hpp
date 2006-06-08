// Camera.hpp
// KlayGE 摄像机类 头文件
// Ver 2.0.0
// 版权所有(C) 龚敏敏, 2003
// Homepage: http://klayge.sourceforge.net
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

#ifdef KLAYGE_DEBUG
	#pragma comment(lib, "KlayGE_Core_d.lib")
#else
	#pragma comment(lib, "KlayGE_Core.lib")
#endif

namespace KlayGE
{
	// 3D摄像机操作
	//////////////////////////////////////////////////////////////////////////////////
	class Camera
	{
	public:
		float3 const & EyePos() const
			{ return eyePos_; }
		float3 const & LookAt() const
			{ return lookat_; }
		float3 const & UpVec() const
			{ return upVec_; }
		float3 const & ViewVec() const
			{ return viewVec_; }

		float FOV() const
			{ return FOV_; }
		float Aspect() const
			{ return aspect_; }
		float NearPlane() const
			{ return nearPlane_; }
		float FarPlane() const
			{ return farPlane_; }

		float4x4 const & ViewMatrix() const
			{ return this->viewMat_; }
		float4x4 const & BillboardMatrix();
		float4x4 const & ProjMatrix() const
			{ return this->projMat_; }

		void ViewParams(float3 const & eyePos, float3 const & lookat,
			float3 const & upVec = float3(0, 1, 0));
		void ProjParams(float FOV, float aspect, float nearPlane, float farPlane);

		Camera();

	private:
		float3		eyePos_;			// 观察矩阵的属性
		float3		lookat_;
		float3		upVec_;
		float3		viewVec_;
		float4x4	viewMat_;

		bool		reEvalBillboard_;
		float4x4	billboardMat_;

		float		FOV_;			// 投射矩阵的属性
		float		aspect_;
		float		nearPlane_;
		float		farPlane_;
		float4x4	projMat_;
	};
}

#endif		// _CAMERA_HPP
