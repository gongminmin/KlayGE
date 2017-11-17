// mesh_extractor.cpp
// KlayGE 3DSMax网格数据析取类 实现文件
// Ver 3.6.0
// 版权所有(C) 龚敏敏, 2005-2007
// Homepage: http://www.klayge.org
//
// 3.6.0
// 能导出骨骼动画 (2007.6.2)
//
// 3.0.0
// 导出顶点格式 (2005.10.25)
//
// 2.5.0
// 初次建立 (2005.5.1)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#pragma warning(push)
#pragma warning(disable: 4100) // Many unreferended parameters.
#pragma warning(disable: 4238) // Rvalue used as lvalue.
#pragma warning(disable: 4239) // Default argument with type conversion.
#pragma warning(disable: 4244) // Many conversion from int to WORD.
#pragma warning(disable: 4245) // Signed/unsigned conversion.
#pragma warning(disable: 4458) // Declaration of '...' hides class member.
#pragma warning(disable: 4459) // Declaration of '...' hides global declaration.
#pragma warning(disable: 4512) // BitArray::NumberSetProxy and DelayedNodeMat don't have assignment operator.
#include <max.h>
#pragma warning(pop)
#include <modstack.h>
#pragma warning(push)
#pragma warning(disable: 4100) // Many unreferended parameters.
#pragma warning(disable: 4458) // Declaration of '...' hides class member.
#include <stdmat.h>
#pragma warning(pop)
#pragma warning(push)
#pragma warning(disable: 4100) // Many unreferended parameters.
#pragma warning(disable: 4458) // Declaration of '...' hides class member.
#include <iparamb2.h>
#pragma warning(pop)
#pragma warning(push)
#pragma warning(disable: 4100) // Many unreferended parameters.
#if VERSION_3DSMAX >= 7 << 16
#include <CS/phyexp.h>
#else
#include <phyexp.h>
#endif
#pragma warning(pop)
#pragma warning(push)
#pragma warning(disable: 4100) // Many unreferended parameters.
#include <iskin.h>
#pragma warning(pop)

#ifdef base_type
#undef base_type
#endif
#ifdef PI
#undef PI
#endif
#include <KFL/KFL.hpp>
#include <KFL/Hash.hpp>

#include <fstream>
#include <algorithm>
#include <set>
#include <vector>
#include <limits>
#include <functional>

#include <boost/lexical_cast.hpp>

#include "util.hpp"
#include "mesh_extractor.hpp"

namespace
{
	struct vertex_index_t
	{
		int pos_index;
		std::vector<int> tex_indices;
		int sm_index;

		std::vector<size_t> ref_triangle;
	};

	bool operator<(vertex_index_t const & lhs, vertex_index_t const & rhs)
	{
		if (lhs.pos_index < rhs.pos_index)
		{
			return true;
		}
		else
		{
			if (lhs.pos_index > rhs.pos_index)
			{
				return false;
			}
			else
			{
				if (lhs.tex_indices < rhs.tex_indices)
				{
					return true;
				}
				else
				{
					if (lhs.tex_indices > rhs.tex_indices)
					{
						return false;
					}
					else
					{
						if (lhs.sm_index < rhs.sm_index)
						{
							return true;
						}
						else
						{
							return false;
						}
					}
				}
			}
		}
	}

	bool bind_cmp(std::pair<INode*, float> const& lhs, std::pair<INode*, float> const& rhs)
	{
		return lhs.second > rhs.second;
	}

	Point3 compute_normal(Point3 const & v0XYZ, Point3 const & v1XYZ, Point3 const & v2XYZ)
	{
		Point3 v1v0 = v1XYZ - v0XYZ;
		Point3 v2v0 = v2XYZ - v0XYZ;

		return CrossProd(v1v0, v2v0);
	}

	void compute_tangent(Point3& tangent, Point3& binormal, Point3 const & v0XYZ, Point3 const & v1XYZ, Point3 const & v2XYZ,
		Point2 const & v0Tex, Point2 const & v1Tex, Point2 const & v2Tex)
	{
		Point3 v1v0 = v1XYZ - v0XYZ;
		Point3 v2v0 = v2XYZ - v0XYZ;

		float s1 = v1Tex.x - v0Tex.x;
		float t1 = v1Tex.y - v0Tex.y;

		float s2 = v2Tex.x - v0Tex.x;
		float t2 = v2Tex.y - v0Tex.y;

		float denominator = s1 * t2 - s2 * t1;
		if (abs(denominator) < std::numeric_limits<float>::epsilon())
		{
			tangent = Point3(1, 0, 0);
			binormal = Point3(0, 1, 0);
		}
		else
		{
			tangent = (t2 * v1v0 - t1 * v2v0) / denominator;
			binormal = (s1 * v2v0 - s2 * v1v0) / denominator;
		}
	}
}

