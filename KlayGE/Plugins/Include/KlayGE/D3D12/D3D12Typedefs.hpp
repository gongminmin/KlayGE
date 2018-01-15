/**
 * @file D3D12Typedefs.hpp
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

#ifndef _D3D12TYPEDEFS_HPP
#define _D3D12TYPEDEFS_HPP

#pragma once

#include <KlayGE/SALWrapper.hpp>
#if defined(KLAYGE_COMPILER_CLANGC2)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmicrosoft-enum-value" // Ignore int enum
#endif
#include <dxgi1_6.h>
#if defined(KLAYGE_COMPILER_CLANGC2)
#pragma clang diagnostic pop
#endif
#if defined(KLAYGE_COMPILER_CLANGC2)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmicrosoft-enum-value" // Ignore int enum
#endif
#include <d3d12.h>
#if defined(KLAYGE_COMPILER_CLANGC2)
#pragma clang diagnostic pop
#endif
#if defined(KLAYGE_COMPILER_GCC) || defined(KLAYGE_COMPILER_CLANGC2)
#undef __out
#endif

namespace KlayGE
{
	typedef std::shared_ptr<IDXGIFactory4>				IDXGIFactory4Ptr;
	typedef std::shared_ptr<IDXGIFactory5>				IDXGIFactory5Ptr;
	typedef std::shared_ptr<IDXGIAdapter1>				IDXGIAdapter1Ptr;
	typedef std::shared_ptr<IDXGISwapChain3>			IDXGISwapChain3Ptr;
	typedef std::shared_ptr<IDXGISwapChain4>			IDXGISwapChain4Ptr;
	typedef std::shared_ptr<ID3D12Device>				ID3D12DevicePtr;
	typedef std::shared_ptr<ID3D12CommandQueue>			ID3D12CommandQueuePtr;
	typedef std::shared_ptr<ID3D12CommandAllocator>		ID3D12CommandAllocatorPtr;
	typedef std::shared_ptr<ID3D12GraphicsCommandList>	ID3D12GraphicsCommandListPtr;
	typedef std::shared_ptr<ID3D12DescriptorHeap>		ID3D12DescriptorHeapPtr;
	typedef std::shared_ptr<ID3D12QueryHeap>			ID3D12QueryHeapPtr;
	typedef std::shared_ptr<ID3D12Resource>				ID3D12ResourcePtr;
	typedef std::shared_ptr<ID3D12Fence>				ID3D12FencePtr;
	typedef std::shared_ptr<ID3D12PipelineState>		ID3D12PipelineStatePtr;
	typedef std::shared_ptr<ID3D12RootSignature>		ID3D12RootSignaturePtr;
}

#endif		// _D3D12TYPEDEFS_HPP
