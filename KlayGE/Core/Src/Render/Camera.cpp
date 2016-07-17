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
		: view_proj_mat_dirty_(true), view_proj_mat_wo_adjust_dirty_(true), frustum_dirty_(true),
			mode_(0), cur_jitter_index_(0)
	{
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		uint32_t num_motion_frames = re.NumMotionFrames();
		prev_view_mats_.resize(num_motion_frames);
		prev_proj_mats_.resize(num_motion_frames);

		this->ViewParams(float3(0, 0, 0), float3(0, 0, 1), float3(0, 1, 0));
		this->ProjParams(PI / 4, 1, 1, 1000);
	}

	// 设置摄像机的观察矩阵
	//////////////////////////////////////////////////////////////////////////////////
	void Camera::ViewParams(float3 const & eye_pos, float3 const & look_at)
	{
		this->ViewParams(eye_pos, look_at, float3(0, 1, 0));
	}

	void Camera::ViewParams(float3 const & eye_pos, float3 const & look_at,
										float3 const & up_vec)
	{
		look_at_dist_ = MathLib::length(look_at - eye_pos);

		view_mat_ = MathLib::look_at_lh(eye_pos, look_at, up_vec);
		inv_view_mat_ = MathLib::inverse(view_mat_);
		view_proj_mat_dirty_ = true;
		view_proj_mat_wo_adjust_dirty_ = true;
		frustum_dirty_ = true;
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
		frustum_dirty_ = true;
	}

	void Camera::BindUpdateFunc(std::function<void(Camera&, float, float)> const & update_func)
	{
		update_func_ = update_func;
	}

	void Camera::Update(float app_time, float elapsed_time)
	{
		if (update_func_)
		{
			update_func_(*this, app_time, elapsed_time);
		}

		prev_view_mats_.push_back(view_mat_);
		prev_proj_mats_.push_back(proj_mat_);

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
			view_proj_mat_dirty_ = true;
			view_proj_mat_wo_adjust_dirty_ = true;
			frustum_dirty_ = true;
		}
	}

	void Camera::AddToSceneManager()
	{
		Context::Instance().SceneManagerInstance().AddCamera(this->shared_from_this());
	}

	void Camera::DelFromSceneManager()
	{
		Context::Instance().SceneManagerInstance().DelCamera(this->shared_from_this());
	}

	float4x4 const & Camera::ViewMatrix() const
	{
		return view_mat_;
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
			view_proj_mat_ = view_mat_ * proj_mat_;
			inv_view_proj_mat_ = inv_proj_mat_ * inv_view_mat_;
			view_proj_mat_dirty_ = false;
		}
		return view_proj_mat_;
	}

	float4x4 const & Camera::ViewProjMatrixWOAdjust() const
	{
		if (view_proj_mat_wo_adjust_dirty_)
		{
			view_proj_mat_wo_adjust_ = view_mat_ * proj_mat_wo_adjust_;
			inv_view_proj_mat_wo_adjust_ = inv_proj_mat_wo_adjust_ * inv_view_mat_;
			view_proj_mat_wo_adjust_dirty_ = false;
		}
		return view_proj_mat_wo_adjust_;
	}
	
	float4x4 const & Camera::InverseViewMatrix() const
	{
		return inv_view_mat_;
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
			view_proj_mat_ = view_mat_ * proj_mat_;
			inv_view_proj_mat_ = inv_proj_mat_ * inv_view_mat_;
			view_proj_mat_dirty_ = false;
		}
		return inv_view_proj_mat_;
	}

	float4x4 const & Camera::InverseViewProjMatrixWOAdjust() const
	{
		if (view_proj_mat_wo_adjust_dirty_)
		{
			view_proj_mat_wo_adjust_ = view_mat_ * proj_mat_wo_adjust_;
			inv_view_proj_mat_wo_adjust_ = inv_proj_mat_wo_adjust_ * inv_view_mat_;
			view_proj_mat_wo_adjust_dirty_ = false;
		}
		return inv_view_proj_mat_wo_adjust_;
	}

	float4x4 const & Camera::PrevViewMatrix() const
	{
		return prev_view_mats_.front();
	}
	
	float4x4 const & Camera::PrevProjMatrix() const
	{
		return prev_proj_mats_.front();
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
}
