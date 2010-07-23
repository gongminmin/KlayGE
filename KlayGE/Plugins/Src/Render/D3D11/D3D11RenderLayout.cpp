// D3D11RenderLayout.cpp
// KlayGE D3D11渲染布局类 实现文件
// Ver 3.8.0
// 版权所有(C) 龚敏敏, 2009
// Homepage: http://www.klayge.org
//
// 3.8.0
// 初次建立 (2009.1.30)
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

#include <KlayGE/D3D11/D3D11RenderEngine.hpp>
#include <KlayGE/D3D11/D3D11Mapping.hpp>
#include <KlayGE/D3D11/D3D11GraphicsBuffer.hpp>
#include <KlayGE/D3D11/D3D11RenderLayout.hpp>

namespace KlayGE
{
	D3D11RenderLayout::D3D11RenderLayout()
		: dirty_decl_(true)
	{
	}

	ID3D11InputLayoutPtr const & D3D11RenderLayout::InputLayout(std::vector<D3D11_SIGNATURE_PARAMETER_DESC> const & signature, ID3DBlobPtr const & vs_code) const
	{
		if (dirty_decl_)
		{
			input_elems_type elems;
			elems.reserve(vertex_streams_.size());

			for (uint32_t i = 0; i < this->NumVertexStreams(); ++ i)
			{
				input_elems_type stream_elems;
				D3D11Mapping::Mapping(stream_elems, i, this->VertexStreamFormat(i), vertex_streams_[i].type, vertex_streams_[i].freq);
				elems.insert(elems.end(), stream_elems.begin(), stream_elems.end());
			}
			if (instance_stream_.stream)
			{
				input_elems_type stream_elems;
				D3D11Mapping::Mapping(stream_elems, this->NumVertexStreams(), this->InstanceStreamFormat(), instance_stream_.type, instance_stream_.freq);
				elems.insert(elems.end(), stream_elems.begin(), stream_elems.end());
			}

			D3D11RenderEngine& re = *checked_cast<D3D11RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			input_layout_ = re.CreateD3D11InputLayout(elems, signature, vs_code);

			dirty_decl_ = false;
		}

		return input_layout_;
	}
}
