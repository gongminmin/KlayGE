// MeshExtractor.cpp
// KlayGE MeshML数据导出类 实现文件
// Ver 3.11.0
// 版权所有(C) 龚敏敏, 2003-2010
// Homepage: http://www.klayge.org
//
// 3.11.0
// 初次建立 (王锐 2011.2.28)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include "MeshExtractor.hpp"
#include <set>
#include <sstream>
#include <algorithm>

namespace KlayGE
{
	struct MaterialIDSortOp
	{
		bool operator()(MeshExtractor::MeshStruct const & lhs, MeshExtractor::MeshStruct const & rhs) const
		{
			return lhs.material_id < rhs.material_id;
		}
	};

	MeshExtractor::MeshExtractor(std::ostream* out)
		: start_frame_(0), end_frame_(0), frame_rate_(25),
			vertexExportSettings_(VES_ALL), userExportSettings_(UES_ALL)
	{
		this->Output(out);
	}

	void MeshExtractor::OptimizeData()
	{
		this->OptimizeJoints();
		this->OptimizeMaterials();
		this->OptimizeMeshes();
	}

	void MeshExtractor::WriteMeshML(int modelVersion, std::string const & encoding)
	{
		if (NULL == output_stream_)
		{
			return;
		}

		std::ostream& os = *output_stream_;

		// Initialize the xml document
		os << "<?xml version=\"1.0\" ";
		if (!encoding.empty())
		{
			os << "encoding=\"" << encoding << "\"";
		}
		os << "?>" << std::endl << std::endl;
		os << "<model version=\"" << modelVersion << "\">" << std::endl;

		if (!joints_.empty())
		{
			this->WriteJointChunk();
		}
		if (!obj_materials_.empty())
		{
			this->WriteMaterialChunk();
		}
		if (!obj_meshes_.empty())
		{
			this->WriteMeshChunk();
		}
		if (!keyframes_.empty())
		{
			this->WriteKeyframeChunk();
		}

		// Finish the writing process
		os << "</model>" << std::endl;
	}

	void MeshExtractor::WriteJointChunk()
	{
		// Ready to sort joints
		std::vector<std::string> sorted_joints;
		for (JointMap::iterator itr = joints_.begin(); itr != joints_.end(); ++ itr)
		{
			sorted_joints.push_back(itr->first);
		}

		// Traverse joints and sort them according to names & parent names
		bool swapping = true;
		JointMap::iterator fitr = joints_.begin();
		while (swapping)
		{
			swapping = false;
			for (size_t i = 0; i < sorted_joints.size(); ++ i)
			{
				int parent_index = -1;
				fitr = joints_.find(sorted_joints[i]);
				if (fitr == joints_.end())
				{
					continue;
				}

				if (!fitr->second.parent_name.empty())
				{
					std::vector<std::string>::iterator sitr =
						std::find(sorted_joints.begin(), sorted_joints.end(), fitr->second.parent_name );
					if (sitr != sorted_joints.end())
					{
						parent_index = static_cast<int>(sitr - sorted_joints.begin());
					}
				}

				if (parent_index > static_cast<int>(i))
				{
					std::swap(sorted_joints[i], sorted_joints[parent_index]);
					swapping = true;
					break;
				}
			}
		}

		// Set joint IDs automatically
		for (size_t i = 0; i < sorted_joints.size(); ++ i)
		{
			fitr = joints_.find(sorted_joints[i]);
			if (fitr != joints_.end())
			{
				fitr->second.joint_id = static_cast<int>(i);
			}
		}

		std::ostream& os = *output_stream_;

		// Write joints out
		os << "\t<bones_chunk>" << std::endl;
		for (size_t i=0; i<sorted_joints.size(); ++i)
		{
			fitr = joints_.find(sorted_joints[i]);
			if (fitr == joints_.end()) continue;

			JointStruct& joint = fitr->second;
			os << "\t\t<bone name=\"" << fitr->first;

			int parent_id = -1;
			fitr = joints_.find(joint.parent_name);
			if (fitr != joints_.end())
				parent_id = fitr->second.joint_id;

			os << "\" parent=\"" << parent_id << "\">" << std::endl;
			os << "\t\t\t<bind_real x=\"" << joint.bind_real[0]
				<< "\" y=\"" << joint.bind_real[1]
				<< "\" z=\"" << joint.bind_real[2]
				<< "\" w=\"" << joint.bind_real[3] << "\"/>" << std::endl;
			os << "\t\t\t<bind_dual x=\"" << joint.bind_dual[0]
				<< "\" y=\"" << joint.bind_dual[1]
				<< "\" z=\"" << joint.bind_dual[2]
				<< "\" w=\"" << joint.bind_dual[3] << "\"/>" << std::endl;
			os << "\t\t</bone>" << std::endl;
		}
		os << "\t</bones_chunk>" << std::endl;
	}

