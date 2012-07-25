// Light.cpp
// KlayGE Light implement file
// Ver 3.12.0
// Copyright(C) Minmin Gong, 2011
// Homepage: http://www.klayge.org
//
// 3.12.0
// First release (2011.1.12)
//
// CHANGE LIST
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/SceneManager.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/FrameBuffer.hpp>
#include <KlayGE/Query.hpp>
#include <KlayGE/Camera.hpp>

#include <KlayGE/Light.hpp>

namespace KlayGE
{
	LightSource::LightSource(LightType type)
		: type_(type), attrib_(0), enabled_(true),
			color_(0, 0, 0, 0)
	{
	}

	LightSource::~LightSource()
	{
	}

	LightType LightSource::Type() const
	{
		return type_;
	}

	int32_t LightSource::Attrib() const
	{
		return attrib_;
	}

	void LightSource::Attrib(int32_t attrib)
	{
		attrib_ = attrib;
	}

	bool LightSource::Enabled() const
	{
		return enabled_;
	}

	void LightSource::Enabled(bool enabled)
	{
		enabled_ = enabled;
	}

	void LightSource::BindUpdateFunc(boost::function<void(LightSource&, float, float)> const & update_func)
	{
		update_func_ = update_func;
	}

	void LightSource::Update(float app_time, float elapsed_time)
	{
		if (update_func_)
		{
			update_func_(*this, app_time, elapsed_time);
		}
	}

	void LightSource::AddToSceneManager()
	{
		Context::Instance().SceneManagerInstance().AddLight(this->shared_from_this());
	}

	void LightSource::DelFromSceneManager()
	{
		Context::Instance().SceneManagerInstance().DelLight(this->shared_from_this());
	}

	float4 const & LightSource::Color() const
	{
		return color_;
	}

	void LightSource::Color(float3 const & clr)
	{
		color_ = float4(clr.x(), clr.y(), clr.z(),
			MathLib::dot(clr, float3(0.2126f, 0.7152f, 0.0722f)));
	}

	float3 const & LightSource::Position() const
	{
		return float3::Zero();
	}

	void LightSource::Position(float3 const & /*pos*/)
	{
	}

	float3 LightSource::Direction() const
	{
		return float3::Zero();
	}
	
	void LightSource::Direction(float3 const & /*dir*/)
	{
	}

	Quaternion const & LightSource::Rotation() const
	{
		return Quaternion::Identity();
	}

	void LightSource::Rotation(Quaternion const & /*quat*/)
	{
	}

	void LightSource::ModelMatrix(float4x4 const & /*model*/)
	{
	}

	float3 const & LightSource::Falloff() const
	{
		return float3::Zero();
	}

	void LightSource::Falloff(float3 const & /*fall_off*/)
	{
	}

	float LightSource::CosInnerAngle() const
	{
		return 0;
	}

	void LightSource::InnerAngle(float /*angle*/)
	{
	}

	float LightSource::CosOuterAngle() const
	{
		return 0;
	}

	float4 const & LightSource::CosOuterInner() const
	{
		return float4::Zero();
	}

	void LightSource::OuterAngle(float /*angle*/)
	{
	}

	TexturePtr const & LightSource::ProjectiveTexture() const
	{
		static const TexturePtr ret;
		return ret;
	}

	void LightSource::ProjectiveTexture(TexturePtr const & /*tex*/)
	{
	}

	ConditionalRenderPtr const & LightSource::ConditionalRenderQuery(uint32_t /*index*/) const
	{
		static const ConditionalRenderPtr ret;
		return ret;
	}

	CameraPtr const & LightSource::SMCamera(uint32_t /*index*/) const
	{
		static const CameraPtr ret;
		return ret;
	}


	AmbientLightSource::AmbientLightSource()
		: LightSource(LT_Ambient)
	{
		attrib_ = LSA_NoShadow;
		color_ = float4(0, 0, 0, 0);
	}

	AmbientLightSource::~AmbientLightSource()
	{
	}

