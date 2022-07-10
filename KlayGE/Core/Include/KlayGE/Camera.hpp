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

#include <KFL/Frustum.hpp>
#include <KFL/Vector.hpp>
#include <KFL/Matrix.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/SceneComponent.hpp>

namespace KlayGE
{
	// 3D摄像机操作
	//////////////////////////////////////////////////////////////////////////////////
	class KLAYGE_CORE_API Camera final : public SceneComponent, public std::enable_shared_from_this<Camera>
	{
	public:
		Camera();

		SceneComponentPtr Clone() const override;

		float3 const& EyePos() const;
		float3 LookAt() const;
		float3 const& RightVec() const;
		float3 const& UpVec() const;
		float3 const& ForwardVec() const;
		float LookAtDist() const
		{
			return look_at_dist_;
		}
		void LookAtDist(float look_at_dist)
		{
			look_at_dist_ = look_at_dist;
		}

		float FOV() const
			{ return fov_; }
		float Aspect() const
			{ return aspect_; }
		float NearPlane() const
			{ return near_plane_; }
		float FarPlane() const
			{ return far_plane_; }

		void ProjParams(float fov, float aspect, float near_plane, float far_plane);
		void ProjOrthoParams(float w, float h, float near_plane, float far_plane);
		void ProjOrthoOffCenterParams(float left, float top, float right, float bottom, float near_plane, float far_plane);

		void MainThreadUpdate(float app_time, float elapsed_time) override;

		void DirtyTransforms();

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
		float4 NearQFarParam() const;

		Frustum const & ViewFrustum() const;

		bool OmniDirectionalMode() const;
		void OmniDirectionalMode(bool omni);
		bool JitterMode() const;
		void JitterMode(bool jitter);

		void Active(RenderEffectConstantBuffer& camera_cbuffer, uint32_t index, float4x4 const& model_mat, float4x4 const& inv_model_mat,
			float4x4 const& prev_model_mat, bool model_mat_dirty, float4x4 const& cascade_crop_mat, bool need_cascade_crop_mat) const;

	private:
		float		look_at_dist_ = 1;

		float		fov_;
		float		aspect_;
		float		near_plane_;
		float		far_plane_;
		float4x4	proj_mat_;
		float4x4	inv_proj_mat_;
		float4x4	proj_mat_wo_adjust_;
		float4x4	inv_proj_mat_wo_adjust_;

		float4x4 prev_view_mat_;
		float4x4 prev_proj_mat_;

		mutable float4x4	view_proj_mat_;
		mutable float4x4	inv_view_proj_mat_;
		mutable bool		view_proj_mat_dirty_ = true;
		mutable float4x4	view_proj_mat_wo_adjust_;
		mutable float4x4	inv_view_proj_mat_wo_adjust_;
		mutable bool		view_proj_mat_wo_adjust_dirty_ = true;
		mutable bool		camera_dirty_ = true;

		mutable Frustum	frustum_;
		mutable bool	frustum_dirty_ = true;

		uint32_t	mode_ = 0;
		int cur_jitter_index_ = 0;
	};
}

#endif		// _CAMERA_HPP