	void MeshExtractor::WriteMaterialChunk()
	{
		std::ostream& os = *output_stream_;

		os << "\t<materials_chunk>" << std::endl;
		for (size_t i=0; i<obj_materials_.size(); ++i)
		{
			MaterialStruct& mtl = obj_materials_[i];
			os << "\t\t<material ambient_r=\"" << mtl.ambient[0]
				<< "\" ambient_g=\"" << mtl.ambient[1]
				<< "\" ambient_b=\"" << mtl.ambient[2]
				<< "\" diffuse_r=\"" << mtl.diffuse[0]
				<< "\" diffuse_g=\"" << mtl.diffuse[1]
				<< "\" diffuse_b=\"" << mtl.diffuse[2]
				<< "\" specular_r=\"" << mtl.specular[0]
				<< "\" specular_g=\"" << mtl.specular[1]
				<< "\" specular_b=\"" << mtl.specular[2]
				<< "\" emit_r=\"" << mtl.emit[0]
				<< "\" emit_g=\"" << mtl.emit[1]
				<< "\" emit_b=\"" << mtl.emit[2]
				<< "\" opacity=\"" << mtl.opacity
				<< "\" specular_level=\"" << mtl.specular_level
				<< "\" shininess=\"" << mtl.shininess << "\">" << std::endl;

			if (!mtl.texture_slots.empty())
			{
				os << "\t\t\t<textures_chunk>" << std::endl;
				for (std::vector<TextureSlot>::iterator itr=mtl.texture_slots.begin();
					itr!=mtl.texture_slots.end(); ++itr)
				{
					os << "\t\t\t\t<texture type=\"" << itr->first
						<< "\" name=\"" << itr->second << "\"/>" << std::endl;
				}
				os << "\t\t\t</textures_chunk>" << std::endl;
			}
			os << "\t\t</material>" << std::endl;
		}
		os << "\t</materials_chunk>" << std::endl;
	}

