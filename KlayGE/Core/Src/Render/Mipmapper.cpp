/**
 * @file Mipmapper.cpp
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

#include <KFL/CXX2a/format.hpp>
#include <KFL/ErrorHandling.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/FrameBuffer.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderFactory.hpp>

#include <KlayGE/Mipmapper.hpp>

namespace KlayGE
{
	Mipmapper::Mipmapper()
	{
		auto const& caps = Context::Instance().RenderFactoryInstance().RenderEngineInstance().DeviceCaps();
		if (caps.cs_support && caps.max_shader_model >= ShaderModel(5, 0))
		{
			effect_ = SyncLoadRenderEffect("Mipmapper.fxml");
			mipmap_tech_[0][0] = effect_->TechniqueByName("Mipmap2DPoint");
			mipmap_tech_[0][1] = effect_->TechniqueByName("Mipmap2DLinear");
			mipmap_tech_[1][0] = effect_->TechniqueByName("Mipmap2DArrayPoint");
			mipmap_tech_[1][1] = effect_->TechniqueByName("Mipmap2DArrayLinear");
			src_level_width_height_param_ = effect_->ParameterByName("src_level_width_height");
			num_levels_param_ = effect_->ParameterByName("num_levels");
			src_2d_tex_param_ = effect_->ParameterByName("src_2d_tex");
			for (uint32_t i = 1; i < MAX_LEVELS; ++i)
			{
				output_2d_tex_param_[i - 1] = effect_->ParameterByName(std::format("output_2d_tex_{}", i));
				output_2d_tex_array_param_[i - 1] = effect_->ParameterByName(std::format("output_2d_tex_array_{}", i));
			}
		}

		blitter_ = MakeUniquePtr<Blitter>();
	}

	void Mipmapper::BuildSubLevels(TexturePtr const& texture, TextureFilter filter) const
	{
		for (uint32_t array_index = 0; array_index < texture->ArraySize(); ++array_index)
		{
			this->BuildSubLevels(texture, array_index, 0, texture->NumMipMaps(), filter);
		}
	}

	void Mipmapper::BuildSubLevels(
		TexturePtr const& texture, uint32_t array_index, uint32_t start_level, uint32_t num_levels, TextureFilter filter) const
	{
		if (!effect_ || IsSRGB(texture->Format()) || (texture->Type() == Texture::TT_3D) || !(texture->AccessHint() & EAH_GPU_Unordered))
		{
			for (uint32_t mip = start_level + 1; mip < start_level + num_levels; ++mip)
			{
				switch (texture->Type())
				{
				case Texture::TT_1D:
				case Texture::TT_2D:
					blitter_->Blit(texture, array_index, mip, 0, 0, texture->Width(mip), texture->Height(mip), texture, array_index, mip - 1, 0,
						0, static_cast<float>(texture->Width(mip - 1)), static_cast<float>(texture->Height(mip - 1)), filter);
					break;

				case Texture::TT_3D:
					blitter_->Blit(texture, array_index, mip, 0, 0, 0, texture->Width(mip), texture->Height(mip), texture->Depth(mip),
						texture, array_index, mip - 1, 0, 0, 0, static_cast<float>(texture->Width(mip - 1)),
						static_cast<float>(texture->Height(mip - 1)), static_cast<float>(texture->Depth(mip - 1)), filter);
					break;

				case Texture::TT_Cube:
					for (uint32_t f = 0; f < 6; ++f)
					{
						auto face = static_cast<Texture::CubeFaces>(f);
						blitter_->Blit(texture, array_index, face, mip, 0, 0, texture->Width(mip), texture->Height(mip), texture,
							array_index, face, mip - 1, 0, 0, static_cast<float>(texture->Width(mip - 1)),
							static_cast<float>(texture->Height(mip - 1)), filter);
					}
					break;
				}
			}
		}
		else
		{
			auto& rf = Context::Instance().RenderFactoryInstance();
			auto& re = rf.RenderEngineInstance();

			re.BindFrameBuffer(re.DefaultFrameBuffer());
			re.DefaultFrameBuffer()->Discard(FrameBuffer::CBM_Color);

			bool const is_array = texture->ArraySize() > 1;
			auto& tech = *mipmap_tech_[is_array ? 1 : 0][static_cast<uint32_t>(filter)];

			auto const& cs_stage = tech.Pass(0).GetShaderObject(*effect_)->Stage(ShaderStage::Compute);
			uint32_t const bx = cs_stage->BlockSizeX();
			uint32_t const by = cs_stage->BlockSizeY();
			BOOST_ASSERT((bx == by) && (bx == (1U << (MAX_LEVELS - 2))));

			while (num_levels > 1)
			{
				uint32_t const num_pass_levels = std::min(MAX_LEVELS, num_levels);
				*src_level_width_height_param_ = uint2(texture->Width(start_level), texture->Height(start_level));
				*num_levels_param_ = num_pass_levels;

				switch (texture->Type())
				{
				case Texture::TT_1D:
				case Texture::TT_2D:
					*src_2d_tex_param_ = rf.MakeTextureSrv(texture, array_index, 1, start_level, 1);
					if (is_array)
					{
						for (uint32_t i = 1; i < num_pass_levels; ++i)
						{
							*output_2d_tex_array_param_[i - 1] = rf.Make2DUav(texture, array_index, 1, start_level + i);
						}
						for (uint32_t i = num_pass_levels; i < MAX_LEVELS; ++i)
						{
							*output_2d_tex_array_param_[i - 1] = UnorderedAccessViewPtr();
						}
					}
					else
					{
						for (uint32_t i = 1; i < num_pass_levels; ++i)
						{
							*output_2d_tex_param_[i - 1] = rf.Make2DUav(texture, array_index, 1, start_level + i);
						}
						for (uint32_t i = num_pass_levels; i < MAX_LEVELS; ++i)
						{
							*output_2d_tex_param_[i - 1] = UnorderedAccessViewPtr();
						}
					}

					re.Dispatch(*effect_, tech, (texture->Width(start_level + 1) + bx - 1) / bx,
						(texture->Height(start_level + 1) + by - 1) / by, 1);
					break;

				case Texture::TT_Cube:
					for (uint32_t f = 0; f < 6; ++f)
					{
						auto face = static_cast<Texture::CubeFaces>(f);

						*src_2d_tex_param_ = rf.MakeTexture2DSrv(texture, array_index, face, start_level, 1);
						for (uint32_t i = 1; i < num_pass_levels; ++i)
						{
							*output_2d_tex_array_param_[i - 1] = rf.Make2DUav(texture, array_index, face, start_level + i);
						}
						for (uint32_t i = num_pass_levels; i < MAX_LEVELS; ++i)
						{
							*output_2d_tex_array_param_[i - 1] = UnorderedAccessViewPtr();
						}

						re.Dispatch(*effect_, tech, (texture->Width(start_level + 1) + bx - 1) / bx,
							(texture->Height(start_level + 1) + by - 1) / by, 1);
					}
					break;

				default:
					KFL_UNREACHABLE("Invalid texture type");
				}

				start_level += (num_pass_levels - 1);
				num_levels -= (num_pass_levels - 1);
			}
		}
	}
} // namespace KlayGE
