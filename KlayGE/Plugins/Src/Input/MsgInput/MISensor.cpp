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
#include <KFL/COMPtr.hpp>
#include <KlayGE/Context.hpp>

#include <KlayGE/MsgInput/MInput.hpp>

#if ((defined KLAYGE_PLATFORM_WINDOWS_DESKTOP) && (_WIN32_WINNT >= _WIN32_WINNT_WIN7) && (_WIN32_WINNT < _WIN32_WINNT_WIN10)) \
	|| (defined KLAYGE_PLATFORM_WINDOWS_RUNTIME) \
	|| (defined KLAYGE_PLATFORM_ANDROID)
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

#if (defined KLAYGE_PLATFORM_WINDOWS_DESKTOP) && (_WIN32_WINNT >= _WIN32_WINNT_WIN7) && (_WIN32_WINNT < _WIN32_WINNT_WIN10)

#if (_WIN32_WINNT < _WIN32_WINNT_WINBLUE)
	// Magnetometer Accuracy Data Types
	DEFINE_PROPERTYKEY(SENSOR_DATA_TYPE_MAGNETOMETER_ACCURACY,
		0X1637D8A2, 0X4248, 0X4275, 0X86, 0X5D, 0X55, 0X8D, 0XE8, 0X4A, 0XED, 0XFD, 22); //[VT_I4]

	#if (_WIN32_WINNT < _WIN32_WINNT_WIN8)
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
			UNREF_PARAM(report_type);
			UNREF_PARAM(new_status);

			return S_OK;
		}

	protected:
		MsgInputSensor* input_sensor_;

	private:
		long ref_;
	};

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
			UNREF_PARAM(sensor);
			UNREF_PARAM(event_id);
			UNREF_PARAM(event_data);

			return S_OK;
		}

		STDMETHODIMP OnLeave(REFSENSOR_ID sensor_id)
		{
			UNREF_PARAM(sensor_id);

			return S_OK;
		}

		STDMETHODIMP OnStateChanged(ISensor* sensor, SensorState state)
		{
			UNREF_PARAM(state);

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

			if ((nullptr == data_report) || (nullptr == sensor))
			{
				return E_INVALIDARG;
			}

			input_sensor_->OnMotionDataUpdated(sensor, data_report);

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

			if ((nullptr == data_report) || (nullptr == sensor))
			{
				return E_INVALIDARG;
			}

			input_sensor_->OnOrientationDataUpdated(sensor, data_report);

			return hr;
		}
	};


	MsgInputSensor::MsgInputSensor()
	{
		HRESULT hr = ::CoInitialize(0);

		if (Context::Instance().Config().location_sensor)
		{
			ILocation* location = nullptr;
			hr = ::CoCreateInstance(CLSID_Location, nullptr, CLSCTX_INPROC_SERVER,
				IID_ILocation, reinterpret_cast<void**>(&location));
			if (SUCCEEDED(hr))
			{
				IID REPORT_TYPES[] = { IID_ILatLongReport };
				hr = location->RequestPermissions(nullptr, REPORT_TYPES,
					sizeof(REPORT_TYPES) / sizeof(REPORT_TYPES[0]), true);
				if (SUCCEEDED(hr))
				{
					locator_ = MakeCOMPtr(location);

					location_event_ = MakeSharedPtr<MsgInputLocationEvents>(this);
					ILocationEvents* sensor_event;
					location_event_->QueryInterface(IID_ILocationEvents, reinterpret_cast<void**>(&sensor_event));

					for (DWORD index = 0; index < sizeof(REPORT_TYPES) / sizeof(REPORT_TYPES[0]); ++ index)
					{
						hr = locator_->RegisterForReport(sensor_event, REPORT_TYPES[index], 1000);
					}

					sensor_event->Release();
				}
				else
				{
					location->Release();
				}
			}
		}

		ISensorManager* sensor_mgr = nullptr;
		hr = ::CoCreateInstance(CLSID_SensorManager, nullptr, CLSCTX_INPROC_SERVER,
			IID_ISensorManager, reinterpret_cast<void**>(&sensor_mgr));
		if (SUCCEEDED(hr))
		{
			ISensorCollection* motion_sensor_collection = nullptr;
			hr = sensor_mgr->GetSensorsByCategory(SENSOR_CATEGORY_MOTION, &motion_sensor_collection);
			if (SUCCEEDED(hr))
			{
				motion_sensor_collection_ = MakeCOMPtr(motion_sensor_collection);

				ULONG count = 0;
				hr = motion_sensor_collection_->GetCount(&count);
				if (SUCCEEDED(hr))
				{
					for (ULONG i = 0; i < count; ++i)
					{
						ISensor* sensor;
						hr = motion_sensor_collection_->GetAt(i, &sensor);
						if (SUCCEEDED(hr))
						{
							std::shared_ptr<MsgInputMotionSensorEvents> motion_event = MakeSharedPtr<MsgInputMotionSensorEvents>(this);
							ISensorEvents* sensor_event;
							motion_event->QueryInterface(IID_ISensorEvents, reinterpret_cast<void**>(&sensor_event));
							motion_sensor_events_.push_back(motion_event);

							sensor->SetEventSink(sensor_event);

							sensor_event->Release();
							sensor->Release();
						}
					}
				}
			}

			ISensorCollection* orientation_sensor_collection = nullptr;
			hr = sensor_mgr->GetSensorsByCategory(SENSOR_CATEGORY_ORIENTATION, &orientation_sensor_collection);
			if (SUCCEEDED(hr))
			{
				orientation_sensor_collection_ = MakeCOMPtr(orientation_sensor_collection);

				ULONG count = 0;
				hr = orientation_sensor_collection_->GetCount(&count);
				if (SUCCEEDED(hr))
				{
					for (ULONG i = 0; i < count; ++i)
					{
						ISensor* sensor;
						hr = orientation_sensor_collection_->GetAt(i, &sensor);
						if (SUCCEEDED(hr))
						{
							std::shared_ptr<MsgInputOrientationSensorEvents> orientation_event = MakeSharedPtr<MsgInputOrientationSensorEvents>(this);
							ISensorEvents* sensor_event;
							orientation_event->QueryInterface(IID_ISensorEvents, reinterpret_cast<void**>(&sensor_event));
							orientation_sensor_events_.push_back(orientation_event);

							sensor->SetEventSink(sensor_event);

							sensor_event->Release();
							sensor->Release();
						}
					}
				}
			}

			sensor_mgr->Release();
		}
	}

	MsgInputSensor::~MsgInputSensor()
	{
		if (locator_)
		{
			IID REPORT_TYPES[] = { IID_ILatLongReport };
			for (DWORD index = 0; index < sizeof(REPORT_TYPES) / sizeof(REPORT_TYPES[0]); ++ index)
			{
				locator_->UnregisterForReport(REPORT_TYPES[index]);
			}

			location_event_.reset();
			locator_.reset();
		}

		if (motion_sensor_collection_)
		{
			ULONG count = 0;
			HRESULT hr = motion_sensor_collection_->GetCount(&count);
			if (SUCCEEDED(hr))
			{
				for (ULONG i = 0; i < count; ++i)
				{
					ISensor* sensor;
					hr = motion_sensor_collection_->GetAt(i, &sensor);
					if (SUCCEEDED(hr))
					{
						sensor->SetEventSink(nullptr);
						sensor->Release();
					}
				}
			}

			motion_sensor_events_.clear();
			motion_sensor_collection_.reset();
		}

		if (orientation_sensor_collection_)
		{
			ULONG count = 0;
			HRESULT hr = orientation_sensor_collection_->GetCount(&count);
			if (SUCCEEDED(hr))
			{
				for (ULONG i = 0; i < count; ++i)
				{
					ISensor* sensor;
					hr = orientation_sensor_collection_->GetAt(i, &sensor);
					if (SUCCEEDED(hr))
					{
						sensor->SetEventSink(nullptr);
						sensor->Release();
					}
				}
			}

			orientation_sensor_events_.clear();
			orientation_sensor_collection_.reset();
		}

		::CoUninitialize();
	}

	void MsgInputSensor::OnLocationChanged(REFIID report_type, ILocationReport* location_report)
	{
		if (IID_ILatLongReport == report_type)
		{
			ILatLongReport* lat_long_report;
			if (SUCCEEDED(location_report->QueryInterface(IID_ILatLongReport,
				reinterpret_cast<void**>(&lat_long_report))))
			{
				if (lat_long_report != nullptr)
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

					lat_long_report->Release();
				}
			}
		}
	}

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

