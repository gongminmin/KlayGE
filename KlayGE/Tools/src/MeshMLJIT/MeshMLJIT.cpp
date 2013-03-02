#include <KlayGE/KlayGE.hpp>
#include <KFL/Math.hpp>
#include <KFL/Util.hpp>
#include <KFL/XMLDom.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/RenderLayout.hpp>
#include <KlayGE/LZMACodec.hpp>
#include <KlayGE/Renderable.hpp>
#include <KlayGE/Mesh.hpp>

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

using namespace std;
using namespace KlayGE;

namespace
{
	std::string const JIT_EXT_NAME = ".model_bin";
	uint32_t const MODEL_BIN_VERSION = 8;

	struct KeyFrames
	{
		std::vector<uint32_t> frame_id;
		std::vector<Quaternion> bind_real;
		std::vector<Quaternion> bind_dual;
		std::vector<float> bind_scale;
	};

	struct AABBKeyFrames
	{
		std::vector<uint32_t> frame_id;
		std::vector<AABBox> bb;
	};

	void CompileMaterialsChunk(XMLNodePtr const & materials_chunk, std::vector<RenderMaterial>& mtls)
	{
		uint32_t mtl_index = 0;
		for (XMLNodePtr mtl_node = materials_chunk->FirstNode("material"); mtl_node; mtl_node = mtl_node->NextSibling("material"), ++ mtl_index)
		{
			RenderMaterial mtl;

			XMLAttributePtr attr = mtl_node->Attrib("ambient");
			if (attr)
			{
				std::istringstream attr_ss(attr->ValueString());
				attr_ss >> mtl.ambient.x() >> mtl.ambient.y() >> mtl.ambient.z();
			}
			else
			{
				mtl.ambient.x() = mtl_node->Attrib("ambient_r")->ValueFloat();
				mtl.ambient.y() = mtl_node->Attrib("ambient_g")->ValueFloat();
				mtl.ambient.z() = mtl_node->Attrib("ambient_b")->ValueFloat();
			}
			attr = mtl_node->Attrib("diffuse");
			if (attr)
			{
				std::istringstream attr_ss(attr->ValueString());
				attr_ss >> mtl.diffuse.x() >> mtl.diffuse.y() >> mtl.diffuse.z();
			}
			else
			{
				mtl.diffuse.x() = mtl_node->Attrib("diffuse_r")->ValueFloat();
				mtl.diffuse.y() = mtl_node->Attrib("diffuse_g")->ValueFloat();
				mtl.diffuse.z() = mtl_node->Attrib("diffuse_b")->ValueFloat();
			}
			attr = mtl_node->Attrib("specular");
			if (attr)
			{
				std::istringstream attr_ss(attr->ValueString());
				attr_ss >> mtl.specular.x() >> mtl.specular.y() >> mtl.specular.z();
			}
			else
			{
				mtl.specular.x() = mtl_node->Attrib("specular_r")->ValueFloat();
				mtl.specular.y() = mtl_node->Attrib("specular_g")->ValueFloat();
				mtl.specular.z() = mtl_node->Attrib("specular_b")->ValueFloat();
			}
			attr = mtl_node->Attrib("emit");
			if (attr)
			{
				std::istringstream attr_ss(attr->ValueString());
				attr_ss >> mtl.emit.x() >> mtl.emit.y() >> mtl.emit.z();
			}
			else
			{
				mtl.emit.x() = mtl_node->Attrib("emit_r")->ValueFloat();
				mtl.emit.y() = mtl_node->Attrib("emit_g")->ValueFloat();
				mtl.emit.z() = mtl_node->Attrib("emit_b")->ValueFloat();
			}
			mtl.opacity = mtl_node->Attrib("opacity")->ValueFloat();
			mtl.specular_level = mtl_node->Attrib("specular_level")->ValueFloat();
			mtl.shininess = mtl_node->Attrib("shininess")->ValueFloat();

			XMLNodePtr tex_node = mtl_node->FirstNode("texture");
			if (!tex_node)
			{
				XMLNodePtr textures_chunk = mtl_node->FirstNode("textures_chunk");
				if (textures_chunk)
				{
					tex_node = textures_chunk->FirstNode("texture");
				}
			}
			if (tex_node)
			{
				for (; tex_node; tex_node = tex_node->NextSibling("texture"))
				{
					mtl.texture_slots.push_back(std::make_pair(tex_node->Attrib("type")->ValueString(), 
						tex_node->Attrib("name")->ValueString()));
				}
			}

			mtls.push_back(mtl);
		}
	}