namespace KlayGE
{
	meshml_extractor::meshml_extractor(INode* root_node, int joints_per_ver, int cur_time, int start_frame, int end_frame, bool combine_meshes)
						: meshml_obj_(static_cast<float>(GetMasterScale(UNITS_METERS))),
							root_node_(root_node),
							joints_per_ver_(joints_per_ver),
							cur_time_(cur_time),
							start_frame_(start_frame), end_frame_(end_frame),
							combine_meshes_(combine_meshes)
	{
		meshml_obj_.NumFrames(end_frame_ - start_frame);
		meshml_obj_.FrameRate(GetFrameRate());
	}

	void meshml_extractor::find_joints(INode* node)
	{
		if (is_bone(node))
		{
			joint_nodes_.emplace(node, Matrix3());

			int joint_id = meshml_obj_.AllocJoint();
			joint_node_to_id_.emplace(node, joint_id);
		}
		for (int i = 0; i < node->NumberOfChildren(); ++ i)
		{
			this->find_joints(node->GetChildNode(i));
		}
	}

	void meshml_extractor::export_objects(std::vector<INode*> const & nodes)
	{
		if (joints_per_ver_ > 0)
		{
			// root bone
			int joint_id = meshml_obj_.AllocJoint();
			meshml_obj_.SetJoint(joint_id, tstr_to_str(root_node_->GetName()), -1, Quaternion(0, 0, 0, 1), Quaternion(0, 0, 0, 0));
			joint_node_to_id_.emplace(root_node_, joint_id);

			int kfs_id = meshml_obj_.AllocKeyframes();
			meshml_obj_.SetKeyframes(kfs_id, joint_id);

			int tpf = GetTicksPerFrame();

			for (int i = start_frame_; i < end_frame_; ++ i)
			{
				int kf_id = meshml_obj_.AllocKeyframe(kfs_id);
				meshml_obj_.SetKeyframe(kfs_id, kf_id, i, this->rh_to_lh(root_node_->GetNodeTM(i * tpf)));
			}

			this->find_joints(root_node_);


			physiques_.clear();
			physique_mods_.clear();
			skins_.clear();
			skin_mods_.clear();
			for (auto const & node : nodes)
			{
				Object* obj_ref = node->GetObjectRef();
				while ((obj_ref != NULL) && (GEN_DERIVOB_CLASS_ID == obj_ref->SuperClassID()))
				{
					IDerivedObject* derived_obj = static_cast<IDerivedObject*>(obj_ref);

					// Iterate over all entries of the modifier stack.
					for (int mod_stack_index = 0; mod_stack_index < derived_obj->NumModifiers(); ++ mod_stack_index)
					{
						Modifier* mod = derived_obj->GetModifier(mod_stack_index);
						if (Class_ID(PHYSIQUE_CLASS_ID_A, PHYSIQUE_CLASS_ID_B) == mod->ClassID())
						{
							IPhysiqueExport* phy_exp = static_cast<IPhysiqueExport*>(mod->GetInterface(I_PHYINTERFACE));
							if (phy_exp != NULL)
							{
								physiques_.push_back(phy_exp);
								physique_mods_.push_back(mod);
							}
						}
						else
						{
							if (SKIN_CLASSID == mod->ClassID())
							{
								ISkin* skin = static_cast<ISkin*>(mod->GetInterface(I_SKIN));
								if (skin != NULL)
								{
									skins_.push_back(skin);
									skin_mods_.push_back(mod);
								}
							}
						}
					}

					obj_ref = derived_obj->GetObjRef();
				}
			}

			this->extract_all_joint_tms();
		}

		std::vector<INode*> jnodes;
		std::vector<INode*> mnodes;

		for (auto const & jn : joint_nodes_)
		{
			if (is_bone(jn.first))
			{
				jnodes.push_back(jn.first);
			}
		}
		for (auto const & node : nodes)
		{
			if (is_bone(node))
			{
				jnodes.push_back(node);
			}
			else
			{
				if (is_mesh(node))
				{
					mnodes.push_back(node);
				}
			}
		}

		std::sort(jnodes.begin(), jnodes.end());
		jnodes.erase(std::unique(jnodes.begin(), jnodes.end()), jnodes.end());
		std::sort(mnodes.begin(), mnodes.end());
		mnodes.erase(std::unique(mnodes.begin(), mnodes.end()), mnodes.end());

		for (auto const & jnode : jnodes)
		{
			this->extract_bone(jnode);
		}
		
		for (auto const & mnode : mnodes)
		{
			this->extract_object(mnode);
		}
	}

