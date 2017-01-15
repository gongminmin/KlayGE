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

#if (defined KLAYGE_PLATFORM_WINDOWS) || (defined KLAYGE_PLATFORM_ANDROID) || (defined KLAYGE_PLATFORM_DARWIN)
namespace
{
	using namespace KlayGE;

	static int32_t const VK_MAPPING[] = 
	{
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		KS_BackSpace,
		KS_Tab,
		-1,
		-1,
		-1,
		KS_Enter,
		-1,
		-1,
		KS_LeftShift,
		KS_LeftCtrl,
		KS_LeftAlt,
		KS_Pause,
		KS_CapsLock,
		KS_Kana,
		-1,
		-1,
		-1,
		-1,
		-1,
		KS_Escape,
		KS_Convert,
		KS_NoConvert,
		-1,
		-1,
		KS_Space,
		KS_PageUp,
		KS_PageDown,
		KS_End,
		KS_Home,
		KS_LeftArrow,
		KS_UpArrow,
		KS_RightArrow,
		KS_DownArrow,
		-1,
		-1,
		-1,
		-1,
		KS_Insert,
		KS_Delete,
		-1,
		KS_0,
		KS_1,
		KS_2,
		KS_3,
		KS_4,
		KS_5,
		KS_6,
		KS_7,
		KS_8,
		KS_9,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		KS_A,
		KS_B,
		KS_C,
		KS_D,
		KS_E,
		KS_F,
		KS_G,
		KS_H,
		KS_I,
		KS_J,
		KS_K,
		KS_L,
		KS_M,
		KS_N,
		KS_O,
		KS_P,
		KS_Q,
		KS_R,
		KS_S,
		KS_T,
		KS_U,
		KS_V,
		KS_W,
		KS_X,
		KS_Y,
		KS_Z,
		KS_LeftWin,
		KS_RightWin,
		KS_Apps,
		-1,
		KS_Sleep,
		KS_NumPad0,
		KS_NumPad1,
		KS_NumPad2,
		KS_NumPad3,
		KS_NumPad4,
		KS_NumPad5,
		KS_NumPad6,
		KS_NumPad7,
		KS_NumPad8,
		KS_NumPad9,
		KS_NumPadStar,
		KS_Equals,
		KS_BackSlash,
		KS_Minus,
		-1,
		KS_Slash,
		KS_F1,
		KS_F2,
		KS_F3,
		KS_F4,
		KS_F5,
		KS_F6,
		KS_F7,
		KS_F8,
		KS_F9,
		KS_F10,
		KS_F11,
		KS_F12,
		KS_F13,
		KS_F14,
		KS_F15,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		KS_NumLock,
		KS_ScrollLock,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		KS_LeftShift,
		KS_RightShift,
		KS_LeftCtrl,
		KS_RightCtrl,
		KS_LeftAlt,
		KS_RightAlt,
		KS_WebBack,
		KS_WebForward,
		KS_WebRefresh,
		KS_WebStop,
		KS_WebSearch,
		KS_WebFavorites,
		KS_WebHome,
		KS_Mute,
		KS_VolumeDown,
		KS_VolumeUp,
		KS_NextTrack,
		KS_PrevTrack,
		KS_MediaStop,
		KS_PlayPause,
		KS_Mail,
		KS_MediaSelect,
		-1,
		-1,
		-1,
		-1,
		KS_Comma,
		KS_NumPadPlus,
		-1,
		KS_NumPadMinus,
		KS_NumPadPeriod,
		KS_Slash,
		KS_Grave,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		KS_LeftBracket,
		KS_BackSlash,
		KS_RightBracket,
		KS_Grave,
		-1,
		-1,
		-1,
		KS_OEM_102,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1
	};
}

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

#if defined KLAYGE_PLATFORM_WINDOWS_DESKTOP
	void MsgInputKeyboard::OnRawInput(RAWINPUT const & ri)
	{
		BOOST_ASSERT(RIM_TYPEKEYBOARD == ri.header.dwType);

		int32_t ks = VK_MAPPING[ri.data.keyboard.VKey];
		if (ks >= 0)
		{
			keys_state_[ks] = (RI_KEY_MAKE == (ri.data.keyboard.Flags & 1UL));
		}
	}
#elif defined(KLAYGE_PLATFORM_WINDOWS_STORE) || defined (KLAYGE_PLATFORM_ANDROID) || defined (KLAYGE_PLATFORM_DARWIN)
	void MsgInputKeyboard::OnKeyDown(uint32_t key)
	{
		// TODO: a VK_MAPPING for Android/Darwin
		int32_t ks = VK_MAPPING[key];
		if (ks >= 0)
		{
			keys_state_[ks] = 1;
		}
	}

	void MsgInputKeyboard::OnKeyUp(uint32_t key)
	{
		// TODO: a VK_MAPPING for Android/Darwin
		int32_t ks = VK_MAPPING[key];
		if (ks >= 0)
		{
			keys_state_[ks] = 0;
		}
	}
#endif

	void MsgInputKeyboard::UpdateInputs()
	{
		index_ = !index_;
		keys_[index_] = keys_state_;
	}
}
#endif
