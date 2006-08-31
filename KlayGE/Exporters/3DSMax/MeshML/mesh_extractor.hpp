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

#include <fstream>
#include <vector>
#include <string>
#include <map>

#include "meshml.hpp"

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
		Point3 tangent;
	};

	typedef std::vector<triangle_t> triangles_t;

	typedef std::pair<std::string, std::string> texture_slot_t;
	typedef std::vector<texture_slot_t> texture_slots_t;

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

		texture_slots_t texture_slots;

		vertices_t		vertices;
		triangles_t		triangles;

		vertex_elements_t vertex_elements;
	};

	class meshml_extractor
	{
	public:
		meshml_extractor(int joints_per_ver, int start_frame, int end_frame);

		void export_objects(std::vector<INode*> const & nodes);
		void write_xml(std::basic_string<TCHAR> const & file_name);

	private:
		void extract_object(INode* node);
		void extract_bone(INode* node);
		void remove_redundant_joints();

		Point3 point_from_matrix(Matrix3 const & mat);
		Quat quat_from_matrix(Matrix3 const & mat);
		Modifier* find_modifier(INode* node, Class_ID const & class_id);
		void physique_modifier(INode* node, std::string const & root_name,
			std::vector<std::pair<Point3, binds_t> >& positions);
		void skin_modifier(INode* node, std::string const & root_name,
			std::vector<std::pair<Point3, binds_t> >& positions);

	private:
		typedef std::vector<object_info_t> objects_info_t;

		objects_info_t objs_info_;

		joints_t joints_;
		int joints_per_ver_;

		int start_frame_;
		int end_frame_;
		int frame_rate_;
		key_frames_t kfs_;
	};
}

#endif		// _MESHML_EXTRACTOR_HPP
