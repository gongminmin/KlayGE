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
	class D3D12OcclusionQuery final : public OcclusionQuery
	{
	public:
		D3D12OcclusionQuery();

		void Begin() override;
		void End() override;

		uint64_t SamplesPassed() override;

	private:
		ID3D12QueryHeapPtr query_heap_;
		ID3D12ResourcePtr query_result_;
	};

	class D3D12ConditionalRender final : public ConditionalRender
	{
	public:
		D3D12ConditionalRender();

		void Begin() override;
		void End() override;

		void BeginConditionalRender() override;
		void EndConditionalRender() override;

		bool AnySamplesPassed() override;

	private:
		ID3D12QueryHeapPtr predicate_heap_;
		ID3D12ResourcePtr predicate_result_;
	};

	class D3D12TimerQuery final : public TimerQuery
	{
	public:
		D3D12TimerQuery();

		void Begin() override;
		void End() override;

		double TimeElapsed() override;

	private:
		ID3D12QueryHeapPtr timestamp_heap_;
		ID3D12ResourcePtr timestamp_result_;
	};

	class D3D12SOStatisticsQuery final : public SOStatisticsQuery
	{
	public:
		D3D12SOStatisticsQuery();

		void Begin() override;
		void End() override;

		uint64_t NumPrimitivesWritten() override;
		uint64_t PrimitivesGenerated() override;

	private:
		ID3D12QueryHeapPtr so_stat_query_heap_;
		ID3D12ResourcePtr so_stat_query_result_;
	};
}

#endif		// _D3D12QUERY_HPP
