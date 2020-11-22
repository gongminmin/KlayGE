/**
 * @file MISensor.cpp
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
#include <KFL/ErrorHandling.hpp>
#include <KlayGE/Context.hpp>

#include <iterator>

#include <KlayGE/MsgInput/MInput.hpp>

#if defined KLAYGE_PLATFORM_ANDROID
#include <android_native_app_glue.h>
#endif

#if defined(KLAYGE_PLATFORM_WINDOWS_DESKTOP) || defined(KLAYGE_PLATFORM_WINDOWS_STORE) || defined(KLAYGE_PLATFORM_ANDROID)
namespace KlayGE
{
	std::wstring const & MsgInputSensor::Name() const
	{
		static std::wstring const name(L"MsgInput Sensor");
		return name;
	}

	void MsgInputSensor::UpdateInputs()
	{
	}
}
#endif

#if defined(KLAYGE_PLATFORM_WINDOWS_DESKTOP)

#if (_WIN32_WINNT < _WIN32_WINNT_WINBLUE)
	#if !(defined(KLAYGE_COMPILER_GCC) && (KLAYGE_COMPILER_VERSION >= 100))
		// Magnetometer Accuracy Data Types
		DEFINE_PROPERTYKEY(SENSOR_DATA_TYPE_MAGNETOMETER_ACCURACY,
			0X1637D8A2, 0X4248, 0X4275, 0X86, 0X5D, 0X55, 0X8D, 0XE8, 0X4A, 0XED, 0XFD, 22); //[VT_I4]
	#endif

	#if (_WIN32_WINNT == _WIN32_WINNT_WIN7) && !defined(KLAYGE_COMPILER_GCC)
		// Additional Motion Data Types
		DEFINE_PROPERTYKEY(SENSOR_DATA_TYPE_ANGULAR_VELOCITY_X_DEGREES_PER_SECOND,
			0X3F8A69A2, 0X7C5, 0X4E48, 0XA9, 0X65, 0XCD, 0X79, 0X7A, 0XAB, 0X56, 0XD5, 10); //[VT_R8]
		DEFINE_PROPERTYKEY(SENSOR_DATA_TYPE_ANGULAR_VELOCITY_Y_DEGREES_PER_SECOND,
			0X3F8A69A2, 0X7C5, 0X4E48, 0XA9, 0X65, 0XCD, 0X79, 0X7A, 0XAB, 0X56, 0XD5, 11); //[VT_R8]
		DEFINE_PROPERTYKEY(SENSOR_DATA_TYPE_ANGULAR_VELOCITY_Z_DEGREES_PER_SECOND,
			0X3F8A69A2, 0X7C5, 0X4E48, 0XA9, 0X65, 0XCD, 0X79, 0X7A, 0XAB, 0X56, 0XD5, 12); //[VT_R8]

		// Additional Compass Data Types
		DEFINE_PROPERTYKEY(SENSOR_DATA_TYPE_MAGNETIC_HEADING_COMPENSATED_MAGNETIC_NORTH_DEGREES,
			0X1637D8A2, 0X4248, 0X4275, 0X86, 0X5D, 0X55, 0X8D, 0XE8, 0X4A, 0XED, 0XFD, 11); //[VT_R8]

		// Additional Orientation Data Types
		DEFINE_PROPERTYKEY(SENSOR_DATA_TYPE_QUATERNION,
			0X1637D8A2, 0X4248, 0X4275, 0X86, 0X5D, 0X55, 0X8D, 0XE8, 0X4A, 0XED, 0XFD, 17); //[VT_VECTOR|VT_UI1]
	#endif
#endif

namespace KlayGE
{
#if (_WIN32_WINNT < _WIN32_WINNT_WIN10)
	class MsgInputLocationEvents : public ILocationEvents
	{
	public:
		explicit MsgInputLocationEvents(MsgInputSensor* input_sensor)
			: input_sensor_(input_sensor), ref_(1)
		{
		}
		virtual ~MsgInputLocationEvents()
		{
		}

		STDMETHODIMP QueryInterface(REFIID iid, void** ppv)
		{
			if (nullptr == ppv)
			{
				return E_POINTER;
			}

			if (IID_IUnknown == iid)
			{
				*ppv = static_cast<IUnknown*>(this);
			}
			else if (IID_ILocationEvents == iid)
			{
				*ppv = static_cast<ILocationEvents*>(this);
			}
			else
			{
				*ppv = nullptr;
				return E_NOINTERFACE;
			}

			this->AddRef();
			return S_OK;
		}

		STDMETHODIMP_(ULONG) AddRef()
		{
			return ::InterlockedIncrement(&ref_);
		}

		STDMETHODIMP_(ULONG) Release()
		{
			ULONG count = ::InterlockedDecrement(&ref_);
			if (0 == count)
			{
				delete this;
			}
			return count;
		}

		STDMETHODIMP OnLocationChanged(REFIID report_type, ILocationReport* location_report)
		{
			HRESULT hr = S_OK;

			if (nullptr == location_report)
			{
				return E_INVALIDARG;
			}

			input_sensor_->OnLocationChanged(report_type, location_report);

			return hr;
		}

		STDMETHODIMP OnStatusChanged(REFIID report_type, LOCATION_REPORT_STATUS new_status)
		{
			KFL_UNUSED(report_type);
			KFL_UNUSED(new_status);

			return S_OK;
		}

	protected:
		MsgInputSensor* input_sensor_;

	private:
		long ref_;
	};
#endif

	class MsgInputSensorEvents : public ISensorEvents
	{
	public:
		explicit MsgInputSensorEvents(MsgInputSensor* input_sensor)
			: input_sensor_(input_sensor), ref_(1)
		{
		}
		virtual ~MsgInputSensorEvents()
		{
		}

		STDMETHODIMP QueryInterface(REFIID iid, void** ppv)
		{
			if (nullptr == ppv)
			{
				return E_POINTER;
			}

			if (IID_IUnknown == iid)
			{
				*ppv = static_cast<IUnknown*>(this);
			}
			else if (IID_ISensorEvents == iid)
			{
				*ppv = static_cast<ISensorEvents*>(this);
			}
			else
			{
				*ppv = nullptr;
				return E_NOINTERFACE;
			}

			this->AddRef();
			return S_OK;
		}

		STDMETHODIMP_(ULONG) AddRef()
		{
			return ::InterlockedIncrement(&ref_);
		}

		STDMETHODIMP_(ULONG) Release()
		{
			ULONG count = ::InterlockedDecrement(&ref_);
			if (0 == count)
			{
				delete this;
			}
			return count;
		}

		STDMETHODIMP OnEvent(ISensor* sensor, REFGUID event_id, IPortableDeviceValues* event_data)
		{
			KFL_UNUSED(sensor);
			KFL_UNUSED(event_id);
			KFL_UNUSED(event_data);

			return S_OK;
		}

		STDMETHODIMP OnLeave(REFSENSOR_ID sensor_id)
		{
			KFL_UNUSED(sensor_id);

			return S_OK;
		}

		STDMETHODIMP OnStateChanged(ISensor* sensor, SensorState state)
		{
			KFL_UNUSED(state);

			HRESULT hr = S_OK;

			if (nullptr == sensor)
			{
				return E_INVALIDARG;
			}

			return hr;
		}

	protected:
		MsgInputSensor* input_sensor_;

	private:
		long ref_;
	};

	class MsgInputMotionSensorEvents : public MsgInputSensorEvents
	{
	public:
		explicit MsgInputMotionSensorEvents(MsgInputSensor* input_sensor)
			: MsgInputSensorEvents(input_sensor)
		{
		}

		STDMETHODIMP OnDataUpdated(ISensor* sensor, ISensorDataReport* data_report)
		{
			HRESULT hr = S_OK;

			if (!input_sensor_->Destroyed())
			{
				if ((nullptr == data_report) || (nullptr == sensor))
				{
					return E_INVALIDARG;
				}

				input_sensor_->OnMotionDataUpdated(sensor, data_report);
			}

			return hr;
		}
	};

	class MsgInputOrientationSensorEvents : public MsgInputSensorEvents
	{
	public:
		explicit MsgInputOrientationSensorEvents(MsgInputSensor* input_sensor)
			: MsgInputSensorEvents(input_sensor)
		{
		}

		STDMETHODIMP OnDataUpdated(ISensor* sensor, ISensorDataReport* data_report)
		{
			HRESULT hr = S_OK;

			if (!input_sensor_->Destroyed())
			{
				if ((nullptr == data_report) || (nullptr == sensor))
				{
					return E_INVALIDARG;
				}

				input_sensor_->OnOrientationDataUpdated(sensor, data_report);
			}

			return hr;
		}
	};


	MsgInputSensor::MsgInputSensor()
		: destroyed_(false)
	{
		HRESULT hr = ::CoInitializeEx(0, COINIT_APARTMENTTHREADED);

#if (_WIN32_WINNT < _WIN32_WINNT_WIN10)
		if (Context::Instance().Config().location_sensor)
		{
			com_ptr<ILocation> location;
			hr = ::CoCreateInstance(CLSID_Location, nullptr, CLSCTX_INPROC_SERVER,
				IID_ILocation, location.put_void());
			if (SUCCEEDED(hr))
			{
				IID REPORT_TYPES[] = { IID_ILatLongReport };
				hr = location->RequestPermissions(nullptr, REPORT_TYPES, static_cast<ULONG>(std::size(REPORT_TYPES)), true);
				if (SUCCEEDED(hr))
				{
					locator_ = location;

					location_event_.reset(new MsgInputLocationEvents(this), false);
					auto sensor_event = location_event_.as<ILocationEvents>(IID_ILocationEvents);
					for (DWORD index = 0; index < std::size(REPORT_TYPES); ++ index)
					{
						hr = locator_->RegisterForReport(sensor_event.get(), REPORT_TYPES[index], 1000);
					}
				}
			}
		}
#endif

		com_ptr<ISensorManager> sensor_mgr;
		hr = ::CoCreateInstance(CLSID_SensorManager, nullptr, CLSCTX_INPROC_SERVER,
			IID_ISensorManager, sensor_mgr.put_void());
		if (SUCCEEDED(hr))
		{
			hr = sensor_mgr->GetSensorsByCategory(SENSOR_CATEGORY_MOTION, motion_sensor_collection_.put());
			if (SUCCEEDED(hr))
			{
				ULONG count = 0;
				hr = motion_sensor_collection_->GetCount(&count);
				if (SUCCEEDED(hr))
				{
					for (ULONG i = 0; i < count; ++ i)
					{
						ISensor* sensor;
						hr = motion_sensor_collection_->GetAt(i, &sensor);
						if (SUCCEEDED(hr))
						{
							com_ptr<ISensorEvents> motion_event(new MsgInputMotionSensorEvents(this), false);
							motion_sensor_events_.push_back(motion_event);
							sensor->SetEventSink(motion_event.get());
						}
					}
				}
			}

			hr = sensor_mgr->GetSensorsByCategory(SENSOR_CATEGORY_ORIENTATION, orientation_sensor_collection_.put());
			if (SUCCEEDED(hr))
			{
				ULONG count = 0;
				hr = orientation_sensor_collection_->GetCount(&count);
				if (SUCCEEDED(hr))
				{
					for (ULONG i = 0; i < count; ++ i)
					{
						com_ptr<ISensor> sensor;
						hr = orientation_sensor_collection_->GetAt(i, sensor.put());
						if (SUCCEEDED(hr))
						{
							com_ptr<ISensorEvents> orientation_event(new MsgInputOrientationSensorEvents(this), false);
							orientation_sensor_events_.push_back(orientation_event);

							sensor->SetEventSink(orientation_event.get());
						}
					}
				}
			}
		}
	}

	MsgInputSensor::~MsgInputSensor()
	{
		destroyed_ = true;

#if (_WIN32_WINNT < _WIN32_WINNT_WIN10)
		if (locator_)
		{
			IID REPORT_TYPES[] = { IID_ILatLongReport };
			for (DWORD index = 0; index < std::size(REPORT_TYPES); ++ index)
			{
				locator_->UnregisterForReport(REPORT_TYPES[index]);
			}

			location_event_.reset();
			locator_.reset();
		}
#endif

		if (motion_sensor_collection_)
		{
			motion_sensor_events_.clear();
			motion_sensor_collection_.reset();
		}

		if (orientation_sensor_collection_)
		{
			orientation_sensor_events_.clear();
			orientation_sensor_collection_.reset();
		}

		::CoUninitialize();
	}

#if (_WIN32_WINNT < _WIN32_WINNT_WIN10)
	void MsgInputSensor::OnLocationChanged(REFIID report_type, ILocationReport* location_report)
	{
		if (IID_ILatLongReport == report_type)
		{
			if (auto lat_long_report = com_ptr<ILocationReport>(location_report).try_as<ILatLongReport>(IID_ILatLongReport))
			{
				double lat = 0;
				double lng = 0;
				double alt = 0;
				double err_radius = 0;
				double alt_err = 0;

				if (SUCCEEDED(lat_long_report->GetLatitude(&lat)))
				{
					latitude_ = static_cast<float>(lat);
				}
				if (SUCCEEDED(lat_long_report->GetLongitude(&lng)))
				{
					longitude_ = static_cast<float>(lng);
				}
				if (SUCCEEDED(lat_long_report->GetAltitude(&alt)))
				{
					altitude_ = static_cast<float>(alt);
				}
				if (SUCCEEDED(lat_long_report->GetErrorRadius(&err_radius)))
				{
					location_error_radius_ = static_cast<float>(err_radius);
				}
				if (SUCCEEDED(lat_long_report->GetAltitudeError(&alt_err)))
				{
					location_altitude_error_ = static_cast<float>(alt_err);
				}
			}
		}
	}
#endif

	void MsgInputSensor::OnMotionDataUpdated(ISensor* sensor, ISensorDataReport* data_report)
	{
		VARIANT_BOOL supported;

		supported = VARIANT_FALSE;
		if (SUCCEEDED(sensor->SupportsDataField(SENSOR_DATA_TYPE_ACCELERATION_X_G, &supported))
			&& (supported != VARIANT_FALSE))
		{
			PROPVARIANT prop;
			::PropVariantInit(&prop);
			if (SUCCEEDED(data_report->GetSensorValue(SENSOR_DATA_TYPE_ACCELERATION_X_G, &prop)))
			{
				if (VT_R8 == prop.vt)
				{
					accel_.x() = static_cast<float>(prop.dblVal);
				}

				::PropVariantClear(&prop);
			}
		}

		supported = VARIANT_FALSE;
		if (SUCCEEDED(sensor->SupportsDataField(SENSOR_DATA_TYPE_ACCELERATION_Y_G, &supported))
			&& (supported != VARIANT_FALSE))
		{
			PROPVARIANT prop;
			::PropVariantInit(&prop);
			if (SUCCEEDED(data_report->GetSensorValue(SENSOR_DATA_TYPE_ACCELERATION_Y_G, &prop)))
			{
				if (VT_R8 == prop.vt)
				{
					accel_.y() = static_cast<float>(prop.dblVal);
				}

				::PropVariantClear(&prop);
			}
		}

		supported = VARIANT_FALSE;
		if (SUCCEEDED(sensor->SupportsDataField(SENSOR_DATA_TYPE_ACCELERATION_Z_G, &supported))
			&& (supported != VARIANT_FALSE))
		{
			PROPVARIANT prop;
			::PropVariantInit(&prop);
			if (SUCCEEDED(data_report->GetSensorValue(SENSOR_DATA_TYPE_ACCELERATION_Z_G, &prop)))
			{
				if (VT_R8 == prop.vt)
				{
					accel_.z() = static_cast<float>(prop.dblVal);
				}

				::PropVariantClear(&prop);
			}
		}

		supported = VARIANT_FALSE;
		if (SUCCEEDED(sensor->SupportsDataField(SENSOR_DATA_TYPE_ANGULAR_VELOCITY_X_DEGREES_PER_SECOND, &supported))
			&& (supported != VARIANT_FALSE))
		{
			PROPVARIANT prop;
			::PropVariantInit(&prop);
			if (SUCCEEDED(data_report->GetSensorValue(SENSOR_DATA_TYPE_ANGULAR_VELOCITY_X_DEGREES_PER_SECOND, &prop)))
			{
				if (VT_R8 == prop.vt)
				{
					angular_velocity_.x() = static_cast<float>(prop.dblVal);
				}

				::PropVariantClear(&prop);
			}
		}

		supported = VARIANT_FALSE;
		if (SUCCEEDED(sensor->SupportsDataField(SENSOR_DATA_TYPE_SPEED_METERS_PER_SECOND, &supported))
			&& (supported != VARIANT_FALSE))
		{
			PROPVARIANT prop;
			::PropVariantInit(&prop);
			if (SUCCEEDED(data_report->GetSensorValue(SENSOR_DATA_TYPE_SPEED_METERS_PER_SECOND, &prop)))
			{
				if (VT_R8 == prop.vt)
				{
					speed_ = static_cast<float>(prop.dblVal);
				}

				::PropVariantClear(&prop);
			}
		}

		supported = VARIANT_FALSE;
		if (SUCCEEDED(sensor->SupportsDataField(SENSOR_DATA_TYPE_ANGULAR_VELOCITY_Y_DEGREES_PER_SECOND, &supported))
			&& (supported != VARIANT_FALSE))
		{
			PROPVARIANT prop;
			::PropVariantInit(&prop);
			if (SUCCEEDED(data_report->GetSensorValue(SENSOR_DATA_TYPE_ANGULAR_VELOCITY_Y_DEGREES_PER_SECOND, &prop)))
			{
				if (VT_R8 == prop.vt)
				{
					angular_velocity_.y() = static_cast<float>(prop.dblVal);
				}

				::PropVariantClear(&prop);
			}
		}

		supported = VARIANT_FALSE;
		if (SUCCEEDED(sensor->SupportsDataField(SENSOR_DATA_TYPE_ANGULAR_VELOCITY_Z_DEGREES_PER_SECOND, &supported))
			&& (supported != VARIANT_FALSE))
		{
			PROPVARIANT prop;
			::PropVariantInit(&prop);
			if (SUCCEEDED(data_report->GetSensorValue(SENSOR_DATA_TYPE_ANGULAR_VELOCITY_Z_DEGREES_PER_SECOND, &prop)))
			{
				if (VT_R8 == prop.vt)
				{
					angular_velocity_.z() = static_cast<float>(prop.dblVal);
				}

				::PropVariantClear(&prop);
			}
		}
	}

	void MsgInputSensor::OnOrientationDataUpdated(ISensor* sensor, ISensorDataReport* data_report)
	{
		VARIANT_BOOL supported;

		supported = VARIANT_FALSE;
		if (SUCCEEDED(sensor->SupportsDataField(SENSOR_DATA_TYPE_TILT_X_DEGREES, &supported))
			&& (supported != VARIANT_FALSE))
		{
			PROPVARIANT prop;
			::PropVariantInit(&prop);
			if (SUCCEEDED(data_report->GetSensorValue(SENSOR_DATA_TYPE_TILT_X_DEGREES, &prop)))
			{
				if (VT_R4 == prop.vt)
				{
					tilt_.x() = prop.fltVal;
				}

				::PropVariantClear(&prop);
			}
		}

		supported = VARIANT_FALSE;
		if (SUCCEEDED(sensor->SupportsDataField(SENSOR_DATA_TYPE_TILT_Y_DEGREES, &supported))
			&& (supported != VARIANT_FALSE))
		{
			PROPVARIANT prop;
			::PropVariantInit(&prop);
			if (SUCCEEDED(data_report->GetSensorValue(SENSOR_DATA_TYPE_TILT_Y_DEGREES, &prop)))
			{
				if (VT_R4 == prop.vt)
				{
					tilt_.y() = prop.fltVal;
				}

				::PropVariantClear(&prop);
			}
		}

		supported = VARIANT_FALSE;
		if (SUCCEEDED(sensor->SupportsDataField(SENSOR_DATA_TYPE_TILT_Z_DEGREES, &supported))
			&& (supported != VARIANT_FALSE))
		{
			PROPVARIANT prop;
			::PropVariantInit(&prop);
			if (SUCCEEDED(data_report->GetSensorValue(SENSOR_DATA_TYPE_TILT_Z_DEGREES, &prop)))
			{
				if (VT_R4 == prop.vt)
				{
					tilt_.z() = prop.fltVal;
				}

				::PropVariantClear(&prop);
			}
		}

		supported = VARIANT_FALSE;
		if (SUCCEEDED(sensor->SupportsDataField(SENSOR_DATA_TYPE_MAGNETIC_HEADING_COMPENSATED_MAGNETIC_NORTH_DEGREES, &supported))
			&& (supported != VARIANT_FALSE))
		{
			PROPVARIANT prop;
			::PropVariantInit(&prop);
			if (SUCCEEDED(data_report->GetSensorValue(SENSOR_DATA_TYPE_MAGNETIC_HEADING_COMPENSATED_MAGNETIC_NORTH_DEGREES, &prop)))
			{
				if (VT_R8 == prop.vt)
				{
					magnetic_heading_north_ = static_cast<float>(prop.dblVal);
				}

				::PropVariantClear(&prop);
			}
		}

		supported = VARIANT_FALSE;
		if (SUCCEEDED(sensor->SupportsDataField(SENSOR_DATA_TYPE_QUATERNION, &supported))
			&& (supported != VARIANT_FALSE))
		{
			PROPVARIANT prop;
			::PropVariantInit(&prop);
			if (SUCCEEDED(data_report->GetSensorValue(SENSOR_DATA_TYPE_QUATERNION, &prop)))
			{
				if ((VT_VECTOR | VT_UI1) == prop.vt)
				{
					BOOST_ASSERT(prop.caub.cElems / sizeof(float) >= 4);
					orientation_quat_ = Quaternion(reinterpret_cast<float const *>(prop.caub.pElems));
				}

				::PropVariantClear(&prop);
			}
		}

		supported = VARIANT_FALSE;
		if (SUCCEEDED(sensor->SupportsDataField(SENSOR_DATA_TYPE_MAGNETOMETER_ACCURACY, &supported))
			&& (supported != VARIANT_FALSE))
		{
			PROPVARIANT prop;
			::PropVariantInit(&prop);
			if (SUCCEEDED(data_report->GetSensorValue(SENSOR_DATA_TYPE_MAGNETOMETER_ACCURACY, &prop)))
			{
				if (VT_I4 == prop.vt)
				{
					magnetometer_accuracy_ = prop.intVal;
				}

				::PropVariantClear(&prop);
			}
		}
	}
}

#elif defined KLAYGE_PLATFORM_WINDOWS_STORE

namespace KlayGE
{
	MsgInputSensor::MsgInputSensor()
	{
		if (Context::Instance().Config().location_sensor)
		{
			locator_ = uwp::Geolocator();
			position_token_ = locator_.PositionChanged(
				winrt::auto_revoke, [this](uwp::Geolocator const& sender, uwp::PositionChangedEventArgs const& args) {
					return this->OnPositionChanged(sender, args);
				});
		}
		{
			accelerometer_ = uwp::Accelerometer::GetDefault();
			if (accelerometer_)
			{
				accelerometer_reading_token_ = accelerometer_.ReadingChanged(
					winrt::auto_revoke, [this](uwp::Accelerometer const& sender, uwp::AccelerometerReadingChangedEventArgs const& args) {
						return this->OnAccelerometeReadingChanged(sender, args);
					});
			}
		}
		{
			gyrometer_ = uwp::Gyrometer::GetDefault();
			if (gyrometer_)
			{
				gyrometer_reading_token_ = gyrometer_.ReadingChanged(
					winrt::auto_revoke, [this](uwp::Gyrometer const& sender, uwp::GyrometerReadingChangedEventArgs const& args) {
						return this->OnGyrometerReadingChanged(sender, args);
					});
			}
		}
		{
			inclinometer_ = uwp::Inclinometer::GetDefault();
			if (inclinometer_)
			{
				inclinometer_reading_token_ = inclinometer_.ReadingChanged(
					winrt::auto_revoke, [this](uwp::Inclinometer const& sender, uwp::InclinometerReadingChangedEventArgs const& args) {
						return this->OnInclinometerReadingChanged(sender, args);
					});
			}
		}
		{
			compass_ = uwp::Compass::GetDefault();
			if (compass_)
			{
				compass_reading_token_ = compass_.ReadingChanged(
					winrt::auto_revoke, [this](uwp::Compass const& sender, uwp::CompassReadingChangedEventArgs const& args) {
						return this->OnCompassReadingChanged(sender, args);
					});
			}
		}
		{
			orientation_ = uwp::OrientationSensor::GetDefault();
			if (orientation_)
			{
				orientation_reading_token_ = orientation_.ReadingChanged(winrt::auto_revoke,
					[this](uwp::OrientationSensor const& sender, uwp::OrientationSensorReadingChangedEventArgs const& args) {
						return this->OnOrientationSensorReadingChanged(sender, args);
					});
			}
		}
	}

	MsgInputSensor::~MsgInputSensor()
	{
		if (Context::Instance().Config().location_sensor)
		{
			position_token_.revoke();
		}
		if (accelerometer_)
		{
			accelerometer_reading_token_.revoke();
		}
		if (gyrometer_)
		{
			gyrometer_reading_token_.revoke();
		}
		if (inclinometer_)
		{
			inclinometer_reading_token_.revoke();
		}
		if (compass_)
		{
			compass_reading_token_.revoke();
		}
		if (orientation_)
		{
			orientation_reading_token_.revoke();
		}
	}

	HRESULT MsgInputSensor::OnPositionChanged(uwp::Geolocator const& sender, uwp::PositionChangedEventArgs const& args)
	{
		KFL_UNUSED(sender);

		auto const position = args.Position();
		auto const coordinate = position.Coordinate();
		
		auto const coordinate_with_point = coordinate.as<uwp::IGeocoordinateWithPoint>();
		auto const point = coordinate_with_point.Point();
		auto const geo_position = point.Position();
		latitude_ = static_cast<float>(geo_position.Latitude);
		longitude_ = static_cast<float>(geo_position.Longitude);
		altitude_ = static_cast<float>(geo_position.Altitude);

		location_error_radius_ = static_cast<float>(coordinate.Accuracy());

		auto rtmp = coordinate.AltitudeAccuracy();
		if (rtmp)
		{
			location_altitude_error_ = static_cast<float>(rtmp.Value());
		}

		rtmp = coordinate.Speed();
		if (rtmp)
		{
			speed_ = static_cast<float>(rtmp.Value());
		}

		return S_OK;
	}

	HRESULT MsgInputSensor::OnAccelerometeReadingChanged(
		uwp::Accelerometer const& sender, uwp::AccelerometerReadingChangedEventArgs const& args)
	{
		KFL_UNUSED(sender);

		auto const reading = args.Reading();

		accel_.x() = static_cast<float>(reading.AccelerationX());
		accel_.y() = static_cast<float>(reading.AccelerationY());
		accel_.z() = static_cast<float>(reading.AccelerationZ());

		return S_OK;
	}

	HRESULT MsgInputSensor::OnGyrometerReadingChanged(uwp::Gyrometer const& sender, uwp::GyrometerReadingChangedEventArgs const& args)
	{
		KFL_UNUSED(sender);

		auto const reading = args.Reading();

		angular_velocity_.x() = static_cast<float>(reading.AngularVelocityX());
		angular_velocity_.y() = static_cast<float>(reading.AngularVelocityY());
		angular_velocity_.z() = static_cast<float>(reading.AngularVelocityZ());

		return S_OK;
	}

	HRESULT MsgInputSensor::OnInclinometerReadingChanged(uwp::Inclinometer const& sender, uwp::InclinometerReadingChangedEventArgs const& args)
	{
		KFL_UNUSED(sender);

		auto const reading = args.Reading();

		tilt_.x() = reading.PitchDegrees();
		tilt_.y() = reading.RollDegrees();
		tilt_.z() = reading.YawDegrees();

		return S_OK;
	}

	HRESULT MsgInputSensor::OnCompassReadingChanged(uwp::Compass const& sender, uwp::CompassReadingChangedEventArgs const& args)
	{
		KFL_UNUSED(sender);

		auto const reading = args.Reading();

		magnetic_heading_north_ = static_cast<float>(reading.HeadingMagneticNorth());

		auto reading_with_accuracy = reading.as<uwp::ICompassReadingHeadingAccuracy>();
		if (reading_with_accuracy)
		{
			auto const accuracy = reading_with_accuracy.HeadingAccuracy();
			magnetometer_accuracy_ = static_cast<int32_t>(accuracy);
		}
		else
		{
			magnetometer_accuracy_ = 0;
		}

		return S_OK;
	}

	HRESULT MsgInputSensor::OnOrientationSensorReadingChanged(
		uwp::OrientationSensor const& sender, uwp::OrientationSensorReadingChangedEventArgs const& args)
	{
		KFL_UNUSED(sender);

		auto const reading = args.Reading();

		auto const quat = reading.Quaternion();
		orientation_quat_.x() = quat.X();
		orientation_quat_.y() = quat.Y();
		orientation_quat_.z() = quat.Z();
		orientation_quat_.w() = quat.W();

		return S_OK;
	}
}

#elif defined KLAYGE_PLATFORM_ANDROID

namespace KlayGE
{
	MsgInputSensor::MsgInputSensor()
	{
#if __ANDROID_API__ >= 26
		sensor_mgr_ = ASensorManager_getInstanceForPackage(nullptr);
#else
		sensor_mgr_ = ASensorManager_getInstance();
#endif
		if (sensor_mgr_)
		{
			sensor_event_queue_ = ASensorManager_createEventQueue(sensor_mgr_, ALooper_forThread(),
				LOOPER_ID_USER, &SensorCallback, this);

			accelerometer_ = ASensorManager_getDefaultSensor(sensor_mgr_, ASENSOR_TYPE_ACCELEROMETER);
			if (accelerometer_)
			{
				if (ASensorEventQueue_enableSensor(sensor_event_queue_, accelerometer_) >= 0)
				{
					ASensorEventQueue_setEventRate(sensor_event_queue_, accelerometer_, 1.0f / 60);
				}
			}

			gyrometer_ = ASensorManager_getDefaultSensor(sensor_mgr_, ASENSOR_TYPE_GYROSCOPE);
			if (gyrometer_)
			{
				if (ASensorEventQueue_enableSensor(sensor_event_queue_, gyrometer_) >= 0)
				{
					ASensorEventQueue_setEventRate(sensor_event_queue_, gyrometer_, 1.0f / 60);
				}
			}

			magnetic_ = ASensorManager_getDefaultSensor(sensor_mgr_, ASENSOR_TYPE_MAGNETIC_FIELD);
			if (magnetic_)
			{
				if (ASensorEventQueue_enableSensor(sensor_event_queue_, magnetic_) >= 0)
				{
					ASensorEventQueue_setEventRate(sensor_event_queue_, magnetic_, 1.0f / 60);
				}
			}
		}
	}

	MsgInputSensor::~MsgInputSensor()
	{
		ASensorManager_destroyEventQueue(sensor_mgr_, sensor_event_queue_);
	}

	int MsgInputSensor::SensorCallback(int fd, int events, void* data)
	{
		KFL_UNUSED(fd);
		KFL_UNUSED(events);

		MsgInputSensor* input_sensor = static_cast<MsgInputSensor*>(data);

		float3 accel;
		bool has_accel = false;
		float3 magnetic;
		bool has_magnetic = false;
		ASensorEvent sensor_event;
		while (ASensorEventQueue_getEvents(input_sensor->sensor_event_queue_, &sensor_event, 1) > 0)
		{
			switch (sensor_event.type)
			{
			case ASENSOR_TYPE_ACCELEROMETER:
				accel = float3(sensor_event.acceleration.x, sensor_event.acceleration.y,
					sensor_event.acceleration.z);
				has_accel = true;
				input_sensor->accel_ = accel;
				break;

			case ASENSOR_TYPE_GYROSCOPE:
				input_sensor->angular_velocity_ = float3(sensor_event.vector.x, sensor_event.vector.y,
					sensor_event.vector.z);
				break;

			case ASENSOR_TYPE_MAGNETIC_FIELD:
				magnetic = float3(sensor_event.magnetic.x, sensor_event.magnetic.y,
					sensor_event.magnetic.z);
				has_magnetic = true;
				break;

			default:
				break;
			}
		}

		if (has_accel && has_magnetic)
		{
			float3 h = MathLib::cross(magnetic, accel);
			float const len_h = MathLib::length(h);
			if (len_h >= 0.1f)
			{
				h /= len_h;
				accel = MathLib::normalize(accel);
				float3 const m = MathLib::cross(accel, h);
				float4x4 rotate(h.x(), h.y(), h.z(), 0,
					m.x(), m.y(), m.z(), 0,
					accel.x(), accel.y(), accel.z(), 0,
					0, 0, 0, 1);
				input_sensor->orientation_quat_ = MathLib::to_quaternion(rotate);
				MathLib::to_yaw_pitch_roll(input_sensor->tilt_.x(), input_sensor->tilt_.y(), input_sensor->tilt_.z(),
					input_sensor->orientation_quat_);
				input_sensor->magnetic_heading_north_ = atan2(MathLib::dot(magnetic, accel),
					MathLib::dot(magnetic, m));
			}
		}

		return 1;
	}
}

#endif
