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
#include <KlayGE/D3D11/D3D11ShaderObject.hpp>

namespace KlayGE
{
	D3D11RenderLayout::D3D11RenderLayout()
	{
	}

	void D3D11RenderLayout::Active(ShaderObject const * so) const
	{
		if (streams_dirty_)
		{
			uint32_t const num_vertex_streams = this->NumVertexStreams();
			uint32_t const all_num_vertex_stream = num_vertex_streams + (this->InstanceStream() ? 1 : 0);

			vertex_elems_.clear();
			vertex_elems_.reserve(all_num_vertex_stream);

			vbs_.resize(all_num_vertex_stream);
			strides_.resize(all_num_vertex_stream);
			offsets_.resize(all_num_vertex_stream);
			for (uint32_t i = 0; i < num_vertex_streams; ++ i)
			{
				std::vector<D3D11_INPUT_ELEMENT_DESC> stream_elems;
				D3D11Mapping::Mapping(stream_elems, i, this->VertexStreamFormat(i), vertex_streams_[i].type, vertex_streams_[i].freq);
				vertex_elems_.insert(vertex_elems_.end(), stream_elems.begin(), stream_elems.end());

				GraphicsBuffer const * stream = this->GetVertexStream(i).get();

				D3D11GraphicsBuffer const & d3dvb = *checked_cast<D3D11GraphicsBuffer const *>(stream);
				vbs_[i] = d3dvb.D3DBuffer();
				strides_[i] = this->VertexSize(i);
				offsets_[i] = 0;
			}
			if (this->InstanceStream())
			{
				std::vector<D3D11_INPUT_ELEMENT_DESC> stream_elems;
				D3D11Mapping::Mapping(stream_elems, this->NumVertexStreams(), this->InstanceStreamFormat(),
					instance_stream_.type, instance_stream_.freq);
				vertex_elems_.insert(vertex_elems_.end(), stream_elems.begin(), stream_elems.end());

				uint32_t number = num_vertex_streams;
				GraphicsBuffer const * stream = this->InstanceStream().get();

				D3D11GraphicsBuffer const & d3dvb = *checked_cast<D3D11GraphicsBuffer const *>(stream);
				vbs_[number] = d3dvb.D3DBuffer();
				strides_[number] = this->InstanceSize();
				offsets_[number] = 0;
			}

			streams_dirty_ = false;
		}

		D3D11ShaderObject const & shader = *checked_cast<D3D11ShaderObject const *>(so);

		bool found = false;
		for (auto const & il : input_layouts_)
		{
			if (il.first == shader.VSSignature())
			{
				found = true;
				layout_ = il.second.get();
				break;
			}
		}

		if (!found)
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();
			D3D11RenderEngine& re = *checked_cast<D3D11RenderEngine*>(&rf.RenderEngineInstance());
			ID3D11InputLayoutPtr new_layout = re.CreateD3D11InputLayout(vertex_elems_, shader.VSSignature(), *shader.VSCode());
			layout_ = new_layout.get();
			input_layouts_.emplace_back(shader.VSSignature(), new_layout);
		}
	}
}
