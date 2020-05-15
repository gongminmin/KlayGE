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

#if defined KLAYGE_PLATFORM_WINDOWS_DESKTOP
#define INITGUID
#include <windows.h>
#if defined KLAYGE_HAVE_LIBOVR
#include <OVR.h>
#endif
#if (_WIN32_WINNT >= _WIN32_WINNT_WINBLUE)
#include <hidsdi.h>
#else
#ifndef _NTDEF_
typedef LONG NTSTATUS;
#endif

typedef USHORT USAGE, *PUSAGE;

#define HID_USAGE_PAGE_UNDEFINED                        ((USAGE) 0x00)
#define HID_USAGE_PAGE_GENERIC                          ((USAGE) 0x01)
#define HID_USAGE_PAGE_SIMULATION                       ((USAGE) 0x02)
#define HID_USAGE_PAGE_VR                               ((USAGE) 0x03)
#define HID_USAGE_PAGE_SPORT                            ((USAGE) 0x04)
#define HID_USAGE_PAGE_GAME                             ((USAGE) 0x05)
#define HID_USAGE_PAGE_KEYBOARD                         ((USAGE) 0x07)
#define HID_USAGE_PAGE_LED                              ((USAGE) 0x08)
#define HID_USAGE_PAGE_BUTTON                           ((USAGE) 0x09)
#define HID_USAGE_PAGE_ORDINAL                          ((USAGE) 0x0A)
#define HID_USAGE_PAGE_TELEPHONY                        ((USAGE) 0x0B)
#define HID_USAGE_PAGE_CONSUMER                         ((USAGE) 0x0C)
#define HID_USAGE_PAGE_DIGITIZER                        ((USAGE) 0x0D)
#define HID_USAGE_PAGE_UNICODE                          ((USAGE) 0x10)
#define HID_USAGE_PAGE_ALPHANUMERIC                     ((USAGE) 0x14)
#define HID_USAGE_PAGE_MICROSOFT_BLUETOOTH_HANDSFREE    ((USAGE) 0xFFF3)

#define HID_USAGE_GENERIC_POINTER      ((USAGE) 0x01)
#define HID_USAGE_GENERIC_MOUSE        ((USAGE) 0x02)
#define HID_USAGE_GENERIC_JOYSTICK     ((USAGE) 0x04)
#define HID_USAGE_GENERIC_GAMEPAD      ((USAGE) 0x05)
#define HID_USAGE_GENERIC_KEYBOARD     ((USAGE) 0x06)
#define HID_USAGE_GENERIC_KEYPAD       ((USAGE) 0x07)
#define HID_USAGE_GENERIC_SYSTEM_CTL   ((USAGE) 0x80)

#define HID_USAGE_GENERIC_X                        ((USAGE) 0x30)
#define HID_USAGE_GENERIC_Y                        ((USAGE) 0x31)
#define HID_USAGE_GENERIC_Z                        ((USAGE) 0x32)
#define HID_USAGE_GENERIC_RX                       ((USAGE) 0x33)
#define HID_USAGE_GENERIC_RY                       ((USAGE) 0x34)
#define HID_USAGE_GENERIC_RZ                       ((USAGE) 0x35)
#define HID_USAGE_GENERIC_SLIDER                   ((USAGE) 0x36)
#define HID_USAGE_GENERIC_DIAL                     ((USAGE) 0x37)

typedef enum _HIDP_REPORT_TYPE
{
	HidP_Input,
	HidP_Output,
	HidP_Feature
} HIDP_REPORT_TYPE;

typedef struct _HIDP_PREPARSED_DATA * PHIDP_PREPARSED_DATA;