	void meshml_extractor::get_material(std::vector<int>& mtls_id, std::vector<std::map<int, std::pair<Matrix3, int>>>& uv_transss, Mtl* max_mtl)
	{
		if (max_mtl != NULL)
		{
			if (0 == max_mtl->NumSubMtls())
			{
				int mtl_id = meshml_obj_.AllocMaterial();
				mtls_id.push_back(mtl_id);

				uv_transss.push_back(std::map<int, std::pair<Matrix3, int>>());
				std::map<int, std::pair<Matrix3, int>>& uv_transs = uv_transss.back();

				char const * name = max_mtl->GetName().data();
				::Color diffuse = max_mtl->GetDiffuse();
				::Color emit;
				if (max_mtl->GetSelfIllumColorOn())
				{
					emit = max_mtl->GetSelfIllumColor();
				}
				else
				{
					emit = max_mtl->GetDiffuse() * max_mtl->GetSelfIllum();
				}
				float opacity = 1 - max_mtl->GetXParency();
				float glossiness = log(max_mtl->GetShininess() * 100) / log(8192.0f);

				meshml_obj_.SetMaterial(mtl_id, name, float4(diffuse.r, diffuse.g, diffuse.b, opacity), 0, glossiness,
					float3(emit.r, emit.g, emit.b), opacity < 1, 0, false, false);

				for (int j = 0; j < max_mtl->NumSubTexmaps(); ++ j)
				{
					Texmap* tex_map = max_mtl->GetSubTexmap(j);
					if ((tex_map != NULL) && (Class_ID(BMTEX_CLASS_ID, 0) == tex_map->ClassID()))
					{
						BitmapTex* bitmap_tex = static_cast<BitmapTex*>(tex_map);
						std::string map_name = tstr_to_str(bitmap_tex->GetMapName());
						if (!map_name.empty())
						{
							Matrix3 uv_mat;
							tex_map->GetUVTransform(uv_mat);

							int tex_u = 0;
							UVGen* uv_gen = tex_map->GetTheUVGen();
							if (uv_gen != NULL)
							{
								int axis = uv_gen->GetAxis();
								switch (axis)
								{
								case AXIS_UV:
									tex_u = 0;
									break;
						
								case AXIS_VW:
									tex_u = 1;
									break;

								case AXIS_WU:
									tex_u = 2;
									break;
								}
							}

							int channel = bitmap_tex->GetMapChannel();
							uv_transs[channel] = std::make_pair(uv_mat, tex_u);

							std::string const slot_name = tstr_to_str(max_mtl->GetSubTexmapSlotName(j).data());
							size_t const slot_name_hash = RT_HASH(slot_name.c_str());
							if ((CT_HASH("Color") == slot_name_hash)
								|| (CT_HASH("Diffuse Color") == slot_name_hash)
								|| (CT_HASH("Diffuse Color Map") == slot_name_hash))
							{
								meshml_obj_.SetTextureSlot(mtl_id, MeshMLObj::Material::TS_Albedo, map_name);
							}
							else if ((CT_HASH("Glossiness") == slot_name_hash)
								|| (CT_HASH("Reflection Glossiness Map") == slot_name_hash))
							{
								meshml_obj_.SetTextureSlot(mtl_id, MeshMLObj::Material::TS_Glossiness, map_name);
							}
							else if (CT_HASH("Self-Illumination") == slot_name_hash)
							{
								meshml_obj_.SetTextureSlot(mtl_id, MeshMLObj::Material::TS_Emissive, map_name);
							}
							else if ((CT_HASH("Bump") == slot_name_hash) || (CT_HASH("Bump Map") == slot_name_hash))
							{
								meshml_obj_.SetTextureSlot(mtl_id, MeshMLObj::Material::TS_Bump, map_name);
							}
							else if ((CT_HASH("Normal") == slot_name_hash) || (CT_HASH("Normal Map") == slot_name_hash))
							{
								meshml_obj_.SetTextureSlot(mtl_id, MeshMLObj::Material::TS_Normal, map_name);
							}
							else if ((CT_HASH("Height") == slot_name_hash) || (CT_HASH("Height Map") == slot_name_hash))
							{
								meshml_obj_.SetTextureSlot(mtl_id, MeshMLObj::Material::TS_Height, map_name);
							}
						}
					}
				}
			}
			else
			{
				for (int i = 0; i < max_mtl->NumSubMtls(); ++ i)
				{
					this->get_material(mtls_id, uv_transss, max_mtl->GetSubMtl(i));
				}
			}
		}
		else
		{
			int mtl_id = meshml_obj_.AllocMaterial();
			mtls_id.push_back(mtl_id);

			uv_transss.push_back(std::map<int, std::pair<Matrix3, int>>());

			meshml_obj_.SetMaterial(mtl_id, "default", float4(0.5f, 0.5f, 0.5f, 1), 0, log(32.0f) / log(8192.0f),
				float3(0, 0, 0), false, 0, false, false);
		}
	}

