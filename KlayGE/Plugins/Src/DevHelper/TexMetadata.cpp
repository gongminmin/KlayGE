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
#include <KFL/CXX17/iterator.hpp>
#include <KFL/ErrorHandling.hpp>
#include <KFL/Hash.hpp>
#include <KlayGE/RenderDeviceCaps.hpp>
#include <KlayGE/ResLoader.hpp>

#include <algorithm>
#include <iterator>
#include <fstream>

#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>

#include <KlayGE/DevHelper/TexMetadata.hpp>

namespace
{
	float GetFloat(rapidjson::Value const & value)
	{
		if (value.IsFloat())
		{
			return value.GetFloat();
		}
		else if (value.IsDouble())
		{
			return static_cast<float>(value.GetDouble());
		}
		else if (value.IsInt())
		{
			return static_cast<float>(value.GetInt());
		}
		else if (value.IsUint())
		{
			return static_cast<float>(value.GetUint());
		}
		else if (value.IsInt64())
		{
			return static_cast<float>(value.GetInt64());
		}
		else if (value.IsUint64())
		{
			return static_cast<float>(value.GetUint64());
		}
		else
		{
			KFL_UNREACHABLE("Invalid value type.");
		}
	}

	int GetInt(rapidjson::Value const & value)
	{
		if (value.IsInt())
		{
			return value.GetInt();
		}
		else if (value.IsUint())
		{
			return static_cast<int>(value.GetUint());
		}
		else if (value.IsInt64())
		{
			return static_cast<int>(value.GetInt64());
		}
		else if (value.IsUint64())
		{
			return static_cast<int>(value.GetUint64());
		}
		else
		{
			KFL_UNREACHABLE("Invalid value type.");
		}
	}
}

namespace KlayGE
{
	TexMetadata::TexMetadata()
	{
	}

