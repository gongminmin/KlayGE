/**
 * @file NullShowEngine.cpp
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

#include "NullShow.hpp"

namespace KlayGE
{
	NullShowEngine::NullShowEngine()
	{
	}

	NullShowEngine::~NullShowEngine()
	{
	}

	void NullShowEngine::DoSuspend()
	{
	}

	void NullShowEngine::DoResume()
	{
	}

	void NullShowEngine::DoPlay()
	{
	}

	void NullShowEngine::DoPause()
	{
	}

	void NullShowEngine::DoStop()
	{
	}

	void NullShowEngine::Load([[maybe_unused]] std::string const & file_name)
	{
		state_ = SS_Stopped;
	}

	bool NullShowEngine::IsComplete()
	{
		return true;
	}

	ShowState NullShowEngine::State([[maybe_unused]] long ms_timeout)
	{
		return state_;
	}

	TexturePtr NullShowEngine::PresentTexture()
	{
		return TexturePtr();
	}
}
