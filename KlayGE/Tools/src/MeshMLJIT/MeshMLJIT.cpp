#include <KlayGE/KlayGE.hpp>
#include <KFL/Math.hpp>
#include <KFL/Util.hpp>
#include <KFL/XMLDom.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/RenderLayout.hpp>
#include <KlayGE/LZMACodec.hpp>
#include <KlayGE/RenderMaterial.hpp>
#include <KlayGE/Renderable.hpp>
#include <KlayGE/Mesh.hpp>
#include <KFL/Hash.hpp>
#include <KFL/CXX17/filesystem.hpp>

#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>

#if defined(KLAYGE_COMPILER_GCC)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations" // Ignore auto_ptr declaration
#endif
#include <boost/algorithm/string/split.hpp>
#if defined(KLAYGE_COMPILER_GCC)
#pragma GCC diagnostic pop
#endif
#include <boost/algorithm/string/trim.hpp>

#if defined(KLAYGE_COMPILER_GCC)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations" // Ignore auto_ptr declaration
#endif
#include <boost/program_options.hpp>
#if defined(KLAYGE_COMPILER_GCC)
#pragma GCC diagnostic pop
#endif

using namespace std;
using namespace KlayGE;

namespace
{
	struct OfflineRenderMaterial
	{
		RenderMaterial material;
		std::vector<std::pair<std::string, std::string>> texture_slots;
	};

	// Return path when appended to a_From will resolve to same as a_To
	filesystem::path make_relative(filesystem::path from_path, filesystem::path to_path)
	{
		from_path = filesystem::absolute(from_path);
		to_path = filesystem::absolute(to_path);

		filesystem::path::const_iterator iter_from(from_path.begin());
		filesystem::path::const_iterator iter_to(to_path.begin());
		for (filesystem::path::const_iterator to_end(to_path.end()), from_end(from_path.end());
			(iter_from != from_end) && (iter_to != to_end) && (*iter_from == *iter_to);
			++ iter_from, ++ iter_to);

		filesystem::path ret;
		for (filesystem::path::const_iterator from_end(from_path.end()); iter_from != from_end; ++ iter_from)
		{
			if (*iter_from != ".")
			{
				ret /= "..";
			}
		}

		for (; iter_to != to_path.end(); ++ iter_to)
		{
			ret /= *iter_to;
		}
		return ret;
	}

	std::string const JIT_EXT_NAME = ".model_bin";

	template <int N>
	void ExtractFVector(std::string const & value_str, float* v)
	{
		std::vector<std::string> strs;
		boost::algorithm::split(strs, value_str, boost::is_any_of(" "));
		for (size_t i = 0; i < N; ++ i)
		{
			if (i < strs.size())
			{
				boost::algorithm::trim(strs[i]);
				v[i] = static_cast<float>(atof(strs[i].c_str()));
			}
			else
			{
				v[i] = 0;
			}
		}
	}

	template <int N>
	void ExtractUIVector(std::string const & value_str, uint32_t* v)
	{
		std::vector<std::string> strs;
		boost::algorithm::split(strs, value_str, boost::is_any_of(" "));
		for (size_t i = 0; i < N; ++ i)
		{
			if (i < strs.size())
			{
				boost::algorithm::trim(strs[i]);
				v[i] = static_cast<uint32_t>(atoi(strs[i].c_str()));
			}
			else
			{
				v[i] = 0;
			}
		}
	}

