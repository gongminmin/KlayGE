/**
* @file Sensor.cpp
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

#include <boost/assert.hpp>

#include <KlayGE/Input.hpp>

namespace KlayGE
{
	InputSensor::InputSensor()
		: latitude_(-180), longitude_(-360), altitude_(-1),
			location_error_radius_(-1), location_altitude_error_(-1),
			accel_(0, 0, 0), angular_velocity_(0, 0, 0),
			tilt_(0, 0, 0), magnetic_heading_north_(-1),
			orientation_quat_(0, 0, 0, 0), magnetometer_accuracy_(0),
			action_param_(MakeSharedPtr<InputSensorActionParam>())
	{
		action_param_->type = InputEngine::IDT_Sensor;
	}

	InputSensor::~InputSensor()
	{
	}

	float InputSensor::Latitude() const
	{
		return latitude_;
	}
	float InputSensor::Longitude() const
	{
		return longitude_;
	}
	float InputSensor::Altitude() const
	{
		return altitude_;
	}
	float InputSensor::LocationErrorRadius() const
	{
		return location_error_radius_;
	}
	float InputSensor::LocationAltitudeError() const
	{
		return location_altitude_error_;
	}
	float3 const & InputSensor::Accel() const
	{
		return accel_;
	}
	float3 const & InputSensor::AngularVelocity() const
	{
		return angular_velocity_;
	}
	float3 const & InputSensor::Tilt() const
	{
		return tilt_;
	}
	float InputSensor::MagneticHeadingNorth() const
	{
		return magnetic_heading_north_;
	}
	Quaternion const & InputSensor::OrientationQuat() const
	{
		return orientation_quat_;
	}
	int32_t InputSensor::MagnetometerAccuracy() const
	{
		return magnetometer_accuracy_;
	}

	void InputSensor::ActionMap(uint32_t id, InputActionMap const & actionMap)
	{
		InputActionMap& iam = actionMaps_[id];

		for (uint16_t i = SS_Latitude; i <= SS_AnySensing; ++ i)
		{
			if (actionMap.HasAction(i))
			{
				iam.AddAction(InputActionDefine(actionMap.Action(i), i));
			}
		}
	}

	InputActionsType InputSensor::UpdateActionMap(uint32_t id)
	{
		InputActionsType ret;

		InputActionMap& iam = actionMaps_[id];

		action_param_->latitude = latitude_;
		action_param_->longitude = longitude_;
		action_param_->altitude = altitude_;
		action_param_->location_error_radius = location_error_radius_;
		action_param_->location_altitude_error = location_altitude_error_;
		action_param_->accel = accel_;
		action_param_->angular_velocity = angular_velocity_;
		action_param_->tilt = tilt_;
		action_param_->magnetic_heading_north = magnetic_heading_north_;
		action_param_->orientation_quat = orientation_quat_;
		action_param_->magnetometer_accuracy = magnetometer_accuracy_;

		bool any_sensing = false;
		if ((latitude_ <= 90) && (latitude_ >= -90))
		{
			iam.UpdateInputActions(ret, SS_Latitude, action_param_);
			any_sensing = true;
		}
		if ((longitude_ <= 180) && (longitude_ > -180))
		{
			iam.UpdateInputActions(ret, SS_Longitude, action_param_);
			any_sensing = true;
		}
		if (altitude_ >= 0)
		{
			iam.UpdateInputActions(ret, SS_Altitude, action_param_);
			any_sensing = true;
		}
		if (location_error_radius_ >= 0)
		{
			iam.UpdateInputActions(ret, SS_LocationErrorRadius, action_param_);
			any_sensing = true;
		}
		if (location_altitude_error_ >= 0)
		{
			iam.UpdateInputActions(ret, SS_LocationAltitudeError, action_param_);
			any_sensing = true;
		}
		if ((accel_.x() != 0) || (accel_.y() != 0) || (accel_.z() != 0))
		{
			iam.UpdateInputActions(ret, SS_Accel, action_param_);
			any_sensing = true;
		}
		if ((angular_velocity_.x() != 0) || (angular_velocity_.y() != 0) || (angular_velocity_.z() != 0))
		{
			iam.UpdateInputActions(ret, SS_AngularVelocity, action_param_);
			any_sensing = true;
		}
		if ((tilt_.x() != 0) || (tilt_.y() != 0) || (tilt_.z() != 0))
		{
			iam.UpdateInputActions(ret, SS_Tilt, action_param_);
			any_sensing = true;
		}
		if (magnetic_heading_north_ >= 0)
		{
			iam.UpdateInputActions(ret, SS_MagneticHeadingNorth, action_param_);
			any_sensing = true;
		}
		if ((orientation_quat_.x() != 0) || (orientation_quat_.y() != 0) || (orientation_quat_.z() != 0)
			|| (orientation_quat_.w() != 0))
		{
			iam.UpdateInputActions(ret, SS_OrientationQuat, action_param_);
			any_sensing = true;
		}
		if (magnetometer_accuracy_ > 0)
		{
			iam.UpdateInputActions(ret, SS_MagnetometerAccuracy, action_param_);
			any_sensing = true;
		}

		if (any_sensing)
		{
			iam.UpdateInputActions(ret, SS_AnySensing, action_param_);
		}

		return ret;
	}
}