typedef struct _HIDP_CAPS
{
	USAGE    Usage;
	USAGE    UsagePage;
	USHORT   InputReportByteLength;
	USHORT   OutputReportByteLength;
	USHORT   FeatureReportByteLength;
	USHORT   Reserved[17];

	USHORT   NumberLinkCollectionNodes;

	USHORT   NumberInputButtonCaps;
	USHORT   NumberInputValueCaps;
	USHORT   NumberInputDataIndices;

	USHORT   NumberOutputButtonCaps;
	USHORT   NumberOutputValueCaps;
	USHORT   NumberOutputDataIndices;

	USHORT   NumberFeatureButtonCaps;
	USHORT   NumberFeatureValueCaps;
	USHORT   NumberFeatureDataIndices;
} HIDP_CAPS, *PHIDP_CAPS;

typedef struct _HIDP_BUTTON_CAPS
{
	USAGE    UsagePage;
	UCHAR    ReportID;
	BOOLEAN  IsAlias;

	USHORT   BitField;
	USHORT   LinkCollection;   // A unique internal index pointer

	USAGE    LinkUsage;
	USAGE    LinkUsagePage;

	BOOLEAN  IsRange;
	BOOLEAN  IsStringRange;
	BOOLEAN  IsDesignatorRange;
	BOOLEAN  IsAbsolute;

	ULONG    Reserved[10];
	union {
		struct {
			USAGE    UsageMin,         UsageMax;
			USHORT   StringMin,        StringMax;
			USHORT   DesignatorMin,    DesignatorMax;
			USHORT   DataIndexMin,     DataIndexMax;
		} Range;
		struct  {
			USAGE    Usage,            Reserved1;
			USHORT   StringIndex,      Reserved2;
			USHORT   DesignatorIndex,  Reserved3;
			USHORT   DataIndex,        Reserved4;
		} NotRange;
	};

} HIDP_BUTTON_CAPS, *PHIDP_BUTTON_CAPS;

typedef struct _HIDP_VALUE_CAPS
{
	USAGE    UsagePage;
	UCHAR    ReportID;
	BOOLEAN  IsAlias;

	USHORT   BitField;
	USHORT   LinkCollection;   // A unique internal index pointer

	USAGE    LinkUsage;
	USAGE    LinkUsagePage;

	BOOLEAN  IsRange;
	BOOLEAN  IsStringRange;
	BOOLEAN  IsDesignatorRange;
	BOOLEAN  IsAbsolute;

	BOOLEAN  HasNull;        // Does this channel have a null report   union
	UCHAR    Reserved;
	USHORT   BitSize;        // How many bits are devoted to this value?

	USHORT   ReportCount;    // See Note below.  Usually set to 1.
	USHORT   Reserved2[5];

	ULONG    UnitsExp;
	ULONG    Units;

	LONG     LogicalMin,       LogicalMax;
	LONG     PhysicalMin,      PhysicalMax;

	union {
		struct {
			USAGE    UsageMin,         UsageMax;
			USHORT   StringMin,        StringMax;
			USHORT   DesignatorMin,    DesignatorMax;
			USHORT   DataIndexMin,     DataIndexMax;
		} Range;

		struct {
			USAGE    Usage,            Reserved1;
			USHORT   StringIndex,      Reserved2;
			USHORT   DesignatorIndex,  Reserved3;
			USHORT   DataIndex,        Reserved4;
		} NotRange;
	};
} HIDP_VALUE_CAPS, *PHIDP_VALUE_CAPS;

#ifndef FACILITY_HID_ERROR_CODE
#define FACILITY_HID_ERROR_CODE 0x11
#endif

#define HIDP_ERROR_CODES(SEV, CODE) \
		((NTSTATUS) (((SEV) << 28) | (FACILITY_HID_ERROR_CODE << 16) | (CODE)))

#define HIDP_STATUS_SUCCESS                  (HIDP_ERROR_CODES(0x0,0))
#endif

#if (defined KLAYGE_PLATFORM_WINDOWS_DESKTOP)
#if (_WIN32_WINNT < _WIN32_WINNT_WIN10)
#include <LocationApi.h>
#endif
#include <SensorsApi.h>
#include <Sensors.h>
#endif
#elif defined KLAYGE_PLATFORM_WINDOWS_STORE
#include <windows.h>
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable: 5205) // winrt::impl::implements_delegate doesn't have virtual destructor
#endif
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Devices.Geolocation.h>
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(pop)
#endif
#include <winrt/Windows.Devices.Sensors.h>

