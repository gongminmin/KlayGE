// mesh_extractor.hpp
// KlayGE 3DSMax网格数据析取类 实现文件
// Ver 2.5.0
// 版权所有(C) 龚敏敏, 2005
// Homepage: http://www.klayge.org
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

#include <MeshMLLib/MeshMLLib.hpp>

class IPhysiqueExport;
class ISkin;

namespace KlayGE
{
	typedef std::vector<std::pair<INode*, float> > binds_t;

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

	struct export_vertex_attrs
	{
		bool normal;
		bool tangent_quat;
		bool tex;
	};

	class meshml_extractor
	{
	public:
		meshml_extractor(INode* root_node, int joints_per_ver, int cur_time, int start_frame, int end_frame, bool combine_meshes);

		void export_objects(std::vector<INode*> const & nodes);
		void write_xml(std::string const & file_name, export_vertex_attrs const & eva);

	private:
		void extract_object(INode* node);
		void extract_bone(INode* node);

		float4x4 rh_to_lh(Matrix3 const & mat);

		void find_joints(INode* node);
		void extract_all_joint_tms();
		void add_joint_weight(binds_t& binds, INode* joint_node, float weight);

		void physique_modifier(Modifier* mod, INode* node,
			std::vector<std::pair<Point3, binds_t> >& positions);
		void skin_modifier(Modifier* mod, INode* node,
			std::vector<std::pair<Point3, binds_t> >& positions);

		void get_material(std::vector<int>& mtls_id, std::vector<std::map<int, std::pair<Matrix3, int> > >& uv_transss, Mtl* max_mtl);

	private:
		MeshMLObj meshml_obj_;
		std::vector<int> objs_mtl_id_;
		std::map<INode*, int> joint_node_to_id_;

		INode* root_node_;

		int joints_per_ver_;
		bool combine_meshes_;

		std::map<INode*, Matrix3> joint_nodes_;

		int cur_time_;
		int start_frame_;
		int end_frame_;

		std::vector<IPhysiqueExport*> physiques_;
		std::vector<Modifier*> physique_mods_;
		std::vector<ISkin*> skins_;
		std::vector<Modifier*> skin_mods_;
	};
}

#endif		// _MESHML_EXTRACTOR_HPP
