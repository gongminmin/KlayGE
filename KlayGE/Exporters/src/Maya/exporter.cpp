#ifndef NOMINMAX
	#define NOMINMAX
#endif

#include <maya/MLibrary.h>
#include <maya/MGlobal.h>
#include <maya/MArgList.h>
#include <maya/MFloatArray.h>
#include <maya/MFloatPointArray.h>
#include <maya/MFloatVectorArray.h>
#include <maya/MFloatMatrix.h>
#include <maya/MMatrix.h>
#include <maya/MItDag.h>
#include <maya/MItDependencyNodes.h>
#include <maya/MItSelectionList.h>
#include <maya/MItGeometry.h>
#include <maya/MItMeshPolygon.h>
#include <maya/MItMeshVertex.h>
#include <maya/MDagPath.h>
#include <maya/MDagPathArray.h>
#include <maya/MFnPlugin.h>
#include <maya/MFnMesh.h>
#include <maya/MFnNurbsSurface.h>
#include <maya/MFnIkJoint.h>
#include <maya/MFnSkinCluster.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MFnLambertShader.h>
#include <maya/MFnPhongShader.h>
#include <maya/MFnBlinnShader.h>
#include <maya/MAnimControl.h>
#include <maya/MPlug.h>
#include <maya/MPlugArray.h>
#include <maya/MPxCommand.h>
#include <maya/MPxFileTranslator.h>
#include <maya/MFileIO.h>
#include <maya/MIOStream.h>
#include <maya/MDistance.h>

#include <fstream>
#include <iostream>
#include <set>
#include <map>
#include <vector>

#include <boost/assert.hpp>

#include <KFL/KFL.hpp>
#include <MeshMLLib/MeshMLLib.hpp>

using namespace KlayGE;

class MayaMeshExporter
{
public:
	struct IndexAttributes
	{
		int id;
		int vertex_index;
		int normal_index;
		int uv_index;
		int tangent_index;

		IndexAttributes()
			: id(0), vertex_index(0), normal_index(0), uv_index(0), tangent_index(0)
		{
		}

		bool operator<(IndexAttributes const & rhs) const
		{
			if (vertex_index < rhs.vertex_index)
			{
				return true;
			}
			else if (vertex_index > rhs.vertex_index)
			{
				return false;
			}

			if (normal_index < rhs.normal_index)
			{
				return true;
			}
			else if (normal_index > rhs.normal_index)
			{
				return false;
			}

			if (uv_index < rhs.uv_index)
			{
				return true;
			}
			else if (uv_index > rhs.uv_index)
			{
				return false;
			}

			return tangent_index < rhs.tangent_index;
		}
	};

	MayaMeshExporter();

	void WriteMeshFile(std::string const & save_file);

	void ExportMayaFile(std::string const & open_file);
	void ExportMayaNodes(MItDag& dag_iterator);

private:
	void ExportNurbsSurface(MString const & obj_name, MFnNurbsSurface& fn_surface, MDagPath& dag_path);
	void ExportMesh(MString const & obj_name, MFnMesh& fn_mesh, MDagPath& dag_path);
	void ExportJoint(MDagPath const * parent_path, MFnIkJoint& fn_joint, MDagPath& dag_path);
	void ExportKeyframe(int kfs_id, int frame_id, MDagPath& dag_path, MMatrix const & inv_parent);
	int ExportMaterialAndTexture(MObject* shader, MObjectArray const & textures);
	int AddDefaultMaterial();

	MeshMLObj meshml_obj_;

	std::map<std::string, int> joint_to_id_;
	std::map<int, MDagPath> joint_id_to_path_;

	std::vector<std::pair<std::shared_ptr<MFnSkinCluster>, std::vector<MObject>>> skin_clusters_;
};

MayaMeshExporter::MayaMeshExporter()
	: meshml_obj_(static_cast<float>(MDistance(1, MDistance::internalUnit()).asMeters()))
{
	meshml_obj_.NumFrames(0);
	meshml_obj_.FrameRate(30);

	// Always add a default material
	this->AddDefaultMaterial();
}