	void AmbientLightSource::Attrib(int32_t attrib)
	{
		LightSource::Attrib(attrib);

		// Disable shadow and GI
		attrib_ |= LSA_NoShadow;
		attrib_ &= ~LSA_IndirectLighting;
	}


	PointLightSource::PointLightSource()
		: LightSource(LT_Point),
			quat_(Quaternion::Identity()),
			pos_(float3::Zero())
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		for (int i = 0; i < 7; ++ i)
		{
			crs_[i] = checked_pointer_cast<ConditionalRender>(rf.MakeConditionalRender());
		}

		for (int i = 0; i < 6; ++ i)
		{
			sm_cameras_[i] = MakeSharedPtr<Camera>();
		}
	}

	PointLightSource::~PointLightSource()
	{
	}

	float3 const & PointLightSource::Position() const
	{
		return pos_;
	}

	void PointLightSource::Position(float3 const & pos)
	{
		pos_ = pos;
		this->UpdateCameras();
	}

	float3 PointLightSource::Direction() const
	{
		return MathLib::transform_quat(float3(0, 0, 1), quat_);
	}
	
	void PointLightSource::Direction(float3 const & dir)
	{
		quat_ = MathLib::unit_axis_to_unit_axis(float3(0, 0, 1), dir);
		this->UpdateCameras();
	}

	Quaternion const & PointLightSource::Rotation() const
	{
		return quat_;
	}

	void PointLightSource::Rotation(Quaternion const & quat)
	{
		quat_ = quat;
		this->UpdateCameras();
	}
	
	void PointLightSource::ModelMatrix(float4x4 const & model)
	{
		float3 scale;
		MathLib::decompose(scale, quat_, pos_, model);
		this->UpdateCameras();
	}

	void PointLightSource::UpdateCameras()
	{
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		CameraPtr const & camera = re.CurFrameBuffer()->GetViewport()->camera;

		for (int j = 0; j < 6; ++ j)
		{
			std::pair<float3, float3> ad = CubeMapViewVector<float>(static_cast<Texture::CubeFaces>(j));
			float3 const & d = ad.first;
			float3 const & u = ad.second;

			float3 lookat = MathLib::transform_quat(d, quat_);
			float3 up = MathLib::transform_quat(u, quat_);

			sm_cameras_[j]->ViewParams(pos_, pos_ + lookat, up);
			sm_cameras_[j]->ProjParams(PI / 2, 1, camera->NearPlane(), camera->FarPlane());
		}
	}

	float3 const & PointLightSource::Falloff() const
	{
		return falloff_;
	}

	void PointLightSource::Falloff(float3 const & falloff)
	{
		falloff_ = falloff;
	}

	TexturePtr const & PointLightSource::ProjectiveTexture() const
	{
		return projective_tex_;
	}

	void PointLightSource::ProjectiveTexture(TexturePtr const & tex)
	{
		projective_tex_ = tex;
	}

	ConditionalRenderPtr const & PointLightSource::ConditionalRenderQuery(uint32_t index) const
	{
		BOOST_ASSERT(index < crs_.size());
		return crs_[index];
	}

	CameraPtr const & PointLightSource::SMCamera(uint32_t index) const
	{
		BOOST_ASSERT(index < sm_cameras_.size());
		return sm_cameras_[index];
	}


	SpotLightSource::SpotLightSource()
		: LightSource(LT_Spot),
			quat_(Quaternion::Identity()),
			pos_(float3::Zero()),
			sm_camera_(MakeSharedPtr<Camera>())
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		cr_ = checked_pointer_cast<ConditionalRender>(rf.MakeConditionalRender());
	}

	SpotLightSource::~SpotLightSource()
	{
	}

	float3 const & SpotLightSource::Position() const
	{
		return pos_;
	}

	void SpotLightSource::Position(float3 const & pos)
	{
		pos_ = pos;
		this->UpdateCamera();
	}

	float3 SpotLightSource::Direction() const
	{
		return MathLib::transform_quat(float3(0, 0, 1), quat_);
	}
	
	void SpotLightSource::Direction(float3 const & dir)
	{
		quat_ = MathLib::unit_axis_to_unit_axis(float3(0, 0, 1), dir);
		this->UpdateCamera();
	}

	Quaternion const & SpotLightSource::Rotation() const
	{
		return quat_;
	}

	void SpotLightSource::Rotation(Quaternion const & quat)
	{
		quat_ = quat;
		this->UpdateCamera();
	}

	void SpotLightSource::ModelMatrix(float4x4 const & model)
	{
		float3 scale;
		MathLib::decompose(scale, quat_, pos_, model);
		this->UpdateCamera();
	}

	void SpotLightSource::UpdateCamera()
	{
		float3 lookat = MathLib::transform_quat(float3(0, 0, 1), quat_);
		float3 up = MathLib::transform_quat(float3(0, 1, 0), quat_);

		sm_camera_->ViewParams(pos_, pos_ + lookat, up);

		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		CameraPtr const & camera = re.CurFrameBuffer()->GetViewport()->camera;
		sm_camera_->ProjParams(cos_outer_inner_.z(), 1, camera->NearPlane(), camera->FarPlane());
	}

	float3 const & SpotLightSource::Falloff() const
	{
		return falloff_;
	}

	void SpotLightSource::Falloff(float3 const & falloff)
	{
		falloff_ = falloff;
	}

	float SpotLightSource::CosInnerAngle() const
	{
		return cos_outer_inner_.y();
	}

	void SpotLightSource::InnerAngle(float angle)
	{
		cos_outer_inner_.y() = cos(angle);
	}

	float SpotLightSource::CosOuterAngle() const
	{
		return cos_outer_inner_.x();
	}

	void SpotLightSource::OuterAngle(float angle)
	{
		cos_outer_inner_.x() = cos(angle);
		cos_outer_inner_.z() = angle * 2;
		cos_outer_inner_.w() = tan(angle);

		this->UpdateCamera();
	}

	float4 const & SpotLightSource::CosOuterInner() const
	{
		return cos_outer_inner_;
	}

	TexturePtr const & SpotLightSource::ProjectiveTexture() const
	{
		return projective_tex_;
	}

	void SpotLightSource::ProjectiveTexture(TexturePtr const & tex)
	{
		projective_tex_ = tex;
	}

	ConditionalRenderPtr const & SpotLightSource::ConditionalRenderQuery(uint32_t /*index*/) const
	{
		return cr_;
	}

	CameraPtr const & SpotLightSource::SMCamera(uint32_t /*index*/) const
	{
		return sm_camera_;
	}


	DirectionalLightSource::DirectionalLightSource()
		: LightSource(LT_Directional),
			quat_(Quaternion::Identity())
	{
		attrib_ = LSA_NoShadow;
	}

	DirectionalLightSource::~DirectionalLightSource()
	{
	}

	void DirectionalLightSource::Attrib(int32_t attrib)
	{
		LightSource::Attrib(attrib);

		// Disable shadow and GI
		attrib_ |= LSA_NoShadow;
		attrib_ &= ~LSA_IndirectLighting;
	}

	Quaternion const & DirectionalLightSource::Rotation() const
	{
		return quat_;
	}

	float3 DirectionalLightSource::Direction() const
	{
		return MathLib::transform_quat(float3(0, 0, 1), quat_);
	}
	
	void DirectionalLightSource::Direction(float3 const & dir)
	{
		quat_ = MathLib::unit_axis_to_unit_axis(float3(0, 0, 1), MathLib::normalize(dir));
	}

	void DirectionalLightSource::Rotation(Quaternion const & quat)
	{
		quat_ = quat;
	}

	void DirectionalLightSource::ModelMatrix(float4x4 const & model)
	{
		float3 scale, pos;
		MathLib::decompose(scale, quat_, pos, model);
	}

	float3 const & DirectionalLightSource::Falloff() const
	{
		return falloff_;
	}

	void DirectionalLightSource::Falloff(float3 const & falloff)
	{
		falloff_ = falloff;
	}
}
