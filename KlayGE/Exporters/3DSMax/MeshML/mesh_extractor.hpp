// mesh_extractor.hpp
// KlayGE 3DSMax网格数据析取类 实现文件
// Ver 2.5.0
// 版权所有(C) 龚敏敏, 2005
// Homepage: http://klayge.sourceforge.net
//
// 2.5.0
// 初次建立 (2005.5.1)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _MESHML_EXTRACTOR_HPP
#define _MESHML_EXTRACTOR_HPP

#include <vector>
#include <string>
#include <map>

class IPhysiqueExport;
class ISkin;

namespace KlayGE
{
	enum vertex_element_usage
	{
		// vertex positions
		VEU_Position,
		// vertex normals included (for lighting)
		VEU_Normal,
		// Vertex colors - diffuse
		VEU_Diffuse,
		// Vertex colors - specular
		VEU_Specular,
		// Vertex blend weights
		VEU_BlendWeight,
		// Vertex blend indices
		VEU_BlendIndex,
		// at least one set of texture coords (exact number specified in class)
		VEU_TextureCoord,
		// Vertex tangent
		VEU_Tangent,
		// Vertex binormal
		VEU_Binormal
	};

	struct vertex_element_t
	{
		vertex_element_t(vertex_element_usage usage, unsigned char usage_index, unsigned char num_components)
			: usage(usage), usage_index(usage_index), num_components(num_components)
		{
		}

		vertex_element_usage usage;
		unsigned char usage_index;
		unsigned char num_components;

		friend bool operator==(vertex_element_t const & lhs, vertex_element_t const & rhs)
		{
			return (lhs.usage == rhs.usage) && (lhs.usage_index == rhs.usage_index) && (lhs.num_components == rhs.num_components);
		}
		friend bool operator!=(vertex_element_t const & lhs, vertex_element_t const & rhs)
		{
			return !(lhs == rhs);
		}
	};

	typedef std::vector<std::pair<std::string, float> > binds_t;

	struct vertex_t
	{
		Point3 pos;
		Point3 normal;
		Point3 tangent;
		Point3 binormal;
		std::vector<Point2> tex;

		binds_t binds;
	};

	typedef std::vector<vertex_t> vertices_t;

	struct triangle_t
	{
		int	vertex_index[3];
	};

	typedef std::vector<triangle_t> triangles_t;

	typedef std::pair<std::string, std::string> texture_slot_t;
	typedef std::vector<texture_slot_t> texture_slots_t;

	struct material_t
	{
		Color ambient;
		Color diffuse;
		Color specular;
		Color emit;
		float opacity;
		float specular_level;
		float shininess;

		texture_slots_t texture_slots;
	};
	typedef std::vector<material_t> materials_t;

	typedef std::vector<vertex_element_t> vertex_elements_t;

	struct joint_t
	{
		Point3 pos;
		Quat quat;

		std::string parent_name;
	};

	typedef std::map<std::string, joint_t> joints_t;

	struct key_frame_t
	{
		std::string joint;

		std::vector<Point3> positions;
		std::vector<Quat> quaternions;
	};

	typedef std::vector<key_frame_t> key_frames_t;

	struct object_info_t
	{
		std::string		name;

		size_t			mtl_id;

		vertices_t		vertices;
		triangles_t		triangles;

		vertex_elements_t vertex_elements;
	};

	struct joint_and_mat_t
	{
		INode* joint_node;
		Matrix3 mesh_init_matrix;
	};

	struct export_vertex_attrs
	{
		bool normal;
		bool tangent;
		bool binormal;
		bool tex;
	};

	class meshml_extractor
	{
	public:
		meshml_extractor(INode* root_node, int joints_per_ver, int cur_time, int start_frame, int end_frame, bool mesh_opt);

		void export_objects(std::vector<INode*> const & nodes);
		void write_xml(std::string const & file_name, export_vertex_attrs const & eva);

	private:
		void extract_object(INode* node);
		void extract_bone(INode* node);
		void remove_redundant_joints();
		void remove_redundant_mtls();
		void mesh_optimize();

		Point3 point_from_matrix(Matrix3 const & mat);
		Quat quat_from_matrix(Matrix3 const & mat);

		void find_joints(INode* node);
		void extract_all_joint_tms();
		void add_joint_weight(binds_t& binds, std::string const & joint_name, float weight);

		void physique_modifier(Modifier* mod, INode* node,
			std::vector<std::pair<Point3, binds_t> >& positions);
		void skin_modifier(Modifier* mod, INode* node,
			std::vector<std::pair<Point3, binds_t> >& positions);

		void get_material(materials_t& mtls, std::vector<std::map<int, std::pair<Matrix3, int> > >& uv_transss, Mtl* max_mtl);

	private:
		typedef std::vector<object_info_t> objects_info_t;
		objects_info_t objs_info_;
		materials_t objs_mtl_;

		INode* root_node_;

		joints_t joints_;
		int joints_per_ver_;
		bool mesh_opt_;

		std::map<std::string, joint_and_mat_t> joint_nodes_;

		int cur_time_;
		int start_frame_;
		int end_frame_;
		int frame_rate_;
		key_frames_t kfs_;

		std::vector<IPhysiqueExport*> physiques_;
		std::vector<Modifier*> physique_mods_;
		std::vector<ISkin*> skins_;
		std::vector<Modifier*> skin_mods_;
	};
}

#endif		// _MESHML_EXTRACTOR_HPP