void MayaMeshExporter::WriteMeshFile(std::string const & save_file)
{
	std::ofstream out_file(save_file.c_str());
	meshml_obj_.WriteMeshML(out_file);
}

void MayaMeshExporter::ExportMayaFile(std::string const & open_file)
{
	MStatus status = MFileIO::open(open_file.c_str(), NULL, true);
	if (!status)
	{
		std::cout << "MFileIO::open(" << open_file << ") failed - " << status.errorString() << std::endl;
		return;
	}

	// Add meshes
	MItDag dag_iterator(MItDag::kDepthFirst, MFn::kInvalid, &status);
	this->ExportMayaNodes(dag_iterator);
}

void MayaMeshExporter::ExportMayaNodes(MItDag& dag_iterator)
{
	int start_frame = static_cast<int>(MAnimControl::minTime().as(MTime::kNTSCField));
	int end_frame = static_cast<int>(MAnimControl::maxTime().as(MTime::kNTSCField));
	int num_frames = end_frame - start_frame + 1;
	meshml_obj_.NumFrames(num_frames);

	MAnimControl::setCurrentTime(MTime(start_frame, MTime::kNTSCField));

	for (MItDependencyNodes dn(MFn::kSkinClusterFilter); !dn.isDone(); dn.next())
	{
		MStatus status = MS::kSuccess;
		MObject object = dn.item();
		std::shared_ptr<MFnSkinCluster> skin_cluster = MakeSharedPtr<MFnSkinCluster>(object, &status);

		std::vector<MObject> objs;
		unsigned int num_geometries = skin_cluster->numOutputConnections();
		for (unsigned int i = 0; i < num_geometries; ++ i) 
		{
			unsigned int index = skin_cluster->indexForOutputConnection(i);
			objs.push_back(skin_cluster->outputShapeAtIndex(index));
		}

		if (num_geometries > 0)
		{
			skin_clusters_.emplace_back(skin_cluster, objs);
		}

		MDagPathArray influence_paths;
		int num_influence_objs = skin_cluster->influenceObjects(influence_paths, &status);

		MDagPath joint_path, root_path;
		for (int i = 0; i < num_influence_objs; ++ i)
		{
			joint_path = influence_paths[i];
			if (joint_path.hasFn(MFn::kJoint))
			{
				// Try to retrieve the root path
				root_path = joint_path;
				while (joint_path.length() > 0)
				{
					joint_path.pop();
					if (joint_path.hasFn(MFn::kJoint) && (joint_path.length() > 0))
					{
						root_path = joint_path;
					}
				}

				if (root_path.hasFn(MFn::kJoint))
				{
					MFnIkJoint fn_joint(root_path);

					// Don't work on existing joints
					auto iter = joint_to_id_.find(fn_joint.fullPathName().asChar());
					if (iter == joint_to_id_.end())
					{
						this->ExportJoint(NULL, fn_joint, root_path);
					}
				}
			}
		}
	}
	
	MDagPath dag_path;
	for (; !dag_iterator.isDone(); dag_iterator.next())
	{
		MStatus status = dag_iterator.getPath(dag_path);
		if (!status)
		{
			std::cout << "Fail to get DAG path." << std::endl;
			continue;
		}

		MString obj_name = dag_iterator.partialPathName();
		switch (dag_path.apiType())
		{
		case MFn::kTransform:
			{
				/*MFnTransform fnTransform(dagPath, &status);
				if (status == MS::kSuccess)
				{
					MFloatMatrix matrix = fnTransform.transformation().asMatrix();
					// TODO: how to handle transformations?
				}
				else
				std::cout << "Fail to initialize transform node." << std::endl;*/
			}
			break;

		case MFn::kMesh:
			{
				MFnMesh fn_mesh(dag_path, &status);
				if (MS::kSuccess == status)
				{
					if (!fn_mesh.isIntermediateObject())
					{
						this->ExportMesh(obj_name, fn_mesh, dag_path);
					}
					else
					{
						std::cout << "Intermediate objects " << fn_mesh.name().asChar()
							<< " will be ignored." << std::endl;
					}
				}
				else
				{
					status.perror("MFnMesh");
				}
			}
			break;

		case MFn::kNurbsSurface:
			{
				MFnNurbsSurface fn_surface(dag_path, &status);
				if (status == MS::kSuccess)
				{
					this->ExportNurbsSurface(obj_name, fn_surface, dag_path);
				}
				else
				{
					status.perror("MFnNurbsSurface");
				}
			}
			break;

		case MFn::kJoint:  // Already handled
			break;

		case MFn::kInvalid:
		case MFn::kWorld:
		case MFn::kCamera:
		case MFn::kGroundPlane:
		default:
			std::cout << "MDagPath::apiType()=" << dag_path.apiType() << " ["
				<< dag_path.fullPathName() << "] not supported." << std::endl;
			break;
		}
	}

	if (num_frames > 0)
	{
		std::map<int, int> joint_id_to_kfs_id;
		for (auto const & joint : joint_to_id_)
		{
			int kfs_id = meshml_obj_.AllocKeyframes();
			meshml_obj_.SetKeyframes(kfs_id, joint.second);

			joint_id_to_kfs_id.emplace(joint.second, kfs_id);
		}

		for (int i = 0; i < num_frames; ++ i)
		{
			MAnimControl::setCurrentTime(MTime(i + start_frame, MTime::kNTSCField));
			for (auto const & joint : joint_to_id_)
			{
				MDagPath& joint_dag_path = joint_id_to_path_[joint.second];
				MMatrix inv_parent;
				if (1 == joint_dag_path.length())
				{
					inv_parent = MMatrix::identity;
				}
				else
				{
					MDagPath parent_path = joint_dag_path;
					parent_path.pop();

					inv_parent = parent_path.inclusiveMatrixInverse();
				}

				int kfs_id = joint_id_to_kfs_id[joint_to_id_[joint_dag_path.fullPathName().asChar()]];
				this->ExportKeyframe(kfs_id, i, joint_dag_path, inv_parent);
			}
		}
	}
}

