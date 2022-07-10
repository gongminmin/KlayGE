// RenderableHelper.hpp
// KlayGE һЩ���õĿ���Ⱦ���� ͷ�ļ�
// Ver 3.9.0
// ��Ȩ����(C) ������, 2005-2010
// Homepage: http://www.klayge.org
//
// 3.10.0
// RenderableSkyBox��RenderableHDRSkyBox������Technique() (2010.1.4)
//
// 3.9.0
// ������RenderableHDRSkyBox (2009.5.4)
//
// 2.7.1
// ������RenderableHelper���� (2005.7.10)
//
// 2.6.0
// ������RenderableSkyBox (2005.5.26)
//
// 2.5.0
// ������RenderablePoint��RenderableLine��RenderableTriangle (2005.4.13)
//
// 2.4.0
// ���ν��� (2005.3.22)
//
// �޸ļ�¼
//////////////////////////////////////////////////////////////////////////////////

#ifndef _RENDERABLEHELPER_HPP
#define _RENDERABLEHELPER_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/Renderable.hpp>
#include <KFL/AABBox.hpp>

namespace KlayGE
{
	class KLAYGE_CORE_API RenderablePoint : public Renderable
	{
	public:
		explicit RenderablePoint();
		RenderablePoint(float3 const & v, Color const & clr);

		void SetPoint(float3 const & v);
		void SetColor(Color const & clr);

	private:
		RenderEffectParameter* v0_ep_;
		RenderEffectParameter* color_ep_;
	};

	class KLAYGE_CORE_API RenderableLine : public Renderable
	{
	public:
		explicit RenderableLine();
		RenderableLine(float3 const & v0, float3 const & v1, Color const & clr);

		void SetLine(float3 const & v0, float3 const & v1);
		void SetColor(Color const & clr);

	private:
		RenderEffectParameter* v0_ep_;
		RenderEffectParameter* v1_ep_;
		RenderEffectParameter* color_ep_;
	};

	class KLAYGE_CORE_API RenderableTriangle : public Renderable
	{
	public:
		explicit RenderableTriangle();
		RenderableTriangle(float3 const & v0, float3 const & v1, float3 const & v2, Color const & clr);

		void SetTriangle(float3 const & v0, float3 const & v1, float3 const & v2);
		void SetColor(Color const & clr);

	private:
		RenderEffectParameter* v0_ep_;
		RenderEffectParameter* v1_ep_;
		RenderEffectParameter* v2_ep_;
		RenderEffectParameter* color_ep_;
	};

	class KLAYGE_CORE_API RenderableTriBox : public Renderable
	{
	public:
		explicit RenderableTriBox();
		RenderableTriBox(OBBox const & obb, Color const & clr);

		void SetBox(OBBox const & obb);
		void SetColor(Color const & clr);

	private:
		RenderEffectParameter* v0_ep_;
		RenderEffectParameter* v1_ep_;
		RenderEffectParameter* v2_ep_;
		RenderEffectParameter* v3_ep_;
		RenderEffectParameter* v4_ep_;
		RenderEffectParameter* v5_ep_;
		RenderEffectParameter* v6_ep_;
		RenderEffectParameter* v7_ep_;
		RenderEffectParameter* color_ep_;
	};

	class KLAYGE_CORE_API RenderableLineBox : public Renderable
	{
	public:
		explicit RenderableLineBox();
		RenderableLineBox(OBBox const & obb, Color const & clr);

		void SetBox(OBBox const & obb);
		void SetColor(Color const & clr);

	private:
		RenderEffectParameter* v0_ep_;
		RenderEffectParameter* v1_ep_;
		RenderEffectParameter* v2_ep_;
		RenderEffectParameter* v3_ep_;
		RenderEffectParameter* v4_ep_;
		RenderEffectParameter* v5_ep_;
		RenderEffectParameter* v6_ep_;
		RenderEffectParameter* v7_ep_;
		RenderEffectParameter* color_ep_;
	};

	class KLAYGE_CORE_API RenderablePlane : public Renderable
	{
	public:
		RenderablePlane(float length, float width, int length_segs, int width_segs,
			bool has_tex_coord, bool has_tangent);
	};

	class KLAYGE_CORE_API RenderDecal : public Renderable
	{
	public:
		RenderDecal(TexturePtr const & normal_tex, TexturePtr const & albedo_tex,
			float3 const & albedo_clr, float metalness, float glossiness);

		void OnRenderBegin();

	private:
		RenderEffectParameter* g_buffer_rt0_tex_param_;
	};
}

#endif		// _RENDERABLEHELPER_HPP
