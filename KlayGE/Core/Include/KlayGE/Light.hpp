// Light.hpp
// KlayGE Light header file
// Ver 3.12.0
// Copyright(C) Minmin Gong, 2011
// Homepage: http://www.klayge.org
//
// 3.12.0
// First release (2011.1.12)
//
// CHANGE LIST
/////////////////////////////////////////////////////////////////////////////////

#ifndef _LIGHT_HPP
#define _LIGHT_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/SceneComponent.hpp>

#include <array>

namespace KlayGE
{
	class KLAYGE_CORE_API LightSource : public SceneComponent, public std::enable_shared_from_this<LightSource>
	{
	public:
#if defined(KLAYGE_COMPILER_CLANGCL)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winconsistent-missing-override"
#endif
		BOOST_TYPE_INDEX_REGISTER_RUNTIME_CLASS((SceneComponent))
#if defined(KLAYGE_COMPILER_CLANGCL)
#pragma clang diagnostic pop
#endif

		enum LightType
		{
			LT_Ambient = 0,
			LT_Directional,
			LT_Point,
			LT_Spot,
			LT_SphereArea,
			LT_TubeArea,

			LT_NumLightTypes
		};

		enum LightSrcAttrib
		{
			LSA_NoShadow = 1UL << 0,
			LSA_NoDiffuse = 1UL << 1,
			LSA_NoSpecular = 1UL << 2,
			LSA_IndirectLighting = 1UL << 3,
			LSA_Temporary = 1UL << 4
		};


	public:
		explicit LightSource(LightType type);
		virtual ~LightSource() noexcept;

		LightType Type() const;

		int32_t Attrib() const;
		virtual void Attrib(int32_t attrib);

		float4 const & Color() const;
		void Color(float3 const & clr);

		virtual TexturePtr const & SkylightTexY() const;
		virtual TexturePtr const & SkylightTexC() const;
		virtual TexturePtr const & SkylightTex() const;
		virtual void SkylightTex(TexturePtr const & tex_y, TexturePtr const & tex_c);
		virtual void SkylightTex(TexturePtr const & tex);

		float3 const & Position() const;
		float3 const & Direction() const;
		Quaternion Rotation() const;
		virtual float3 const & Falloff() const;
		virtual void Falloff(float3 const & fall_off);
		virtual float CosInnerAngle() const;
		virtual void InnerAngle(float angle);
		virtual float CosOuterAngle() const;
		virtual void OuterAngle(float angle);
		virtual float4 const & CosOuterInner() const;
		virtual float Range() const;
		virtual void Range(float range);

		virtual TexturePtr const & ProjectiveTexture() const;
		virtual void ProjectiveTexture(TexturePtr const & tex);

		virtual ConditionalRenderPtr const & ConditionalRenderQuery(uint32_t index) const;
		virtual CameraPtr const & SMCamera(uint32_t index) const;

		// For sphere area
		virtual float Radius() const;
		virtual void Radius(float radius);

		// For tube area
		virtual float3 const & Extend() const;
		virtual void Extend(float3 const & extend);

	protected:
		void CloneTo(LightSource& light) const;

	protected:
		LightType type_;
		int32_t attrib_ = 0;
		float4 color_ = float4(0, 0, 0, 0);
		float3 falloff_;
		float range_ = -1;

		std::function<void(LightSource&, float, float)> update_func_;
	};

	class KLAYGE_CORE_API AmbientLightSource final : public LightSource
	{
	public:
#if defined(KLAYGE_COMPILER_CLANGCL)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winconsistent-missing-override"
#endif
		BOOST_TYPE_INDEX_REGISTER_RUNTIME_CLASS((LightSource))
#if defined(KLAYGE_COMPILER_CLANGCL)
#pragma clang diagnostic pop
#endif

		AmbientLightSource();

		SceneComponentPtr Clone() const override;

		using LightSource::Attrib;
		virtual void Attrib(int32_t attrib) override;

		virtual TexturePtr const & SkylightTexY() const override;
		virtual TexturePtr const & SkylightTexC() const override;
		virtual TexturePtr const & SkylightTex() const override;
		virtual void SkylightTex(TexturePtr const & tex_y, TexturePtr const & tex_c) override;
		virtual void SkylightTex(TexturePtr const & tex) override;

	private:
		mutable TexturePtr sky_tex_y_;
		mutable TexturePtr sky_tex_c_;
	};

