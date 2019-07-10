/**
 * @file MotionBlur.cpp
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
#include <KFL/ErrorHandling.hpp>
#include <KlayGE/PostProcess.hpp>
#include <KlayGE/RenderView.hpp>

#include <random>

#include <KlayGE/MotionBlur.hpp>

namespace KlayGE
{
	MotionBlurPostProcess::MotionBlurPostProcess()
		: PostProcess(L"MotionBlur", false,
			MakeSpan<std::string>({"exposure", "blur_radius", "reconstruction_samples", "display_type"}),
			MakeSpan<std::string>({"color_tex", "depth_tex", "velocity_tex"}),
			MakeSpan<std::string>({"output"}),
			RenderEffectPtr(), nullptr),
			visualize_velocity_type_(VT_Result),
			exposure_(1), blur_radius_(2), reconstruction_samples_(15)
	{
		motion_blur_tile_max_x_dir_pp_ = SyncLoadPostProcess("MotionBlur.ppml", "MotionBlurTileMaxXDir");
		motion_blur_tile_max_x_dir_pp_->SetParam(0, static_cast<float>(blur_radius_));

		motion_blur_tile_max_y_dir_pp_ = SyncLoadPostProcess("MotionBlur.ppml", "MotionBlurTileMaxYDir");
		motion_blur_tile_max_y_dir_pp_->SetParam(0, static_cast<float>(blur_radius_));

		motion_blur_neighbor_max_pp_ = SyncLoadPostProcess("MotionBlur.ppml", "MotionBlurNeighborMax");

		motion_blur_gather_pp_ = SyncLoadPostProcess("MotionBlur.ppml", "MotionBlurGather");
		motion_blur_gather_pp_->SetParam(0, 0.5f * exposure_);
		motion_blur_gather_pp_->SetParam(1, static_cast<float>(blur_radius_));
		motion_blur_gather_pp_->SetParam(2, static_cast<float>(reconstruction_samples_));

		motion_blur_visualize_pp_ = SyncLoadPostProcess("MotionBlur.ppml", "MotionBlurVisualize");
	}

	void MotionBlurPostProcess::InputPin(uint32_t index, ShaderResourceViewPtr const& srv)
	{
		if ((index == 2) && srv)
		{
			uint32_t old_width = 0;
			uint32_t old_height = 0;
			auto const* velocity_srv = motion_blur_gather_pp_->InputPin(2).get();
			if (velocity_srv)
			{
				auto const* velocity_tex = velocity_srv->TextureResource().get();
				old_width = velocity_tex->Width(0);
				old_height = velocity_tex->Height(0);
			}

			motion_blur_gather_pp_->InputPin(index, srv);
			auto const& tex = srv->TextureResource();
			if ((tex->Width(0) != old_width) || (tex->Height(0) != old_height))
			{
				this->RecreateTextures(tex, srv);
			}

			motion_blur_gather_pp_->SetParam(3, (2 * tex->Height(0) + 1056) / 416.0f);
		}

		motion_blur_gather_pp_->InputPin(index, srv);
	}

	void MotionBlurPostProcess::SetParam(uint32_t index, uint32_t const & value)
	{
		switch (index)
		{
		case 1:
			if (value != blur_radius_)
			{
				blur_radius_ = value;
				auto const& srv = motion_blur_gather_pp_->InputPin(2);
				if (srv)
				{
					this->RecreateTextures(srv->TextureResource(), srv);
				}
				motion_blur_tile_max_x_dir_pp_->SetParam(0, static_cast<float>(value));
				motion_blur_tile_max_y_dir_pp_->SetParam(0, static_cast<float>(value));
				motion_blur_gather_pp_->SetParam(1, static_cast<float>(value));
			}
			break;

		case 2:
			reconstruction_samples_ = value;
			motion_blur_gather_pp_->SetParam(2, static_cast<float>(value));
			break;

		case 3:
			visualize_velocity_type_ = static_cast<VisualizeType>(value);
			this->BindVisualizeTextures();
			break;

		default:
			KFL_UNREACHABLE("Wrong param index.");
		}
	}

	void MotionBlurPostProcess::SetParam(uint32_t index, float const & value)
	{
		if (index == 0)
		{
			exposure_ = value;
			motion_blur_gather_pp_->SetParam(0, 0.5f * exposure_);
		}
		else
		{
			KFL_UNREACHABLE("Wrong param index.");
		}
	}

	void MotionBlurPostProcess::Apply()
	{
		motion_blur_tile_max_x_dir_pp_->Apply();
		motion_blur_tile_max_y_dir_pp_->Apply();
		motion_blur_neighbor_max_pp_->Apply();

		if (visualize_velocity_type_ == VT_Result)
		{
			motion_blur_gather_pp_->Apply();
		}
		else
		{
			motion_blur_visualize_pp_->Apply();
		}
	}

	void MotionBlurPostProcess::RecreateTextures(TexturePtr const& tex, ShaderResourceViewPtr const& srv)
	{
		BOOST_ASSERT(tex);

		uint32_t const tile_width = tex->Width(0) / blur_radius_;
		uint32_t const tile_height = tex->Height(0) / blur_radius_;

		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		auto velocity_tile_max_x_dir_tex =
			rf.MakeTexture2D(tile_width, tex->Height(0), 1, 1, tex->Format(), 1, 0, EAH_GPU_Read | EAH_GPU_Write);
		auto velocity_tile_max_x_dir_srv = rf.MakeTextureSrv(velocity_tile_max_x_dir_tex);
		auto velocity_tile_max_x_dir_rtv = rf.Make2DRtv(velocity_tile_max_x_dir_tex, 0, 1, 0);
		auto velocity_tile_max_tex = rf.MakeTexture2D(tile_width, tile_height, 1, 1, tex->Format(), 1, 0, EAH_GPU_Read | EAH_GPU_Write);
		auto velocity_tile_max_srv = rf.MakeTextureSrv(velocity_tile_max_tex);
		auto velocity_tile_max_rtv = rf.Make2DRtv(velocity_tile_max_tex, 0, 1, 0);
		auto velocity_neighbor_max_tex = rf.MakeTexture2D(tile_width, tile_height, 1, 1, tex->Format(), 1, 0, EAH_GPU_Read | EAH_GPU_Write);
		auto velocity_neighbor_max_srv = rf.MakeTextureSrv(velocity_neighbor_max_tex);
		auto velocity_neighbor_max_rtv = rf.Make2DRtv(velocity_neighbor_max_tex, 0, 1, 0);

		std::ranlux24_base gen;
		std::uniform_int_distribution<> random_dis(0, 255);
		std::vector<uint8_t> rand_data(tile_width * tile_height);
		for (uint32_t j = 0; j < tile_height; ++j)
		{
			for (uint32_t i = 0; i < tile_width; ++i)
			{
				rand_data[j * tile_width + i] = static_cast<uint8_t>(random_dis(gen));
			}
		}
		ElementInitData init_data;
		init_data.data = &rand_data[0];
		init_data.row_pitch = tile_width;
		init_data.slice_pitch = tile_width * tile_height;
		auto random_srv = rf.MakeTextureSrv(
			rf.MakeTexture2D(tile_width, tile_height, 1, 1, EF_R8, 1, 0, EAH_GPU_Read | EAH_Immutable, MakeSpan<1>(init_data)));

		motion_blur_tile_max_x_dir_pp_->InputPin(0, srv);
		motion_blur_tile_max_x_dir_pp_->OutputPin(0, velocity_tile_max_x_dir_rtv);

		motion_blur_tile_max_y_dir_pp_->InputPin(0, velocity_tile_max_x_dir_srv);
		motion_blur_tile_max_y_dir_pp_->OutputPin(0, velocity_tile_max_rtv);

		motion_blur_neighbor_max_pp_->InputPin(0, velocity_tile_max_srv);
		motion_blur_neighbor_max_pp_->OutputPin(0, velocity_neighbor_max_rtv);

		motion_blur_gather_pp_->InputPin(3, velocity_neighbor_max_srv);
		motion_blur_gather_pp_->InputPin(4, random_srv);

		this->BindVisualizeTextures();
	}

	void MotionBlurPostProcess::BindVisualizeTextures()
	{
		switch (visualize_velocity_type_)
		{
		case VT_VelocityTileMax:
			motion_blur_visualize_pp_->InputPin(0, motion_blur_neighbor_max_pp_->InputPin(0));
			break;

		case VT_VelocityNeighborMax:
			motion_blur_visualize_pp_->InputPin(0, motion_blur_gather_pp_->InputPin(3));
			break;

		case VT_Velocity:
		case VT_Result:
		default:
			motion_blur_visualize_pp_->InputPin(0, motion_blur_gather_pp_->InputPin(2));
			break;
		}
	}
}