namespace uwp
{
	using winrt::event_token;

	using namespace winrt::Windows::Foundation;
	using namespace winrt::Windows::Devices::Geolocation;
	using namespace winrt::Windows::Devices::Sensors;
}
#elif defined KLAYGE_PLATFORM_ANDROID
#include <android/sensor.h>
#endif

#include <KlayGE/Input.hpp>
#include <KFL/com_ptr.hpp>
#include <KFL/Timer.hpp>

#include <array>

namespace KlayGE
{
	class MsgInputEngine final : public InputEngine
	{
	public:
		MsgInputEngine();
		~MsgInputEngine();

		std::wstring const & Name() const override;
		void EnumDevices() override;

#if defined KLAYGE_PLATFORM_WINDOWS_DESKTOP
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
#endif

	private:
#if defined KLAYGE_PLATFORM_WINDOWS_DESKTOP
		Signal::Connection on_raw_input_;
#elif defined(KLAYGE_PLATFORM_WINDOWS_STORE) || defined(KLAYGE_PLATFORM_ANDROID) || defined(KLAYGE_PLATFORM_DARWIN)
		Signal::Connection on_key_down_;
		Signal::Connection on_key_up_;
#if defined KLAYGE_PLATFORM_ANDROID
		Signal::Connection on_mouse_down_;
		Signal::Connection on_mouse_up_;
		Signal::Connection on_mouse_move_;
		Signal::Connection on_mouse_wheel_;
		Signal::Connection on_joystick_axis_;
		Signal::Connection on_joystick_buttons_;
#endif
#endif
		Signal::Connection on_pointer_down_;
		Signal::Connection on_pointer_up_;
		Signal::Connection on_pointer_update_;
		Signal::Connection on_pointer_wheel_;

#if defined KLAYGE_PLATFORM_WINDOWS_DESKTOP
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
#endif

	private:
		void DoSuspend() override;
		void DoResume() override;

#if defined KLAYGE_PLATFORM_WINDOWS_DESKTOP
		void OnRawInput(Window const & wnd, HRAWINPUT ri);
#elif defined(KLAYGE_PLATFORM_WINDOWS_STORE) || defined(KLAYGE_PLATFORM_ANDROID) || defined(KLAYGE_PLATFORM_DARWIN)
		void OnKeyDown(uint32_t key);
		void OnKeyUp(uint32_t key);
#if defined KLAYGE_PLATFORM_ANDROID
		void OnMouseDown(int2 const & pt, uint32_t buttons);
		void OnMouseUp(int2 const & pt, uint32_t buttons);
		void OnMouseMove(int2 const & pt);
		void OnMouseWheel(int2 const & pt, int32_t wheel_delta);
		void OnJoystickAxis(uint32_t axis, int32_t value);
		void OnJoystickButtons(uint32_t buttons);
#endif
#endif
		void OnPointerDown(int2 const & pt, uint32_t id);
		void OnPointerUp(int2 const & pt, uint32_t id);
		void OnPointerUpdate(int2 const & pt, uint32_t id, bool down);
		void OnPointerWheel(int2 const & pt, uint32_t id, int32_t wheel_delta);
	};

	class MsgInputKeyboard final : public InputKeyboard
	{
	public:
		MsgInputKeyboard();

		std::wstring const & Name() const override;
#if defined KLAYGE_PLATFORM_WINDOWS_DESKTOP
		void OnRawInput(RAWINPUT const & ri);
#elif defined(KLAYGE_PLATFORM_WINDOWS_STORE) || defined(KLAYGE_PLATFORM_ANDROID) || defined(KLAYGE_PLATFORM_DARWIN)
		void OnKeyDown(uint32_t key);
		void OnKeyUp(uint32_t key);
#endif

	private:
		void UpdateInputs() override;

	private:
		std::array<bool, 256> keys_state_;
	};

