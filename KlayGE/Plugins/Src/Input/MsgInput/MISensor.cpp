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
	MsgInputSensor::MsgInputSensor()
		: location_sensor_(false), motion_sensor_collection_(nullptr), orientation_sensor_collection_(nullptr)
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
			}

			ISensorCollection* orientation_sensor_collection = nullptr;
			hr = sensor_mgr->GetSensorsByCategory(SENSOR_CATEGORY_ORIENTATION, &orientation_sensor_collection);
			if (SUCCEEDED(hr))
			{
				orientation_sensor_collection_ = MakeCOMPtr(orientation_sensor_collection);
			}

			sensor_mgr->Release();
		}
	}

	std::wstring const & MsgInputSensor::Name() const
	{
		static std::wstring const name(L"MsgInput Sensor");
		return name;
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

		if (motion_sensor_collection_)
		{
			ULONG count = 0;
			HRESULT hr = motion_sensor_collection_->GetCount(&count);
			for (ULONG i = 0; i < count; ++ i)
			{
				ISensor* sensor;
				hr = motion_sensor_collection_->GetAt(i, &sensor);
				if (SUCCEEDED(hr))
				{
					ISensorDataReport* data_report;
					hr = sensor->GetData(&data_report);
					if (SUCCEEDED(hr))
					{
						VARIANT_BOOL supported = VARIANT_FALSE;

						sensor->SupportsDataField(SENSOR_DATA_TYPE_ACCELERATION_X_G, &supported);
						if (supported != 0)
						{
							PROPVARIANT prop;
							data_report->GetSensorValue(SENSOR_DATA_TYPE_ACCELERATION_X_G, &prop);
							if (VT_R8 == prop.vt)
							{
								accel_.x() = static_cast<float>(prop.dblVal);
							}
						}

						hr = sensor->SupportsDataField(SENSOR_DATA_TYPE_ACCELERATION_Y_G, &supported);
						if (supported != 0)
						{
							PROPVARIANT prop;
							data_report->GetSensorValue(SENSOR_DATA_TYPE_ACCELERATION_Y_G, &prop);
							if (VT_R8 == prop.vt)
							{
								accel_.y() = static_cast<float>(prop.dblVal);
							}
						}

						hr = sensor->SupportsDataField(SENSOR_DATA_TYPE_ACCELERATION_Z_G, &supported);
						if (supported != 0)
						{
							PROPVARIANT prop;
							data_report->GetSensorValue(SENSOR_DATA_TYPE_ACCELERATION_Z_G, &prop);
							if (VT_R8 == prop.vt)
							{
								accel_.z() = static_cast<float>(prop.dblVal);
							}
						}

						hr = sensor->SupportsDataField(SENSOR_DATA_TYPE_SPEED_METERS_PER_SECOND, &supported);
						if (supported != 0)
						{
							PROPVARIANT prop;
							data_report->GetSensorValue(SENSOR_DATA_TYPE_SPEED_METERS_PER_SECOND, &prop);
							if (VT_R8 == prop.vt)
							{
								speed_ = static_cast<float>(prop.dblVal);
							}
						}

						hr = sensor->SupportsDataField(SENSOR_DATA_TYPE_ANGULAR_ACCELERATION_X_DEGREES_PER_SECOND_SQUARED, &supported);
						if (supported != 0)
						{
							PROPVARIANT prop;
							data_report->GetSensorValue(SENSOR_DATA_TYPE_ANGULAR_ACCELERATION_X_DEGREES_PER_SECOND_SQUARED, &prop);
							if (VT_R8 == prop.vt)
							{
								angular_accel_.x() = static_cast<float>(prop.dblVal);
							}
						}

						hr = sensor->SupportsDataField(SENSOR_DATA_TYPE_ANGULAR_ACCELERATION_Y_DEGREES_PER_SECOND_SQUARED, &supported);
						if (supported != 0)
						{
							PROPVARIANT prop;
							data_report->GetSensorValue(SENSOR_DATA_TYPE_ANGULAR_ACCELERATION_Y_DEGREES_PER_SECOND_SQUARED, &prop);
							if (VT_R8 == prop.vt)
							{
								angular_accel_.y() = static_cast<float>(prop.dblVal);
							}
						}

						hr = sensor->SupportsDataField(SENSOR_DATA_TYPE_ANGULAR_ACCELERATION_Z_DEGREES_PER_SECOND_SQUARED, &supported);
						if (supported != 0)
						{
							PROPVARIANT prop;
							data_report->GetSensorValue(SENSOR_DATA_TYPE_ANGULAR_ACCELERATION_Z_DEGREES_PER_SECOND_SQUARED, &prop);
							if (VT_R8 == prop.vt)
							{
								angular_accel_.z() = static_cast<float>(prop.dblVal);
							}
						}

						hr = sensor->SupportsDataField(SENSOR_DATA_TYPE_ANGULAR_VELOCITY_X_DEGREES_PER_SECOND, &supported);
						if (supported != 0)
						{
							PROPVARIANT prop;
							data_report->GetSensorValue(SENSOR_DATA_TYPE_ANGULAR_VELOCITY_X_DEGREES_PER_SECOND, &prop);
							if (VT_R8 == prop.vt)
							{
								angular_velocity_.x() = static_cast<float>(prop.dblVal);
							}
						}

						hr = sensor->SupportsDataField(SENSOR_DATA_TYPE_ANGULAR_VELOCITY_Y_DEGREES_PER_SECOND, &supported);
						if (supported != 0)
						{
							PROPVARIANT prop;
							data_report->GetSensorValue(SENSOR_DATA_TYPE_ANGULAR_VELOCITY_Y_DEGREES_PER_SECOND, &prop);
							if (VT_R8 == prop.vt)
							{
								angular_velocity_.y() = static_cast<float>(prop.dblVal);
							}
						}

						hr = sensor->SupportsDataField(SENSOR_DATA_TYPE_ANGULAR_VELOCITY_Z_DEGREES_PER_SECOND, &supported);
						if (supported != 0)
						{
							PROPVARIANT prop;
							data_report->GetSensorValue(SENSOR_DATA_TYPE_ANGULAR_VELOCITY_Z_DEGREES_PER_SECOND, &prop);
							if (VT_R8 == prop.vt)
							{
								angular_velocity_.z() = static_cast<float>(prop.dblVal);
							}
						}

						data_report->Release();
					}

					sensor->Release();
				}
			}
		}

		if (orientation_sensor_collection_)
		{
			ULONG count = 0;
			HRESULT hr = orientation_sensor_collection_->GetCount(&count);
			for (ULONG i = 0; i < count; ++ i)
			{
				ISensor* sensor;
				hr = orientation_sensor_collection_->GetAt(i, &sensor);
				if (SUCCEEDED(hr))
				{
					ISensorDataReport* data_report;
					hr = sensor->GetData(&data_report);
					if (SUCCEEDED(hr))
					{
						VARIANT_BOOL supported = VARIANT_FALSE;

						sensor->SupportsDataField(SENSOR_DATA_TYPE_TILT_X_DEGREES, &supported);
						if (supported != 0)
						{
							PROPVARIANT prop;
							data_report->GetSensorValue(SENSOR_DATA_TYPE_TILT_X_DEGREES, &prop);
							if (VT_R4 == prop.vt)
							{
								tilt_.x() = prop.fltVal;
							}
						}

						hr = sensor->SupportsDataField(SENSOR_DATA_TYPE_TILT_Y_DEGREES, &supported);
						if (supported != 0)
						{
							PROPVARIANT prop;
							data_report->GetSensorValue(SENSOR_DATA_TYPE_TILT_Y_DEGREES, &prop);
							if (VT_R4 == prop.vt)
							{
								tilt_.y() = prop.fltVal;
							}
						}

						hr = sensor->SupportsDataField(SENSOR_DATA_TYPE_TILT_Z_DEGREES, &supported);
						if (supported != 0)
						{
							PROPVARIANT prop;
							data_report->GetSensorValue(SENSOR_DATA_TYPE_TILT_Z_DEGREES, &prop);
							if (VT_R8 == prop.vt)
							{
								tilt_.z() = prop.fltVal;
							}
						}

						hr = sensor->SupportsDataField(SENSOR_DATA_TYPE_MAGNETIC_HEADING_X_DEGREES, &supported);
						if (supported != 0)
						{
							PROPVARIANT prop;
							data_report->GetSensorValue(SENSOR_DATA_TYPE_MAGNETIC_HEADING_X_DEGREES, &prop);
							if (VT_R4 == prop.vt)
							{
								magnetic_heading_.x() = prop.fltVal;
							}
						}

						hr = sensor->SupportsDataField(SENSOR_DATA_TYPE_MAGNETIC_HEADING_Y_DEGREES, &supported);
						if (supported != 0)
						{
							PROPVARIANT prop;
							data_report->GetSensorValue(SENSOR_DATA_TYPE_MAGNETIC_HEADING_Y_DEGREES, &prop);
							if (VT_R4 == prop.vt)
							{
								magnetic_heading_.y() = prop.fltVal;
							}
						}

						hr = sensor->SupportsDataField(SENSOR_DATA_TYPE_MAGNETIC_HEADING_Z_DEGREES, &supported);
						if (supported != 0)
						{
							PROPVARIANT prop;
							data_report->GetSensorValue(SENSOR_DATA_TYPE_MAGNETIC_HEADING_Z_DEGREES, &prop);
							if (VT_R4 == prop.vt)
							{
								magnetic_heading_.z() = prop.fltVal;
							}
						}

						data_report->Release();
					}

					sensor->Release();
				}
			}
		}
	}
}
#endif