	void MeshExtractor::WriteMeshChunk()
	{
		std::ostream& os = *output_stream_;

		os << "\t<meshes_chunk>" << std::endl;
		for (size_t i=0; i<obj_meshes_.size(); ++i)
		{
			MeshStruct& mesh = obj_meshes_[i];
			os << "\t\t<mesh name=\"" << mesh.name
				<< "\" mtl_id=\"" << mesh.material_id << "\">" << std::endl;

			os << "\t\t\t<vertices_chunk>" << std::endl;
			for (std::vector<VertexStruct>::iterator itr = mesh.vertices.begin();
				itr != mesh.vertices.end(); ++ itr)
			{
				VertexStruct& vertex = (*itr);
				os << "\t\t\t\t<vertex x=\"" << vertex.position[0]
					<< "\" y=\"" << vertex.position[1]
					<< "\" z=\"" << vertex.position[2] << "\">" << std::endl;

				if (vertexExportSettings_ & VES_NORMAL)
				{
					os << "\t\t\t\t\t<normal x=\"" << vertex.normal[0]
						<< "\" y=\"" << vertex.normal[1]
						<< "\" z=\"" << vertex.normal[2] << "\"/>" << std::endl;
				}

				if (vertexExportSettings_ & VES_TANGENT)
				{
					os << "\t\t\t\t\t<tangent x=\"" << vertex.tangent[0]
						<< "\" y=\"" << vertex.tangent[1]
						<< "\" z=\"" << vertex.tangent[2] << "\"/>" << std::endl;
				}

				if (vertexExportSettings_ & VES_BINORMAL)
				{
					os << "\t\t\t\t\t<binormal x=\"" << vertex.binormal[0]
						<< "\" y=\"" << vertex.binormal[1]
						<< "\" z=\"" << vertex.binormal[2] << "\"/>" << std::endl;
				}

				if (vertexExportSettings_ & VES_TEXCOORD)
				{
					switch (vertex.texcoord_components)
					{
					case 1:
						for (std::vector<TexCoord>::iterator titr = vertex.texcoords.begin();
							titr != vertex.texcoords.end(); ++ titr)
						{
							os << "\t\t\t\t\t<tex_coord u=\"" << (*titr)[0] << "\"/>" << std::endl;
						}
						break;

					case 2:
						for (std::vector<TexCoord>::iterator titr = vertex.texcoords.begin();
							titr != vertex.texcoords.end(); ++ titr)
						{
							os << "\t\t\t\t\t<tex_coord u=\"" << (*titr)[0]
								<< "\" v=\"" << (*titr)[1] << "\"/>" << std::endl;
						}
						break;

					case 3:
						for (std::vector<TexCoord>::iterator titr = vertex.texcoords.begin();
							titr != vertex.texcoords.end(); ++ titr)
						{
							os << "\t\t\t\t\t<tex_coord u=\"" << (*titr)[0] << "\" v=\"" << (*titr)[1]
								<< "\" w=\"" << (*titr)[2] << "\"/>" << std::endl;
						}
						break;

					default:
						break;
					}
				}

				for (std::vector<JointBinding>::iterator jitr = vertex.binds.begin();
					jitr != vertex.binds.end(); ++ jitr)
				{
					JointMap::iterator fitr = joints_.find(jitr->first);
					if (fitr == joints_.end())
					{
						continue;
					}

					os << "\t\t\t\t\t<weight bone_index=\"" << fitr->second.joint_id
						<< "\" weight=\"" << jitr->second << "\"/>" << std::endl;
				}
				os << "\t\t\t\t</vertex>" << std::endl;
			}
			os << "\t\t\t</vertices_chunk>" << std::endl;

			os << "\t\t\t<triangles_chunk>" << std::endl;
			for (std::vector<TriangleStruct>::iterator itr = mesh.triangles.begin();
				itr != mesh.triangles.end(); ++ itr)
			{
				TriangleStruct& tri = *itr;
				os << "\t\t\t\t<triangle a=\"" << tri.vertex_index[0]
					<< "\" b=\"" << tri.vertex_index[1]
					<< "\" c=\"" << tri.vertex_index[2] << "\"/>" << std::endl;
			}
			os << "\t\t\t</triangles_chunk>" << std::endl;

			os << "\t\t</mesh>" << std::endl;
		}
		os << "\t</meshes_chunk>" << std::endl;
	}

	void MeshExtractor::WriteKeyframeChunk()
	{
		std::ostream& os = *output_stream_;

		os << "\t<key_frames_chunk start_frame=\"" << start_frame_
			<< "\" end_frame=\"" << end_frame_
			<< "\" frame_rate=\"" << frame_rate_ << "\">" << std::endl;
		for (size_t i = 0; i < keyframes_.size(); ++ i)
		{
			KeyframeStruct& kf = keyframes_[i];
			size_t frames = std::min<size_t>(kf.bind_reals.size(), kf.bind_duals.size());

			os << "\t\t<key_frame joint=\"" << kf.joint << "\">" << std::endl;
			for (size_t j = 0; j < frames; ++ j)
			{
				Quat const & bind_real = kf.bind_duals[j];
				Quat const & bind_dual = kf.bind_duals[j];

				os << "\t\t\t<key>" << std::endl;
				os << "\t\t\t\t<bind_real x=\"" << bind_real[0]
					<< "\" y=\"" << bind_real[1]
					<< "\" z=\"" << bind_real[2]
					<< "\" w=\"" << bind_real[3] << "\"/>" << std::endl;
				os << "\t\t\t\t<bind_dual x=\"" << bind_dual[0]
					<< "\" y=\"" << bind_dual[1]
					<< "\" z=\"" << bind_dual[2]
					<< "\" w=\"" << bind_dual[3] << "\"/>" << std::endl;
				os << "\t\t\t</key>" << std::endl;
			}
			os << "\t\t</key_frame>" << std::endl;
		}
		os << "\t</key_frames_chunk>" << std::endl;
	}