	void CompileMaterialsChunk(XMLNodePtr const & materials_chunk, std::vector<OfflineRenderMaterial>& mtls)
	{
		uint32_t mtl_index = 0;
		for (XMLNodePtr mtl_node = materials_chunk->FirstNode("material"); mtl_node; mtl_node = mtl_node->NextSibling("material"), ++ mtl_index)
		{
			OfflineRenderMaterial offline_mtl;
			auto& mtl = offline_mtl.material;

			mtl.name = "Material " + boost::lexical_cast<std::string>(mtl_index);

			mtl.albedo = float4(0, 0, 0, 1);
			mtl.metalness = 0;
			mtl.glossiness = 0;
			mtl.emissive = float3(0, 0, 0);
			mtl.transparent = false;
			mtl.alpha_test = 0;
			mtl.sss = false;
			mtl.two_sided = false;

			mtl.detail_mode = RenderMaterial::SDM_Parallax;
			mtl.height_offset_scale = float2(-0.5f, 0.06f);
			mtl.tess_factors = float4(5, 5, 1, 9);

			{
				XMLAttributePtr attr = mtl_node->Attrib("name");
				if (attr)
				{
					mtl.name = attr->ValueString();
				}
			}

			XMLNodePtr albedo_node = mtl_node->FirstNode("albedo");
			if (albedo_node)
			{
				XMLAttributePtr attr = albedo_node->Attrib("color");
				if (attr)
				{
					ExtractFVector<4>(attr->ValueString(), &mtl.albedo[0]);
				}
				attr = albedo_node->Attrib("texture");
				if (attr)
				{
					offline_mtl.texture_slots.emplace_back("Albedo", attr->ValueString());
				}
			}
			else
			{
				XMLAttributePtr attr = mtl_node->Attrib("diffuse");
				if (attr)
				{
					ExtractFVector<3>(attr->ValueString(), &mtl.albedo[0]);
				}
				else
				{
					attr = mtl_node->Attrib("diffuse_r");
					if (attr)
					{
						mtl.albedo.x() = attr->ValueFloat();
					}
					attr = mtl_node->Attrib("diffuse_g");
					if (attr)
					{
						mtl.albedo.y() = attr->ValueFloat();
					}
					attr = mtl_node->Attrib("diffuse_b");
					if (attr)
					{
						mtl.albedo.z() = attr->ValueFloat();
					}
				}

				attr = mtl_node->Attrib("opacity");
				if (attr)
				{
					mtl.albedo.w() = mtl_node->Attrib("opacity")->ValueFloat();
				}
			}

			XMLNodePtr metalness_node = mtl_node->FirstNode("metalness");
			if (metalness_node)
			{
				XMLAttributePtr attr = metalness_node->Attrib("value");
				if (attr)
				{
					mtl.metalness = attr->ValueFloat();
				}
				attr = metalness_node->Attrib("texture");
				if (attr)
				{
					offline_mtl.texture_slots.emplace_back("Metalness", attr->ValueString());
				}
			}

			XMLNodePtr glossiness_node = mtl_node->FirstNode("glossiness");
			if (glossiness_node)
			{
				XMLAttributePtr attr = glossiness_node->Attrib("value");
				if (attr)
				{
					mtl.glossiness = attr->ValueFloat();
				}
				attr = glossiness_node->Attrib("texture");
				if (attr)
				{
					offline_mtl.texture_slots.emplace_back("Glossiness", attr->ValueString());
				}
			}
			else
			{
				XMLAttributePtr attr = mtl_node->Attrib("shininess");
				if (attr)
				{
					float shininess = mtl_node->Attrib("shininess")->ValueFloat();
					shininess = MathLib::clamp(shininess, 1.0f, MAX_SHININESS);
					mtl.glossiness = Shininess2Glossiness(shininess);
				}
			}

			XMLNodePtr emissive_node = mtl_node->FirstNode("emissive");
			if (emissive_node)
			{
				XMLAttributePtr attr = emissive_node->Attrib("color");
				if (attr)
				{
					ExtractFVector<3>(attr->ValueString(), &mtl.emissive[0]);
				}
				attr = emissive_node->Attrib("texture");
				if (attr)
				{
					offline_mtl.texture_slots.emplace_back("Emissive", attr->ValueString());
				}
			}
			else
			{
				XMLAttributePtr attr = mtl_node->Attrib("emit");
				if (attr)
				{
					ExtractFVector<3>(attr->ValueString(), &mtl.emissive[0]);
				}
				else
				{
					attr = mtl_node->Attrib("emit_r");
					if (attr)
					{
						mtl.emissive.x() = attr->ValueFloat();
					}
					attr = mtl_node->Attrib("emit_g");
					if (attr)
					{
						mtl.emissive.y() = attr->ValueFloat();
					}
					attr = mtl_node->Attrib("emit_b");
					if (attr)
					{
						mtl.emissive.z() = attr->ValueFloat();
					}
				}
			}

			XMLNodePtr bump_node = mtl_node->FirstNode("bump");
			if (bump_node)
			{
				XMLAttributePtr attr = bump_node->Attrib("texture");
				if (attr)
				{
					offline_mtl.texture_slots.emplace_back("Bump", attr->ValueString());
				}
			}
			
			XMLNodePtr normal_node = mtl_node->FirstNode("normal");
			if (normal_node)
			{
				XMLAttributePtr attr = normal_node->Attrib("texture");
				if (attr)
				{
					offline_mtl.texture_slots.emplace_back("Normal", attr->ValueString());
				}
			}

			XMLNodePtr height_node = mtl_node->FirstNode("height");
			if (height_node)
			{
				XMLAttributePtr attr = height_node->Attrib("texture");
				if (attr)
				{
					offline_mtl.texture_slots.emplace_back("Height", attr->ValueString());
				}

				attr = height_node->Attrib("offset");
				if (attr)
				{
					mtl.height_offset_scale.x() = attr->ValueFloat();
				}

				attr = height_node->Attrib("scale");
				if (attr)
				{
					mtl.height_offset_scale.y() = attr->ValueFloat();
				}
			}

			XMLNodePtr detail_node = mtl_node->FirstNode("detail");
			if (detail_node)
			{
				XMLAttributePtr attr = detail_node->Attrib("mode");
				if (attr)
				{
					std::string const & mode_str = attr->ValueString();
					size_t const mode_hash = RT_HASH(mode_str.c_str());
					if (CT_HASH("Flat Tessellation") == mode_hash)
					{
						mtl.detail_mode = RenderMaterial::SDM_FlatTessellation;
					}
					else if (CT_HASH("Smooth Tessellation") == mode_hash)
					{
						mtl.detail_mode = RenderMaterial::SDM_SmoothTessellation;
					}
				}

				attr = detail_node->Attrib("height_offset");
				if (attr)
				{
					mtl.height_offset_scale.x() = attr->ValueFloat();
				}

				attr = detail_node->Attrib("height_scale");
				if (attr)
				{
					mtl.height_offset_scale.y() = attr->ValueFloat();
				}

				XMLNodePtr tess_node = detail_node->FirstNode("tess");
				if (tess_node)
				{
					attr = tess_node->Attrib("edge_hint");
					if (attr)
					{
						mtl.tess_factors.x() = attr->ValueFloat();
					}
					attr = tess_node->Attrib("inside_hint");
					if (attr)
					{
						mtl.tess_factors.y() = attr->ValueFloat();
					}
					attr = tess_node->Attrib("min");
					if (attr)
					{
						mtl.tess_factors.z() = attr->ValueFloat();
					}
					attr = tess_node->Attrib("max");
					if (attr)
					{
						mtl.tess_factors.w() = attr->ValueFloat();
					}
				}
				else
				{
					attr = detail_node->Attrib("edge_tess_hint");
					if (attr)
					{
						mtl.tess_factors.x() = attr->ValueFloat();
					}
					attr = detail_node->Attrib("inside_tess_hint");
					if (attr)
					{
						mtl.tess_factors.y() = attr->ValueFloat();
					}
					attr = detail_node->Attrib("min_tess");
					if (attr)
					{
						mtl.tess_factors.z() = attr->ValueFloat();
					}
					attr = detail_node->Attrib("max_tess");
					if (attr)
					{
						mtl.tess_factors.w() = attr->ValueFloat();
					}
				}
			}

			XMLNodePtr transparent_node = mtl_node->FirstNode("transparent");
			if (transparent_node)
			{
				XMLAttributePtr attr = transparent_node->Attrib("value");
				if (attr)
				{
					mtl.transparent = attr->ValueInt() ? true : false;
				}
			}

			XMLNodePtr alpha_test_node = mtl_node->FirstNode("alpha_test");
			if (alpha_test_node)
			{
				XMLAttributePtr attr = alpha_test_node->Attrib("value");
				if (attr)
				{
					mtl.alpha_test = attr->ValueFloat();
				}
			}

			XMLNodePtr sss_node = mtl_node->FirstNode("sss");
			if (sss_node)
			{
				XMLAttributePtr attr = sss_node->Attrib("value");
				if (attr)
				{
					mtl.sss = attr->ValueInt() ? true : false;
				}
			}
			else
			{
				XMLAttributePtr attr = mtl_node->Attrib("sss");
				if (attr)
				{
					mtl.sss = attr->ValueInt() ? true : false;
				}
			}

			XMLNodePtr two_sided_node = mtl_node->FirstNode("two_sided");
			if (two_sided_node)
			{
				XMLAttributePtr attr = two_sided_node->Attrib("value");
				if (attr)
				{
					mtl.two_sided = attr->ValueInt() ? true : false;
				}
			}

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
					offline_mtl.texture_slots.emplace_back(tex_node->Attrib("type")->ValueString(),
						tex_node->Attrib("name")->ValueString());
				}
			}