	void meshml_extractor::extract_object(INode* node)
	{
		assert(is_mesh(node));

		std::string		obj_name;
		vertices_t		obj_vertices;
		triangles_t		obj_triangles;

		obj_name = tstr_to_str(node->GetName());

		Matrix3 obj_matrix = node->GetObjTMAfterWSM(cur_time_);
		bool flip_normals = obj_matrix.Parity() ? true : false;

		std::vector<std::pair<Point3, binds_t>> positions;
		std::map<int, std::vector<Point2>> texs;
		std::vector<int> pos_indices;
		std::map<int, std::vector<int>> tex_indices;
		std::vector<std::map<int, std::pair<Matrix3, int>>> uv_transs;

		size_t mtl_base_index = objs_mtl_id_.size();
		this->get_material(objs_mtl_id_, uv_transs, node->GetMtl());

		std::vector<unsigned int> face_sm_group;
		std::vector<unsigned int> face_mtl_id;
		std::vector<std::vector<std::vector<unsigned int>>> vertex_sm_group;

		Object* obj = node->EvalWorldState(cur_time_).obj;
		if ((obj != NULL) && obj->CanConvertToType(Class_ID(TRIOBJ_CLASS_ID, 0)))
		{
			TriObject* tri = static_cast<TriObject*>(obj->ConvertToType(0, Class_ID(TRIOBJ_CLASS_ID, 0)));
			assert(tri != NULL);

			bool need_delete = false;
			if (obj != tri)
			{
				need_delete = true;
			}

			Mesh& mesh = tri->GetMesh();
			if (mesh.getNumFaces() > 0)
			{
				face_sm_group.resize(mesh.getNumFaces());
				face_mtl_id.resize(mesh.getNumFaces());

				obj_triangles.resize(mesh.getNumFaces());

				for (int channel = 1; channel < MAX_MESHMAPS; ++ channel)
				{
					if (mesh.mapSupport(channel))
					{
						int const num_map_verts = mesh.getNumMapVerts(channel);
						if (num_map_verts > 0)
						{
							texs[channel].resize(num_map_verts, Point2(0.0f, 0.0f));

							Matrix3 tex_mat;
							tex_mat.IdentityMatrix();
							int tex_u = 0;
							for (size_t j = 0; j < uv_transs.size(); ++ j)
							{
								if (uv_transs[j].find(channel) == uv_transs[j].end())
								{
									uv_transs[j][channel] = std::make_pair(tex_mat, tex_u);
								}
							}

							UVVert* uv_verts = mesh.mapVerts(channel);
							TVFace* tv_faces = mesh.mapFaces(channel);
							for (size_t i = 0; i < obj_triangles.size(); ++ i)
							{
								int mtl_id = mesh.getFaceMtlIndex(static_cast<int>(i)) % (objs_mtl_id_.size() - mtl_base_index);

								tex_mat = uv_transs[mtl_id][channel].first;
								tex_u = uv_transs[mtl_id][channel].second;

								for (int j = 2; j >= 0; -- j)
								{
									int ti = tv_faces[i].t[j];
									tex_indices[channel].push_back(ti);

									Point3 uvw;
									// NaN test
									if ((uv_verts[ti].x != uv_verts[ti].x)
										|| (uv_verts[ti].y != uv_verts[ti].y)
										|| (uv_verts[ti].z != uv_verts[ti].z))
									{
										uvw = Point3(0, 0, 0);
									}
									else
									{
										uvw = uv_verts[ti] * tex_mat;
									}

									texs[channel][ti].x = uvw[tex_u];
									texs[channel][ti].y = uvw[(tex_u + 1) % 3];
								}
							}
						}
					}
				}

				for (int i = 0; i < mesh.getNumFaces(); ++ i)
				{
					face_sm_group[i] = mesh.faces[i].getSmGroup();
					face_mtl_id[i] = mesh.faces[i].getMatID();
					if (objs_mtl_id_.size() != mtl_base_index)
					{
						face_mtl_id[i] = static_cast<unsigned int>(mtl_base_index + face_mtl_id[i] % (objs_mtl_id_.size() - mtl_base_index));
					}
					for (int j = 2; j >= 0; -- j)
					{
						pos_indices.push_back(mesh.faces[i].v[j]);
					}
				}

				positions.resize(mesh.getNumVerts());
				vertex_sm_group.resize(mesh.getNumVerts());
				for (int i = 0; i < mesh.getNumVerts(); ++ i)
				{
					positions[i] = std::make_pair(mesh.getVert(i), binds_t());
				}
				for (int i = 0; i < mesh.getNumFaces(); ++ i)
				{
					unsigned int sm = face_sm_group[i];
					for (int j = 2; j >= 0; -- j)
					{
						int index = mesh.faces[i].v[j];
						bool found = false;
						for (size_t k = 0; k < vertex_sm_group[index].size() && !found; ++ k)
						{
							for (size_t l = 0; l < vertex_sm_group[index][k].size(); ++ l)
							{
								if (face_sm_group[vertex_sm_group[index][k][l]] & sm)
								{
									vertex_sm_group[index][k].push_back(i);
									found = true;
									break;
								}
							}
						}

						if (!found)
						{
							vertex_sm_group[index].push_back(std::vector<unsigned int>(1, i));
						}
					}
				}
			}

			if (need_delete)
			{
				delete tri;
			}
		}

		if (!obj_triangles.empty())
		{
			if (tex_indices.empty())
			{
				tex_indices[1] = pos_indices;
				texs[1].resize(positions.size(), Point2(0.0f, 0.0f));
			}

			std::set<vertex_index_t> vertex_indices;
			for (size_t i = 0; i < obj_triangles.size(); ++ i)
			{
				for (size_t j = 0; j < 3; ++ j)
				{
					vertex_index_t vertex_index;

					size_t offset;
					if (!flip_normals)
					{
						offset = i * 3 + j;
					}
					else
					{
						offset = i * 3 + (2 - j);
					}

					vertex_index.pos_index = pos_indices[offset];
					for (auto const & tex_index : tex_indices)
					{
						vertex_index.tex_indices.push_back(tex_index.second[offset]);
					}

					for (size_t k = 0; k < vertex_sm_group[vertex_index.pos_index].size(); ++ k)
					{
						for (size_t l = 0; l < vertex_sm_group[vertex_index.pos_index][k].size(); ++ l)
						{
							if (vertex_sm_group[vertex_index.pos_index][k][l] == static_cast<unsigned int>(i))
							{
								vertex_index.sm_index = static_cast<int>(k);
								break;
							}
						}
					}

					auto v_iter = vertex_indices.find(vertex_index);
					if (v_iter != vertex_indices.end())
					{
						// Respect set Immutability in C++0x
						vertex_index.ref_triangle = v_iter->ref_triangle;
						vertex_indices.erase(v_iter);
					}
					vertex_index.ref_triangle.push_back(i * 3 + j);
					vertex_indices.insert(vertex_index);
				}
			}

			if (joints_per_ver_ > 0)
			{
				Object* obj_ref = node->GetObjectRef();
				while ((obj_ref != NULL) && (GEN_DERIVOB_CLASS_ID == obj_ref->SuperClassID()))
				{
					IDerivedObject* derived_obj = static_cast<IDerivedObject*>(obj_ref);

					// Iterate over all entries of the modifier stack.
					for (int mod_stack_index = 0; mod_stack_index < derived_obj->NumModifiers(); ++ mod_stack_index)
					{
						Modifier* mod = derived_obj->GetModifier(mod_stack_index);
						if (Class_ID(PHYSIQUE_CLASS_ID_A, PHYSIQUE_CLASS_ID_B) == mod->ClassID())
						{
							this->physique_modifier(mod, node, positions);
						}
						else
						{
							if (SKIN_CLASSID == mod->ClassID())
							{
								this->skin_modifier(mod, node, positions);
							}
						}
					}

					obj_ref = derived_obj->GetObjRef();
				}

				Matrix3 tm = node->GetObjTMAfterWSM(0);
				for (auto& pos_binds : positions)
				{
					if (pos_binds.second.empty())
					{
						INode* parent_node = node->GetParentNode();
						while ((parent_node != root_node_) && !is_bone(parent_node))
						{
							parent_node = parent_node->GetParentNode();
						}

						pos_binds.second.emplace_back(parent_node, 1.0f);
					}

					Point3 v0 = pos_binds.first * tm;
					pos_binds.first = Point3(0, 0, 0);
					for (size_t i = 0 ; i < pos_binds.second.size(); ++ i)
					{
						assert(joint_nodes_.find(pos_binds.second[i].first) != joint_nodes_.end());

						pos_binds.first += pos_binds.second[i].second
							* (v0 * joint_nodes_[pos_binds.second[i].first]);
					}

					if (pos_binds.second.size() > static_cast<size_t>(joints_per_ver_))
					{
						std::nth_element(pos_binds.second.begin(), pos_binds.second.begin() + joints_per_ver_,
							pos_binds.second.end(), bind_cmp);
						pos_binds.second.resize(joints_per_ver_);

						float sum_weight = 0;
						for (int j = 0; j < joints_per_ver_; ++ j)
						{
							sum_weight += pos_binds.second[j].second;
						}
						assert(sum_weight > 0);

						for (int j = 0; j < joints_per_ver_; ++ j)
						{
							pos_binds.second[j].second /= sum_weight;
						}
					}
				}
			}
			else
			{
				for (auto& pos_binds : positions)
				{
					pos_binds.first = pos_binds.first * obj_matrix;
				}
			}

			std::vector<Point3> face_normals(obj_triangles.size());
			std::vector<Point3> face_tangents(obj_triangles.size());
			std::vector<Point3> face_binormals(obj_triangles.size());
			int uv_layer = texs.begin()->first;
			for (size_t i = 0; i < obj_triangles.size(); ++ i)
			{
				face_normals[i] = compute_normal(positions[pos_indices[i * 3 + 2]].first,
					positions[pos_indices[i * 3 + 1]].first, positions[pos_indices[i * 3 + 0]].first);

				compute_tangent(face_tangents[i], face_binormals[i], positions[pos_indices[i * 3 + 2]].first,
					positions[pos_indices[i * 3 + 1]].first, positions[pos_indices[i * 3 + 0]].first,
					texs[uv_layer][tex_indices[uv_layer][i * 3 + 2]], texs[uv_layer][tex_indices[uv_layer][i * 3 + 1]],
					texs[uv_layer][tex_indices[uv_layer][i * 3 + 0]]);
			}

			obj_vertices.resize(vertex_indices.size());
			int ver_index = 0;
			for (auto const & vertex_index : vertex_indices)
			{
				vertex_t& vertex = obj_vertices[ver_index];

				vertex.pos = positions[vertex_index.pos_index].first;
				std::swap(vertex.pos.y, vertex.pos.z);

				Point3 normal(0, 0, 0);
				Point3 tangent(0, 0, 0);
				Point3 binormal(0, 0, 0);
				for (size_t i = 0; i < vertex_sm_group[vertex_index.pos_index][vertex_index.sm_index].size(); ++ i)
				{
					unsigned int tri_id = vertex_sm_group[vertex_index.pos_index][vertex_index.sm_index][i];
					normal += face_normals[tri_id];
					tangent += face_tangents[tri_id];
					binormal += face_binormals[tri_id];
				}
				if (flip_normals)
				{
					normal = -normal;
					tangent = -tangent;
					binormal = -binormal;
				}
				vertex.normal = normal.Normalize();
				// Gram-Schmidt orthogonalize
				vertex.tangent = (tangent - vertex.normal * (tangent % vertex.normal)).Normalize();
				vertex.binormal = (vertex.normal ^ vertex.tangent).Normalize();
				// Calculate handedness
				if (vertex.binormal % binormal < 0)
				{
					vertex.binormal = -vertex.binormal;
				}

				std::swap(vertex.normal.y, vertex.normal.z);
				std::swap(vertex.tangent.y, vertex.tangent.z);
				std::swap(vertex.binormal.y, vertex.binormal.z);

				vertex.binormal = -vertex.binormal;

				int uv_layer_index = 0;
				for (auto uv_iter = texs.begin(); uv_iter != texs.end(); ++ uv_iter, ++ uv_layer_index)
				{
					Point2 const & tex = uv_iter->second[vertex_index.tex_indices[uv_layer_index]];
					obj_vertices[ver_index].tex.push_back(Point2(tex.x, 1 - tex.y));
				}

				for (size_t i = 0; i < vertex_index.ref_triangle.size(); ++ i)
				{
					obj_triangles[vertex_index.ref_triangle[i] / 3].vertex_index[vertex_index.ref_triangle[i] % 3] = ver_index;
				}

				vertex.binds = positions[vertex_index.pos_index].second;

				++ ver_index;
			}

			for (size_t i = mtl_base_index; i < objs_mtl_id_.size(); ++ i)
			{
				triangles_t obj_info_tris;
				std::vector<int> index_set;
				for (size_t j = 0; j < obj_triangles.size(); ++ j)
				{
					if (face_mtl_id[j] == i)
					{
						index_set.push_back(obj_triangles[j].vertex_index[0]);
						index_set.push_back(obj_triangles[j].vertex_index[1]);
						index_set.push_back(obj_triangles[j].vertex_index[2]);

						obj_info_tris.push_back(obj_triangles[j]);
					}
				}
				std::sort(index_set.begin(), index_set.end());
				index_set.erase(std::unique(index_set.begin(), index_set.end()), index_set.end());

				if (!obj_info_tris.empty())
				{
					int mesh_id = meshml_obj_.AllocMesh();

					std::map<int, int> mapping;
					int new_index = 0;
					for (auto iter = index_set.begin(); iter != index_set.end(); ++ iter, ++ new_index)
					{
						mapping.emplace(*iter, new_index);

						vertex_t const & vert = obj_vertices[*iter];

						int vertex_id = meshml_obj_.AllocVertex(mesh_id, 0);
						float3 pos(vert.pos.x, vert.pos.y, vert.pos.z);
						float3 normal(vert.normal.x, vert.normal.y, vert.normal.z);
						float3 tangent(vert.tangent.x, vert.tangent.y, vert.tangent.z);
						float3 binormal(vert.binormal.x, vert.binormal.y, vert.binormal.z);
						std::vector<float3> tex_coords(vert.tex.size());
						for (size_t ti = 0; ti < texs.size(); ++ ti)
						{
							tex_coords[ti] = float3(vert.tex[ti].x, vert.tex[ti].y, 0);
						}
						meshml_obj_.SetVertex(mesh_id, 0, vertex_id, pos, tangent, binormal, normal, 2, tex_coords);

						for (size_t bi = 0; bi < obj_vertices[*iter].binds.size(); ++ bi)
						{
							int binding_id = meshml_obj_.AllocJointBinding(mesh_id, 0, vertex_id);
							meshml_obj_.SetJointBinding(mesh_id, 0, vertex_id, binding_id,
								joint_node_to_id_[obj_vertices[*iter].binds[bi].first], obj_vertices[*iter].binds[bi].second);
						}
					}
					for (size_t j = 0; j < obj_info_tris.size(); ++ j)
					{
						int triangle_id = meshml_obj_.AllocTriangle(mesh_id, 0);
						meshml_obj_.SetTriangle(mesh_id, 0, triangle_id, mapping[obj_info_tris[j].vertex_index[0]],
							mapping[obj_info_tris[j].vertex_index[1]], mapping[obj_info_tris[j].vertex_index[2]]);
					}
				
					if (objs_mtl_id_.size() - mtl_base_index <= 1)
					{
						meshml_obj_.SetMesh(mesh_id, objs_mtl_id_[i], obj_name, 1);
					}
					else
					{
						meshml_obj_.SetMesh(mesh_id, objs_mtl_id_[i], 
							obj_name + "__mat_" + boost::lexical_cast<std::string>(i - mtl_base_index), 1);
					}
				}
			}
		}
	}