	void CompileMeshesVerticesChunk(XMLNodePtr const & vertices_chunk, bool normal_in_8_bit,
		AABBox& pos_bb, AABBox& tc_bb, std::vector<vertex_element>& vertex_elements,
		std::vector<int16_t>& positions, std::vector<uint32_t>& normals,
		std::vector<uint32_t>& tangent_quats, 
		std::vector<uint32_t>& diffuses, std::vector<uint32_t>& speculars,
		std::vector<int16_t>& tex_coords, 
		std::vector<uint32_t>& bone_indices, std::vector<uint32_t>& bone_weights)
	{
		std::vector<float3> mesh_positions;
		std::vector<float3> mesh_normals;
		std::vector<float4> mesh_tangents;
		std::vector<float3> mesh_binormals;
		std::vector<Quaternion> mesh_tangent_quats;
		std::vector<float4> mesh_diffuses;
		std::vector<float3> mesh_speculars;
		std::vector<float2> mesh_tex_coords;
		std::vector<uint32_t> mesh_bone_indices;
		std::vector<uint32_t> mesh_bone_weights;

		bool recompute_pos_bb;
		XMLNodePtr pos_bb_node = vertices_chunk->FirstNode("pos_bb");
		if (pos_bb_node)
		{
			float3 pos_min_bb, pos_max_bb;
			{
				XMLAttributePtr attr = pos_bb_node->Attrib("min");
				if (attr)
				{
					std::istringstream attr_ss(attr->ValueString());
					attr_ss >> pos_min_bb.x() >> pos_min_bb.y() >> pos_min_bb.z();
				}
				else
				{
					XMLNodePtr pos_min_node = pos_bb_node->FirstNode("min");
					pos_min_bb.x() = pos_min_node->Attrib("x")->ValueFloat();
					pos_min_bb.y() = pos_min_node->Attrib("y")->ValueFloat();
					pos_min_bb.z() = pos_min_node->Attrib("z")->ValueFloat();
				}
			}
			{
				XMLAttributePtr attr = pos_bb_node->Attrib("max");
				if (attr)
				{
					std::istringstream attr_ss(attr->ValueString());
					attr_ss >> pos_max_bb.x() >> pos_max_bb.y() >> pos_max_bb.z();
				}
				else
				{
					XMLNodePtr pos_max_node = pos_bb_node->FirstNode("max");
					pos_max_bb.x() = pos_max_node->Attrib("x")->ValueFloat();
					pos_max_bb.y() = pos_max_node->Attrib("y")->ValueFloat();
					pos_max_bb.z() = pos_max_node->Attrib("z")->ValueFloat();
				}
			}
			pos_bb = AABBox(pos_min_bb, pos_max_bb);

			recompute_pos_bb = false;
		}
		else
		{
			recompute_pos_bb = true;
		}

		bool recompute_tc_bb;
		XMLNodePtr tc_bb_node = vertices_chunk->FirstNode("tc_bb");
		if (tc_bb_node)
		{
			float3 tc_min_bb, tc_max_bb;
			{
				XMLAttributePtr attr = tc_bb_node->Attrib("min");
				if (attr)
				{
					std::istringstream attr_ss(attr->ValueString());
					attr_ss >> tc_min_bb.x() >> tc_min_bb.y();
				}
				else
				{
					XMLNodePtr tc_min_node = tc_bb_node->FirstNode("min");
					tc_min_bb.x() = tc_min_node->Attrib("x")->ValueFloat();
					tc_min_bb.y() = tc_min_node->Attrib("y")->ValueFloat();
				}
			}
			{
				XMLAttributePtr attr = tc_bb_node->Attrib("max");
				if (attr)
				{
					std::istringstream attr_ss(attr->ValueString());
					attr_ss >> tc_max_bb.x() >> tc_max_bb.y();
				}
				else
				{
					XMLNodePtr tc_max_node = tc_bb_node->FirstNode("max");							
					tc_max_bb.x() = tc_max_node->Attrib("x")->ValueFloat();
					tc_max_bb.y() = tc_max_node->Attrib("y")->ValueFloat();
				}
			}

			tc_min_bb.z() = 0;
			tc_max_bb.z() = 0;
			tc_bb = AABBox(tc_min_bb, tc_max_bb);

			recompute_tc_bb = false;
		}
		else
		{
			recompute_tc_bb = true;
		}

		bool has_normal = false;
		bool has_diffuse = false;
		bool has_specular = false;
		bool has_weight = false;
		bool has_tex_coord = false;
		bool has_tangent = false;
		bool has_binormal = false;
		bool has_tangent_quat = false;

		for (XMLNodePtr vertex_node = vertices_chunk->FirstNode("vertex"); vertex_node; vertex_node = vertex_node->NextSibling("vertex"))
		{
			{
				float3 pos;
				XMLAttributePtr attr = vertex_node->Attrib("x");
				if (attr)
				{
					pos.x() = vertex_node->Attrib("x")->ValueFloat();
					pos.y() = vertex_node->Attrib("y")->ValueFloat();
					pos.z() = vertex_node->Attrib("z")->ValueFloat();

					attr = vertex_node->Attrib("u");
					if (attr)
					{
						float2 tex_coord;
						tex_coord.x() = vertex_node->Attrib("u")->ValueFloat();
						tex_coord.y() = vertex_node->Attrib("v")->ValueFloat();
						mesh_tex_coords.push_back(tex_coord);
					}
				}
				else
				{
					std::istringstream attr_ss(vertex_node->Attrib("v")->ValueString());
					attr_ss >> pos.x() >> pos.y() >> pos.z();
				}
				mesh_positions.push_back(pos);
			}

			XMLNodePtr diffuse_node = vertex_node->FirstNode("diffuse");
			if (diffuse_node)
			{
				has_diffuse = true;

				float4 diffuse;
				XMLAttributePtr attr = diffuse_node->Attrib("v");
				if (attr)
				{
					std::istringstream attr_ss(attr->ValueString());
					attr_ss >> diffuse.x() >> diffuse.y() >> diffuse.z() >> diffuse.w();
				}
				else
				{
					diffuse.x() = diffuse_node->Attrib("r")->ValueFloat();
					diffuse.y() = diffuse_node->Attrib("g")->ValueFloat();
					diffuse.z() = diffuse_node->Attrib("b")->ValueFloat();
					diffuse.w() = diffuse_node->Attrib("a")->ValueFloat();										
				}
				mesh_diffuses.push_back(diffuse);
			}

			XMLNodePtr specular_node = vertex_node->FirstNode("specular");
			if (specular_node)
			{
				has_specular = true;

				float3 specular;
				XMLAttributePtr attr = specular_node->Attrib("v");
				if (attr)
				{
					std::istringstream attr_ss(attr->ValueString());
					attr_ss >> specular.x() >> specular.y() >> specular.z();
				}
				else
				{
					specular.x() = specular_node->Attrib("r")->ValueFloat();
					specular.y() = specular_node->Attrib("g")->ValueFloat();
					specular.z() = specular_node->Attrib("b")->ValueFloat();
				}
				mesh_speculars.push_back(specular);
			}

			if (!vertex_node->Attrib("u"))
			{
				XMLNodePtr tex_coord_node = vertex_node->FirstNode("tex_coord");
				if (tex_coord_node)
				{
					has_tex_coord = true;

					float2 tex_coord;
					XMLAttributePtr attr = tex_coord_node->Attrib("u");
					if (attr)
					{
						tex_coord.x() = tex_coord_node->Attrib("u")->ValueFloat();
						tex_coord.y() = tex_coord_node->Attrib("v")->ValueFloat();
					}
					else
					{
						std::istringstream attr_ss(tex_coord_node->Attrib("v")->ValueString());
						attr_ss >> tex_coord.x() >> tex_coord.y();
					}
					mesh_tex_coords.push_back(tex_coord);
				}
			}

			XMLNodePtr weight_node = vertex_node->FirstNode("weight");
			if (weight_node)
			{
				has_weight = true;

				uint32_t bone_index32[4] = { 0, 0, 0, 0 };
				float bone_weight32[4] = { 0, 0, 0, 0 };

				uint32_t num_blend = 0;
				XMLAttributePtr attr = weight_node->Attrib("joint");
				if (!attr)
				{
					attr = weight_node->Attrib("bone_index");
				}
				if (attr)
				{
					std::istringstream attr_ss_index(attr->ValueString());
					std::istringstream attr_ss_weight(weight_node->Attrib("weight")->ValueString());
					while (attr_ss_index && attr_ss_weight && (num_blend < 4))
					{
						attr_ss_index >> bone_index32[num_blend];
						attr_ss_weight >> bone_weight32[num_blend];
						++ num_blend;
					}
				}
				else
				{
					while (weight_node && (num_blend < 4))
					{
						bone_index32[num_blend] = weight_node->Attrib("bone_index")->ValueUInt();
						bone_weight32[num_blend] = weight_node->Attrib("weight")->ValueFloat();

						weight_node = weight_node->NextSibling("weight");
						++ num_blend;
					}
				}

				uint32_t index32 = 0;
				uint32_t weight32 = 0;
				for (size_t j = 0; j < 4; ++ j)
				{
					uint8_t bone_index = static_cast<uint8_t>(bone_index32[j]);
					uint8_t bone_weight = static_cast<uint8_t>(MathLib::clamp(static_cast<int>(bone_weight32[j] * 255), 0, 255));

					index32 |= (bone_index << (j * 8));
					weight32 |= (bone_weight << (j * 8));
				}
				mesh_bone_indices.push_back(index32);
				mesh_bone_weights.push_back(weight32);
			}
						
			XMLNodePtr normal_node = vertex_node->FirstNode("normal");
			if (normal_node)
			{
				has_normal = true;

				float3 normal;
				XMLAttributePtr attr = normal_node->Attrib("v");
				if (attr)
				{
					std::istringstream attr_ss(attr->ValueString());
					attr_ss >> normal.x() >> normal.y() >> normal.z();
				}
				else
				{
					normal.x() = normal_node->Attrib("x")->ValueFloat();
					normal.y() = normal_node->Attrib("y")->ValueFloat();
					normal.z() = normal_node->Attrib("z")->ValueFloat();
				}
				mesh_normals.push_back(normal);
			}

			XMLNodePtr tangent_node = vertex_node->FirstNode("tangent");
			if (tangent_node)
			{
				has_tangent = true;

				float4 tangent;
				XMLAttributePtr attr = tangent_node->Attrib("v");
				if (attr)
				{
					std::istringstream attr_ss(attr->ValueString());
					attr_ss >> tangent.x() >> tangent.y() >> tangent.z() >> tangent.w();
				}
				else
				{
					tangent.x() = tangent_node->Attrib("x")->ValueFloat();
					tangent.y() = tangent_node->Attrib("y")->ValueFloat();
					tangent.z() = tangent_node->Attrib("z")->ValueFloat();
					attr = tangent_node->Attrib("w");
					if (attr)
					{
						tangent.w() = attr->ValueFloat();
					}
					else
					{
						tangent.w() = 1;
					}
				}
				mesh_tangents.push_back(tangent);
			}

			XMLNodePtr binormal_node = vertex_node->FirstNode("binormal");
			if (binormal_node)
			{
				has_binormal = true;

				float3 binormal;
				XMLAttributePtr attr = binormal_node->Attrib("v");
				if (attr)
				{
					std::istringstream attr_ss(attr->ValueString());
					attr_ss >> binormal.x() >> binormal.y() >> binormal.z();
				}
				else
				{
					binormal.x() = binormal_node->Attrib("x")->ValueFloat();
					binormal.y() = binormal_node->Attrib("y")->ValueFloat();
					binormal.z() = binormal_node->Attrib("z")->ValueFloat();
				}
				mesh_binormals.push_back(binormal);
			}

			XMLNodePtr tangent_quat_node = vertex_node->FirstNode("tangent_quat");
			if (tangent_quat_node)
			{
				has_tangent_quat = true;

				Quaternion tangent_quat;
				XMLAttributePtr attr = tangent_quat_node->Attrib("v");
				if (attr)
				{
					std::istringstream attr_ss(tangent_quat_node->Attrib("v")->ValueString());
					attr_ss >> tangent_quat.x() >> tangent_quat.y() >> tangent_quat.z() >> tangent_quat.w();
				}
				else
				{
					tangent_quat.x() = tangent_quat_node->Attrib("x")->ValueFloat();
					tangent_quat.y() = tangent_quat_node->Attrib("y")->ValueFloat();
					tangent_quat.z() = tangent_quat_node->Attrib("z")->ValueFloat();
					tangent_quat.w() = tangent_quat_node->Attrib("w")->ValueFloat();
				}
				mesh_tangent_quats.push_back(tangent_quat);
			}
		}

		bool recompute_tangent_quat = false;

		{
			vertex_element ve;

			{
				ve.usage = VEU_Position;
				ve.usage_index = 0;
				ve.format = EF_SIGNED_ABGR16;
				vertex_elements.push_back(ve);
			}

			if (has_diffuse)
			{
				ve.usage = VEU_Diffuse;
				ve.usage_index = 0;
				ve.format = EF_ABGR8;
				vertex_elements.push_back(ve);
			}

			if (has_specular)
			{
				ve.usage = VEU_Specular;
				ve.usage_index = 0;
				ve.format = EF_ABGR8;
				vertex_elements.push_back(ve);
			}

			if (has_weight)
			{
				ve.usage = VEU_BlendWeight;
				ve.usage_index = 0;
				ve.format = EF_ABGR8;
				vertex_elements.push_back(ve);

				ve.usage = VEU_BlendIndex;
				ve.usage_index = 0;
				ve.format = EF_ABGR8UI;
				vertex_elements.push_back(ve);
			}

			if (has_tex_coord)
			{
				ve.usage = VEU_TextureCoord;
				ve.usage_index = 0;
				ve.format = EF_SIGNED_GR16;
				vertex_elements.push_back(ve);
			}

			if (has_tangent_quat)
			{
				ve.usage = VEU_Tangent;
				ve.usage_index = 0;
				ve.format = EF_ABGR8;
				vertex_elements.push_back(ve);
			}
			else
			{
				if (has_normal && !has_tangent && !has_binormal)
				{
					ve.usage = VEU_Normal;
					ve.usage_index = 0;
					ve.format = normal_in_8_bit ? EF_ABGR8 : EF_A2BGR10;
					vertex_elements.push_back(ve);
				}
				else
				{
					if ((has_normal && has_tangent) || (has_normal && has_binormal)
						|| (has_tangent && has_binormal))
					{
						ve.usage = VEU_Tangent;
						ve.usage_index = 0;
						ve.format = EF_ABGR8;
						vertex_elements.push_back(ve);

						if (!has_tangent_quat)
						{
							recompute_tangent_quat = true;
						}
					}
				}
			}
		}

		if (recompute_pos_bb)
		{
			for (uint32_t index = 0; index < mesh_positions.size(); ++ index)
			{
				float3 pos_min_bb, pos_max_bb;
				float3 const & pos = mesh_positions[index];
				if (0 == index)
				{
					pos_min_bb = pos_max_bb = pos;
				}
				else
				{
					pos_min_bb = MathLib::minimize(pos_min_bb, pos);
					pos_max_bb = MathLib::maximize(pos_max_bb, pos);
				}

				pos_bb = AABBox(pos_min_bb, pos_max_bb);
			}
		}
		if (recompute_tc_bb)
		{
			for (uint32_t index = 0; index < mesh_tex_coords.size(); ++ index)
			{
				float3 tc_min_bb, tc_max_bb;
				float3 tex_coord = float3(mesh_tex_coords[index].x(), mesh_tex_coords[index].y(), 0.0f);
				if (0 == index)
				{
					tc_min_bb = tc_max_bb = tex_coord;
				}
				else
				{
					tc_min_bb = MathLib::minimize(tc_min_bb, tex_coord);
					tc_max_bb = MathLib::maximize(tc_max_bb, tex_coord);
				}

				tc_bb = AABBox(tc_min_bb, tc_max_bb);
			}
		}
		if (recompute_tangent_quat)
		{
			mesh_tangent_quats.resize(mesh_positions.size());
			for (uint32_t index = 0; index < mesh_positions.size(); ++ index)
			{
				float3 tangent, binormal, normal;
				if (has_tangent)
				{
					tangent = float3(mesh_tangents[index].x(), mesh_tangents[index].y(),
						mesh_tangents[index].z());
				}
				if (has_binormal)
				{
					binormal = mesh_binormals[index];
				}
				if (has_normal)
				{
					normal = mesh_normals[index];
				}

				if (!has_tangent)
				{
					BOOST_ASSERT(has_binormal && has_normal);

					tangent = MathLib::cross(binormal, normal);
				}
				if (!has_binormal)
				{
					BOOST_ASSERT(has_tangent && has_normal);

					binormal = MathLib::cross(normal, tangent) * mesh_tangents[index].w();
				}
				if (!has_normal)
				{
					BOOST_ASSERT(has_tangent && has_binormal);

					normal = MathLib::cross(tangent, binormal);
				}

				mesh_tangent_quats[index] = MathLib::to_quaternion(tangent, binormal, normal, 8);
			}
		}

		float3 const pos_center = pos_bb.Center();
		float3 const pos_extent = pos_bb.HalfSize();
		float3 const tc_center = tc_bb.Center();
		float3 const tc_extent = tc_bb.HalfSize();

		for (uint32_t index = 0; index < mesh_positions.size(); ++ index)
		{
			float3 pos = mesh_positions[index];
			pos = (pos - pos_center) / pos_extent * 0.5f + 0.5f;
			int16_t s_pos[4] = 
			{
				static_cast<int16_t>(MathLib::clamp<int32_t>(static_cast<int32_t>(pos.x() * 65535 - 32768), -32768, 32767)),
				static_cast<int16_t>(MathLib::clamp<int32_t>(static_cast<int32_t>(pos.y() * 65535 - 32768), -32768, 32767)),
				static_cast<int16_t>(MathLib::clamp<int32_t>(static_cast<int32_t>(pos.z() * 65535 - 32768), -32768, 32767)),
				32767
			};

			positions.push_back(s_pos[0]);
			positions.push_back(s_pos[1]);
			positions.push_back(s_pos[2]);
			positions.push_back(s_pos[3]);
		}
		for (uint32_t index = 0; index < mesh_diffuses.size(); ++ index)
		{
			float4 const & diffuse = mesh_diffuses[index];
			uint32_t compact = (MathLib::clamp<uint32_t>(static_cast<uint32_t>((diffuse.x() * 0.5f + 0.5f) * 255), 0, 255) << 0)
				| (MathLib::clamp<uint32_t>(static_cast<uint32_t>((diffuse.y() * 0.5f + 0.5f) * 255), 0, 255) << 8)
				| (MathLib::clamp<uint32_t>(static_cast<uint32_t>((diffuse.z() * 0.5f + 0.5f) * 255), 0, 255) << 16)
				| (MathLib::clamp<uint32_t>(static_cast<uint32_t>((diffuse.w() * 0.5f + 0.5f) * 255), 0, 255) << 24);
			diffuses.push_back(compact);
		}
		for (uint32_t index = 0; index < mesh_speculars.size(); ++ index)
		{
			float3 const & specular = mesh_speculars[index];
			uint32_t compact = (MathLib::clamp<uint32_t>(static_cast<uint32_t>((specular.x() * 0.5f + 0.5f) * 255), 0, 255) << 0)
				| (MathLib::clamp<uint32_t>(static_cast<uint32_t>((specular.y() * 0.5f + 0.5f) * 255), 0, 255) << 8)
				| (MathLib::clamp<uint32_t>(static_cast<uint32_t>((specular.z() * 0.5f + 0.5f) * 255), 0, 255) << 16)
				| 0xFF000000;
			speculars.push_back(compact);
		}
		for (uint32_t index = 0; index < mesh_tex_coords.size(); ++ index)
		{
			float3 tex_coord = float3(mesh_tex_coords[index].x(), mesh_tex_coords[index].y(), 0.0f);
			tex_coord = (tex_coord - tc_center) / tc_extent * 0.5f + 0.5f;
			int16_t s_tc[2] = 
			{
				static_cast<int16_t>(MathLib::clamp<int32_t>(static_cast<int32_t>(tex_coord.x() * 65535 - 32768), -32768, 32767)),
				static_cast<int16_t>(MathLib::clamp<int32_t>(static_cast<int32_t>(tex_coord.y() * 65535 - 32768), -32768, 32767)),
			};

			tex_coords.push_back(s_tc[0]);
			tex_coords.push_back(s_tc[1]);
		}
		for (uint32_t index = 0; index < mesh_tangent_quats.size(); ++ index)
		{
			Quaternion const & tangent_quat = mesh_tangent_quats[index];
			uint32_t compact = (MathLib::clamp<uint32_t>(static_cast<uint32_t>((tangent_quat.x() * 0.5f + 0.5f) * 255), 0, 255) << 0)
				| (MathLib::clamp<uint32_t>(static_cast<uint32_t>((tangent_quat.y() * 0.5f + 0.5f) * 255), 0, 255) << 8)
				| (MathLib::clamp<uint32_t>(static_cast<uint32_t>((tangent_quat.z() * 0.5f + 0.5f) * 255), 0, 255) << 16)
				| (MathLib::clamp<uint32_t>(static_cast<uint32_t>((tangent_quat.w() * 0.5f + 0.5f) * 255), 0, 255) << 24);
			tangent_quats.push_back(compact);
		}
		for (uint32_t index = 0; index < mesh_normals.size(); ++ index)
		{
			float3 const normal = MathLib::normalize(mesh_normals[index]) * 0.5f + 0.5f;
			uint32_t compact;
			if (normal_in_8_bit)
			{
				compact = MathLib::clamp<uint32_t>(static_cast<uint32_t>(normal.x() * 255), 0, 255)
					| (MathLib::clamp<uint32_t>(static_cast<uint32_t>(normal.y() * 255), 0, 255) << 8)
					| (MathLib::clamp<uint32_t>(static_cast<uint32_t>(normal.z() * 255), 0, 255) << 16);
			}
			else
			{
				compact = MathLib::clamp<uint32_t>(static_cast<uint32_t>(normal.x() * 1023), 0, 1023)
					| (MathLib::clamp<uint32_t>(static_cast<uint32_t>(normal.y() * 1023), 0, 1023) << 10)
					| (MathLib::clamp<uint32_t>(static_cast<uint32_t>(normal.z() * 1023), 0, 1023) << 20);
			}
			normals.push_back(compact);					
		}
		bone_indices = mesh_bone_indices;
		bone_weights = mesh_bone_weights;
	}

