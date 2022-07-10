// Camera.cpp
// KlayGE 摄像机类 实现文件
// Ver 3.10.0
// 版权所有(C) 龚敏敏, 2003-2010
// Homepage: http://www.klayge.org
//
// 3.10.0
// 支持Motion blur (2010.2.22)
// 支持Stereo (2010.4.2)
//
// 2.0.0
// 初次建立(2003.10.1)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/FrameBuffer.hpp>
#include <KlayGE/SceneManager.hpp>

#include <KlayGE/Camera.hpp>

namespace KlayGE
{
	enum CameraMode
	{
		CM_Jitter = 1UL << 0,
		CM_Omni = 1UL << 1
	};

	// 构造函数
	//////////////////////////////////////////////////////////////////////////////////
	Camera::Camera()
	{
		this->ProjParams(PI / 4, 1, 1, 1000);
	}

	SceneComponentPtr Camera::Clone() const
	{
		auto ret = MakeSharedPtr<Camera>();

		ret->look_at_dist_ = look_at_dist_;

		ret->fov_ = fov_;
		ret->aspect_ = aspect_;
		ret->near_plane_ = near_plane_;
		ret->far_plane_ = far_plane_;
		ret->proj_mat_ = proj_mat_;
		ret->inv_proj_mat_ = inv_proj_mat_;
		ret->proj_mat_wo_adjust_ = proj_mat_wo_adjust_;
		ret->inv_proj_mat_wo_adjust_ = inv_proj_mat_wo_adjust_;

		ret->prev_view_mat_ = prev_view_mat_;
		ret->prev_proj_mat_ = prev_proj_mat_;

		ret->view_proj_mat_ = view_proj_mat_;
		ret->inv_view_proj_mat_ = inv_view_proj_mat_;
		ret->view_proj_mat_dirty_ = view_proj_mat_dirty_;
		ret->view_proj_mat_wo_adjust_ = view_proj_mat_wo_adjust_;
		ret->inv_view_proj_mat_wo_adjust_ = inv_view_proj_mat_wo_adjust_;
		ret->view_proj_mat_wo_adjust_dirty_ = view_proj_mat_wo_adjust_dirty_;
		ret->camera_dirty_ = camera_dirty_;

		ret->frustum_ = frustum_;
		ret->frustum_dirty_ = frustum_dirty_;

		ret->mode_ = mode_;
		ret->cur_jitter_index_ = cur_jitter_index_;

		return ret;
	}

	float3 const& Camera::EyePos() const
	{
		float4x4 const& inv_view_mat = this->InverseViewMatrix();
		return *reinterpret_cast<float3 const *>(&inv_view_mat.Row(3));
	}

	float3 Camera::LookAt() const
	{
		return this->EyePos() + this->ForwardVec() * this->LookAtDist();
	}

	float3 const& Camera::RightVec() const
	{
		float4x4 const& inv_view_mat = this->InverseViewMatrix();
		return *reinterpret_cast<float3 const *>(&inv_view_mat.Row(0));
	}

	float3 const& Camera::UpVec() const
	{
		float4x4 const& inv_view_mat = this->InverseViewMatrix();
		return *reinterpret_cast<float3 const *>(&inv_view_mat.Row(1));
	}

	float3 const& Camera::ForwardVec() const
	{
		float4x4 const& inv_view_mat = this->InverseViewMatrix();
		return *reinterpret_cast<float3 const *>(&inv_view_mat.Row(2));
	}

	// 设置摄像机的投射矩阵
	//////////////////////////////////////////////////////////////////////////////////
	void Camera::ProjParams(float fov, float aspect, float near_plane, float far_plane)
	{
		fov_		= fov;
		aspect_		= aspect;
		near_plane_	= near_plane;
		far_plane_	= far_plane;

		proj_mat_ = MathLib::perspective_fov_lh(fov, aspect, near_plane, far_plane);
		proj_mat_wo_adjust_ = proj_mat_;
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		re.AdjustProjectionMatrix(proj_mat_);
		inv_proj_mat_ = MathLib::inverse(proj_mat_);
		inv_proj_mat_wo_adjust_ = MathLib::inverse(proj_mat_wo_adjust_);
		view_proj_mat_dirty_ = true;
		view_proj_mat_wo_adjust_dirty_ = true;
		camera_dirty_ = true;
		frustum_dirty_ = true;
	}

