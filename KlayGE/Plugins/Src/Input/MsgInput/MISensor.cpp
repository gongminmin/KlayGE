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
#include <KlayGE/App3D.hpp>
#include <KlayGE/Window.hpp>

#include <KlayGE/MsgInput/MInput.hpp>

#if defined KLAYGE_PLATFORM_WINDOWS_DESKTOP && (_WIN32_WINNT >= 0x0601 /*_WIN32_WINNT_WIN7*/)
namespace KlayGE
{
	class MsgInputSensorEvents : public ISensorEvents
	{
	public:
		explicit MsgInputSensorEvents(MsgInputSensor* input_sensor)
			: input_sensor_(input_sensor)
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
				*ppv = NULL;
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
		::CoInitialize(0);

		ILocation* location = nullptr;
		HRESULT hr = ::CoCreateInstance(CLSID_Location, nullptr, CLSCTX_INPROC_SERVER,
			IID_ILocation, reinterpret_cast<void**>(&location));
		if (SUCCEEDED(hr))
		{
			IID REPORT_TYPES[] = { IID_ILatLongReport };
			hr = location->RequestPermissions(nullptr, REPORT_TYPES,
				sizeof(REPORT_TYPES) / sizeof(REPORT_TYPES[0]), true);
			if (SUCCEEDED(hr))
			{
				location_sensor_ = MakeCOMPtr(location);
			}
			else
			{
				location->Release();
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
				HRESULT hr = motion_sensor_collection_->GetCount(&count);
				for (ULONG i = 0; i < count; ++ i)
				{
					ISensor* sensor;
					hr = motion_sensor_collection_->GetAt(i, &sensor);
					if (SUCCEEDED(hr))
					{
						shared_ptr<MsgInputMotionSensorEvents> motion_event = MakeSharedPtr<MsgInputMotionSensorEvents>(this);
						ISensorEvents* sensor_event;
						motion_event->QueryInterface(IID_ISensorEvents, reinterpret_cast<void**>(&sensor_event));
						motion_sensor_events_.push_back(motion_event);

						sensor->SetEventSink(sensor_event);

						sensor_event->Release();
						sensor->Release();
					}
				}
			}

			ISensorCollection* orientation_sensor_collection = nullptr;
			hr = sensor_mgr->GetSensorsByCategory(SENSOR_CATEGORY_ORIENTATION, &orientation_sensor_collection);
			if (SUCCEEDED(hr))
			{
				orientation_sensor_collection_ = MakeCOMPtr(orientation_sensor_collection);

				ULONG count = 0;
				HRESULT hr = orientation_sensor_collection_->GetCount(&count);
				for (ULONG i = 0; i < count; ++ i)
				{
					ISensor* sensor;
					hr = orientation_sensor_collection_->GetAt(i, &sensor);
					if (SUCCEEDED(hr))
					{
						shared_ptr<MsgInputOrientationSensorEvents> orientation_event = MakeSharedPtr<MsgInputOrientationSensorEvents>(this);
						ISensorEvents* sensor_event;
						orientation_event->QueryInterface(IID_ISensorEvents, reinterpret_cast<void**>(&sensor_event));
						orientation_sensor_events_.push_back(orientation_event);

						sensor->SetEventSink(sensor_event);

						sensor_event->Release();
						sensor->Release();
					}
				}
			}

			sensor_mgr->Release();
		}
	}

	std::wstring const & MsgInputSensor::Name() const
	{
		static std::wstring const name(L"MsgInput Sensor");
		return name;
	}