	TexMetadata::TexMetadata(std::string_view name)
	{
		this->Load(name, true);
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

		ResIdentifierPtr metadata_file = ResLoader::Instance().Open(name);
		if (metadata_file)
		{
			std::string metadata;
			std::copy(std::istream_iterator<char>(metadata_file->input_stream()), std::istream_iterator<char>(),
				std::back_inserter<std::string>(metadata));

			rapidjson::Document document;
			document.Parse(metadata.data());
			BOOST_ASSERT(!document.HasParseError());

			uint32_t const version = document["version"].GetUint();
			Verify(version == 1);

			if (document.HasMember("type"))
			{
				auto const & type_val = document["type"];
				BOOST_ASSERT(type_val.IsString());
				size_t const type_hash = RT_HASH(type_val.GetString());
				switch (type_hash)
				{
				case CT_HASH("2D"):
					new_metadata.type_ = Texture::TT_2D;
					break;

				// TODO: 3D and Cube?

				default:
					KFL_UNREACHABLE("Invalid texture slot.");
					break;
				}
			}

			if (document.HasMember("slot"))
			{
				auto const & slot_val = document["slot"];
				BOOST_ASSERT(slot_val.IsString());
				size_t const slot_hash = RT_HASH(slot_val.GetString());
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

			if (document.HasMember("prefered_format"))
			{
				auto const & prefered_format_val = document["prefered_format"];
				BOOST_ASSERT(prefered_format_val.IsString());
				size_t const format_hash = RT_HASH(prefered_format_val.GetString());
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

			if (document.HasMember("force_srgb"))
			{
				auto const & force_srgb_val = document["force_srgb"];
				BOOST_ASSERT(force_srgb_val.IsBool());
				new_metadata.force_srgb_ = force_srgb_val.GetBool();
			}
			else if (assign_default_values)
			{
				new_metadata.force_srgb_
					= ((new_metadata.slot_ == RenderMaterial::TS_Albedo) || (new_metadata.slot_ == RenderMaterial::TS_Emissive));
			}

			if (document.HasMember("channel_mapping"))
			{
				auto const & channel_mapping_val = document["channel_mapping"];
				BOOST_ASSERT(channel_mapping_val.IsArray());
				BOOST_ASSERT(channel_mapping_val.Size() == 4);
				uint32_t index = 0;
				for (auto iter = channel_mapping_val.Begin(); (iter != channel_mapping_val.End()) && (index < 4); ++ iter, ++ index)
				{
					BOOST_ASSERT(iter->IsInt() || iter->IsUint() || iter->IsInt64() || iter->IsUint64());
					new_metadata.channel_mapping_[index] = static_cast<int8_t>(GetInt(*iter));
				}
			}

			if (document.HasMember("rgb_to_lum"))
			{
				auto const & rgb_to_lum_val = document["rgb_to_lum"];
				BOOST_ASSERT(rgb_to_lum_val.IsBool());
				new_metadata.rgb_to_lum_ = rgb_to_lum_val.GetBool();
			}
			else if (assign_default_values)
			{
				new_metadata.rgb_to_lum_ = (new_metadata.slot_ == RenderMaterial::TS_Height);
			}

			if (document.HasMember("mipmap"))
			{
				auto const & mipmap_val = document["mipmap"];

				if (mipmap_val.HasMember("enabled"))
				{
					auto const & enabled_val = mipmap_val["enabled"];
					BOOST_ASSERT(enabled_val.IsBool());
					new_metadata.mipmap_.enabled = enabled_val.GetBool();
				}

				if (mipmap_val.HasMember("auto_gen"))
				{
					auto const & auto_gen_val = mipmap_val["auto_gen"];
					BOOST_ASSERT(auto_gen_val.IsBool());
					new_metadata.mipmap_.auto_gen = auto_gen_val.GetBool();
				}

				if (mipmap_val.HasMember("num_levels"))
				{
					auto const & num_levels_val = mipmap_val["num_levels"];
					BOOST_ASSERT(num_levels_val.IsInt() || num_levels_val.IsUint()
						|| num_levels_val.IsInt64() || num_levels_val.IsUint64());
					new_metadata.mipmap_.num_levels = GetInt(num_levels_val);
				}

				if (mipmap_val.HasMember("linear"))
				{
					auto const & linear_val = mipmap_val["linear"];
					BOOST_ASSERT(linear_val.IsBool());
					new_metadata.mipmap_.linear = linear_val.GetBool();
				}
			}
			else if (assign_default_values)
			{
				new_metadata.mipmap_.enabled = true;
				new_metadata.mipmap_.auto_gen = true;
			}

			if (((new_metadata.slot_ == RenderMaterial::TS_Normal) || (new_metadata.slot_ == RenderMaterial::TS_Height) ||
					(new_metadata.slot_ == RenderMaterial::TS_Occlusion)) &&
				document.HasMember("bump"))
			{
				auto const & bump_val = document["bump"];

				if (bump_val.HasMember("to_normal"))
				{
					auto const & to_normal_val = bump_val["to_normal"];
					BOOST_ASSERT(to_normal_val.IsBool());
					new_metadata.bump_.to_normal = to_normal_val.GetBool();
				}
				if (bump_val.HasMember("scale"))
				{
					auto const & scale_val = bump_val["scale"];
					BOOST_ASSERT(scale_val.IsNumber());
					new_metadata.bump_.scale = GetFloat(scale_val);
				}
				if (bump_val.HasMember("to_occlusion"))
				{
					auto const& to_occlusion_val = bump_val["to_occlusion"];
					BOOST_ASSERT(to_occlusion_val.IsBool());
					new_metadata.bump_.to_occlusion = to_occlusion_val.GetBool();
				}
				if (bump_val.HasMember("occlusion_amplitude"))
				{
					auto const& occlusion_amplitude_val = bump_val["occlusion_amplitude"];
					BOOST_ASSERT(occlusion_amplitude_val.IsNumber());
					new_metadata.bump_.occlusion_amplitude = GetFloat(occlusion_amplitude_val);
				}

				if (bump_val.HasMember("from_normal"))
				{
					auto const & from_normal_val = bump_val["from_normal"];
					BOOST_ASSERT(from_normal_val.IsBool());
					new_metadata.bump_.from_normal = from_normal_val.GetBool();
				}
				if (bump_val.HasMember("min_z"))
				{
					auto const & min_z_val = bump_val["min_z"];
					BOOST_ASSERT(min_z_val.IsNumber());
					new_metadata.bump_.min_z = GetFloat(min_z_val);
				}
			}

			if (document.HasMember("array"))
			{
				auto const & array_val = document["array"];
				BOOST_ASSERT(array_val.IsArray());

				for (auto array_iter = array_val.Begin(); array_iter != array_val.End(); ++ array_iter)
				{
					if (array_iter->IsArray())
					{
						std::vector<std::string> mip_names;
						for (auto mip_iter = array_iter->Begin(); mip_iter != array_iter->End(); ++ mip_iter)
						{
							BOOST_ASSERT(mip_iter->IsString());

							mip_names.push_back(mip_iter->GetString());
						}

						new_metadata.plane_file_names_.push_back(std::move(mip_names));
					}
					else
					{
						BOOST_ASSERT(array_iter->IsString());

						new_metadata.plane_file_names_.push_back(std::vector<std::string>(1, std::string(array_iter->GetString())));
					}
				}
			}
			else
			{
				new_metadata.plane_file_names_.assign(1, std::vector<std::string>(1, std::string(name)));
			}
		}
		else if (!name.empty())
		{
			LogInfo() << "Could NOT find " << name << ". Fallback to default metadata." << std::endl;
		}

		*this = std::move(new_metadata);
	}

	void TexMetadata::Save(std::string const & name) const
	{
		rapidjson::Document document;
		document.SetObject();

		auto& allocator = document.GetAllocator();

		document.AddMember("version", 1U, allocator);

		document.AddMember("type", "2D", allocator);

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
		document.AddMember("slot", rapidjson::StringRef(slot_str.c_str(), static_cast<rapidjson::SizeType>(slot_str.size())), allocator);

		std::string preferred_format_str;
		if (prefered_format_ != EF_Unknown)
		{
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
			document.AddMember("prefered_format",
				rapidjson::StringRef(preferred_format_str.c_str(), static_cast<rapidjson::SizeType>(preferred_format_str.size())),
				allocator);
		}

		if (force_srgb_)
		{
			document.AddMember("force_srgb", force_srgb_, allocator);
		}

		bool need_swizzle = false;
		for (size_t i = 0; i < std::size(channel_mapping_); ++ i)
		{
			if (channel_mapping_[i] != static_cast<int8_t>(i))
			{
				need_swizzle = true;
				break;
			}
		}
		if (need_swizzle)
		{
			rapidjson::Value channel_mapping_val;
			channel_mapping_val.SetArray();
			for (size_t i = 0; i < std::size(channel_mapping_); ++ i)
			{
				channel_mapping_val.PushBack(static_cast<int>(channel_mapping_[i]), allocator);
			}
			document.AddMember("channel_mapping", channel_mapping_val, allocator);
		}

		if (rgb_to_lum_)
		{
			document.AddMember("rgb_to_lum", rgb_to_lum_, allocator);
		}

		if (mipmap_.enabled)
		{
			rapidjson::Value mipmap_val;
			mipmap_val.SetObject();

			mipmap_val.AddMember("enabled", mipmap_.enabled, allocator);
			mipmap_val.AddMember("auto_gen", mipmap_.auto_gen, allocator);
			mipmap_val.AddMember("num_levels", mipmap_.num_levels, allocator);
			mipmap_val.AddMember("linear", mipmap_.linear, allocator);

			document.AddMember("mipmap", mipmap_val, allocator);
		}

		if (((slot_ == RenderMaterial::TS_Normal) || (slot_ == RenderMaterial::TS_Height) || (slot_ == RenderMaterial::TS_Occlusion)) &&
			(bump_.to_normal || bump_.from_normal || bump_.to_occlusion))
		{
			rapidjson::Value bump_val;
			bump_val.SetObject();

			if ((slot_ == RenderMaterial::TS_Normal) && bump_.to_normal)
			{
				bump_val.AddMember("to_normal", bump_.to_normal, allocator);
				bump_val.AddMember("scale", bump_.scale, allocator);
			}
			if ((slot_ == RenderMaterial::TS_Occlusion) && bump_.to_occlusion)
			{
				bump_val.AddMember("to_occlusion", bump_.to_normal, allocator);
				bump_val.AddMember("occlusion_amplitude", bump_.occlusion_amplitude, allocator);
			}
			if ((slot_ == RenderMaterial::TS_Height) && bump_.from_normal)
			{
				bump_val.AddMember("from_normal", bump_.from_normal, allocator);
				bump_val.AddMember("min_z", bump_.min_z, allocator);
			}

			document.AddMember("bump", bump_val, allocator);
		}

		if ((plane_file_names_.size() > 1) || ((plane_file_names_.size() == 1) && (plane_file_names_[0].size() > 1)))
		{
			rapidjson::Value array_names_val;
			array_names_val.SetArray();

			for (size_t i = 0; i < plane_file_names_.size(); ++ i)
			{
				if (plane_file_names_[i].size() > 1)
				{
					rapidjson::Value mip_names_val;
					mip_names_val.SetArray();
					for (size_t j = 0; j < plane_file_names_[i].size(); ++ j)
					{
						auto const & str = plane_file_names_[i][j];
						mip_names_val.PushBack(rapidjson::StringRef(str.c_str(), str.size()), allocator);
					}
					array_names_val.PushBack(mip_names_val, allocator);
				}
				else
				{
					auto const & str = plane_file_names_[i][0];
					array_names_val.PushBack(rapidjson::StringRef(str.c_str(), str.size()), allocator);
				}
			}

			document.AddMember("array", array_names_val, allocator);
		}

		rapidjson::StringBuffer sb;
		rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(sb);
		document.Accept(writer);
		std::ofstream ofs(name);
		ofs << sb.GetString();
	}

	void TexMetadata::DeviceDependentAdjustment(RenderDeviceCaps const & caps)
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
		return plane_file_names_[array_index][mip];
	}

	void TexMetadata::PlaneFileName(uint32_t array_index, uint32_t mip, std::string_view name)
	{
		plane_file_names_[array_index][mip] = std::string(name);
	}
}