void MayaMeshExporter::ExportNurbsSurface(MString const & obj_name, MFnNurbsSurface& fn_surface, MDagPath& dag_path)
{
	MFnDagNode surface_dn(dag_path);
	MPlug surface_plug;

	int modeU = 0, modeV = 0, numberU = 0, numberV = 0;
	surface_plug = surface_dn.findPlug("modeU");
	surface_plug.getValue(modeU);
	surface_plug = surface_dn.findPlug("numberU");
	surface_plug.getValue(numberU);
	surface_plug = surface_dn.findPlug("modeV");
	surface_plug.getValue(modeV);
	surface_plug = surface_dn.findPlug("numberV");
	surface_plug.getValue(numberV);

	// Set tesselation parameters
	MTesselationParams params;
	params.setFormatType(MTesselationParams::kGeneralFormat);
	params.setOutputType(MTesselationParams::kTriangles);

	switch (modeU)
	{
	case 1:
		params.setUIsoparmType(MTesselationParams::kSurface3DEquiSpaced);
		break;

	case 2:
		params.setUIsoparmType(MTesselationParams::kSurfaceEquiSpaced);
		break;

	case 3:
		params.setUIsoparmType(MTesselationParams::kSpanEquiSpaced);
		break;

	case 4:
		params.setUIsoparmType(MTesselationParams::kSurfaceEquiSpaced);
		break;

	default:
		break;
	}

	switch (modeV)
	{
	case 1:
		params.setVIsoparmType(MTesselationParams::kSurface3DEquiSpaced);
		break;

	case 2:
		params.setVIsoparmType(MTesselationParams::kSurfaceEquiSpaced);
		break;

	case 3:
		params.setVIsoparmType(MTesselationParams::kSpanEquiSpaced);
		break;

	case 4:
		params.setVIsoparmType(MTesselationParams::kSurfaceEquiSpaced);
		break;

	default:
		break;
	}

	params.setUNumber(numberU);
	params.setVNumber(numberV);

	// Try convert the NURBS surface to polygon mesh
	MStatus status = MS::kSuccess;
	MDagPath mesh_path = dag_path;
	MObject mesh_parent = mesh_path.node();
	MObject converted_mesh = fn_surface.tesselate(params, mesh_parent, &status);
	if (status == MS::kSuccess)
	{
		mesh_path.push(converted_mesh);

		// Export the new generated mesh
		MFnMesh fn_converted_mesh(converted_mesh, &status);
		this->ExportMesh(obj_name, fn_converted_mesh, dag_path);

		// Remove the mesh to keep the scene clean
		MFnDagNode parent_node(mesh_parent, &status);
		parent_node.removeChild(converted_mesh);
	}
	else
	{
		status.perror("MFnNurbsSurface::tesselate");
	}
}

