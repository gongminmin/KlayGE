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

namespace KlayGE
{
	class KLAYGE_CORE_API LightSource : public enable_shared_from_this<LightSource>
	{
	public:
		enum LightType
		{
			LT_Ambient = 0,
			LT_Sun,
			LT_Directional,
			LT_Point,
			LT_Spot,
			LT_SphereArea,

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
		virtual ~LightSource();

		LightType Type() const;

		int32_t Attrib() const;
		virtual void Attrib(int32_t attrib);

		bool Enabled() const;
		void Enabled(bool enabled);

		void BindUpdateFunc(function<void(LightSource&, float, float)> const & update_func);

		virtual void Update(float app_time, float elapsed_time);

		virtual void AddToSceneManager();
		virtual void DelFromSceneManager();

		float4 const & Color() const;
		void Color(float3 const & clr);

		virtual TexturePtr const & SkylightTexY() const;
		virtual TexturePtr const & SkylightTexC() const;
		virtual TexturePtr const & SkylightTex() const;
		virtual void SkylightTex(TexturePtr const & tex_y, TexturePtr const & tex_c);
		virtual void SkylightTex(TexturePtr const & tex);
		virtual void SkylightTex(KlayGE::function<TexturePtr()> const & y_cube_tl,
			KlayGE::function<TexturePtr()> const & c_cube_tl);
		virtual void SkylightTex(KlayGE::function<TexturePtr()> const & cube_tl);

		virtual float3 const & Position() const;
		virtual void Position(float3 const & pos);
		virtual float3 Direction() const;
		virtual void Direction(float3 const & dir);
		virtual Quaternion const & Rotation() const;
		virtual void Rotation(Quaternion const & quat);
		virtual void ModelMatrix(float4x4 const & model);
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

	protected:
		LightType type_;
		int32_t attrib_;
		bool enabled_;
		float4 color_;
		Quaternion quat_;
		float3 pos_;
		float3 falloff_;
		float range_;

		function<void(LightSource&, float, float)> update_func_;
	};

	class KLAYGE_CORE_API AmbientLightSource : public LightSource
	{
	public:
		AmbientLightSource();
		virtual ~AmbientLightSource();

		using LightSource::Attrib;
		virtual void Attrib(int32_t attrib) KLAYGE_OVERRIDE;

		virtual TexturePtr const & SkylightTexY() const KLAYGE_OVERRIDE;
		virtual TexturePtr const & SkylightTexC() const KLAYGE_OVERRIDE;
		virtual TexturePtr const & SkylightTex() const KLAYGE_OVERRIDE;
		virtual void SkylightTex(TexturePtr const & tex_y, TexturePtr const & tex_c) KLAYGE_OVERRIDE;
		virtual void SkylightTex(TexturePtr const & tex) KLAYGE_OVERRIDE;
		virtual void SkylightTex(KlayGE::function<TexturePtr()> const & y_cube_tl,
			KlayGE::function<TexturePtr()> const & c_cube_tl) KLAYGE_OVERRIDE;
		virtual void SkylightTex(KlayGE::function<TexturePtr()> const & cube_tl) KLAYGE_OVERRIDE;

	private:
		mutable TexturePtr sky_tex_y_;
		mutable TexturePtr sky_tex_c_;
		KlayGE::function<TexturePtr()> sky_tex_y_tl_;
		KlayGE::function<TexturePtr()> sky_tex_c_tl_;
	};

	class KLAYGE_CORE_API PointLightSource : public LightSource
	{
	public:
		PointLightSource();
		virtual ~PointLightSource();

		using LightSource::Position;
		virtual void Position(float3 const & pos) KLAYGE_OVERRIDE;
		using LightSource::Direction;
		virtual void Direction(float3 const & dir) KLAYGE_OVERRIDE;
		using LightSource::Rotation;
		virtual void Rotation(Quaternion const & quat) KLAYGE_OVERRIDE;
		void ModelMatrix(float4x4 const & model);

		virtual TexturePtr const & ProjectiveTexture() const KLAYGE_OVERRIDE;
		virtual void ProjectiveTexture(TexturePtr const & tex) KLAYGE_OVERRIDE;

		virtual ConditionalRenderPtr const & ConditionalRenderQuery(uint32_t index) const KLAYGE_OVERRIDE;
		virtual CameraPtr const & SMCamera(uint32_t index) const KLAYGE_OVERRIDE;

	protected:
		void UpdateCameras();

	protected:
		TexturePtr projective_tex_;

		array<ConditionalRenderPtr, 7> crs_;
		array<CameraPtr, 6> sm_cameras_;
	};

	class KLAYGE_CORE_API SpotLightSource : public LightSource
	{
	public:
		SpotLightSource();
		virtual ~SpotLightSource();

		using LightSource::Position;
		virtual void Position(float3 const & pos) KLAYGE_OVERRIDE;
		using LightSource::Direction;
		virtual void Direction(float3 const & dir) KLAYGE_OVERRIDE;
		using LightSource::Rotation;
		virtual void Rotation(Quaternion const & quat) KLAYGE_OVERRIDE;
		virtual void ModelMatrix(float4x4 const & model) KLAYGE_OVERRIDE;

		virtual float CosInnerAngle() const KLAYGE_OVERRIDE;
		virtual void InnerAngle(float angle) KLAYGE_OVERRIDE;

		virtual float CosOuterAngle() const KLAYGE_OVERRIDE;
		virtual void OuterAngle(float angle) KLAYGE_OVERRIDE;

		virtual float4 const & CosOuterInner() const KLAYGE_OVERRIDE;

		virtual TexturePtr const & ProjectiveTexture() const KLAYGE_OVERRIDE;
		virtual void ProjectiveTexture(TexturePtr const & tex) KLAYGE_OVERRIDE;

		virtual ConditionalRenderPtr const & ConditionalRenderQuery(uint32_t index) const KLAYGE_OVERRIDE;
		virtual CameraPtr const & SMCamera(uint32_t index) const KLAYGE_OVERRIDE;

	protected:
		void UpdateCamera();

	protected:
		float4 cos_outer_inner_;

		TexturePtr projective_tex_;

		ConditionalRenderPtr cr_;
		CameraPtr sm_camera_;
	};

	class KLAYGE_CORE_API DirectionalLightSource : public LightSource
	{
	public:
		DirectionalLightSource();
		virtual ~DirectionalLightSource();

		using LightSource::Attrib;
		virtual void Attrib(int32_t attrib) KLAYGE_OVERRIDE;
	};

	class KLAYGE_CORE_API SunLightSource : public LightSource
	{
	public:
		SunLightSource();
		virtual ~SunLightSource();

		using LightSource::Attrib;
		virtual void Attrib(int32_t attrib) KLAYGE_OVERRIDE;

		virtual CameraPtr const & SMCamera(uint32_t index) const KLAYGE_OVERRIDE;

		void UpdateSMCamera(Camera const & scene_camera);

	protected:
		CameraPtr sm_camera_;
	};

	class KLAYGE_CORE_API SphereAreaLightSource : public PointLightSource
	{
	public:
		SphereAreaLightSource();
		virtual ~SphereAreaLightSource();

		virtual float Radius() const;
		virtual void Radius(float radius);

	protected:
		float radius_;
	};
}

#endif		// _LIGHT_HPP