	void meshml_extractor::extract_bone(INode* node)
	{
		assert(is_bone(node));

		int tpf = GetTicksPerFrame();

		INode* parent_node = node->GetParentNode();
		if (!is_bone(parent_node))
		{
			parent_node = root_node_;
		}

		int kfs_id = meshml_obj_.AllocKeyframes();
		meshml_obj_.SetKeyframes(kfs_id, joint_node_to_id_[node]);

		for (int i = start_frame_; i < end_frame_; ++ i)
		{
			int kf_id = meshml_obj_.AllocKeyframe(kfs_id);
			meshml_obj_.SetKeyframe(kfs_id, kf_id, i,
				this->rh_to_lh(node->GetNodeTM(i * tpf) * Inverse(parent_node->GetNodeTM(i * tpf))));
		}
	}

	float4x4 meshml_extractor::rh_to_lh(Matrix3 const & mat)
	{
		return float4x4(mat.GetRow(0).x, mat.GetRow(0).z, mat.GetRow(0).y, 0,
			mat.GetRow(2).x, mat.GetRow(2).z, mat.GetRow(2).y, 0,
			mat.GetRow(1).x, mat.GetRow(1).z, mat.GetRow(1).y, 0,
			mat.GetRow(3).x, mat.GetRow(3).z, mat.GetRow(3).y, 1);
	}