	class MsgInputMouse final : public InputMouse
	{
	public:
#if defined KLAYGE_PLATFORM_WINDOWS_DESKTOP
		MsgInputMouse(HWND hwnd, HANDLE device);
#elif defined KLAYGE_PLATFORM_ANDROID
		MsgInputMouse();
#endif

		std::wstring const & Name() const override;
#if defined KLAYGE_PLATFORM_WINDOWS_DESKTOP
		void OnRawInput(RAWINPUT const & ri);
#elif defined KLAYGE_PLATFORM_ANDROID
		void OnMouseDown(int2 const & pt, uint32_t buttons);
		void OnMouseUp(int2 const & pt, uint32_t buttons);
		void OnMouseMove(int2 const & pt);
		void OnMouseWheel(int2 const & pt, int32_t wheel_delta);
#endif

	private:
		void UpdateInputs() override;

	private:
#if defined KLAYGE_PLATFORM_WINDOWS_DESKTOP
		HWND hwnd_;
		uint32_t device_id_;
		int2 last_abs_state_;
#elif defined KLAYGE_PLATFORM_ANDROID
		int2 last_abs_state_;
		int2 abs_state_;
#endif
		int3 offset_state_;
		std::array<bool, 8> buttons_state_;
	};

	class MsgInputJoystick final : public InputJoystick
	{
	public:
#if defined KLAYGE_PLATFORM_WINDOWS_DESKTOP
		explicit MsgInputJoystick(HANDLE device);
#elif defined KLAYGE_PLATFORM_ANDROID
		MsgInputJoystick();
#endif

		std::wstring const & Name() const override;
#if defined KLAYGE_PLATFORM_WINDOWS_DESKTOP
		void OnRawInput(RAWINPUT const & ri);
#elif defined KLAYGE_PLATFORM_ANDROID
		void OnJoystickAxis(uint32_t axis, int32_t value);
		void OnJoystickButtons(uint32_t buttons);
#endif

	private:
		void UpdateInputs() override;

	private:
#if defined KLAYGE_PLATFORM_WINDOWS_DESKTOP
		uint32_t device_id_{0xFFFFFFFF};
#endif
		std::array<float3, 2> thumbs_state_{float3(0, 0, 0), float3(0, 0, 0)};
		std::array<float, 2> triggers_state_{};
		std::array<bool, 32> buttons_state_{};
	};

#if defined(KLAYGE_PLATFORM_WINDOWS)
	class MsgInputXInput final : public InputJoystick
	{
	public:
		explicit MsgInputXInput(uint32_t device_id);

		std::wstring const& Name() const override;

		void VibrationMotorSpeed(uint32_t n, float motor_speed) override;

	private:
		void UpdateInputs() override;

	private:
		uint32_t device_id_;
		std::array<uint16_t, 2> motor_speeds_{};
	};
#endif

#if defined KLAYGE_HAVE_LIBOVR
	class MsgInputOVR final : public InputSensor, public OVR::MessageHandler
	{
	public:
		MsgInputOVR();
		~MsgInputOVR() override;

		std::wstring const & Name() const override;

		void OnMessage(OVR::Message const & msg) override;

	private:
		void UpdateInputs() override;

	private:
		OVR::Ptr<OVR::DeviceManager> manager_;
		OVR::Ptr<OVR::SensorDevice> sensor_;
		OVR::Ptr<OVR::HMDDevice> hmd_;
		OVR::SensorFusion sfusion_;
		OVR::HMDInfo hmd_info_;

		OVR::Util::Render::StereoConfig sconfig_;
	};
#endif

	class MsgInputTouch final : public InputTouch
	{
	public:
		MsgInputTouch();

		std::wstring const & Name() const override;

		void OnPointerDown(int2 const & pt, uint32_t id);
		void OnPointerUp(int2 const & pt, uint32_t id);
		void OnPointerUpdate(int2 const & pt, uint32_t id, bool down);
		void OnPointerWheel(int2 const & pt, uint32_t id, int32_t wheel_delta);

	private:
		void UpdateInputs() override;
		int2 AdjustPoint(int2 const & pt) const;

