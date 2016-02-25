/**
* @file SSVOPostProcess.cpp
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
#include <KlayGE/App3D.hpp>
#include <KlayGE/Camera.hpp>
#include <KlayGE/RenderEffect.hpp>

#include <KlayGE/SSVOPostProcess.hpp>

namespace KlayGE
{
	SSVOPostProcess::SSVOPostProcess()
			: PostProcess(L"SSVO")
	{
		input_pins_.emplace_back("g_buffer_tex", TexturePtr());
		input_pins_.emplace_back("depth_tex", TexturePtr());

		output_pins_.emplace_back("out_tex", TexturePtr());

		this->Technique(SyncLoadRenderEffect("SSVO.fxml")->TechniqueByName("SSVO"));

		proj_param_ = technique_->Effect().ParameterByName("proj");
		inv_proj_param_ = technique_->Effect().ParameterByName("inv_proj");
		far_plane_param_ = technique_->Effect().ParameterByName("far_plane");
	}

	void SSVOPostProcess::OnRenderBegin()
	{
		PostProcess::OnRenderBegin();

		Camera const & camera = Context::Instance().AppInstance().ActiveCamera();
		*proj_param_ = camera.ProjMatrix();
		*inv_proj_param_ = camera.InverseProjMatrix();
		*far_plane_param_ = float2(camera.FarPlane(), 1.0f / camera.FarPlane());
	}
}