	void meshml_extractor::extract_all_joint_tms()
	{
		for (auto& jn : joint_nodes_)
		{
			INode* parent_node = jn.first->GetParentNode();
			if (!is_bone(parent_node))
			{
				parent_node = root_node_;
			}

			Matrix3 tmp_tm;
			Matrix3 skin_init_tm;
			skin_init_tm.IdentityMatrix();
			bool found = false;
			for (size_t i = 0; i < physiques_.size(); ++ i)
			{
				if (MATRIX_RETURNED == physiques_[i]->GetInitNodeTM(jn.first, tmp_tm))
				{
					skin_init_tm = tmp_tm;
					found = true;
					break;
				}
			}
			if (!found)
			{
				for (size_t i = 0; i < skins_.size(); ++ i)
				{
					if (SKIN_OK == skins_[i]->GetBoneInitTM(jn.first, tmp_tm, false))
					{
						skin_init_tm = tmp_tm;
						found = true;
						break;
					}
				}
			}
			if (!found)
			{
				// fake bone
				skin_init_tm = jn.first->GetNodeTM(0);
			}

			jn.second = Inverse(jn.first->GetNodeTM(0)) * skin_init_tm;

			auto iter = joint_node_to_id_.find(jn.first);
			assert(iter != joint_node_to_id_.end());
			auto par_iter = joint_node_to_id_.find(parent_node);
			assert(par_iter != joint_node_to_id_.end());
			meshml_obj_.SetJoint(iter->second, tstr_to_str(jn.first->GetName()), par_iter->second,
				this->rh_to_lh(skin_init_tm));
		}
	}

