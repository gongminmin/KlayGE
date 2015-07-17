/**
 * @file MeshMLLib.cpp
 * @author Rui Wang, Minmin Gong
 *
 * @section DESCRIPTION
 *
 * This source file is part of MeshMLLib, a subproject of KlayGE
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

#include <KFL/KFL.hpp>
#include <MeshMLLib/MeshMLLib.hpp>

#include <set>
#include <algorithm>
#include <cmath>
#include <limits>

#include <boost/lexical_cast.hpp>
#include <boost/assert.hpp>

namespace
{
	std::string RemoveQuote(std::string const & str)
	{
		std::string ret = str;
		ret.erase(std::remove(ret.begin(), ret.end(), '\"'), ret.end());
		return ret;
	}
}

namespace KlayGE
{
	bool MeshMLObj::Material::operator==(MeshMLObj::Material const & rhs) const
	{
		bool same = (ambient == rhs.ambient) && (diffuse == rhs.diffuse)
			&& (specular == rhs.specular) && (emit == rhs.emit)
			&& (opacity == rhs.opacity) && (shininess == rhs.shininess)
			&& (texture_slots.size() == rhs.texture_slots.size());
		if (same)
		{
			for (size_t i = 0; i < texture_slots.size(); ++ i)
			{
				bool found = false;
				for (size_t j = 0; j < rhs.texture_slots.size(); ++ j)
				{
					if (texture_slots[i] == rhs.texture_slots[j])
					{
						found = true;
						break;
					}
				}

				if (!found)
				{
					return false;
				}
			}

			return true;
		}
		else
		{
			return false;
		}
	}

	std::pair<std::pair<Quaternion, Quaternion>, float> MeshMLObj::Keyframes::Frame(float frame) const
	{
		frame = std::fmod(frame, static_cast<float>(frame_ids.back() + 1));

		auto iter = std::upper_bound(frame_ids.begin(), frame_ids.end(), frame);
		int index = static_cast<int>(iter - frame_ids.begin());

		int index0 = index - 1;
		int index1 = index % frame_ids.size();
		int frame0 = frame_ids[index0];
		int frame1 = frame_ids[index1];
		float factor = (frame - frame0) / (frame1 - frame0);
		std::pair<std::pair<Quaternion, Quaternion>, float> ret;
		ret.first = MathLib::sclerp(bind_reals[index0], bind_duals[index0], bind_reals[index1], bind_duals[index1], factor);
		ret.second = MathLib::lerp(bind_scales[index0], bind_scales[index1], factor);
		return ret;
	}


	MeshMLObj::MeshMLObj(float unit_scale)
		: unit_scale_(unit_scale), num_frames_(0), frame_rate_(25)
	{
	}

	int MeshMLObj::AllocJoint()
	{
		int id = static_cast<int>(joints_.size());
		joints_.emplace(id, Joint());
		return id;
	}

	void MeshMLObj::SetJoint(int joint_id, std::string const & joint_name, int parent_id,
		float4x4 const & bind_mat)
	{
		Quaternion real, dual;
		this->MatrixToDQ(bind_mat, real, dual);

		this->SetJoint(joint_id, joint_name, parent_id, real, dual);
	}

	void MeshMLObj::SetJoint(int joint_id, std::string const & joint_name, int parent_id,
		Quaternion const & bind_quat, float3 const & bind_pos)
	{
		this->SetJoint(joint_id, joint_name, parent_id, bind_quat, MathLib::quat_trans_to_udq(bind_quat, bind_pos));
	}

	void MeshMLObj::SetJoint(int joint_id, std::string const & joint_name, int parent_id,
		Quaternion const & bind_real, Quaternion const & bind_dual)
	{
		float scale = MathLib::length(bind_real);
		if (bind_real.w() < 0)
		{
			scale = -scale;
		}

		Joint& joint = joints_[joint_id];
		joint.name = joint_name;
		joint.parent_id = parent_id;
		joint.bind_real = bind_real / scale;
		joint.bind_dual = bind_dual;
		joint.bind_scale = scale;
	}

	int MeshMLObj::AllocMaterial()
	{
		int id = static_cast<int>(materials_.size());
		materials_.push_back(Material());
		return id;
	}

	void MeshMLObj::SetMaterial(int mtl_id, float3 const & ambient, float3 const & diffuse,
			float3 const & specular, float3 const & emit, float opacity, float shininess)
	{
		BOOST_ASSERT(static_cast<int>(materials_.size()) > mtl_id);

		Material& mtl = materials_[mtl_id];
		mtl.ambient = ambient;
		mtl.diffuse = diffuse;
		mtl.specular = specular;
		mtl.emit = emit;
		mtl.opacity = opacity;
		mtl.shininess = shininess;
	}

	int MeshMLObj::AllocTextureSlot(int mtl_id)
	{
		BOOST_ASSERT(static_cast<int>(materials_.size()) > mtl_id);

		Material& mtl = materials_[mtl_id];
		int id = static_cast<int>(mtl.texture_slots.size());
		mtl.texture_slots.push_back(TextureSlot());
		return id;
	}

	void MeshMLObj::SetTextureSlot(int mtl_id, int slot_id, std::string const & type, std::string const & name)
	{
		BOOST_ASSERT(static_cast<int>(materials_.size()) > mtl_id);
		BOOST_ASSERT(static_cast<int>(materials_[mtl_id].texture_slots.size()) > slot_id);

		TextureSlot& ts = materials_[mtl_id].texture_slots[slot_id];
		ts.first = type;
		ts.second = name;
	}

	int MeshMLObj::AllocMesh()
	{
		int id = static_cast<int>(meshes_.size());
		meshes_.push_back(Mesh());
		return id;
	}

	void MeshMLObj::SetMesh(int mesh_id, int material_id, std::string const & name)
	{
		BOOST_ASSERT(static_cast<int>(meshes_.size()) > mesh_id);

		Mesh& mesh = meshes_[mesh_id];
		mesh.material_id = material_id;
		mesh.name = name;
	}

	int MeshMLObj::AllocVertex(int mesh_id)
	{
		BOOST_ASSERT(static_cast<int>(meshes_.size()) > mesh_id);

		Mesh& mesh = meshes_[mesh_id];
		int id = static_cast<int>(mesh.vertices.size());
		mesh.vertices.push_back(Vertex());
		return id;
	}

	void MeshMLObj::SetVertex(int mesh_id, int vertex_id, float3 const & pos, float3 const & normal,
		int texcoord_components, std::vector<float3> const & texcoords)
	{
		BOOST_ASSERT(static_cast<int>(meshes_.size()) > mesh_id);
		BOOST_ASSERT(static_cast<int>(meshes_[mesh_id].vertices.size()) > vertex_id);

		Vertex& vertex = meshes_[mesh_id].vertices[vertex_id];
		vertex.position = pos * unit_scale_;
		vertex.normal = normal;
		vertex.texcoord_components = texcoord_components;
		vertex.texcoords = texcoords;
	}

	void MeshMLObj::SetVertex(int mesh_id, int vertex_id, float3 const & pos,
		float3 const & tangent, float3 const & binormal, float3 const & normal,
		int texcoord_components, std::vector<float3> const & texcoords)
	{
		this->SetVertex(mesh_id, vertex_id, pos, MathLib::to_quaternion(tangent, binormal, normal, 8),
			texcoord_components, texcoords);
	}

	void MeshMLObj::SetVertex(int mesh_id, int vertex_id, float3 const & pos, Quaternion const & tangent_quat,
		int texcoord_components, std::vector<float3> const & texcoords)
	{
		BOOST_ASSERT(static_cast<int>(meshes_.size()) > mesh_id);
		BOOST_ASSERT(static_cast<int>(meshes_[mesh_id].vertices.size()) > vertex_id);

		Vertex& vertex = meshes_[mesh_id].vertices[vertex_id];
		vertex.position = pos * unit_scale_;
		vertex.tangent_quat = tangent_quat;
		vertex.texcoord_components = texcoord_components;
		vertex.texcoords = texcoords;
	}

	int MeshMLObj::AllocJointBinding(int mesh_id, int vertex_id)
	{
		BOOST_ASSERT(static_cast<int>(meshes_.size()) > mesh_id);
		BOOST_ASSERT(static_cast<int>(meshes_[mesh_id].vertices.size()) > vertex_id);

		Vertex& vertex = meshes_[mesh_id].vertices[vertex_id];
		int id = static_cast<int>(vertex.binds.size());
		vertex.binds.push_back(JointBinding());
		return id;
	}

	void MeshMLObj::SetJointBinding(int mesh_id, int vertex_id, int binding_id,
			int joint_id, float weight)
	{
		BOOST_ASSERT(static_cast<int>(meshes_.size()) > mesh_id);
		BOOST_ASSERT(static_cast<int>(meshes_[mesh_id].vertices.size()) > vertex_id);
		BOOST_ASSERT(static_cast<int>(meshes_[mesh_id].vertices[vertex_id].binds.size()) > binding_id);

		JointBinding& binding = meshes_[mesh_id].vertices[vertex_id].binds[binding_id];
		binding.first = joint_id;
		binding.second = weight;
	}

	int MeshMLObj::AllocTriangle(int mesh_id)
	{
		BOOST_ASSERT(static_cast<int>(meshes_.size()) > mesh_id);

		Mesh& mesh = meshes_[mesh_id];
		int id = static_cast<int>(mesh.triangles.size());
		mesh.triangles.push_back(Triangle());
		return id;
	}

	void MeshMLObj::SetTriangle(int mesh_id, int triangle_id, int index0, int index1, int index2)
	{
		BOOST_ASSERT(static_cast<int>(meshes_.size()) > mesh_id);
		BOOST_ASSERT(static_cast<int>(meshes_[mesh_id].triangles.size()) > triangle_id);

		Triangle& triangle = meshes_[mesh_id].triangles[triangle_id];
		triangle.vertex_index[0] = index0;
		triangle.vertex_index[1] = index1;
		triangle.vertex_index[2] = index2;
	}

	int MeshMLObj::AllocKeyframes()
	{
		int id = static_cast<int>(keyframes_.size());
		keyframes_.push_back(Keyframes());
		return id;
	}

	void MeshMLObj::SetKeyframes(int kfs_id, int joint_id)
	{
		BOOST_ASSERT(static_cast<int>(keyframes_.size()) > kfs_id);

		Keyframes& kfs = keyframes_[kfs_id];
		kfs.joint_id = joint_id;
	}

	int MeshMLObj::AllocKeyframe(int kfs_id)
	{
		BOOST_ASSERT(static_cast<int>(keyframes_.size()) > kfs_id);

		Keyframes& kfs = keyframes_[kfs_id];
		int id = static_cast<int>(kfs.frame_ids.size());
		kfs.frame_ids.push_back(id);
		kfs.bind_reals.push_back(Quaternion());
		kfs.bind_duals.push_back(Quaternion());
		kfs.bind_scales.push_back(1);
		return id;
	}

	void MeshMLObj::SetKeyframe(int kfs_id, int kf_id, int frame_id, float4x4 const & bind_mat)
	{
		Quaternion real, dual;
		this->MatrixToDQ(bind_mat, real, dual);

		this->SetKeyframe(kfs_id, kf_id, frame_id, real, dual);
	}

	void MeshMLObj::SetKeyframe(int kfs_id, int kf_id, int frame_id, Quaternion const & bind_quat, float3 const & bind_pos)
	{
		this->SetKeyframe(kfs_id, kf_id, frame_id, bind_quat, MathLib::quat_trans_to_udq(bind_quat, bind_pos));
	}

	void MeshMLObj::SetKeyframe(int kfs_id, int kf_id, int frame_id, Quaternion const & bind_real, Quaternion const & bind_dual)
	{
		BOOST_ASSERT(static_cast<int>(keyframes_.size()) > kfs_id);
		BOOST_ASSERT(static_cast<int>(keyframes_[kfs_id].bind_reals.size()) > kf_id);

		float scale = MathLib::length(bind_real);
		if (bind_real.w() < 0)
		{
			scale = -scale;
		}
		
		Keyframes& kfs = keyframes_[kfs_id];
		kfs.frame_ids[kf_id] = frame_id;
		kfs.bind_reals[kf_id] = bind_real / scale;
		kfs.bind_duals[kf_id] = bind_dual;
		kfs.bind_scales[kf_id] = scale;
	}

	int MeshMLObj::AllocAction()
	{
		int id = static_cast<int>(actions_.size());
		actions_.push_back(AnimationAction());
		return id;
	}

	void MeshMLObj::SetAction(int action_id, std::string const & name, int start_frame, int end_frame)
	{
		BOOST_ASSERT(static_cast<int>(actions_.size()) > action_id);

		AnimationAction& action = actions_[action_id];
		action.name = name;
		action.start_frame = start_frame;
		action.end_frame = end_frame;
	}

	void MeshMLObj::WriteMeshML(std::ostream& os, int vertex_export_settings, int user_export_settings, std::string const & encoding)
	{
		this->OptimizeJoints();
		this->OptimizeMaterials();
		this->OptimizeMeshes(user_export_settings);

		std::map<int, int> joint_id_to_index;
		std::vector<int> joint_index_to_id;
		if (joints_.empty())
		{
			keyframes_.clear();
		}
		else
		{
			for (auto const & joint : joints_)
			{
				joint_index_to_id.push_back(joint.first);
			}

			for (int i = 0; i < static_cast<int>(joint_index_to_id.size()); ++ i)
			{
				joint_id_to_index.emplace(joint_index_to_id[i], i);
			}

			// Replace parent_id
			for (auto& joint : joints_)
			{
				if (joint.second.parent_id != -1)
				{
					auto fiter = joint_id_to_index.find(joint.second.parent_id);
					BOOST_ASSERT(fiter != joint_id_to_index.end());

					joint.second.parent_id = fiter->second;
				}
			}

			// Replace joint_id in weight
			for (auto& mesh : meshes_)
			{
				for (auto& vertex : mesh.vertices)
				{
					for (auto& bind : vertex.binds)
					{
						auto fiter = joint_id_to_index.find(bind.first);
						BOOST_ASSERT(fiter != joint_id_to_index.end());

						bind.first = fiter->second;
					}
				}
			}

			// Replace joint_id in keyframes and remove unused keyframes
			for (auto iter = keyframes_.begin(); iter != keyframes_.end();)
			{
				auto fiter = joint_id_to_index.find(iter->joint_id);
				if (fiter != joint_id_to_index.end())
				{
					iter->joint_id = fiter->second;
					++ iter;
				}
				else
				{
					iter = keyframes_.erase(iter);
				}
			}
		}

		int model_ver = 6;

		// Initialize the xml document
		os << "<?xml version=\"1.0\"";
		if (!encoding.empty())
		{
			os << " encoding=\"" << encoding << "\"";
		}
		os << "?>" << std::endl << std::endl;
		os << "<model version=\"" << model_ver << "\">" << std::endl;

		if (!joints_.empty())
		{
			this->WriteJointChunk(os);
		}
		if (!materials_.empty())
		{
			this->WriteMaterialChunk(os);
		}
		if (!meshes_.empty())
		{
			this->WriteMeshChunk(os, vertex_export_settings);
		}
		if (!keyframes_.empty())
		{
			this->WriteKeyframeChunk(os);
			this->WriteAABBKeyframeChunk(os);
			this->WriteActionChunk(os);
		}

		// Finish the writing process
		os << "</model>" << std::endl;
	}

	void MeshMLObj::WriteJointChunk(std::ostream& os)
	{
		os << "\t<bones_chunk>" << std::endl;
		for (auto const & joint : joints_)
		{
			os << "\t\t<bone name=\"" << RemoveQuote(joint.second.name);

			Quaternion const bind_real = joint.second.bind_real * joint.second.bind_scale;
			Quaternion const & bind_dual = joint.second.bind_dual;

			os << "\" parent=\"" << joint.second.parent_id << "\">" << std::endl;
			os << "\t\t\t<real v=\"" << bind_real[0]
				<< " " << bind_real[1] << " " << bind_real[2]
				<< " " << bind_real[3] << "\"/>" << std::endl;
			os << "\t\t\t<dual v=\"" << bind_dual[0]
				<< " " << bind_dual[1] << " " << bind_dual[2]
				<< " " << bind_dual[3] << "\"/>" << std::endl;
			os << "\t\t</bone>" << std::endl;
		}
		os << "\t</bones_chunk>" << std::endl;
	}

	void MeshMLObj::WriteMaterialChunk(std::ostream& os)
	{
		os << "\t<materials_chunk>" << std::endl;
		for (auto const & mtl : materials_)
		{
			os << "\t\t<material ambient=\"" << mtl.ambient[0]
				<< " " << mtl.ambient[1]
				<< " " << mtl.ambient[2]
				<< "\" diffuse=\"" << mtl.diffuse[0]
				<< " " << mtl.diffuse[1]
				<< " " << mtl.diffuse[2]
				<< "\" specular=\"" << mtl.specular[0]
				<< " " << mtl.specular[1]
				<< " " << mtl.specular[2]
				<< "\" emit=\"" << mtl.emit[0]
				<< " " << mtl.emit[1]
				<< " " << mtl.emit[2]
				<< "\" opacity=\"" << mtl.opacity
				<< "\" shininess=\"" << mtl.shininess << "\"";

			if (mtl.texture_slots.empty())
			{
				os << "/>" << std::endl;
			}
			else
			{
				os << ">" << std::endl;

				for (auto const & tl : mtl.texture_slots)
				{
					os << "\t\t\t<texture type=\"" << RemoveQuote(tl.first)
						<< "\" name=\"" << RemoveQuote(tl.second) << "\"/>" << std::endl;
				}

				os << "\t\t</material>" << std::endl;
			}
		}
		os << "\t</materials_chunk>" << std::endl;
	}

	void MeshMLObj::WriteMeshChunk(std::ostream& os, int vertex_export_settings)
	{
		os << "\t<meshes_chunk>" << std::endl;
		for (auto const & mesh : meshes_)
		{
			os << "\t\t<mesh name=\"" << RemoveQuote(mesh.name)
				<< "\" mtl_id=\"" << mesh.material_id << "\">" << std::endl;

			os << "\t\t\t<vertices_chunk>" << std::endl;

			float3 pos_min_bb = mesh.vertices[0].position;
			float3 pos_max_bb = pos_min_bb;
			float2 tc_min_bb(-1, -1);
			float2 tc_max_bb(+1, +1);
			if (vertex_export_settings & VES_Texcoord)
			{
				tc_min_bb = tc_max_bb = mesh.vertices[0].texcoords[0];
			}
			for (auto const & vertex : mesh.vertices)
			{
				pos_min_bb.x() = std::min(pos_min_bb.x(), vertex.position.x());
				pos_min_bb.y() = std::min(pos_min_bb.y(), vertex.position.y());
				pos_min_bb.z() = std::min(pos_min_bb.z(), vertex.position.z());

				pos_max_bb.x() = std::max(pos_max_bb.x(), vertex.position.x());
				pos_max_bb.y() = std::max(pos_max_bb.y(), vertex.position.y());
				pos_max_bb.z() = std::max(pos_max_bb.z(), vertex.position.z());

				if (vertex_export_settings & VES_Texcoord)
				{
					tc_min_bb.x() = std::min(tc_min_bb.x(), vertex.texcoords[0].x());
					tc_min_bb.y() = std::min(tc_min_bb.y(), vertex.texcoords[0].y());

					tc_max_bb.x() = std::max(tc_max_bb.x(), vertex.texcoords[0].x());
					tc_max_bb.y() = std::max(tc_max_bb.y(), vertex.texcoords[0].y());
				}
			}

			os << "\t\t\t\t<pos_bb min=\"" << pos_min_bb.x() << " " << pos_min_bb.y()
				<< " " << pos_min_bb.z() << "\" max=\"" << pos_max_bb.x() << " " << pos_max_bb.y()
				<< " " << pos_max_bb.z() << "\"/>" << std::endl;
			if (vertex_export_settings & VES_Texcoord)
			{
				os << "\t\t\t\t<tc_bb min=\"" << tc_min_bb.x() << " " << tc_min_bb.y()
					<< "\" max=\"" << tc_max_bb.x() << " " << tc_max_bb.y() << "\"/>" << std::endl;
			}
			os << std::endl;

			for (auto const & vertex : mesh.vertices)
			{
				os << "\t\t\t\t<vertex v=\"" << vertex.position.x()
					<< " " << vertex.position.y()
					<< " " << vertex.position.z() << "\"";
				if (vertex_export_settings != VES_None)
				{
					os << ">" << std::endl;

					if (vertex_export_settings & VES_Normal)
					{
						os << "\t\t\t\t\t<normal v=\"" << vertex.normal.x()
							<< " " << vertex.normal.y()
							<< " " << vertex.normal.z() << "\"/>" << std::endl;
					}

					if (vertex_export_settings & VES_TangentQuat)
					{
						os << "\t\t\t\t\t<tangent_quat v=\"" << vertex.tangent_quat.x()
							<< " " << vertex.tangent_quat.y()
							<< " " << vertex.tangent_quat.z()
							<< " " << vertex.tangent_quat.w() << "\"/>" << std::endl;
					}

					if (vertex_export_settings & VES_Texcoord)
					{
						switch (vertex.texcoord_components)
						{
						case 1:
							for (auto const & tc : vertex.texcoords)
							{
								os << "\t\t\t\t\t<tex_coord v=\"" << tc.x() << "\"/>" << std::endl;
							}
							break;

						case 2:
							for (auto const & tc : vertex.texcoords)
							{
								os << "\t\t\t\t\t<tex_coord v=\"" << tc.x()
									<< " " << tc.y() << "\"/>" << std::endl;
							}
							break;

						case 3:
							for (auto const & tc : vertex.texcoords)
							{
								os << "\t\t\t\t\t<tex_coord v=\"" << tc.x()
									<< " " << tc.y() << " " << tc.z() << "\"/>" << std::endl;
							}
							break;

						default:
							break;
						}
					}

					if (!vertex.binds.empty())
					{
						os << "\t\t\t\t\t<weight joint=\"";
						for (size_t i = 0; i < vertex.binds.size(); ++ i)
						{
							os << vertex.binds[i].first;
							if (i != vertex.binds.size() - 1)
							{
								os << ' ';
							}
						}
						os << "\" weight=\"";
						for (size_t i = 0; i < vertex.binds.size(); ++ i)
						{
							os << vertex.binds[i].second;
							if (i != vertex.binds.size() - 1)
							{
								os << ' ';
							}
						}
						os << "\"/>" << std::endl;
					}

					os << "\t\t\t\t</vertex>" << std::endl;
				}
				else
				{
					os << "/>" << std::endl;
				}
			}
			os << "\t\t\t</vertices_chunk>" << std::endl;

			os << "\t\t\t<triangles_chunk>" << std::endl;
			for (auto const & tri : mesh.triangles)
			{
				os << "\t\t\t\t<triangle index=\"" << tri.vertex_index[0]
					<< " " << tri.vertex_index[1]
					<< " " << tri.vertex_index[2] << "\"/>" << std::endl;
			}
			os << "\t\t\t</triangles_chunk>" << std::endl;

			os << "\t\t</mesh>" << std::endl;
		}
		os << "\t</meshes_chunk>" << std::endl;
	}

	void MeshMLObj::WriteKeyframeChunk(std::ostream& os)
	{
		float const THRESHOLD = 1e-3f;

		std::map<int, int> joint_index_to_kf;
		for (size_t i = 0; i < keyframes_.size(); ++ i)
		{
			joint_index_to_kf.emplace(keyframes_[i].joint_id, static_cast<int>(i));
		}

		os << "\t<key_frames_chunk num_frames=\"" << num_frames_
			<< "\" frame_rate=\"" << frame_rate_ << "\">" << std::endl;
		for (int joint_index = 0; joint_index < static_cast<int>(joints_.size()); ++ joint_index)
		{
			auto iter = joint_index_to_kf.find(joint_index);
			BOOST_ASSERT(iter != joint_index_to_kf.end());

			Keyframes kf = keyframes_[iter->second];

			BOOST_ASSERT(kf.bind_reals.size() == kf.bind_duals.size());

			int base = 0;
			while (base < static_cast<int>(kf.frame_ids.size() - 2))
			{
				int const frame0 = kf.frame_ids[base + 0];
				int const frame1 = kf.frame_ids[base + 1];
				int const frame2 = kf.frame_ids[base + 2];
				float const factor = static_cast<float>(frame1 - frame0) / (frame2 - frame0);
				std::pair<Quaternion, Quaternion> interpolate = MathLib::sclerp(kf.bind_reals[base + 0], kf.bind_duals[base + 0],
					kf.bind_reals[base + 2], kf.bind_duals[base + 2], factor);
				float const scale = MathLib::lerp(kf.bind_scales[base + 0], kf.bind_scales[base + 2], factor);

				if (MathLib::dot(kf.bind_reals[base + 1], interpolate.first) < 0)
				{
					interpolate.first = -interpolate.first;
					interpolate.second = -interpolate.second;
				}

				std::pair<Quaternion, Quaternion> diff_dq = MathLib::inverse(kf.bind_reals[base + 1], kf.bind_duals[base + 1]);
				diff_dq.second = MathLib::mul_dual(diff_dq.first, diff_dq.second * scale, interpolate.first, interpolate.second);
				diff_dq.first = MathLib::mul_real(diff_dq.first, interpolate.first);
				float diff_scale = scale * kf.bind_scales[base + 1];

				if ((MathLib::abs(diff_dq.first.x()) < THRESHOLD) && (MathLib::abs(diff_dq.first.y()) < THRESHOLD)
					&& (MathLib::abs(diff_dq.first.z()) < THRESHOLD) && (MathLib::abs(diff_dq.first.w() - 1) < THRESHOLD)
					&& (MathLib::abs(diff_dq.second.x()) < THRESHOLD) && (MathLib::abs(diff_dq.second.y()) < THRESHOLD)
					&& (MathLib::abs(diff_dq.second.z()) < THRESHOLD) && (MathLib::abs(diff_dq.second.w()) < THRESHOLD)
					&& (MathLib::abs(diff_scale - 1) < THRESHOLD))
				{
					kf.frame_ids.erase(kf.frame_ids.begin() + base + 1);
					kf.bind_reals.erase(kf.bind_reals.begin() + base + 1);
					kf.bind_duals.erase(kf.bind_duals.begin() + base + 1);
					kf.bind_scales.erase(kf.bind_scales.begin() + base + 1);
				}
				else
				{
					++ base;
				}
			}

			os << "\t\t<key_frame joint=\"" << kf.joint_id << "\">" << std::endl;
			for (size_t j = 0; j < kf.frame_ids.size(); ++ j)
			{
				Quaternion const bind_real = kf.bind_reals[j] * kf.bind_scales[j];
				Quaternion const & bind_dual = kf.bind_duals[j];

				os << "\t\t\t<key id=\"" << kf.frame_ids[j] << "\">" << std::endl;
				os << "\t\t\t\t<real v=\"" << bind_real.x()
					<< " " << bind_real.y()
					<< " " << bind_real.z()
					<< " " << bind_real.w() << "\"/>" << std::endl;
				os << "\t\t\t\t<dual v=\"" << bind_dual.x()
					<< " " << bind_dual.y()
					<< " " << bind_dual.z()
					<< " " << bind_dual.w() << "\"/>" << std::endl;
				os << "\t\t\t</key>" << std::endl;
			}
			os << "\t\t</key_frame>" << std::endl;
		}
		os << "\t</key_frames_chunk>" << std::endl;
	}

	void MeshMLObj::WriteAABBKeyframeChunk(std::ostream& os)
	{
		float const THRESHOLD = 1e-3f;

		std::vector<Quaternion> bind_reals;
		std::vector<Quaternion> bind_duals;
		std::vector<std::vector<int>> frame_ids(meshes_.size());
		std::vector<std::vector<float3>> bb_min_key_frames(meshes_.size());
		std::vector<std::vector<float3>> bb_max_key_frames(meshes_.size());
		for (size_t m = 0; m < meshes_.size(); ++ m)
		{
			frame_ids[m].resize(num_frames_);
			bb_min_key_frames[m].resize(num_frames_);
			bb_max_key_frames[m].resize(num_frames_);
		}

		for (int f = 0; f < num_frames_; ++ f)
		{
			this->UpdateJoints(f, bind_reals, bind_duals);
			for (size_t m = 0; m < meshes_.size(); ++ m)
			{
				float3 bb_min(+1e10f, +1e10f, +1e10f);
				float3 bb_max(-1e10f, -1e10f, -1e10f);
				for (size_t v = 0; v < meshes_[m].vertices.size(); ++ v)
				{
					Vertex const & vertex = meshes_[m].vertices[v];

					Quaternion const & dp0 = bind_reals[vertex.binds[0].first];
	
					float3 pos_s(0, 0, 0);
					Quaternion blend_real(0, 0, 0, 0);
					Quaternion blend_dual(0, 0, 0, 0);
					for (size_t bi = 0; bi < vertex.binds.size(); ++ bi)
					{
						Quaternion joint_real = bind_reals[vertex.binds[bi].first];
						Quaternion joint_dual = bind_duals[vertex.binds[bi].first];
		
						float scale = MathLib::length(joint_real);
						joint_real /= scale;

						float weight = vertex.binds[bi].second;
		
						if (MathLib::dot(dp0, joint_real) < 0)
						{
							joint_real = -joint_real;
							joint_dual = -joint_dual;
						}

						pos_s += vertex.position * scale * weight;
						blend_real += joint_real * weight;
						blend_dual += joint_dual * weight;
					}
	
					float len = MathLib::length(blend_real);
					blend_real /= len;
					blend_dual /= len;

					Quaternion trans = MathLib::mul(Quaternion(blend_dual.x(), blend_dual.y(), blend_dual.z(), -blend_dual.w()), blend_real);
					float3 result_pos = MathLib::transform_quat(pos_s, blend_real) + 2 * float3(trans.x(), trans.y(), trans.z());

					if ((result_pos.x() == result_pos.x()) && (result_pos.y() == result_pos.y())
						&& (result_pos.z() == result_pos.z()))
					{
						bb_min.x() = std::min(bb_min.x(), result_pos.x());
						bb_min.y() = std::min(bb_min.y(), result_pos.y());
						bb_min.z() = std::min(bb_min.z(), result_pos.z());

						bb_max.x() = std::max(bb_max.x(), result_pos.x());
						bb_max.y() = std::max(bb_max.y(), result_pos.y());
						bb_max.z() = std::max(bb_max.z(), result_pos.z());
					}
				}

				frame_ids[m][f] = f;
				bb_min_key_frames[m][f] = bb_min;
				bb_max_key_frames[m][f] = bb_max;
			}
		}

		os << "\t<bb_key_frames_chunk>" << std::endl;
		for (size_t m = 0; m < meshes_.size(); ++ m)
		{
			std::vector<int>& fid = frame_ids[m];
			std::vector<float3>& min_kf = bb_min_key_frames[m];
			std::vector<float3>& max_kf = bb_max_key_frames[m];

			int base = 0;
			while (base < static_cast<int>(fid.size() - 2))
			{
				int const frame0 = fid[base + 0];
				int const frame1 = fid[base + 1];
				int const frame2 = fid[base + 2];
				float const factor = static_cast<float>(frame1 - frame0) / (frame2 - frame0);
				float3 const interpolate_min = MathLib::lerp(min_kf[base + 0], min_kf[base + 2], factor);
				float3 const interpolate_max = MathLib::lerp(max_kf[base + 0], max_kf[base + 2], factor);

				float3 const diff_min = min_kf[base + 1] - interpolate_min;
				float3 const diff_max = max_kf[base + 1] - interpolate_max;

				if ((MathLib::abs(diff_min.x()) < THRESHOLD) && (MathLib::abs(diff_min.y()) < THRESHOLD)
					&& (MathLib::abs(diff_min.z()) < THRESHOLD)
					&& (MathLib::abs(diff_max.x()) < THRESHOLD) && (MathLib::abs(diff_max.y()) < THRESHOLD)
					&& (MathLib::abs(diff_max.z()) < THRESHOLD))
				{
					fid.erase(fid.begin() + base + 1);
					min_kf.erase(min_kf.begin() + base + 1);
					max_kf.erase(max_kf.begin() + base + 1);
				}
				else
				{
					++ base;
				}
			}

			os << "\t\t<bb_key_frame mesh=\"" << m << "\">" << std::endl;
			for (size_t f = 0; f < fid.size(); ++ f)
			{
				float3 const & bb_min = min_kf[f];
				float3 const & bb_max = max_kf[f];

				os << "\t\t\t<key id=\"" << fid[f]
					<< "\" min=\"" << bb_min.x()
					<< " " << bb_min.y()
					<< " " << bb_min.z()
					<< "\" max=\"" << bb_max.x()
					<< " " << bb_max.y()
					<< " " << bb_max.z() << "\"/>" << std::endl;
			}
			os << "\t\t</bb_key_frame>" << std::endl;
		}
		os << "\t</bb_key_frames_chunk>" << std::endl;
	}

	void MeshMLObj::WriteActionChunk(std::ostream& os)
	{
		os << "\t<actions_chunk>" << std::endl;
		if (actions_.empty())
		{
			os << "\t\t<action name=\"" << "root"
				<< "\" start=\"" << 0
				<< "\" end=\"" << num_frames_
				<< "\"/>" << std::endl;
		}
		else
		{
			for (size_t a = 0; a < actions_.size(); ++ a)
			{
				AnimationAction const & action = actions_[a];

				os << "\t\t<action name=\"" << action.name
					<< "\" start=\"" << action.start_frame
					<< "\" end=\"" << action.end_frame
					<< "\"/>" << std::endl;
			}
		}
		os << "\t</actions_chunk>" << std::endl;
	}

	void MeshMLObj::OptimizeJoints()
	{
		std::set<int> joints_used;

		// Find all joints used in the mesh list
		for (auto const & mesh : meshes_)
		{
			for (auto const & vertex : mesh.vertices)
			{
				for (auto const & bind : vertex.binds)
				{
					joints_used.insert(bind.first);
				}
			}
		}

		// Traverse the joint list and see if used joints' parents can be added
		std::set<int> parent_joints_used;
		for (auto const & joint : joints_)
		{
			if (joints_used.find(joint.first) != joints_used.end())
			{
				Joint const * j = &joint.second;
				while (j->parent_id != -1)
				{
					parent_joints_used.insert(j->parent_id);
					j = &joints_[j->parent_id];
				}
			}
		}

		joints_used.insert(parent_joints_used.begin(), parent_joints_used.end());

		// Traverse the joint list and erase those never recorded by joints_used
		for (auto iter = joints_.begin(); iter != joints_.end();)
		{
			if (joints_used.find(iter->first) == joints_used.end())
			{
				joints_.erase(iter ++);
			}
			else
			{
				++ iter;
			}
		}
	}

	void MeshMLObj::OptimizeMaterials()
	{
		std::vector<int> mtl_mapping(materials_.size());
		std::vector<Material> mtls_used;

		// Traverse materials and setup IDs
		for (size_t i = 0; i < materials_.size(); ++ i)
		{
			bool found = false;
			for (size_t j = 0; j < mtls_used.size(); ++ j)
			{
				if (mtls_used[j] == materials_[i])
				{
					mtl_mapping[i] = static_cast<int>(j);
					found = true;
					break;
				}
			}

			if (!found)
			{
				mtl_mapping[i] = static_cast<int>(mtls_used.size());
				mtls_used.push_back(materials_[i]);
			}
		}

		materials_ = mtls_used;

		for (auto& mesh : meshes_)
		{
			mesh.material_id = mtl_mapping[mesh.material_id];
		}
	}

	void MeshMLObj::OptimizeMeshes(int user_export_settings)
	{
		if (user_export_settings & UES_CombineMeshes)
		{
			std::set<size_t> meshid_to_remove;
			std::vector<Mesh> meshes_finished;
			for (size_t i = 0; i < materials_.size(); ++ i)
			{
				// Find all meshes sharing one material
				std::vector<Mesh> meshes_to_combine;
				for (size_t j = 0; j < meshes_.size(); ++ j)
				{
					if (meshes_[j].material_id == static_cast<int>(i))
					{
						meshes_to_combine.push_back(meshes_[j]);
						meshid_to_remove.insert(j);
					}
				}

				// Combine these meshes
				if (!meshes_to_combine.empty())
				{
					Mesh opt_mesh;
					opt_mesh.material_id = static_cast<int>(i);
					opt_mesh.name = "combined_for_mtl_" + boost::lexical_cast<std::string>(i);

					for (auto const & mesh : meshes_to_combine)
					{
						int base = static_cast<int>(opt_mesh.vertices.size());
						opt_mesh.vertices.insert(opt_mesh.vertices.end(),
							mesh.vertices.begin(), mesh.vertices.end());

						for (auto const & tri : mesh.triangles)
						{
							Triangle opt_tri;
							opt_tri.vertex_index[0] = tri.vertex_index[0] + base;
							opt_tri.vertex_index[1] = tri.vertex_index[1] + base;
							opt_tri.vertex_index[2] = tri.vertex_index[2] + base;
							opt_mesh.triangles.push_back(opt_tri);
						}
					}
					meshes_finished.push_back(opt_mesh);
				}
			}

			// Rebuild the mesh list
			for (size_t i = 0; i < meshes_.size(); ++ i)
			{
				if (meshid_to_remove.find(i) == meshid_to_remove.end())
				{
					meshes_finished.push_back(meshes_[i]);
				}
			}
			meshes_ = meshes_finished;
		}

		if (user_export_settings & UES_SortMeshes)
		{
			std::sort(meshes_.begin(), meshes_.end(), MaterialIDSortOp());
		}
	}

	void MeshMLObj::MatrixToDQ(float4x4 const & mat, Quaternion& real, Quaternion& dual) const
	{
		float4x4 tmp_mat = mat;
		float flip = 1;
		if (MathLib::dot(MathLib::cross(float3(tmp_mat(0, 0), tmp_mat(0, 1), tmp_mat(0, 2)),
			float3(tmp_mat(1, 0), tmp_mat(1, 1), tmp_mat(1, 2))),
			float3(tmp_mat(2, 0), tmp_mat(2, 1), tmp_mat(2, 2))) < 0)
		{
			tmp_mat(2, 0) = -tmp_mat(2, 0);
			tmp_mat(2, 1) = -tmp_mat(2, 1);
			tmp_mat(2, 2) = -tmp_mat(2, 2);

			flip = -1;
		}

		float3 scale;
		float3 trans;
		MathLib::decompose(scale, real, trans, tmp_mat);

		dual = MathLib::quat_trans_to_udq(real, trans * unit_scale_);

		if (flip * real.w() < 0)
		{
			real = -real;
			dual = -dual;
		}

		real *= scale.x();
	}

	void MeshMLObj::UpdateJoints(int frame, std::vector<Quaternion>& bind_reals, std::vector<Quaternion>& bind_duals) const
	{
		std::vector<Joint> bind_joints;
		for (auto const & joint : joints_)
		{
			bind_joints.push_back(joint.second);
		}

		std::vector<Joint> bind_inverse_joints(bind_joints.size());
		for (size_t i = 0; i < bind_joints.size(); ++ i)
		{
			Joint inverse_joint;

			float flip = MathLib::sgn(bind_joints[i].bind_scale);

			inverse_joint.bind_scale = 1 / bind_joints[i].bind_scale;

			if (flip > 0)
			{
				std::pair<Quaternion, Quaternion> inv = MathLib::inverse(bind_joints[i].bind_real, bind_joints[i].bind_dual);
				inverse_joint.bind_real = inv.first;
				inverse_joint.bind_dual = inv.second;
			}
			else
			{
				float4x4 tmp_mat = MathLib::scaling(MathLib::abs(bind_joints[i].bind_scale), MathLib::abs(bind_joints[i].bind_scale), bind_joints[i].bind_scale)
					* MathLib::to_matrix(bind_joints[i].bind_real)
					* MathLib::translation(MathLib::udq_to_trans(bind_joints[i].bind_real, bind_joints[i].bind_dual));
				tmp_mat = MathLib::inverse(tmp_mat);
				tmp_mat(2, 0) = -tmp_mat(2, 0);
				tmp_mat(2, 1) = -tmp_mat(2, 1);
				tmp_mat(2, 2) = -tmp_mat(2, 2);

				float3 scale;
				Quaternion rot;
				float3 trans;
				MathLib::decompose(scale, rot, trans, tmp_mat);

				inverse_joint.bind_real = rot;
				inverse_joint.bind_dual = MathLib::quat_trans_to_udq(rot, trans);
				inverse_joint.bind_scale = -scale.x();
			}

			bind_inverse_joints[i] = inverse_joint;
		}

		for (size_t i = 0; i < bind_joints.size(); ++ i)
		{
			size_t kf_id = 0;
			Joint& joint = bind_joints[i];
			for (size_t j = 0; j < keyframes_.size(); ++ j)
			{
				Keyframes const & kf = keyframes_[j];
				if (kf.joint_id == static_cast<int>(i))
				{
					kf_id = j;
					break;
				}
			}

			Keyframes const & kf = keyframes_[kf_id];

			std::pair<std::pair<Quaternion, Quaternion>, float> key_dq = kf.Frame(static_cast<float>(frame));

			if (joint.parent_id != -1)
			{
				Joint const & parent(bind_joints[joint.parent_id]);

				if (MathLib::dot(key_dq.first.first, parent.bind_real) < 0)
				{
					key_dq.first.first = -key_dq.first.first;
					key_dq.first.second = -key_dq.first.second;
				}

				if ((key_dq.second > 0) && (parent.bind_scale > 0))
				{
					joint.bind_real = MathLib::mul_real(key_dq.first.first, parent.bind_real);
					joint.bind_dual = MathLib::mul_dual(key_dq.first.first, key_dq.first.second * parent.bind_scale, parent.bind_real, parent.bind_dual);
					joint.bind_scale = key_dq.second * parent.bind_scale;
				}
				else
				{
					float4x4 tmp_mat = MathLib::scaling(MathLib::abs(key_dq.second), MathLib::abs(key_dq.second), key_dq.second)
						* MathLib::to_matrix(key_dq.first.first)
						* MathLib::translation(MathLib::udq_to_trans(key_dq.first.first, key_dq.first.second))
						* MathLib::scaling(MathLib::abs(parent.bind_scale), MathLib::abs(parent.bind_scale), parent.bind_scale)
						* MathLib::to_matrix(parent.bind_real)
						* MathLib::translation(MathLib::udq_to_trans(parent.bind_real, parent.bind_dual));

					float flip = 1;
					if (MathLib::dot(MathLib::cross(float3(tmp_mat(0, 0), tmp_mat(0, 1), tmp_mat(0, 2)),
						float3(tmp_mat(1, 0), tmp_mat(1, 1), tmp_mat(1, 2))),
						float3(tmp_mat(2, 0), tmp_mat(2, 1), tmp_mat(2, 2))) < 0)
					{
						tmp_mat(2, 0) = -tmp_mat(2, 0);
						tmp_mat(2, 1) = -tmp_mat(2, 1);
						tmp_mat(2, 2) = -tmp_mat(2, 2);

						flip = -1;
					}

					float3 scale;
					Quaternion rot;
					float3 trans;
					MathLib::decompose(scale, rot, trans, tmp_mat);

					joint.bind_real = rot;
					joint.bind_dual = MathLib::quat_trans_to_udq(rot, trans);
					joint.bind_scale = flip * scale.x();
				}
			}
			else
			{
				joint.bind_real = key_dq.first.first;
				joint.bind_dual = key_dq.first.second;
				joint.bind_scale = key_dq.second;
			}
		}

		bind_reals.resize(bind_joints.size());
		bind_duals.resize(bind_joints.size());
		for (size_t i = 0; i < bind_joints.size(); ++ i)
		{
			Joint const & joint = bind_joints[i];
			Joint const & inverse_joint = bind_inverse_joints[i];

			Quaternion bind_real, bind_dual;
			float bind_scale;
			if ((inverse_joint.bind_scale > 0) && (joint.bind_scale > 0))
			{
				bind_real = MathLib::mul_real(inverse_joint.bind_real, joint.bind_real);
				bind_dual = MathLib::mul_dual(inverse_joint.bind_real, inverse_joint.bind_dual,
					joint.bind_real, joint.bind_dual);
				bind_scale = inverse_joint.bind_scale * joint.bind_scale;

				if (bind_real.w() < 0)
				{
					bind_real = -bind_real;
					bind_dual = -bind_dual;
				}
			}
			else
			{
				float4x4 tmp_mat = MathLib::scaling(MathLib::abs(inverse_joint.bind_scale), MathLib::abs(inverse_joint.bind_scale), inverse_joint.bind_scale)
					* MathLib::to_matrix(inverse_joint.bind_real)
					* MathLib::translation(MathLib::udq_to_trans(inverse_joint.bind_real, inverse_joint.bind_dual))
					* MathLib::scaling(MathLib::abs(joint.bind_scale), MathLib::abs(joint.bind_scale), joint.bind_scale)
					* MathLib::to_matrix(joint.bind_real)
					* MathLib::translation(MathLib::udq_to_trans(joint.bind_real, joint.bind_dual));

				float flip = 1;
				if (MathLib::dot(MathLib::cross(float3(tmp_mat(0, 0), tmp_mat(0, 1), tmp_mat(0, 2)),
					float3(tmp_mat(1, 0), tmp_mat(1, 1), tmp_mat(1, 2))),
					float3(tmp_mat(2, 0), tmp_mat(2, 1), tmp_mat(2, 2))) < 0)
				{
					tmp_mat(2, 0) = -tmp_mat(2, 0);
					tmp_mat(2, 1) = -tmp_mat(2, 1);
					tmp_mat(2, 2) = -tmp_mat(2, 2);

					flip = -1;
				}

				float3 scale;
				Quaternion rot;
				float3 trans;
				MathLib::decompose(scale, rot, trans, tmp_mat);

				bind_real = rot;
				bind_dual = MathLib::quat_trans_to_udq(rot, trans);
				bind_scale = scale.x();

				if (flip * bind_real.w() < 0)
				{
					bind_real = -bind_real;
					bind_dual = -bind_dual;
				}
			}

			bind_reals[i] = bind_real * bind_scale;
			bind_duals[i] = bind_dual;
		}
	}
}  // namespace KlayGE