void MayaMeshExporter::ExportMesh(MString const & obj_name, MFnMesh& fn_mesh, MDagPath& dag_path)
{
	MStatus status = MS::kSuccess;
	MObject component = MObject::kNullObj;
	MSpace::Space space = MSpace::kWorld;

	MItMeshPolygon poly_iterator(dag_path, component, &status);
	if (status != MS::kSuccess)
	{
		std::cout << "Fail to initialize polygons." << std::endl;
		return;
	}

	int poly_vertex_count = poly_iterator.polygonVertexCount();
	if (poly_vertex_count > 2)
	{
		int mesh_id = meshml_obj_.AllocMesh();

		// Start to save mesh
		int num_vertices = fn_mesh.numVertices();
		MString* uv_set_name = NULL;

		// Get fixed-pipeline shader (material)
		MObjectArray shader_objects;
		MIntArray shader_indices;
		fn_mesh.getConnectedShaders(0, shader_objects, shader_indices);

		// Get UV set names and associated textures
		// FIXME: At present only the first UV set is recorded and used
		MObjectArray texture_objects;
		MStringArray uv_set_names;
		fn_mesh.getUVSetNames(uv_set_names);
		if (uv_set_names.length() > 0)
		{
			uv_set_name = &uv_set_names[0];
			fn_mesh.getAssociatedUVSetTextures(*uv_set_name, texture_objects);
		}

		// Export shader (material) and texture object
		// FIXME: At present only the first shader is recorded and used
		int mtl_id = this->ExportMaterialAndTexture((shader_objects.length() > 0) ? &shader_objects[0] : NULL,
			texture_objects);

		meshml_obj_.SetMesh(mesh_id, mtl_id, obj_name.asChar(), 1);

		// Get points
		MFloatPointArray points;
		fn_mesh.getPoints(points, space);

		// Get normals
		MFloatVectorArray normals;
		fn_mesh.getNormals(normals, space);

		// Get UV coords of default UV set
		MFloatArray u_array, v_array;
		fn_mesh.getUVs(u_array, v_array, uv_set_name);
		bool has_texcoords = (fn_mesh.numUVs() > 0);

		// Get tangents of default UV set
		MFloatVectorArray tangents;
		fn_mesh.getTangents(tangents, space, uv_set_name);

		// Get binormals of default UV set
		MFloatVectorArray binormals;
		fn_mesh.getBinormals(binormals, space, uv_set_name);

		// Get associated joints and weights
		std::vector<std::vector<float>> weights(num_vertices);
		std::vector<std::vector<MDagPath>> joint_paths(num_vertices);

		int skin_cluster_index = -1;
		for (size_t i = 0; (i < skin_clusters_.size()) && (skin_cluster_index < 0); ++ i)
		{
			for (size_t j = 0; j < skin_clusters_[i].second.size(); ++ j)
			{
				if (skin_clusters_[i].second[j] == fn_mesh.object())
				{
					skin_cluster_index = static_cast<int>(i);
					break;
				}
			}
		}

		if (skin_cluster_index >= 0)
		{
			unsigned int num_weights;
			MItGeometry geom_iterator(dag_path);
			for (int i = 0; !geom_iterator.isDone(); geom_iterator.next(), ++ i)
			{
				MObject geom_component = geom_iterator.component();
				MFloatArray vertex_weights;
				status = skin_clusters_[skin_cluster_index].first->getWeights(dag_path, geom_component, vertex_weights, num_weights);
				if (status != MS::kSuccess)
				{
					std::cout << "Fail to retrieve vertex weights." << std::endl;
					continue;
				}

				weights[i].resize(vertex_weights.length());
				joint_paths[i].resize(vertex_weights.length());

				MDagPathArray influence_objs;
				skin_clusters_[skin_cluster_index].first->influenceObjects(influence_objs, &status);
				if (MS::kSuccess == status)
				{
					for (unsigned int j = 0; j < influence_objs.length(); ++ j)
					{
						auto iter = joint_to_id_.find(influence_objs[j].fullPathName().asChar());
						BOOST_ASSERT(iter != joint_to_id_.end());

						joint_paths[i][j] = influence_objs[j];
						weights[i][j] = vertex_weights[j];
					}
				}
				else
				{
					std::cout << "Fail to retrieve influence objects for the skin cluster." << std::endl;
				}
			}
		}
		else
		{
			// Static mesh connect on joint

			MDagPath parent_path = dag_path;
			parent_path.pop();

			while (parent_path.length() > 0)
			{
				auto iter = joint_to_id_.find(parent_path.fullPathName().asChar());
				if (iter != joint_to_id_.end())
				{
					MItGeometry geom_iterator(dag_path);
					for (int i = 0; !geom_iterator.isDone(); geom_iterator.next(), ++ i)
					{
						weights[i].resize(1);
						joint_paths[i].resize(1);

						weights[i][0] = 1;
						joint_paths[i][0] = parent_path;
					}

					break;
				}

				parent_path.pop();
			}
		}

		// Traverse polygons
		MItMeshPolygon polygon_iterator(dag_path, component, &status);
		if (status != MS::kSuccess)
		{
			std::cout << "Fail to traverse polygons." << std::endl;
			return;
		}

		int id_count = 0;
		int tri_indices[4];
		std::set<IndexAttributes> index_attributes;
		for (; !polygon_iterator.isDone(); polygon_iterator.next())
		{
			for (int i = 0; i < poly_vertex_count; ++ i)
			{
				IndexAttributes attr;
				attr.vertex_index = polygon_iterator.vertexIndex(i);
				attr.normal_index = polygon_iterator.normalIndex(i);
				if (has_texcoords)
				{
					polygon_iterator.getUVIndex(i, attr.uv_index);
					attr.tangent_index = polygon_iterator.tangentIndex(i);
				}

				auto itr = index_attributes.find(attr);
				if (itr != index_attributes.end())
				{
					// Reuse existing vertex
					tri_indices[i] = itr->id;
				}
				else
				{
					// Create a new vertex and add it to the mesh
					MFloatVector pos = points[attr.vertex_index];
					MFloatVector norm = normals[attr.normal_index];
					std::vector<float> const & vert_weights = weights[attr.vertex_index];
					std::vector<MDagPath> const & joint_dag_paths = joint_paths[attr.vertex_index];
					BOOST_ASSERT(vert_weights.size() == joint_dag_paths.size());
					size_t num_binds = vert_weights.size();

					int vertex_id = meshml_obj_.AllocVertex(mesh_id, 0);
					if (has_texcoords)
					{
						MFloatVector tang = tangents[attr.tangent_index];
						MFloatVector bi = binormals[attr.tangent_index];
						float u = u_array[attr.uv_index];
						float v = v_array[attr.uv_index];

						std::vector<KlayGE::float3> texcoords;
						texcoords.push_back(KlayGE::float3(u, v, 0));

						meshml_obj_.SetVertex(mesh_id, 0, vertex_id, KlayGE::float3(pos[0], pos[1], pos[2]),
							KlayGE::float3(tang[0], tang[1], tang[2]), KlayGE::float3(bi[0], bi[1], bi[2]),
							KlayGE::float3(norm[0], norm[1], norm[2]), 2, texcoords);
					}
					else
					{
						meshml_obj_.SetVertex(mesh_id, 0, vertex_id, KlayGE::float3(pos[0], pos[1], pos[2]),
							KlayGE::float3(norm[0], norm[1], norm[2]), 2, std::vector<KlayGE::float3>());
					}
					
					for (size_t n = 0; n < num_binds; ++ n)
					{
						if (vert_weights[n] > 0)
						{
							auto iter = joint_to_id_.find(joint_dag_paths[n].fullPathName().asChar());
							BOOST_ASSERT(iter != joint_to_id_.end());

							int binding_id = meshml_obj_.AllocJointBinding(mesh_id, 0, vertex_id);
							meshml_obj_.SetJointBinding(mesh_id, 0, vertex_id, binding_id, iter->second, vert_weights[n]);
						}
					}

					// Record the index attributes
					attr.id = id_count;
					tri_indices[i] = attr.id;
					index_attributes.insert(attr);

					++ id_count;
				}
			}

			if (3 == poly_vertex_count)
			{
				int tri_id = meshml_obj_.AllocTriangle(mesh_id, 0);
				meshml_obj_.SetTriangle(mesh_id, 0, tri_id, tri_indices[0], tri_indices[1], tri_indices[2]);
			}
			else if (4 == poly_vertex_count)
			{
				int tri0_id = meshml_obj_.AllocTriangle(mesh_id, 0);
				meshml_obj_.SetTriangle(mesh_id, 0, tri0_id, tri_indices[0], tri_indices[1], tri_indices[2]);

				int tri1_id = meshml_obj_.AllocTriangle(mesh_id, 0);
				meshml_obj_.SetTriangle(mesh_id, 0, tri1_id, tri_indices[0], tri_indices[2], tri_indices[3]);
			}
		}
	}
	else
	{
		std::cout << "The polygons may not be standard triangles or quads: vertex number = "
			<< poly_vertex_count << std::endl;
	}
}

