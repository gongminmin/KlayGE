/**
 * @file D3D12RenderLayout.cpp
 * @author Minmin Gong
 *
 * @section DESCRIPTION
 *
 * This source file is part of KlayGE
 * For the latest info, see http://www.klayge.org
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * You may alternatively use this source under the terms of
 * the KlayGE Proprietary License (KPL). You can obtained such a license
 * from http://www.klayge.org/licensing/.
 */

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

#include <KlayGE/D3D12/D3D12RenderEngine.hpp>
#include <KlayGE/D3D12/D3D12Mapping.hpp>
#include <KlayGE/D3D12/D3D12GraphicsBuffer.hpp>
#include <KlayGE/D3D12/D3D12RenderLayout.hpp>

namespace KlayGE
{
	D3D12RenderLayout::D3D12RenderLayout()
	{
	}

	std::vector<D3D12_INPUT_ELEMENT_DESC> const & D3D12RenderLayout::InputElementDesc() const
	{
		if (vertex_elems_.empty())
		{
			std::vector<D3D12_INPUT_ELEMENT_DESC> elems;
			elems.reserve(vertex_streams_.size());

			for (uint32_t i = 0; i < this->NumVertexStreams(); ++ i)
			{
				std::vector<D3D12_INPUT_ELEMENT_DESC> stream_elems;
				D3D12Mapping::Mapping(stream_elems, i, this->VertexStreamFormat(i), vertex_streams_[i].type, vertex_streams_[i].freq);
				elems.insert(elems.end(), stream_elems.begin(), stream_elems.end());
			}
			if (instance_stream_.stream)
			{
				std::vector<D3D12_INPUT_ELEMENT_DESC> stream_elems;
				D3D12Mapping::Mapping(stream_elems, this->NumVertexStreams(), this->InstanceStreamFormat(), instance_stream_.type, instance_stream_.freq);
				elems.insert(elems.end(), stream_elems.begin(), stream_elems.end());
			}

			vertex_elems_.swap(elems);
		}

		return vertex_elems_;
	}

	void D3D12RenderLayout::Active() const
	{
		uint32_t const num_vertex_streams = this->NumVertexStreams();
		uint32_t const all_num_vertex_stream = num_vertex_streams + (this->InstanceStream() ? 1 : 0);

		if (streams_dirty_)
		{
			vbvs_.resize(all_num_vertex_stream);
			for (uint32_t i = 0; i < num_vertex_streams; ++ i)
			{
				D3D12GraphicsBuffer& d3dvb = *checked_cast<D3D12GraphicsBuffer*>(this->GetVertexStream(i).get());
				vbvs_[i].BufferLocation = d3dvb.D3DBuffer()->GetGPUVirtualAddress();
				vbvs_[i].SizeInBytes = d3dvb.Size();
				vbvs_[i].StrideInBytes = this->VertexSize(i);
			}

			if (this->InstanceStream())
			{
				uint32_t const number = num_vertex_streams;

				D3D12GraphicsBuffer& d3dvb = *checked_cast<D3D12GraphicsBuffer*>(this->InstanceStream().get());
				vbvs_[number].BufferLocation = d3dvb.D3DBuffer()->GetGPUVirtualAddress();
				vbvs_[number].SizeInBytes = d3dvb.Size();
				vbvs_[number].StrideInBytes = this->InstanceSize();
			}

			if (this->UseIndices())
			{
				D3D12GraphicsBuffer& ib = *checked_cast<D3D12GraphicsBuffer*>(this->GetIndexStream().get());
				ibv_.BufferLocation = ib.D3DBuffer()->GetGPUVirtualAddress();
				ibv_.SizeInBytes = ib.Size();
				ibv_.Format = D3D12Mapping::MappingFormat(index_format_);
			}

			streams_dirty_ = false;
		}

		D3D12RenderEngine& re = *checked_cast<D3D12RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		ID3D12GraphicsCommandListPtr const & cmd_list = re.D3DRenderCmdList();
		if (all_num_vertex_stream != 0)
		{
			cmd_list->IASetVertexBuffers(0, all_num_vertex_stream, &vbvs_[0]);
		}
		if (this->UseIndices())
		{
			cmd_list->IASetIndexBuffer(&ibv_);
		}
	}
}
