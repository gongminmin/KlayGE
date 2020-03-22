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
#include <KFL/ErrorHandling.hpp>
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
		: type_(type)
	{
	}

	LightSource::~LightSource() noexcept = default;

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
		KFL_UNREACHABLE("Can't be called");
	}

	TexturePtr const & LightSource::SkylightTexC() const
	{
		KFL_UNREACHABLE("Can't be called");
	}

	TexturePtr const & LightSource::SkylightTex() const
	{
		KFL_UNREACHABLE("Can't be called");
	}

	void LightSource::SkylightTex(TexturePtr const & tex_y, TexturePtr const & tex_c)
	{
		KFL_UNUSED(tex_y);
		KFL_UNUSED(tex_c);

		KFL_UNREACHABLE("Can't be called");
	}

	void LightSource::SkylightTex(TexturePtr const & tex)
	{
		KFL_UNUSED(tex);

		KFL_UNREACHABLE("Can't be called");
	}

	float3 const & LightSource::Position() const
	{
		float4x4 const& mat = this->BoundSceneNode()->TransformToWorld();
		return *reinterpret_cast<float3 const*>(&mat.Row(3));
	}

	float3 const& LightSource::Direction() const
	{
		float4x4 const& mat = this->BoundSceneNode()->TransformToWorld();
		return *reinterpret_cast<float3 const*>(&mat.Row(2));
	}

	Quaternion LightSource::Rotation() const
	{
		float3 scale;
		Quaternion rot;
		float3 trans;
		MathLib::decompose(scale, rot, trans, this->BoundSceneNode()->TransformToWorld());
		return rot;
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

	void LightSource::CloneTo(LightSource& light) const
	{
		light.type_ = type_;
		light.attrib_ = attrib_;
		light.color_ = color_;
		light.falloff_ = falloff_;
		light.range_ = range_;

		light.update_func_ = update_func_;
	}


	AmbientLightSource::AmbientLightSource()
		: LightSource(LT_Ambient)
	{
		attrib_ = LSA_NoShadow;
		color_ = float4(0, 0, 0, 0);
	}

	SceneComponentPtr AmbientLightSource::Clone() const
	{
		auto ret = MakeSharedPtr<AmbientLightSource>();

		this->CloneTo(*ret);
		ret->sky_tex_y_ = sky_tex_y_;
		ret->sky_tex_c_ = sky_tex_c_;

		return ret;
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

	SceneComponentPtr PointLightSource::Clone() const
	{
		auto ret = MakeSharedPtr<PointLightSource>();
		this->CloneToPoint(*ret);
		return ret;
	}

	void PointLightSource::BindSceneNode(SceneNode* node)
	{
		if (!(attrib_ & LSA_NoShadow))
		{
			auto* curr_node = this->BoundSceneNode();
			if (curr_node != nullptr)
			{
				for (int i = 0; i < 6; ++i)
				{
					auto* curr_camera_node = sm_cameras_[i]->BoundSceneNode();
					if (curr_camera_node != nullptr)
					{
						curr_node->RemoveChild(curr_camera_node);
					}
				}
			}
		}

		SceneComponent::BindSceneNode(node);

		if (!(attrib_ & LSA_NoShadow))
		{
			if (node != nullptr)
			{
				for (uint32_t i = 0; i < 6; ++i)
				{
					auto camera_node = MakeSharedPtr<SceneNode>(
						L"ShadowCameraNode", SceneNode::SOA_Cullable | SceneNode::SOA_Moveable | SceneNode::SOA_NotCastShadow);
					camera_node->AddComponent(sm_cameras_[i]);
					node->AddChild(camera_node);

					float3 lookat, up;
					std::tie(lookat, up) = CubeMapViewVector<float>(static_cast<Texture::CubeFaces>(i));

					sm_cameras_[i]->LookAtDist(1);
					camera_node->TransformToParent(MathLib::inverse(MathLib::look_at_lh(float3(0, 0, 0), lookat, up)));
				}
			}
		}
	}

	void PointLightSource::MainThreadUpdate(float app_time, float elapsed_time)
	{
		SceneComponent::MainThreadUpdate(app_time, elapsed_time);

		if (!(attrib_ & LSA_NoShadow))
		{
			this->UpdateCameras();
		}
	}

	void PointLightSource::CloneToPoint(PointLightSource& point_light) const
	{
		this->CloneTo(point_light);
		point_light.projective_tex_ = projective_tex_;
		point_light.crs_ = crs_;
		for (size_t i = 0; i < sm_cameras_.size(); ++i)
		{
			point_light.sm_cameras_[i] = checked_pointer_cast<Camera>(sm_cameras_[i]->Clone());
		}
	}

	void PointLightSource::UpdateCameras()
	{
		auto& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		auto const& scene_camera = *re.CurFrameBuffer()->Viewport()->Camera();

		for (uint32_t i = 0; i < 6; ++i)
		{
			sm_cameras_[i]->ProjParams(PI / 2, 1, scene_camera.NearPlane(), scene_camera.FarPlane());
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

	SceneComponentPtr SpotLightSource::Clone() const
	{
		auto ret = MakeSharedPtr<SpotLightSource>();

		this->CloneTo(*ret);
		ret->cos_outer_inner_ = cos_outer_inner_;
		ret->projective_tex_ = projective_tex_;
		ret->cr_ = cr_;
		ret->sm_camera_ = checked_pointer_cast<Camera>(sm_camera_->Clone());

		return ret;
	}

	void SpotLightSource::BindSceneNode(SceneNode* node)
	{
		if (!(attrib_ & LSA_NoShadow))
		{
			auto* curr_node = this->BoundSceneNode();
			if (curr_node != nullptr)
			{
				auto* curr_camera_node = sm_camera_->BoundSceneNode();
				if (curr_camera_node != nullptr)
				{
					curr_node->RemoveChild(curr_camera_node);
				}
			}
		}

		SceneComponent::BindSceneNode(node);

		if (!(attrib_ & LSA_NoShadow))
		{
			if (node != nullptr)
			{
				auto camera_node = MakeSharedPtr<SceneNode>(
					L"ShadowCameraNode", SceneNode::SOA_Cullable | SceneNode::SOA_Moveable | SceneNode::SOA_NotCastShadow);
				camera_node->AddComponent(sm_camera_);
				node->AddChild(camera_node);

				sm_camera_->LookAtDist(1);
				camera_node->TransformToParent(MathLib::inverse(MathLib::look_at_lh(float3(0, 0, 0), float3(0, 0, 1), float3(0, 1, 0))));
			}
		}
	}

	void SpotLightSource::MainThreadUpdate(float app_time, float elapsed_time)
	{
		SceneComponent::MainThreadUpdate(app_time, elapsed_time);

		if (!(attrib_ & LSA_NoShadow))
		{
			this->UpdateCamera();
		}
	}

	void SpotLightSource::UpdateCamera()
	{
		auto& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		auto const& scene_camera = *re.CurFrameBuffer()->Viewport()->Camera();
		sm_camera_->ProjParams(cos_outer_inner_.z(), 1, scene_camera.NearPlane(), scene_camera.FarPlane());
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
			sm_camera_(MakeSharedPtr<Camera>())
	{
		attrib_ = 0;
	}

	SceneComponentPtr DirectionalLightSource::Clone() const
	{
		auto ret = MakeSharedPtr<DirectionalLightSource>();

		this->CloneTo(*ret);
		ret->sm_camera_ = checked_pointer_cast<Camera>(sm_camera_->Clone());

		return ret;
	}

	void DirectionalLightSource::BindSceneNode(SceneNode* node)
	{
		if (!(attrib_ & LSA_NoShadow))
		{
			auto* curr_node = this->BoundSceneNode();
			if (curr_node != nullptr)
			{
				auto* curr_camera_node = sm_camera_->BoundSceneNode();
				if (curr_camera_node != nullptr)
				{
					curr_node->RemoveChild(curr_camera_node);
				}
			}
		}

		SceneComponent::BindSceneNode(node);

		if (!(attrib_ & LSA_NoShadow))
		{
			if (node != nullptr)
			{
				auto camera_node = MakeSharedPtr<SceneNode>(
					L"ShadowCameraNode", SceneNode::SOA_Cullable | SceneNode::SOA_Moveable | SceneNode::SOA_NotCastShadow);
				camera_node->AddComponent(sm_camera_);
				node->AddChild(camera_node);
			}
		}
	}

	void DirectionalLightSource::Attrib(int32_t attrib)
	{
		LightSource::Attrib(attrib);

		// Disable GI
		attrib_ &= ~LSA_IndirectLighting;
	}

	void DirectionalLightSource::MainThreadUpdate(float app_time, float elapsed_time)
	{
		LightSource::MainThreadUpdate(app_time, elapsed_time);

		if (!(attrib_ & LSA_NoShadow))
		{
			this->UpdateCamera();
		}
	}

	CameraPtr const & DirectionalLightSource::SMCamera(uint32_t /*index*/) const
	{
		return sm_camera_;
	}

	void DirectionalLightSource::UpdateCamera()
	{
		auto& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		auto const& scene_camera = *re.CurFrameBuffer()->Viewport()->Camera();

		float3 const& dir = this->Direction();

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

		sm_camera_->LookAtDist(1);
		auto& camera_node = *sm_camera_->BoundSceneNode();
		camera_node.TransformToWorld(MathLib::inverse(MathLib::look_at_lh(view_pos, view_pos + dir, up_vec)));

		float3 dimensions = aabb.Max() - aabb.Min();
		sm_camera_->ProjOrthoParams(dimensions.x(), dimensions.y(), 0.0f, dimensions.z());
	}


	SphereAreaLightSource::SphereAreaLightSource()
	{
		type_ = LT_SphereArea;
	}

	SceneComponentPtr SphereAreaLightSource::Clone() const
	{
		auto ret = MakeSharedPtr<SphereAreaLightSource>();

		this->CloneToPoint(*ret);
		ret->radius_ = radius_;

		return ret;
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

	SceneComponentPtr TubeAreaLightSource::Clone() const
	{
		auto ret = MakeSharedPtr<TubeAreaLightSource>();

		this->CloneToPoint(*ret);
		ret->extend_ = extend_;

		return ret;
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
