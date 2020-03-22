/**
 * @file Fence.hpp
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

#ifndef _FENCE_HPP
#define _FENCE_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>

namespace KlayGE
{
	class KLAYGE_CORE_API Fence : boost::noncopyable
	{
	public:
		enum FenceType
		{
			FT_Render,
			FT_Compute,
			FT_Copy
		};

	public:
		virtual ~Fence() noexcept;

		virtual uint64_t Signal(FenceType ft) = 0;
		virtual void Wait(uint64_t id) = 0;
		virtual bool Completed(uint64_t id) = 0;
	};
}

#endif		// _FENCE_HPP