	void MsgInputSensor::OnMotionDataUpdated(ISensor* sensor, ISensorDataReport* data_report)
	{
		VARIANT_BOOL supported;

		supported = VARIANT_FALSE;
		sensor->SupportsDataField(SENSOR_DATA_TYPE_ACCELERATION_X_G, &supported);
		if (supported != VARIANT_FALSE)
		{
			PROPVARIANT prop;
			data_report->GetSensorValue(SENSOR_DATA_TYPE_ACCELERATION_X_G, &prop);
			if (VT_R8 == prop.vt)
			{
				accel_.x() = static_cast<float>(prop.dblVal);
			}
		}

		supported = VARIANT_FALSE;
		sensor->SupportsDataField(SENSOR_DATA_TYPE_ACCELERATION_Y_G, &supported);
		if (supported != VARIANT_FALSE)
		{
			PROPVARIANT prop;
			data_report->GetSensorValue(SENSOR_DATA_TYPE_ACCELERATION_Y_G, &prop);
			if (VT_R8 == prop.vt)
			{
				accel_.y() = static_cast<float>(prop.dblVal);
			}
		}

		supported = VARIANT_FALSE;
		sensor->SupportsDataField(SENSOR_DATA_TYPE_ACCELERATION_Z_G, &supported);
		if (supported != VARIANT_FALSE)
		{
			PROPVARIANT prop;
			data_report->GetSensorValue(SENSOR_DATA_TYPE_ACCELERATION_Z_G, &prop);
			if (VT_R8 == prop.vt)
			{
				accel_.z() = static_cast<float>(prop.dblVal);
			}
		}

		supported = VARIANT_FALSE;
		sensor->SupportsDataField(SENSOR_DATA_TYPE_SPEED_METERS_PER_SECOND, &supported);
		if (supported != VARIANT_FALSE)
		{
			PROPVARIANT prop;
			data_report->GetSensorValue(SENSOR_DATA_TYPE_SPEED_METERS_PER_SECOND, &prop);
			if (VT_R8 == prop.vt)
			{
				speed_ = static_cast<float>(prop.dblVal);
			}
		}

		supported = VARIANT_FALSE;
		sensor->SupportsDataField(SENSOR_DATA_TYPE_ANGULAR_ACCELERATION_X_DEGREES_PER_SECOND_SQUARED, &supported);
		if (supported != VARIANT_FALSE)
		{
			PROPVARIANT prop;
			data_report->GetSensorValue(SENSOR_DATA_TYPE_ANGULAR_ACCELERATION_X_DEGREES_PER_SECOND_SQUARED, &prop);
			if (VT_R8 == prop.vt)
			{
				angular_accel_.x() = static_cast<float>(prop.dblVal);
			}
		}

		supported = VARIANT_FALSE;
		sensor->SupportsDataField(SENSOR_DATA_TYPE_ANGULAR_ACCELERATION_Y_DEGREES_PER_SECOND_SQUARED, &supported);
		if (supported != VARIANT_FALSE)
		{
			PROPVARIANT prop;
			data_report->GetSensorValue(SENSOR_DATA_TYPE_ANGULAR_ACCELERATION_Y_DEGREES_PER_SECOND_SQUARED, &prop);
			if (VT_R8 == prop.vt)
			{
				angular_accel_.y() = static_cast<float>(prop.dblVal);
			}
		}

		supported = VARIANT_FALSE;
		sensor->SupportsDataField(SENSOR_DATA_TYPE_ANGULAR_ACCELERATION_Z_DEGREES_PER_SECOND_SQUARED, &supported);
		if (supported != VARIANT_FALSE)
		{
			PROPVARIANT prop;
			data_report->GetSensorValue(SENSOR_DATA_TYPE_ANGULAR_ACCELERATION_Z_DEGREES_PER_SECOND_SQUARED, &prop);
			if (VT_R8 == prop.vt)
			{
				angular_accel_.z() = static_cast<float>(prop.dblVal);
			}
		}

		supported = VARIANT_FALSE;
		sensor->SupportsDataField(SENSOR_DATA_TYPE_ANGULAR_VELOCITY_X_DEGREES_PER_SECOND, &supported);
		if (supported != VARIANT_FALSE)
		{
			PROPVARIANT prop;
			data_report->GetSensorValue(SENSOR_DATA_TYPE_ANGULAR_VELOCITY_X_DEGREES_PER_SECOND, &prop);
			if (VT_R8 == prop.vt)
			{
				angular_velocity_.x() = static_cast<float>(prop.dblVal);
			}
		}

		supported = VARIANT_FALSE;
		sensor->SupportsDataField(SENSOR_DATA_TYPE_ANGULAR_VELOCITY_Y_DEGREES_PER_SECOND, &supported);
		if (supported != VARIANT_FALSE)
		{
			PROPVARIANT prop;
			data_report->GetSensorValue(SENSOR_DATA_TYPE_ANGULAR_VELOCITY_Y_DEGREES_PER_SECOND, &prop);
			if (VT_R8 == prop.vt)
			{
				angular_velocity_.y() = static_cast<float>(prop.dblVal);
			}
		}

		supported = VARIANT_FALSE;
		sensor->SupportsDataField(SENSOR_DATA_TYPE_ANGULAR_VELOCITY_Z_DEGREES_PER_SECOND, &supported);
		if (supported != VARIANT_FALSE)
		{
			PROPVARIANT prop;
			data_report->GetSensorValue(SENSOR_DATA_TYPE_ANGULAR_VELOCITY_Z_DEGREES_PER_SECOND, &prop);
			if (VT_R8 == prop.vt)
			{
				angular_velocity_.z() = static_cast<float>(prop.dblVal);
			}
		}
	}

