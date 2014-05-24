/**
* @file MIOVR.cpp
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

#if (defined KLAYGE_PLATFORM_WINDOWS_DESKTOP) && (defined KLAYGE_HAVE_LIBOVR)

#include <OVR.h>
using namespace OVR;

namespace KlayGE
{
	MsgInputOVR::MsgInputOVR()
	{
		manager_ = *DeviceManager::Create();
		manager_->SetMessageHandler(this);

		hmd_ = *manager_->EnumerateDevices<HMDDevice>().CreateDevice();
		if (hmd_)
		{
			sensor_ = *hmd_->GetSensor();
			if (hmd_->GetDeviceInfo(&hmd_info_))
			{
				sconfig_.SetHMDInfo(hmd_info_);
			}
		}
		else
		{
			sensor_ = *manager_->EnumerateDevices<SensorDevice>().CreateDevice();
		}

		if (sensor_)
		{
			sfusion_.AttachToSensor(sensor_);
			sfusion_.SetDelegateMessageHandler(this);
			sfusion_.SetPredictionEnabled(true);
		}

		sconfig_.SetFullViewport(Util::Render::Viewport::Viewport(0, 0, hmd_info_.HResolution, hmd_info_.VResolution));
		sconfig_.SetStereoMode(Util::Render::Stereo_LeftRight_Multipass);

		if (hmd_info_.HScreenSize > 0.0f)
		{
			if (hmd_info_.HScreenSize > 0.140f) // 7"
			{
				sconfig_.SetDistortionFitPointVP(-1.0f, 0.0f);
			}
			else
			{
				sconfig_.SetDistortionFitPointVP(0.0f, 1.0f);
			}
		}

		sconfig_.Set2DAreaFov(DegreeToRad(85.0f));
	}

	MsgInputOVR::~MsgInputOVR()
	{
		this->RemoveHandlerFromDevices();
		sensor_.Clear();
		hmd_.Clear();
	}

	std::wstring const & MsgInputOVR::Name() const
	{
		static std::wstring const name(L"MsgInput OculusVR");
		return name;
	}

	void MsgInputOVR::UpdateInputs()
	{
		if (sensor_)
		{
			orientation_quat_ = Quaternion(sfusion_.GetOrientation().x, 
				sfusion_.GetOrientation().y,
				sfusion_.GetOrientation().z,
				sfusion_.GetOrientation().w);
		}
	}

	void MsgInputOVR::OnMessage(const Message& msg)
	{
		if ((Message_DeviceAdded == msg.Type) && (msg.pDevice == manager_))
		{
		}
		else if ((Message_DeviceRemoved == msg.Type) && (msg.pDevice == manager_))
		{
		}
		else if ((Message_DeviceAdded == msg.Type) && (msg.pDevice == sensor_))
		{
		}
		else if ((Message_DeviceRemoved == msg.Type) && (msg.pDevice == sensor_))
		{
		}
	}
}

#endif
