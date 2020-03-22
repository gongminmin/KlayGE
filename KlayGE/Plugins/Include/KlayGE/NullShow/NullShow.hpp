/**
 * @file NullShow.hpp
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

#ifndef KLAYGE_PLUGINS_NULL_SHOW_HPP
#define KLAYGE_PLUGINS_NULL_SHOW_HPP

#pragma once

#include <string>

#include <KlayGE/Show.hpp>

namespace KlayGE
{
	class NullShowEngine final : public ShowEngine
	{
	public:
		NullShowEngine();
		~NullShowEngine() override;

		bool IsComplete() override;

		void Load(std::string const & file_name) override;
		TexturePtr PresentTexture() override;

		ShowState State(long ms_timeout = -1) override;

	private:
		void DoSuspend() override;
		void DoResume() override;

		void DoPlay() override;
		void DoStop() override;
		void DoPause() override;
	};
}

#endif		// KLAYGE_PLUGINS_NULL_SHOW_HPP