void MayaMeshExporter::ExportJoint(MDagPath const * parent_path, MFnIkJoint& fn_joint, MDagPath& dag_path)
{
	std::string joint_name = fn_joint.partialPathName().asChar();
	if (joint_name.empty())
	{
		joint_name = "0";  // Each joint must have a non-empty name
	}

	int joint_id = meshml_obj_.AllocJoint();

	// Compute joint transformation
	MFloatMatrix bind_mat(dag_path.inclusiveMatrix().matrix);
	int parent_id = -1;
	if (parent_path)
	{
		auto iter = joint_to_id_.find(parent_path->fullPathName().asChar());
		BOOST_ASSERT(iter != joint_to_id_.end());

		parent_id = iter->second;
	}
	joint_to_id_.emplace(dag_path.fullPathName().asChar(), joint_id);
	joint_id_to_path_.emplace(joint_id, dag_path);

	meshml_obj_.SetJoint(joint_id, joint_name, parent_id,
		float4x4(bind_mat(0, 0), bind_mat(0, 1), bind_mat(0, 2), bind_mat(0, 3),
			bind_mat(1, 0), bind_mat(1, 1), bind_mat(1, 2), bind_mat(1, 3),
			bind_mat(2, 0), bind_mat(2, 1), bind_mat(2, 2), bind_mat(2, 3),
			bind_mat(3, 0), bind_mat(3, 1), bind_mat(3, 2), bind_mat(3, 3)));
	
	// Traverse child joints
	for (unsigned int i = 0; i < dag_path.childCount(); ++ i)
	{
		MObject child = dag_path.child(i);
		MDagPath child_path = dag_path;
		child_path.push(child);
		if (child_path.hasFn(MFn::kJoint))
		{
			MFnIkJoint fn_child_joint(child_path);
			auto iter = joint_to_id_.find(fn_child_joint.fullPathName().asChar());
			if (iter == joint_to_id_.end())
			{
				this->ExportJoint(&dag_path, fn_child_joint, child_path);
			}
		}
	}
}

