/**
 * @file D3D12RenderStateObject.hpp
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

#ifndef _D3D12RENDERSTATEOBJECT_HPP
#define _D3D12RENDERSTATEOBJECT_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/RenderStateObject.hpp>

namespace KlayGE
{
	class D3D12RasterizerStateObject : public RasterizerStateObject
	{
	public:
		explicit D3D12RasterizerStateObject(RasterizerStateDesc const & desc);

		void Active();

		D3D12_RASTERIZER_DESC const & D3DDesc() const
		{
			return rasterizer_desc_;
		}

	private:
		D3D12_RASTERIZER_DESC rasterizer_desc_;
	};

	class D3D12DepthStencilStateObject : public DepthStencilStateObject
	{
	public:
		explicit D3D12DepthStencilStateObject(DepthStencilStateDesc const & desc);

		void Active(uint16_t front_stencil_ref, uint16_t back_stencil_ref);

		D3D12_DEPTH_STENCIL_DESC const & D3DDesc() const
		{
			return depth_stencil_desc_;
		}

	private:
		D3D12_DEPTH_STENCIL_DESC depth_stencil_desc_;
	};

	class D3D12BlendStateObject : public BlendStateObject
	{
	public:
		explicit D3D12BlendStateObject(BlendStateDesc const & desc);

		void Active(Color const & blend_factor, uint32_t sample_mask);

		D3D12_BLEND_DESC const & D3DDesc() const
		{
			return blend_desc_;
		}

	private:
		D3D12_BLEND_DESC blend_desc_;
	};

	class D3D12SamplerStateObject : public SamplerStateObject
	{
	public:
		explicit D3D12SamplerStateObject(SamplerStateDesc const & desc);

		D3D12_SAMPLER_DESC const & D3DDesc() const
		{
			return sampler_desc_;
		}

	private:
		D3D12_SAMPLER_DESC sampler_desc_;
	};
}

#endif			// _D3D12RENDERSTATEOBJECT_HPP
