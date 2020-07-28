/**
 * @file PlatformDefinition.hpp
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
#include <KFL/XMLDom.hpp>
#include <KlayGE/ResLoader.hpp>

#include <KlayGE/DevHelper/PlatformDefinition.hpp>

namespace
{
	using namespace KlayGE;

	int RetrieveAttrValue(XMLNodePtr node, std::string const & attr_name, int default_value)
	{
		XMLAttributePtr attr = node->Attrib(attr_name);
		if (attr)
		{
			return attr->ValueInt();
		}

		return default_value;
	}

	std::string RetrieveAttrValue(XMLNodePtr node, std::string const & attr_name, std::string const & default_value)
	{
		XMLAttributePtr attr = node->Attrib(attr_name);
		if (attr)
		{
			return std::string(attr->ValueString());
		}

		return default_value;
	}

	int RetrieveNodeValue(XMLNodePtr root, std::string const & node_name, int default_value)
	{
		XMLNodePtr node = root->FirstNode(node_name);
		if (node)
		{
			return RetrieveAttrValue(node, "value", default_value);
		}

		return default_value;
	}

	std::string RetrieveNodeValue(XMLNodePtr root, std::string const & node_name, std::string const & default_value)
	{
		XMLNodePtr node = root->FirstNode(node_name);
		if (node)
		{
			return RetrieveAttrValue(node, "value", default_value);
		}

		return default_value;
	}
}

namespace KlayGE
{
	PlatformDefinition::PlatformDefinition()
	{
	}

	PlatformDefinition::PlatformDefinition(std::string_view name)
	{
		ResIdentifierPtr plat = ResLoader::Instance().Open(name);

		KlayGE::XMLDocument doc;
		XMLNodePtr root = doc.Parse(*plat);

		platform = RetrieveAttrValue(root, "name", "");
		major_version = static_cast<uint8_t>(RetrieveAttrValue(root, "major_version", 0));
		minor_version = static_cast<uint8_t>(RetrieveAttrValue(root, "minor_version", 0));

		requires_flipping = RetrieveNodeValue(root, "requires_flipping", 0) ? true : false;
		std::string const fourcc_str = RetrieveNodeValue(root, "native_shader_fourcc", "");
		native_shader_fourcc
			= (fourcc_str[0] << 0) + (fourcc_str[1] << 8) + (fourcc_str[2] << 16) + (fourcc_str[3] << 24);
		native_shader_version = RetrieveNodeValue(root, "native_shader_version", 0);

		bool const srgb_support = RetrieveNodeValue(root, "srgb_support", 0) ? true : false;

		std::vector<ElementFormat> texture_formats =
		{
			EF_R8,
			EF_ABGR8,
			EF_ARGB8,
		};
		if (RetrieveNodeValue(root, "bc1_support", 0))
		{
			texture_formats.push_back(EF_BC1);
			texture_formats.push_back(EF_SIGNED_BC1);
			if (srgb_support)
			{
				texture_formats.push_back(EF_BC1_SRGB);
			}
		}
		if (RetrieveNodeValue(root, "bc2_support", 0))
		{
			texture_formats.push_back(EF_BC2);
			texture_formats.push_back(EF_SIGNED_BC2);
			if (srgb_support)
			{
				texture_formats.push_back(EF_BC2_SRGB);
			}
		}
		if (RetrieveNodeValue(root, "bc3_support", 0))
		{
			texture_formats.push_back(EF_BC3);
			texture_formats.push_back(EF_SIGNED_BC3);
			if (srgb_support)
			{
				texture_formats.push_back(EF_BC3_SRGB);
			}
		}
		if (RetrieveNodeValue(root, "bc4_support", 0))
		{
			texture_formats.push_back(EF_BC4);
			texture_formats.push_back(EF_SIGNED_BC4);
			if (srgb_support)
			{
				texture_formats.push_back(EF_BC4_SRGB);
			}
		}
		if (RetrieveNodeValue(root, "bc5_support", 0))
		{
			texture_formats.push_back(EF_BC5);
			texture_formats.push_back(EF_SIGNED_BC5);
			if (srgb_support)
			{
				texture_formats.push_back(EF_BC5_SRGB);
			}
		}
		if (RetrieveNodeValue(root, "bc6_support", 0))
		{
			texture_formats.push_back(EF_BC6);
			texture_formats.push_back(EF_SIGNED_BC6);
		}
		if (RetrieveNodeValue(root, "bc7_support", 0))
		{
			texture_formats.push_back(EF_BC7);
			if (srgb_support)
			{
				texture_formats.push_back(EF_BC7_SRGB);
			}
		}
		if (RetrieveNodeValue(root, "etc1_support", 0))
		{
			texture_formats.push_back(EF_ETC1);
		}
		if (RetrieveNodeValue(root, "r16_support", 0))
		{
			texture_formats.push_back(EF_R16);
			texture_formats.push_back(EF_SIGNED_R16);
		}
		if (RetrieveNodeValue(root, "r16f_support", 0))
		{
			texture_formats.push_back(EF_R16F);
		}

		std::vector<ElementFormat> uav_formats;
		if (device_caps.max_shader_model >= ShaderModel(5, 1))
		{
			uav_formats.insert(uav_formats.end(),
				{
					EF_ABGR16F,
					EF_B10G11R11F,
					EF_ABGR8,
					EF_R16UI,
					EF_R32UI,
					EF_R32F
				});
		}

		device_caps.AssignTextureFormats(std::move(texture_formats));
		device_caps.AssignUavFormats(std::move(uav_formats));

		XMLNodePtr max_shader_model_node = root->FirstNode("max_shader_model");
		device_caps.max_shader_model = ShaderModel(
			static_cast<uint8_t>(RetrieveAttrValue(max_shader_model_node, "major", 0)),
			static_cast<uint8_t>(RetrieveAttrValue(max_shader_model_node, "minor", 0)));

		device_caps.max_texture_depth = RetrieveNodeValue(root, "max_texture_depth", 0);
		device_caps.max_texture_array_length = RetrieveNodeValue(root, "max_texture_array_length", 0);
		device_caps.max_pixel_texture_units = static_cast<uint8_t>(RetrieveNodeValue(root, "max_pixel_texture_units", 0));
		device_caps.max_simultaneous_rts = static_cast<uint8_t>(RetrieveNodeValue(root, "max_simultaneous_rts", 0));

		device_caps.fp_color_support = RetrieveNodeValue(root, "fp_color_support", 0) ? true : false;
		device_caps.pack_to_rgba_required = RetrieveNodeValue(root, "pack_to_rgba_required", 0) ? true : false;
		device_caps.render_to_texture_array_support
			= RetrieveNodeValue(root, "render_to_texture_array_support", 0) ? true : false;
		device_caps.uavs_at_every_stage_support = RetrieveNodeValue(root, "uavs_at_every_stage_support", 0) ? true : false;
		device_caps.explicit_multi_sample_support = RetrieveNodeValue(root, "explicit_multi_sample_support", 0) ? true : false;
		device_caps.vp_rt_index_at_every_stage_support = RetrieveNodeValue(root, "vp_rt_index_at_every_stage_support", 0) ? true : false;

		device_caps.gs_support = RetrieveNodeValue(root, "gs_support", 0) ? true : false;
		device_caps.cs_support = RetrieveNodeValue(root, "cs_support", 0) ? true : false;
		device_caps.hs_support = RetrieveNodeValue(root, "hs_support", 0) ? true : false;
		device_caps.ds_support = RetrieveNodeValue(root, "ds_support", 0) ? true : false;

		frag_depth_support = RetrieveNodeValue(root, "frag_depth_support", 0) ? true : false;
	}
}
