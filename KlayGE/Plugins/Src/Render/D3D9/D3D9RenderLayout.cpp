// D3D9RenderLayout.cpp
// KlayGE D3D9渲染布局类 实现文件
// Ver 3.2.0
// 版权所有(C) 龚敏敏, 2006
// Homepage: http://klayge.sourceforge.net
//
// 3.2.0
// 初次建立 (2006.1.9)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/COMPtr.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/Context.hpp>

#include <algorithm>
#include <cstring>

#include <KlayGE/D3D9/D3D9RenderEngine.hpp>
#include <KlayGE/D3D9/D3D9Mapping.hpp>
#include <KlayGE/D3D9/D3D9GraphicsBuffer.hpp>
#include <KlayGE/D3D9/D3D9RenderLayout.hpp>

namespace KlayGE
{
	D3D9RenderLayout::D3D9RenderLayout()
		: dirty_decl_(true)
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

	ID3D9VertexDeclarationPtr D3D9RenderLayout::VertexDeclaration() const
	{
		if (dirty_decl_)
		{
			D3D9RenderEngine const & renderEngine(*checked_cast<D3D9RenderEngine const *>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance()));
			ID3D9DevicePtr d3d_device = renderEngine.D3DDevice();

			vertex_elems_type elems;
			elems.reserve(vertex_streams_.size() + 1);

			for (uint32_t i = 0; i < this->NumVertexStreams(); ++ i)
			{
				std::vector<D3DVERTEXELEMENT9> stream_elems;
				D3D9Mapping::Mapping(stream_elems, i, this->VertexStreamFormat(i));
				elems.insert(elems.end(), stream_elems.begin(), stream_elems.end());
			}
			if (instance_stream_.stream)
			{
				std::vector<D3DVERTEXELEMENT9> stream_elems;
				D3D9Mapping::Mapping(stream_elems, this->NumVertexStreams(), this->InstanceStreamFormat());
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
