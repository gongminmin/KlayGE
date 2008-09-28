// D3D10RenderLayout.cpp
// KlayGE D3D10渲染布局类 实现文件
// Ver 3.8.0
// 版权所有(C) 龚敏敏, 2008
// Homepage: http://klayge.sourceforge.net
//
// 3.8.0
// 初次建立 (2008.9.21)
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

#include <KlayGE/D3D10/D3D10RenderEngine.hpp>
#include <KlayGE/D3D10/D3D10Mapping.hpp>
#include <KlayGE/D3D10/D3D10GraphicsBuffer.hpp>
#include <KlayGE/D3D10/D3D10RenderLayout.hpp>

namespace KlayGE
{
	D3D10RenderLayout::D3D10RenderLayout()
		: dirty_decl_(true)
	{
	}

	ID3D10InputLayoutPtr D3D10RenderLayout::InputLayout(ID3D10BlobPtr const & vs_code) const
	{
		if (dirty_decl_)
		{
			D3D10RenderEngine const & renderEngine(*checked_cast<D3D10RenderEngine const *>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance()));
			ID3D10DevicePtr const & d3d_device = renderEngine.D3DDevice();

			input_elems_type elems;
			elems.reserve(vertex_streams_.size());

			for (uint32_t i = 0; i < this->NumVertexStreams(); ++ i)
			{
				input_elems_type stream_elems;
				D3D10Mapping::Mapping(stream_elems, i, this->VertexStreamFormat(i), vertex_streams_[i].type, vertex_streams_[i].freq);
				elems.insert(elems.end(), stream_elems.begin(), stream_elems.end());
			}
			if (instance_stream_.stream)
			{
				input_elems_type stream_elems;
				D3D10Mapping::Mapping(stream_elems, this->NumVertexStreams(), this->InstanceStreamFormat(), instance_stream_.type, instance_stream_.freq);
				elems.insert(elems.end(), stream_elems.begin(), stream_elems.end());
			}

			ID3D10InputLayout* input_layout;
			TIF(d3d_device->CreateInputLayout(&elems[0], elems.size(), vs_code->GetBufferPointer(), vs_code->GetBufferSize(), &input_layout));
			input_layout_ = MakeCOMPtr(input_layout);

			dirty_decl_ = false;
		}

		return input_layout_;
	}
}
