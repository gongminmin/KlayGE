/**
 * @file MInput.hpp
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

#ifndef _MINPUT_HPP
#define _MINPUT_HPP

#pragma once

#include <windows.h>
#include <hidsdi.h>

#include <KlayGE/Input.hpp>
#include <boost/noncopyable.hpp>

namespace KlayGE
{
	class MsgInputEngine : boost::noncopyable, public InputEngine
	{
	public:
		MsgInputEngine();
		~MsgInputEngine();

		std::wstring const & Name() const;
		void EnumDevices();

		NTSTATUS HidP_GetCaps(PHIDP_PREPARSED_DATA PreparsedData, PHIDP_CAPS Capabilities) const;
		NTSTATUS HidP_GetButtonCaps(HIDP_REPORT_TYPE ReportType, PHIDP_BUTTON_CAPS ButtonCaps,
			PUSHORT ButtonCapsLength, PHIDP_PREPARSED_DATA PreparsedData) const;
		NTSTATUS HidP_GetValueCaps(HIDP_REPORT_TYPE ReportType, PHIDP_VALUE_CAPS ValueCaps,
			PUSHORT ValueCapsLength, PHIDP_PREPARSED_DATA PreparsedData) const;
		NTSTATUS HidP_GetUsages(HIDP_REPORT_TYPE ReportType, USAGE UsagePage,
			USHORT LinkCollection, PUSAGE UsageList, PULONG UsageLength, PHIDP_PREPARSED_DATA PreparsedData,
			PCHAR Report, ULONG ReportLength) const;
		NTSTATUS HidP_GetUsageValue(HIDP_REPORT_TYPE ReportType, USAGE UsagePage,
			USHORT LinkCollection, USAGE Usage, PULONG UsageValue, PHIDP_PREPARSED_DATA PreparsedData,
			PCHAR Report, ULONG ReportLength) const;

	private:
		boost::signals2::connection on_raw_input_;

		HMODULE mod_hid_;
		typedef NTSTATUS (WINAPI *HidP_GetCapsFunc)(PHIDP_PREPARSED_DATA PreparsedData, PHIDP_CAPS Capabilities);
		typedef NTSTATUS (WINAPI *HidP_GetButtonCapsFunc)(HIDP_REPORT_TYPE ReportType, PHIDP_BUTTON_CAPS ButtonCaps,
			PUSHORT ButtonCapsLength, PHIDP_PREPARSED_DATA PreparsedData);
		typedef NTSTATUS (WINAPI *HidP_GetValueCapsFunc)(HIDP_REPORT_TYPE ReportType, PHIDP_VALUE_CAPS ValueCaps,
			PUSHORT ValueCapsLength, PHIDP_PREPARSED_DATA PreparsedData);
		typedef NTSTATUS (WINAPI *HidP_GetUsagesFunc)(HIDP_REPORT_TYPE ReportType, USAGE UsagePage,
			USHORT LinkCollection, PUSAGE UsageList, PULONG UsageLength, PHIDP_PREPARSED_DATA PreparsedData,
			PCHAR Report, ULONG ReportLength);
		typedef NTSTATUS (WINAPI *HidP_GetUsageValueFunc)(HIDP_REPORT_TYPE ReportType, USAGE UsagePage,
			USHORT LinkCollection, USAGE Usage, PULONG UsageValue, PHIDP_PREPARSED_DATA PreparsedData,
			PCHAR Report, ULONG ReportLength);
		HidP_GetCapsFunc DynamicHidP_GetCaps_;
		HidP_GetButtonCapsFunc DynamicHidP_GetButtonCaps_;
		HidP_GetValueCapsFunc DynamicHidP_GetValueCaps_;
		HidP_GetUsagesFunc DynamicHidP_GetUsages_;
		HidP_GetUsageValueFunc DynamicHidP_GetUsageValue_;

	private:
		void OnRawInput(Window const & wnd, uint64_t param);
	};

	class MsgInputDevice
	{
	public:
		virtual ~MsgInputDevice()
		{
		}

		virtual void OnRawInput(RAWINPUT const & ri) = 0;
	};

	class MsgInputKeyboard : public InputKeyboard, public MsgInputDevice
	{
	public:
		MsgInputKeyboard();

		virtual std::wstring const & Name() const KLAYGE_OVERRIDE;
		virtual void OnRawInput(RAWINPUT const & ri) KLAYGE_OVERRIDE;

	private:
		virtual void UpdateInputs() KLAYGE_OVERRIDE;

		array<bool, 256> keys_state_;
	};

	class MsgInputMouse : public InputMouse, public MsgInputDevice
	{
	public:
		MsgInputMouse();

		virtual std::wstring const & Name() const KLAYGE_OVERRIDE;
		virtual void OnRawInput(RAWINPUT const & ri) KLAYGE_OVERRIDE;

	private:
		virtual void UpdateInputs() KLAYGE_OVERRIDE;

		Vector_T<long, 3> offset_state_;
		array<bool, 8> buttons_state_;
	};

	class MsgInputJoystick : public InputJoystick, public MsgInputDevice
	{
	public:
		MsgInputJoystick();

		virtual std::wstring const & Name() const KLAYGE_OVERRIDE;
		virtual void OnRawInput(RAWINPUT const & ri) KLAYGE_OVERRIDE;

	private:
		virtual void UpdateInputs() KLAYGE_OVERRIDE;

		Vector_T<long, 3> pos_state_;
		Vector_T<long, 3> rot_state_;
		Vector_T<long, 2> slider_state_;
		array<bool, 32> buttons_state_;
	};
}

#endif		// _MINPUT_HPP
