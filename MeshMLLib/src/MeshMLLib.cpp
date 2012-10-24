// MeshMLObj.cpp
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

#include <KlayGE/Types.hpp>
#include <MeshMLLib/MeshMLLib.hpp>

#include <set>
#include <sstream>
#include <algorithm>

#include <boost/assert.hpp>
#include <boost/typeof/typeof.hpp>
#include <boost/foreach.hpp>

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
	namespace MathLib
	{
		// From Quake III. But the magic number is from http://www.lomont.org/Math/Papers/2003/InvSqrt.pdf
		float recip_sqrt(float number)
		{
			float const threehalfs = 1.5f;

			float x2 = number * 0.5f;
			union FNI
			{
				float f;
				int32_t i;
			} fni;
			fni.f = number;											// evil floating point bit level hacking
			fni.i = 0x5f375a86 - (fni.i >> 1);						// what the fuck?
			fni.f = fni.f * (threehalfs - (x2 * fni.f * fni.f));	// 1st iteration
			fni.f = fni.f * (threehalfs - (x2 * fni.f * fni.f));		// 2nd iteration, this can be removed

			return fni.f;
		}

		template <typename T>
		bool equal(T const & lhs, T const & rhs)
		{
			return (lhs == rhs);
		}
		template <>
		bool equal<float>(float const & lhs, float const & rhs)
		{
			return (std::abs(lhs - rhs)
				<= std::numeric_limits<float>::epsilon());
		}

		template <typename T>
		typename T::value_type dot(T const & lhs, T const & rhs)
		{
			return detail::dot_helper<typename T::value_type,
							T::elem_num>::Do(&lhs[0], &rhs[0]);
		}

		template <typename T>
		typename T::value_type length_sq(T const & rhs)
		{
			return dot(rhs, rhs);
		}

		template <typename T>
		T normalize(T const & rhs)
		{
			return rhs * recip_sqrt(length_sq(rhs));
		}
		
		template <typename T>
		Vector_T<T, 3> cross(Vector_T<T, 3> const & lhs, Vector_T<T, 3> const & rhs)
		{
			return Vector_T<T, 3>(lhs.y() * rhs.z() - lhs.z() * rhs.y(),
				lhs.z() * rhs.x() - lhs.x() * rhs.z(),
				lhs.x() * rhs.y() - lhs.y() * rhs.x());
		}

		template <typename T>
		void decompose(Vector_T<T, 3>& scale, Quaternion_T<T>& rot, Vector_T<T, 3>& trans, Matrix4_T<T> const & rhs)
		{
			scale.x() = sqrt(rhs(0, 0) * rhs(0, 0) + rhs(0, 1) * rhs(0, 1) + rhs(0, 2) * rhs(0, 2));
			scale.y() = sqrt(rhs(1, 0) * rhs(1, 0) + rhs(1, 1) * rhs(1, 1) + rhs(1, 2) * rhs(1, 2));
			scale.z() = sqrt(rhs(2, 0) * rhs(2, 0) + rhs(2, 1) * rhs(2, 1) + rhs(2, 2) * rhs(2, 2));

			trans = Vector_T<T, 3>(rhs(3, 0), rhs(3, 1), rhs(3, 2));

			Matrix4_T<T> rot_mat;
			rot_mat(0, 0) = rhs(0, 0) / scale.x();
			rot_mat(0, 1) = rhs(0, 1) / scale.x();
			rot_mat(0, 2) = rhs(0, 2) / scale.x();
			rot_mat(0, 3) = 0;
			rot_mat(1, 0) = rhs(1, 0) / scale.y();
			rot_mat(1, 1) = rhs(1, 1) / scale.y();
			rot_mat(1, 2) = rhs(1, 2) / scale.y();
			rot_mat(1, 3) = 0;
			rot_mat(2, 0) = rhs(2, 0) / scale.z();
			rot_mat(2, 1) = rhs(2, 1) / scale.z();
			rot_mat(2, 2) = rhs(2, 2) / scale.z();
			rot_mat(2, 3) = 0;
			rot_mat(3, 0) = 0;
			rot_mat(3, 1) = 0;
			rot_mat(3, 2) = 0;
			rot_mat(3, 3) = 1;
			rot = to_quaternion(rot_mat);
		}

		template <typename T>
		Quaternion_T<T> mul(Quaternion_T<T> const & lhs, Quaternion_T<T> const & rhs)
		{
			return Quaternion_T<T>(
				lhs.x() * rhs.w() - lhs.y() * rhs.z() + lhs.z() * rhs.y() + lhs.w() * rhs.x(),
				lhs.x() * rhs.z() + lhs.y() * rhs.w() - lhs.z() * rhs.x() + lhs.w() * rhs.y(),
				lhs.y() * rhs.x() - lhs.x() * rhs.y() + lhs.z() * rhs.w() + lhs.w() * rhs.z(),
				lhs.w() * rhs.w() - lhs.x() * rhs.x() - lhs.y() * rhs.y() - lhs.z() * rhs.z());
		}

		template <typename T>
		Quaternion_T<T> to_quaternion(Matrix4_T<T> const & mat)
		{
			Quaternion_T<T> quat;
			T s;
			T const tr = mat(0, 0) + mat(1, 1) + mat(2, 2) + 1;

			// check the diagonal
			if (tr > 1)
			{
				s = sqrt(tr);
				quat.w() = s * T(0.5);
				s = T(0.5) / s;
				quat.x() = (mat(1, 2) - mat(2, 1)) * s;
				quat.y() = (mat(2, 0) - mat(0, 2)) * s;
				quat.z() = (mat(0, 1) - mat(1, 0)) * s;
			}
			else
			{
				int maxi = 0;
				T maxdiag = mat(0, 0);
				for (int i = 1; i < 3; ++ i)
				{
					if (mat(i, i) > maxdiag)
					{
						maxi = i;
						maxdiag = mat(i, i);
					}
				}

				switch (maxi)
				{
				case 0:
					s = sqrt((mat(0, 0) - (mat(1, 1) + mat(2, 2))) + 1);

					quat.x() = s * T(0.5);

					if (!equal<T>(s, 0))
					{
						s = T(0.5) / s;
					}

					quat.w() = (mat(1, 2) - mat(2, 1)) * s;
					quat.y() = (mat(1, 0) + mat(0, 1)) * s;
					quat.z() = (mat(2, 0) + mat(0, 2)) * s;
					break;

				case 1:
					s = sqrt((mat(1, 1) - (mat(2, 2) + mat(0, 0))) + 1);
					quat.y() = s * T(0.5);

					if (!equal<T>(s, 0))
					{
						s = T(0.5) / s;
					}

					quat.w() = (mat(2, 0) - mat(0, 2)) * s;
					quat.z() = (mat(2, 1) + mat(1, 2)) * s;
					quat.x() = (mat(0, 1) + mat(1, 0)) * s;
					break;

				case 2:
				default:
					s = sqrt((mat(2, 2) - (mat(0, 0) + mat(1, 1))) + 1);

					quat.z() = s * T(0.5);

					if (!equal<T>(s, 0))
					{
						s = T(0.5) / s;
					}

					quat.w() = (mat(0, 1) - mat(1, 0)) * s;
					quat.x() = (mat(0, 2) + mat(2, 0)) * s;
					quat.y() = (mat(1, 2) + mat(2, 1)) * s;
					break;
				}
			}

			return normalize(quat);
		}

		template <typename T>
		Quaternion_T<T> to_quaternion(Vector_T<T, 3> const & tangent, Vector_T<T, 3> const & binormal, Vector_T<T, 3> const & normal, int bits)
		{
			T k = 1;
			if (dot(binormal, cross(normal, tangent)) < 0)
			{
				k = -1;
			}

			Matrix4_T<T> tangent_frame(tangent.x(), tangent.y(), tangent.z(), 0,
				k * binormal.x(), k * binormal.y(), k * binormal.z(), 0,
				normal.x(), normal.y(), normal.z(), 0,
				0, 0, 0, 1);
			Quaternion_T<T> tangent_quat = to_quaternion(tangent_frame);
			if (tangent_quat.w() < 0)
			{
				tangent_quat = -tangent_quat;
			}
			T const bias = T(1) / ((1UL << (bits - 1)) - 1);
			if (tangent_quat.w() < bias)
			{
				T const factor = sqrt(1 - bias * bias);
				tangent_quat.x() *= factor;
				tangent_quat.y() *= factor;
				tangent_quat.z() *= factor;
				tangent_quat.w() = bias;
			}
			if (k < 0)
			{
				tangent_quat = -tangent_quat;
			}

			return tangent_quat;
		}

		template <typename T>
		Quaternion_T<T> quat_trans_to_udq(Quaternion_T<T> const & q, Vector_T<T, 3> const & t)
		{
			return mul(q, Quaternion_T<T>(T(0.5) * t.x(), T(0.5) * t.y(), T(0.5) * t.z(), T(0.0)));
		}
	}

	MeshMLObj::MeshMLObj(float unit_scale)
		: unit_scale_(unit_scale), start_frame_(0), end_frame_(0), frame_rate_(25)
	{
	}

	int MeshMLObj::AllocJoint()
	{
		int id = static_cast<int>(joints_.size());
		joints_.insert(std::make_pair(id, Joint()));
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
		Joint& joint = joints_[joint_id];
		joint.name = joint_name;
		joint.parent_id = parent_id;
		joint.bind_real = bind_real;
		joint.bind_dual = bind_dual;
	}

	int MeshMLObj::AllocMaterial()
	{
		int id = static_cast<int>(materials_.size());
		materials_.push_back(Material());
		return id;
	}

	void MeshMLObj::SetMaterial(int mtl_id, float3 const & ambient, float3 const & diffuse,
			float3 const & specular, float3 const & emit, float opacity, float specular_level, float shininess)
	{
		BOOST_ASSERT(static_cast<int>(materials_.size()) > mtl_id);

		Material& mtl = materials_[mtl_id];
		mtl.ambient = ambient;
		mtl.diffuse = diffuse;
		mtl.specular = specular;
		mtl.emit = emit;
		mtl.opacity = opacity;
		mtl.specular_level = specular_level;
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
		int id = static_cast<int>(kfs.bind_reals.size());
		kfs.bind_reals.push_back(Quaternion());
		kfs.bind_duals.push_back(Quaternion());
		return id;
	}

	void MeshMLObj::SetKeyframe(int kfs_id, int kf_id, float4x4 const & bind_mat)
	{
		Quaternion real, dual;
		this->MatrixToDQ(bind_mat, real, dual);

		this->SetKeyframe(kfs_id, kf_id, real, dual);
	}

	void MeshMLObj::SetKeyframe(int kfs_id, int kf_id, Quaternion const & bind_quat, float3 const & bind_pos)
	{
		this->SetKeyframe(kfs_id, kf_id, bind_quat, MathLib::quat_trans_to_udq(bind_quat, bind_pos));
	}

	void MeshMLObj::SetKeyframe(int kfs_id, int kf_id, Quaternion const & bind_real, Quaternion const & bind_dual)
	{
		BOOST_ASSERT(static_cast<int>(keyframes_.size()) > kfs_id);
		BOOST_ASSERT(static_cast<int>(keyframes_[kfs_id].bind_reals.size()) > kf_id);

		Keyframes& kfs = keyframes_[kfs_id];
		kfs.bind_reals[kf_id] = bind_real;
		kfs.bind_duals[kf_id] = bind_dual;
	}

	void MeshMLObj::WriteMeshML(std::ostream& os, int vertex_export_settings, int user_export_settings, std::string const & encoding)
	{
		this->OptimizeJoints();
		this->OptimizeMaterials();
		this->OptimizeMeshes(user_export_settings);

		std::map<int, int> joint_id_to_index;
		{
			int index = 0;
			for (std::map<int, Joint>::iterator iter = joints_.begin(); iter != joints_.end(); ++ iter, ++ index)
			{
				joint_id_to_index.insert(std::make_pair(iter->first, index));
			}
		}

		int model_ver = 5;

		// Initialize the xml document
		os << "<?xml version=\"1.0\" ";
		if (!encoding.empty())
		{
			os << "encoding=\"" << encoding << "\"";
		}
		os << "?>" << std::endl << std::endl;
		os << "<model version=\"" << model_ver << "\">" << std::endl;

		if (!joints_.empty())
		{
			this->WriteJointChunk(os, joint_id_to_index);
		}
		if (!materials_.empty())
		{
			this->WriteMaterialChunk(os);
		}
		if (!meshes_.empty())
		{
			this->WriteMeshChunk(os, joint_id_to_index, vertex_export_settings);
		}
		if (!keyframes_.empty())
		{
			this->WriteKeyframeChunk(os, joint_id_to_index);
		}

		// Finish the writing process
		os << "</model>" << std::endl;
	}

	void MeshMLObj::WriteJointChunk(std::ostream& os, std::map<int, int> const & joint_id_to_index)
	{
		os << "\t<bones_chunk>" << std::endl;
		for (std::map<int, Joint>::iterator iter = joints_.begin(); iter != joints_.end(); ++ iter)
		{
			Joint const & joint = iter->second;
			os << "\t\t<bone name=\"" << RemoveQuote(joint.name);

			int parent_id;
			if (-1 == joint.parent_id)
			{
				parent_id = -1;
			}
			else
			{
				std::map<int, int>::const_iterator fiter = joint_id_to_index.find(joint.parent_id);
				BOOST_ASSERT(fiter != joint_id_to_index.end());

				parent_id = fiter->second;
			}

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

	void MeshMLObj::WriteMaterialChunk(std::ostream& os)
	{
		os << "\t<materials_chunk>" << std::endl;
		for (size_t i = 0; i < materials_.size(); ++ i)
		{
			Material const & mtl = materials_[i];
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
				<< "\" shininess=\"" << mtl.shininess << "\"";

			if (!mtl.texture_slots.empty())
			{
				os << ">" << std::endl;

				os << "\t\t\t<textures_chunk>" << std::endl;
				for (std::vector<TextureSlot>::const_iterator iter = mtl.texture_slots.begin();
					iter != mtl.texture_slots.end(); ++ iter)
				{
					os << "\t\t\t\t<texture type=\"" << RemoveQuote(iter->first)
						<< "\" name=\"" << RemoveQuote(iter->second) << "\"/>" << std::endl;
				}
				os << "\t\t\t</textures_chunk>" << std::endl;

				os << "\t\t</material>" << std::endl;
			}
			else
			{
				os << "/>" << std::endl;
			}
		}
		os << "\t</materials_chunk>" << std::endl;
	}

	void MeshMLObj::WriteMeshChunk(std::ostream& os, std::map<int, int> const & joint_id_to_index, int vertex_export_settings)
	{
		os << "\t<meshes_chunk>" << std::endl;
		for (size_t i = 0; i < meshes_.size(); ++ i)
		{
			Mesh const & mesh = meshes_[i];
			os << "\t\t<mesh name=\"" << RemoveQuote(mesh.name)
				<< "\" mtl_id=\"" << mesh.material_id << "\">" << std::endl;

			os << "\t\t\t<vertices_chunk>" << std::endl;
			for (std::vector<Vertex>::const_iterator iter = mesh.vertices.begin();
				iter != mesh.vertices.end(); ++ iter)
			{
				Vertex const & vertex = *iter;
				os << "\t\t\t\t<vertex x=\"" << vertex.position.x()
					<< "\" y=\"" << vertex.position.y()
					<< "\" z=\"" << vertex.position.z() << "\"";
				if (vertex_export_settings != VES_None)
				{
					os << ">" << std::endl;

					if (vertex_export_settings & VES_Normal)
					{
						os << "\t\t\t\t\t<normal x=\"" << vertex.normal.x()
							<< "\" y=\"" << vertex.normal.y()
							<< "\" z=\"" << vertex.normal.z() << "\"/>" << std::endl;
					}

					if (vertex_export_settings & VES_TangentQuat)
					{
						os << "\t\t\t\t\t<tangent_quat x=\"" << vertex.tangent_quat.x()
							<< "\" y=\"" << vertex.tangent_quat.y()
							<< "\" z=\"" << vertex.tangent_quat.z()
							<< "\" w=\"" << vertex.tangent_quat.w() << "\"/>" << std::endl;
					}

					if (vertex_export_settings & VES_Texcoord)
					{
						switch (vertex.texcoord_components)
						{
						case 1:
							for (std::vector<float3>::const_iterator titer = vertex.texcoords.begin();
								titer != vertex.texcoords.end(); ++ titer)
							{
								os << "\t\t\t\t\t<tex_coord u=\"" << titer->x() << "\"/>" << std::endl;
							}
							break;

						case 2:
							for (std::vector<float3>::const_iterator titer = vertex.texcoords.begin();
								titer != vertex.texcoords.end(); ++ titer)
							{
								os << "\t\t\t\t\t<tex_coord u=\"" << titer->x()
									<< "\" v=\"" << titer->y() << "\"/>" << std::endl;
							}
							break;

						case 3:
							for (std::vector<float3>::const_iterator titer = vertex.texcoords.begin();
								titer != vertex.texcoords.end(); ++ titer)
							{
								os << "\t\t\t\t\t<tex_coord u=\"" << titer->x()
									<< "\" v=\"" << titer->y()
									<< "\" w=\"" << titer->z() << "\"/>" << std::endl;
							}
							break;

						default:
							break;
						}
					}

					for (std::vector<JointBinding>::const_iterator jiter = vertex.binds.begin();
						jiter != vertex.binds.end(); ++ jiter)
					{
						std::map<int, int>::const_iterator fiter = joint_id_to_index.find(jiter->first);
						BOOST_ASSERT(fiter != joint_id_to_index.end());

						os << "\t\t\t\t\t<weight bone_index=\"" << fiter->second
							<< "\" weight=\"" << jiter->second << "\"/>" << std::endl;
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
			for (std::vector<Triangle>::const_iterator iter = mesh.triangles.begin();
				iter != mesh.triangles.end(); ++ iter)
			{
				Triangle const & tri = *iter;
				os << "\t\t\t\t<triangle a=\"" << tri.vertex_index[0]
					<< "\" b=\"" << tri.vertex_index[1]
					<< "\" c=\"" << tri.vertex_index[2] << "\"/>" << std::endl;
			}
			os << "\t\t\t</triangles_chunk>" << std::endl;

			os << "\t\t</mesh>" << std::endl;
		}
		os << "\t</meshes_chunk>" << std::endl;
	}

	void MeshMLObj::WriteKeyframeChunk(std::ostream& os, std::map<int, int> const & joint_id_to_index)
	{
		os << "\t<key_frames_chunk start_frame=\"" << start_frame_
			<< "\" end_frame=\"" << end_frame_
			<< "\" frame_rate=\"" << frame_rate_ << "\">" << std::endl;
		for (size_t i = 0; i < keyframes_.size(); ++ i)
		{
			Keyframes const & kf = keyframes_[i];			
			std::map<int, int>::const_iterator fiter = joint_id_to_index.find(kf.joint_id);
			if (fiter != joint_id_to_index.end())
			{
				size_t frames = std::min<size_t>(kf.bind_reals.size(), kf.bind_duals.size());

				os << "\t\t<key_frame joint=\"" << RemoveQuote(joints_[kf.joint_id].name) << "\">" << std::endl;
				for (size_t j = 0; j < frames; ++ j)
				{
					Quaternion const & bind_real = kf.bind_reals[j];
					Quaternion const & bind_dual = kf.bind_duals[j];

					os << "\t\t\t<key>" << std::endl;
					os << "\t\t\t\t<bind_real x=\"" << bind_real.x()
						<< "\" y=\"" << bind_real.y()
						<< "\" z=\"" << bind_real.z()
						<< "\" w=\"" << bind_real.w() << "\"/>" << std::endl;
					os << "\t\t\t\t<bind_dual x=\"" << bind_dual.x()
						<< "\" y=\"" << bind_dual.y()
						<< "\" z=\"" << bind_dual.z()
						<< "\" w=\"" << bind_dual.w() << "\"/>" << std::endl;
					os << "\t\t\t</key>" << std::endl;
				}
				os << "\t\t</key_frame>" << std::endl;
			}
		}
		os << "\t</key_frames_chunk>" << std::endl;
	}

	void MeshMLObj::OptimizeJoints()
	{
		std::set<int> joints_used;

		// Find all joints used in the mesh list
		for (size_t i = 0; i < meshes_.size(); ++ i)
		{
			Mesh const & mesh = meshes_[i];
			for (std::vector<Vertex>::const_iterator iter = mesh.vertices.begin();
				iter != mesh.vertices.end(); ++ iter)
			{
				Vertex const & vertex = *iter;
				for (std::vector<JointBinding>::const_iterator jiter = vertex.binds.begin();
					jiter != vertex.binds.end(); ++ jiter)
				{
					joints_used.insert(jiter->first);
				}
			}
		}

		// Traverse the joint list and see if used joints' parents can be added
		std::set<int> parent_joints_used;
		for (std::map<int, Joint>::const_iterator iter = joints_.begin(); iter != joints_.end(); ++ iter)
		{
			if (joints_used.find(iter->first) != joints_used.end())
			{
				Joint const * j = &iter->second;
				while (j->parent_id != -1)
				{
					parent_joints_used.insert(j->parent_id);
					j = &joints_[j->parent_id];
				}
			}
		}

		for (std::set<int>::const_iterator iter = parent_joints_used.begin();
			iter != parent_joints_used.end(); ++ iter)
		{
			joints_used.insert(*iter);
		}

		// Traverse the joint list and erase those never recorded by joints_used
		for (std::map<int, Joint>::iterator iter = joints_.begin(); iter != joints_.end();)
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

		for (std::map<int, Joint>::iterator iter = joints_.begin(); iter != joints_.end(); ++ iter)
		{
			BOOST_ASSERT(iter->second.parent_id < iter->first);
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

		typedef BOOST_TYPEOF(meshes_) MeshesType;
		BOOST_FOREACH(MeshesType::reference mesh, meshes_)
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
					std::stringstream ss;
					ss << "combined_for_mtl_" << i;

					Mesh opt_mesh;
					opt_mesh.material_id = static_cast<int>(i);
					opt_mesh.name = ss.str();

					for (size_t j = 0; j < meshes_to_combine.size(); ++ j)
					{
						int base = static_cast<int>(opt_mesh.vertices.size());
						Mesh& mesh = meshes_to_combine[j];
						opt_mesh.vertices.insert(opt_mesh.vertices.end(),
							mesh.vertices.begin(), mesh.vertices.end());

						for (std::vector<Triangle>::iterator iter = mesh.triangles.begin();
							iter != mesh.triangles.end(); ++ iter)
						{
							Triangle tri;
							tri.vertex_index[0] = iter->vertex_index[0] + base;
							tri.vertex_index[1] = iter->vertex_index[1] + base;
							tri.vertex_index[2] = iter->vertex_index[2] + base;
							opt_mesh.triangles.push_back(tri);
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

	void MeshMLObj::MatrixToDQ(float4x4 const & mat, Quaternion& real, Quaternion& dual)
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
}  // namespace KlayGE