			mtls.push_back(offline_mtl);
		}
	}

	void CompileMeshBoundingBox(XMLNodePtr const & mesh_node,
		AABBox& pos_bb, AABBox& tc_bb,
		bool& recompute_pos_bb, bool& recompute_tc_bb)
	{
		XMLNodePtr pos_bb_node = mesh_node->FirstNode("pos_bb");
		if (pos_bb_node)
		{
			float3 pos_min_bb, pos_max_bb;
			{
				XMLAttributePtr attr = pos_bb_node->Attrib("min");
				if (attr)
				{
					ExtractFVector<3>(attr->ValueString(), &pos_min_bb[0]);
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
					ExtractFVector<3>(attr->ValueString(), &pos_max_bb[0]);
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

		XMLNodePtr tc_bb_node = mesh_node->FirstNode("tc_bb");
		if (tc_bb_node)
		{
			float3 tc_min_bb, tc_max_bb;
			{
				XMLAttributePtr attr = tc_bb_node->Attrib("min");
				if (attr)
				{
					ExtractFVector<2>(attr->ValueString(), &tc_min_bb[0]);
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
					ExtractFVector<2>(attr->ValueString(), &tc_max_bb[0]);
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
	}

	void CompileMeshesVerticesChunk(XMLNodePtr const & vertices_chunk,
		AABBox& pos_bb, AABBox& tc_bb, bool recompute_pos_bb, bool recompute_tc_bb,
		std::vector<VertexElement>& vertex_elements,
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
					ExtractFVector<3>(vertex_node->Attrib("v")->ValueString(), &pos[0]);
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
					ExtractFVector<4>(attr->ValueString(), &diffuse[0]);
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
					ExtractFVector<3>(attr->ValueString(), &specular[0]);
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
						ExtractFVector<2>(tex_coord_node->Attrib("v")->ValueString(), &tex_coord[0]);
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
					XMLAttributePtr weight_attr = weight_node->Attrib("weight");

					std::vector<std::string> index_strs;
					std::vector<std::string> weight_strs;
					boost::algorithm::split(index_strs, attr->ValueString(), boost::is_any_of(" "));
					boost::algorithm::split(weight_strs, weight_attr->ValueString(), boost::is_any_of(" "));
					
					for (num_blend = 0; num_blend < 4; ++ num_blend)
					{
						if ((num_blend < index_strs.size()) && (num_blend < weight_strs.size()))
						{
							bone_index32[num_blend] = static_cast<uint32_t>(atoi(index_strs[num_blend].c_str()));
							bone_weight32[num_blend] = static_cast<float>(atof(weight_strs[num_blend].c_str()));
						}
						else
						{
							break;
						}
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
					ExtractFVector<3>(attr->ValueString(), &normal[0]);
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
					ExtractFVector<4>(attr->ValueString(), &tangent[0]);
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
					ExtractFVector<3>(attr->ValueString(), &binormal[0]);
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
				XMLAttributePtr const & attr = tangent_quat_node->Attrib("v");
				if (attr)
				{
					ExtractFVector<4>(attr->ValueString(), &tangent_quat[0]);
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
			VertexElement ve;

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
					ve.format = EF_ABGR8;
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
			uint32_t compact = MathLib::clamp<uint32_t>(static_cast<uint32_t>(normal.x() * 255), 0, 255)
				| (MathLib::clamp<uint32_t>(static_cast<uint32_t>(normal.y() * 255), 0, 255) << 8)
				| (MathLib::clamp<uint32_t>(static_cast<uint32_t>(normal.z() * 255), 0, 255) << 16);
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
			uint32_t ind[3];
			XMLAttributePtr attr = tri_node->Attrib("index");
			if (attr)
			{
				ExtractUIVector<3>(attr->ValueString(), &ind[0]);
			}
			else
			{
				ind[0] = tri_node->Attrib("a")->ValueUInt();
				ind[1] = tri_node->Attrib("b")->ValueUInt();
				ind[2] = tri_node->Attrib("c")->ValueUInt();
			}
			mesh_triangle_indices.push_back(ind[0]);
			mesh_triangle_indices.push_back(ind[1]);
			mesh_triangle_indices.push_back(ind[2]);

			if ((ind[0] > 0xFFFF) || (ind[1] > 0xFFFF) || (ind[2] > 0xFFFF))
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
			std::memcpy(&triangle_indices[0], &mesh_triangle_indices[0], triangle_indices.size());
		}
	}

	void AppendMeshVertices(std::vector<VertexElement> const & ves,
		std::vector<int16_t> const & positions, std::vector<uint32_t> const & normals,
		std::vector<uint32_t> const & tangent_quats, 
		std::vector<uint32_t> const & diffuses, std::vector<uint32_t> const & speculars,
		std::vector<int16_t> const & tex_coords, 
		std::vector<uint32_t> const & bone_indices, std::vector<uint32_t> const & bone_weights,
		std::vector<uint32_t>& mesh_num_vertices,
		std::vector<uint32_t>& mesh_base_vertices,
		std::vector<VertexElement>& merged_ves,
		std::vector<std::vector<uint8_t>>& merged_vertices)
	{
		uint32_t num_vertices = static_cast<uint32_t>(positions.size() / 4);
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
								Native2LE(positions[vert_index * 4 + 0]),
								Native2LE(positions[vert_index * 4 + 1]),
								Native2LE(positions[vert_index * 4 + 2]),
								Native2LE(positions[vert_index * 4 + 3])
							};
							std::memcpy(&merged_vertices[buf_index][(base_vertices + vert_index) * merged_ves[buf_index].element_size()],
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
							uint32_t compact = Native2LE(diffuses[vert_index]);
							std::memcpy(&merged_vertices[buf_index][(base_vertices + vert_index) * merged_ves[buf_index].element_size()],
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
							uint32_t compact = Native2LE(speculars[vert_index]);
							std::memcpy(&merged_vertices[buf_index][(base_vertices + vert_index) * merged_ves[buf_index].element_size()],
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
							uint32_t compact = Native2LE(bone_indices[vert_index]);
							std::memcpy(&merged_vertices[buf_index][(base_vertices + vert_index) * merged_ves[buf_index].element_size()],
								&compact, sizeof(compact));
							break;
						}
					}
					for (size_t i = 0; i < ves.size(); ++ i)
					{
						if (VEU_BlendWeight == ves[i].usage)
						{
							uint32_t buf_index = ves_mapping[i];
							uint32_t compact = Native2LE(bone_weights[vert_index]);
							std::memcpy(&merged_vertices[buf_index][(base_vertices + vert_index) * merged_ves[buf_index].element_size()],
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
								Native2LE(tex_coords[vert_index * 2 + 0]),
								Native2LE(tex_coords[vert_index * 2 + 1])
							};
							std::memcpy(&merged_vertices[buf_index][(base_vertices + vert_index) * merged_ves[buf_index].element_size()],
								&s_tc, sizeof(s_tc));
							break;
						}
					}
				}

				if (tangent_quats.empty())
				{
					if (!normals.empty())
					{
						for (size_t i = 0; i < ves.size(); ++ i)
						{
							if (VEU_Normal == ves[i].usage)
							{
								uint32_t buf_index = ves_mapping[i];
								uint32_t compact = Native2LE(normals[vert_index]);
								std::memcpy(&merged_vertices[buf_index][(base_vertices + vert_index) * merged_ves[buf_index].element_size()],
									&compact, sizeof(compact));
								break;
							}
						}
					}
				}
				else
				{
					for (size_t i = 0; i < ves.size(); ++ i)
					{
						if (VEU_Tangent == ves[i].usage)
						{
							uint32_t buf_index = ves_mapping[i];
							uint32_t compact = Native2LE(tangent_quats[vert_index]);
							std::memcpy(&merged_vertices[buf_index][(base_vertices + vert_index) * merged_ves[buf_index].element_size()],
								&compact, sizeof(compact));
							break;
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

		uint32_t num_indices = static_cast<uint32_t>(triangle_indices.size() / (is_index_16s ? 2 : 4));
		uint32_t start_indicees = mesh_start_indices.back();
		mesh_num_indices.push_back(num_indices);
		mesh_start_indices.push_back(start_indicees + num_indices);

		merged_indices.resize(merged_indices.size() + num_indices * 4);

		for (uint32_t ind_index = 0; ind_index < num_indices; ++ ind_index)
		{
			if (is_index_16s)
			{
				uint32_t ind32 = *reinterpret_cast<uint16_t const *>(&triangle_indices[ind_index * sizeof(uint16_t)]);
				std::memcpy(&merged_indices[(start_indicees + ind_index) * 4],
					&ind32, sizeof(ind32));
			}
			else
			{
				std::memcpy(&merged_indices[(start_indicees + ind_index) * 4],
					&triangle_indices[ind_index * sizeof(uint32_t)], sizeof(uint32_t));
			}
		}
	}

	void CompileMeshLodChunk(XMLNodePtr const & lod_node, uint32_t mesh_index,
		std::vector<AABBox>& pos_bbs, std::vector<AABBox>& tc_bbs, bool recompute_pos_bb, bool recompute_tc_bb,
		std::vector<uint32_t>& mesh_num_vertices, std::vector<uint32_t>& mesh_base_vertices,
		std::vector<uint32_t>& mesh_num_indices, std::vector<uint32_t>& mesh_start_indices,
		std::vector<VertexElement>& merged_ves, std::vector<std::vector<uint8_t>>& merged_vertices,
		std::vector<uint8_t>& merged_indices, char& is_index_16_bit)
	{
		std::vector<VertexElement> ves;
		std::vector<int16_t> positions;
		std::vector<uint32_t> normals;
		std::vector<uint32_t> tangent_quats;
		std::vector<uint32_t> diffuses;
		std::vector<uint32_t> speculars;
		std::vector<int16_t> tex_coords;
		std::vector<uint32_t> bone_indices;
		std::vector<uint32_t> bone_weights;

		XMLNodePtr vertices_chunk = lod_node->FirstNode("vertices_chunk");
		if (vertices_chunk)
		{
			CompileMeshesVerticesChunk(vertices_chunk,
				pos_bbs[mesh_index], tc_bbs[mesh_index], recompute_pos_bb, recompute_tc_bb,
				ves,
				positions, normals, tangent_quats,
				diffuses, speculars, tex_coords,
				bone_indices, bone_weights);
			AppendMeshVertices(ves,
				positions, normals, tangent_quats,
				diffuses, speculars, tex_coords,
				bone_indices, bone_weights,
				mesh_num_vertices, mesh_base_vertices,
				merged_ves, merged_vertices);
		}

		std::vector<uint8_t> triangle_indices;

		XMLNodePtr triangles_chunk = lod_node->FirstNode("triangles_chunk");
		if (triangles_chunk)
		{
			char is_index_16s = true;
			CompileMeshesTrianglesChunk(triangles_chunk,
				triangle_indices, is_index_16s);
			AppendMeshIndices(triangle_indices, is_index_16s,
				mesh_num_indices, mesh_start_indices, merged_indices,
				is_index_16_bit);
		}
	}

	void CompileMeshesChunk(XMLNodePtr const & meshes_chunk,
		std::vector<std::string>& mesh_names, std::vector<int32_t>& mtl_ids, std::vector<uint32_t>& mesh_lods,
		std::vector<AABBox>& pos_bbs, std::vector<AABBox>& tc_bbs, 
		std::vector<uint32_t>& mesh_num_vertices, std::vector<uint32_t>& mesh_base_vertices,
		std::vector<uint32_t>& mesh_num_indices, std::vector<uint32_t>& mesh_start_indices,
		std::vector<VertexElement>& merged_ves, std::vector<std::vector<uint8_t>>& merged_vertices,
		std::vector<uint8_t>& merged_indices, char& is_index_16_bit)
	{
		mesh_names.clear();
		mtl_ids.clear();
		mesh_lods.clear();

		mesh_num_vertices.clear();
		mesh_num_indices.clear();
		mesh_base_vertices.assign(1, 0);
		mesh_start_indices.assign(1, 0);
		merged_ves.clear();
		merged_vertices.clear();
		merged_indices.clear();
		is_index_16_bit = true;

		uint32_t mesh_index = 0;
		for (XMLNodePtr mesh_node = meshes_chunk->FirstNode("mesh"); mesh_node; mesh_node = mesh_node->NextSibling("mesh"), ++ mesh_index)
		{
			mesh_names.push_back(mesh_node->Attrib("name")->ValueString());
			mtl_ids.push_back(mesh_node->Attrib("mtl_id")->ValueInt());

			pos_bbs.resize(mesh_index + 1);
			tc_bbs.resize(pos_bbs.size());

			bool recompute_pos_bb, recompute_tc_bb;
			CompileMeshBoundingBox(mesh_node, pos_bbs[mesh_index], tc_bbs[mesh_index], recompute_pos_bb, recompute_tc_bb);
			if (recompute_pos_bb && recompute_tc_bb)
			{
				XMLNodePtr vertices_chunk = mesh_node->FirstNode("vertices_chunk");
				if (vertices_chunk)
				{
					CompileMeshBoundingBox(vertices_chunk, pos_bbs[mesh_index], tc_bbs[mesh_index], recompute_pos_bb, recompute_tc_bb);
				}
			}

			uint32_t mesh_lod;

			XMLNodePtr lod_node = mesh_node->FirstNode("lod");
			if (lod_node)
			{
				mesh_lod = 0;

				for (; lod_node; lod_node = lod_node->NextSibling("lod"))
				{
					++ mesh_lod;
				}

				std::vector<XMLNodePtr> lod_nodes(mesh_lod);
				for (lod_node = mesh_node->FirstNode("lod"); lod_node; lod_node = lod_node->NextSibling("lod"))
				{
					uint32_t const lod = lod_node->Attrib("value")->ValueUInt();
					lod_nodes[lod] = lod_node;
				}

				for (uint32_t lod = 0; lod < mesh_lod; ++ lod)
				{
					CompileMeshLodChunk(lod_nodes[lod], mesh_index,
						pos_bbs, tc_bbs, recompute_pos_bb, recompute_tc_bb,
						mesh_num_vertices, mesh_base_vertices,
						mesh_num_indices, mesh_start_indices,
						merged_ves, merged_vertices,
						merged_indices, is_index_16_bit);

					recompute_pos_bb = false;
					recompute_tc_bb = false;
				}
			}
			else
			{
				mesh_lod = 1;
				CompileMeshLodChunk(mesh_node, mesh_index,
					pos_bbs, tc_bbs, recompute_pos_bb, recompute_tc_bb,
					mesh_num_vertices, mesh_base_vertices,
					mesh_num_indices, mesh_start_indices,
					merged_ves, merged_vertices,
					merged_indices, is_index_16_bit);
			}

			mesh_lods.push_back(mesh_lod);
		}

		if (is_index_16_bit)
		{
			std::vector<uint8_t> merged_indices_16(merged_indices.size() / 2);
			for (uint32_t ind_index = 0; ind_index < mesh_start_indices.back(); ++ ind_index)
			{
				uint16_t ind16 = Native2LE(static_cast<uint16_t>(*reinterpret_cast<uint32_t*>(&merged_indices[ind_index * sizeof(uint32_t)])));
				std::memcpy(&merged_indices_16[ind_index * sizeof(uint16_t)], &ind16, sizeof(ind16));
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
				joint.bind_scale = scale;
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
					ExtractFVector<4>(attr->ValueString(), &joint.bind_real[0]);
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
					ExtractFVector<4>(attr->ValueString(), &joint.bind_dual[0]);
				}
				else
				{
					joint.bind_dual.x() = bind_dual_node->Attrib("x")->ValueFloat();
					joint.bind_dual.y() = bind_dual_node->Attrib("y")->ValueFloat();
					joint.bind_dual.z() = bind_dual_node->Attrib("z")->ValueFloat();
					joint.bind_dual.w() = bind_dual_node->Attrib("w")->ValueFloat();
				}

				joint.bind_scale = MathLib::length(joint.bind_real);
				joint.bind_real /= joint.bind_scale;
				if (MathLib::SignBit(joint.bind_real.w()) < 0)
				{
					joint.bind_real = -joint.bind_real;
					joint.bind_scale = -joint.bind_scale;
				}
			}

			joints.push_back(joint);
		}
	}

	void CompileKeyFramesChunk(XMLNodePtr const & key_frames_chunk,
		uint32_t& num_frames, uint32_t& frame_rate,
		KeyFramesType& kfss)
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

		uint32_t joint_id = 0;
		for (XMLNodePtr kf_node = key_frames_chunk->FirstNode("key_frame"); kf_node; kf_node = kf_node->NextSibling("key_frame"))
		{
			XMLAttributePtr joint_attr = kf_node->Attrib("joint");
			if (joint_attr)
			{
				joint_id = joint_attr->ValueUInt();
			}
			else
			{
				++ joint_id;
			}
			KeyFrames& kfs = kfss[joint_id];

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
						ExtractFVector<4>(attr->ValueString(), &bind_real[0]);
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
						ExtractFVector<4>(attr->ValueString(), &bind_dual[0]);
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
					if (MathLib::SignBit(bind_real.w()) < 0)
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
						ExtractFVector<3>(attr->ValueString(), &bb_min[0]);
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
						ExtractFVector<3>(attr->ValueString(), &bb_max[0]);
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

	void ConvertTextures(std::string const & output_name, std::vector<OfflineRenderMaterial>& mtls, std::string const & platform)
	{
		std::map<filesystem::path, std::vector<std::pair<size_t, size_t>>> all_texture_slots;
		for (size_t i = 0; i < mtls.size(); ++ i)
		{
			for (size_t j = 0; j < mtls[i].texture_slots.size(); ++ j)
			{
				all_texture_slots[filesystem::path(mtls[i].texture_slots[j].second)].emplace_back(i, j);
			}
		}

		std::vector<std::pair<filesystem::path, std::string>> deploy_files;
		for (auto const & slot : all_texture_slots)
		{
			std::string ext_name = slot.first.extension().string();
			if (ext_name != ".dds")
			{
				std::string cmd = "texconv -f A8B8G8R8 -ft DDS -m 1 \"" + slot.first.string() + "\"";
				system(cmd.c_str());

				std::string tex_base = (slot.first.parent_path() / slot.first.stem()).string();
				deploy_files.emplace_back(filesystem::path(tex_base + ".dds"),
					mtls[slot.second[0].first].texture_slots[slot.second[0].second].first);
			}
		}

		std::vector<std::pair<filesystem::path, filesystem::path>> dup_files;
		std::map<filesystem::path, std::vector<std::pair<size_t, size_t>>> augmented_texture_slots;
		for (auto const & slot : all_texture_slots)
		{
			std::string tex_base = (slot.first.parent_path() / slot.first.stem()).string();
			augmented_texture_slots[filesystem::path(tex_base + ".dds")].push_back(slot.second[0]);

			for (size_t i = 1; i < slot.second.size(); ++ i)
			{
				std::pair<size_t, size_t> const & slot_index = slot.second[i];
				std::string const & type = mtls[slot_index.first].texture_slots[slot_index.second].first;
				if (type != mtls[slot.second[0].first].texture_slots[slot.second[0].second].first)
				{
					filesystem::path new_name(filesystem::path(tex_base + "_" + type + ".dds"));
					if (filesystem::exists(new_name))
					{
						size_t j = 0;
						do
						{
							std::stringstream ss;
							ss << tex_base << "_" << type << "_" << j << ".dds";
							new_name = filesystem::path(ss.str());
							++ j;
						} while (filesystem::exists(new_name));
					}

					if (augmented_texture_slots.find(new_name) == augmented_texture_slots.end())
					{
						dup_files.emplace_back(filesystem::path(tex_base + ".dds"), new_name);
						deploy_files.emplace_back(new_name, type);
					}
					augmented_texture_slots[new_name].push_back(slot_index);
				}
			}
		}

		for (auto const & dup : dup_files)
		{
			filesystem::copy_file(std::get<0>(dup), std::get<1>(dup));
		}

		for (auto const & df : deploy_files)
		{
			std::string deploy_type;
			size_t const type_hash = RT_HASH(df.second.c_str());
			if ((CT_HASH("Color") == type_hash) || (CT_HASH("Diffuse Color") == type_hash)
				|| (CT_HASH("Diffuse Color Map") == type_hash)
				|| (CT_HASH("Albedo") == type_hash))
			{
				deploy_type = "albedo";
			}
			else if (CT_HASH("Metalness") == type_hash)
			{
				deploy_type = "metalness";
			}
			else if ((CT_HASH("Glossiness") == type_hash) || (CT_HASH("Reflection Glossiness Map") == type_hash))
			{
				deploy_type = "glossiness";
			}
			else if ((CT_HASH("Self-Illumination") == type_hash) || (CT_HASH("Emissive") == type_hash))
			{
				deploy_type = "emissive";
			}
			else if ((CT_HASH("Bump") == type_hash) || (CT_HASH("Bump Map") == type_hash))
			{
				deploy_type = "bump";
			}
			else if ((CT_HASH("Normal") == type_hash) || (CT_HASH("Normal Map") == type_hash))
			{
				deploy_type = "normal";
			}
			else if ((CT_HASH("Height") == type_hash) || (CT_HASH("Height Map") == type_hash))
			{
				deploy_type = "height";
			}
			else
			{
				// TODO
				deploy_type = df.second;
			}

			cout << "Processing " << df.first.string() << endl;

			std::string cmd = "platformdeployer -P " + platform + " -I \"" + df.first.string() + "\" -T " + deploy_type;
			system(cmd.c_str());
		}

		filesystem::path output_folder = filesystem::path(output_name).parent_path();
		for (auto const & slot : augmented_texture_slots)
		{
			std::string rel_path = make_relative(output_folder, slot.first).string();

			for (auto const & slot_index : slot.second)
			{
				mtls[slot_index.first].texture_slots[slot_index.second].second = rel_path;
			}
		}
	}

	std::string ReplaceExtToDDS(std::string const & name)
	{
		std::string ret;
		size_t dot_pos = name.find_last_of('.');
		if (dot_pos != std::string::npos)
		{
			std::string base_name = name.substr(0, dot_pos);
			ret = base_name + ".dds";
		}
		else
		{
			ret = name;
		}
		return ret;
	}

	void MeshMLJIT(std::string const & meshml_name, std::string const & output_name, std::string const & platform)
	{
		ResIdentifierPtr file = ResLoader::Instance().Open(meshml_name);
		KlayGE::XMLDocument doc;
		XMLNodePtr root = doc.Parse(file);

		BOOST_ASSERT(root->Attrib("version") && (root->Attrib("version")->ValueInt() >= 1));

		XMLNodePtr materials_chunk = root->FirstNode("materials_chunk");
		std::vector<OfflineRenderMaterial> mtls;
		if (materials_chunk)
		{
			CompileMaterialsChunk(materials_chunk, mtls);
			if (!platform.empty())
			{
				ConvertTextures(output_name, mtls, platform);
			}

			for (size_t i = 0; i < mtls.size(); ++ i)
			{
				for (size_t j = 0; j < mtls[i].texture_slots.size(); ++ j)
				{
					size_t const type_hash = RT_HASH(mtls[i].texture_slots[j].first.c_str());
					if ((CT_HASH("Color") == type_hash) || (CT_HASH("Diffuse Color") == type_hash)
						|| (CT_HASH("Diffuse Color Map") == type_hash)
						|| (CT_HASH("Albedo") == type_hash))
					{
						mtls[i].material.tex_names[RenderMaterial::TS_Albedo] = ReplaceExtToDDS(mtls[i].texture_slots[j].second);
					}
					else if (CT_HASH("Metalness") == type_hash)
					{
						mtls[i].material.tex_names[RenderMaterial::TS_Metalness] = ReplaceExtToDDS(mtls[i].texture_slots[j].second);
					}
					else if ((CT_HASH("Glossiness") == type_hash) || (CT_HASH("Reflection Glossiness Map") == type_hash))
					{
						mtls[i].material.tex_names[RenderMaterial::TS_Glossiness] = ReplaceExtToDDS(mtls[i].texture_slots[j].second);
					}
					else if ((CT_HASH("Self-Illumination") == type_hash) || (CT_HASH("Emissive") == type_hash))
					{
						mtls[i].material.tex_names[RenderMaterial::TS_Emissive] = ReplaceExtToDDS(mtls[i].texture_slots[j].second);
					}
					else if ((CT_HASH("Bump") == type_hash) || (CT_HASH("Bump Map") == type_hash)
						|| (CT_HASH("Normal") == type_hash) || (CT_HASH("Normal Map") == type_hash))
					{
						mtls[i].material.tex_names[RenderMaterial::TS_Normal] = ReplaceExtToDDS(mtls[i].texture_slots[j].second);
					}
					else if ((CT_HASH("Height") == type_hash) || (CT_HASH("Height Map") == type_hash))
					{
						mtls[i].material.tex_names[RenderMaterial::TS_Height] = ReplaceExtToDDS(mtls[i].texture_slots[j].second);
					}
				}
			}
		}

		XMLNodePtr meshes_chunk = root->FirstNode("meshes_chunk");
		std::vector<std::string> mesh_names;
		std::vector<int32_t> mtl_ids;
		std::vector<uint32_t> mesh_lods;
		std::vector<AABBox> pos_bbs;
		std::vector<AABBox> tc_bbs;
		std::vector<uint32_t> mesh_num_vertices;
		std::vector<uint32_t> mesh_base_vertices;
		std::vector<uint32_t> mesh_num_indices;
		std::vector<uint32_t> mesh_start_indices;
		std::vector<VertexElement> merged_ves;
		std::vector<std::vector<uint8_t>> merged_vertices;
		std::vector<uint8_t> merged_indices;
		char is_index_16_bit = true;
		if (meshes_chunk)
		{
			CompileMeshesChunk(meshes_chunk, mesh_names, mtl_ids, mesh_lods, pos_bbs, tc_bbs,
				mesh_num_vertices, mesh_base_vertices,
				mesh_num_indices, mesh_start_indices,
				merged_ves, merged_vertices, merged_indices,
				is_index_16_bit);
		}

		XMLNodePtr bones_chunk = root->FirstNode("bones_chunk");
		std::vector<Joint> joints;
		if (bones_chunk)
		{
			CompileBonesChunk(bones_chunk, joints);
		}

		XMLNodePtr key_frames_chunk = root->FirstNode("key_frames_chunk");
		uint32_t num_frames = 0;
		uint32_t frame_rate = 0;
		std::vector<KeyFrames> kfs(joints.size());
		std::vector<AABBKeyFrames> bb_kfs;
		if (key_frames_chunk)
		{
			CompileKeyFramesChunk(key_frames_chunk, num_frames, frame_rate, kfs);

			for (size_t i = 0; i < kfs.size(); ++ i)
			{
				if (kfs[i].frame_id.empty())
				{
					Quaternion inv_parent_real;
					Quaternion inv_parent_dual;
					float inv_parent_scale;
					if (joints[i].parent < 0)
					{
						inv_parent_real = Quaternion::Identity();
						inv_parent_dual = Quaternion(0, 0, 0, 0);
						inv_parent_scale = 1;
					}
					else
					{
						std::tie(inv_parent_real, inv_parent_dual)
							= MathLib::inverse(joints[joints[i].parent].bind_real, joints[joints[i].parent].bind_dual);
						inv_parent_scale = 1 / joints[joints[i].parent].bind_scale;
					}

					kfs[i].frame_id.push_back(0);
					kfs[i].bind_real.push_back(MathLib::mul_real(joints[i].bind_real, inv_parent_real));
					kfs[i].bind_dual.push_back(MathLib::mul_dual(joints[i].bind_real, joints[i].bind_dual * inv_parent_scale,
						inv_parent_real, inv_parent_dual));
					kfs[i].bind_scale.push_back(joints[i].bind_scale * inv_parent_scale);
				}
			}

			XMLNodePtr bb_kfs_chunk = root->FirstNode("bb_key_frames_chunk");
			CompileBBKeyFramesChunk(bb_kfs_chunk, pos_bbs, num_frames, bb_kfs);
		}

		XMLNodePtr actions_chunk = root->FirstNode("actions_chunk");
		std::vector<AnimationAction> actions;
		if (actions_chunk)
		{
			CompileActionsChunk(actions_chunk, num_frames, actions);
		}

		std::vector<RenderMaterialPtr> output_mtls(mtls.size());
		for (size_t i = 0; i < mtls.size(); ++ i)
		{
			output_mtls[i] = MakeSharedPtr<RenderMaterial>(mtls[i].material);
		}
		SaveModel(output_name, output_mtls, merged_ves, is_index_16_bit, merged_vertices, merged_indices,
			mesh_names, mtl_ids, mesh_lods, pos_bbs, tc_bbs,
			mesh_num_vertices, mesh_base_vertices, mesh_num_indices, mesh_start_indices,
			joints, MakeSharedPtr<std::vector<AnimationAction>>(actions), MakeSharedPtr<KeyFramesType>(kfs), num_frames, frame_rate);
	}
}

int main(int argc, char* argv[])
{
	std::string input_name;
	filesystem::path target_folder;
	std::string platform;
	bool quiet = false;

	boost::program_options::options_description desc("Allowed options");
	desc.add_options()
		("help,H", "Produce help message")
		("input-name,I", boost::program_options::value<std::string>(), "Input meshml name.")
		("target-folder,T", boost::program_options::value<std::string>(), "Target folder.")
		("platform,P", boost::program_options::value<std::string>()->implicit_value(""), "Platform name.")
		("quiet,q", boost::program_options::value<bool>()->implicit_value(true), "Quiet mode.")
		("version,v", "Version.");

	boost::program_options::variables_map vm;
	boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc), vm);
	boost::program_options::notify(vm);

	if ((argc <= 1) || (vm.count("help") > 0))
	{
		cout << desc << endl;
		return 1;
	}
	if (vm.count("version") > 0)
	{
		cout << "KlayGE MeshMLJIT, Version 1.0.0" << endl;
		return 1;
	}
	if (vm.count("input-name") > 0)
	{
		input_name = vm["input-name"].as<std::string>();
	}
	else
	{
		cout << "Need input meshml name." << endl;
		return 1;
	}
	if (vm.count("target-folder") > 0)
	{
		target_folder = vm["target-folder"].as<std::string>();
	}
	if (vm.count("platform") > 0)
	{
		platform = vm["platform"].as<std::string>();
	}
	if (vm.count("quiet") > 0)
	{
		quiet = vm["quiet"].as<bool>();
	}

	std::string meshml_name = ResLoader::Instance().Locate(input_name);
	if (meshml_name.empty())
	{
		filesystem::path input_path(input_name);
		std::string base_name = input_path.stem().string();
		filesystem::path folder = input_path.parent_path();
		meshml_name = (folder / filesystem::path(base_name)).string() + ".7z//" + base_name + ".meshml";
	}

	std::string::size_type const pkt_offset(meshml_name.find("//"));
	std::string file_name;
	if (pkt_offset != std::string::npos)
	{
		std::string pkt_name = meshml_name.substr(0, pkt_offset);
		std::string::size_type const password_offset = pkt_name.find("|");
		if (password_offset != std::string::npos)
		{
			pkt_name = pkt_name.substr(0, password_offset - 1);
		}

		if (target_folder.empty())
		{
			target_folder = filesystem::path(pkt_name).parent_path();
		}

		file_name = meshml_name.substr(pkt_offset + 2);
	}
	else
	{
		filesystem::path meshml_path(meshml_name);
		if (target_folder.empty())
		{
			target_folder = meshml_path.parent_path();
		}
		file_name = meshml_path.filename().string();
	}

	std::string output_name = (target_folder / filesystem::path(file_name)).string() + JIT_EXT_NAME;

	MeshMLJIT(meshml_name, output_name, platform);

	if (!quiet)
	{
		cout << "Binary model has been saved to " << output_name << "." << endl;
	}

	Context::Destroy();

	return 0;
}
