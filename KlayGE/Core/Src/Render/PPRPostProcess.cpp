/**
* @file PPRPostProcess.cpp
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

#include <KlayGE/PPRPostProcess.hpp>

namespace KlayGE
{
	PPRPostProcess::PPRPostProcess(bool multi_sample)
		: PostProcess(L"PixelProjectedReflection", false, MakeSpan<std::string>({"plane"}),
			  MakeSpan<std::string>(
				  {multi_sample ? "g_buffer_rt0_tex_ms" : "g_buffer_rt0_tex", multi_sample ? "g_buffer_rt1_tex_ms" : "g_buffer_rt1_tex",
					  "shading_tex", multi_sample ? "depth_tex_ms" : "depth_tex", "ppr_skybox_tex", "ppr_skybox_C_tex"}),
			  MakeSpan<std::string>({"output"}), RenderEffectPtr(), nullptr)
	{
#ifdef KLAYGE_DEBUG
		auto& rf = Context::Instance().RenderFactoryInstance();
		auto& re = rf.RenderEngineInstance();
		auto const& caps = re.DeviceCaps();
		BOOST_ASSERT((caps.max_shader_model >= ShaderModel(5, 0)) && caps.cs_support);
#endif

		auto effect = SyncLoadRenderEffect("PPR.fxml");
		coord_tech_ = effect->TechniqueByName(multi_sample ? "PixelProjectedReflectionCoordMS" : "PixelProjectedReflectionCoord");
		project_tech_ =
			effect->TechniqueByName(multi_sample ? "PixelProjectedReflectionPostProcessMS" : "PixelProjectedReflectionPostProcess");
		this->Technique(effect, project_tech_);

		inv_view_param_ = effect_->ParameterByName("inv_view");
		inv_proj_param_ = effect_->ParameterByName("inv_proj");
		view_to_reflected_proj_param_ = effect_->ParameterByName("view_to_reflected_proj");
		reflection_width_height_param_ = effect_->ParameterByName("reflection_width_height");
		upper_left_param_ = effect_->ParameterByName("upper_left");
		xy_dir_param_ = effect_->ParameterByName("xy_dir");
		far_plane_param_ = effect_->ParameterByName("far_plane");

		rw_coord_tex_param_ = effect_->ParameterByName("rw_coord_tex");
		coord_tex_param_ = effect_->ParameterByName("coord_tex");
	}

	void PPRPostProcess::SetParam(uint32_t index, float4 const& value)
	{
		if (index == 0)
		{
			reflect_plane_ = reinterpret_cast<Plane const&>(value);
		}
		else
		{
			PostProcess::SetParam(index, value);
		}
	}

	void PPRPostProcess::GetParam(uint32_t index, float4& value)
	{
		if (index == 0)
		{
			value = reinterpret_cast<float4 const&>(reflect_plane_);
		}
		else
		{
			PostProcess::GetParam(index, value);
		}	
	}

	void PPRPostProcess::InputPin(uint32_t index, ShaderResourceViewPtr const& srv)
	{
		PostProcess::InputPin(index, srv);

		if (index == 2)
		{
			auto const& tex = srv->TextureResource();
			uint32_t const width = tex->Width(0);
			uint32_t const height = tex->Height(0);
			*reflection_width_height_param_ = float4(static_cast<float>(width), static_cast<float>(height), 1.0f / width, 1.0f / height);

			if (!coord_tex_ || (coord_tex_->Width(0) != width) || (coord_tex_->Height(0) != height))
			{
				auto& rf = Context::Instance().RenderFactoryInstance();

				coord_tex_ = rf.MakeTexture2D(width, height, 1, 1, EF_R32UI, 1, 0, EAH_GPU_Read | EAH_GPU_Unordered);
				KLAYGE_TEXTURE_DEBUG_NAME(coord_tex_);

				*coord_tex_param_ = rf.MakeTextureSrv(coord_tex_);
				coord_uav_ = rf.Make2DUav(coord_tex_, 0, 1, 0);

				*rw_coord_tex_param_ = coord_uav_;
			}
		}
	}

	void PPRPostProcess::Apply()
	{
		auto& rf = Context::Instance().RenderFactoryInstance();
		auto& re = rf.RenderEngineInstance();

		Camera& camera = Context::Instance().AppInstance().ActiveCamera();
		float4x4 const& inv_proj = camera.InverseProjMatrix();

		*inv_view_param_ = camera.InverseViewMatrix();
		*inv_proj_param_ = inv_proj;

		float const flipping = re.RequiresFlipping() ? -1.0f : +1.0f;
		float3 const upper_left = MathLib::transform_coord(float3(-1, -flipping, 1), inv_proj);
		float3 const upper_right = MathLib::transform_coord(float3(+1, -flipping, 1), inv_proj);
		float3 const lower_left = MathLib::transform_coord(float3(-1, flipping, 1), inv_proj);
		*upper_left_param_ = upper_left;
		*xy_dir_param_ = float2(upper_right.x() - upper_left.x(), lower_left.y() - upper_left.y());
		*far_plane_param_ = camera.FarPlane();

		*view_to_reflected_proj_param_ = camera.InverseViewMatrix() * MathLib::reflect(reflect_plane_) * camera.ViewProjMatrix();

		coord_uav_->Clear(uint4(0xFFFFFFFFU, 0, 0, 0));
		re.Dispatch(*effect_, *coord_tech_, (coord_tex_->Width(0) + 15) / 16, (coord_tex_->Height(0) + 15) / 16, 1);

		this->Render();
	}
}