#elif defined KLAYGE_PLATFORM_WINDOWS_RUNTIME

using namespace Windows::Foundation;
using namespace Windows::Devices::Geolocation;
using namespace Windows::Devices::Sensors;

namespace KlayGE
{
	ref class MetroMsgInputSensorEvent sealed
	{
		friend MsgInputSensor;

	public:
		MetroMsgInputSensorEvent()
		{
		}

	private:
		void OnPositionChanged(Geolocator^ sender, PositionChangedEventArgs^ e)
		{
			input_sensor_->OnPositionChanged(sender, e);
		}

		void OnAccelerometeReadingChanged(Accelerometer^ sender, AccelerometerReadingChangedEventArgs^ e)
		{
			input_sensor_->OnAccelerometeReadingChanged(sender, e);
		}

		void OnGyrometerReadingChanged(Gyrometer^ sender, GyrometerReadingChangedEventArgs^ e)
		{
			input_sensor_->OnGyrometerReadingChanged(sender, e);
		}

		void OnInclinometerReadingChanged(Inclinometer^ sender, InclinometerReadingChangedEventArgs^ e)
		{
			input_sensor_->OnInclinometerReadingChanged(sender, e);
		}

		void OnCompassReadingChanged(Compass^ sender, CompassReadingChangedEventArgs^ e)
		{
			input_sensor_->OnCompassReadingChanged(sender, e);
		}

		void OnOrientationSensorReadingChanged(OrientationSensor^ sender, OrientationSensorReadingChangedEventArgs^ e)
		{
			input_sensor_->OnOrientationSensorReadingChanged(sender, e);
		}

		void BindSensor(MsgInputSensor* input_sensor)
		{
			input_sensor_ = input_sensor;
		}

	private:
		MsgInputSensor* input_sensor_;
	};