	void CompileMeshesTrianglesChunk(XMLNodePtr const & triangles_chunk,
		std::vector<uint8_t>& triangle_indices, char& is_index_16)
	{
		std::vector<uint32_t> mesh_triangle_indices;

		is_index_16 = true;
		for (XMLNodePtr tri_node = triangles_chunk->FirstNode("triangle"); tri_node; tri_node = tri_node->NextSibling("triangle"))
		{
			uint32_t a, b, c;
			XMLAttributePtr attr = tri_node->Attrib("index");
			if (attr)
			{
				std::istringstream attr_ss(attr->ValueString());
				attr_ss >> a >> b >> c;
			}
			else
			{
				a = tri_node->Attrib("a")->ValueUInt();
				b = tri_node->Attrib("b")->ValueUInt();
				c = tri_node->Attrib("c")->ValueUInt();
			}
			mesh_triangle_indices.push_back(a);
			mesh_triangle_indices.push_back(b);
			mesh_triangle_indices.push_back(c);

			if ((a > 0xFFFF) || (b > 0xFFFF) || (c > 0xFFFF))
			{
				is_index_16 = false;
			}
		}

		if (is_index_16)
		{
			triangle_indices.resize(mesh_triangle_indices.size() * 2);
			for (uint32_t index = 0; index < mesh_triangle_indices.size(); ++ index)
			{
				*reinterpret_cast<uint16_t*>(&triangle_indices[index * 2])
					= static_cast<uint16_t>(mesh_triangle_indices[index]);
			}
		}
		else
		{
			triangle_indices.resize(mesh_triangle_indices.size() * 4);
			memcpy(&triangle_indices[0], &mesh_triangle_indices[0], triangle_indices.size());
		}
	}

