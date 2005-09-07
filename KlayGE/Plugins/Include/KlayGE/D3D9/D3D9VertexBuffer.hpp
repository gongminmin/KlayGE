// D3D9VertexBuffer.hpp
// KlayGE D3D9顶点缓冲区类 头文件
// Ver 3.0.0
// 版权所有(C) 龚敏敏, 2005
// Homepage: http://klayge.sourceforge.net
//
// 3.0.0
// 初次建立 (2005.9.7)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _D3D9VERTEXBUFFER_HPP
#define _D3D9VERTEXBUFFER_HPP

#include <boost/smart_ptr.hpp>

#include <vector>
#include <d3d9.h>

#include <KlayGE/VertexBuffer.hpp>
#include <KlayGE/D3D9/D3D9Resource.hpp>

#ifdef KLAYGE_DEBUG
	#pragma comment(lib, "KlayGE_RenderEngine_D3D9_d.lib")
#else
	#pragma comment(lib, "KlayGE_RenderEngine_D3D9.lib")
#endif

namespace KlayGE
{
	class D3D9VertexBuffer : public VertexBuffer, public D3D9Resource
	{
	public:
		explicit D3D9VertexBuffer(BufferType type);
		~D3D9VertexBuffer();
		
		boost::shared_ptr<IDirect3DVertexDeclaration9> VertexDeclaration() const;

	private:
		void DoOnLostDevice();
		void DoOnResetDevice();

	private:
		typedef std::vector<D3DVERTEXELEMENT9> vertex_elems_type;
		vertex_elems_type vertex_elems_;

		mutable bool dirty_decl_;
		mutable boost::shared_ptr<IDirect3DVertexDeclaration9> decl_;
	};
}

#endif			// _D3D9VERTEXBUFFER_HPP