void MayaMeshExporter::ExportKeyframe(int kfs_id, int frame_id, MDagPath& dag_path, MMatrix const & inv_parent)
{
	MFloatMatrix local_mat((dag_path.inclusiveMatrix() * inv_parent).matrix);

	int kf_id = meshml_obj_.AllocKeyframe(kfs_id);
	meshml_obj_.SetKeyframe(kfs_id, kf_id, frame_id,
		float4x4(local_mat(0, 0), local_mat(0, 1), local_mat(0, 2), local_mat(0, 3),
			local_mat(1, 0), local_mat(1, 1), local_mat(1, 2), local_mat(1, 3),
			local_mat(2, 0), local_mat(2, 1), local_mat(2, 2), local_mat(2, 3),
			local_mat(3, 0), local_mat(3, 1), local_mat(3, 2), local_mat(3, 3)));
}

int MayaMeshExporter::ExportMaterialAndTexture(MObject* shader, MObjectArray const & /*textures*/)
{
	int mtl_id = -1;
	MFnDependencyNode dn(*shader);
	MPlugArray connections;

	MObject surface_shader;
	bool has_surface_shader = false;
	if (shader->hasFn(MFn::kShadingEngine))
	{
		// Get only connections of the surface shader
		MPlug plug_surface_shader = dn.findPlug("surfaceShader", true);
		plug_surface_shader.connectedTo(connections, true, false);
		if (connections.length() > 0)
		{
			surface_shader = connections[0].node();
			has_surface_shader = true;

			if (surface_shader.hasFn(MFn::kLambert))
			{
				MFnLambertShader lambert(surface_shader);
				MColor dc = lambert.color();
				MColor ec = lambert.incandescence();
				MColor tr = lambert.transparency();
				float dcoeff = lambert.diffuseCoeff();
				MString name = lambert.name();

				float shininess = 32.0f;
				if (surface_shader.hasFn(MFn::kPhong))
				{
					MFnPhongShader phong(surface_shader);
					shininess = phong.cosPower();
				}
				else if (surface_shader.hasFn(MFn::kBlinn))
				{
					MFnBlinnShader blinn(surface_shader);
					shininess = blinn.specularRollOff();
				}

				float opacity = 1.0f - (tr.r + tr.g + tr.b) / 3.0f;
				float glossiness = log(shininess) / log(8192.0f);

				mtl_id = meshml_obj_.AllocMaterial();
				meshml_obj_.SetMaterial(mtl_id, name.asChar(), KlayGE::float4(dcoeff * dc.r, dcoeff * dc.g, dcoeff * dc.b, opacity),
					0.0f, glossiness, KlayGE::float3(ec.r, ec.g, ec.b), opacity < 1, 0, false, false);
			}
			else
			{
				MGlobal::displayError("Unknown material type.");
			}
		}
		else
		{
			MGlobal::displayError("Only one surface shader is accepted while constructing mesh material.");
		}
	}

	if (mtl_id < 0)
	{
		mtl_id = this->AddDefaultMaterial();
	}

	// Setup texture types and corresponding names in Maya
	// FIXME: how to handle other types: NormalMap, OpacityMap, etc.
	std::map<MeshMLObj::Material::TextureSlot, std::string> texture_type_map;
	texture_type_map.emplace(MeshMLObj::Material::TS_Albedo, "color");
	texture_type_map.emplace(MeshMLObj::Material::TS_Emissive, "incandescence");
	texture_type_map.emplace(MeshMLObj::Material::TS_Bump, "bumpValue");

	// Record all texture types that are binded to this material
	if (has_surface_shader)
	{
		MFnDependencyNode surface_dn(surface_shader);
		for (auto const & tex : texture_type_map)
		{
			MPlug plug_specified = surface_dn.findPlug(tex.second.c_str(), true);
			if (!plug_specified.isConnected())
			{
				continue;
			}

			connections.clear();
			plug_specified.connectedTo(connections, true, false);
			if (connections.length() > 0)
			{
				MObject tex_obj = connections[0].node();
				if (tex_obj.hasFn(MFn::kFileTexture))
				{
					MFnDependencyNode tex_dn(tex_obj);
					MPlug plug = tex_dn.findPlug("fileTextureName");

					MString texture_name;
					plug.getValue(texture_name);

					meshml_obj_.SetTextureSlot(mtl_id, tex.first, texture_name.asChar());
				}
				else
				{
					std::cout << "Unknown texture data type, not a valid file texture." << std::endl;
				}
			}
		}
	}

	return mtl_id;
}

