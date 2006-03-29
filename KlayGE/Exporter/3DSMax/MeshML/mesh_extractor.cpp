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
}

namespace KlayGE
{
	meshml_extractor::meshml_extractor()
	{
	}

	void meshml_extractor::export_objects(std::vector<INode*> const & nodes, bool flip_normals)
	{
		for (size_t i = 0; i < nodes.size(); ++ i)
		{
			this->extract_object(nodes[i], flip_normals);
		}
	}

	void meshml_extractor::extract_object(INode* node, bool flip_normals)
	{
		if (!is_mesh(node))
		{
			return;
		}

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

		std::vector<Point3> positions;
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
				positions.push_back(mesh.verts[i]);
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

		obj_info.vertices.resize(vertex_indices.size());
		int ver_index = 0;
		for (std::set<vertex_index_t>::iterator iter = vertex_indices.begin();
			iter != vertex_indices.end(); ++ iter, ++ ver_index)
		{
			Point3 pos = positions[iter->pos_index] * obj_matrix;
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
		}

		obj_info.vertex_elements.push_back(vertex_element_t(VEU_Position, 0, 3));
		obj_info.vertex_elements.push_back(vertex_element_t(VEU_Normal, 0, 3));
		for (size_t i = 0; i < obj_info.vertices[0].tex.size(); ++ i)
		{
			obj_info.vertex_elements.push_back(vertex_element_t(VEU_TextureCoord, static_cast<unsigned char>(i), 2));
		}

		objs_info_.push_back(obj_info);
	}

	void meshml_extractor::write_xml(std::basic_string<TCHAR> const & file_name)
	{
		std::ofstream ofs(tstr_to_str(file_name).c_str());
		if (!ofs)
		{
			return;
		}

		using std::endl;

		ofs << "<?xml version=\'1.0\' encoding=\'utf-8\' standalone=\'no\'?>" << endl;
		ofs << "<!DOCTYPE Model SYSTEM \'model.dtd\'>" << endl << endl;
		ofs << "<model version=\'2\'>" << endl;

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
		ofs << "</model>" << endl;
	}
}