	void MeshExtractor::OptimizeJoints()
	{
		std::set<std::string> joints_used;

		// Find all joints used in the mesh list
		for (size_t i = 0; i < obj_meshes_.size(); ++ i)
		{
			MeshStruct& mesh = obj_meshes_[i];
			for (std::vector<VertexStruct>::iterator itr = mesh.vertices.begin();
				itr != mesh.vertices.end(); ++ itr)
			{
				VertexStruct& vertex = *itr;
				for (std::vector<JointBinding>::iterator jitr = vertex.binds.begin();
					jitr != vertex.binds.end(); ++ jitr)
				{
					joints_used.insert(jitr->first);
				}
			}
		}

		// Traverse the joint list and see if used joints' parents can be added
		for (JointMap::iterator itr = joints_.begin(); itr != joints_.end(); ++ itr)
		{
			if (joints_used.find(itr->first) == joints_used.end())
			{
				continue;
			}

			JointMap::iterator fitr = joints_.find(itr->second.parent_name);
			while (fitr != joints_.end())
			{
				joints_used.insert(fitr->second.parent_name);
				fitr = joints_.find(fitr->second.parent_name);
			}
		}

		// Traverse the joint list and erase those never recorded by joints_used
		for (JointMap::iterator itr = joints_.begin(); itr != joints_.end();)
		{
			if (joints_used.find(itr->first) == joints_used.end())
			{
				joints_.erase(itr ++);
			}
			else
			{
				++ itr;
			}
		}
	}

	void MeshExtractor::OptimizeMaterials()
	{
		std::vector<size_t> mtl_mapping(obj_materials_.size());
		std::vector<MaterialStruct> mtls_used;

		// Traverse materials and setup IDs
		for (size_t i = 0; i < obj_materials_.size(); ++ i)
		{
			std::vector<MaterialStruct>::iterator fitr =
				std::find(mtls_used.begin(), mtls_used.end(), obj_materials_[i]);
			if (fitr == mtls_used.end())
			{
				mtl_mapping[i] = mtls_used.size();
				mtls_used.push_back(obj_materials_[i]);
			}
			else
			{
				mtl_mapping[i] = fitr - mtls_used.begin();
			}
		}

		// Rebuild materials and mesh IDs
		obj_materials_ = mtls_used;
		for (size_t i = 0; i < obj_meshes_.size(); ++ i)
		{
			size_t mtl_id = obj_meshes_[i].material_id;
			if (mtl_id < mtl_mapping.size())
			{
				obj_meshes_[i].material_id = mtl_mapping[mtl_id];
			}
			else
			{
				obj_meshes_[i].material_id = 0;
			}
		}
	}

	void MeshExtractor::OptimizeMeshes()
	{
		if (userExportSettings_ & UES_COMBINE_MESHES)
		{
			std::set<size_t> meshid_to_remove;
			std::vector<MeshStruct> meshes_finished;
			for (size_t i = 0; i < obj_materials_.size(); ++ i)
			{
				// Find all meshes sharing one material
				std::vector<MeshStruct> meshes_to_combine;
				for (size_t j=0; j<obj_meshes_.size(); ++j)
				{
					if (obj_meshes_[j].material_id == i)
					{
						meshes_to_combine.push_back(obj_meshes_[j]);
						meshid_to_remove.insert(j);
					}
				}

				// Combine these meshes
				if (!meshes_to_combine.empty())
				{
					std::stringstream ss;
					ss << "combined_for_mtl_" << i;

					MeshStruct opt_mesh;
					opt_mesh.material_id = i;
					opt_mesh.name = ss.str();

					for (size_t j = 0; j < meshes_to_combine.size(); ++ j)
					{
						int base = static_cast<int>(opt_mesh.vertices.size());
						MeshStruct& mesh = meshes_to_combine[j];
						opt_mesh.vertices.insert(opt_mesh.vertices.end(),
							mesh.vertices.begin(), mesh.vertices.end());

						for (std::vector<TriangleStruct>::iterator itr = mesh.triangles.begin();
							itr != mesh.triangles.end(); ++ itr)
						{
							TriangleStruct tri;
							tri.vertex_index[0] = itr->vertex_index[0] + base;
							tri.vertex_index[1] = itr->vertex_index[1] + base;
							tri.vertex_index[2] = itr->vertex_index[2] + base;
							opt_mesh.triangles.push_back(tri);
						}
					}
					meshes_finished.push_back(opt_mesh);
				}
			}

			// Rebuild the mesh list
			for (size_t i = 0; i < obj_meshes_.size(); ++ i)
			{
				if (meshid_to_remove.find(i) == meshid_to_remove.end())
				{
					meshes_finished.push_back(obj_meshes_[i]);
				}
			}
			obj_meshes_ = meshes_finished;
		}

		if (userExportSettings_ & UES_SORT_MESHES)
		{
			std::sort(obj_meshes_.begin(), obj_meshes_.end(), MaterialIDSortOp());
		}
	}

}  // namespace KlayGE