	void AppendMeshVertices(std::vector<vertex_element> const & ves,
		std::vector<int16_t> const & positions, std::vector<uint32_t> const & normals,
		std::vector<uint32_t> const & tangent_quats, 
		std::vector<uint32_t> const & diffuses, std::vector<uint32_t> const & speculars,
		std::vector<int16_t> const & tex_coords, 
		std::vector<uint32_t> const & bone_indices, std::vector<uint32_t> const & bone_weights,
		std::vector<uint32_t>& mesh_num_vertices,
		std::vector<uint32_t>& mesh_base_vertices,
		std::vector<vertex_element>& merged_ves,
		std::vector<std::vector<uint8_t> >& merged_vertices)
	{
		uint32_t num_vertices = positions.size() / 4;
		uint32_t base_vertices = mesh_base_vertices.back();
		mesh_num_vertices.push_back(num_vertices);
		mesh_base_vertices.push_back(base_vertices + num_vertices);

		std::vector<uint32_t> ves_mapping(ves.size());
		for (uint32_t ve_index = 0; ve_index < ves.size(); ++ ve_index)
		{
			bool found = false;
			for (uint32_t mve_index = 0; mve_index < merged_ves.size(); ++ mve_index)
			{
				if (ves[ve_index] == merged_ves[mve_index])
				{
					ves_mapping[ve_index] = mve_index;
					found = true;
					break;
				}
			}
			if (!found)
			{
				ves_mapping[ve_index] = static_cast<uint32_t>(merged_ves.size());
				merged_ves.push_back(ves[ve_index]);
				merged_vertices.resize(merged_vertices.size() + 1);
				merged_vertices.back().resize(base_vertices * ves[ve_index].element_size(), 0);
			}
		}

		for (size_t i = 0; i < merged_vertices.size(); ++ i)
		{
			merged_vertices[i].resize(merged_vertices[i].size() + num_vertices * merged_ves[i].element_size(), 0);
		}

		{
			for (uint32_t vert_index = 0; vert_index < num_vertices; ++ vert_index)
			{
				{
					for (size_t i = 0; i < ves.size(); ++ i)
					{
						if (VEU_Position == ves[i].usage)
						{
							uint32_t buf_index = ves_mapping[i];
							int16_t s_pos[4] = 
							{
								positions[vert_index * 4 + 0],
								positions[vert_index * 4 + 1],
								positions[vert_index * 4 + 2],
								positions[vert_index * 4 + 3]
							};
							NativeToLittleEndian<sizeof(s_pos[0])>(&s_pos[0]);
							NativeToLittleEndian<sizeof(s_pos[1])>(&s_pos[1]);
							NativeToLittleEndian<sizeof(s_pos[2])>(&s_pos[2]);
							NativeToLittleEndian<sizeof(s_pos[3])>(&s_pos[3]);
							memcpy(&merged_vertices[buf_index][(base_vertices + vert_index) * merged_ves[buf_index].element_size()],
								s_pos, sizeof(s_pos));
							break;
						}
					}
				}

				if (!diffuses.empty())
				{
					for (size_t i = 0; i < ves.size(); ++ i)
					{
						if (VEU_Diffuse == ves[i].usage)
						{
							uint32_t buf_index = ves_mapping[i];
							uint32_t compact = diffuses[vert_index];
							NativeToLittleEndian<sizeof(compact)>(&compact);
							memcpy(&merged_vertices[buf_index][(base_vertices + vert_index) * merged_ves[buf_index].element_size()],
								&compact, sizeof(compact));
							break;
						}
					}
				}

				if (!speculars.empty())
				{
					for (size_t i = 0; i < ves.size(); ++ i)
					{
						if (VEU_Specular == ves[i].usage)
						{
							uint32_t buf_index = ves_mapping[i];
							uint32_t compact = speculars[vert_index];
							NativeToLittleEndian<sizeof(compact)>(&compact);
							memcpy(&merged_vertices[buf_index][(base_vertices + vert_index) * merged_ves[buf_index].element_size()],
								&compact, sizeof(compact));
							break;
						}
					}
				}

				if (!bone_indices.empty())
				{
					for (size_t i = 0; i < ves.size(); ++ i)
					{
						if (VEU_BlendIndex == ves[i].usage)
						{
							uint32_t buf_index = ves_mapping[i];
							uint32_t compact = bone_indices[vert_index];
							NativeToLittleEndian<sizeof(compact)>(&compact);
							memcpy(&merged_vertices[buf_index][(base_vertices + vert_index) * merged_ves[buf_index].element_size()],
								&compact, sizeof(compact));
							break;
						}
					}
					for (size_t i = 0; i < ves.size(); ++ i)
					{
						if (VEU_BlendWeight == ves[i].usage)
						{
							uint32_t buf_index = ves_mapping[i];
							uint32_t compact = bone_weights[vert_index];
							NativeToLittleEndian<sizeof(compact)>(&compact);
							memcpy(&merged_vertices[buf_index][(base_vertices + vert_index) * merged_ves[buf_index].element_size()],
								&compact, sizeof(compact));
							break;
						}
					}
				}

				if (!tex_coords.empty())
				{
					for (size_t i = 0; i < ves.size(); ++ i)
					{
						if (VEU_TextureCoord == ves[i].usage)
						{
							uint32_t buf_index = ves_mapping[i];
							int16_t s_tc[2] = 
							{
								tex_coords[vert_index * 2 + 0],
								tex_coords[vert_index * 2 + 1]
							};
							NativeToLittleEndian<sizeof(s_tc[0])>(&s_tc[0]);
							NativeToLittleEndian<sizeof(s_tc[1])>(&s_tc[1]);
							memcpy(&merged_vertices[buf_index][(base_vertices + vert_index) * merged_ves[buf_index].element_size()],
								&s_tc, sizeof(s_tc));
							break;
						}
					}
				}

				if (!tangent_quats.empty())
				{
					for (size_t i = 0; i < ves.size(); ++ i)
					{
						if (VEU_Tangent == ves[i].usage)
						{
							uint32_t buf_index = ves_mapping[i];
							uint32_t compact = tangent_quats[vert_index];
							NativeToLittleEndian<sizeof(compact)>(&compact);
							memcpy(&merged_vertices[buf_index][(base_vertices + vert_index) * merged_ves[buf_index].element_size()],
								&compact, sizeof(compact));
							break;
						}
					}
				}
				else
				{
					if (!normals.empty())
					{
						for (size_t i = 0; i < ves.size(); ++ i)
						{
							if (VEU_Normal == ves[i].usage)
							{
								uint32_t buf_index = ves_mapping[i];
								uint32_t compact = normals[vert_index];
								NativeToLittleEndian<sizeof(compact)>(&compact);
								memcpy(&merged_vertices[buf_index][(base_vertices + vert_index) * merged_ves[buf_index].element_size()],
									&compact, sizeof(compact));
								break;
							}
						}
					}
				}
			}
		}
	}

	void AppendMeshIndices(std::vector<uint8_t> const & triangle_indices, char is_index_16s,
		std::vector<uint32_t>& mesh_num_indices,
		std::vector<uint32_t>& mesh_start_indices,
		std::vector<uint8_t>& merged_indices,
		char& is_index_16_bit)
	{
		is_index_16_bit &= is_index_16s;

		uint32_t num_indices = triangle_indices.size() / (is_index_16s ? 2 : 4);
		uint32_t start_indicees = mesh_start_indices.back();
		mesh_num_indices.push_back(num_indices);
		mesh_start_indices.push_back(start_indicees + num_indices);

		merged_indices.resize(merged_indices.size() + num_indices * 4);

		for (uint32_t ind_index = 0; ind_index < num_indices; ++ ind_index)
		{
			if (is_index_16s)
			{
				uint32_t ind32 = *reinterpret_cast<uint16_t const *>(&triangle_indices[ind_index * sizeof(uint16_t)]);
				memcpy(&merged_indices[(start_indicees + ind_index) * 4],
					&ind32, sizeof(ind32));
			}
			else
			{
				memcpy(&merged_indices[(start_indicees + ind_index) * 4],
					&triangle_indices[ind_index * sizeof(uint32_t)], sizeof(uint32_t));
			}
		}
	}

