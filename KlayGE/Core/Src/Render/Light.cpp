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
#include <KlayGE/RenderFactory.hpp>
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

	float4 const & LightSource::Color() const
	{
		return color_;
	}

	void LightSource::Color(float3 const & clr)
	{
		color_ = float4(clr.x(), clr.y(), clr.z(),
			MathLib::dot(clr, float3(0.27f, 0.67f, 0.06f)));
	}

	float3 const & LightSource::Position() const
	{
		return float3::Zero();
	}

	void LightSource::Position(float3 const & /*pos*/)
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

	ConditionalRenderPtr LightSource::ConditionalRenderQuery(uint32_t /*index*/) const
	{
		return ConditionalRenderPtr();
	}

	CameraPtr LightSource::SMCamera(uint32_t /*index*/) const
	{
		return CameraPtr();
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


	PointLightSource::PointLightSource()
		: LightSource(LT_Point)
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		for (int i = 0; i < 7; ++ i)
		{
			crs_.push_back(checked_pointer_cast<ConditionalRender>(rf.MakeConditionalRender()));
		}

		for (int i = 0; i < 6; ++ i)
		{
			sm_cameras_.push_back(MakeSharedPtr<Camera>());
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
		for (int j = 0; j < 6; ++ j)
		{
			std::pair<float3, float3> ad = CubeMapViewVector<float>(static_cast<Texture::CubeFaces>(j));
			float3 const & d = ad.first;
			float3 const & u = ad.second;

			float3 lookat = MathLib::transform_quat(d, quat_);
			float3 up = MathLib::transform_quat(u, quat_);

			sm_cameras_[j]->ViewParams(pos_, pos_ + lookat, up);
			sm_cameras_[j]->ProjParams(PI / 2, 1, 0.1f, 100.0f);
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

	ConditionalRenderPtr PointLightSource::ConditionalRenderQuery(uint32_t index) const
	{
		BOOST_ASSERT(index < crs_.size());
		return crs_[index];
	}

	CameraPtr PointLightSource::SMCamera(uint32_t index) const
	{
		BOOST_ASSERT(index < sm_cameras_.size());
		return sm_cameras_[index];
	}


	SpotLightSource::SpotLightSource()
		: LightSource(LT_Spot),
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
		sm_camera_->ProjParams(cos_outer_inner_.z(), 1, 0.1f, 100.0f);
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
	}

	float4 const & SpotLightSource::CosOuterInner() const
	{
		return cos_outer_inner_;
	}

	ConditionalRenderPtr SpotLightSource::ConditionalRenderQuery(uint32_t /*index*/) const
	{
		return cr_;
	}

	CameraPtr SpotLightSource::SMCamera(uint32_t /*index*/) const
	{
		return sm_camera_;
	}


	DirectionalLightSource::DirectionalLightSource()
		: LightSource(LT_Directional)
	{
		attrib_ = LSA_NoShadow;
	}

	DirectionalLightSource::~DirectionalLightSource()
	{
	}

	Quaternion const & DirectionalLightSource::Rotation() const
	{
		return quat_;
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
