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

#include <boost/circular_buffer.hpp>

#include <KFL/Frustum.hpp>
#include <KFL/Vector.hpp>
#include <KFL/Matrix.hpp>

namespace KlayGE
{
	// 3D摄像机操作
	//////////////////////////////////////////////////////////////////////////////////
	class KLAYGE_CORE_API Camera : boost::noncopyable, public std::enable_shared_from_this<Camera>
	{
	public:
		Camera();

		float3 const & EyePos() const
			{ return *reinterpret_cast<float3 const *>(&inv_view_mat_.Row(3)); }
		float3 LookAt() const
			{ return this->EyePos() + this->ForwardVec() * look_at_dist_; }
		float3 const & RightVec() const
			{ return *reinterpret_cast<float3 const *>(&inv_view_mat_.Row(0)); }
		float3 const & UpVec() const
			{ return *reinterpret_cast<float3 const *>(&inv_view_mat_.Row(1)); }
		float3 const & ForwardVec() const
			{ return *reinterpret_cast<float3 const *>(&inv_view_mat_.Row(2)); }
		float LookAtDist() const
		{
			return look_at_dist_;
		}

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
		void ProjOrthoParams(float w, float h, float near_plane, float far_plane);
		void ProjOrthoOffCenterParams(float left, float top, float right, float bottom, float near_plane, float far_plane);

		void BindUpdateFunc(std::function<void(Camera&, float, float)> const & update_func);

		void Update(float app_time, float elapsed_time);

		void AddToSceneManager();
		void DelFromSceneManager();

		float4x4 const & ViewMatrix() const;
		float4x4 const & ProjMatrix() const;
		float4x4 const & ProjMatrixWOAdjust() const;
		float4x4 const & ViewProjMatrix() const;
		float4x4 const & ViewProjMatrixWOAdjust() const;
		float4x4 const & InverseViewMatrix() const;
		float4x4 const & InverseProjMatrix() const;
		float4x4 const & InverseProjMatrixWOAdjust() const;
		float4x4 const & InverseViewProjMatrix() const;
		float4x4 const & InverseViewProjMatrixWOAdjust() const;
		float4x4 const & PrevViewMatrix() const;
		float4x4 const & PrevProjMatrix() const;

		Frustum const & ViewFrustum() const;

		bool OmniDirectionalMode() const;
		void OmniDirectionalMode(bool omni);
		bool JitterMode() const;
		void JitterMode(bool jitter);

	private:
		float		look_at_dist_;
		float4x4	view_mat_;
		float4x4	inv_view_mat_;

		float		fov_;
		float		aspect_;
		float		near_plane_;
		float		far_plane_;
		float4x4	proj_mat_;
		float4x4	inv_proj_mat_;
		float4x4	proj_mat_wo_adjust_;
		float4x4	inv_proj_mat_wo_adjust_;

		boost::circular_buffer<float4x4> prev_view_mats_;
		boost::circular_buffer<float4x4> prev_proj_mats_;

		mutable float4x4	view_proj_mat_;
		mutable float4x4	inv_view_proj_mat_;
		mutable bool		view_proj_mat_dirty_;
		mutable float4x4	view_proj_mat_wo_adjust_;
		mutable float4x4	inv_view_proj_mat_wo_adjust_;
		mutable bool		view_proj_mat_wo_adjust_dirty_;

		mutable Frustum	frustum_;
		mutable bool	frustum_dirty_;

		uint32_t	mode_;
		int cur_jitter_index_;

		std::function<void(Camera&, float, float)> update_func_;
	};
}

#endif		// _CAMERA_HPP
