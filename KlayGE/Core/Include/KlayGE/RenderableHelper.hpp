// RenderableHelper.hpp
// KlayGE 一些常用的可渲染对象 头文件
// Ver 2.7.1
// 版权所有(C) 龚敏敏, 2005
// Homepage: http://klayge.sourceforge.net
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

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/Renderable.hpp>
#include <KlayGE/Box.hpp>

namespace KlayGE
{
	class RenderableHelper : public Renderable
	{
	public:
		RenderableHelper(std::wstring const & name);
		virtual ~RenderableHelper()
		{
		}

		virtual RenderEffectPtr GetRenderEffect() const;
		virtual VertexBufferPtr GetVertexBuffer() const;

		virtual Box GetBound() const;

		virtual std::wstring const & Name() const;

	protected:
		std::wstring name_;

		Box box_;

		VertexBufferPtr vb_;
		RenderEffectPtr effect_;
	};

	class RenderablePoint : public RenderableHelper
	{
	public:
		RenderablePoint(Vector3 const & v, Color const & clr);
		virtual ~RenderablePoint()
		{
		}

		void OnRenderBegin();

	private:
		Vector4 clr_;
	};

	class RenderableLine : public RenderableHelper
	{
	public:
		explicit RenderableLine(Vector3 const & v0, Vector3 const & v1, Color const & clr);
		virtual ~RenderableLine()
		{
		}

		void OnRenderBegin();

	private:
		Vector4 clr_;
	};

	class RenderableTriangle : public RenderableHelper
	{
	public:
		RenderableTriangle(Vector3 const & v0, Vector3 const & v1, Vector3 const & v2, Color const & clr);
		virtual ~RenderableTriangle()
		{
		}

		void OnRenderBegin();

	private:
		Vector4 clr_;
	};

	class RenderableBox : public RenderableHelper
	{
	public:
		explicit RenderableBox(Box const & box, Color const & clr);
		virtual ~RenderableBox()
		{
		}

		void OnRenderBegin();

	private:
		Vector4 clr_;
	};

	class RenderableSkyBox : public RenderableHelper
	{
	public:
		RenderableSkyBox();
		virtual ~RenderableSkyBox()
		{
		}

		void CubeMap(TexturePtr const & cube);

		void OnRenderBegin();

	private:
		SamplerPtr cube_sampler_;
	};

	class RenderablePlane : public RenderableHelper
	{
	public:
		RenderablePlane(float length, float width, int length_segs, int width_segs, bool has_tex_coord);
		virtual ~RenderablePlane()
		{
		}
	};
}

#endif		// _RENDERABLEHELPER_HPP