	void CompileMeshesChunk(XMLNodePtr const & meshes_chunk, bool normal_in_8_bit,
		std::vector<std::string>& mesh_names, std::vector<int32_t>& mtl_ids,
		std::vector<AABBox>& pos_bbs, std::vector<AABBox>& tc_bbs, 
		std::vector<uint32_t>& mesh_num_vertices, std::vector<uint32_t>& mesh_base_vertices,
		std::vector<uint32_t>& mesh_num_indices, std::vector<uint32_t>& mesh_start_indices,
		std::vector<vertex_element>& merged_ves, std::vector<std::vector<uint8_t> >& merged_vertices,
		std::vector<uint8_t>& merged_indices, char& is_index_16_bit)
	{
		mesh_names.clear();
		mtl_ids.clear();

		mesh_num_vertices.clear();
		mesh_num_indices.clear();
		mesh_base_vertices.assign(1, 0);
		mesh_start_indices.assign(1, 0);
		merged_ves.clear();
		merged_vertices.clear();
		merged_indices.clear();
		is_index_16_bit = true;

		std::vector<vertex_element> ves;
		std::vector<int16_t> positions;
		std::vector<uint32_t> normals;
		std::vector<uint32_t> tangent_quats;
		std::vector<uint32_t> diffuses;
		std::vector<uint32_t> speculars;
		std::vector<int16_t> tex_coords;
		std::vector<uint32_t> bone_indices;
		std::vector<uint32_t> bone_weights;
		std::vector<uint8_t> triangle_indices;
		char is_index_16s;

		uint32_t mesh_index = 0;
		for (XMLNodePtr mesh_node = meshes_chunk->FirstNode("mesh"); mesh_node; mesh_node = mesh_node->NextSibling("mesh"), ++ mesh_index)
		{
			mesh_names.push_back(mesh_node->Attrib("name")->ValueString());
			mtl_ids.push_back(mesh_node->Attrib("mtl_id")->ValueInt());

			pos_bbs.resize(mesh_index + 1);
			tc_bbs.resize(pos_bbs.size());

			ves.clear();
			positions.clear();
			normals.clear();
			tangent_quats.clear();
			diffuses.clear();
			speculars.clear();
			tex_coords.clear();
			bone_indices.clear();
			bone_weights.clear();

			XMLNodePtr vertices_chunk = mesh_node->FirstNode("vertices_chunk");
			if (vertices_chunk)
			{
				CompileMeshesVerticesChunk(vertices_chunk, normal_in_8_bit,
					pos_bbs[mesh_index], tc_bbs[mesh_index], ves,
					positions, normals,	tangent_quats,
					diffuses, speculars, tex_coords,
					bone_indices, bone_weights);
				AppendMeshVertices(ves,
					positions, normals, tangent_quats, 
					diffuses, speculars, tex_coords, 
					bone_indices, bone_weights,
					mesh_num_vertices, mesh_base_vertices,
					merged_ves, merged_vertices);
			}

			triangle_indices.clear();
			is_index_16s = true;

			XMLNodePtr triangles_chunk = mesh_node->FirstNode("triangles_chunk");
			if (triangles_chunk)
			{
				CompileMeshesTrianglesChunk(triangles_chunk,
					triangle_indices, is_index_16s);
				AppendMeshIndices(triangle_indices, is_index_16s,
					mesh_num_indices, mesh_start_indices, merged_indices,
					is_index_16_bit);
			}
		}

		if (is_index_16_bit)
		{
			std::vector<uint8_t> merged_indices_16(merged_indices.size() / 2);
			for (uint32_t ind_index = 0; ind_index < mesh_start_indices.back(); ++ ind_index)
			{
				uint16_t ind16 = static_cast<uint16_t>(*reinterpret_cast<uint32_t*>(&merged_indices[ind_index * sizeof(uint32_t)]));
				NativeToLittleEndian<sizeof(ind16)>(&ind16);
				memcpy(&merged_indices_16[ind_index * sizeof(uint16_t)], &ind16, sizeof(ind16));
			}

			merged_indices.swap(merged_indices_16);
		}
	}

	void CompileBonesChunk(XMLNodePtr const & bones_chunk,
		std::vector<Joint>& joints)
	{
		Joint joint;
		for (XMLNodePtr bone_node = bones_chunk->FirstNode("bone"); bone_node; bone_node = bone_node->NextSibling("bone"))
		{
			joint.name = bone_node->Attrib("name")->ValueString();
			joint.parent = static_cast<int16_t>(bone_node->Attrib("parent")->ValueInt());

			XMLNodePtr bind_pos_node = bone_node->FirstNode("bind_pos");
			if (bind_pos_node)
			{
				float3 bind_pos(bind_pos_node->Attrib("x")->ValueFloat(), bind_pos_node->Attrib("y")->ValueFloat(),
					bind_pos_node->Attrib("z")->ValueFloat());

				XMLNodePtr bind_quat_node = bone_node->FirstNode("bind_quat");
				Quaternion bind_quat(bind_quat_node->Attrib("x")->ValueFloat(), bind_quat_node->Attrib("y")->ValueFloat(),
					bind_quat_node->Attrib("z")->ValueFloat(), bind_quat_node->Attrib("w")->ValueFloat());

				float scale = MathLib::length(bind_quat);
				bind_quat /= scale;

				joint.bind_dual = MathLib::quat_trans_to_udq(bind_quat, bind_pos);
				joint.bind_real = bind_quat * scale;
			}
			else
			{
				XMLNodePtr bind_real_node = bone_node->FirstNode("real");
				if (!bind_real_node)
				{
					bind_real_node = bone_node->FirstNode("bind_real");
				}
				XMLAttributePtr attr = bind_real_node->Attrib("v");
				if (attr)
				{
					std::istringstream attr_ss(attr->ValueString());
					attr_ss >> joint.bind_real.x() >> joint.bind_real.y()
						>> joint.bind_real.z() >> joint.bind_real.w();
				}
				else
				{
					joint.bind_real.x() = bind_real_node->Attrib("x")->ValueFloat();
					joint.bind_real.y() = bind_real_node->Attrib("y")->ValueFloat();
					joint.bind_real.z() = bind_real_node->Attrib("z")->ValueFloat();
					joint.bind_real.w() = bind_real_node->Attrib("w")->ValueFloat();
				}

				XMLNodePtr bind_dual_node = bone_node->FirstNode("dual");
				if (!bind_dual_node)
				{
					bind_dual_node = bone_node->FirstNode("bind_dual");
				}
				attr = bind_dual_node->Attrib("v");
				if (attr)
				{
					std::istringstream attr_ss(attr->ValueString());
					attr_ss >> joint.bind_dual.x() >> joint.bind_dual.y()
						>> joint.bind_dual.z() >> joint.bind_dual.w();
				}
				else
				{
					joint.bind_dual.x() = bind_dual_node->Attrib("x")->ValueFloat();
					joint.bind_dual.y() = bind_dual_node->Attrib("y")->ValueFloat();
					joint.bind_dual.z() = bind_dual_node->Attrib("z")->ValueFloat();
					joint.bind_dual.w() = bind_dual_node->Attrib("w")->ValueFloat();
				}
			}

			joints.push_back(joint);
		}
	}