	void Camera::ProjOrthoParams(float w, float h, float near_plane, float far_plane)
	{
		fov_		= 0;
		aspect_		= w / h;
		near_plane_	= near_plane;
		far_plane_	= far_plane;

		proj_mat_ = MathLib::ortho_lh(w, h, near_plane, far_plane);
		proj_mat_wo_adjust_ = proj_mat_;
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		re.AdjustProjectionMatrix(proj_mat_);
		inv_proj_mat_ = MathLib::inverse(proj_mat_);
		inv_proj_mat_wo_adjust_ = MathLib::inverse(proj_mat_wo_adjust_);
		view_proj_mat_dirty_ = true;
		view_proj_mat_wo_adjust_dirty_ = true;
		camera_dirty_ = true;
		frustum_dirty_ = true;
	}

	void Camera::ProjOrthoOffCenterParams(float left, float top, float right, float bottom, float near_plane, float far_plane)
	{
		fov_ = 0;
		aspect_ = (right - left) / (top - bottom);
		near_plane_ = near_plane;
		far_plane_ = far_plane;

		proj_mat_ = MathLib::ortho_off_center_lh(left, right, bottom, top, near_plane, far_plane);
		proj_mat_wo_adjust_ = proj_mat_;
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		re.AdjustProjectionMatrix(proj_mat_);
		inv_proj_mat_ = MathLib::inverse(proj_mat_);
		inv_proj_mat_wo_adjust_ = MathLib::inverse(proj_mat_wo_adjust_);
		view_proj_mat_dirty_ = true;
		view_proj_mat_wo_adjust_dirty_ = true;
		camera_dirty_ = true;
		frustum_dirty_ = true;
	}

	void Camera::MainThreadUpdate(float app_time, float elapsed_time)
	{
		prev_view_mat_ = this->ViewMatrix();
		prev_proj_mat_ = proj_mat_;

		SceneComponent::MainThreadUpdate(app_time, elapsed_time);

		if (this->JitterMode())
		{
			cur_jitter_index_ = (cur_jitter_index_ + 1) & 1;

			float top = near_plane_ * tan(fov_ / 2);
			float bottom = -top;
			float right = top * aspect_;
			float left = -right;
			float width = right - left;
			float height = top - bottom;

			RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
			int win_width = re.CurFrameBuffer()->Width();
			int win_height = re.CurFrameBuffer()->Height();

			float const pixel_dx[2] = { -0.25f, +0.25f };
			float const pixel_dy[2] = { +0.25f, -0.25f };
			float dx = -pixel_dx[cur_jitter_index_] * width / win_width;
			float dy = -pixel_dy[cur_jitter_index_] * height / win_height;
			proj_mat_ = MathLib::perspective_off_center_lh(left + dx, right + dx, bottom + dy, top + dy,
								near_plane_, far_plane_);
			proj_mat_wo_adjust_ = proj_mat_;
			re.AdjustProjectionMatrix(proj_mat_);
			inv_proj_mat_ = MathLib::inverse(proj_mat_);
			inv_proj_mat_wo_adjust_ = MathLib::inverse(proj_mat_wo_adjust_);
		}

		this->DirtyTransforms();
	}

	void Camera::DirtyTransforms()
	{
		view_proj_mat_dirty_ = true;
		view_proj_mat_wo_adjust_dirty_ = true;
		camera_dirty_ = true;
		frustum_dirty_ = true;
	}

	float4x4 const & Camera::ViewMatrix() const
	{
		return this->BoundSceneNode()->InverseTransformToWorld();
	}

	float4x4 const & Camera::ProjMatrix() const
	{
		return proj_mat_;
	}

	float4x4 const & Camera::ProjMatrixWOAdjust() const
	{
		return proj_mat_wo_adjust_;
	}

	float4x4 const & Camera::ViewProjMatrix() const
	{
		if (view_proj_mat_dirty_)
		{
			view_proj_mat_ = this->ViewMatrix() * this->ProjMatrix();
			inv_view_proj_mat_ = this->InverseProjMatrix() * this->InverseViewMatrix();
			view_proj_mat_dirty_ = false;
		}
		return view_proj_mat_;
	}

	float4x4 const & Camera::ViewProjMatrixWOAdjust() const
	{
		if (view_proj_mat_wo_adjust_dirty_)
		{
			view_proj_mat_wo_adjust_ = this->ViewMatrix() * this->ProjMatrixWOAdjust();
			inv_view_proj_mat_wo_adjust_ = this->InverseProjMatrixWOAdjust() * this->InverseViewMatrix();
			view_proj_mat_wo_adjust_dirty_ = false;
		}
		return view_proj_mat_wo_adjust_;
	}
	
