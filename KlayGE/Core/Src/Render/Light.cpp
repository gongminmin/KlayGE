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
#include <KlayGE/CascadedShadowLayer.hpp>

#include <KlayGE/Light.hpp>

namespace KlayGE
{
	LightSource::LightSource(LightType type)
		: type_(type), attrib_(0), enabled_(true),
			color_(0, 0, 0, 0),
			quat_(Quaternion::Identity()),
			pos_(float3::Zero()),
			range_(-1)
	{
	}

	LightSource::~LightSource()
	{
	}

	LightSource::LightType LightSource::Type() const
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

	void LightSource::BindUpdateFunc(std::function<void(LightSource&, float, float)> const & update_func)
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

		this->Range(-1);
	}

	TexturePtr const & LightSource::SkylightTexY() const
	{
		BOOST_ASSERT(false);
		static TexturePtr ret;
		return ret;
	}

	TexturePtr const & LightSource::SkylightTexC() const
	{
		BOOST_ASSERT(false);
		static TexturePtr ret;
		return ret;
	}

	TexturePtr const & LightSource::SkylightTex() const
	{
		BOOST_ASSERT(false);
		static TexturePtr ret;
		return ret;
	}

	void LightSource::SkylightTex(TexturePtr const & tex_y, TexturePtr const & tex_c)
	{
		KFL_UNUSED(tex_y);
		KFL_UNUSED(tex_c);

		BOOST_ASSERT(false);
	}

	void LightSource::SkylightTex(TexturePtr const & tex)
	{
		KFL_UNUSED(tex);

		BOOST_ASSERT(false);
	}

	float3 const & LightSource::Position() const
	{
		return pos_;
	}

	void LightSource::Position(float3 const & pos)
	{
		pos_ = pos;
	}

	float3 LightSource::Direction() const
	{
		return MathLib::transform_quat(float3(0, 0, 1), quat_);
	}
	
	void LightSource::Direction(float3 const & dir)
	{
		quat_ = MathLib::unit_axis_to_unit_axis(float3(0, 0, 1), dir);
	}

	Quaternion const & LightSource::Rotation() const
	{
		return quat_;
	}

	void LightSource::Rotation(Quaternion const & quat)
	{
		quat_ = quat;
	}

	void LightSource::ModelMatrix(float4x4 const & model)
	{
		float3 scale;
		MathLib::decompose(scale, quat_, pos_, model);
	}

	float3 const & LightSource::Falloff() const
	{
		return falloff_;
	}

	void LightSource::Falloff(float3 const & fall_off)
	{
		falloff_ = fall_off;

		this->Range(-1);
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

	float LightSource::Range() const
	{
		return range_ >= 0 ? range_ : -range_;
	}
	
	void LightSource::Range(float range)
	{
		if (range <= 0)
		{
			const float4 RGB_TO_LUM(0.2126f, 0.7152f, 0.0722f, 0);
			float lum = MathLib::dot(color_, RGB_TO_LUM);
			if (MathLib::abs(falloff_.z()) < 1e-6f)
			{
				if (MathLib::abs(falloff_.y()) < 1e-6f)
				{
					range_ = 100;
				}
				else
				{
					range_ = MathLib::abs(falloff_.y()) < 1e-6f ? 1 : -(falloff_.x() - lum * 255) / falloff_.y();
				}
			}
			else
			{
				float delta = falloff_.y() * falloff_.y() - 4 * falloff_.z() * (falloff_.x() - lum * 255);
				range_ = delta < 0 ? 1 : (-falloff_.y() + sqrt(delta)) / (2 * falloff_.z());
			}
			range_ = -std::min(range_, 100.0f);
		}
		else
		{
			range_ = range;
		}
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

	float LightSource::Radius() const
	{
		return 0;
	}

	void LightSource::Radius(float radius)
	{
		KFL_UNUSED(radius);
	}

	float3 const & LightSource::Extend() const
	{
		static const float3 ret(0, 0, 0);
		return ret;
	}

	void LightSource::Extend(float3 const & extend)
	{
		KFL_UNUSED(extend);
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

	TexturePtr const & AmbientLightSource::SkylightTexY() const
	{
		return sky_tex_y_;
	}

	TexturePtr const & AmbientLightSource::SkylightTexC() const
	{
		return sky_tex_c_;
	}

	TexturePtr const & AmbientLightSource::SkylightTex() const
	{
		return sky_tex_y_;
	}

	void AmbientLightSource::SkylightTex(TexturePtr const & tex_y, TexturePtr const & tex_c)
	{
		sky_tex_y_ = tex_y;
		sky_tex_c_ = tex_c;
	}

	void AmbientLightSource::SkylightTex(TexturePtr const & tex)
	{
		sky_tex_y_ = tex;
	}


	PointLightSource::PointLightSource()
		: LightSource(LT_Point)
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

	void PointLightSource::Position(float3 const & pos)
	{
		LightSource::Position(pos);
		this->UpdateCameras();
	}
	
	void PointLightSource::Direction(float3 const & dir)
	{
		LightSource::Direction(dir);
		this->UpdateCameras();
	}

	void PointLightSource::Rotation(Quaternion const & quat)
	{
		LightSource::Rotation(quat);
		this->UpdateCameras();
	}
	
	void PointLightSource::ModelMatrix(float4x4 const & model)
	{
		LightSource::ModelMatrix(model);
		this->UpdateCameras();
	}

	void PointLightSource::UpdateCameras()
	{
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		Camera const & camera = *re.CurFrameBuffer()->GetViewport()->camera;

		for (int j = 0; j < 6; ++ j)
		{
			float3 d, u;
			std::tie(d, u) = CubeMapViewVector<float>(static_cast<Texture::CubeFaces>(j));

			float3 lookat = MathLib::transform_quat(d, quat_);
			float3 up = MathLib::transform_quat(u, quat_);

			sm_cameras_[j]->ViewParams(pos_, pos_ + lookat, up);
			sm_cameras_[j]->ProjParams(PI / 2, 1, camera.NearPlane(), camera.FarPlane());
		}
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
			sm_camera_(MakeSharedPtr<Camera>())
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		cr_ = checked_pointer_cast<ConditionalRender>(rf.MakeConditionalRender());
	}

	SpotLightSource::~SpotLightSource()
	{
	}

	void SpotLightSource::Position(float3 const & pos)
	{
		LightSource::Position(pos);
		this->UpdateCamera();
	}
	
	void SpotLightSource::Direction(float3 const & dir)
	{
		LightSource::Direction(dir);
		this->UpdateCamera();
	}

	void SpotLightSource::Rotation(Quaternion const & quat)
	{
		LightSource::Rotation(quat);
		this->UpdateCamera();
	}

	void SpotLightSource::ModelMatrix(float4x4 const & model)
	{
		LightSource::ModelMatrix(model);
		this->UpdateCamera();
	}

	void SpotLightSource::UpdateCamera()
	{
		float3 lookat = MathLib::transform_quat(float3(0, 0, 1), quat_);
		float3 up = MathLib::transform_quat(float3(0, 1, 0), quat_);

		sm_camera_->ViewParams(pos_, pos_ + lookat, up);

		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		Camera const & camera = *re.CurFrameBuffer()->GetViewport()->camera;
		sm_camera_->ProjParams(cos_outer_inner_.z(), 1, camera.NearPlane(), camera.FarPlane());
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
		: LightSource(LT_Directional)
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


	SunLightSource::SunLightSource()
		: LightSource(LT_Sun),
			sm_camera_(MakeSharedPtr<Camera>())
	{
		attrib_ = 0;
	}

	SunLightSource::~SunLightSource()
	{
	}

	void SunLightSource::Attrib(int32_t attrib)
	{
		LightSource::Attrib(attrib);

		// Enable shadow and disable GI
		attrib_ &= ~LSA_NoShadow;
		attrib_ &= ~LSA_IndirectLighting;
	}

	CameraPtr const & SunLightSource::SMCamera(uint32_t /*index*/) const
	{
		return sm_camera_;
	}

	void SunLightSource::UpdateSMCamera(Camera const & scene_camera)
	{
		float3 const dir = this->Direction();

		float3 up_vec;
		if (MathLib::abs(MathLib::dot(-dir, scene_camera.UpVec())) > 0.95f)
		{
			up_vec = scene_camera.RightVec();
		}
		else
		{
			up_vec = scene_camera.UpVec();
		}

		float4x4 light_view = MathLib::look_at_lh(-dir, float3(0, 0, 0), up_vec);

		AABBox const aabb = CalcFrustumExtents(scene_camera, scene_camera.NearPlane(), scene_camera.FarPlane(), light_view);

		float3 const & center = aabb.Center();
		float3 view_pos = MathLib::transform_coord(float3(center.x(), center.y(), aabb.Min().z()), MathLib::inverse(light_view));
		sm_camera_->ViewParams(view_pos, view_pos + dir, up_vec);

		float3 dimensions = aabb.Max() - aabb.Min();
		sm_camera_->ProjOrthoParams(dimensions.x(), dimensions.y(), 0.0f, dimensions.z());
	}


	SphereAreaLightSource::SphereAreaLightSource()
	{
		type_ = LT_SphereArea;
	}

	SphereAreaLightSource::~SphereAreaLightSource()
	{
	}

	float SphereAreaLightSource::Radius() const
	{
		return radius_;
	}

	void SphereAreaLightSource::Radius(float radius)
	{
		radius_ = radius;
	}


	TubeAreaLightSource::TubeAreaLightSource()
	{
		type_ = LT_TubeArea;
	}

	TubeAreaLightSource::~TubeAreaLightSource()
	{
	}

	void TubeAreaLightSource::Falloff(float3 const & fall_off)
	{
		KFL_UNUSED(fall_off);
		LightSource::Falloff(float3(1, 0, 0));
	}

	float3 const & TubeAreaLightSource::Extend() const
	{
		return extend_;
	}

	void TubeAreaLightSource::Extend(float3 const & extend)
	{
		extend_ = extend;
	}
}