	void CompileKeyFramesChunk(XMLNodePtr const & key_frames_chunk,
		uint32_t& num_frames, uint32_t& frame_rate,
		std::vector<KeyFrames>& kfss)
	{
		XMLAttributePtr nf_attr = key_frames_chunk->Attrib("num_frames");
		if (nf_attr)
		{
			num_frames = nf_attr->ValueUInt();
		}
		else
		{
			int32_t start_frame = key_frames_chunk->Attrib("start_frame")->ValueInt();
			int32_t end_frame = key_frames_chunk->Attrib("end_frame")->ValueInt();
			num_frames = end_frame - start_frame;
		}
		frame_rate = key_frames_chunk->Attrib("frame_rate")->ValueUInt();

		KeyFrames kfs;
		for (XMLNodePtr kf_node = key_frames_chunk->FirstNode("key_frame"); kf_node; kf_node = kf_node->NextSibling("key_frame"))
		{
			kfs.frame_id.clear();
			kfs.bind_real.clear();
			kfs.bind_dual.clear();
			kfs.bind_scale.clear();

			int32_t frame_id = -1;
			for (XMLNodePtr key_node = kf_node->FirstNode("key"); key_node; key_node = key_node->NextSibling("key"))
			{
				XMLAttributePtr id_attr = key_node->Attrib("id");
				if (id_attr)
				{
					frame_id = id_attr->ValueInt();
				}
				else
				{
					++ frame_id;
				}
				kfs.frame_id.push_back(frame_id);

				Quaternion bind_real, bind_dual;
				float bind_scale;
				XMLNodePtr pos_node = key_node->FirstNode("pos");
				if (pos_node)
				{
					float3 bind_pos(pos_node->Attrib("x")->ValueFloat(), pos_node->Attrib("y")->ValueFloat(),
						pos_node->Attrib("z")->ValueFloat());

					XMLNodePtr quat_node = key_node->FirstNode("quat");
					bind_real = Quaternion(quat_node->Attrib("x")->ValueFloat(), quat_node->Attrib("y")->ValueFloat(),
						quat_node->Attrib("z")->ValueFloat(), quat_node->Attrib("w")->ValueFloat());

					bind_scale = MathLib::length(bind_real);
					bind_real /= bind_scale;

					bind_dual = MathLib::quat_trans_to_udq(bind_real, bind_pos);
				}
				else
				{
					XMLNodePtr bind_real_node = key_node->FirstNode("real");
					if (!bind_real_node)
					{
						bind_real_node = key_node->FirstNode("bind_real");
					}
					XMLAttributePtr attr = bind_real_node->Attrib("v");
					if (attr)
					{
						std::istringstream attr_ss(attr->ValueString());
						attr_ss >> bind_real.x() >> bind_real.y() >> bind_real.z() >> bind_real.w();
					}
					else
					{
						bind_real.x() = bind_real_node->Attrib("x")->ValueFloat();
						bind_real.y() = bind_real_node->Attrib("y")->ValueFloat();
						bind_real.z() = bind_real_node->Attrib("z")->ValueFloat();
						bind_real.w() = bind_real_node->Attrib("w")->ValueFloat();
					}
							
					XMLNodePtr bind_dual_node = key_node->FirstNode("dual");
					if (!bind_dual_node)
					{
						bind_dual_node = key_node->FirstNode("bind_dual");
					}
					attr = bind_dual_node->Attrib("v");
					if (attr)
					{
						std::istringstream attr_ss(attr->ValueString());
						attr_ss >> bind_dual.x() >> bind_dual.y() >> bind_dual.z() >> bind_dual.w();
					}
					else
					{
						bind_dual.x() = bind_dual_node->Attrib("x")->ValueFloat();
						bind_dual.y() = bind_dual_node->Attrib("y")->ValueFloat();
						bind_dual.z() = bind_dual_node->Attrib("z")->ValueFloat();
						bind_dual.w() = bind_dual_node->Attrib("w")->ValueFloat();
					}

					bind_scale = MathLib::length(bind_real);
					bind_real /= bind_scale;
					if (bind_real.w() < 0)
					{
						bind_real = -bind_real;
						bind_scale = -bind_scale;
					}
				}

				kfs.bind_real.push_back(bind_real);
				kfs.bind_dual.push_back(bind_dual);
				kfs.bind_scale.push_back(bind_scale);
			}

			kfss.push_back(kfs);
		}
	}

	void CompileBBKeyFramesChunk(XMLNodePtr const & bb_kfs_chunk,
		std::vector<AABBox> const & pos_bbs, uint32_t num_frames,
		std::vector<AABBKeyFrames>& bb_kfss)
	{
		AABBKeyFrames bb_kfs;
		if (bb_kfs_chunk)
		{
			for (XMLNodePtr bb_kf_node = bb_kfs_chunk->FirstNode("bb_key_frame"); bb_kf_node; bb_kf_node = bb_kf_node->NextSibling("bb_key_frame"))
			{
				bb_kfs.frame_id.clear();
				bb_kfs.bb.clear();

				int32_t frame_id = -1;
				for (XMLNodePtr key_node = bb_kf_node->FirstNode("key"); key_node; key_node = key_node->NextSibling("key"))
				{
					XMLAttributePtr id_attr = key_node->Attrib("id");
					if (id_attr)
					{
						frame_id = id_attr->ValueInt();
					}
					else
					{
						++ frame_id;
					}
					bb_kfs.frame_id.push_back(frame_id);

					float3 bb_min, bb_max;
					XMLAttributePtr attr = key_node->Attrib("min");
					if (attr)
					{
						std::istringstream attr_ss(attr->ValueString());
						attr_ss >> bb_min.x() >> bb_min.y() >> bb_min.z();
					}
					else
					{
						XMLNodePtr min_node = key_node->FirstNode("min");
						bb_min.x() = min_node->Attrib("x")->ValueFloat();
						bb_min.y() = min_node->Attrib("y")->ValueFloat();
						bb_min.z() = min_node->Attrib("z")->ValueFloat();
					}
					attr = key_node->Attrib("max");
					if (attr)
					{
						std::istringstream attr_ss(attr->ValueString());
						attr_ss >> bb_max[0] >> bb_max[1] >> bb_max[2];
					}
					else
					{
						XMLNodePtr max_node = key_node->FirstNode("max");
						bb_max.x() = max_node->Attrib("x")->ValueFloat();
						bb_max.y() = max_node->Attrib("y")->ValueFloat();
						bb_max.z() = max_node->Attrib("z")->ValueFloat();
					}

					bb_kfs.bb.push_back(AABBox(bb_min, bb_max));
				}

				bb_kfss.push_back(bb_kfs);
			}
		}
		else
		{
			bb_kfs.frame_id.resize(2);
			bb_kfs.bb.resize(2);

			bb_kfs.frame_id[0] = 0;
			bb_kfs.frame_id[1] = num_frames - 1;

			for (uint32_t mesh_index = 0; mesh_index < pos_bbs.size(); ++ mesh_index)
			{
				bb_kfs.bb[0] = pos_bbs[mesh_index];
				bb_kfs.bb[1] = pos_bbs[mesh_index];

				bb_kfss.push_back(bb_kfs);
			}
		}
	}

	void CompileActionsChunk(XMLNodePtr const & actions_chunk,
		uint32_t num_frames,
		std::vector<AnimationAction>& actions)
	{
		XMLNodePtr action_node;
		if (actions_chunk)
		{
			action_node = actions_chunk->FirstNode("action");
		}

		AnimationAction action;
		if (action_node)
		{
			for (; action_node; action_node = action_node->NextSibling("action"))
			{
				action.name = action_node->Attrib("name")->ValueString();

				action.start_frame = action_node->Attrib("start")->ValueUInt();
				action.end_frame = action_node->Attrib("end")->ValueUInt();

				actions.push_back(action);
			}
		}
		else
		{
			action.name = "root";
			action.start_frame = 0;
			action.end_frame = num_frames;

			actions.push_back(action);
		}
	}

	void WriteMaterialsChunk(std::vector<RenderMaterial> const & mtls, std::ostream& os)
	{
		for (size_t i = 0; i < mtls.size(); ++ i)
		{
			RenderMaterial mtl = mtls[i];

			NativeToLittleEndian<sizeof(mtl.ambient[0])>(&mtl.ambient[0]);
			NativeToLittleEndian<sizeof(mtl.ambient[1])>(&mtl.ambient[1]);
			NativeToLittleEndian<sizeof(mtl.ambient[2])>(&mtl.ambient[2]);
			NativeToLittleEndian<sizeof(mtl.diffuse[0])>(&mtl.diffuse[0]);
			NativeToLittleEndian<sizeof(mtl.diffuse[1])>(&mtl.diffuse[1]);
			NativeToLittleEndian<sizeof(mtl.diffuse[2])>(&mtl.diffuse[2]);
			NativeToLittleEndian<sizeof(mtl.specular[0])>(&mtl.specular[0]);
			NativeToLittleEndian<sizeof(mtl.specular[1])>(&mtl.specular[1]);
			NativeToLittleEndian<sizeof(mtl.specular[2])>(&mtl.specular[2]);
			NativeToLittleEndian<sizeof(mtl.emit[0])>(&mtl.emit[0]);
			NativeToLittleEndian<sizeof(mtl.emit[1])>(&mtl.emit[1]);
			NativeToLittleEndian<sizeof(mtl.emit[2])>(&mtl.emit[2]);
			NativeToLittleEndian<sizeof(mtl.opacity)>(&mtl.opacity);
			NativeToLittleEndian<sizeof(mtl.specular_level)>(&mtl.specular_level);
			NativeToLittleEndian<sizeof(mtl.shininess)>(&mtl.shininess);

			os.write(reinterpret_cast<char*>(&mtl.ambient), sizeof(mtl.ambient));
			os.write(reinterpret_cast<char*>(&mtl.diffuse), sizeof(mtl.diffuse));
			os.write(reinterpret_cast<char*>(&mtl.specular), sizeof(mtl.specular));
			os.write(reinterpret_cast<char*>(&mtl.emit), sizeof(mtl.emit));
			os.write(reinterpret_cast<char*>(&mtl.opacity), sizeof(mtl.opacity));
			os.write(reinterpret_cast<char*>(&mtl.specular_level), sizeof(mtl.specular_level));
			os.write(reinterpret_cast<char*>(&mtl.shininess), sizeof(mtl.shininess));

			uint32_t num_texs = static_cast<uint32_t>(mtl.texture_slots.size());
			NativeToLittleEndian<sizeof(num_texs)>(&num_texs);
			os.write(reinterpret_cast<char*>(&num_texs), sizeof(num_texs));

			if (!mtl.texture_slots.empty())
			{
				for (size_t j = 0; j < mtl.texture_slots.size(); ++ j)
				{
					WriteShortString(os, mtl.texture_slots[j].first);
					WriteShortString(os, mtl.texture_slots[j].second);
				}
			}
		}
	}

