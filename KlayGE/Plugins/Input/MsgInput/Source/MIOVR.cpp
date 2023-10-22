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
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/RenderEngine.hpp>

#include "MInput.hpp"

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

		uint32_t width;
		uint32_t height;
		if (hmd_info_.HResolution > 0)
		{
			width = hmd_info_.HResolution;
			height = hmd_info_.VResolution;
		}
		else
		{
			width = 1280;
			height = 800;
		}

		sconfig_.SetFullViewport(Util::Render::Viewport::Viewport(0, 0, width, height));
		sconfig_.SetStereoMode(Util::Render::Stereo_LeftRight_Multipass);

		if (hmd_info_.HScreenSize > 0.0f)
		{
			if (hmd_info_.HScreenSize > 0.140f) // 7" screen
			{
				sconfig_.SetDistortionFitPointVP(-1.0f, 0.0f);
			}
			else
			{
				sconfig_.SetDistortionFitPointVP(0.0f, 1.0f);
			}
		}

		sconfig_.Set2DAreaFov(DegreeToRad(85.0f));

		Util::Render::StereoEyeParams left_eye = sconfig_.GetEyeRenderParams(Util::Render::StereoEye_Left);
		float scale = sconfig_.GetDistortionScale();

		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		if (hmd_info_.VScreenSize > 0)
		{
			re.DefaultFOV(atan(hmd_info_.VScreenSize / 2 * scale / hmd_info_.EyeToScreenDistance) * 2);
			re.DefaultRenderWidthScale(scale / 2);
			re.DefaultRenderHeightScale(scale);
			re.StereoSeparation(-hmd_info_.InterpupillaryDistance * scale);
		}
		re.OVRScale(scale);
		re.OVRHMDWarpParam(float4(left_eye.pDistortion->K));
		re.OVRChromAbParam(float4(left_eye.pDistortion->ChromaticAberration));
		re.OVRXCenterOffset(left_eye.pDistortion->XCenterOffset);
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
			OVR::Quatf const & quat = sfusion_.GetPredictedOrientation();
			orientation_quat_ = Quaternion(quat.x, quat.y, quat.z, quat.w);

			OVR::Vector3f const & accel = sfusion_.GetAcceleration();
			accel_ = float3(accel.x, accel.y, accel.z);

			OVR::Vector3f const & ang_vel = sfusion_.GetAngularVelocity();
			angular_velocity_ = float3(ang_vel.x, ang_vel.y, ang_vel.z);
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
