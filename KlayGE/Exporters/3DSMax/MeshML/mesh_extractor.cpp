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

#include <max.h>
#include <modstack.h>
#include <stdmat.h>
#include <iparamb2.h>
#if VERSION_3DSMAX >= 7 << 16
#include <CS/phyexp.h>
#else
#include <phyexp.h>
#endif
#include <iskin.h>

#include <fstream>
#include <sstream>
#include <algorithm>
#include <set>
#include <vector>
#include <limits>
#include <functional>

#include <boost/typeof/typeof.hpp>
#include <boost/foreach.hpp>

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

	struct less_Point2 : public std::binary_function<Point2, Point2, bool>
	{
		bool operator()(Point2 const & lhs, Point2 const & rhs) const
		{
			if (lhs.x < rhs.x)
			{
				return true;
			}
			else
			{
				if (lhs.x > rhs.x)
				{
					return false;
				}
				else
				{
					if (lhs.y < rhs.y)
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
	};

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

	Quat ToQuaternion(Matrix3 const & mat)
	{
		Quat quat;
		float s;
		float const tr = mat.GetRow(0).x + mat.GetRow(1).y + mat.GetRow(2).z;

		// check the diagonal
		if (tr > 0)
		{
			s = sqrt(tr + 1);
			quat.w = s * 0.5f;
			s = 0.5f / s;
			quat.x = (mat.GetRow(1).z - mat.GetRow(2).y) * s;
			quat.y = (mat.GetRow(2).x - mat.GetRow(0).z) * s;
			quat.z = (mat.GetRow(0).y - mat.GetRow(1).x) * s;
		}
		else
		{
			if ((mat.GetRow(1).y > mat.GetRow(0).x) && (mat.GetRow(2).z <= mat.GetRow(1).y))
			{
				s = sqrt((mat.GetRow(1).y - (mat.GetRow(2).z + mat.GetRow(0).x)) + 1);

				quat.y = s * 0.5f;

				if (abs(s) > 1e-6f)
				{
					s = 0.5f / s;
				}

				quat.w = (mat.GetRow(2).x - mat.GetRow(0).z) * s;
				quat.z = (mat.GetRow(2).y + mat.GetRow(1).z) * s;
				quat.x = (mat.GetRow(0).y + mat.GetRow(1).x) * s;
			}
			else
			{
				if (((mat.GetRow(1).y <= mat.GetRow(0).x) && (mat.GetRow(2).z > mat.GetRow(0).x)) || (mat.GetRow(2).z > mat.GetRow(1).y))
				{
					s = sqrt((mat.GetRow(2).z - (mat.GetRow(0).x + mat.GetRow(1).y)) + 1);

					quat.z = s * 0.5f;

					if (abs(s) > 1e-6f)
					{
						s = 0.5f / s;
					}

					quat.w = (mat.GetRow(0).y - mat.GetRow(1).x) * s;
					quat.x = (mat.GetRow(0).z + mat.GetRow(2).x) * s;
					quat.y = (mat.GetRow(1).z + mat.GetRow(2).y) * s;
				}
				else
				{
					s = sqrt((mat.GetRow(0).x - (mat.GetRow(1).y + mat.GetRow(2).z)) + 1);

					quat.x = s * 0.5f;

					if (abs(s) > 1e-6f)
					{
						s = 0.5f / s;
					}

					quat.w = (mat.GetRow(1).z - mat.GetRow(2).y) * s;
					quat.y = (mat.GetRow(1).x + mat.GetRow(0).y) * s;
					quat.z = (mat.GetRow(2).x + mat.GetRow(0).z) * s;
				}
			}
		}

		quat.Normalize();
		return quat;
	}

	Quat ToQuaternion(Point3 const & tangent, Point3 const & binormal, Point3 const & normal, int bits)
	{
		float k = 1;
		if (binormal % (normal ^ tangent) < 0)
		{
			k = -1;
		}

		Matrix3 tangent_frame(tangent, k * binormal, normal, Point3(0, 0, 0));
		Quat tangent_quat = ToQuaternion(tangent_frame);
		if (tangent_quat.w < 0)
		{
			tangent_quat = -tangent_quat;
		}
		float const bias = 1.0f / ((1UL << (bits - 1)) - 1);
		if (tangent_quat.w < bias)
		{
			float const factor = sqrt(1 - bias * bias);
			tangent_quat.x *= factor;
			tangent_quat.y *= factor;
			tangent_quat.z *= factor;
			tangent_quat.w = bias;
		}
		if (k < 0)
		{
			tangent_quat = -tangent_quat;
		}

		return tangent_quat;
	}

	Quat QuatTransToUDQ(Quat const & q, Point3 const & t)
	{
		return Quat(+0.5f * (+t[0] * q[3] + t[1] * q[2] - t[2] * q[1]),
			+0.5f * (-t[0] * q[2] + t[1] * q[3] + t[2] * q[0]),
			+0.5f * (+t[0] * q[1] - t[1] * q[0] + t[2] * q[3]),
			-0.5f * (+t[0] * q[0] + t[1] * q[1] + t[2] * q[2]));
	}

	std::string RemoveQuote(std::string const & str)
	{
		std::string ret = str;
		ret.erase(std::remove(ret.begin(), ret.end(), '\"'), ret.end());
		return ret;
	}
}

namespace KlayGE
{
	meshml_extractor::meshml_extractor(INode* root_node, int joints_per_ver, int cur_time, int start_frame, int end_frame, bool combine_meshes)
						: root_node_(root_node),
							unit_scale_(static_cast<float>(GetMasterScale(UNITS_METERS))),
							joints_per_ver_(joints_per_ver),
							cur_time_(cur_time),
							start_frame_(start_frame), end_frame_(end_frame),
							frame_rate_(GetFrameRate()),
							combine_meshes_(combine_meshes)
	{
	}

	void meshml_extractor::find_joints(INode* node)
	{
		if (is_bone(node))
		{
			joint_nodes_.insert(std::make_pair(node, Matrix3()));
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
			joint_t root;
			root.real.Identity();
			root.dual.x = root.dual.y = root.dual.z = root.dual.w = 0;
			root.parent_node = NULL;
			joints_.insert(std::make_pair(root_node_, root));

			int tpf = GetTicksPerFrame();

			key_frame_t kf;
			for (int i = start_frame_; i < end_frame_; ++ i)
			{
				Matrix3 root_tm = root_node_->GetNodeTM(i * tpf);

				Point3 scale;
				Quat real, dual;
				this->decompose_matrix(scale, real, dual, root_tm);
				kf.reals.push_back(real * scale.x);
				kf.duals.push_back(dual);
			}
			kfs_.insert(std::make_pair(root_node_, kf));

			this->find_joints(root_node_);


			physiques_.clear();
			physique_mods_.clear();
			skins_.clear();
			skin_mods_.clear();
			typedef BOOST_TYPEOF(nodes) nodes_type;
			BOOST_FOREACH(nodes_type::const_reference node, nodes)
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

		typedef BOOST_TYPEOF(joint_nodes_) jn_type;
		BOOST_FOREACH(jn_type::const_reference jn, joint_nodes_)
		{
			if (is_bone(jn.first))
			{
				jnodes.push_back(jn.first);
			}
		}
		typedef BOOST_TYPEOF(nodes) nodes_type;
		BOOST_FOREACH(nodes_type::const_reference node, nodes)
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

		typedef BOOST_TYPEOF(jnodes) jnodes_type;
		BOOST_FOREACH(jnodes_type::const_reference jnode, jnodes)
		{
			this->extract_bone(jnode);
		}
		
		typedef BOOST_TYPEOF(mnodes) mnodes_type;
		BOOST_FOREACH(mnodes_type::const_reference mnode, mnodes)
		{
			this->extract_object(mnode);
		}
	}

	void meshml_extractor::get_material(materials_t& mtls, std::vector<std::map<int, std::pair<Matrix3, int> > >& uv_transss, Mtl* max_mtl)
	{
		if (max_mtl)
		{
			if (0 == max_mtl->NumSubMtls())
			{
				mtls.push_back(material_t());
				material_t& mtl = mtls.back();
				uv_transss.push_back(std::map<int, std::pair<Matrix3, int> >());
				std::map<int, std::pair<Matrix3, int> >& uv_transs = uv_transss.back();

				mtl.ambient = max_mtl->GetAmbient();
				mtl.diffuse = max_mtl->GetDiffuse();
				mtl.specular = max_mtl->GetSpecular();
				if (max_mtl->GetSelfIllumColorOn())
				{
					mtl.emit = max_mtl->GetSelfIllumColor();
				}
				else
				{
					mtl.emit = max_mtl->GetDiffuse() * max_mtl->GetSelfIllum();
				}
				mtl.opacity = 1 - max_mtl->GetXParency();
				mtl.specular_level = max_mtl->GetShinStr();
				mtl.shininess = max_mtl->GetShininess() * 100;

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

							mtl.texture_slots.push_back(texture_slot_t(tstr_to_str(max_mtl->GetSubTexmapSlotName(j).data()), map_name));
						}
					}
				}
			}
			else
			{
				for (int i = 0; i < max_mtl->NumSubMtls(); ++ i)
				{
					this->get_material(mtls, uv_transss, max_mtl->GetSubMtl(i));
				}
			}
		}
	}

	void meshml_extractor::extract_object(INode* node)
	{
		assert(is_mesh(node));

		std::string		obj_name;
		vertices_t		obj_vertices;
		triangles_t		obj_triangles;
		vertex_elements_t obj_vertex_elements;

		obj_name = tstr_to_str(node->GetName());

		Matrix3 obj_matrix = node->GetObjTMAfterWSM(cur_time_);
		bool flip_normals = obj_matrix.Parity() ? true : false;

		std::vector<std::pair<Point3, binds_t> > positions;
		std::map<int, std::vector<Point2> > texs;
		std::vector<int> pos_indices;
		std::map<int, std::vector<int> > tex_indices;
		std::vector<std::map<int, std::pair<Matrix3, int> > > uv_transs;

		size_t mtl_base_index = objs_mtl_.size();
		Mtl* mtl = node->GetMtl();
		if (mtl != NULL)
		{
			this->get_material(objs_mtl_, uv_transs, mtl);
		}

		std::vector<unsigned int> face_sm_group;
		std::vector<unsigned int> face_mtl_id;
		std::vector<std::vector<std::vector<unsigned int> > > vertex_sm_group;

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
								int mtl_id = mesh.getFaceMtlIndex(static_cast<int>(i)) % (objs_mtl_.size() - mtl_base_index);

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
					if (objs_mtl_.size() != mtl_base_index)
					{
						face_mtl_id[i] = static_cast<unsigned int>(mtl_base_index + face_mtl_id[i] % (objs_mtl_.size() - mtl_base_index));
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
					typedef BOOST_TYPEOF(tex_indices) ti_type;
					BOOST_FOREACH(ti_type::const_reference tex_index, tex_indices)
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

					std::set<vertex_index_t>::iterator v_iter = vertex_indices.find(vertex_index);
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
				typedef BOOST_TYPEOF(positions) positions_type;
				BOOST_FOREACH(positions_type::reference pos_binds, positions)
				{
					if (pos_binds.second.empty())
					{
						INode* parent_node = node->GetParentNode();
						while ((parent_node != root_node_) && !is_bone(parent_node))
						{
							parent_node = parent_node->GetParentNode();
						}

						pos_binds.second.push_back(std::make_pair(parent_node, 1.0f));
					}

					Point3 v0 = pos_binds.first * tm;
					pos_binds.first = Point3(0, 0, 0);
					for (size_t i = 0 ; i < pos_binds.second.size(); ++ i)
					{
						assert(joints_.find(pos_binds.second[i].first) != joints_.end());

						pos_binds.first += pos_binds.second[i].second
							* (v0 * joint_nodes_[pos_binds.second[i].first]);
					}

					if (pos_binds.second.size() > static_cast<size_t>(joints_per_ver_))
					{
						std::nth_element(pos_binds.second.begin(), pos_binds.second.begin() + joints_per_ver_, pos_binds.second.end(), bind_cmp);
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
				typedef BOOST_TYPEOF(positions) positions_type;
				BOOST_FOREACH(positions_type::reference pos_binds, positions)
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
					texs[uv_layer][tex_indices[uv_layer][i * 3 + 2]], texs[uv_layer][tex_indices[uv_layer][i * 3 + 1]], texs[uv_layer][tex_indices[uv_layer][i * 3 + 0]]);
			}

			obj_vertices.resize(vertex_indices.size());
			int ver_index = 0;
			typedef BOOST_TYPEOF(vertex_indices) vi_type;
			BOOST_FOREACH(vi_type::const_reference vertex_index, vertex_indices)
			{
				vertex_t& vertex = obj_vertices[ver_index];

				vertex.pos = positions[vertex_index.pos_index].first * unit_scale_;
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
				Point3 vertex_tangent = (tangent - vertex.normal * (tangent % vertex.normal)).Normalize();
				Point3 vertex_binormal = (vertex.normal ^ vertex_tangent).Normalize();
				// Calculate handedness
				if (vertex_binormal % binormal < 0)
				{
					vertex_binormal = -vertex_binormal;
				}

				std::swap(vertex.normal.y, vertex.normal.z);
				std::swap(vertex_tangent.y, vertex_tangent.z);
				std::swap(vertex_binormal.y, vertex_binormal.z);

				vertex.tangent_quat = ToQuaternion(vertex_tangent, -vertex_binormal, vertex.normal, 8);

				int uv_layer = 0;
				for (std::map<int, std::vector<Point2> >::iterator uv_iter = texs.begin();
					uv_iter != texs.end(); ++ uv_iter, ++ uv_layer)
				{
					Point2 const & tex = uv_iter->second[vertex_index.tex_indices[uv_layer]];
					obj_vertices[ver_index].tex.push_back(Point2(tex.x, 1 - tex.y));
				}

				for (size_t i = 0; i < vertex_index.ref_triangle.size(); ++ i)
				{
					obj_triangles[vertex_index.ref_triangle[i] / 3].vertex_index[vertex_index.ref_triangle[i] % 3] = ver_index;
				}

				vertex.binds = positions[vertex_index.pos_index].second;

				++ ver_index;
			}

			obj_vertex_elements.push_back(vertex_element_t(VEU_Position, 0, 3));
			obj_vertex_elements.push_back(vertex_element_t(VEU_Normal, 0, 3));
			obj_vertex_elements.push_back(vertex_element_t(VEU_Tangent, 0, 3));
			obj_vertex_elements.push_back(vertex_element_t(VEU_Binormal, 0, 3));
			for (size_t i = 0; i < obj_vertices[0].tex.size(); ++ i)
			{
				obj_vertex_elements.push_back(vertex_element_t(VEU_TextureCoord, static_cast<unsigned char>(i), 2));
			}
			if (!obj_vertices[0].binds.empty())
			{
				obj_vertex_elements.push_back(vertex_element_t(VEU_BlendWeight, 0, 4));
				obj_vertex_elements.push_back(vertex_element_t(VEU_BlendIndex, 0, 4));
			}

			for (size_t i = mtl_base_index; i < objs_mtl_.size(); ++ i)
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
					objs_info_.push_back(object_info_t());

					object_info_t& obj_info = objs_info_.back();
					obj_info.vertex_elements = obj_vertex_elements;

					obj_info.triangles.resize(obj_info_tris.size());

					std::map<int, int> mapping;
					int new_index = 0;
					for (std::vector<int>::iterator iter = index_set.begin(); iter != index_set.end(); ++ iter, ++ new_index)
					{
						obj_info.vertices.push_back(obj_vertices[*iter]);
						mapping.insert(std::make_pair(*iter, new_index));
					}
					for (size_t j = 0; j < obj_info_tris.size(); ++ j)
					{
						obj_info.triangles[j].vertex_index[0] = mapping[obj_info_tris[j].vertex_index[0]];
						obj_info.triangles[j].vertex_index[1] = mapping[obj_info_tris[j].vertex_index[1]];
						obj_info.triangles[j].vertex_index[2] = mapping[obj_info_tris[j].vertex_index[2]];
					}
				
					if (objs_mtl_.size() - mtl_base_index <= 1)
					{
						obj_info.name = obj_name;
					}
					else
					{
						std::ostringstream oss;
						oss << obj_name << "__mat_" << (i - mtl_base_index);
						obj_info.name = oss.str();
					}
					obj_info.mtl_id = i;
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

		key_frame_t kf;
		for (int i = start_frame_; i < end_frame_; ++ i)
		{
			Matrix3 local_tm = node->GetNodeTM(i * tpf) * Inverse(parent_node->GetNodeTM(i * tpf));

			Point3 scale;
			Quat real, dual;
			this->decompose_matrix(scale, real, dual, local_tm);
			kf.reals.push_back(real * scale.x);
			kf.duals.push_back(dual);
		}

		kfs_.insert(std::make_pair(node, kf));
	}

	void meshml_extractor::decompose_matrix(Point3& scale, Quat& real, Quat& dual, Matrix3 const & mat)
	{
		Matrix3 mat_lh;
		mat_lh.SetRow(0, Point3(mat.GetRow(0).x, mat.GetRow(0).z, mat.GetRow(0).y));
		mat_lh.SetRow(1, Point3(mat.GetRow(2).x, mat.GetRow(2).z, mat.GetRow(2).y));
		mat_lh.SetRow(2, Point3(mat.GetRow(1).x, mat.GetRow(1).z, mat.GetRow(1).y));
		mat_lh.SetRow(3, Point3(mat.GetRow(3).x, mat.GetRow(3).z, mat.GetRow(3).y));

		int flip = mat_lh.Parity() ? -1 : +1;

		scale.x = mat_lh.GetRow(0).Length();
		scale.y = mat_lh.GetRow(1).Length();
		scale.z = flip * mat_lh.GetRow(2).Length();

		Point3 trans = mat_lh.GetRow(3);

		mat_lh.SetRow(0, mat_lh.GetRow(0) / scale.x);
		mat_lh.SetRow(1, mat_lh.GetRow(1) / scale.y);
		mat_lh.SetRow(2, mat_lh.GetRow(2) / scale.z);
		mat_lh.SetRow(3, Point3(0, 0, 0));
		real = ToQuaternion(mat_lh);

		dual = QuatTransToUDQ(real, trans * unit_scale_);

		if (flip * real.w < 0)
		{
			real = -real;
			dual = -dual;
		}
	}

	void meshml_extractor::extract_all_joint_tms()
	{
		typedef BOOST_TYPEOF(joint_nodes_) jn_type;
		BOOST_FOREACH(jn_type::reference jn, joint_nodes_)
		{
			joint_t joint;

			INode* parent_node = jn.first->GetParentNode();
			if (!is_bone(parent_node))
			{
				parent_node = root_node_;
			}
			joint.parent_node = parent_node;

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

			Point3 scale;
			Quat real, dual;
			this->decompose_matrix(scale, real, dual, skin_init_tm);
			joint.real = real * scale.x;
			joint.dual = dual;

			joints_[jn.first] = joint;
		}
	}

	void meshml_extractor::add_joint_weight(binds_t& binds, INode* joint_node, float weight)
	{
		if (weight > 0)
		{
			bool repeat = false;
			typedef BOOST_TYPEOF(binds) binds_type;
			BOOST_FOREACH(binds_type::reference bind, binds)
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
				binds.push_back(std::make_pair(joint_node, weight));
			}
		}
	}

	void meshml_extractor::physique_modifier(Modifier* mod, INode* node,
										std::vector<std::pair<Point3, binds_t> >& positions)
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
									std::vector<std::pair<Point3, binds_t> >& positions)
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

	void meshml_extractor::remove_redundant_joints()
	{
		std::set<INode*> joints_used;
		typedef BOOST_TYPEOF(objs_info_) oi_type;
		BOOST_FOREACH(oi_type::const_reference obj_info, objs_info_)
		{
			typedef BOOST_TYPEOF(obj_info.vertices) ov_type;
			BOOST_FOREACH(ov_type::const_reference vertex, obj_info.vertices)
			{
				typedef BOOST_TYPEOF(vertex.binds) vb_type;
				BOOST_FOREACH(vb_type::const_reference bind, vertex.binds)
				{
					joints_used.insert(bind.first);
				}
			}
		}

		std::set<INode*> parent_joints_used;
		typedef BOOST_TYPEOF(joints_) joints_type;
		BOOST_FOREACH(joints_type::const_reference joint, joints_)
		{
			if (joints_used.find(joint.first) != joints_used.end())
			{
				joint_t const * j = &joint.second;
				while (j->parent_node != NULL)
				{
					parent_joints_used.insert(j->parent_node);
					j = &joints_[j->parent_node];
				}
			}
		}

		BOOST_FOREACH(std::set<INode*>::const_reference joint, parent_joints_used)
		{
			joints_used.insert(joint);
		}

		for (BOOST_AUTO(iter, joints_.begin()); iter != joints_.end();)
		{
			if (joints_used.find(iter->first) == joints_used.end())
			{
				iter = joints_.erase(iter);
			}
			else
			{
				++ iter;
			}
		}
	}

	void meshml_extractor::remove_redundant_mtls()
	{
		std::vector<size_t> mtl_mapping(objs_mtl_.size());
		materials_t mtls_used;
		for (size_t i = 0; i < objs_mtl_.size(); ++ i)
		{
			bool found = false;
			for (size_t j = 0; j < mtls_used.size(); ++ j)
			{
				if ((mtls_used[j].ambient == objs_mtl_[i].ambient)
					&& (mtls_used[j].diffuse == objs_mtl_[i].diffuse)
					&& (mtls_used[j].specular == objs_mtl_[i].specular)
					&& (mtls_used[j].emit == objs_mtl_[i].emit)
					&& (mtls_used[j].opacity == objs_mtl_[i].opacity)
					&& (mtls_used[j].specular_level == objs_mtl_[i].specular_level)
					&& (mtls_used[j].shininess == objs_mtl_[i].shininess)
					&& (mtls_used[j].texture_slots == objs_mtl_[i].texture_slots))
				{
					mtl_mapping[i] = j;
					found = true;
					break;
				}
			}

			if (!found)
			{
				mtl_mapping[i] = mtls_used.size();
				mtls_used.push_back(objs_mtl_[i]);
			}
		}

		objs_mtl_ = mtls_used;

		typedef BOOST_TYPEOF(objs_info_) oi_type;
		BOOST_FOREACH(oi_type::reference obj_info, objs_info_)
		{
			obj_info.mtl_id = mtl_mapping[obj_info.mtl_id];
		}
	}

	void meshml_extractor::combine_meshes_with_same_mtl()
	{
		objects_info_t opt_objs_info;
		for (size_t i = 0; i < objs_mtl_.size(); ++ i)
		{
			std::vector<vertex_elements_t> ves;
			std::vector<std::pair<size_t, size_t> > oids;
			for (size_t j = 0; j < objs_info_.size(); ++ j)
			{
				if (objs_info_[j].mtl_id == i)
				{
					bool found = false;
					for (size_t k = 0; k < ves.size(); ++ k)
					{
						if (ves[k] == objs_info_[j].vertex_elements)
						{
							oids.push_back(std::make_pair(j, k));
							found = true;
							break;
						}
					}

					if (!found)
					{
						oids.push_back(std::make_pair(j, ves.size()));
						ves.push_back(objs_info_[j].vertex_elements);
					}
				}
			}

			for (size_t j = 0; j < ves.size(); ++ j)
			{
				opt_objs_info.push_back(object_info_t());
				object_info_t& opt_obj = opt_objs_info.back();

				std::ostringstream oss;
				oss << "mesh_for_mtl_" << i << "_ve_" << j;
				opt_obj.name = oss.str();
				opt_obj.mtl_id = i;
				opt_obj.vertex_elements = ves[j];

				typedef BOOST_TYPEOF(oids) oids_type;
				BOOST_FOREACH(oids_type::reference oid, oids)
				{
					int base = static_cast<int>(opt_obj.vertices.size());
					if (oid.second == j)
					{
						opt_obj.vertices.insert(opt_obj.vertices.end(),
							objs_info_[oid.first].vertices.begin(), objs_info_[oid.first].vertices.end());

						typedef BOOST_TYPEOF(objs_info_[oid.first].triangles) tris_type;
						BOOST_FOREACH(tris_type::reference old_tri, objs_info_[oid.first].triangles)
						{
							triangle_t tri = old_tri;
							tri.vertex_index[0] += base;
							tri.vertex_index[1] += base;
							tri.vertex_index[2] += base;

							opt_obj.triangles.push_back(tri);
						}
					}
				}
			}
		}

		objs_info_ = opt_objs_info;
	}

	void meshml_extractor::sort_meshes_by_mtl()
	{
		std::vector<std::pair<size_t, size_t> > mtl_ids(objs_info_.size());
		for (size_t i = 0; i < objs_info_.size(); ++ i)
		{
			mtl_ids[i].first = objs_info_[i].mtl_id;
			mtl_ids[i].second = i;
		}

		std::sort(mtl_ids.begin(), mtl_ids.end());

		objects_info_t opt_objs_info;
		for (size_t i = 0; i < mtl_ids.size(); ++ i)
		{
			opt_objs_info.push_back(objs_info_[mtl_ids[i].second]);
		}

		objs_info_ = opt_objs_info;
	}

	void meshml_extractor::write_xml(std::string const & file_name, export_vertex_attrs const & eva)
	{
		std::ofstream ofs(file_name.c_str());
		if (!ofs)
		{
			return;
		}

		this->remove_redundant_joints();
		this->remove_redundant_mtls();
		if (combine_meshes_)
		{
			this->combine_meshes_with_same_mtl();
		}
		else
		{
			this->sort_meshes_by_mtl();
		}

		std::map<INode*, int> joints_node_to_id;
		std::vector<INode*> joints_id_to_node;
		{
			typedef BOOST_TYPEOF(joints_) joints_type;
			BOOST_FOREACH(joints_type::const_reference joint, joints_)
			{
				joints_id_to_node.push_back(joint.first);
			}

			bool swapped = true;
			while (swapped)
			{
				swapped = false;
				for (int i = 0; i < static_cast<int>(joints_id_to_node.size()); ++ i)
				{
					int par_index = -1;
					if (joints_[joints_id_to_node[i]].parent_node != NULL)
					{
						std::vector<INode*>::iterator par_iter = std::find(joints_id_to_node.begin(), joints_id_to_node.end(),
							joints_[joints_id_to_node[i]].parent_node);
						assert(par_iter != joints_id_to_node.end());
						par_index = static_cast<int>(par_iter - joints_id_to_node.begin());
					}

					if (par_index > i)
					{
						std::swap(joints_id_to_node[i], joints_id_to_node[par_index]);
						swapped = true;
						break;
					}
				}
			}

			for (int i = 0; i < static_cast<int>(joints_id_to_node.size()); ++ i)
			{
				joints_node_to_id.insert(std::make_pair(joints_id_to_node[i], i));
			}
		}

		using std::endl;

		ofs << "<?xml version=\"1.0\"?>" << endl << endl;
		ofs << "<model version=\"5\">" << endl;

		if (joints_per_ver_ > 0)
		{
			ofs << "\t<bones_chunk>" << endl;
			typedef BOOST_TYPEOF(joints_id_to_node) jitn_type;
			BOOST_FOREACH(jitn_type::const_reference joint_node, joints_id_to_node)
			{
				int parent_id = -1;
				if (joints_[joint_node].parent_node != NULL)
				{
					assert(joints_node_to_id.find(joints_[joint_node].parent_node) != joints_node_to_id.end());
					parent_id = joints_node_to_id[joints_[joint_node].parent_node];
					assert(parent_id < joints_node_to_id[joint_node]);
				}

				ofs << "\t\t<bone name=\"" << RemoveQuote(tstr_to_str(joint_node->GetName()))
					<< "\" parent=\"" << parent_id
					<< "\">" << endl;

				joint_t const & joint = joints_[joint_node];

				ofs << "\t\t\t<bind_real x=\"" << joint.real.x
					<< "\" y=\"" << joint.real.y
					<< "\" z=\"" << joint.real.z
					<< "\" w=\"" << joint.real.w << "\"/>" << endl;
				ofs << "\t\t\t<bind_dual x=\"" << joint.dual.x
					<< "\" y=\"" << joint.dual.y
					<< "\" z=\"" << joint.dual.z
					<< "\" w=\"" << joint.dual.w << "\"/>" << endl;

				ofs << "\t\t</bone>" << endl;
			}
			ofs << "\t</bones_chunk>" << endl;
		}

		if (!objs_mtl_.empty())
		{
			ofs << "\t<materials_chunk>" << endl;
			for (size_t i = 0; i < objs_mtl_.size(); ++ i)
			{
				ofs << "\t\t<material ambient_r=\"" << objs_mtl_[i].ambient.r
					<< "\" ambient_g=\"" << objs_mtl_[i].ambient.g
					<< "\" ambient_b=\"" << objs_mtl_[i].ambient.b
					<< "\" diffuse_r=\"" << objs_mtl_[i].diffuse.r
					<< "\" diffuse_g=\"" << objs_mtl_[i].diffuse.g
					<< "\" diffuse_b=\"" << objs_mtl_[i].diffuse.b
					<< "\" specular_r=\"" << objs_mtl_[i].specular.r
					<< "\" specular_g=\"" << objs_mtl_[i].specular.g
					<< "\" specular_b=\"" << objs_mtl_[i].specular.b
					<< "\" emit_r=\"" << objs_mtl_[i].emit.r
					<< "\" emit_g=\"" << objs_mtl_[i].emit.g
					<< "\" emit_b=\"" << objs_mtl_[i].emit.b
					<< "\" opacity=\"" << objs_mtl_[i].opacity
					<< "\" specular_level=\"" << objs_mtl_[i].specular_level
					<< "\" shininess=\"" << objs_mtl_[i].shininess
					<< "\"";
				if (!objs_mtl_[i].texture_slots.empty())
				{
					ofs << ">" << endl;

					ofs << "\t\t\t<textures_chunk>" << endl;
					typedef BOOST_TYPEOF(objs_mtl_[i].texture_slots) slots_type;
					BOOST_FOREACH(slots_type::const_reference ts, objs_mtl_[i].texture_slots)
					{
						ofs << "\t\t\t\t<texture type=\"" << RemoveQuote(ts.first)
							<< "\" name=\"" << RemoveQuote(ts.second) << "\"/>" << endl;
					}
					ofs << "\t\t\t</textures_chunk>" << endl;

					ofs << "\t\t</material>" << endl;
				}
				else
				{
					ofs << "/>" << endl;
				}
			}
			ofs << "\t</materials_chunk>" << endl;
		}

		ofs << "\t<meshes_chunk>" << endl;
		typedef BOOST_TYPEOF(objs_info_) oi_type;
		BOOST_FOREACH(oi_type::const_reference obj_info, objs_info_)
		{
			ofs << "\t\t<mesh name=\"" << RemoveQuote(obj_info.name) << "\" mtl_id=\"" << obj_info.mtl_id << "\">" << endl;

			ofs << "\t\t\t<vertices_chunk>" << endl;
			typedef BOOST_TYPEOF(obj_info.vertices) ov_type;
			BOOST_FOREACH(ov_type::const_reference vertex, obj_info.vertices)
			{
				ofs << "\t\t\t\t<vertex x=\"" << vertex.pos.x
					<< "\" y=\"" << vertex.pos.y
					<< "\" z=\"" << vertex.pos.z << "\">" << endl;

				if (eva.normal)
				{
					ofs << "\t\t\t\t\t<normal x=\"" << vertex.normal.x
						<< "\" y=\"" << vertex.normal.y
						<< "\" z=\"" << vertex.normal.z << "\"/>" << endl;
				}
				if (eva.tangent_quat)
				{
					ofs << "\t\t\t\t\t<tangent_quat x=\"" << vertex.tangent_quat.x
						<< "\" y=\"" << vertex.tangent_quat.y
						<< "\" z=\"" << vertex.tangent_quat.z
						<< "\" w=\"" << vertex.tangent_quat.w << "\"/>" << endl;
				}

				if (eva.tex)
				{
					typedef BOOST_TYPEOF(vertex.tex) vt_type;
					BOOST_FOREACH(vt_type::const_reference tex, vertex.tex)
					{
						ofs << "\t\t\t\t\t<tex_coord u=\"" << tex.x
							<< "\" v=\"" << tex.y << "\"/>" << endl;
					}
				}

				typedef BOOST_TYPEOF(vertex.binds) vb_type;
				BOOST_FOREACH(vb_type::const_reference bind, vertex.binds)
				{
					assert(joints_node_to_id.find(bind.first) != joints_node_to_id.end());
					ofs << "\t\t\t\t\t<weight bone_index=\"" << joints_node_to_id[bind.first]
						<< "\" weight=\"" << bind.second << "\"/>" << endl;
				}

				ofs << "\t\t\t\t</vertex>" << endl;
			}
			ofs << "\t\t\t</vertices_chunk>" << endl;

			ofs << "\t\t\t<triangles_chunk>" << endl;
			typedef BOOST_TYPEOF(obj_info.triangles) oit_type;
			BOOST_FOREACH(oit_type::const_reference tri, obj_info.triangles)
			{
				ofs << "\t\t\t\t<triangle a=\"" << tri.vertex_index[0]
					<< "\" b=\"" << tri.vertex_index[1]
					<< "\" c=\"" << tri.vertex_index[2] << "\"/>" << endl;
			}
			ofs << "\t\t\t</triangles_chunk>" << endl;

			ofs << "\t\t</mesh>" << endl;
		}
		ofs << "\t</meshes_chunk>" << endl;

		if (joints_per_ver_ > 0)
		{
			ofs << "\t<key_frames_chunk start_frame=\"" << start_frame_
				<< "\" end_frame=\"" << end_frame_
				<< "\" frame_rate=\"" << frame_rate_ << "\">" << endl;
			typedef BOOST_TYPEOF(joints_id_to_node) jitn_type;
			BOOST_FOREACH(jitn_type::const_reference joint_node, joints_id_to_node)
			{
				assert(kfs_.find(joint_node) != kfs_.end());
				key_frame_t const & kf = kfs_[joint_node];
				assert(kf.reals.size() == kf.duals.size());

				ofs << "\t\t<key_frame joint=\"" << RemoveQuote(tstr_to_str(joint_node->GetName())) << "\">" << endl;

				for (size_t i = 0; i < kf.reals.size(); ++ i)
				{
					ofs << "\t\t\t<key>" << endl;

					ofs << "\t\t\t\t<bind_real x=\"" << kf.reals[i].x
						<< "\" y=\"" << kf.reals[i].y
						<< "\" z=\"" << kf.reals[i].z
						<< "\" w=\"" << kf.reals[i].w << "\"/>" << endl;
					ofs << "\t\t\t\t<bind_dual x=\"" << kf.duals[i].x
						<< "\" y=\"" << kf.duals[i].y
						<< "\" z=\"" << kf.duals[i].z
						<< "\" w=\"" << kf.duals[i].w << "\"/>" << endl;

					ofs << "\t\t\t</key>" << endl;
				}

				ofs << "\t\t</key_frame>" << endl;
			}
			ofs << "\t</key_frames_chunk>" << endl;
		}

		ofs << "</model>" << endl;
	}
}