	MsgInputSensor::MsgInputSensor()
		: sensor_event_(ref new MetroMsgInputSensorEvent),
			accelerometer_(Accelerometer::GetDefault()),
			gyrometer_(Gyrometer::GetDefault()),
			inclinometer_(Inclinometer::GetDefault()),
			compass_(Compass::GetDefault()),
			orientation_(OrientationSensor::GetDefault())
	{
		sensor_event_->BindSensor(this);

		if (Context::Instance().Config().location_sensor)
		{
			locator_ = ref new Geolocator;
			position_token_ = locator_->PositionChanged::add(
				ref new TypedEventHandler<Geolocator^, PositionChangedEventArgs^>(sensor_event_,
				&MetroMsgInputSensorEvent::OnPositionChanged));
		}
		if (accelerometer_)
		{
			accelerometer_reading_token_ = accelerometer_->ReadingChanged::add(
				ref new TypedEventHandler<Accelerometer^, AccelerometerReadingChangedEventArgs^>(sensor_event_,
					&MetroMsgInputSensorEvent::OnAccelerometeReadingChanged));
		}
		if (gyrometer_)
		{
			gyrometer_reading_token_ = gyrometer_->ReadingChanged::add(
				ref new TypedEventHandler<Gyrometer^, GyrometerReadingChangedEventArgs^>(sensor_event_,
					&MetroMsgInputSensorEvent::OnGyrometerReadingChanged));
		}
		if (inclinometer_)
		{
			inclinometer_reading_token_ = inclinometer_->ReadingChanged::add(
				ref new TypedEventHandler<Inclinometer^, InclinometerReadingChangedEventArgs^>(sensor_event_,
					&MetroMsgInputSensorEvent::OnInclinometerReadingChanged));
		}
		if (compass_)
		{
			compass_reading_token_ = compass_->ReadingChanged::add(
				ref new TypedEventHandler<Compass^, CompassReadingChangedEventArgs^>(sensor_event_,
					&MetroMsgInputSensorEvent::OnCompassReadingChanged));
		}
		if (orientation_)
		{
			orientation_reading_token_ = orientation_->ReadingChanged::add(
				ref new TypedEventHandler<OrientationSensor^, OrientationSensorReadingChangedEventArgs^>(sensor_event_,
					&MetroMsgInputSensorEvent::OnOrientationSensorReadingChanged));
		}
	}