	void WriteMeshesChunk(std::vector<std::string> const & mesh_names, std::vector<int32_t> const & mtl_ids,
		std::vector<AABBox> const & pos_bbs, std::vector<AABBox> const & tc_bbs,
		std::vector<uint32_t> const & mesh_num_vertices, std::vector<uint32_t> const & mesh_base_vertices,
		std::vector<uint32_t> const & mesh_num_indices, std::vector<uint32_t> const & mesh_start_indices,
		std::vector<vertex_element> const & merged_ves,
		std::vector<std::vector<uint8_t> > const & merged_vertices, std::vector<uint8_t> const & merged_indices,
		char is_index_16_bit, std::ostream& os)
	{
		uint32_t num_merged_ves = static_cast<uint32_t>(merged_ves.size());
		NativeToLittleEndian<sizeof(num_merged_ves)>(&num_merged_ves);
		os.write(reinterpret_cast<char*>(&num_merged_ves), sizeof(num_merged_ves));
		for (size_t i = 0; i < merged_ves.size(); ++ i)
		{
			vertex_element ve = merged_ves[i];
			NativeToLittleEndian<sizeof(ve.usage)>(&ve.usage);
			NativeToLittleEndian<sizeof(ve.format)>(&ve.format);
			os.write(reinterpret_cast<char*>(&ve), sizeof(ve));
		}

		uint32_t num_vertices = mesh_base_vertices.back();
		NativeToLittleEndian<sizeof(num_vertices)>(&num_vertices);
		os.write(reinterpret_cast<char*>(&num_vertices), sizeof(num_vertices));
		uint32_t num_indices = mesh_start_indices.back();
		NativeToLittleEndian<sizeof(num_indices)>(&num_indices);
		os.write(reinterpret_cast<char*>(&num_indices), sizeof(num_indices));
		os.write(&is_index_16_bit, sizeof(is_index_16_bit));

		for (size_t i = 0; i < merged_vertices.size(); ++ i)
		{
			os.write(reinterpret_cast<char const *>(&merged_vertices[i][0]), merged_vertices[i].size() * sizeof(merged_vertices[i][0]));
		}
		os.write(reinterpret_cast<char const *>(&merged_indices[0]), merged_indices.size() * sizeof(merged_indices[0]));

		for (uint32_t mesh_index = 0; mesh_index < mesh_num_vertices.size(); ++ mesh_index)
		{
			WriteShortString(os, mesh_names[mesh_index]);

			int32_t mtl_id = mtl_ids[mesh_index];
			NativeToLittleEndian<sizeof(mtl_id)>(&mtl_id);
			os.write(reinterpret_cast<char*>(&mtl_id), sizeof(mtl_id));

			float3 min_bb = pos_bbs[mesh_index].Min();
			NativeToLittleEndian<sizeof(min_bb[0])>(&min_bb[0]);
			NativeToLittleEndian<sizeof(min_bb[1])>(&min_bb[1]);
			NativeToLittleEndian<sizeof(min_bb[2])>(&min_bb[2]);
			os.write(reinterpret_cast<char*>(&min_bb), sizeof(min_bb));
			float3 max_bb = pos_bbs[mesh_index].Max();
			NativeToLittleEndian<sizeof(max_bb[0])>(&max_bb[0]);
			NativeToLittleEndian<sizeof(max_bb[1])>(&max_bb[1]);
			NativeToLittleEndian<sizeof(max_bb[2])>(&max_bb[2]);
			os.write(reinterpret_cast<char*>(&max_bb), sizeof(max_bb));

			min_bb = tc_bbs[mesh_index].Min();
			NativeToLittleEndian<sizeof(min_bb[0])>(&min_bb[0]);
			NativeToLittleEndian<sizeof(min_bb[1])>(&min_bb[1]);
			os.write(reinterpret_cast<char*>(&min_bb[0]), sizeof(min_bb[0]));
			os.write(reinterpret_cast<char*>(&min_bb[1]), sizeof(min_bb[1]));
			max_bb = tc_bbs[mesh_index].Max();
			NativeToLittleEndian<sizeof(max_bb[0])>(&max_bb[0]);
			NativeToLittleEndian<sizeof(max_bb[1])>(&max_bb[1]);
			os.write(reinterpret_cast<char*>(&max_bb[0]), sizeof(max_bb[0]));
			os.write(reinterpret_cast<char*>(&max_bb[1]), sizeof(max_bb[1]));

			uint32_t nv = mesh_num_vertices[mesh_index];
			NativeToLittleEndian<sizeof(nv)>(&nv);
			os.write(reinterpret_cast<char*>(&nv), sizeof(nv));
			uint32_t bv = mesh_base_vertices[mesh_index];
			NativeToLittleEndian<sizeof(bv)>(&bv);
			os.write(reinterpret_cast<char*>(&bv), sizeof(bv));
			uint32_t ni = mesh_num_indices[mesh_index];
			NativeToLittleEndian<sizeof(ni)>(&ni);
			os.write(reinterpret_cast<char*>(&ni), sizeof(ni));
			uint32_t si = mesh_start_indices[mesh_index];
			NativeToLittleEndian<sizeof(si)>(&si);
			os.write(reinterpret_cast<char*>(&si), sizeof(si));
		}
	}

	void WriteBonesChunk(std::vector<Joint> const & joints, std::ostream& os)
	{
		for (size_t i = 0; i < joints.size(); ++ i)
		{
			WriteShortString(os, joints[i].name);

			int16_t joint_parent = joints[i].parent;
			NativeToLittleEndian<sizeof(joint_parent)>(&joint_parent);
			os.write(reinterpret_cast<char*>(&joint_parent), sizeof(joint_parent));

			Quaternion bind_real = joints[i].bind_real;
			Quaternion bind_dual = joints[i].bind_dual;
			NativeToLittleEndian<sizeof(bind_real[0])>(&bind_real[0]);
			NativeToLittleEndian<sizeof(bind_real[1])>(&bind_real[1]);
			NativeToLittleEndian<sizeof(bind_real[2])>(&bind_real[2]);
			NativeToLittleEndian<sizeof(bind_real[3])>(&bind_real[3]);
			os.write(reinterpret_cast<char*>(&bind_real), sizeof(bind_real));
			NativeToLittleEndian<sizeof(bind_dual[0])>(&bind_dual[0]);
			NativeToLittleEndian<sizeof(bind_dual[1])>(&bind_dual[1]);
			NativeToLittleEndian<sizeof(bind_dual[2])>(&bind_dual[2]);
			NativeToLittleEndian<sizeof(bind_dual[3])>(&bind_dual[3]);
			os.write(reinterpret_cast<char*>(&bind_dual), sizeof(bind_dual));
		}
	}

	void WriteKeyFramesChunk(uint32_t num_frames, uint32_t frame_rate, std::vector<KeyFrames>& kfs,
		std::ostream& os)
	{
		NativeToLittleEndian<sizeof(num_frames)>(&num_frames);
		os.write(reinterpret_cast<char*>(&num_frames), sizeof(num_frames));
		NativeToLittleEndian<sizeof(frame_rate)>(&frame_rate);
		os.write(reinterpret_cast<char*>(&frame_rate), sizeof(frame_rate));

		for (size_t i = 0; i < kfs.size(); ++ i)
		{
			uint32_t num_kf = static_cast<uint32_t>(kfs[i].frame_id.size());
			NativeToLittleEndian<sizeof(num_kf)>(&num_kf);
			os.write(reinterpret_cast<char*>(&num_kf), sizeof(num_kf));

			for (size_t j = 0; j < kfs[i].frame_id.size(); ++ j)
			{
				uint32_t frame_id = kfs[i].frame_id[j];
				Quaternion bind_real = kfs[i].bind_real[j];
				Quaternion bind_dual = kfs[i].bind_dual[j];
				float bind_scale = MathLib::length(bind_real);
				bind_real /= bind_scale;
				if (bind_real.w() < 0)
				{
					bind_real = -bind_real;
					bind_scale = -bind_scale;
				}

				NativeToLittleEndian<sizeof(frame_id)>(&frame_id);
				os.write(reinterpret_cast<char*>(&frame_id), sizeof(frame_id));
				bind_real *= bind_scale;
				NativeToLittleEndian<sizeof(bind_real[0])>(&bind_real[0]);
				NativeToLittleEndian<sizeof(bind_real[1])>(&bind_real[1]);
				NativeToLittleEndian<sizeof(bind_real[2])>(&bind_real[2]);
				NativeToLittleEndian<sizeof(bind_real[3])>(&bind_real[3]);
				os.write(reinterpret_cast<char*>(&bind_real), sizeof(bind_real));
				NativeToLittleEndian<sizeof(bind_dual[0])>(&bind_dual[0]);
				NativeToLittleEndian<sizeof(bind_dual[1])>(&bind_dual[1]);
				NativeToLittleEndian<sizeof(bind_dual[2])>(&bind_dual[2]);
				NativeToLittleEndian<sizeof(bind_dual[3])>(&bind_dual[3]);
				os.write(reinterpret_cast<char*>(&bind_dual), sizeof(bind_dual));
			}
		}
	}

