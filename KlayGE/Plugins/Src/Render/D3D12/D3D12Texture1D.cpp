/**
 * @file D3D12Texture1D.cpp
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
#include <KlayGE/D3D12/D3D12RenderLayout.hpp>

namespace KlayGE
{
	D3D12Texture1D::D3D12Texture1D(uint32_t width, uint32_t numMipMaps, uint32_t array_size, ElementFormat format,
						uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint)
					: D3D12Texture(TT_1D, sample_count, sample_quality, access_hint)
	{
		if (0 == numMipMaps)
		{
			numMipMaps = 1;
			uint32_t w = width;
			while (w != 1)
			{
				++ numMipMaps;

				w = std::max<uint32_t>(1U, w / 2);
			}
		}
		num_mip_maps_ = numMipMaps;

		array_size_ = array_size;
		format_		= format;
		dxgi_fmt_ = D3D12Mapping::MappingFormat(format_);
		width_ = width;
	}

	uint32_t D3D12Texture1D::Width(uint32_t level) const
	{
		BOOST_ASSERT(level < num_mip_maps_);

		return std::max<uint32_t>(1U, width_ >> level);
	}

	void D3D12Texture1D::CopyToTexture(Texture& target)
	{
		BOOST_ASSERT(type_ == target.Type());

		if ((this->Width(0) == target.Width(0)) && (this->Format() == target.Format()) && (this->NumMipMaps() == target.NumMipMaps()))
		{
			this->DoHWCopyToTexture(target);
		}
		else
		{
			uint32_t const array_size = std::min(this->ArraySize(), target.ArraySize());
			uint32_t const num_mips = std::min(this->NumMipMaps(), target.NumMipMaps());
			for (uint32_t index = 0; index < array_size; ++ index)
			{
				for (uint32_t level = 0; level < num_mips; ++ level)
				{
					this->ResizeTexture1D(target, index, level, 0, target.Width(level),
						index, level, 0, this->Width(level), true);
				}
			}
		}
	}

	void D3D12Texture1D::CopyToSubTexture1D(Texture& target,
			uint32_t dst_array_index, uint32_t dst_level, uint32_t dst_x_offset, uint32_t dst_width,
			uint32_t src_array_index, uint32_t src_level, uint32_t src_x_offset, uint32_t src_width)
	{
		BOOST_ASSERT(type_ == target.Type());

		if ((src_width == dst_width) && (this->Format() == target.Format()))
		{
			uint32_t const src_subres = CalcSubresource(src_level, src_array_index, 0,
				this->NumMipMaps(), this->ArraySize());
			uint32_t const dst_subres = CalcSubresource(dst_level, dst_array_index, 0,
				target.NumMipMaps(), target.ArraySize());

			this->DoHWCopyToSubTexture(target, dst_subres, dst_x_offset, 0, 0,
				src_subres, src_x_offset, 0, 0,
				src_width, 1, 1);
		}
		else
		{
			this->ResizeTexture1D(target, dst_array_index, dst_level, dst_x_offset, dst_width,
				src_array_index, src_level, src_x_offset, src_width, true);
		}
	}

	D3D12_SHADER_RESOURCE_VIEW_DESC D3D12Texture1D::FillSRVDesc(uint32_t first_array_index, uint32_t num_items, uint32_t first_level,
		uint32_t num_levels) const
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC desc;
		switch (format_)
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
			desc.Format = dxgi_fmt_;
			break;
		}
		desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

		if (array_size_ > 1)
		{
			desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1DARRAY;
			desc.Texture1DArray.MostDetailedMip = first_level;
			desc.Texture1DArray.MipLevels = num_levels;
			desc.Texture1DArray.ArraySize = num_items;
			desc.Texture1DArray.FirstArraySlice = first_array_index;
			desc.Texture1DArray.ResourceMinLODClamp = 0;
		}
		else
		{
			desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1D;
			desc.Texture1D.MostDetailedMip = first_level;
			desc.Texture1D.MipLevels = num_levels;
			desc.Texture1D.ResourceMinLODClamp = 0;
		}

		return desc;
	}

	D3D12_UNORDERED_ACCESS_VIEW_DESC D3D12Texture1D::FillUAVDesc(uint32_t first_array_index, uint32_t num_items, uint32_t level) const
	{
		D3D12_UNORDERED_ACCESS_VIEW_DESC desc;
		desc.Format = dxgi_fmt_;
		if (array_size_ > 1)
		{
			desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1DARRAY;
			desc.Texture1DArray.MipSlice = level;
			desc.Texture1DArray.ArraySize = num_items;
			desc.Texture1DArray.FirstArraySlice = first_array_index;
		}
		else
		{
			desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1D;
			desc.Texture1D.MipSlice = level;
		}

		return desc;
	}

	D3D12_RENDER_TARGET_VIEW_DESC D3D12Texture1D::FillRTVDesc(uint32_t first_array_index, uint32_t array_size, uint32_t level) const
	{
		D3D12_RENDER_TARGET_VIEW_DESC desc;
		desc.Format = D3D12Mapping::MappingFormat(this->Format());
		if (this->ArraySize() > 1)
		{
			desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE1DARRAY;
			desc.Texture1DArray.MipSlice = level;
			desc.Texture1DArray.ArraySize = array_size;
			desc.Texture1DArray.FirstArraySlice = first_array_index;
		}
		else
		{
			desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE1D;
			desc.Texture1D.MipSlice = level;
		}

		return desc;
	}

	D3D12_DEPTH_STENCIL_VIEW_DESC D3D12Texture1D::FillDSVDesc(uint32_t first_array_index, uint32_t array_size, uint32_t level) const
	{
		D3D12_DEPTH_STENCIL_VIEW_DESC desc;
		desc.Format = D3D12Mapping::MappingFormat(this->Format());
		desc.Flags = D3D12_DSV_FLAG_NONE;
		if (this->ArraySize() > 1)
		{
			desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE1DARRAY;
			desc.Texture1DArray.MipSlice = level;
			desc.Texture1DArray.ArraySize = array_size;
			desc.Texture1DArray.FirstArraySlice = first_array_index;
		}
		else
		{
			desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE1D;
			desc.Texture1D.MipSlice = level;
		}

		return desc;
	}

	void D3D12Texture1D::Map1D(uint32_t array_index, uint32_t level, TextureMapAccess tma,
			uint32_t x_offset, uint32_t width,
			void*& data)
	{
		uint32_t const subres = CalcSubresource(level, array_index, 0, num_mip_maps_, array_size_);

		uint32_t row_pitch;
		uint32_t slice_pitch;
		this->DoMap(subres, tma, x_offset, 0, 0, width, 1, 1, data, row_pitch, slice_pitch);
	}

	void D3D12Texture1D::Unmap1D(uint32_t array_index, uint32_t level)
	{
		uint32_t const subres = CalcSubresource(level, array_index, 0, num_mip_maps_, array_size_);
		this->DoUnmap(subres);
	}

	void D3D12Texture1D::BuildMipSubLevels()
	{
		// TODO
		// Depth stencil formats
		// Compression formats
		if (IsDepthFormat(format_) || IsCompressedFormat(format_))
		{
			for (uint32_t index = 0; index < this->ArraySize(); ++ index)
			{
				for (uint32_t level = 1; level < this->NumMipMaps(); ++ level)
				{
					this->ResizeTexture1D(*this, index, level, 0, this->Width(level),
						index, level - 1, 0, this->Width(level - 1), true);
				}
			}
		}
		else
		{
			D3D12RenderEngine& re = *checked_cast<D3D12RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			ID3D12DevicePtr const & device = re.D3DDevice();
			ID3D12GraphicsCommandListPtr const & cmd_list = re.D3DRenderCmdList();

			auto const & effect = *re.BlitEffect();
			auto const & tech = *re.BilinearBlitTech();
			auto& pass = tech.Pass(0);
			pass.Bind(effect);
			D3D12ShaderObjectPtr so = checked_pointer_cast<D3D12ShaderObject>(pass.GetShaderObject(effect));

			D3D12RenderLayout& rl = *checked_pointer_cast<D3D12RenderLayout>(re.PostProcessRenderLayout());

			D3D12_GRAPHICS_PIPELINE_STATE_DESC pso_desc;
			pso_desc.pRootSignature = so->RootSignature().get();
			{
				auto const & blob = so->ShaderBlob(ShaderObject::ST_VertexShader);
				if (blob && !blob->empty())
				{
					pso_desc.VS.pShaderBytecode = blob->data();
					pso_desc.VS.BytecodeLength = static_cast<UINT>(blob->size());
				}
				else
				{
					pso_desc.VS.pShaderBytecode = nullptr;
					pso_desc.VS.BytecodeLength = 0;
				}
			}
			{
				auto const & blob = so->ShaderBlob(ShaderObject::ST_PixelShader);
				if (blob && !blob->empty())
				{
					pso_desc.PS.pShaderBytecode = blob->data();
					pso_desc.PS.BytecodeLength = static_cast<UINT>(blob->size());
				}
				else
				{
					pso_desc.PS.pShaderBytecode = nullptr;
					pso_desc.PS.BytecodeLength = 0;
				}
			}
			{
				pso_desc.DS.pShaderBytecode = nullptr;
				pso_desc.DS.BytecodeLength = 0;
			}
			{
				pso_desc.HS.pShaderBytecode = nullptr;
				pso_desc.HS.BytecodeLength = 0;
			}
			{
				pso_desc.GS.pShaderBytecode = nullptr;
				pso_desc.GS.BytecodeLength = 0;
			}

			pso_desc.StreamOutput.pSODeclaration = nullptr;
			pso_desc.StreamOutput.NumEntries = 0;
			pso_desc.StreamOutput.pBufferStrides = nullptr;
			pso_desc.StreamOutput.NumStrides = 0;
			pso_desc.StreamOutput.RasterizedStream = 0;

			pso_desc.BlendState = checked_pointer_cast<D3D12RenderStateObject>(pass.GetRenderStateObject())->D3DBlendDesc();
			pso_desc.SampleMask = 0xFFFFFFFF;
			pso_desc.RasterizerState = checked_pointer_cast<D3D12RenderStateObject>(pass.GetRenderStateObject())->D3DRasterizerDesc();
			pso_desc.DepthStencilState = checked_pointer_cast<D3D12RenderStateObject>(pass.GetRenderStateObject())->D3DDepthStencilDesc();
			pso_desc.InputLayout.pInputElementDescs = &rl.InputElementDesc()[0];
			pso_desc.InputLayout.NumElements = static_cast<UINT>(rl.InputElementDesc().size());
			pso_desc.IBStripCutValue = (EF_R16UI == rl.IndexStreamFormat())
				? D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_0xFFFF : D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_0xFFFFFFFF;

			RenderLayout::topology_type tt = rl.TopologyType();
			pso_desc.PrimitiveTopologyType = D3D12Mapping::MappingPriTopoType(tt);

			pso_desc.NumRenderTargets = 1;
			pso_desc.RTVFormats[0] = dxgi_fmt_;
			for (uint32_t i = pso_desc.NumRenderTargets; i < sizeof(pso_desc.RTVFormats) / sizeof(pso_desc.RTVFormats[0]); ++i)
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

			cmd_list->SetPipelineState(pso.get());
			cmd_list->SetGraphicsRootSignature(so->RootSignature().get());

			ID3D12DescriptorHeapPtr cbv_srv_uav_heap = re.CreateDynamicCBVSRVUAVDescriptorHeap(array_size_ * (num_mip_maps_ - 1));
			ID3D12DescriptorHeapPtr sampler_heap = so->SamplerHeap();

			ID3D12DescriptorHeap* heaps[] = { cbv_srv_uav_heap.get(), sampler_heap.get() };
			cmd_list->SetDescriptorHeaps(sizeof(heaps) / sizeof(heaps[0]), heaps);

			if (sampler_heap)
			{
				D3D12_GPU_DESCRIPTOR_HANDLE gpu_sampler_handle = sampler_heap->GetGPUDescriptorHandleForHeapStart();
				cmd_list->SetGraphicsRootDescriptorTable(1, gpu_sampler_handle);
			}

			D3D12GraphicsBuffer& vb = *checked_pointer_cast<D3D12GraphicsBuffer>(rl.GetVertexStream(0));

			D3D12_VERTEX_BUFFER_VIEW vbv;
			vbv.BufferLocation = vb.D3DBuffer()->GetGPUVirtualAddress();
			vbv.SizeInBytes = vb.Size();
			vbv.StrideInBytes = rl.VertexSize(0);

			cmd_list->IASetVertexBuffers(0, 1, &vbv);

			cmd_list->IASetPrimitiveTopology(D3D12Mapping::Mapping(tt));

			D3D12_VIEWPORT vp;
			vp.TopLeftX = 0;
			vp.TopLeftY = 0;
			vp.MinDepth = 0;
			vp.MaxDepth = 1;

			D3D12_RECT scissor_rc;
			scissor_rc.left = 0;
			scissor_rc.top = 0;

			D3D12_RESOURCE_BARRIER barrier_before[2];
			barrier_before[0].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			barrier_before[0].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			barrier_before[0].Transition.pResource = d3d_texture_.get();
			barrier_before[0].Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
			barrier_before[0].Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
			barrier_before[1].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			barrier_before[1].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			barrier_before[1].Transition.pResource = d3d_texture_.get();
			barrier_before[1].Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
			barrier_before[1].Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

			D3D12_RESOURCE_BARRIER barrier_after[2];
			barrier_after[0].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			barrier_after[0].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			barrier_after[0].Transition.pResource = d3d_texture_.get();
			barrier_after[0].Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
			barrier_after[0].Transition.StateAfter = D3D12_RESOURCE_STATE_COMMON;
			barrier_after[1].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			barrier_after[1].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			barrier_after[1].Transition.pResource = d3d_texture_.get();
			barrier_after[1].Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
			barrier_after[1].Transition.StateAfter = D3D12_RESOURCE_STATE_COMMON;

			D3D12_CPU_DESCRIPTOR_HANDLE cpu_cbv_srv_uav_handle = cbv_srv_uav_heap->GetCPUDescriptorHandleForHeapStart();
			D3D12_GPU_DESCRIPTOR_HANDLE gpu_cbv_srv_uav_handle = cbv_srv_uav_heap->GetGPUDescriptorHandleForHeapStart();
			uint32_t const srv_desc_size = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
			for (uint32_t index = 0; index < array_size_; ++ index)
			{
				for (uint32_t level = 1; level < num_mip_maps_; ++ level)
				{
					cmd_list->SetGraphicsRootDescriptorTable(0, gpu_cbv_srv_uav_handle);

					barrier_before[0].Transition.Subresource = CalcSubresource(level - 1, index, 0, num_mip_maps_, array_size_);
					barrier_before[1].Transition.Subresource = CalcSubresource(level, index, 0, num_mip_maps_, array_size_);
					cmd_list->ResourceBarrier(2, barrier_before);

					D3D12_CPU_DESCRIPTOR_HANDLE const & rt_handle = this->RetriveD3DRenderTargetView(index, 1, level)->Handle();

					D3D12_CPU_DESCRIPTOR_HANDLE const & sr_handle = this->RetriveD3DShaderResourceView(index, 1, level - 1, 1)->Handle();
					device->CopyDescriptorsSimple(1, cpu_cbv_srv_uav_handle, sr_handle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

					cmd_list->OMSetRenderTargets(1, &rt_handle, false, nullptr);

					vp.Width = static_cast<float>(this->Width(level));
					vp.Height = static_cast<float>(this->Height(level));
					cmd_list->RSSetViewports(1, &vp);

					scissor_rc.right = this->Width(level);
					scissor_rc.bottom = this->Height(level);
					cmd_list->RSSetScissorRects(1, &scissor_rc);

					cmd_list->DrawInstanced(4, 1, 0, 0);

					barrier_after[0].Transition.Subresource = barrier_before[0].Transition.Subresource;
					barrier_after[1].Transition.Subresource = barrier_before[1].Transition.Subresource;
					cmd_list->ResourceBarrier(2, barrier_after);

					cpu_cbv_srv_uav_handle.ptr += srv_desc_size;
					gpu_cbv_srv_uav_handle.ptr += srv_desc_size;
				}
			}

			pass.Unbind(effect);
		}
	}

	void D3D12Texture1D::CreateHWResource(ElementInitData const * init_data)
	{
		this->DoCreateHWResource(D3D12_RESOURCE_DIMENSION_TEXTURE1D,
			width_, 1, 1, array_size_, init_data);
	}
}
