#include "MeshExtractor.hpp"

#include <maya/MLibrary.h>
#include <maya/MGlobal.h>
#include <maya/MArgList.h>
#include <maya/MFloatArray.h>
#include <maya/MPointArray.h>
#include <maya/MFloatVectorArray.h>
#include <maya/MQuaternion.h>
#include <maya/MColor.h>
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

#include <fstream>
#include <iostream>
#include <set>

using namespace KlayGE;
/////////////////////////////////////////////////////////////////////////////////////////////////////
MeshExtractor::Quat QuatTransToUDQ(MeshExtractor::Quat const & q, MeshExtractor::Point3 const & t)
{
	return MeshExtractor::Quat(4,
		+0.5f * (+t[0] * q[3] + t[1] * q[2] - t[2] * q[1]),
		+0.5f * (-t[0] * q[2] + t[1] * q[3] + t[2] * q[0]),
		+0.5f * (+t[0] * q[1] - t[1] * q[0] + t[2] * q[3]),
		-0.5f * (+t[0] * q[0] + t[1] * q[1] + t[2] * q[2]));
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
class MayaMeshExporter
{
public:
	struct IndexAttributes
	{
		int id;
		int vertexIndex;
		int normalIndex;
		int uvIndex;
		int tangentIndex;

		IndexAttributes()
			: id(0), vertexIndex(0), normalIndex(0), uvIndex(0), tangentIndex(0) {}

		bool operator<(IndexAttributes const & rhs) const
		{
			if (vertexIndex < rhs.vertexIndex)
			{
				return true;
			}
			else if (vertexIndex > rhs.vertexIndex)
			{
				return false;
			}

			if (normalIndex < rhs.normalIndex)
			{
				return true;
			}
			else if (normalIndex > rhs.normalIndex)
			{
				return false;
			}

			if (uvIndex < rhs.uvIndex)
			{
				return true;
			}
			else if (uvIndex > rhs.uvIndex)
			{
				return false;
			}

			return tangentIndex < rhs.tangentIndex;
		}
	};

	MayaMeshExporter();
	void WriteMeshFile(std::string const & savefile);

	void ExportMayaFile(std::string const & openfile);
	void ExportMayaNodes(MItDag& dagIterator);

private:
	void ExportNurbsSurface(MString const & objName, MFnNurbsSurface& fnSurface, MDagPath& dagPath);
	void ExportMesh(MString const & objName, MFnMesh& fnMesh, MFnSkinCluster* fnSkinClusterPtr, MDagPath& dagPath);
	void ExportJoint(MString const * parentName, MFnIkJoint& fnJoint, MDagPath& dagPath);
	void ExportKeyframe(MeshExtractor::KeyframeStruct* key, MTransformationMatrix const & initMatrix,
		MDagPath& dagPath, MMatrix const & inv_parent);
	int ExportMaterialAndTexture(MObject* shader, MObjectArray const & textures);
	MeshExtractor::MaterialStruct* CreateDefaultMaterial();
	MFnSkinCluster* ExportSkinCluster(MFnMesh& fnMesh, MDagPath& dagPath);

	MeshExtractor exporter_;
	std::vector<MeshExtractor::MaterialStruct*> export_materials_;
	std::vector<MeshExtractor::MeshStruct*> export_meshes_;

	typedef std::map<std::string, MeshExtractor::JointStruct*> JointMap;
	typedef std::map<std::string, MeshExtractor::KeyframeStruct*> KeyframeMap;
	typedef std::pair<MTransformationMatrix, MDagPath> MatrixAndParentPath;

	JointMap export_joints_;
	KeyframeMap export_keyframes_;
	std::map<std::string, MatrixAndParentPath> joints_info_;

	int start_frame_;
	int end_frame_;
	int frame_rate_;
};

MayaMeshExporter::MayaMeshExporter()
	: start_frame_(0), end_frame_(0), frame_rate_(30)
{
}

void MayaMeshExporter::WriteMeshFile(std::string const & savefile)
{
	std::ofstream out_file(savefile.c_str());
	exporter_.Output(&out_file);

	// Always add a default material
	MeshExtractor::MaterialStruct* defMaterial = CreateDefaultMaterial();
	exporter_.AddMaterial(*defMaterial);

	// Add materials
	for (size_t i=0; i<export_materials_.size(); ++i)
	{
		exporter_.AddMaterial(*(export_materials_[i]));
	}

	// Add meshes
	for (size_t i=0; i<export_meshes_.size(); ++i)
	{
		exporter_.AddMesh(*(export_meshes_[i]));
	}

	// Add joints and keyframes
	for (JointMap::iterator itr=export_joints_.begin(); itr!=export_joints_.end(); ++itr)
	{
		exporter_.AddJoint(itr->first, *(itr->second));
	}

	exporter_.StartFrame(start_frame_);
	exporter_.EndFrame(end_frame_);
	exporter_.FrameRate(frame_rate_);
	for (KeyframeMap::iterator itr=export_keyframes_.begin(); itr!=export_keyframes_.end(); ++itr)
	{
		exporter_.AddKeyframe(*(itr->second));
	}

	// Write to file and cleanup
	exporter_.WriteMeshML(5);

	delete defMaterial;
	for (size_t i=0; i<export_materials_.size(); ++i)
	{
		delete export_materials_[i];
	}

	for (size_t i=0; i<export_meshes_.size(); ++i)
	{
		delete export_meshes_[i];
	}

	for (JointMap::iterator itr=export_joints_.begin(); itr!=export_joints_.end(); ++itr)
	{
		delete itr->second;
	}

	for (KeyframeMap::iterator itr=export_keyframes_.begin(); itr!=export_keyframes_.end(); ++itr)
	{
		delete itr->second;
	}
}

void MayaMeshExporter::ExportMayaFile(std::string const & openfile)
{
	MStatus status = MFileIO::open(openfile.c_str(), NULL, true);
	if (!status)
	{
		std::cout << "MFileIO::open(" << openfile << ") failed - " << status.errorString() << std::endl;
		return;
	}

	// Add meshes
	MItDag dagIterator(MItDag::kDepthFirst, MFn::kInvalid, &status);
	ExportMayaNodes(dagIterator);
}

void MayaMeshExporter::ExportMayaNodes(MItDag& dagIterator)
{
	unsigned int depth = 0;
	MDagPath dagPath;
	for (; !dagIterator.isDone(); dagIterator.next())
	{
		MStatus status = dagIterator.getPath(dagPath);
		if (!status)
		{
			std::cout << "Fail to get DAG path." << std::endl;
			continue;
		}

		MString objName = dagIterator.partialPathName();
		switch (dagPath.apiType())
		{
		case MFn::kTransform:
			{
				/*MFnTransform fnTransform(dagPath, &status);
				if (status == MS::kSuccess)
				{
				MMatrix matrix = fnTransform.transformation().asMatrix();
				// TODO: how to handle transformations?
				}
				else
				std::cout << "Fail to initialize transform node." << std::endl;*/
			}
			break;

		case MFn::kMesh:
			{
				MFnMesh fnMesh(dagPath, &status);
				if (status == MS::kSuccess)
				{
					if (!fnMesh.isIntermediateObject())
					{
						MFnSkinCluster* fnSkinClusterPtr = ExportSkinCluster(fnMesh, dagPath);
						ExportMesh(objName, fnMesh, fnSkinClusterPtr, dagPath);
						if (fnSkinClusterPtr) delete fnSkinClusterPtr;
					}
					else
					{
						std::cout << "Intermediate objects " << fnMesh.name().asChar()
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
				MFnNurbsSurface fnSurface(dagPath, &status);
				if (status == MS::kSuccess)
				{
					ExportNurbsSurface(objName, fnSurface, dagPath);
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
			std::cout << "MDagPath::apiType()=" << dagPath.apiType() << " ["
				<< dagPath.fullPathName() << "] not supported." << std::endl;
			break;
		}
		depth = dagIterator.depth();
	}

	start_frame_ = static_cast<int>(MAnimControl::minTime().as(MTime::kNTSCField));
	end_frame_   = static_cast<int>(MAnimControl::maxTime().as(MTime::kNTSCField));
	int numFrames = end_frame_ - start_frame_ + 1;
	if (numFrames > 0)
	{
		for (int i = 0; i < numFrames; ++ i)
		{
			MAnimControl::setCurrentTime(MTime(i + start_frame_, MTime::kNTSCField));
			for (JointMap::iterator itr = export_joints_.begin(); itr != export_joints_.end(); ++ itr)
			{
				std::string const & jointNameString = itr->first;
				MeshExtractor::KeyframeStruct* key = export_keyframes_[jointNameString];
				if (!key)
				{
					key = new MeshExtractor::KeyframeStruct;
					key->joint = jointNameString;
					export_keyframes_[jointNameString] = key;
				}

				MatrixAndParentPath& mapp = joints_info_[jointNameString];
				MMatrix inv_parent;
				if (itr->second->parent_name.empty())
				{
					inv_parent = MMatrix::identity;
				}
				else
				{
					inv_parent = joints_info_[itr->second->parent_name].second.inclusiveMatrixInverse();
				}
				ExportKeyframe(key, mapp.first, mapp.second, inv_parent);
			}
		}
	}
}

void MayaMeshExporter::ExportNurbsSurface(MString const & objName, MFnNurbsSurface& fnSurface, MDagPath& dagPath)
{
	MFnDagNode surface_dn(dagPath);
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

	params.setUNumber( numberU );
	params.setVNumber( numberV );

	// Try convert the NURBS surface to polygon mesh
	MStatus status = MS::kSuccess;
	MDagPath meshPath = dagPath;
	MObject meshParent = meshPath.node();
	MObject convertedMesh = fnSurface.tesselate(params, meshParent, &status);
	if (status == MS::kSuccess)
	{
		meshPath.push(convertedMesh);

		// Export the new generated mesh
		MFnMesh fnConvertedMesh(convertedMesh, &status);
		ExportMesh(objName, fnConvertedMesh, NULL, dagPath);

		// Remove the mesh to keep the scene clean
		MFnDagNode parentNode(meshParent, &status);
		parentNode.removeChild(convertedMesh);
	}
	else
	{
		status.perror("MFnNurbsSurface::tesselate");
	}
}

void MayaMeshExporter::ExportMesh(MString const & objName, MFnMesh& fnMesh,
	MFnSkinCluster* fnSkinClusterPtr, MDagPath& dagPath)
{
	MStatus status = MS::kSuccess;
	MObject component = MObject::kNullObj;
	MSpace::Space space = MSpace::kWorld;

	MItMeshPolygon polyIterator(dagPath, component, &status);
	if (status != MS::kSuccess)
	{
		std::cout << "Fail to initialize polygons." << std::endl;
		return;
	}

	int polyVertexCount = polyIterator.polygonVertexCount();
	if (polyVertexCount > 2)
	{
		// Initialize the exported mesh
		MeshExtractor::MeshStruct* export_mesh = new MeshExtractor::MeshStruct;
		export_mesh->material_id = 0;
		export_mesh->name = objName.asChar();

		// Start to save mesh
		int numVertices = fnMesh.numVertices();
		//int numNormals = fnMesh.numNormals();
		//int numPolygons = fnMesh.numPolygons();
		//int numTexCoords = fnMesh.numUVs();
		bool hasTexCoords = false;
		MString* uvSetName = NULL;

		// Get fixed-pipeline shader (material)
		MObjectArray shaderObjects;
		MIntArray shaderIndices;
		fnMesh.getConnectedShaders(0, shaderObjects, shaderIndices);

		// Get UV set names and associated textures
		// FIXME: At present only the first UV set is recorded and used
		MObjectArray textureObjects;
		MStringArray uvSetNames;
		fnMesh.getUVSetNames(uvSetNames);
		if (uvSetNames.length() > 0)
		{
			uvSetName = &uvSetNames[0];
			fnMesh.getAssociatedUVSetTextures(*uvSetName, textureObjects);
		}

		// Export shader (material) and texture object
		// FIXME: At present only the first shader is recorded and used
		if (shaderObjects.length() > 0)
		{
			export_mesh->material_id = ExportMaterialAndTexture(&shaderObjects[0], textureObjects);
		}
		else
		{
			export_mesh->material_id = ExportMaterialAndTexture(NULL, textureObjects);
		}

		// Get points
		MPointArray points;
		fnMesh.getPoints(points, space);

		// Get normals
		MFloatVectorArray normals;
		fnMesh.getNormals(normals, space);

		// Get UV coords of default UV set
		MFloatArray uArray, vArray;
		fnMesh.getUVs(uArray, vArray, uvSetName);
		if (fnMesh.numUVs() > 0)
		{
			hasTexCoords = true;
		}

		// Get tangents of default UV set
		MFloatVectorArray tangents;
		fnMesh.getTangents(tangents, space, uvSetName);

		// Get binormals of default UV set
		MFloatVectorArray binormals;
		fnMesh.getBinormals(binormals, space, uvSetName);

		// Get associated joints and weights
		std::vector<MFloatArray> weights(numVertices);
		std::vector<MStringArray> jointNames(numVertices);
		if (fnSkinClusterPtr)
		{
			unsigned int numWeights;
			MItGeometry geomIterator(dagPath);
			for (int i = 0; !geomIterator.isDone(); geomIterator.next(), ++ i)
			{
				MObject component = geomIterator.component();
				MFloatArray vertexWeights;
				status = fnSkinClusterPtr->getWeights(dagPath, component, vertexWeights, numWeights);
				if (status != MS::kSuccess)
				{
					std::cout << "Fail to retrieve vertex weights." << std::endl;
					continue;
				}

				weights[i] = vertexWeights;
				jointNames[i].setLength(vertexWeights.length());

				MDagPathArray influenceObjs;
				fnSkinClusterPtr->influenceObjects(influenceObjs, &status);
				if (status != MS::kSuccess)
				{
					std::cout << "Fail to retrieve influence objects for the skin cluster." << std::endl;
					continue;
				}

				for (unsigned int j=0; j<influenceObjs.length(); ++j)
				{
					MString partialPathName = influenceObjs[j].partialPathName();
					JointMap::iterator itr = export_joints_.find(partialPathName.asChar());
					if (itr == export_joints_.end()) continue;

					// Record joint name
					jointNames[i][j] = partialPathName;
				}
			}
		}

		// Traverse polygons
		MItMeshPolygon polygonIterator(dagPath, component, &status);
		if (status != MS::kSuccess)
		{
			std::cout << "Fail to traverse polygons." << std::endl;
			return;
		}

		int idCount = 0, triIndices[4];
		std::set<IndexAttributes> indexAttributes;
		for (; !polygonIterator.isDone(); polygonIterator.next())
		{
			for (int i=0; i<polyVertexCount; ++i)
			{
				IndexAttributes attr;
				attr.vertexIndex = polygonIterator.vertexIndex(i);
				attr.normalIndex = polygonIterator.normalIndex(i);
				if (hasTexCoords)
				{
					polygonIterator.getUVIndex(i, attr.uvIndex);
					attr.tangentIndex = polygonIterator.tangentIndex(i);
				}

				std::set<IndexAttributes>::iterator itr = indexAttributes.find(attr);
				if (itr != indexAttributes.end())
				{
					// Reuse existing vertex
					triIndices[i] = itr->id;
				}
				else
				{
					// Create a new vertex and add it to the mesh
					MPoint pos = points[attr.vertexIndex];
					MFloatVector norm = normals[attr.normalIndex];
					MFloatArray vWeights = weights[attr.vertexIndex];
					MStringArray vJoints = jointNames[attr.vertexIndex];
					unsigned int numBinds = std::min<unsigned int>(vWeights.length(), vJoints.length());

					MeshExtractor::VertexStruct vertex;
					vertex.position = MeshExtractor::Point3(3, pos[0], pos[1], pos[2]);
					vertex.normal = MeshExtractor::Point3(3, norm[0], norm[1], norm[2]);
					for (unsigned int n=0; n<numBinds; ++n)
					{
						vertex.binds.push_back(MeshExtractor::JointBinding(vJoints[n].asChar(), vWeights[n]));
					}

					if (hasTexCoords)
					{
						MFloatVector teng = tangents[attr.tangentIndex];
						MFloatVector bi = binormals[attr.tangentIndex];
						float u = uArray[attr.uvIndex], v = vArray[attr.uvIndex];

						vertex.tangent = MeshExtractor::Point3(3, teng[0], teng[1], teng[2]);
						vertex.binormal = MeshExtractor::Point3(3, bi[0], bi[1], bi[2]);
						vertex.texcoords.push_back(MeshExtractor::TexCoord(2, u, v));
						vertex.texcoord_components = 2;
					}
					export_mesh->vertices.push_back(vertex);

					// Record the index attributes
					attr.id = idCount++;
					triIndices[i] = attr.id;
					indexAttributes.insert(attr);
				}
			}

			if (polyVertexCount == 3)
			{
				MeshExtractor::TriangleStruct triangle;
				for (int i=0; i<3; ++i)
					triangle.vertex_index[i] = triIndices[i];
				export_mesh->triangles.push_back(triangle);
			}
			else if (polyVertexCount == 4)
			{
				MeshExtractor::TriangleStruct triangle1;
				for (int i=0; i<3; ++i)
					triangle1.vertex_index[i] = triIndices[i];
				export_mesh->triangles.push_back(triangle1);

				MeshExtractor::TriangleStruct triangle2;
				triangle2.vertex_index[0] = triIndices[0];
				triangle2.vertex_index[1] = triIndices[2];
				triangle2.vertex_index[2] = triIndices[3];
				export_mesh->triangles.push_back(triangle2);
			}
		}
		export_meshes_.push_back(export_mesh);
	}
	else
	{
		std::cout << "The polygons may not be standard triangles or quads: vertex number = "
			<< polyVertexCount << std::endl;
	}
}

void MayaMeshExporter::ExportJoint(MString const * parentName, MFnIkJoint& fnJoint, MDagPath& dagPath)
{
	MString jointName = fnJoint.partialPathName();
	std::string jointNameString = jointName.asChar();
	if (jointNameString.empty())
		jointNameString = "0";  // Each joint must have a non-empty name

	MeshExtractor::JointStruct* export_joint = new MeshExtractor::JointStruct;
	export_joints_[jointNameString] = export_joint;

	// Compute joint transformation
	MMatrix bindMatrix = dagPath.inclusiveMatrix();
	MTransformationMatrix localMatrix = bindMatrix;
	if (parentName)
	{
		MDagPath parentDagPath = joints_info_[parentName->asChar()].second;
		export_joint->parent_name = parentName->asChar();
	}
	joints_info_[jointNameString] = MatrixAndParentPath(localMatrix, dagPath);

	double qx, qy, qz, qw;
	MVector pos = localMatrix.translation(MSpace::kPostTransform);
	localMatrix.getRotationQuaternion(qx, qy, qz, qw);
	MeshExtractor::Point3 bind_pos = MeshExtractor::Point3(3, pos.x, pos.y, pos.z);
	MeshExtractor::Quat bind_quat = MeshExtractor::Quat(4, qx, qy, qz, qw);
	MeshExtractor::Quat bind_dual = QuatTransToUDQ(bind_quat, bind_pos);
	export_joint->bind_real = bind_quat;
	export_joint->bind_dual = bind_dual;
	
	// Traverse child joints
	for (unsigned int i=0; i<dagPath.childCount(); ++i)
	{
		MObject child = dagPath.child(i);
		MDagPath childPath = dagPath; childPath.push(child);
		if (!childPath.hasFn(MFn::kJoint)) continue;

		MFnIkJoint fnChildJoint(childPath);
		JointMap::iterator itr = export_joints_.find(fnChildJoint.partialPathName().asChar());
		if (itr == export_joints_.end())
		{
			ExportJoint(&jointName, fnChildJoint, childPath);
		}
	}
}

void MayaMeshExporter::ExportKeyframe(MeshExtractor::KeyframeStruct* key, MTransformationMatrix const & /*initMatrix*/,
	MDagPath& dagPath, MMatrix const & inv_parent)
{
	MMatrix bindMatrix = dagPath.inclusiveMatrix();
	MTransformationMatrix localMatrix = bindMatrix * inv_parent;

	double qx, qy, qz, qw;
#if 0
	// Relative to the joint initial matrix
	MTransformationMatrix relMatrix = localMatrix.asMatrix() * initMatrix.asMatrix().inverse();
	MVector pos = localMatrix.translation(MSpace::kPostTransform) - initMatrix.translation(MSpace::kPostTransform);
	relMatrix.getRotationQuaternion(qx, qy, qz, qw);
#else
	// Absolute transformation
	MVector pos = localMatrix.translation(MSpace::kPostTransform);
	localMatrix.getRotationQuaternion(qx, qy, qz, qw);
#endif
	MeshExtractor::Point3 bind_pos = MeshExtractor::Point3(3, pos.x, pos.y, pos.z);
	MeshExtractor::Quat bind_quat = MeshExtractor::Quat(4, qx, qy, qz, qw);
	MeshExtractor::Quat bind_dual = QuatTransToUDQ(bind_quat, bind_pos);
	key->bind_reals.push_back(bind_quat);
	key->bind_duals.push_back(bind_dual);
}

int MayaMeshExporter::ExportMaterialAndTexture(MObject* shader, MObjectArray const & /*textures*/)
{
	MeshExtractor::MaterialStruct* material = NULL;
	MFnDependencyNode dn(*shader);
	MPlugArray connections;

	MObject surface_shader;
	bool hasSurfaceShader = false;
	if (shader->hasFn(MFn::kShadingEngine))
	{
		// Get only connections of the surface shader
		MPlug plug_surface_shader = dn.findPlug("surfaceShader", true);
		plug_surface_shader.connectedTo(connections, true, false);
		if (connections.length() > 0)
		{
			surface_shader = connections[0].node();
			hasSurfaceShader = true;

			if (surface_shader.hasFn(MFn::kLambert))
			{
				MFnLambertShader lambert(surface_shader);
				MColor ac = lambert.ambientColor();
				MColor dc = lambert.color();
				MColor ec = lambert.incandescence();
				MColor tr = lambert.transparency();
				float dcoeff = lambert.diffuseCoeff();

				MColor spec(1.0f, 1.0f, 1.0f, 1.0f);
				float shininess = 32.0f;
				if (surface_shader.hasFn(MFn::kPhong))
				{
					MFnPhongShader phong(surface_shader);
					spec = phong.specularColor();
					shininess = phong.cosPower();
				}
				else if (surface_shader.hasFn(MFn::kBlinn))
				{
					MFnBlinnShader blinn(surface_shader);
					spec = blinn.specularColor();
					shininess = blinn.specularRollOff();
				}

				material = new MeshExtractor::MaterialStruct;
				material->ambient = MeshExtractor::Color(4, ac.r, ac.g, ac.b, ac.a);
				material->diffuse = MeshExtractor::Color(4, dcoeff*dc.r, dcoeff*dc.g, dcoeff*dc.b, dc.a);
				material->specular = MeshExtractor::Color(4, spec.r, spec.g, spec.b, spec.a);
				material->emit = MeshExtractor::Color(4, ec.r, ec.g, ec.b, ec.a);
				material->opacity = 1.0f - ((tr.r + tr.g + tr.b) / 3.0f);
				material->specular_level = 0.36f;  // FIXME: what does this mean in Maya API?
				material->shininess = shininess;
			}
			else
			{
				std::cout << "Unknown material type." << std::endl;
			}
		}
		else
		{
			std::cout << "Only one surface shader is accepted while constructing mesh material." << std::endl;
		}
	}

	if (!material)
	{
		material = CreateDefaultMaterial();
	}

	// Setup texture types and corresponding names in Maya
	// FIXME: how to handle other types: NormalMap, OpacityMap, etc.
	std::map<std::string, std::string> textureTypeMap;
	textureTypeMap["DiffuseMap"] = "color";
	textureTypeMap["AmbientMap"] = "ambientColor";
	textureTypeMap["SpecularMap"] = "specularColor";
	textureTypeMap["EmitMap"] = "incandescence";
	textureTypeMap["TransparencyMap"] = "transparency";
	textureTypeMap["BumpMap"] = "bumpValue";

	// Record all texture types that are binded to this material
	if (hasSurfaceShader)
	{
		MFnDependencyNode surface_dn(surface_shader);
		for (std::map<std::string, std::string>::const_iterator itr = textureTypeMap.begin();
			itr != textureTypeMap.end(); ++ itr)
		{
			MPlug plug_specified = surface_dn.findPlug(itr->second.c_str(), true);
			if (!plug_specified.isConnected())
			{
				continue;
			}

			connections.clear();
			plug_specified.connectedTo(connections, true, false);
			if (connections.length() == 0)
			{
				continue;
			}

			MObject texObj = connections[0].node();
			if (texObj.hasFn(MFn::kFileTexture))
			{
				MFnDependencyNode tex_dn(texObj);
				MPlug plug = tex_dn.findPlug("fileTextureName");

				MString textureName;
				plug.getValue(textureName);
				material->texture_slots.push_back(
					MeshExtractor::TextureSlot(itr->first.c_str(), textureName.asChar()));
			}
			else
			{
				std::cout << "Unknown texture data type, not a valid file texture." << std::endl;
			}
		}
	}

	export_materials_.push_back(material);
	return static_cast<int>(export_materials_.size());
}

MeshExtractor::MaterialStruct* MayaMeshExporter::CreateDefaultMaterial()
{
	MeshExtractor::MaterialStruct* defMaterial = new MeshExtractor::MaterialStruct;
	defMaterial->ambient = MeshExtractor::Color(4, 0.0f, 0.0f, 0.0f, 1.0f);
	defMaterial->diffuse = MeshExtractor::Color(4, 0.0f, 0.0f, 0.0f, 1.0f);
	defMaterial->specular = MeshExtractor::Color(4, 0.9f, 0.9f, 0.9f, 1.0f);
	defMaterial->emit = MeshExtractor::Color(4, 0.0f, 0.0f, 0.0f, 1.0f);
	defMaterial->opacity = 1.0f;
	defMaterial->specular_level = 0.36f;
	defMaterial->shininess = 32.0f;
	return defMaterial;
}

MFnSkinCluster* MayaMeshExporter::ExportSkinCluster(MFnMesh& fnMesh, MDagPath& /*dagPath*/)
{
	MStatus status = MS::kSuccess;
	MFnSkinCluster* fnSkinClusterPtr = NULL;
	MItDependencyNodes dn(MFn::kSkinClusterFilter);
	for (; !dn.isDone() && !fnSkinClusterPtr; dn.next())
	{
		MObject object = dn.item();
		fnSkinClusterPtr = new MFnSkinCluster(object, &status);

		unsigned int numGeometries = fnSkinClusterPtr->numOutputConnections();
		for (unsigned int i=0; i<numGeometries && fnSkinClusterPtr; ++i) 
		{
			unsigned int index = fnSkinClusterPtr->indexForOutputConnection(i);
			MObject outputObject = fnSkinClusterPtr->outputShapeAtIndex(index);

			// Check if skin cluster is invalid
			if (outputObject != fnMesh.object())
			{
				delete fnSkinClusterPtr;
				fnSkinClusterPtr = NULL;
			}
		}
	}

	if (fnSkinClusterPtr)
	{
		MDagPathArray influencePaths;
		int numInfluenceObjs = fnSkinClusterPtr->influenceObjects(influencePaths, &status);

		MDagPath jointPath,rootPath;
		for (int i=0; i<numInfluenceObjs; ++i)
		{
			jointPath = influencePaths[i];
			if (!jointPath.hasFn(MFn::kJoint))
			{
				continue;
			}

			// Try to retrieve the root path
			rootPath = jointPath;
			while (jointPath.length() > 0)
			{
				jointPath.pop();
				if (jointPath.hasFn(MFn::kJoint) && (jointPath.length() > 0))
				{
					rootPath = jointPath;
				}
			}

			if (rootPath.hasFn(MFn::kJoint))
			{
				MFnIkJoint fnJoint(rootPath);

				// Don't work on existing joints
				JointMap::iterator itr = export_joints_.find(fnJoint.partialPathName().asChar());
				if (itr == export_joints_.end())
					ExportJoint(NULL, fnJoint, rootPath);
			}
		}
	}
	return fnSkinClusterPtr;
}

// Maya/standalone interface

#ifndef STANDALONE_APPLICATION

class MayaToMeshML : public MPxFileTranslator
{
public:
	MayaToMeshML() {}
	virtual ~MayaToMeshML() {}

	virtual MString defaultExtension() const { return "meshml"; }
	virtual MString filter() const { return "*.meshml"; }

	virtual bool haveReadMethod() const { return false; }
	virtual bool haveWriteMethod() const { return true; }

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
		MItDag dagIterator(MItDag::kDepthFirst, MFn::kInvalid, &status);
		if (!status)
		{
			std::cout << "Failed to initialize the plugin - " << status.errorString() << std::endl;
			return MStatus::kFailure;
		}

		if ((MPxFileTranslator::kExportAccessMode == mode) || (MPxFileTranslator::kSaveAccessMode == mode))
		{
			// Export all
			exporter.ExportMayaNodes(dagIterator);
		}
		else if (MPxFileTranslator::kExportActiveAccessMode == mode)
		{
			// Export selected
			MSelectionList selectionList;
			MGlobal::getActiveSelectionList(selectionList);
			if (selectionList.length() > 0)
			{
				MItSelectionList selIterator(selectionList);
				for (; !selIterator.isDone(); selIterator.next())
				{
					MDagPath objectPath;
					selIterator.getDagPath(objectPath);
					dagIterator.reset(objectPath.node(), MItDag::kDepthFirst, MFn::kInvalid);
					exporter.ExportMayaNodes(dagIterator);
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

char const* objOptionScript = "MeshMLMayaExporterOptions";
char const* objDefaultOptions =" ";

MStatus initializePlugin(MObject obj)
{
	MFnPlugin plugin(obj, "KlayGE", "3.0", "Any");
	return plugin.registerFileTranslator("MeshMLMayaExporter", "none",
		MayaToMeshML::creator, const_cast<char*>(objOptionScript), const_cast<char*>(objDefaultOptions));
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

	MStatus status = MLibrary::initialize(true, "KlayGE: Maya file reader", true);
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
