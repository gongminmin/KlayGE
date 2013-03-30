/**
 * @file MIKeyboard.cpp
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

#include <KlayGE/MsgInput/MInput.hpp>

namespace KlayGE
{
	MsgInputKeyboard::MsgInputKeyboard()
	{
		keys_state_.fill(false);
	}

	std::wstring const & MsgInputKeyboard::Name() const
	{
		static std::wstring const name(L"MsgInput Keyboard");
		return name;
	}

	void MsgInputKeyboard::OnRawInput(RAWINPUT const & ri)
	{
		if (RIM_TYPEKEYBOARD == ri.header.dwType)
		{
			switch (ri.data.keyboard.Flags)
			{
			case RI_KEY_MAKE:
				keys_state_[ri.data.keyboard.MakeCode] = true;
				break;

			case RI_KEY_BREAK:
				keys_state_[ri.data.keyboard.MakeCode] = false;
				break;
			}
		}
	}

	void MsgInputKeyboard::UpdateInputs()
	{
		index_ = !index_;
		keys_[index_] = keys_state_;
	}
}
