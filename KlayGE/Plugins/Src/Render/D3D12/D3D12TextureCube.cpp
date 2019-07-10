/**
 * @file D3D12TextureCube.cpp
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
#include <KFL/CXX17/iterator.hpp>
#include <KFL/Util.hpp>
#include <KFL/COMPtr.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/Texture.hpp>
#include <KlayGE/RenderEffect.hpp>

#include <cstring>

#include <KlayGE/D3D12/D3D12Typedefs.hpp>
#include <KlayGE/D3D12/D3D12RenderEngine.hpp>
#include <KlayGE/D3D12/D3D12Mapping.hpp>
#include <KlayGE/D3D12/D3D12Texture.hpp>
#include <KlayGE/D3D12/D3D12ShaderObject.hpp>
#include <KlayGE/D3D12/D3D12RenderStateObject.hpp>
#include <KlayGE/D3D12/D3D12FrameBuffer.hpp>
#include <KlayGE/D3D12/D3D12RenderLayout.hpp>

namespace KlayGE
{
	D3D12TextureCube::D3D12TextureCube(uint32_t size, uint32_t numMipMaps, uint32_t array_size, ElementFormat format,
						uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint)
					: D3D12Texture(TT_Cube, sample_count, sample_quality, access_hint)
	{
		if (0 == numMipMaps)
		{
			numMipMaps = 1;
			uint32_t w = size;
			while (w != 1)
			{
				++ numMipMaps;

				w = std::max<uint32_t>(1U, w / 2);
			}
		}
		num_mip_maps_ = numMipMaps;

		array_size_ = array_size;
		format_		= format;
		width_ = size;

		switch (format_)
		{
		case EF_D16:
			dxgi_fmt_ = DXGI_FORMAT_R16_TYPELESS;
			break;

		case EF_D24S8:
			dxgi_fmt_ = DXGI_FORMAT_R24G8_TYPELESS;
			break;

		case EF_D32F:
			dxgi_fmt_ = DXGI_FORMAT_R32_TYPELESS;
			break;

		default:
			dxgi_fmt_ = D3D12Mapping::MappingFormat(format_);
			break;
		}

		curr_states_.assign(6 * array_size_ * num_mip_maps_, D3D12_RESOURCE_STATE_COMMON);
	}

	uint32_t D3D12TextureCube::Width(uint32_t level) const
	{
		BOOST_ASSERT(level < num_mip_maps_);

		return std::max<uint32_t>(1U, width_ >> level);
	}

	uint32_t D3D12TextureCube::Height(uint32_t level) const
	{
		return this->Width(level);
	}

	void D3D12TextureCube::CopyToTexture(Texture& target)
	{
		BOOST_ASSERT(type_ == target.Type());

		if ((this->Width(0) == target.Width(0)) && (this->Format() == target.Format())
			&& (this->ArraySize() == target.ArraySize()) && (this->NumMipMaps() == target.NumMipMaps()))
		{
			this->DoHWCopyToTexture(target);
		}
		else
		{
			uint32_t const array_size = std::min(this->ArraySize(), target.ArraySize());
			uint32_t const num_mips = std::min(this->NumMipMaps(), target.NumMipMaps());
			for (uint32_t index = 0; index < array_size; ++ index)
			{
				for (int f = 0; f < 6; ++ f)
				{
					CubeFaces const face = static_cast<CubeFaces>(f);
					for (uint32_t level = 0; level < num_mips; ++ level)
					{
						this->ResizeTextureCube(target, index, face, level, 0, 0, target.Width(level), target.Height(level),
							index, face, level, 0, 0, this->Width(level), this->Height(level), true);
					}
				}
			}
		}
	}

	void D3D12TextureCube::CopyToSubTexture2D(Texture& target,
			uint32_t dst_array_index, uint32_t dst_level, uint32_t dst_x_offset, uint32_t dst_y_offset, uint32_t dst_width, uint32_t dst_height,
			uint32_t src_array_index, uint32_t src_level, uint32_t src_x_offset, uint32_t src_y_offset, uint32_t src_width, uint32_t src_height)
	{
		BOOST_ASSERT((TT_2D == target.Type()) || (TT_Cube == target.Type()));

		if ((src_width == dst_width) && (src_height == dst_height) && (this->Format() == target.Format()))
		{
			uint32_t const src_subres = CalcSubresource(src_level, src_array_index, 0,
				this->NumMipMaps(), this->ArraySize() * 6);
			uint32_t const dst_subres = CalcSubresource(dst_level, dst_array_index, 0,
				target.NumMipMaps(), target.ArraySize() * 6);

			this->DoHWCopyToSubTexture(target, dst_subres, dst_x_offset, dst_y_offset, 0,
				src_subres, src_x_offset, src_y_offset, 0,
				src_width, src_height, 1);
		}
		else
		{
			this->ResizeTexture2D(target, dst_array_index, dst_level, dst_x_offset, dst_y_offset, dst_width, dst_height,
				src_array_index, src_level, src_x_offset, src_y_offset, src_width, src_height, true);
		}
	}

	void D3D12TextureCube::CopyToSubTextureCube(Texture& target,
			uint32_t dst_array_index, CubeFaces dst_face, uint32_t dst_level, uint32_t dst_x_offset, uint32_t dst_y_offset, uint32_t dst_width, uint32_t dst_height,
			uint32_t src_array_index, CubeFaces src_face, uint32_t src_level, uint32_t src_x_offset, uint32_t src_y_offset, uint32_t src_width, uint32_t src_height)
	{
		BOOST_ASSERT(type_ == target.Type());

		if ((src_width == dst_width) && (src_height == dst_height) && (this->Format() == target.Format()))
		{
			uint32_t const src_subres = CalcSubresource(src_level, src_array_index * 6 + src_face - CF_Positive_X, 0,
				this->NumMipMaps(), this->ArraySize() * 6);
			uint32_t const dst_subres = CalcSubresource(dst_level, dst_array_index * 6 + dst_face - CF_Positive_X, 0,
				target.NumMipMaps(), target.ArraySize() * 6);

			this->DoHWCopyToSubTexture(target, dst_subres, dst_x_offset, dst_y_offset, 0,
				src_subres, src_x_offset, src_y_offset, 0,
				src_width, src_height, 1);
		}
		else
		{
			this->ResizeTextureCube(target, dst_array_index, dst_face, dst_level, dst_x_offset, dst_y_offset, dst_width, dst_height,
				src_array_index, src_face, src_level, src_x_offset, src_y_offset, src_width, src_height, true);
		}
	}

	D3D12_SHADER_RESOURCE_VIEW_DESC D3D12TextureCube::FillSRVDesc(ElementFormat pf, uint32_t first_array_index, uint32_t array_size,
		uint32_t first_level, uint32_t num_levels) const
	{
		BOOST_ASSERT(0 == first_array_index);
		BOOST_ASSERT(1 == array_size);
		KFL_UNUSED(first_array_index);
		KFL_UNUSED(array_size);

		D3D12_SHADER_RESOURCE_VIEW_DESC desc;
		switch (pf)
		{
		case EF_D16:
			desc.Format = DXGI_FORMAT_R16_UNORM;
			break;

		case EF_D24S8:
			desc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
			break;

		case EF_D32F:
			desc.Format = DXGI_FORMAT_R32_FLOAT;
			break;

		default:
			desc.Format = D3D12Mapping::MappingFormat(pf);
			break;
		}
		desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

		desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
		desc.TextureCube.MostDetailedMip = first_level;
		desc.TextureCube.MipLevels = num_levels;
		desc.TextureCube.ResourceMinLODClamp = 0;

		return desc;
	}

	D3D12_RENDER_TARGET_VIEW_DESC D3D12TextureCube::FillRTVDesc(ElementFormat pf, uint32_t first_array_index, uint32_t array_size,
		uint32_t level) const
	{
		D3D12_RENDER_TARGET_VIEW_DESC desc;
		desc.Format = D3D12Mapping::MappingFormat(pf);
		if (this->SampleCount() > 1)
		{
			desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMSARRAY;
		}
		else
		{
			desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
		}
		desc.Texture2DArray.MipSlice = level;
		desc.Texture2DArray.ArraySize = array_size * 6;
		desc.Texture2DArray.FirstArraySlice = first_array_index * 6;
		desc.Texture2DArray.PlaneSlice = (desc.Format == DXGI_FORMAT_X24_TYPELESS_G8_UINT) ? 1 : 0;

		return desc;
	}

	D3D12_RENDER_TARGET_VIEW_DESC D3D12TextureCube::FillRTVDesc(ElementFormat pf, uint32_t array_index, CubeFaces face,
		uint32_t level) const
	{
		BOOST_ASSERT(this->AccessHint() & EAH_GPU_Write);

		uint32_t const first_slice = array_index * 6 + face - CF_Positive_X;

		D3D12_RENDER_TARGET_VIEW_DESC desc;
		desc.Format = D3D12Mapping::MappingFormat(pf);
		if (this->SampleCount() > 1)
		{
			desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMSARRAY;
			desc.Texture2DMSArray.FirstArraySlice = first_slice;
			desc.Texture2DMSArray.ArraySize = 1;
		}
		else
		{
			desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
			desc.Texture2DArray.MipSlice = level;
			desc.Texture2DArray.FirstArraySlice = first_slice;
			desc.Texture2DArray.ArraySize = 1;
			desc.Texture2DArray.PlaneSlice = (desc.Format == DXGI_FORMAT_X24_TYPELESS_G8_UINT) ? 1 : 0;
		}

		return desc;
	}

	D3D12_DEPTH_STENCIL_VIEW_DESC D3D12TextureCube::FillDSVDesc(ElementFormat pf, uint32_t first_array_index, uint32_t array_size,
		uint32_t level) const
	{
		D3D12_DEPTH_STENCIL_VIEW_DESC desc;
		desc.Format = D3D12Mapping::MappingFormat(pf);
		desc.Flags = D3D12_DSV_FLAG_NONE;
		if (this->SampleCount() > 1)
		{
			desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMSARRAY;
			desc.Texture2DMSArray.FirstArraySlice = first_array_index * 6;
			desc.Texture2DMSArray.ArraySize = array_size * 6;
		}
		else
		{
			desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
			desc.Texture2DArray.MipSlice = level;
			desc.Texture2DArray.FirstArraySlice = first_array_index * 6;
			desc.Texture2DArray.ArraySize = array_size * 6;
		}

		return desc;
	}

	D3D12_DEPTH_STENCIL_VIEW_DESC D3D12TextureCube::FillDSVDesc(ElementFormat pf, uint32_t array_index, CubeFaces face,
		uint32_t level) const
	{
		BOOST_ASSERT(this->AccessHint() & EAH_GPU_Write);

		uint32_t const first_slice = array_index * 6 + face - CF_Positive_X;

		D3D12_DEPTH_STENCIL_VIEW_DESC desc;
		desc.Format = D3D12Mapping::MappingFormat(pf);
		desc.Flags = D3D12_DSV_FLAG_NONE;
		if (this->SampleCount() > 1)
		{
			desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMSARRAY;
			desc.Texture2DMSArray.FirstArraySlice = first_slice;
			desc.Texture2DMSArray.ArraySize = 1;
		}
		else
		{
			desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
			desc.Texture2DArray.MipSlice = level;
			desc.Texture2DArray.FirstArraySlice = first_slice;
			desc.Texture2DArray.ArraySize = 1;
		}

		return desc;
	}

	D3D12_UNORDERED_ACCESS_VIEW_DESC D3D12TextureCube::FillUAVDesc(ElementFormat pf, uint32_t first_array_index, uint32_t array_size,
		uint32_t level) const
	{
		return this->FillUAVDesc(pf, first_array_index, array_size, CF_Positive_X, 6, level);
	}

	D3D12_UNORDERED_ACCESS_VIEW_DESC D3D12TextureCube::FillUAVDesc(ElementFormat pf, uint32_t first_array_index, uint32_t array_size,
		CubeFaces first_face, uint32_t num_faces, uint32_t level) const
	{
		BOOST_ASSERT(this->AccessHint() & EAH_GPU_Read);

		D3D12_UNORDERED_ACCESS_VIEW_DESC desc;
		desc.Format = D3D12Mapping::MappingFormat(pf);
		desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
		desc.Texture2DArray.MipSlice = level;
		desc.Texture2DArray.ArraySize = array_size * 6 + num_faces;
		desc.Texture2DArray.FirstArraySlice = first_array_index * 6 + first_face;
		desc.Texture2DArray.PlaneSlice = (desc.Format == DXGI_FORMAT_X24_TYPELESS_G8_UINT) ? 1 : 0;

		return desc;
	}

	void D3D12TextureCube::MapCube(uint32_t array_index, CubeFaces face, uint32_t level, TextureMapAccess tma,
			uint32_t x_offset, uint32_t y_offset, uint32_t width, uint32_t height,
			void*& data, uint32_t& row_pitch)
	{
		uint32_t const subres = CalcSubresource(level, array_index * 6 + face - CF_Positive_X, 0, num_mip_maps_, array_size_);

		uint32_t slice_pitch;
		this->DoMap(subres, tma, x_offset, y_offset, 0, width, height, 1, data, row_pitch, slice_pitch);
	}

	void D3D12TextureCube::UnmapCube(uint32_t array_index, CubeFaces face, uint32_t level)
	{
		uint32_t const subres = CalcSubresource(level, array_index * 6 + face - CF_Positive_X, 0, num_mip_maps_, array_size_);
		this->DoUnmap(subres);
	}

	void D3D12TextureCube::BuildMipSubLevels()
	{
		// TODO
		// Depth stencil formats
		// Compression formats
		if (IsDepthFormat(format_) || IsCompressedFormat(format_))
		{
			for (uint32_t index = 0; index < this->ArraySize(); ++ index)
			{
				for (int f = 0; f < 6; ++ f)
				{
					CubeFaces const face = static_cast<CubeFaces>(f);
					for (uint32_t level = 1; level < this->NumMipMaps(); ++ level)
					{
						this->ResizeTextureCube(*this, index, face, level, 0, 0, this->Width(level), this->Height(level),
							index, face, level - 1, 0, 0, this->Width(level - 1), this->Height(level - 1), true);
					}
				}
			}
		}
		else
		{
			auto& re = checked_cast<D3D12RenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			ID3D12Device*device = re.D3DDevice();
			ID3D12GraphicsCommandList* cmd_list = re.D3DRenderCmdList();

			auto const & effect = *re.BlitEffect();
			auto const & tech = *re.BilinearBlitTech();
			auto& pass = tech.Pass(0);
			pass.Bind(effect);
			auto& d3d12_so = checked_cast<D3D12ShaderObject&>(*pass.GetShaderObject(effect));

			auto& d3d12_rl = checked_cast<D3D12RenderLayout&>(*re.PostProcessRenderLayout());

			D3D12_GRAPHICS_PIPELINE_STATE_DESC pso_desc;
			d3d12_so.UpdatePsoDesc(pso_desc);

			pso_desc.StreamOutput.pSODeclaration = nullptr;
			pso_desc.StreamOutput.NumEntries = 0;
			pso_desc.StreamOutput.pBufferStrides = nullptr;
			pso_desc.StreamOutput.NumStrides = 0;
			pso_desc.StreamOutput.RasterizedStream = 0;

			pso_desc.BlendState = checked_pointer_cast<D3D12RenderStateObject>(pass.GetRenderStateObject())->D3DBlendDesc();
			pso_desc.SampleMask = 0xFFFFFFFF;
			pso_desc.RasterizerState = checked_pointer_cast<D3D12RenderStateObject>(pass.GetRenderStateObject())->D3DRasterizerDesc();
			pso_desc.DepthStencilState = checked_pointer_cast<D3D12RenderStateObject>(pass.GetRenderStateObject())->D3DDepthStencilDesc();
			pso_desc.InputLayout.pInputElementDescs = &d3d12_rl.InputElementDesc()[0];
			pso_desc.InputLayout.NumElements = static_cast<UINT>(d3d12_rl.InputElementDesc().size());
			pso_desc.IBStripCutValue = (EF_R16UI == d3d12_rl.IndexStreamFormat())
				? D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_0xFFFF : D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_0xFFFFFFFF;

			RenderLayout::topology_type tt = d3d12_rl.TopologyType();
			pso_desc.PrimitiveTopologyType = D3D12Mapping::MappingPriTopoType(tt);

			pso_desc.NumRenderTargets = 1;
			pso_desc.RTVFormats[0] = dxgi_fmt_;
			for (uint32_t i = pso_desc.NumRenderTargets; i < std::size(pso_desc.RTVFormats); ++ i)
			{
				pso_desc.RTVFormats[i] = DXGI_FORMAT_UNKNOWN;
			}
			pso_desc.DSVFormat = DXGI_FORMAT_UNKNOWN;
			pso_desc.SampleDesc.Count = 1;
			pso_desc.SampleDesc.Quality = 0;
			pso_desc.NodeMask = 0;
			pso_desc.CachedPSO.pCachedBlob = nullptr;
			pso_desc.CachedPSO.CachedBlobSizeInBytes = 0;
			pso_desc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

			ID3D12PipelineStatePtr const & pso = re.CreateRenderPSO(pso_desc);

			re.SetPipelineState(pso.get());
			re.SetGraphicsRootSignature(d3d12_so.RootSignature());

			ID3D12DescriptorHeapPtr cbv_srv_uav_heap = re.CreateDynamicCBVSRVUAVDescriptorHeap(array_size_ * 6 * (num_mip_maps_ - 1));
			auto sampler_heap = d3d12_so.SamplerHeap();

			std::array<ID3D12DescriptorHeap*, 2> heaps;
			uint32_t num_heaps = 0;
			{
				heaps[num_heaps] = cbv_srv_uav_heap.get();
				++ num_heaps;
			}
			if (sampler_heap)
			{
				heaps[num_heaps] = sampler_heap;
				++ num_heaps;
			}
			re.SetDescriptorHeaps(MakeSpan(heaps.data(), num_heaps));

			if (sampler_heap)
			{
				D3D12_GPU_DESCRIPTOR_HANDLE gpu_sampler_handle = sampler_heap->GetGPUDescriptorHandleForHeapStart();
				cmd_list->SetGraphicsRootDescriptorTable(1, gpu_sampler_handle);
			}

			auto& vb = checked_cast<D3D12GraphicsBuffer&>(*d3d12_rl.GetVertexStream(0));

			D3D12_VERTEX_BUFFER_VIEW vbv;
			vbv.BufferLocation = vb.GPUVirtualAddress();
			vbv.SizeInBytes = vb.Size();
			vbv.StrideInBytes = d3d12_rl.VertexSize(0);

			re.IASetVertexBuffers(0, MakeSpan<1>(vbv));

			re.IASetPrimitiveTopology(tt);

			D3D12_VIEWPORT vp;
			vp.TopLeftX = 0;
			vp.TopLeftY = 0;
			vp.MinDepth = 0;
			vp.MaxDepth = 1;

			D3D12_RECT scissor_rc;
			scissor_rc.left = 0;
			scissor_rc.top = 0;

			D3D12_CPU_DESCRIPTOR_HANDLE cpu_cbv_srv_uav_handle = cbv_srv_uav_heap->GetCPUDescriptorHandleForHeapStart();
			D3D12_GPU_DESCRIPTOR_HANDLE gpu_cbv_srv_uav_handle = cbv_srv_uav_heap->GetGPUDescriptorHandleForHeapStart();
			uint32_t const srv_desc_size = re.CBVSRVUAVDescSize();
			for (uint32_t index = 0; index < array_size_; ++ index)
			{
				for (int f = 0; f < 6; ++ f)
				{
					for (uint32_t level = 1; level < num_mip_maps_; ++ level)
					{
						cmd_list->SetGraphicsRootDescriptorTable(0, gpu_cbv_srv_uav_handle);

						this->UpdateResourceBarrier(cmd_list, CalcSubresource(level - 1, index * 6 + f, 0, num_mip_maps_, array_size_),
							D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
						this->UpdateResourceBarrier(cmd_list, CalcSubresource(level, index * 6 + f, 0, num_mip_maps_, array_size_),
							D3D12_RESOURCE_STATE_RENDER_TARGET);
						re.FlushResourceBarriers(cmd_list);

						D3D12_CPU_DESCRIPTOR_HANDLE const & rt_handle
							= this->RetrieveD3DRenderTargetView(format_, index * 6 + f, 1, level)->Handle();

						D3D12_CPU_DESCRIPTOR_HANDLE const & sr_handle
							= this->RetrieveD3DShaderResourceView(format_, index * 6 + f, 1, level - 1, 1)->Handle();
						device->CopyDescriptorsSimple(1, cpu_cbv_srv_uav_handle, sr_handle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

						cmd_list->OMSetRenderTargets(1, &rt_handle, false, nullptr);

						vp.Width = static_cast<float>(this->Width(level));
						vp.Height = static_cast<float>(this->Height(level));
						re.RSSetViewports(1, &vp);

						scissor_rc.right = this->Width(level);
						scissor_rc.bottom = this->Height(level);
						re.RSSetScissorRects(scissor_rc);

						cmd_list->DrawInstanced(4, 1, 0, 0);

						cpu_cbv_srv_uav_handle.ptr += srv_desc_size;
						gpu_cbv_srv_uav_handle.ptr += srv_desc_size;
					}
				}
			}

			pass.Unbind(effect);

			auto& fb = checked_cast<D3D12FrameBuffer&>(*re.CurFrameBuffer());
			fb.SetRenderTargets();
		}
	}

	void D3D12TextureCube::CreateHWResource(std::span<ElementInitData const> init_data, float4 const * clear_value_hint)
	{
		this->DoCreateHWResource(D3D12_RESOURCE_DIMENSION_TEXTURE2D,
			width_, width_, 1, array_size_ * 6, init_data, clear_value_hint);
	}
}