	class KLAYGE_CORE_API PointLightSource : public LightSource
	{
	public:
#if defined(KLAYGE_COMPILER_CLANGCL)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winconsistent-missing-override"
#endif
		BOOST_TYPE_INDEX_REGISTER_RUNTIME_CLASS((LightSource))
#if defined(KLAYGE_COMPILER_CLANGCL)
#pragma clang diagnostic pop
#endif

		PointLightSource();

		SceneComponentPtr Clone() const override;

		void BindSceneNode(SceneNode* node) override;

		void MainThreadUpdate(float app_time, float elapsed_time) override;

		virtual TexturePtr const & ProjectiveTexture() const override;
		virtual void ProjectiveTexture(TexturePtr const & tex) override;

		virtual ConditionalRenderPtr const & ConditionalRenderQuery(uint32_t index) const override;
		virtual CameraPtr const & SMCamera(uint32_t index) const override;

	protected:
		void CloneToPoint(PointLightSource& point_light) const;
		void UpdateCameras();

	protected:
		TexturePtr projective_tex_;

		std::array<ConditionalRenderPtr, 7> crs_;
		std::array<CameraPtr, 6> sm_cameras_;
	};

	class KLAYGE_CORE_API SpotLightSource final : public LightSource
	{
	public:
#if defined(KLAYGE_COMPILER_CLANGCL)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winconsistent-missing-override"
#endif
		BOOST_TYPE_INDEX_REGISTER_RUNTIME_CLASS((LightSource))
#if defined(KLAYGE_COMPILER_CLANGCL)
#pragma clang diagnostic pop
#endif

		SpotLightSource();

		SceneComponentPtr Clone() const override;

		void BindSceneNode(SceneNode* node) override;

		void MainThreadUpdate(float app_time, float elapsed_time) override;

		virtual float CosInnerAngle() const override;
		virtual void InnerAngle(float angle) override;

		virtual float CosOuterAngle() const override;
		virtual void OuterAngle(float angle) override;

		virtual float4 const & CosOuterInner() const override;

		virtual TexturePtr const & ProjectiveTexture() const override;
		virtual void ProjectiveTexture(TexturePtr const & tex) override;

		virtual ConditionalRenderPtr const & ConditionalRenderQuery(uint32_t index) const override;
		virtual CameraPtr const & SMCamera(uint32_t index) const override;

	protected:
		void UpdateCamera();

	protected:
		float4 cos_outer_inner_;

		TexturePtr projective_tex_;

		ConditionalRenderPtr cr_;
		CameraPtr sm_camera_;
	};

	class KLAYGE_CORE_API DirectionalLightSource final : public LightSource
	{
	public:
#if defined(KLAYGE_COMPILER_CLANGCL)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winconsistent-missing-override"
#endif
		BOOST_TYPE_INDEX_REGISTER_RUNTIME_CLASS((LightSource))
#if defined(KLAYGE_COMPILER_CLANGCL)
#pragma clang diagnostic pop
#endif

		DirectionalLightSource();

		SceneComponentPtr Clone() const override;

		void BindSceneNode(SceneNode* node) override;

		void MainThreadUpdate(float app_time, float elapsed_time) override;

		using LightSource::Attrib;
		virtual void Attrib(int32_t attrib) override;

		virtual CameraPtr const & SMCamera(uint32_t index) const override;

	protected:
		void UpdateCamera();

	protected:
		CameraPtr sm_camera_;
	};

	class KLAYGE_CORE_API SphereAreaLightSource final : public PointLightSource
	{
	public:
#if defined(KLAYGE_COMPILER_CLANGCL)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winconsistent-missing-override"
#endif
		BOOST_TYPE_INDEX_REGISTER_RUNTIME_CLASS((LightSource))
#if defined(KLAYGE_COMPILER_CLANGCL)
#pragma clang diagnostic pop
#endif

		SphereAreaLightSource();

		SceneComponentPtr Clone() const override;

		virtual float Radius() const override;
		virtual void Radius(float radius) override;

	protected:
		float radius_;
	};

	class KLAYGE_CORE_API TubeAreaLightSource final : public PointLightSource
	{
	public:
#if defined(KLAYGE_COMPILER_CLANGCL)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winconsistent-missing-override"
#endif
		BOOST_TYPE_INDEX_REGISTER_RUNTIME_CLASS((LightSource))
#if defined(KLAYGE_COMPILER_CLANGCL)
#pragma clang diagnostic pop
#endif

		TubeAreaLightSource();

		SceneComponentPtr Clone() const override;

		using LightSource::Falloff;
		void Falloff(float3 const & fall_off) override;
		float3 const & Extend() const override;
		void Extend(float3 const & extend) override;

	protected:
		float3 extend_;
	};
}

#endif		// _LIGHT_HPP