int MayaMeshExporter::AddDefaultMaterial()
{
	int mtl_id = meshml_obj_.AllocMaterial();
	meshml_obj_.SetMaterial(mtl_id, "default", KlayGE::float4(0, 0, 0, 1), 0, log(32.0f) / log(8192.0f), KlayGE::float3(0, 0, 0),
		false, 0, false, false);
	return mtl_id;
}

// Maya/standalone interface

#ifndef STANDALONE_APPLICATION

class MayaToMeshML : public MPxFileTranslator
{
public:
	MayaToMeshML()
	{
	}
	virtual ~MayaToMeshML()
	{
	}

	virtual MString defaultExtension() const
	{
		return "meshml";
	}
	virtual MString filter() const
	{
		return "*.meshml";
	}

	virtual bool haveReadMethod() const
	{
		return false;
	}
	virtual bool haveWriteMethod() const
	{
		return true;
	}

	virtual MStatus reader(MFileObject const & /*file*/, MString const & /*optionsString*/, FileAccessMode /*mode*/)
	{
		return MS::kFailure;
	}

	virtual MStatus writer(MFileObject const & file, MString const & optionsString,  FileAccessMode mode)
	{
		MayaMeshExporter exporter;
		MString filename = file.fullName();
		if (optionsString.length() > 0)
		{
			// TODO: options
		}

		MStatus status;
		MItDag dag_iterator(MItDag::kDepthFirst, MFn::kInvalid, &status);
		if (!status)
		{
			std::cout << "Failed to initialize the plugin - " << status.errorString() << std::endl;
			return MStatus::kFailure;
		}

		if ((MPxFileTranslator::kExportAccessMode == mode) || (MPxFileTranslator::kSaveAccessMode == mode))
		{
			// Export all
			exporter.ExportMayaNodes(dag_iterator);
		}
		else if (MPxFileTranslator::kExportActiveAccessMode == mode)
		{
			// Export selected
			MSelectionList selection_list;
			MGlobal::getActiveSelectionList(selection_list);
			if (selection_list.length() > 0)
			{
				MItSelectionList sel_iterator(selection_list);
				for (; !sel_iterator.isDone(); sel_iterator.next())
				{
					MDagPath object_path;
					sel_iterator.getDagPath(object_path);
					dag_iterator.reset(object_path.node(), MItDag::kDepthFirst, MFn::kInvalid);
					exporter.ExportMayaNodes(dag_iterator);
				}
			}
		}

		exporter.WriteMeshFile(filename.asChar());
		return MStatus::kSuccess;
	}