	void meshml_extractor::add_joint_weight(binds_t& binds, INode* joint_node, float weight)
	{
		if (weight > 0)
		{
			bool repeat = false;
			for (auto& bind : binds)
			{
				if (bind.first == joint_node)
				{
					bind.second += weight;
					repeat = true;
					break;
				}
			}
			if (!repeat && (weight > 0))
			{
				binds.emplace_back(joint_node, weight);
			}
		}
	}

	void meshml_extractor::physique_modifier(Modifier* mod, INode* node,
										std::vector<std::pair<Point3, binds_t>>& positions)
	{
		assert(mod != NULL);
		// Is this Physique?
		assert(Class_ID(PHYSIQUE_CLASS_ID_A, PHYSIQUE_CLASS_ID_B) == mod->ClassID());

		IPhysiqueExport* phy_exp = static_cast<IPhysiqueExport*>(mod->GetInterface(I_PHYINTERFACE));
		if (phy_exp != NULL)
		{
			// create a ModContext Export Interface for the specific node of the Physique Modifier
			IPhyContextExport* mod_context = phy_exp->GetContextInterface(node);
			if (mod_context != NULL)
			{
				// needed by vertex interface (only Rigid supported by now)
				mod_context->ConvertToRigid(true);

				// more than a single bone per vertex
				mod_context->AllowBlending(true);

				for (int i = 0; i < mod_context->GetNumberVertices(); ++ i)
				{
					IPhyVertexExport* phy_ver_exp = mod_context->GetVertexInterface(i);
					if (phy_ver_exp != NULL)
					{
						switch (phy_ver_exp->GetVertexType())
						{
						case RIGID_NON_BLENDED_TYPE:
							{
								IPhyRigidVertex* phy_rigid_ver = static_cast<IPhyRigidVertex*>(phy_ver_exp);
								this->add_joint_weight(positions[i].second, phy_rigid_ver->GetNode(), 1);
							}
							break;

						case RIGID_BLENDED_TYPE:
							{
								IPhyBlendedRigidVertex* phy_blended_rigid_ver = static_cast<IPhyBlendedRigidVertex*>(phy_ver_exp);
								for (int j = 0; j < phy_blended_rigid_ver->GetNumberNodes(); ++ j)
								{
									this->add_joint_weight(positions[i].second,
										phy_blended_rigid_ver->GetNode(j),
										phy_blended_rigid_ver->GetWeight(j));
								}
							}
							break;
						}
					}
				}
			}

			phy_exp->ReleaseContextInterface(mod_context);
		}

		mod->ReleaseInterface(I_PHYINTERFACE, phy_exp);
	}