	MsgInputSensor::~MsgInputSensor()
	{
		if (Context::Instance().Config().location_sensor)
		{
			locator_->PositionChanged::remove(position_token_);
		}
		if (accelerometer_)
		{
			accelerometer_->ReadingChanged::remove(accelerometer_reading_token_);
		}
		if (gyrometer_)
		{
			gyrometer_->ReadingChanged::remove(gyrometer_reading_token_);
		}
		if (inclinometer_)
		{
			inclinometer_->ReadingChanged::remove(inclinometer_reading_token_);
		}
		if (compass_)
		{
			compass_->ReadingChanged::remove(compass_reading_token_);
		}
		if (orientation_)
		{
			orientation_->ReadingChanged::remove(orientation_reading_token_);
		}
	}

	void MsgInputSensor::OnPositionChanged(Geolocator^ sender, PositionChangedEventArgs^ e)
	{
		Geocoordinate^ coordinate = e->Position->Coordinate;
#if (_WIN32_WINNT >= _WIN32_WINNT_WINBLUE)
		latitude_ = static_cast<float>(coordinate->Point->Position.Latitude);
		longitude_ = static_cast<float>(coordinate->Point->Position.Longitude);
		altitude_ = static_cast<float>(coordinate->Point->Position.Altitude);
#else
		latitude_ = static_cast<float>(coordinate->Latitude);
		longitude_ = static_cast<float>(coordinate->Longitude);
		if (coordinate->Altitude)
		{
			altitude_ = static_cast<float>(coordinate->Altitude->Value);
		}
#endif

		location_error_radius_ = static_cast<float>(coordinate->Accuracy);
		if (coordinate->AltitudeAccuracy)
		{
			location_altitude_error_ = static_cast<float>(coordinate->AltitudeAccuracy->Value);
		}

		if (coordinate->Speed)
		{
			speed_ = static_cast<float>(coordinate->Speed->Value);
		}
	}

	void MsgInputSensor::OnAccelerometeReadingChanged(Accelerometer^ sender, AccelerometerReadingChangedEventArgs^ e)
	{
		AccelerometerReading^ reading = e->Reading;
		accel_ = float3(static_cast<float>(reading->AccelerationX), static_cast<float>(reading->AccelerationY),
			static_cast<float>(reading->AccelerationZ));
	}

	void MsgInputSensor::OnGyrometerReadingChanged(Gyrometer^ sender, GyrometerReadingChangedEventArgs^ e)
	{
		GyrometerReading^ reading = e->Reading;
		angular_velocity_ = float3(static_cast<float>(reading->AngularVelocityX), static_cast<float>(reading->AngularVelocityY),
			static_cast<float>(reading->AngularVelocityZ));
	}

	void MsgInputSensor::OnInclinometerReadingChanged(Inclinometer^ sender, InclinometerReadingChangedEventArgs^ e)
	{
		InclinometerReading^ reading = e->Reading;
		tilt_ = float3(reading->PitchDegrees, reading->RollDegrees, reading->YawDegrees);
	}

	void MsgInputSensor::OnCompassReadingChanged(Compass^ sender, CompassReadingChangedEventArgs^ e)
	{
		CompassReading^ reading = e->Reading;
		magnetic_heading_north_ = static_cast<float>(reading->HeadingMagneticNorth);
#if (_WIN32_WINNT >= _WIN32_WINNT_WINBLUE)
		magnetometer_accuracy_ = static_cast<int32_t>(reading->HeadingAccuracy);
#else
		magnetometer_accuracy_ = 0;
#endif
	}

	void MsgInputSensor::OnOrientationSensorReadingChanged(OrientationSensor^ sender,
		OrientationSensorReadingChangedEventArgs^ e)
	{
		OrientationSensorReading^ reading = e->Reading;
		orientation_quat_ = Quaternion(reading->Quaternion->X, reading->Quaternion->Y,
			reading->Quaternion->Z, reading->Quaternion->W);
	}
}

#elif defined KLAYGE_PLATFORM_ANDROID

namespace KlayGE
{
	MsgInputSensor::MsgInputSensor()
	{
		sensor_mgr_ = ASensorManager_getInstance();
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
		UNREF_PARAM(fd);
		UNREF_PARAM(events);

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