	virtual MFileKind identifyFile(MFileObject const & fileName, char const * /*buffer*/, short /*size*/) const
	{
		std::string filename = fileName.name().asChar();
		if (filename.substr(filename.size() - 6, 6).compare("meshml"))
		{
			return kCouldBeMyFileType;
		}
		else
		{
			return kNotMyFileType;
		}
	}

	static void* creator()
	{
		return new MayaToMeshML();
	}
};

char const* obj_option_script = "MeshMLMayaExporterOptions";
char const* obj_default_options =" ";

MStatus initializePlugin(MObject obj)
{
	MFnPlugin plugin(obj, "KlayGE", "4.0", "Any");
	char pixmapName[] = "none";
	return plugin.registerFileTranslator("MeshMLMayaExporter", pixmapName,
		MayaToMeshML::creator, const_cast<char*>(obj_option_script), const_cast<char*>(obj_default_options));
}

MStatus uninitializePlugin(MObject obj)
{
	MFnPlugin plugin(obj);
	return plugin.deregisterFileTranslator("MeshMLMayaExporter");
}

#else

int main(int argc, char** argv)
{
	if (argc < 3)
	{
		std::cout << "exporter input_file output_file." << std::endl;
		return 1;
	}

	MStatus status = MLibrary::initialize(true, "KlayGE: Maya file exporter", true);
	if (!status)
	{
		std::cout << "MLibrary::initialize() failed - " << status.errorString() << std::endl;
		return 1;
	}

	MayaMeshExporter exporter;
	exporter.ExportMayaFile(argv[1]);
	exporter.WriteMeshFile(argv[2]);

	MLibrary::cleanup();
	return 0;
}

#endif
