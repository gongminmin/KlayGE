/**
 * @file D3D12Query.hpp
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

#ifndef _D3D12QUERY_HPP
#define _D3D12QUERY_HPP

#pragma once

#include <KlayGE/Query.hpp>

namespace KlayGE
{
	class D3D12OcclusionQuery : public OcclusionQuery
	{
	public:
		D3D12OcclusionQuery();

		void Begin();
		void End();

		uint64_t SamplesPassed();

	private:
		ID3D12QueryHeapPtr query_heap_;
		ID3D12ResourcePtr query_result_;
	};

	class D3D12ConditionalRender : public ConditionalRender
	{
	public:
		D3D12ConditionalRender();

		void Begin();
		void End();

		void BeginConditionalRender();
		void EndConditionalRender();

		bool AnySamplesPassed();

	private:
		ID3D12QueryHeapPtr predicate_heap_;
		ID3D12ResourcePtr predicate_result_;
	};

	class D3D12TimerQuery : public TimerQuery
	{
	public:
		D3D12TimerQuery();

		void Begin();
		void End();

		double TimeElapsed() override;

	private:
		ID3D12QueryHeapPtr timestamp_heap_;
		ID3D12ResourcePtr timestamp_result_;
	};
}

#endif		// _D3D12QUERY_HPP
