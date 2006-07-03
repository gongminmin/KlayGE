// mesh_extractor.cpp
// KlayGE 3DSMax网格数据析取类 实现文件
// Ver 3.0.0
// 版权所有(C) 龚敏敏, 2005
// Homepage: http://klayge.sourceforge.net
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
#include <iparamb2.h>
#include <modstack.h>
#include <phyexp.h>
#include <iskin.h>
#include <stdmat.h>

#include <algorithm>
#include <set>
#include <vector>

#include "util.hpp"
#include "mesh_extractor.hpp"

namespace
{
	struct vertex_index_t
	{
		int pos_index;
		std::vector<int> tex_indices;

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
					return false;
				}
			}
		}
	}

	bool bind_cmp(std::pair<std::string, float> const& lhs,
		std::pair<std::string, float> const& rhs)
	{
		return lhs.second > rhs.second;
	}
}

namespace KlayGE
{
	meshml_extractor::meshml_extractor(int joints_per_ver, int start_frame, int end_frame)
						: joints_per_ver_(joints_per_ver),
							start_frame_(start_frame), end_frame_(end_frame),
							frame_rate_(GetFrameRate())
	{
	}

	void meshml_extractor::export_objects(std::vector<INode*> const & nodes, bool flip_normals)
	{
		for (size_t i = 0; i < nodes.size(); ++ i)
		{
			if (is_mesh(nodes[i]))
			{
				this->extract_object(nodes[i], flip_normals);
			}
			else
			{
				assert(is_bone(nodes[i]));

				this->extract_bone(nodes[i]);
			}
		}
	}

	void meshml_extractor::extract_object(INode* node, bool flip_normals)
	{
		assert(is_mesh(node));

		object_info_t obj_info;

		obj_info.name = tstr_to_str(node->GetName());

		Matrix3 obj_matrix = node->GetObjectTM(0);
		Point3 Row0 = obj_matrix.GetRow(0);
		Point3 Row1 = obj_matrix.GetRow(1);
		Point3 Row2 = obj_matrix.GetRow(2);
		if ((Row0 ^ Row1) % Row2 <= 0)
		{
			flip_normals = !flip_normals;
		}
		Matrix3 normal_matrix = obj_matrix;
		normal_matrix.NoTrans();

		std::vector<std::pair<Point3, binds_t> > positions;
		std::vector<Point3> normals;
		std::map<int, std::vector<Point2> > texs;
		std::vector<int> pos_indices;
		std::map<int, std::vector<int> > tex_indices;
		std::map<int, Matrix2> uv_transs;

		Mtl* mtl = node->GetMtl();
		if (mtl != NULL)
		{
			if ((Class_ID(DMTL_CLASS_ID, 0) == mtl->ClassID()) && (0 == mtl->NumSubMtls()))
			{
				for (int i = 0; i < mtl->NumSubTexmaps(); ++ i)
				{
					Texmap* tex_map = mtl->GetSubTexmap(i);
					if (tex_map != NULL)
					{
						if (Class_ID(BMTEX_CLASS_ID, 0) == tex_map->ClassID())
						{
							BitmapTex* bitmap_tex = static_cast<BitmapTex*>(tex_map);

							Matrix3 uv_mat;
							tex_map->GetUVTransform(uv_mat);
							Matrix2 uv_trans(TRUE);
							uv_trans.SetRow(0, Point2(uv_mat.GetRow(0)[0], uv_mat.GetRow(0)[1]));
							uv_trans.SetRow(1, Point2(uv_mat.GetRow(1)[0], uv_mat.GetRow(1)[1]));
							uv_trans.SetRow(2, Point2(uv_mat.GetRow(2)[0], uv_mat.GetRow(2)[1]));

							obj_info.texture_slots.push_back(texture_slot_t(tstr_to_str(mtl->GetSubTexmapSlotName(i).data()),
								tstr_to_str(bitmap_tex->GetMapName())));
							uv_transs[i] = uv_trans;
						}
					}
				}
			}
		}

		Object* obj = node->EvalWorldState(0).obj;
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
			mesh.buildNormals();

			obj_info.triangles.resize(mesh.getNumFaces());
			for (size_t i = 0; i < obj_info.triangles.size(); ++ i)
			{
				for (int j = 2; j >= 0; -- j)
				{
					pos_indices.push_back(mesh.faces[i].v[j]);
				}
			}
			for (size_t i = 0; i < mesh.getNumVerts(); ++ i)
			{
				positions.push_back(std::make_pair(mesh.verts[i], binds_t()));
				normals.push_back(mesh.gfxNormals[i]);
			}

			for (int channel = 1; channel < MAX_MESHMAPS; channel ++)
			{
				if (mesh.mapSupport(channel))
				{
					const int num_map_verts = mesh.getNumMapVerts(channel);
					if (num_map_verts > 0)
					{
						texs[channel].resize(num_map_verts);

						UVVert* uv_verts = mesh.mapVerts(channel);
						for (size_t i = 0; i < texs[channel].size(); ++ i)
						{
							texs[channel][i].x = uv_verts[i].x;
							texs[channel][i].y = uv_verts[i].y;
						}

						TVFace* tv_faces = mesh.mapFaces(channel);
						for (size_t i = 0; i < obj_info.triangles.size(); ++ i)
						{
							for (int j = 2; j >= 0; -- j)
							{
								tex_indices[channel].push_back(tv_faces[i].t[j]);
							}
						}
					}
				}
			}

			if (need_delete)
			{
				delete tri;
			}
		}

