/**
 * @file TexMetadata.cpp
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
#include <KFL/Hash.hpp>
#include <KFL/JsonDom.hpp>
#include <KFL/Log.hpp>
#include <KlayGE/RenderDeviceCaps.hpp>
#include <KlayGE/ResLoader.hpp>

#include <algorithm>
#include <fstream>
#include <iterator>

#include "MetadataUtil.hpp"

#include <KlayGE/DevHelper/TexMetadata.hpp>

namespace KlayGE
{
	TexMetadata::TexMetadata() = default;

	TexMetadata::TexMetadata(std::string_view name) : TexMetadata(name, true)
	{
	}

	TexMetadata::TexMetadata(std::string_view name, bool assign_default_values)
	{
		this->Load(name, assign_default_values);
	}

	void TexMetadata::Load(std::string_view name)
	{
		this->Load(name, true);
	}

	void TexMetadata::Load(std::string_view name, bool assign_default_values)
	{
		TexMetadata new_metadata;

		ResIdentifierPtr metadata_file = Context::Instance().ResLoaderInstance().Open(name);
		if (metadata_file)
		{
			const auto root_value = LoadJson(*metadata_file);

			uint32_t const version = GetInt(*root_value.Member("version"));
			Verify(version == 1);

			if (auto const* type_val = root_value.Member("type"))
			{
				size_t const type_hash = HashValue(type_val->ValueString());
				switch (type_hash)
				{
				case CT_HASH("2D"):
					new_metadata.type_ = Texture::TT_2D;
					break;

					// TODO: 3D and Cube?

				default:
					KFL_UNREACHABLE("Invalid texture slot.");
				}
			}

			if (auto const* slot_val = root_value.Member("slot"))
			{
				size_t const slot_hash = HashValue(slot_val->ValueString());
				switch (slot_hash)
				{
				case CT_HASH("albedo"):
					new_metadata.slot_ = RenderMaterial::TS_Albedo;
					break;

				case CT_HASH("metalness_glossiness"):
					new_metadata.slot_ = RenderMaterial::TS_MetalnessGlossiness;
					break;

				case CT_HASH("emissive"):
					new_metadata.slot_ = RenderMaterial::TS_Emissive;
					break;

				case CT_HASH("normal"):
					new_metadata.slot_ = RenderMaterial::TS_Normal;
					break;

				case CT_HASH("height"):
					new_metadata.slot_ = RenderMaterial::TS_Height;
					break;

				case CT_HASH("occlusion"):
					new_metadata.slot_ = RenderMaterial::TS_Occlusion;
					break;

				default:
					KFL_UNREACHABLE("Invalid texture slot.");
				}
			}

			if (auto const* prefered_format_val = root_value.Member("prefered_format"))
			{
				size_t const format_hash = HashValue(prefered_format_val->ValueString());
				switch (format_hash)
				{
				case CT_HASH("BC1"):
					new_metadata.prefered_format_ = EF_BC1;
					break;
				case CT_HASH("BC1_SRGB"):
					new_metadata.prefered_format_ = EF_BC1_SRGB;
					break;
				case CT_HASH("BC2"):
					new_metadata.prefered_format_ = EF_BC2;
					break;
				case CT_HASH("BC2_SRGB"):
					new_metadata.prefered_format_ = EF_BC2_SRGB;
					break;
				case CT_HASH("BC3"):
					new_metadata.prefered_format_ = EF_BC3;
					break;
				case CT_HASH("BC3_SRGB"):
					new_metadata.prefered_format_ = EF_BC3_SRGB;
					break;
				case CT_HASH("BC4"):
					new_metadata.prefered_format_ = EF_BC4;
					break;
				case CT_HASH("BC4_SRGB"):
					new_metadata.prefered_format_ = EF_BC4_SRGB;
					break;
				case CT_HASH("BC5"):
					new_metadata.prefered_format_ = EF_BC5;
					break;
				case CT_HASH("BC5_SRGB"):
					new_metadata.prefered_format_ = EF_BC5_SRGB;
					break;
				case CT_HASH("BC6"):
					new_metadata.prefered_format_ = EF_BC6;
					break;
				case CT_HASH("BC7"):
					new_metadata.prefered_format_ = EF_BC7;
					break;
				case CT_HASH("BC7_SRGB"):
					new_metadata.prefered_format_ = EF_BC7_SRGB;
					break;
				case CT_HASH("ETC1"):
					new_metadata.prefered_format_ = EF_ETC1;
					break;

				case CT_HASH("GR8"):
					new_metadata.prefered_format_ = EF_GR8;
					break;

				default:
					new_metadata.prefered_format_ = EF_Unknown;
					break;
				}
			}

			if (auto const* force_srgb_val = root_value.Member("force_srgb"))
			{
				new_metadata.force_srgb_ = force_srgb_val->ValueBool();
			}
			else if (assign_default_values)
			{
				new_metadata.force_srgb_ =
					((new_metadata.slot_ == RenderMaterial::TS_Albedo) || (new_metadata.slot_ == RenderMaterial::TS_Emissive));
			}

			if (auto const* channel_mapping_val = root_value.Member("channel_mapping"))
			{
				auto const& values = channel_mapping_val->ValueArray();
				BOOST_ASSERT(values.size() == 4);
				uint32_t index = 0;
				for (auto iter = values.begin(); (iter != values.end()) && (index < 4); ++iter, ++index)
				{
					new_metadata.channel_mapping_[index] = static_cast<int8_t>(GetInt(*iter));
				}
			}

			if (auto const* rgb_to_lum_val = root_value.Member("rgb_to_lum"))
			{
				new_metadata.rgb_to_lum_ = rgb_to_lum_val->ValueBool();
			}
			else if (assign_default_values)
			{
				new_metadata.rgb_to_lum_ = (new_metadata.slot_ == RenderMaterial::TS_Height);
			}

			if (auto const* mipmap_val = root_value.Member("mipmap"))
			{
				if (auto const* enabled_val = mipmap_val->Member("enabled"))
				{
					new_metadata.mipmap_.enabled = enabled_val->ValueBool();
				}
				if (auto const* auto_gen_val = mipmap_val->Member("auto_gen"))
				{
					new_metadata.mipmap_.auto_gen = auto_gen_val->ValueBool();
				}
				if (auto const* num_levels_val = mipmap_val->Member("num_levels"))
				{
					new_metadata.mipmap_.num_levels = GetInt(*num_levels_val);
				}
				if (auto const* linear_val = mipmap_val->Member("linear"))
				{
					new_metadata.mipmap_.linear = linear_val->ValueBool();
				}
			}
			else if (assign_default_values)
			{
				new_metadata.mipmap_.enabled = true;
				new_metadata.mipmap_.auto_gen = true;
			}

			{
				auto const* bump_val = root_value.Member("bump");
				if (((new_metadata.slot_ == RenderMaterial::TS_Normal) || (new_metadata.slot_ == RenderMaterial::TS_Height) ||
						(new_metadata.slot_ == RenderMaterial::TS_Occlusion)) &&
					(bump_val != nullptr))
				{
					if (auto const* to_normal_val = bump_val->Member("to_normal"))
					{
						new_metadata.bump_.to_normal = to_normal_val->ValueBool();
					}
					if (auto const* scale_val = bump_val->Member("scale"))
					{
						new_metadata.bump_.scale = GetFloat(*scale_val);
					}
					if (auto const* to_occlusion_val = bump_val->Member("to_occlusion"))
					{
						new_metadata.bump_.to_occlusion = to_occlusion_val->ValueBool();
					}
					if (auto const* occlusion_amplitude_val = bump_val->Member("occlusion_amplitude"))
					{
						new_metadata.bump_.occlusion_amplitude = GetFloat(*occlusion_amplitude_val);
					}

					if (auto const* from_normal_val = bump_val->Member("from_normal"))
					{
						new_metadata.bump_.from_normal = from_normal_val->ValueBool();
					}
					if (auto const* min_z_val = bump_val->Member("min_z"))
					{
						new_metadata.bump_.min_z = GetFloat(*min_z_val);
					}
				}
			}

			auto load_sources = [](JsonValue const& value) {
				std::vector<std::string> ret;
				if (value.Type() == JsonValueType::Array)
				{
					auto const& array_values = value.ValueArray();
					for (auto iter = array_values.begin(); iter != array_values.end(); ++iter)
					{
						ret.emplace_back(iter->ValueString());
					}
				}
				else
				{
					ret.emplace_back(value.ValueString());
				}

				return ret;
			};

			if (auto const* source_val = root_value.Member("source"))
			{
				if (source_val->Type() == JsonValueType::String)
				{
					new_metadata.plane_file_names_.push_back(load_sources(*source_val));
				}
				else
				{
					if (auto const* mips_val = source_val->Member("mips"))
					{
						new_metadata.plane_file_names_.push_back(load_sources(*mips_val));
					}
					else if (auto const* array_val = source_val->Member("array"))
					{
						auto const& array_values = array_val->ValueArray();
						for (auto array_iter = array_values.begin(); array_iter != array_values.end(); ++array_iter)
						{
							if (array_iter->Type() == JsonValueType::String)
							{
								new_metadata.plane_file_names_.push_back(load_sources(*array_iter));
							}
							else
							{
								auto const* array_mips_val = array_iter->Member("mips");
								BOOST_ASSERT(array_mips_val != nullptr);
								new_metadata.plane_file_names_.push_back(load_sources(*array_mips_val));
							}
						}
					}
				}
			}
		}
		else if (!name.empty())
		{
			LogInfo() << "Could NOT find " << name << ". Fallback to default metadata." << std::endl;
		}

		if (new_metadata.plane_file_names_.empty())
		{
			std::string_view implicit_source_name = name.substr(0, name.rfind(".kmeta"));
			new_metadata.plane_file_names_.assign(1, std::vector<std::string>(1, std::string(std::move(implicit_source_name))));
		}

		*this = std::move(new_metadata);
	}

	void TexMetadata::Save(std::string const& name) const
	{
		JsonValue root_value(JsonValueType::Object);

		root_value.AppendValue("version", JsonValue(1U));

		root_value.AppendValue("type", JsonValue(std::string_view("2D")));

		{
			std::string slot_str;
			switch (slot_)
			{
			case RenderMaterial::TS_Albedo:
				slot_str = "albedo";
				break;

			case RenderMaterial::TS_MetalnessGlossiness:
				slot_str = "metalness_glossiness";
				break;

			case RenderMaterial::TS_Emissive:
				slot_str = "emissive";
				break;

			case RenderMaterial::TS_Normal:
				slot_str = "normal";
				break;

			case RenderMaterial::TS_Height:
				slot_str = "height";
				break;

			default:
				KFL_UNREACHABLE("Invalid texture slot.");
			}
			root_value.AppendValue("slot", JsonValue(std::move(slot_str)));
		}

		if (prefered_format_ != EF_Unknown)
		{
			std::string preferred_format_str;
			switch (prefered_format_)
			{
			case EF_BC1:
				preferred_format_str = "BC1";
				break;
			case EF_BC1_SRGB:
				preferred_format_str = "BC1_SRGB";
				break;
			case EF_BC2:
				preferred_format_str = "BC2";
				break;
			case EF_BC2_SRGB:
				preferred_format_str = "BC2_SRGB";
				break;
			case EF_BC3:
				preferred_format_str = "BC3";
				break;
			case EF_BC3_SRGB:
				preferred_format_str = "BC3_SRGB";
				break;
			case EF_BC4:
				preferred_format_str = "BC4";
				break;
			case EF_BC4_SRGB:
				preferred_format_str = "BC4_SRGB";
				break;
			case EF_BC5:
				preferred_format_str = "BC5";
				break;
			case EF_BC5_SRGB:
				preferred_format_str = "BC5_SRGB";
				break;
			case EF_BC6:
				preferred_format_str = "BC6";
				break;
			case EF_BC7:
				preferred_format_str = "BC7";
				break;
			case EF_BC7_SRGB:
				preferred_format_str = "BC7_SRGB";
				break;
			case EF_ETC1:
				preferred_format_str = "ETC1";
				break;

			case EF_GR8:
				preferred_format_str = "GR8";
				break;

			default:
				preferred_format_str = "Unknown";
				break;
			}
			root_value.AppendValue("prefered_format", JsonValue(std::move(preferred_format_str)));
		}

		if (force_srgb_)
		{
			root_value.AppendValue("force_srgb", JsonValue(force_srgb_));
		}

		bool need_swizzle = false;
		for (size_t i = 0; i < std::size(channel_mapping_); ++i)
		{
			if (channel_mapping_[i] != static_cast<int8_t>(i))
			{
				need_swizzle = true;
				break;
			}
		}
		if (need_swizzle)
		{
			JsonValue channel_mapping_val(JsonValueType::Array);
			for (auto item : channel_mapping_)
			{
				channel_mapping_val.AppendValue(JsonValue(static_cast<int32_t>(item)));
			}
			root_value.AppendValue("channel_mapping", std::move(channel_mapping_val));
		}

		if (rgb_to_lum_)
		{
			root_value.AppendValue("rgb_to_lum", JsonValue(rgb_to_lum_));
		}

		if (mipmap_.enabled)
		{
			JsonValue mipmap_val(JsonValueType::Object);

			mipmap_val.AppendValue("enabled", JsonValue(mipmap_.enabled));
			mipmap_val.AppendValue("auto_gen", JsonValue(mipmap_.auto_gen));
			mipmap_val.AppendValue("num_levels", JsonValue(mipmap_.num_levels));
			mipmap_val.AppendValue("linear", JsonValue(mipmap_.linear));

			root_value.AppendValue("mipmap", std::move(mipmap_val));
		}

		if (((slot_ == RenderMaterial::TS_Normal) || (slot_ == RenderMaterial::TS_Height) || (slot_ == RenderMaterial::TS_Occlusion)) &&
			(bump_.to_normal || bump_.from_normal || bump_.to_occlusion))
		{
			JsonValue bump_val(JsonValueType::Object);

			if ((slot_ == RenderMaterial::TS_Normal) && bump_.to_normal)
			{
				bump_val.AppendValue("to_normal", JsonValue(bump_.to_normal));
				bump_val.AppendValue("scale", JsonValue(bump_.scale));
			}
			if ((slot_ == RenderMaterial::TS_Occlusion) && bump_.to_occlusion)
			{
				bump_val.AppendValue("to_occlusion", JsonValue(bump_.to_normal));
				bump_val.AppendValue("occlusion_amplitude", JsonValue(bump_.occlusion_amplitude));
			}
			if ((slot_ == RenderMaterial::TS_Height) && bump_.from_normal)
			{
				bump_val.AppendValue("from_normal", JsonValue(bump_.from_normal));
				bump_val.AppendValue("min_z", JsonValue(bump_.min_z));
			}

			root_value.AppendValue("bump", std::move(bump_val));
		}

		auto store_mips = [this](size_t index) {
			JsonValue mip_names_val(JsonValueType::Array);
			for (auto const& plane_file : plane_file_names_[index])
			{
				mip_names_val.AppendValue(JsonValue(plane_file));
			}

			JsonValue ret(JsonValueType::Object);
			ret.AppendValue("mips", std::move(mip_names_val));
			return ret;
		};

		JsonValue source_val;
		if (plane_file_names_.size() == 1)
		{
			if (plane_file_names_[0].size() == 1)
			{
				source_val = JsonValue(plane_file_names_[0][0]);
			}
			else
			{
				source_val = store_mips(0);
			}
		}
		else
		{
			auto array_names_val = JsonValue(JsonValueType::Array);

			for (size_t i = 0; i < plane_file_names_.size(); ++i)
			{
				if (plane_file_names_[i].size() > 1)
				{
					array_names_val.AppendValue(store_mips(i));
				}
				else
				{
					array_names_val.AppendValue(JsonValue(plane_file_names_[i][0]));
				}
			}

			source_val = JsonValue(JsonValueType::Object);
			source_val.AppendValue("array", std::move(array_names_val));
		}
		root_value.AppendValue("source", std::move(source_val));

		std::ofstream ofs(name);
		SaveJson(root_value, ofs);
	}

	void TexMetadata::DeviceDependentAdjustment(RenderDeviceCaps const& caps)
	{
		if (prefered_format_ == EF_Unknown)
		{
			switch (slot_)
			{
			case RenderMaterial::TS_Albedo:
			case RenderMaterial::TS_Emissive:
				prefered_format_ = caps.BestMatchTextureFormat(MakeSpan({EF_BC7_SRGB, EF_BC1_SRGB, EF_ETC1}));
				break;

			case RenderMaterial::TS_MetalnessGlossiness:
			case RenderMaterial::TS_Normal:
				prefered_format_ = caps.BestMatchTextureFormat(MakeSpan({EF_BC5, EF_BC3, EF_GR8}));
				break;

			case RenderMaterial::TS_Height:
				prefered_format_ = caps.BestMatchTextureFormat(MakeSpan({EF_BC4, EF_BC1, EF_ETC1}));
				break;

			default:
				KFL_UNREACHABLE("Invalid texture slot");
			}
		}
	}

	uint32_t TexMetadata::ArraySize() const
	{
		return plane_file_names_.empty() ? 1 : static_cast<uint32_t>(plane_file_names_.size());
	}

	void TexMetadata::ArraySize(uint32_t size)
	{
		plane_file_names_.resize(size);
	}

	std::string_view TexMetadata::PlaneFileName(uint32_t array_index, uint32_t mip) const
	{
		if ((plane_file_names_.size() > array_index) && (plane_file_names_[array_index].size() > mip))
		{
			return plane_file_names_[array_index][mip];
		}
		else
		{
			return "";
		}
	}

	void TexMetadata::PlaneFileName(uint32_t array_index, uint32_t mip, std::string_view name)
	{
		plane_file_names_[array_index][mip] = std::string(std::move(name));
	}
} // namespace KlayGE
