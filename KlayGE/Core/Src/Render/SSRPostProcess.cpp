/**
* @file SSRPostProcess.cpp
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

#include <KlayGE/SSRPostProcess.hpp>

namespace KlayGE
{
	SSRPostProcess::SSRPostProcess(bool multi_sample)
			: PostProcess(L"ScreenSpaceReflection", false,
				MakeSpan<std::string>({"min_samples", "max_samples"}),
				MakeSpan<std::string>({
					multi_sample ? "g_buffer_rt0_tex_ms" : "g_buffer_rt0_tex",
					multi_sample ? "g_buffer_rt1_tex_ms" : "g_buffer_rt1_tex",
					"front_side_depth_tex",
					"front_side_tex",
					multi_sample ? "foreground_depth_tex_ms" : "foreground_depth_tex"
				}),
				MakeSpan<std::string>({"output"}),
				RenderEffectPtr(), nullptr)
	{
		RenderEffectPtr effect = SyncLoadRenderEffect("SSR.fxml");
		this->Technique(effect,
			effect->TechniqueByName(multi_sample ? "ScreenSpaceReflectionPostProcessMS" : "ScreenSpaceReflectionPostProcess"));

		if (technique_ && technique_->Validate())
		{
			proj_param_ = effect->ParameterByName("proj");
			inv_proj_param_ = effect->ParameterByName("inv_proj");
			near_q_far_param_ = effect->ParameterByName("near_q_far");
			ray_length_param_ = effect->ParameterByName("ray_length");
		}

		this->SetParam(0, static_cast<int32_t>(20));
		this->SetParam(1, static_cast<int32_t>(30));
	}

	void SSRPostProcess::Apply()
	{
		Camera& camera = Context::Instance().AppInstance().ActiveCamera();
		float4 const near_q_far = camera.NearQFarParam();

		*proj_param_ = camera.ProjMatrix();
		*inv_proj_param_ = camera.InverseProjMatrix();
		*near_q_far_param_ = float3(near_q_far.x(), near_q_far.y(), near_q_far.z());
		*ray_length_param_ = camera.FarPlane() - camera.NearPlane();

		this->Render();
	}
}