	void MsgInputSensor::OnOrientationDataUpdated(ISensor* sensor, ISensorDataReport* data_report)
	{
		VARIANT_BOOL supported;

		supported = VARIANT_FALSE;
		sensor->SupportsDataField(SENSOR_DATA_TYPE_TILT_X_DEGREES, &supported);
		if (supported != VARIANT_FALSE)
		{
			PROPVARIANT prop;
			data_report->GetSensorValue(SENSOR_DATA_TYPE_TILT_X_DEGREES, &prop);
			if (VT_R4 == prop.vt)
			{
				tilt_.x() = prop.fltVal;
			}
		}

		supported = VARIANT_FALSE;
		sensor->SupportsDataField(SENSOR_DATA_TYPE_TILT_Y_DEGREES, &supported);
		if (supported != VARIANT_FALSE)
		{
			PROPVARIANT prop;
			data_report->GetSensorValue(SENSOR_DATA_TYPE_TILT_Y_DEGREES, &prop);
			if (VT_R4 == prop.vt)
			{
				tilt_.y() = prop.fltVal;
			}
		}

		supported = VARIANT_FALSE;
		sensor->SupportsDataField(SENSOR_DATA_TYPE_TILT_Z_DEGREES, &supported);
		if (supported != VARIANT_FALSE)
		{
			PROPVARIANT prop;
			data_report->GetSensorValue(SENSOR_DATA_TYPE_TILT_Z_DEGREES, &prop);
			if (VT_R4 == prop.vt)
			{
				tilt_.z() = prop.fltVal;
			}
		}

		supported = VARIANT_FALSE;
		sensor->SupportsDataField(SENSOR_DATA_TYPE_MAGNETIC_HEADING_X_DEGREES, &supported);
		if (supported != VARIANT_FALSE)
		{
			PROPVARIANT prop;
			data_report->GetSensorValue(SENSOR_DATA_TYPE_MAGNETIC_HEADING_X_DEGREES, &prop);
			if (VT_R4 == prop.vt)
			{
				magnetic_heading_.x() = prop.fltVal;
			}
		}

		supported = VARIANT_FALSE;
		sensor->SupportsDataField(SENSOR_DATA_TYPE_MAGNETIC_HEADING_Y_DEGREES, &supported);
		if (supported != VARIANT_FALSE)
		{
			PROPVARIANT prop;
			data_report->GetSensorValue(SENSOR_DATA_TYPE_MAGNETIC_HEADING_Y_DEGREES, &prop);
			if (VT_R4 == prop.vt)
			{
				magnetic_heading_.y() = prop.fltVal;
			}
		}

		supported = VARIANT_FALSE;
		sensor->SupportsDataField(SENSOR_DATA_TYPE_MAGNETIC_HEADING_Z_DEGREES, &supported);
		if (supported != VARIANT_FALSE)
		{
			PROPVARIANT prop;
			data_report->GetSensorValue(SENSOR_DATA_TYPE_MAGNETIC_HEADING_Z_DEGREES, &prop);
			if (VT_R4 == prop.vt)
			{
				magnetic_heading_.z() = prop.fltVal;
			}
		}

		supported = VARIANT_FALSE;
		sensor->SupportsDataField(SENSOR_DATA_TYPE_QUATERNION, &supported);
		if (supported != VARIANT_FALSE)
		{
			PROPVARIANT prop;
			data_report->GetSensorValue(SENSOR_DATA_TYPE_QUATERNION, &prop);
			if ((VT_VECTOR | VT_UI1) == prop.vt)
			{
				orientation_quat_.x() = prop.caub.pElems[0] / 255.0f * 2 - 1;
				orientation_quat_.y() = prop.caub.pElems[1] / 255.0f * 2 - 1;
				orientation_quat_.z() = prop.caub.pElems[2] / 255.0f * 2 - 1;
				orientation_quat_.w() = prop.caub.pElems[3] / 255.0f;
			}
		}

		supported = VARIANT_FALSE;
		sensor->SupportsDataField(SENSOR_DATA_TYPE_MAGNETOMETER_ACCURACY, &supported);
		if (supported != VARIANT_FALSE)
		{
			PROPVARIANT prop;
			data_report->GetSensorValue(SENSOR_DATA_TYPE_MAGNETOMETER_ACCURACY, &prop);
			if (VT_I4 == prop.vt)
			{
				magnetometer_accuracy_ = prop.intVal;
			}
		}
	}

	void MsgInputSensor::UpdateInputs()
	{
		if (location_sensor_)
		{
			LOCATION_REPORT_STATUS status = REPORT_NOT_SUPPORTED;
			HRESULT hr = location_sensor_->GetReportStatus(IID_ILatLongReport, &status);
			if (SUCCEEDED(hr) && (REPORT_RUNNING == status))
			{
				ILocationReport* location_report;
				ILatLongReport* latLong_report;
				if (SUCCEEDED(location_sensor_->GetReport(IID_ILatLongReport, &location_report)))
				{
					if (SUCCEEDED(location_report->QueryInterface(&latLong_report)))
					{
						double lat = 0;
						double lng = 0;
						double alt = 0;
						double err_radius = 0;
						double alt_err = 0;

						// Fetch the latitude & longitude
						latLong_report->GetLatitude(&lat);
						latLong_report->GetLongitude(&lng);
						latLong_report->GetAltitude(&alt);
						latLong_report->GetErrorRadius(&err_radius);
						latLong_report->GetAltitudeError(&alt_err);

						latitude_ = static_cast<float>(lat);
						longitude_ = static_cast<float>(lng);
						altitude_ = static_cast<float>(alt);
						location_error_radius_ = static_cast<float>(err_radius);
						location_altitude_error_ = static_cast<float>(alt_err);

						latLong_report->Release();
					}

					location_report->Release();
				}
			}
		}
	}
}
#endif
