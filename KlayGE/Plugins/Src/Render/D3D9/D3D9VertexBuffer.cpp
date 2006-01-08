// D3D9VertexBuffer.cpp
// KlayGE D3D9顶点缓冲区类 实现文件
// Ver 3.0.0
// 版权所有(C) 龚敏敏, 2005
// Homepage: http://klayge.sourceforge.net
//
// 3.0.0
// 初次建立 (2005.9.7)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/Util.hpp>

#include <algorithm>
#include <cstring>

#include <KlayGE/D3D9/D3D9RenderEngine.hpp>
#include <KlayGE/D3D9/D3D9Mapping.hpp>
#include <KlayGE/D3D9/D3D9VertexStream.hpp>
#include <KlayGE/D3D9/D3D9VertexBuffer.hpp>

namespace KlayGE
{
	D3D9RenderLayout::D3D9RenderLayout(buffer_type type)
		: RenderLayout(type),
			dirty_decl_(true)
	{
	}

	D3D9RenderLayout::~D3D9RenderLayout()
	{
		decl_.reset();
	}

	void D3D9RenderLayout::DoOnLostDevice()
	{
		dirty_decl_ = true;
	}
	
	void D3D9RenderLayout::DoOnResetDevice()
	{
		dirty_decl_ = true;
	}

	boost::shared_ptr<IDirect3DVertexDeclaration9> D3D9RenderLayout::VertexDeclaration() const
	{
		if (dirty_decl_)
		{
			D3D9RenderEngine const & renderEngine(*checked_cast<D3D9RenderEngine const *>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance()));
			boost::shared_ptr<IDirect3DDevice9> d3d_device = renderEngine.D3DDevice();

			vertex_elems_type elems;
			elems.reserve(vertex_streams_.size() + 1);

			std::vector<VertexStreamPtr> vss;
			for (uint32_t i = 0; i < this->NumVertexStreams(); ++ i)
			{
				vss.push_back(this->GetVertexStream(i));
			}
			if (instance_stream_.stream)
			{
				vss.push_back(instance_stream_.stream);
			}

			for (std::vector<VertexStreamPtr>::const_iterator iter = vss.begin(); iter != vss.end(); ++ iter)
			{
				VertexStream& stream(*(*iter));

				std::vector<D3DVERTEXELEMENT9> stream_elems;
				D3D9Mapping::Mapping(stream_elems, iter - vss.begin(), *this);
				elems.insert(elems.end(), stream_elems.begin(), stream_elems.end());
			}

			{
				D3DVERTEXELEMENT9 end_element;
				end_element.Stream		= 0xFF;
				end_element.Offset		= 0;
				end_element.Type		= D3DDECLTYPE_UNUSED;
				end_element.Method		= D3DDECLMETHOD_DEFAULT;
				end_element.Usage		= 0;
				end_element.UsageIndex	= 0;
				elems.push_back(end_element);
			}

			IDirect3DVertexDeclaration9* vertex_decl;
			TIF(d3d_device->CreateVertexDeclaration(&elems[0], &vertex_decl));
			decl_ = MakeCOMPtr(vertex_decl);

			dirty_decl_ = false;
		}

		return decl_;
	}
}
