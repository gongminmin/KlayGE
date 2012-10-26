// RenderableHelper.hpp
// KlayGE 一些常用的可渲染对象 头文件
// Ver 3.9.0
// 版权所有(C) 龚敏敏, 2005-2010
// Homepage: http://www.klayge.org
//
// 3.10.0
// RenderableSkyBox和RenderableHDRSkyBox增加了Technique() (2010.1.4)
//
// 3.9.0
// 增加了RenderableHDRSkyBox (2009.5.4)
//
// 2.7.1
// 增加了RenderableHelper基类 (2005.7.10)
//
// 2.6.0
// 增加了RenderableSkyBox (2005.5.26)
//
// 2.5.0
// 增加了RenderablePoint，RenderableLine和RenderableTriangle (2005.4.13)
//
// 2.4.0
// 初次建立 (2005.3.22)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#ifndef _RENDERABLEHELPER_HPP
#define _RENDERABLEHELPER_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/Renderable.hpp>
#include <KlayGE/AABBox.hpp>

namespace KlayGE
{
	class KLAYGE_CORE_API RenderableHelper : public Renderable
	{
	public:
		RenderableHelper(std::wstring const & name);
		virtual ~RenderableHelper()
		{
		}

		virtual RenderTechniquePtr const & GetRenderTechnique() const;
		virtual RenderLayoutPtr const & GetRenderLayout() const;

		virtual AABBox const & PosBound() const;
		virtual AABBox const & TexcoordBound() const;

		virtual std::wstring const & Name() const;

	protected:
		std::wstring name_;

		AABBox pos_aabb_;
		AABBox tc_aabb_;

		RenderLayoutPtr rl_;

		RenderEffectParameterPtr color_ep_;
		RenderEffectParameterPtr matViewProj_ep_;
	};

	class KLAYGE_CORE_API RenderablePoint : public RenderableHelper
	{
	public:
		RenderablePoint(float3 const & v, Color const & clr);
		virtual ~RenderablePoint()
		{
		}

		void OnRenderBegin();
	};

	class KLAYGE_CORE_API RenderableLine : public RenderableHelper
	{
	public:
		explicit RenderableLine(float3 const & v0, float3 const & v1, Color const & clr);
		virtual ~RenderableLine()
		{
		}

		void OnRenderBegin();
	};

	class KLAYGE_CORE_API RenderableTriangle : public RenderableHelper
	{
	public:
		RenderableTriangle(float3 const & v0, float3 const & v1, float3 const & v2, Color const & clr);
		virtual ~RenderableTriangle()
		{
		}

		void OnRenderBegin();
	};

	class KLAYGE_CORE_API RenderableTriBox : public RenderableHelper
	{
	public:
		explicit RenderableTriBox(AABBox const & aabb, Color const & clr);
		virtual ~RenderableTriBox()
		{
		}

		void OnRenderBegin();
	};

	class KLAYGE_CORE_API RenderableLineBox : public RenderableHelper
	{
	public:
		explicit RenderableLineBox(AABBox const & aabb, Color const & clr);
		virtual ~RenderableLineBox()
		{
		}

		void OnRenderBegin();
	};

	class KLAYGE_CORE_API RenderableSkyBox : public RenderableHelper
	{
	public:
		RenderableSkyBox();
		virtual ~RenderableSkyBox()
		{
		}

		virtual void Technique(RenderTechniquePtr const & tech);
		void CubeMap(TexturePtr const & cube);

		void OnRenderBegin();

		// For deferred only

		virtual void Pass(PassType type);

	protected:
		RenderEffectParameterPtr depth_far_ep_;
		RenderEffectParameterPtr inv_mvp_ep_;
		RenderEffectParameterPtr skybox_cube_tex_ep_;
	};

	class KLAYGE_CORE_API RenderableHDRSkyBox : public RenderableSkyBox
	{
	public:
		RenderableHDRSkyBox();
		virtual ~RenderableHDRSkyBox()
		{
		}

		virtual void Technique(RenderTechniquePtr const & tech);
		void CompressedCubeMap(TexturePtr const & y_cube, TexturePtr const & c_cube);

	protected:
		RenderEffectParameterPtr skybox_Ccube_tex_ep_;
	};

	class KLAYGE_CORE_API RenderablePlane : public RenderableHelper
	{
	public:
		RenderablePlane(float length, float width, int length_segs, int width_segs, bool has_tex_coord);
		virtual ~RenderablePlane()
		{
		}
	};

	class KLAYGE_CORE_API RenderDecal : public RenderableHelper
	{
	public:
		RenderDecal(TexturePtr const & normal_tex, TexturePtr const & diffuse_tex, float3 const & diffuse_clr,
				TexturePtr const & specular_tex, float3 const & specular_level, float shininess);

		void OnRenderBegin();

	private:
		RenderEffectParameterPtr inv_mv_ep_;
		float3 diffuse_clr_;
		float specular_level_;
		float shininess_;
	};
}

#endif		// _RENDERABLEHELPER_HPP