		std::set<vertex_index_t> vertex_indices;
		for (size_t i = 0; i < obj_info.triangles.size(); ++ i)
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
				for (std::map<int, std::vector<int> >::iterator iter = tex_indices.begin();
					iter != tex_indices.end(); ++ iter)
				{
					vertex_index.tex_indices.push_back(iter->second[offset]);
				}

				std::set<vertex_index_t>::iterator v_iter = vertex_indices.find(vertex_index);
				if (v_iter != vertex_indices.end())
				{
					v_iter->ref_triangle.push_back(i * 3 + j);
				}
				else
				{
					vertex_index.ref_triangle.resize(1, i * 3 + j);
					vertex_indices.insert(vertex_index);
				}
			}
		}

		this->physique_modifier(node, "root", positions);
		this->skin_modifier(node, "root", positions);

		if (joints_.empty())
		{
			joint_t root;
			root.pos = Point3(0, 0, 0);
			root.quat.Identity();
			root.parent_name = "";
			joints_.insert(std::make_pair("root", root));
		}

		for (std::vector<std::pair<Point3, binds_t> >::iterator iter = positions.begin();
			iter != positions.end(); ++ iter)
		{
			binds_t binds = iter->second;

			if (binds.size() > joints_per_ver_)
			{
				std::nth_element(binds.begin(), binds.begin() + joints_per_ver_, binds.end(), bind_cmp);
				binds.resize(joints_per_ver_);

				float totalweight = 0;
				for (int j = 0; j < joints_per_ver_; ++ j)
				{
					totalweight += binds[j].second;
				}

				for (int j = 0; j < joints_per_ver_; ++ j)
				{
					binds[j].second /= totalweight;
				}
			}
			else
			{
				for (int j = static_cast<int>(binds.size()); j < joints_per_ver_; ++ j)
				{
					binds.push_back(std::make_pair("root", 0));
				}
			}

			iter->second = binds;
		}

		obj_info.vertices.resize(vertex_indices.size());
		int ver_index = 0;
		for (std::set<vertex_index_t>::iterator iter = vertex_indices.begin();
			iter != vertex_indices.end(); ++ iter, ++ ver_index)
		{
			Point3 pos = positions[iter->pos_index].first * obj_matrix;
			obj_info.vertices[ver_index].pos.x = pos.x;
			obj_info.vertices[ver_index].pos.y = pos.z;
			obj_info.vertices[ver_index].pos.z = pos.y;

			Point3 normal = normals[iter->pos_index] * normal_matrix;
			if (flip_normals)
			{
				normal = -normal;
			}
			normal = normal.Normalize();
			obj_info.vertices[ver_index].normal.x = normal.x;
			obj_info.vertices[ver_index].normal.y = normal.z;
			obj_info.vertices[ver_index].normal.z = normal.y;

			int uv_layer = 0;
			for (std::map<int, std::vector<Point2> >::iterator uv_iter = texs.begin();
				uv_iter != texs.end(); ++ uv_iter, ++ uv_layer)
			{
				Point2 tex = uv_iter->second[iter->tex_indices[uv_layer]];
				if (uv_transs.find(uv_layer) != uv_transs.end())
				{
					tex = tex * uv_transs[uv_layer];
				}

				obj_info.vertices[ver_index].tex.push_back(Point2(tex.x, 1 - tex.y));
			}

			for (size_t i = 0; i < iter->ref_triangle.size(); ++ i)
			{
				obj_info.triangles[iter->ref_triangle[i] / 3].vertex_index[iter->ref_triangle[i] % 3] = ver_index;
			}

			obj_info.vertices[ver_index].binds = positions[iter->pos_index].second;
		}

		obj_info.vertex_elements.push_back(vertex_element_t(VEU_Position, 0, 3));
		obj_info.vertex_elements.push_back(vertex_element_t(VEU_Normal, 0, 3));
		for (size_t i = 0; i < obj_info.vertices[0].tex.size(); ++ i)
		{
			obj_info.vertex_elements.push_back(vertex_element_t(VEU_TextureCoord, static_cast<unsigned char>(i), 2));
		}

		objs_info_.push_back(obj_info);
	}

	void meshml_extractor::extract_bone(INode* node)
	{
		assert(is_bone(node));

		int tpf = GetTicksPerFrame();
		int start_tick = start_frame_ * tpf;
		int end_tick = end_frame_ * tpf;
		int tps = frame_rate_ * tpf;

		key_frame_t kf;
		kf.joint = tstr_to_str(node->GetName());

		for (int i = start_tick; i != end_tick; i += tps)
		{
			Matrix3 tm = node->GetNodeTM(i);

			kf.positions.push_back(point_from_matrix(tm));
			kf.quaternions.push_back(quat_from_matrix(tm));
		}

		kfs_.push_back(kf);
	}

	Point3 meshml_extractor::point_from_matrix(Matrix3 const & mat)
	{
		Point3 pos(mat.GetTrans());
		std::swap(pos.y, pos.z);

		return pos;
	}

	Quat meshml_extractor::quat_from_matrix(Matrix3 const & mat)
	{
		Quat quat(mat);
		std::swap(quat.y, quat.z);

		return quat;
	}

	Modifier* meshml_extractor::find_modifier(INode* node, Class_ID const & class_id)
	{
		Object* obj_ref = node->GetObjectRef();
		while ((obj_ref != NULL) && (obj_ref->SuperClassID() == GEN_DERIVOB_CLASS_ID))
		{
			IDerivedObject* DerivedObjectPtr = static_cast<IDerivedObject*>(obj_ref);

			// Iterate over all entries of the modifier stack.
			for (int mod_stack_index = 0; mod_stack_index < DerivedObjectPtr->NumModifiers(); ++ mod_stack_index)
			{
				Modifier* mod = DerivedObjectPtr->GetModifier(mod_stack_index);
				if (class_id == mod->ClassID())
				{
					return mod;
				}
			}

			obj_ref = DerivedObjectPtr->GetObjRef();
		}

		return NULL;
	}

	void meshml_extractor::physique_modifier(INode* node, std::string const & root_name,
										std::vector<std::pair<Point3, binds_t> >& positions)
	{
		Modifier* mod = this->find_modifier(node, Class_ID(PHYSIQUE_CLASS_ID_A, PHYSIQUE_CLASS_ID_B));
		if (mod != NULL)
		{
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

									INode* joint_node = phy_rigid_ver->GetNode();
									std::string joint_name = tstr_to_str(joint_node->GetName());

									joints_t::iterator joint_iter = joints_.find(joint_name);
									if (joint_iter == joints_.end())
									{
										Matrix3 tm = joint_node->GetNodeTM(0);

										joint_t joint;
										joint.pos = this->point_from_matrix(tm);
										joint.quat = this->quat_from_matrix(tm);

										INode* parent_node = joint_node->GetParentNode();
										if (is_bone(parent_node))
										{
											joint.parent_name = tstr_to_str(parent_node->GetName());
										}
										else
										{
											joint.parent_name = root_name;
										}

										joints_[joint_name] = joint;
									}

									for (binds_t::iterator iter = positions[i].second.begin();
										iter != positions[i].second.end(); ++ iter)
									{
										if (iter->first == joint_name)
										{
											iter->second = 1;
										}
									}
								}
								break;

							case RIGID_BLENDED_TYPE:
								{
									IPhyBlendedRigidVertex* phy_blended_rigid_ver = static_cast<IPhyBlendedRigidVertex*>(phy_ver_exp);

									for (int i = 0; i < phy_blended_rigid_ver->GetNumberNodes(); ++ i)
									{
										INode* joint_node = phy_blended_rigid_ver->GetNode(i);
										std::string joint_name = tstr_to_str(joint_node->GetName());

										joints_t::iterator joint_iter = joints_.find(joint_name);
										if (joint_iter == joints_.end())
										{
											Matrix3 tm = joint_node->GetNodeTM(0);

											joint_t joint;
											joint.pos = this->point_from_matrix(tm);
											joint.quat = this->quat_from_matrix(tm);

											INode* parent_node = joint_node->GetParentNode();
											if (is_bone(parent_node))
											{
												joint.parent_name = tstr_to_str(parent_node->GetName());
											}
											else
											{
												joint.parent_name = root_name;
											}

											joints_[joint_name] = joint;
										}

										for (binds_t::iterator iter = positions[i].second.begin();
											iter != positions[i].second.end(); ++ iter)
										{
											if (iter->first == joint_name)
											{
												iter->second = phy_blended_rigid_ver->GetWeight(i);
											}
										}
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
	}

	void meshml_extractor::skin_modifier(INode* node, std::string const & root_name,
									std::vector<std::pair<Point3, binds_t> >& positions)
	{
		Modifier* mod = this->find_modifier(node, SKIN_CLASSID);
		if (mod != NULL)
		{
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
							INode* joint_node = skin->GetBone(skin_cd->GetAssignedBone(i, j));
							std::string const joint_name = tstr_to_str(joint_node->GetName());
							for (binds_t::iterator iter = positions[i].second.begin();
								iter != positions[i].second.end(); ++ iter)
							{
								if (iter->first == joint_name)
								{
									iter->second = skin_cd->GetBoneWeight(i, j);
								}
							}
						}
					}

					for (int i = 0; i < skin->GetNumBones(); ++ i)
					{
						INode* joint_node = skin->GetBone(i);
						std::string joint_name = tstr_to_str(joint_node->GetName());

						Matrix3 joint_init_mat;
						skin->GetBoneInitTM(joint_node, joint_init_mat, false);

						joints_t::iterator joint_iter = joints_.find(joint_name);
						if (joint_iter == joints_.end())
						{
							Matrix3 tm = joint_node->GetNodeTM(0);

							joint_t joint;
							joint.pos = this->point_from_matrix(tm);
							joint.quat = this->quat_from_matrix(tm);

							INode* parent_node = joint_node->GetParentNode();
							if (is_bone(parent_node))
							{
								joint.parent_name = tstr_to_str(parent_node->GetName());
							}
							else
							{
								joint.parent_name = root_name;
							}

							joints_[joint_name] = joint;
						}
					}
				}

				mod->ReleaseInterface(I_SKIN, skin);
			}
		}
	}

	void meshml_extractor::remove_redundant_joints()
	{
		std::set<std::string> joints_used;
		for (size_t i = 0; i < objs_info_.size(); ++ i)
		{
			for (size_t j = 0; j < objs_info_[i].vertices.size(); ++ j)
			{
				for (binds_t::iterator k = objs_info_[i].vertices[j].binds.begin();
					k != objs_info_[i].vertices[j].binds.end(); ++ k)
				{
					joints_used.insert(k->first);
				}
			}
		}

		for (joints_t::iterator iter = joints_.begin(); iter != joints_.end(); ++ iter)
		{
			if (joints_used.find(iter->first) != joints_used.end())
			{
				joints_used.insert(iter->second.parent_name);
			}
		}

		for (joints_t::iterator iter = joints_.begin(); iter != joints_.end();)
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

	void meshml_extractor::write_xml(std::basic_string<TCHAR> const & file_name)
	{
		std::ofstream ofs(tstr_to_str(file_name).c_str());
		if (!ofs)
		{
			return;
		}

		this->remove_redundant_joints();

		std::map<std::string, int> joints_name_to_id;
		std::vector<std::string> joints_id_to_name;
		{
			for (joints_t::const_iterator iter = joints_.begin(); iter != joints_.end(); ++ iter)
			{
				joints_id_to_name.push_back(iter->first);
			}

			bool swapped = true;
			while (swapped)
			{
				swapped = false;
				for (int i = 0; i < joints_id_to_name.size(); ++ i)
				{
					int par_index = -1;
					if (!joints_[joints_id_to_name[i]].parent_name.empty())
					{
						std::vector<std::string>::iterator par_iter = std::find(joints_id_to_name.begin(), joints_id_to_name.end(),
							joints_[joints_id_to_name[i]].parent_name);
						assert(par_iter != joints_id_to_name.end());
						par_index = par_iter - joints_id_to_name.begin();
					}

					if (par_index > i)
					{
						std::swap(joints_id_to_name[i], joints_id_to_name[par_index]);
						swapped = true;
						break;
					}
				}
			}

			for (int i = 0; i < joints_id_to_name.size(); ++ i)
			{
				joints_name_to_id.insert(std::make_pair(joints_id_to_name[i], i));
			}
		}

		using std::endl;

		ofs << "<?xml version=\'1.0\' encoding=\'utf-8\' standalone=\'no\'?>" << endl;
		ofs << "<!DOCTYPE Model SYSTEM \'model.dtd\'>" << endl << endl;
		ofs << "<model version=\'2\'>" << endl;

		ofs << "\t<bones_chunk>" << endl;
		for (std::vector<std::string>::const_iterator iter = joints_id_to_name.begin(); iter != joints_id_to_name.end(); ++ iter)
		{
			int parent_id = -1;
			if (!joints_[*iter].parent_name.empty())
			{
				assert(joints_name_to_id.find(joints_[*iter].parent_name) != joints_name_to_id.end());
				parent_id = joints_name_to_id[joints_[*iter].parent_name];
			}

			ofs << "\t\t<bone name=\'" << *iter
				<< "\' parent=\'" << parent_id
				<< "\'>" << endl;

			joint_t& joint = joints_[*iter];

			ofs << "\t\t\t<bind_pos x=\'" << joint.pos.x
				<< "\' y=\'" << joint.pos.y
				<< "\' z=\'" << joint.pos.z << "\'/>" << endl;

			ofs << "\t\t\t<bind_quat x=\'" << joint.quat.x
				<< "\' y=\'" << joint.quat.y
				<< "\' z=\'" << joint.quat.z
				<< "\' w=\'" << joint.quat.w << "\'/>" << endl;

			ofs << "\t\t</bone>" << endl;
		}
		ofs << "\t</bones_chunk>" << endl << endl;

		ofs << "\t<meshes_chunk>" << endl;
		for (objects_info_t::const_iterator oi_iter = objs_info_.begin(); oi_iter != objs_info_.end(); ++ oi_iter)
		{
			ofs << "\t\t<mesh name=\'" + oi_iter->name + "\'>" << endl;

			ofs << "\t\t\t<vertex_elements_chunk>" << endl;
			for (vertex_elements_t::const_iterator ve_iter = oi_iter->vertex_elements.begin();
				ve_iter != oi_iter->vertex_elements.end(); ++ ve_iter)
			{
				ofs << "\t\t\t\t<vertex_element usage=\'" << ve_iter->usage
					<< "\' usage_index=\'" << int(ve_iter->usage_index)
					<< "\' num_components=\'" << int(ve_iter->num_components) << "\'/>" << endl;
			}
			ofs << "\t\t\t</vertex_elements_chunk>" << endl << endl;

			ofs << "\t\t\t<textures_chunk>" << endl;
			for (texture_slots_t::const_iterator ts_iter = oi_iter->texture_slots.begin();
				ts_iter != oi_iter->texture_slots.end(); ++ ts_iter)
			{
				ofs << "\t\t\t\t<texture type=\'" << ts_iter->first
					<< "\' name=\'" << ts_iter->second << "\'/>" << endl;
			}
			ofs << "\t\t\t</textures_chunk>" << endl << endl;

			ofs << "\t\t\t<vertices_chunk>" << endl;
			for (vertices_t::const_iterator v_iter = oi_iter->vertices.begin();
				v_iter != oi_iter->vertices.end(); ++ v_iter)
			{
				ofs << "\t\t\t\t<vertex x=\'" << v_iter->pos.x
					<< "\' y=\'" << v_iter->pos.y
					<< "\' z=\'" << v_iter->pos.z << "\'>" << endl;

				ofs << "\t\t\t\t\t<normal x=\'" << v_iter->normal.x
					<< "\' y=\'" << v_iter->normal.y
					<< "\' z=\'" << v_iter->normal.z << "\'/>" << endl;

				for (std::vector<Point2>::const_iterator t_iter = v_iter->tex.begin();
					t_iter != v_iter->tex.end(); ++ t_iter)
				{
					ofs << "\t\t\t\t\t<tex_coord u=\'" << t_iter->x
						<< "\' v=\'" << t_iter->y << "\'/>" << endl;
				}

				for (binds_t::const_iterator b_iter = v_iter->binds.begin();
					b_iter != v_iter->binds.end(); ++ b_iter)
				{
					assert(joints_name_to_id.find(b_iter->first) != joints_name_to_id.end());
					ofs << "\t\t\t\t\t<weight bone_index=\'" << joints_name_to_id[b_iter->first]
						<< "\' weight=\'" << b_iter->second << "\'/>" << endl;
				}

				ofs << "\t\t\t\t</vertex>" << endl;
			}
			ofs << "\t\t\t</vertices_chunk>" << endl << endl;

			ofs << "\t\t\t<triangles_chunk>" << endl;
			for (triangles_t::const_iterator t_iter = oi_iter->triangles.begin();
				t_iter != oi_iter->triangles.end(); ++ t_iter)
			{
				ofs << "\t\t\t\t<triangle a=\'" << t_iter->vertex_index[0]
					<< "\' b=\'" << t_iter->vertex_index[1]
					<< "\' c=\'" << t_iter->vertex_index[2] << "\'/>" << endl;
			}
			ofs << "\t\t\t</triangles_chunk>" << endl;

			ofs << "\t\t</mesh>" << endl;
		}
		ofs << "\t</meshes_chunk>" << endl;

		ofs << "\t<key_frames_chunk start_frame=\'" << start_frame_
			<< "\' end_frame=\'" << end_frame_
			<< "\' frame_rate=\'" << frame_rate_ << "\'>" << endl;
		for (key_frames_t::const_iterator iter = kfs_.begin(); iter != kfs_.end(); ++ iter)
		{
			assert(iter->positions.size() == iter->quaternions.size());

			ofs << "\t\t<key_frame joint=\'" << iter->joint << "\'>" << endl;

			for (size_t i = 0; i < iter->positions.size(); ++ i)
			{
				ofs << "\t\t\t<key>" << endl;

				ofs << "\t\t\t\t<pos x=\'" << iter->positions[i].x
					<< "\' y=\'" << iter->positions[i].y
					<< "\' z=\'" << iter->positions[i].z << "\'/>" << endl;

				ofs << "\t\t\t\t<quat x=\'" << iter->quaternions[i].x
					<< "\' y=\'" << iter->quaternions[i].y
					<< "\' z=\'" << iter->quaternions[i].z
					<< "\' w=\'" << iter->quaternions[i].w << "\'/>" << endl;

				ofs << "\t\t\t</key>" << endl;
			}

			ofs << "\t\t</key_frame>" << endl;
		}
		ofs << "\t</key_frames_chunk>" << endl << endl;

		ofs << "</model>" << endl;
	}
}