	void meshml_extractor::skin_modifier(Modifier* mod, INode* node,
									std::vector<std::pair<Point3, binds_t>>& positions)
	{
		assert(mod != NULL);
		// Is this Skin?
		assert(SKIN_CLASSID == mod->ClassID());

		ISkin* skin = static_cast<ISkin*>(mod->GetInterface(I_SKIN));
		if (skin != NULL)
		{
			ISkinContextData* skin_cd = skin->GetContextInterface(node);
			if (skin_cd != NULL)
			{
				for (int i = 0; i < skin_cd->GetNumPoints(); ++ i)
				{
					for (int j = 0; j < skin_cd->GetNumAssignedBones(i); ++ j)
					{
						this->add_joint_weight(positions[i].second,
							skin->GetBone(skin_cd->GetAssignedBone(i, j)),
							skin_cd->GetBoneWeight(i, j));
					}
				}
			}

			mod->ReleaseInterface(I_SKIN, skin);
		}
	}

	void meshml_extractor::write_xml(std::string const & file_name, export_vertex_attrs const & eva)
	{
		std::ofstream ofs(file_name.c_str());

		int vertex_export_settings = 0;
		if (eva.normal)
		{
			vertex_export_settings |= MeshMLObj::VES_Normal;
		}
		if (eva.tangent_quat)
		{
			vertex_export_settings |= MeshMLObj::VES_TangentQuat;
		}
		if (eva.tex)
		{
			vertex_export_settings |= MeshMLObj::VES_Texcoord;
		}

		int user_export_settings = 0;
		if (combine_meshes_)
		{
			user_export_settings |= MeshMLObj::UES_CombineMeshes;
		}
		user_export_settings |= MeshMLObj::UES_SortMeshes;

		meshml_obj_.WriteMeshML(ofs, vertex_export_settings, user_export_settings);
	}
}