	float4x4 const & Camera::InverseViewMatrix() const
	{
		return this->BoundSceneNode()->TransformToWorld();
	}

	float4x4 const & Camera::InverseProjMatrix() const
	{
		return inv_proj_mat_;
	}

	float4x4 const & Camera::InverseProjMatrixWOAdjust() const
	{
		return inv_proj_mat_wo_adjust_;
	}
	
	float4x4 const & Camera::InverseViewProjMatrix() const
	{
		if (view_proj_mat_dirty_)
		{
			view_proj_mat_ = this->ViewMatrix() * this->ProjMatrix();
			inv_view_proj_mat_ = this->InverseProjMatrix() * this->InverseViewMatrix();
			view_proj_mat_dirty_ = false;
		}
		return inv_view_proj_mat_;
	}

	float4x4 const & Camera::InverseViewProjMatrixWOAdjust() const
	{
		if (view_proj_mat_wo_adjust_dirty_)
		{
			view_proj_mat_wo_adjust_ = this->ViewMatrix() * this->ProjMatrixWOAdjust();
			inv_view_proj_mat_wo_adjust_ = this->InverseProjMatrixWOAdjust() * this->InverseViewMatrix();
			view_proj_mat_wo_adjust_dirty_ = false;
		}
		return inv_view_proj_mat_wo_adjust_;
	}

	float4x4 const & Camera::PrevViewMatrix() const
	{
		return prev_view_mat_;
	}
	
	float4x4 const & Camera::PrevProjMatrix() const
	{
		return prev_proj_mat_;
	}

	float4 Camera::NearQFarParam() const
	{
		float const q = far_plane_ / (far_plane_ - near_plane_);
		return float4(near_plane_ * q, q, far_plane_, 1 / far_plane_);
	}

	Frustum const & Camera::ViewFrustum() const
	{
		if (frustum_dirty_)
		{
			frustum_.ClipMatrix(this->ViewProjMatrixWOAdjust(), this->InverseViewProjMatrixWOAdjust());
			frustum_dirty_ = false;
		}
		return frustum_;
	}

	bool Camera::OmniDirectionalMode() const
	{
		return (mode_ & CM_Omni) > 0;
	}

	void Camera::OmniDirectionalMode(bool omni)
	{
		if (omni)
		{
			mode_ |= CM_Omni;
		}
		else
		{
			mode_ &= ~CM_Omni;
		}
	}

	bool Camera::JitterMode() const
	{
		return (mode_ & CM_Jitter) > 0;
	}

	void Camera::JitterMode(bool jitter)
	{
		if (jitter)
		{
			mode_ |= CM_Jitter;
		}
		else
		{
			mode_ &= ~CM_Jitter;
		}
	}

	void Camera::Active(RenderEffectConstantBuffer& camera_cbuffer, uint32_t index, float4x4 const& model_mat,
		float4x4 const& inv_model_mat, float4x4 const& prev_model_mat, bool model_mat_dirty, float4x4 const& cascade_crop_mat,
		bool need_cascade_crop_mat) const
	{
		if (model_mat_dirty || camera_dirty_)
		{
			RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
			auto const& pccb = re.PredefinedCameraCBufferInstance();

			float4x4 mvp = model_mat * this->ViewProjMatrix();
			float4x4 prev_mvp = prev_model_mat * prev_view_mat_ * prev_proj_mat_;
			if (need_cascade_crop_mat)
			{
				mvp *= cascade_crop_mat;
				prev_mvp *= cascade_crop_mat;
			}

			auto& camera_info = pccb.Camera(camera_cbuffer, index);
			camera_info.model_view = MathLib::transpose(model_mat * this->ViewMatrix());
			camera_info.mvp = MathLib::transpose(mvp);
			camera_info.inv_mv = MathLib::transpose(this->InverseViewMatrix() * inv_model_mat);
			camera_info.inv_mvp = MathLib::transpose(this->InverseViewProjMatrix() * inv_model_mat);
			camera_info.eye_pos = this->EyePos();
			camera_info.forward_vec = this->ForwardVec();
			camera_info.up_vec = this->UpVec();

			pccb.PrevMvp(camera_cbuffer, index) = MathLib::transpose(prev_mvp);

			camera_cbuffer.Dirty(true);
		}
	}
}