		Timer timer_;
		std::array<int2, 16> touch_coord_state_;
		std::array<bool, 16> touch_down_state_;
		int32_t wheel_delta_state_;
	};
	
#if defined(KLAYGE_PLATFORM_WINDOWS_DESKTOP)
	class MsgInputSensor final : public InputSensor
	{
	public:
		MsgInputSensor();
		~MsgInputSensor() override;

		std::wstring const & Name() const override;

#if (_WIN32_WINNT < _WIN32_WINNT_WIN10)
		void OnLocationChanged(REFIID report_type, ILocationReport* location_report);
#endif
		void OnMotionDataUpdated(ISensor* sensor, ISensorDataReport* data_report);
		void OnOrientationDataUpdated(ISensor* sensor, ISensorDataReport* data_report);

		bool Destroyed() const
		{
			return destroyed_;
		}

	private:
		void UpdateInputs() override;

	private:
#if (_WIN32_WINNT < _WIN32_WINNT_WIN10)
		com_ptr<ILocation> locator_;
		com_ptr<ILocationEvents> location_event_;
#endif
		com_ptr<ISensorCollection> motion_sensor_collection_;
		std::vector<com_ptr<ISensorEvents>> motion_sensor_events_;
		com_ptr<ISensorCollection> orientation_sensor_collection_;
		std::vector<com_ptr<ISensorEvents>> orientation_sensor_events_;

		bool destroyed_;
	};
#elif defined KLAYGE_PLATFORM_WINDOWS_STORE
	class MsgInputSensor final : public InputSensor
	{
	public:
		MsgInputSensor();
		~MsgInputSensor() override;

		std::wstring const & Name() const override;

		HRESULT OnPositionChanged(uwp::Geolocator const& sender, uwp::PositionChangedEventArgs const& args);
		HRESULT OnAccelerometeReadingChanged(uwp::Accelerometer const& sender, uwp::AccelerometerReadingChangedEventArgs const& args);
		HRESULT OnGyrometerReadingChanged(uwp::Gyrometer const& sender, uwp::GyrometerReadingChangedEventArgs const& args);
		HRESULT OnInclinometerReadingChanged(uwp::Inclinometer const& sender, uwp::InclinometerReadingChangedEventArgs const& args);
		HRESULT OnCompassReadingChanged(uwp::Compass const& sender, uwp::CompassReadingChangedEventArgs const& args);
		HRESULT OnOrientationSensorReadingChanged(
			uwp::OrientationSensor const& sender, uwp::OrientationSensorReadingChangedEventArgs const& args);

	private:
		void UpdateInputs() override;

	private:
		uwp::Geolocator locator_{nullptr};
		uwp::Geolocator::PositionChanged_revoker position_token_;

		uwp::Accelerometer accelerometer_{nullptr};
		uwp::Accelerometer::ReadingChanged_revoker accelerometer_reading_token_;

		uwp::Gyrometer gyrometer_{nullptr};
		uwp::Gyrometer::ReadingChanged_revoker gyrometer_reading_token_;

		uwp::Inclinometer inclinometer_{nullptr};
		uwp::Inclinometer::ReadingChanged_revoker inclinometer_reading_token_;

		uwp::Compass compass_{nullptr};
		uwp::Compass::ReadingChanged_revoker compass_reading_token_;

		uwp::OrientationSensor orientation_{nullptr};
		uwp::OrientationSensor::ReadingChanged_revoker orientation_reading_token_;
	};
#elif defined KLAYGE_PLATFORM_ANDROID
	class MsgInputSensor final : public InputSensor
	{
	public:
		MsgInputSensor();
		~MsgInputSensor() override;

		std::wstring const & Name() const override;

	private:
		void UpdateInputs() override;

		static int SensorCallback(int fd, int events, void* data);

	private:
		ASensorManager* sensor_mgr_;
		ASensorEventQueue* sensor_event_queue_;
		ASensor const * accelerometer_;
		ASensor const * gyrometer_;
		ASensor const * magnetic_;
	};
#endif
}

#endif		// _MINPUT_HPP
