// Camera.hpp
// KlayGE 摄像机类 头文件
// Ver 3.10.0
// 版权所有(C) 龚敏敏, 2003-2010
// Homepage: http://www.klayge.org
//
// 3.10.0
// 支持Motion blur (2010.2.22)
// 支持Stereo (2010.4.2)
//
// 2.0.0
// 初次建立 (2003.5.31)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#ifndef _CAMERA_HPP
#define _CAMERA_HPP

#pragma once

#ifndef KLAYGE_CORE_SOURCE
#define KLAYGE_LIB_NAME KlayGE_Core
#include <KlayGE/config/auto_link.hpp>
#endif

#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable: 6011)
#endif
#include <boost/circular_buffer.hpp>
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(pop)
#endif
#include <boost/function.hpp>

#include <KlayGE/Frustum.hpp>
#include <KlayGE/Vector.hpp>
#include <KlayGE/Matrix.hpp>

namespace KlayGE
{
	// 3D摄像机操作
	//////////////////////////////////////////////////////////////////////////////////
	class KLAYGE_CORE_API Camera
	{
	public:
		Camera();

		float3 const & EyePos() const
			{ return eye_pos_; }
		float3 const & LookAt() const
			{ return look_at_; }
		float3 const & UpVec() const
			{ return up_vec_; }
		float3 const & ViewVec() const
			{ return view_vec_; }

		float FOV() const
			{ return fov_; }
		float Aspect() const
			{ return aspect_; }
		float NearPlane() const
			{ return near_plane_; }
		float FarPlane() const
			{ return far_plane_; }

		void ViewParams(float3 const & eye_pos, float3 const & look_at);
		void ViewParams(float3 const & eye_pos, float3 const & look_at, float3 const & up_vec);
		void ProjParams(float fov, float aspect, float near_plane, float far_plane);

		void BindUpdateFunc(boost::function<void(Camera&, float, float)> const & update_func);

		void Update(float app_time, float elapsed_time);

		float4x4 const & ViewMatrix() const;
		float4x4 const & ProjMatrix() const;
		float4x4 const & PrevViewMatrix() const;
		float4x4 const & PrevProjMatrix() const;

		Frustum const & ViewFrustum() const;

		bool OmniDirectionalMode() const;
		void OmniDirectionalMode(bool omni);

	private:
		float3		eye_pos_;		// 观察矩阵的属性
		float3		look_at_;
		float3		up_vec_;
		float3		view_vec_;
		float4x4	view_mat_;

		float		fov_;			// 投射矩阵的属性
		float		aspect_;
		float		near_plane_;
		float		far_plane_;
		float4x4	proj_mat_;

		boost::circular_buffer<float4x4> prev_view_mats_;
		boost::circular_buffer<float4x4> prev_proj_mats_;

		mutable Frustum	frustum_;
		mutable bool	frustum_dirty_;

		uint32_t	mode_;

		boost::function<void(Camera&, float, float)> update_func_;
	};
}

#endif		// _CAMERA_HPP
