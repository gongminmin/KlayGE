// RenderableHelper.hpp
// KlayGE 一些常用的可渲染对象 头文件
// Ver 2.5.0
// 版权所有(C) 龚敏敏, 2005
// Homepage: http://klayge.sourceforge.net
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
	class RenderablePoint : public Renderable
	{
	public:
		explicit RenderablePoint(Vector3 const & v);

		RenderEffectPtr GetRenderEffect() const;
		VertexBufferPtr GetVertexBuffer() const;
		std::wstring const & Name() const;

		Box GetBound() const;

	private:
		Box box_;

		VertexBufferPtr vb_;
		RenderEffectPtr effect_;
	};

	class RenderableLine : public Renderable
	{
	public:
		explicit RenderableLine(Vector3 const & v0, Vector3 const & v1);

		RenderEffectPtr GetRenderEffect() const;
		VertexBufferPtr GetVertexBuffer() const;
		std::wstring const & Name() const;

		Box GetBound() const;

	private:
		Box box_;

		VertexBufferPtr vb_;
		RenderEffectPtr effect_;
	};

	class RenderableTriangle : public Renderable
	{
	public:
		RenderableTriangle(Vector3 const & v0, Vector3 const & v1, Vector3 const & v2);

		RenderEffectPtr GetRenderEffect() const;
		VertexBufferPtr GetVertexBuffer() const;
		std::wstring const & Name() const;

		Box GetBound() const;

	private:
		Box box_;

		VertexBufferPtr vb_;
		RenderEffectPtr effect_;
	};

	class RenderableBox : public Renderable
	{
	public:
		explicit RenderableBox(Box const & box);

		RenderEffectPtr GetRenderEffect() const;
		VertexBufferPtr GetVertexBuffer() const;
		std::wstring const & Name() const;

		Box GetBound() const;

	private:
		Box box_;

		VertexBufferPtr vb_;
		RenderEffectPtr effect_;
	};
}

#endif		//_RENDERABLEHELPER_HPP
