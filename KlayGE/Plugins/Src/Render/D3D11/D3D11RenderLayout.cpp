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
#include <KFL/Util.hpp>
#include <KFL/COMPtr.hpp>
#include <KFL/ThrowErr.hpp>
#include <KFL/Math.hpp>
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
	{
	}

	ID3D11InputLayout* D3D11RenderLayout::InputLayout(size_t signature, std::vector<uint8_t> const & vs_code) const
	{
		for (size_t i = 0; i < input_layouts_.size(); ++ i)
		{
			if (input_layouts_[i].first == signature)
			{
				return input_layouts_[i].second.get();
			}
		}

		std::vector<D3D11_INPUT_ELEMENT_DESC> elems;
		elems.reserve(vertex_streams_.size());

		for (uint32_t i = 0; i < this->NumVertexStreams(); ++ i)
		{
			std::vector<D3D11_INPUT_ELEMENT_DESC> stream_elems;
			D3D11Mapping::Mapping(stream_elems, i, this->VertexStreamFormat(i), vertex_streams_[i].type, vertex_streams_[i].freq);
			elems.insert(elems.end(), stream_elems.begin(), stream_elems.end());
		}
		if (instance_stream_.stream)
		{
			std::vector<D3D11_INPUT_ELEMENT_DESC> stream_elems;
			D3D11Mapping::Mapping(stream_elems, this->NumVertexStreams(), this->InstanceStreamFormat(), instance_stream_.type, instance_stream_.freq);
			elems.insert(elems.end(), stream_elems.begin(), stream_elems.end());
		}

		D3D11RenderEngine& re = *checked_cast<D3D11RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		ID3D11InputLayoutPtr const & ret = re.CreateD3D11InputLayout(elems, signature, vs_code);
		input_layouts_.emplace_back(signature, ret);

		return input_layouts_.back().second.get();
	}
}
