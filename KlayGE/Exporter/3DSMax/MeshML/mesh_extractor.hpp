#ifndef _MESHML_EXTRACTOR_HPP
#define _MESHML_EXTRACTOR_HPP

#include <fstream>
#include <vector>
#include <string>
#include <map>

#include "meshml.hpp"

typedef std::map<std::string, float> binds_t;

struct vertex_t
{
	Point3 pos;
	Point2 tex;

	binds_t binds;
};

typedef std::vector<vertex_t> vertices_t;

struct triangle_t
{
	int	vertex_index[3];
};

typedef std::vector<triangle_t> triangles_t;

struct bind_bone_t
{
	Point3 bind_pos;
	Quat bind_quat;

	std::string parent_name;
};

typedef std::map<std::string, bind_bone_t> bones_t;

typedef std::pair<std::string, std::string> texture_slot_t;
typedef std::vector<texture_slot_t> texture_slots_t;

struct object_info_t
{
	std::string		name;

	texture_slots_t texture_slots;

	vertices_t		vertices;
	triangles_t		triangles;
};

class meshml_extractor
{
public:
	meshml_extractor();

	void export_objects(std::vector<INode*> const & nodes);
	void write_xml(std::basic_string<TCHAR> const & file_name);

private:
	void extract_object(INode* node);

private:
	typedef std::vector<object_info_t> objects_info_t;

	objects_info_t objs_info_;
};

#endif		// _MESHML_EXTRACTOR_HPP
