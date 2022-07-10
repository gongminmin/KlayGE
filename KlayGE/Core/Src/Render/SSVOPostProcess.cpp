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
			: PostProcess(L"SSVO", false,
				MakeSpan<std::string>(),
				MakeSpan<std::string>({"g_buffer_rt0_tex", "depth_tex"}),
				MakeSpan<std::string>({"out_tex"}),
				RenderEffectPtr(), nullptr)
	{
		auto effect = SyncLoadRenderEffect("SSVO.fxml");
		this->Technique(effect, effect->TechniqueByName("SSVO"));

		proj_param_ = effect->ParameterByName("proj");
		inv_proj_param_ = effect->ParameterByName("inv_proj");

		upper_left_param_ = effect->ParameterByName("upper_left");
		x_dir_param_ = effect->ParameterByName("x_dir");
		y_dir_param_ = effect->ParameterByName("y_dir");

		aspect_param_ = effect_->ParameterByName("aspect");
	}

	void SSVOPostProcess::OnRenderBegin()
	{
		PostProcess::OnRenderBegin();

		auto& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		auto const& viewport = *re.CurFrameBuffer()->Viewport();

		Camera const& camera = *viewport.Camera();
		auto const & inv_proj = camera.InverseProjMatrix();
		*proj_param_ = camera.ProjMatrix();
		*inv_proj_param_ = inv_proj;

		float const flipping = re.RequiresFlipping() ? -1.0f : +1.0f;
		float3 const upper_left = MathLib::transform_coord(float3(-1, -flipping, 1), inv_proj);
		float3 const upper_right = MathLib::transform_coord(float3(+1, -flipping, 1), inv_proj);
		float3 const lower_left = MathLib::transform_coord(float3(-1, flipping, 1), inv_proj);
		*upper_left_param_ = upper_left;
		*x_dir_param_ = upper_right - upper_left;
		*y_dir_param_ = lower_left - upper_left;

		*aspect_param_ = static_cast<float>(viewport.Width()) / viewport.Height();
	}
}