	void WriteBBKeyFramesChunk(std::vector<AABBKeyFrames> const & bb_kfs, std::ostream& os)
	{
		for (size_t i = 0; i < bb_kfs.size(); ++ i)
		{
			uint32_t num_bb_kf = static_cast<uint32_t>(bb_kfs[i].frame_id.size());
			NativeToLittleEndian<sizeof(num_bb_kf)>(&num_bb_kf);
			os.write(reinterpret_cast<char*>(&num_bb_kf), sizeof(num_bb_kf));

			for (uint32_t j = 0; j < bb_kfs[i].frame_id.size(); ++ j)
			{
				uint32_t frame_id = bb_kfs[i].frame_id[j];
				NativeToLittleEndian<sizeof(frame_id)>(&frame_id);
				os.write(reinterpret_cast<char*>(&frame_id), sizeof(frame_id));
				float3 bb_min = bb_kfs[i].bb[j].Min();
				float3 bb_max = bb_kfs[i].bb[j].Max();
				NativeToLittleEndian<sizeof(bb_min[0])>(&bb_min[0]);
				NativeToLittleEndian<sizeof(bb_min[1])>(&bb_min[1]);
				NativeToLittleEndian<sizeof(bb_min[2])>(&bb_min[2]);
				os.write(reinterpret_cast<char*>(&bb_min), sizeof(bb_min));
				NativeToLittleEndian<sizeof(bb_max[0])>(&bb_max[0]);
				NativeToLittleEndian<sizeof(bb_max[1])>(&bb_max[1]);
				NativeToLittleEndian<sizeof(bb_max[2])>(&bb_max[2]);
				os.write(reinterpret_cast<char*>(&bb_max), sizeof(bb_max));
			}
		}
	}

	void WriteActionsChunk(std::vector<AnimationAction> const & actions, std::ostream& os)
	{
		for (size_t i = 0; i < actions.size(); ++ i)
		{
			WriteShortString(os, actions[i].name);

			uint32_t sf = actions[i].start_frame;
			uint32_t ef = actions[i].end_frame;

			NativeToLittleEndian<sizeof(sf)>(&sf);
			os.write(reinterpret_cast<char*>(&sf), sizeof(sf));

			NativeToLittleEndian<sizeof(ef)>(&ef);
			os.write(reinterpret_cast<char*>(&ef), sizeof(ef));
		}
	}

	void MeshMLJIT(std::string const & meshml_name, std::string const & output_name, bool normal_in_8_bit)
	{
		shared_ptr<std::stringstream> ss = MakeSharedPtr<std::stringstream>();

		ResIdentifierPtr file = ResLoader::Instance().Open(meshml_name);
		XMLDocument doc;
		XMLNodePtr root = doc.Parse(file);

		BOOST_ASSERT(root->Attrib("version") && (root->Attrib("version")->ValueInt() >= 1));

		XMLNodePtr materials_chunk = root->FirstNode("materials_chunk");
		std::vector<RenderMaterial> mtls;
		if (materials_chunk)
		{
			CompileMaterialsChunk(materials_chunk, mtls);
		}
		{
			uint32_t num_mtls = static_cast<uint32_t>(mtls.size());
			NativeToLittleEndian<sizeof(num_mtls)>(&num_mtls);
			ss->write(reinterpret_cast<char*>(&num_mtls), sizeof(num_mtls));
		}

		XMLNodePtr meshes_chunk = root->FirstNode("meshes_chunk");
		std::vector<std::string> mesh_names;
		std::vector<int32_t> mtl_ids;
		std::vector<AABBox> pos_bbs;
		std::vector<AABBox> tc_bbs;
		std::vector<uint32_t> mesh_num_vertices;
		std::vector<uint32_t> mesh_base_vertices;
		std::vector<uint32_t> mesh_num_indices;
		std::vector<uint32_t> mesh_start_indices;
		std::vector<vertex_element> merged_ves;
		std::vector<std::vector<uint8_t> > merged_vertices;
		std::vector<uint8_t> merged_indices;
		char is_index_16_bit = true;
		if (meshes_chunk)
		{
			CompileMeshesChunk(meshes_chunk, normal_in_8_bit, mesh_names, mtl_ids, pos_bbs, tc_bbs,
				mesh_num_vertices, mesh_base_vertices,
				mesh_num_indices, mesh_start_indices,
				merged_ves, merged_vertices, merged_indices,
				is_index_16_bit);
		}
		{
			uint32_t num_meshes = static_cast<uint32_t>(pos_bbs.size());
			NativeToLittleEndian<sizeof(num_meshes)>(&num_meshes);
			ss->write(reinterpret_cast<char*>(&num_meshes), sizeof(num_meshes));
		}

		XMLNodePtr bones_chunk = root->FirstNode("bones_chunk");
		std::vector<Joint> joints;
		if (bones_chunk)
		{
			CompileBonesChunk(bones_chunk, joints);
		}
		{
			uint32_t num_joints = static_cast<uint32_t>(joints.size());
			NativeToLittleEndian<sizeof(num_joints)>(&num_joints);
			ss->write(reinterpret_cast<char*>(&num_joints), sizeof(num_joints));
		}

		XMLNodePtr key_frames_chunk = root->FirstNode("key_frames_chunk");
		uint32_t num_frames = 0;
		uint32_t frame_rate = 0;
		std::vector<KeyFrames> kfs;
		std::vector<AABBKeyFrames> bb_kfs;
		if (key_frames_chunk)
		{
			CompileKeyFramesChunk(key_frames_chunk, num_frames, frame_rate, kfs);

			XMLNodePtr bb_kfs_chunk = root->FirstNode("bb_key_frames_chunk");
			CompileBBKeyFramesChunk(bb_kfs_chunk, pos_bbs, num_frames, bb_kfs);
		}
		{
			uint32_t num_kfs = static_cast<uint32_t>(kfs.size());
			NativeToLittleEndian<sizeof(num_kfs)>(&num_kfs);
			ss->write(reinterpret_cast<char*>(&num_kfs), sizeof(num_kfs));
		}

		XMLNodePtr actions_chunk = root->FirstNode("actions_chunk");
		std::vector<AnimationAction> actions;
		if (actions_chunk)
		{
			CompileActionsChunk(actions_chunk, num_frames, actions);
		}
		{
			uint32_t num_actions = key_frames_chunk ? std::max(static_cast<uint32_t>(actions.size()), 1U) : 0;
			NativeToLittleEndian<sizeof(num_actions)>(&num_actions);
			ss->write(reinterpret_cast<char*>(&num_actions), sizeof(num_actions));
		}

		if (materials_chunk)
		{
			WriteMaterialsChunk(mtls, *ss);
		}

		if (meshes_chunk)
		{
			WriteMeshesChunk(mesh_names, mtl_ids, pos_bbs, tc_bbs,
				mesh_num_vertices, mesh_base_vertices, mesh_num_indices, mesh_start_indices,
				merged_ves, merged_vertices, merged_indices, is_index_16_bit, *ss);
		}

		if (bones_chunk)
		{
			WriteBonesChunk(joints, *ss);
		}

		if (key_frames_chunk)
		{
			WriteKeyFramesChunk(num_frames, frame_rate, kfs, *ss);
			WriteBBKeyFramesChunk(bb_kfs, *ss);
			WriteActionsChunk(actions, *ss);
		}

		std::ofstream ofs(output_name.c_str(), std::ios_base::binary);
		BOOST_ASSERT(ofs);
		uint32_t fourcc = MakeFourCC<'K', 'L', 'M', ' '>::value;
		NativeToLittleEndian<sizeof(fourcc)>(&fourcc);
		ofs.write(reinterpret_cast<char*>(&fourcc), sizeof(fourcc));

		uint32_t ver = MODEL_BIN_VERSION;
		NativeToLittleEndian<sizeof(ver)>(&ver);
		ofs.write(reinterpret_cast<char*>(&ver), sizeof(ver));

		uint64_t original_len = ss->str().size();
		NativeToLittleEndian<sizeof(original_len)>(&original_len);
		ofs.write(reinterpret_cast<char*>(&original_len), sizeof(original_len));

		std::ofstream::pos_type p = ofs.tellp();
		uint64_t len = 0;
		ofs.write(reinterpret_cast<char*>(&len), sizeof(len));

		LZMACodec lzma;
		len = lzma.Encode(ofs, ss->str().c_str(), ss->str().size());

		ofs.seekp(p, std::ios_base::beg);
		NativeToLittleEndian<sizeof(len)>(&len);
		ofs.write(reinterpret_cast<char*>(&len), sizeof(len));
	}
}

int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		cout << "Usage: MeshMLJIT xxx.meshml [N8|N10] [xxx.meshml.model_bin] [-q]" << endl;
		return 1;
	}

	std::string meshml_name = argv[1];

	bool normal_in_8_bit = false;
	if ((argc >= 3) && ("N8" == std::string(argv[2])))
	{
		normal_in_8_bit = true;
	}

	std::string output_name;
	if (argc >= 4)
	{
		output_name = argv[3];
	}
	else
	{
		output_name = meshml_name + JIT_EXT_NAME;
	}

	bool quiet = false;
	if ((argc >= 5) && ("-q" == std::string(argv[4])))
	{
		quiet = true;
	}

	MeshMLJIT(meshml_name, output_name, normal_in_8_bit);

	if (!quiet)
	{
		cout << "Binary model has been saved to " << output_name << "." << endl;
	}

	return 0;
}
