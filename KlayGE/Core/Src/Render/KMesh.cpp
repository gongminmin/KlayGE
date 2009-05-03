// KMesh.cpp
// KlayGE KMesh类 实现文件
// Ver 3.9.0
// 版权所有(C) 龚敏敏, 2005-2009
// Homepage: http://klayge.sourceforge.net
//
// 3.9.0
// 直接读取MeshML (2009.5.3)
//
// 3.4.0
// 支持蒙皮模型的载入和保存 (2006.8.23)
//
// 2.7.1
// LoadKMesh可以使用自定义类 (2005.7.13)
//
// 2.7.0
// 初次建立 (2005.6.17)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/Texture.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/App3D.hpp>
#include <KlayGE/Camera.hpp>
#include <KlayGE/XMLDom.hpp>

#include <fstream>
#include <boost/assert.hpp>
#include <boost/typeof/typeof.hpp>
#include <boost/foreach.hpp>

#include <KlayGE/KMesh.hpp>

namespace
{
	std::string ReadShortString(std::istream& is)
	{
		KlayGE::uint8_t len;
		is.read(reinterpret_cast<char*>(&len), sizeof(len));
		std::vector<char> str(len, 0);
		is.read(&str[0], static_cast<std::streamsize>(str.size()));

		return std::string(str.begin(), str.end());
	}

	void WriteShortString(std::ostream& os, std::string const & str)
	{
		KlayGE::uint8_t const len = static_cast<KlayGE::uint8_t const>(str.size());
		os.write(reinterpret_cast<char const *>(&len), sizeof(len));
		os.write(&str[0], static_cast<std::streamsize>(str.size()));
	}
}

namespace KlayGE
{
	KMesh::KMesh(RenderModelPtr model, std::wstring const & name)
						: StaticMesh(model, name),
							model_matrix_(float4x4::Identity())
	{
		// 载入fx
		RenderEffectPtr effect;
		if (!ResLoader::Instance().Locate("KMesh.fxml").empty())
		{
			effect = Context::Instance().RenderFactoryInstance().LoadEffect("KMesh.fxml");
		}
		else
		{
			effect = RenderEffect::NullObject();
		}

		technique_ = effect->TechniqueByName("KMeshNoTexTec");

		texSampler_ep_ = technique_->Effect().ParameterByName("texSampler");
		modelviewproj_ep_ = technique_->Effect().ParameterByName("modelviewproj");
		modelIT_ep_ = technique_->Effect().ParameterByName("modelIT");
	}

	KMesh::~KMesh()
	{
	}

	void KMesh::BuildMeshInfo()
	{
		RenderModel::Material const & mtl = model_.lock()->GetMaterial(mtl_id_);

		TexturePtr tex;
		if (!mtl.texture_slots.empty())
		{
			tex = LoadTexture(mtl.texture_slots[0].second, EAH_GPU_Read)();
		}

		if (tex)
		{
			technique_ = technique_->Effect().TechniqueByName("KMeshTec");
			*texSampler_ep_ = tex;
		}
		else
		{
			technique_ = technique_->Effect().TechniqueByName("KMeshNoTexTec");
		}
	}

	void KMesh::OnRenderBegin()
	{
		App3DFramework const & app = Context::Instance().AppInstance();
		Camera const & camera = app.ActiveCamera();

		*modelviewproj_ep_ = model_matrix_ * camera.ViewMatrix() * camera.ProjMatrix();
	}

	void KMesh::SetModelMatrix(float4x4 const & model)
	{
		model_matrix_ = model;
		*modelIT_ep_ = MathLib::transpose(MathLib::inverse(model_matrix_));
	}


	RenderModelPtr LoadModel(std::string const & meshml_name, uint32_t access_hint,
		boost::function<RenderModelPtr (std::wstring const &)> CreateModelFactoryFunc,
		boost::function<StaticMeshPtr (RenderModelPtr, std::wstring const &)> CreateMeshFactoryFunc)
	{
		BOOST_ASSERT(CreateModelFactoryFunc);
		BOOST_ASSERT(CreateMeshFactoryFunc);

		std::vector<StaticMeshPtr> meshes;

		ResIdentifierPtr file = ResLoader::Instance().Load(meshml_name);
		XMLDocument doc;
		XMLNodePtr root = doc.Parse(file);

		XMLNodePtr bones_chunk = root->FirstNode("bones_chunk");
		std::wstring model_name;
		if (bones_chunk)
		{
			XMLNodePtr bones = bones_chunk->FirstNode("bone");
			RenderModelPtr ret;
			if (bones)
			{
				model_name = L"KSkinnedMesh";
			}
			else
			{
				model_name = L"KMesh";
			}
		}
		else
		{
			model_name = L"KMesh";
		}
		RenderModelPtr ret = CreateModelFactoryFunc(model_name);

		XMLNodePtr materials_chunk = root->FirstNode("materials_chunk");
		if (materials_chunk)
		{
			uint8_t num_mtls = 0;
			for (XMLNodePtr mtl_node = materials_chunk->FirstNode("material"); mtl_node; mtl_node = mtl_node->NextSibling("material"))
			{
				++ num_mtls;
			}

			ret->NumMaterials(num_mtls);
			uint32_t mtl_index = 0;
			for (XMLNodePtr mtl_node = materials_chunk->FirstNode("material"); mtl_node; mtl_node = mtl_node->NextSibling("material"), ++ mtl_index)
			{
				RenderModel::Material& mtl = ret->GetMaterial(mtl_index);
				mtl.ambient.x() = mtl_node->Attrib("ambient_r")->ValueFloat();
				mtl.ambient.y() = mtl_node->Attrib("ambient_g")->ValueFloat();
				mtl.ambient.z() = mtl_node->Attrib("ambient_b")->ValueFloat();
				mtl.diffuse.x() = mtl_node->Attrib("diffuse_r")->ValueFloat();
				mtl.diffuse.y() = mtl_node->Attrib("diffuse_g")->ValueFloat();
				mtl.diffuse.z() = mtl_node->Attrib("diffuse_b")->ValueFloat();
				mtl.specular.x() = mtl_node->Attrib("specular_r")->ValueFloat();
				mtl.specular.y() = mtl_node->Attrib("specular_g")->ValueFloat();
				mtl.specular.z() = mtl_node->Attrib("specular_b")->ValueFloat();
				mtl.emit.x() = mtl_node->Attrib("emit_r")->ValueFloat();
				mtl.emit.y() = mtl_node->Attrib("emit_g")->ValueFloat();
				mtl.emit.z() = mtl_node->Attrib("emit_b")->ValueFloat();
				mtl.opacity = mtl_node->Attrib("opacity")->ValueFloat();
				mtl.specular_level = mtl_node->Attrib("specular_level")->ValueFloat();
				mtl.shininess = mtl_node->Attrib("shininess")->ValueFloat();

				XMLNodePtr textures_chunk = mtl_node->FirstNode("textures_chunk");
				if (textures_chunk)
				{
					for (XMLNodePtr tex_node = textures_chunk->FirstNode("texture"); tex_node; tex_node = tex_node->NextSibling("texture"))
					{
						mtl.texture_slots.push_back(std::make_pair(tex_node->Attrib("type")->ValueString(),
							tex_node->Attrib("name")->ValueString()));
					}
				}
			}
		}

		XMLNodePtr meshes_chunk = root->FirstNode("meshes_chunk");
		if (meshes_chunk)
		{
			for (XMLNodePtr mesh_node = meshes_chunk->FirstNode("mesh"); mesh_node; mesh_node = mesh_node->NextSibling("mesh"))
			{
				std::wstring wname;
				Convert(wname, mesh_node->Attrib("name")->ValueString());
				StaticMeshPtr mesh = CreateMeshFactoryFunc(ret, wname);

				int32_t mtl_id = mesh_node->Attrib("mtl_id")->ValueUInt();
				mesh->MaterialID(mtl_id);

				uint8_t num_vertex_elems;
				file->read(reinterpret_cast<char*>(&num_vertex_elems), sizeof(num_vertex_elems));

				XMLNodePtr vertex_elements_chunk = mesh_node->FirstNode("vertex_elements_chunk");

				std::vector<vertex_element> vertex_elements;
				for (XMLNodePtr ve_node = vertex_elements_chunk->FirstNode("vertex_element"); ve_node; ve_node = ve_node->NextSibling("vertex_element"))
				{
					vertex_element ve;

					ve.usage = static_cast<VertexElementUsage>(ve_node->Attrib("usage")->ValueUInt());
					ve.usage_index = static_cast<uint8_t>(ve_node->Attrib("usage_index")->ValueUInt());
					uint8_t num_components = static_cast<uint8_t>(ve_node->Attrib("num_components")->ValueUInt());
					if (ve.usage != VEU_BlendIndex)
					{
						switch (num_components)
						{
						case 1:
							ve.format = EF_R32F;
							break;

						case 2:
							ve.format = EF_GR32F;
							break;

						case 3:
							ve.format = EF_BGR32F;
							break;

						case 4:
							ve.format = EF_ABGR32F;
							break;
						}
					}
					else
					{
						switch (num_components)
						{
						case 1:
							ve.format = EF_R8;
							break;

						case 2:
							ve.format = EF_GR8;
							break;

						case 3:
							ve.format = EF_BGR8;
							break;

						case 4:
							ve.format = EF_ABGR8;
							break;
						}
					}

					vertex_elements.push_back(ve);
				}

				XMLNodePtr vertices_chunk = mesh_node->FirstNode("vertices_chunk");
				BOOST_FOREACH(BOOST_TYPEOF(vertex_elements)::const_reference ve, vertex_elements)
				{
					switch (ve.usage)
					{
					case VEU_Position:
						{
							std::vector<float3> buf;
							for (XMLNodePtr vertex_node = vertices_chunk->FirstNode("vertex"); vertex_node; vertex_node = vertex_node->NextSibling("vertex"))
							{
								buf.push_back(float3(vertex_node->Attrib("x")->ValueFloat(),
									vertex_node->Attrib("y")->ValueFloat(), vertex_node->Attrib("z")->ValueFloat()));
							}

							mesh->AddVertexStream(&buf[0], static_cast<uint32_t>(buf.size() * sizeof(buf[0])), ve, access_hint);
						}
						break;

					case VEU_Normal:
						{
							std::vector<float3> buf;
							for (XMLNodePtr vertex_node = vertices_chunk->FirstNode("vertex"); vertex_node; vertex_node = vertex_node->NextSibling("vertex"))
							{
								XMLNodePtr normal_node = vertex_node->FirstNode("normal");
								buf.push_back(float3(normal_node->Attrib("x")->ValueFloat(),
									normal_node->Attrib("y")->ValueFloat(), normal_node->Attrib("z")->ValueFloat()));
							}

							mesh->AddVertexStream(&buf[0], static_cast<uint32_t>(buf.size() * sizeof(buf[0])), ve, access_hint);
						}
						break;

					case VEU_Diffuse:
						{
							std::vector<float4> buf;
							for (XMLNodePtr vertex_node = vertices_chunk->FirstNode("vertex"); vertex_node; vertex_node = vertex_node->NextSibling("vertex"))
							{
								XMLNodePtr diffuse_node = vertex_node->FirstNode("diffuse");
								buf.push_back(float4(diffuse_node->Attrib("r")->ValueFloat(), diffuse_node->Attrib("g")->ValueFloat(),
									diffuse_node->Attrib("b")->ValueFloat(), diffuse_node->Attrib("a")->ValueFloat()));
							}

							mesh->AddVertexStream(&buf[0], static_cast<uint32_t>(buf.size() * sizeof(buf[0])), ve, access_hint);
						}
						break;

					case VEU_Specular:
						{
							std::vector<float4> buf;
							for (XMLNodePtr vertex_node = vertices_chunk->FirstNode("vertex"); vertex_node; vertex_node = vertex_node->NextSibling("vertex"))
							{
								XMLNodePtr specular_node = vertex_node->FirstNode("diffuse");
								buf.push_back(float4(specular_node->Attrib("r")->ValueFloat(), specular_node->Attrib("g")->ValueFloat(),
									specular_node->Attrib("b")->ValueFloat(), specular_node->Attrib("a")->ValueFloat()));
							}

							mesh->AddVertexStream(&buf[0], static_cast<uint32_t>(buf.size() * sizeof(buf[0])), ve, access_hint);
						}
						break;

					case VEU_BlendIndex:
						{
							std::vector<uint8_t> buf;
							for (XMLNodePtr vertex_node = vertices_chunk->FirstNode("vertex"); vertex_node; vertex_node = vertex_node->NextSibling("vertex"))
							{
								for (XMLNodePtr weight_node = vertex_node->FirstNode("weight"); weight_node; weight_node = weight_node->NextSibling("weight"))
								{
									buf.push_back(static_cast<uint8_t>(weight_node->Attrib("bone_index")->ValueUInt()));
								}
							}

							mesh->AddVertexStream(&buf[0], static_cast<uint32_t>(buf.size() * sizeof(buf[0])), ve, access_hint);
						}
						break;

					case VEU_BlendWeight:
						{
							std::vector<float> buf;
							for (XMLNodePtr vertex_node = vertices_chunk->FirstNode("vertex"); vertex_node; vertex_node = vertex_node->NextSibling("vertex"))
							{
								for (XMLNodePtr weight_node = vertex_node->FirstNode("weight"); weight_node; weight_node = weight_node->NextSibling("weight"))
								{
									buf.push_back(weight_node->Attrib("weight")->ValueFloat());
								}
							}

							mesh->AddVertexStream(&buf[0], static_cast<uint32_t>(buf.size() * sizeof(buf[0])), ve, access_hint);
						}
						break;

					case VEU_TextureCoord:
						{
							std::vector<float> buf;
							for (XMLNodePtr vertex_node = vertices_chunk->FirstNode("vertex"); vertex_node; vertex_node = vertex_node->NextSibling("vertex"))
							{
								XMLNodePtr tex_coord_node = vertex_node->FirstNode("tex_coord");
								for (uint32_t usage = 0; tex_coord_node && (usage < ve.usage_index); tex_coord_node = tex_coord_node->NextSibling("tex_coord"));

								uint32_t num_components = NumComponents(ve.format);
								if (num_components >= 1)
								{
									buf.push_back(tex_coord_node->Attrib("u")->ValueFloat());
								}
								if (num_components >= 2)
								{
									buf.push_back(tex_coord_node->Attrib("v")->ValueFloat());
								}
								if (num_components >= 3)
								{
									buf.push_back(tex_coord_node->Attrib("w")->ValueFloat());
								}
							}

							mesh->AddVertexStream(&buf[0], static_cast<uint32_t>(buf.size() * sizeof(buf[0])), ve, access_hint);
						}
						break;

					case VEU_Tangent:
						{
							std::vector<float3> buf;
							for (XMLNodePtr vertex_node = vertices_chunk->FirstNode("vertex"); vertex_node; vertex_node = vertex_node->NextSibling("vertex"))
							{
								XMLNodePtr tangent_node = vertex_node->FirstNode("tangent");
								buf.push_back(float3(tangent_node->Attrib("x")->ValueFloat(),
									tangent_node->Attrib("y")->ValueFloat(), tangent_node->Attrib("z")->ValueFloat()));
							}

							mesh->AddVertexStream(&buf[0], static_cast<uint32_t>(buf.size() * sizeof(buf[0])), ve, access_hint);
						}
						break;

					case VEU_Binormal:
						{
							std::vector<float3> buf;
							for (XMLNodePtr vertex_node = vertices_chunk->FirstNode("vertex"); vertex_node; vertex_node = vertex_node->NextSibling("vertex"))
							{
								XMLNodePtr binormal_node = vertex_node->FirstNode("binormal");
								buf.push_back(float3(binormal_node->Attrib("x")->ValueFloat(),
									binormal_node->Attrib("y")->ValueFloat(), binormal_node->Attrib("z")->ValueFloat()));
							}

							mesh->AddVertexStream(&buf[0], static_cast<uint32_t>(buf.size() * sizeof(buf[0])), ve, access_hint);
						}
						break;
					}
				}

				std::vector<uint16_t> indices;
				XMLNodePtr triangles_chunk = mesh_node->FirstNode("triangles_chunk");
				for (XMLNodePtr tri_node = triangles_chunk->FirstNode("triangle"); tri_node; tri_node = tri_node->NextSibling("triangle"))
				{
					indices.push_back(static_cast<uint16_t>(tri_node->Attrib("a")->ValueUInt()));
					indices.push_back(static_cast<uint16_t>(tri_node->Attrib("b")->ValueUInt()));
					indices.push_back(static_cast<uint16_t>(tri_node->Attrib("c")->ValueUInt()));
				}
				mesh->AddIndexStream(&indices[0], static_cast<uint32_t>(indices.size() * sizeof(indices[0])), EF_R16UI, access_hint);

				meshes.push_back(mesh);
			}
		}

		std::vector<Joint> joints;
		if (bones_chunk)
		{
			for (XMLNodePtr bone_node = bones_chunk->FirstNode("bone"); bone_node; bone_node = bone_node->NextSibling("bone"))
			{
				Joint joint;
				
				joint.name = bone_node->Attrib("name")->ValueString();
				joint.parent = static_cast<int16_t>(bone_node->Attrib("parent")->ValueInt());

				XMLNodePtr bind_pos_node = bone_node->FirstNode("bind_pos");
				joint.bind_pos = float3(bind_pos_node->Attrib("x")->ValueFloat(), bind_pos_node->Attrib("y")->ValueFloat(),
					bind_pos_node->Attrib("z")->ValueFloat());

				XMLNodePtr bind_quat_node = bone_node->FirstNode("bind_quat");
				joint.bind_quat = Quaternion(bind_quat_node->Attrib("x")->ValueFloat(), bind_quat_node->Attrib("y")->ValueFloat(),
					bind_quat_node->Attrib("z")->ValueFloat(), bind_quat_node->Attrib("w")->ValueFloat());

				joint.inverse_origin_quat = MathLib::inverse(joint.bind_quat);
				joint.inverse_origin_pos = MathLib::transform_quat(-joint.bind_pos, joint.inverse_origin_quat);

				joints.push_back(joint);
			}
		}

		XMLNodePtr key_frames_chunk = root->FirstNode("key_frames_chunk");
		if (key_frames_chunk)
		{
			boost::shared_ptr<KeyFramesType> kfs = MakeSharedPtr<KeyFramesType>();
			for (XMLNodePtr kf_node = key_frames_chunk->FirstNode("key_frame"); kf_node; kf_node = kf_node->NextSibling("key_frame"))
			{
				std::string name = kf_node->Attrib("joint")->ValueString();

				KeyFrames kf;
				for (XMLNodePtr key_node = kf_node->FirstNode("key"); key_node; key_node = key_node->NextSibling("key"))
				{
					XMLNodePtr pos_node = key_node->FirstNode("pos");
					kf.bind_pos.push_back(float3(pos_node->Attrib("x")->ValueFloat(), pos_node->Attrib("y")->ValueFloat(),
						pos_node->Attrib("z")->ValueFloat()));

					XMLNodePtr quat_node = key_node->FirstNode("quat");
					kf.bind_quat.push_back(Quaternion(quat_node->Attrib("x")->ValueFloat(), quat_node->Attrib("y")->ValueFloat(),
						quat_node->Attrib("z")->ValueFloat(), quat_node->Attrib("w")->ValueFloat()));
				}

				kfs->insert(std::make_pair(name, kf));
			}

			if (ret->IsSkinned())
			{
				SkinnedModelPtr skinned = checked_pointer_cast<SkinnedModel>(ret);

				skinned->AssignJoints(joints.begin(), joints.end());
				skinned->AttachKeyFrames(kfs);

				skinned->StartFrame(key_frames_chunk->Attrib("start_frame")->ValueInt());
				skinned->EndFrame(key_frames_chunk->Attrib("end_frame")->ValueInt());
				skinned->FrameRate(key_frames_chunk->Attrib("frame_rate")->ValueInt());
			}
		}

		BOOST_FOREACH(BOOST_TYPEOF(meshes)::reference mesh, meshes)
		{
			mesh->BuildMeshInfo();
		}
		ret->AssignMeshes(meshes.begin(), meshes.end());

		return ret;
	}

	void SaveModel(RenderModelPtr model, std::string const & meshml_name)
	{
		KlayGE::XMLDocument doc;

		XMLNodePtr root = doc.AllocNode(XNT_Element, "model");
		doc.RootNode(root);
		root->AppendAttrib(doc.AllocAttribUInt("version", 4));

		if (model->IsSkinned())
		{
			SkinnedModelPtr skinned = checked_pointer_cast<SkinnedModel>(model);

			XMLNodePtr bones_chunk = doc.AllocNode(XNT_Element, "bones_chunk");
			root->AppendNode(bones_chunk);

			uint32_t num_joints = skinned->NumJoints();
			for (uint32_t i = 0; i < num_joints; ++ i)
			{
				XMLNodePtr bone_node = doc.AllocNode(XNT_Element, "bone");
				bones_chunk->AppendNode(bone_node);

				Joint const & joint = checked_pointer_cast<SkinnedModel>(model)->GetJoint(i);

				bone_node->AppendAttrib(doc.AllocAttribString("name", joint.name));
				bone_node->AppendAttrib(doc.AllocAttribInt("parent", joint.parent));

				Quaternion bind_quat = MathLib::inverse(joint.inverse_origin_quat);
				float3 bind_pos = -MathLib::transform_quat(joint.inverse_origin_pos, bind_quat);

				XMLNodePtr bind_pos_node = doc.AllocNode(XNT_Element, "bind_pos");
				bone_node->AppendNode(bind_pos_node);
				bind_pos_node->AppendAttrib(doc.AllocAttribFloat("x", bind_pos.x()));
				bind_pos_node->AppendAttrib(doc.AllocAttribFloat("y", bind_pos.y()));
				bind_pos_node->AppendAttrib(doc.AllocAttribFloat("z", bind_pos.z()));

				XMLNodePtr bind_quat_node = doc.AllocNode(XNT_Element, "bind_quat");
				bone_node->AppendNode(bind_quat_node);
				bind_quat_node->AppendAttrib(doc.AllocAttribFloat("x", bind_quat.x()));
				bind_quat_node->AppendAttrib(doc.AllocAttribFloat("y", bind_quat.y()));
				bind_quat_node->AppendAttrib(doc.AllocAttribFloat("z", bind_quat.z()));
				bind_quat_node->AppendAttrib(doc.AllocAttribFloat("w", bind_quat.w()));
			}
		}

		if (model->NumMaterials() > 0)
		{
			XMLNodePtr materials_chunk = doc.AllocNode(XNT_Element, "materials_chunk");
			root->AppendNode(materials_chunk);

			for (uint32_t i = 0; i < model->NumMaterials(); ++ i)
			{
				RenderModel::Material const & mtl = model->GetMaterial(i);

				XMLNodePtr mtl_node = doc.AllocNode(XNT_Element, "material");
				materials_chunk->AppendNode(mtl_node);

				mtl_node->AppendAttrib(doc.AllocAttribFloat("ambient_r", mtl.ambient.x()));
				mtl_node->AppendAttrib(doc.AllocAttribFloat("ambient_g", mtl.ambient.y()));
				mtl_node->AppendAttrib(doc.AllocAttribFloat("ambient_b", mtl.ambient.z()));
				mtl_node->AppendAttrib(doc.AllocAttribFloat("diffuse_r", mtl.diffuse.x()));
				mtl_node->AppendAttrib(doc.AllocAttribFloat("diffuse_g", mtl.diffuse.y()));
				mtl_node->AppendAttrib(doc.AllocAttribFloat("diffuse_b", mtl.diffuse.z()));
				mtl_node->AppendAttrib(doc.AllocAttribFloat("specular_r", mtl.specular.x()));
				mtl_node->AppendAttrib(doc.AllocAttribFloat("specular_g", mtl.specular.y()));
				mtl_node->AppendAttrib(doc.AllocAttribFloat("specular_b", mtl.specular.z()));
				mtl_node->AppendAttrib(doc.AllocAttribFloat("emit_r", mtl.emit.x()));
				mtl_node->AppendAttrib(doc.AllocAttribFloat("emit_g", mtl.emit.y()));
				mtl_node->AppendAttrib(doc.AllocAttribFloat("emit_b", mtl.emit.z()));
				mtl_node->AppendAttrib(doc.AllocAttribFloat("opacity", mtl.opacity));
				mtl_node->AppendAttrib(doc.AllocAttribFloat("specular_level", mtl.specular_level));
				mtl_node->AppendAttrib(doc.AllocAttribFloat("shininess", mtl.shininess));

				uint32_t const num_textures = static_cast<uint32_t const>(mtl.texture_slots.size());
				if (num_textures > 0)
				{
					XMLNodePtr textures_chunk = doc.AllocNode(XNT_Element, "textures_chunk");
					mtl_node->AppendNode(textures_chunk);

					for (uint8_t j = 0; j < num_textures; ++ j)
					{
						XMLNodePtr texture_node = doc.AllocNode(XNT_Element, "texture");
						textures_chunk->AppendNode(texture_node);

						texture_node->AppendAttrib(doc.AllocAttribString("type", mtl.texture_slots[j].first));
						texture_node->AppendAttrib(doc.AllocAttribString("name", mtl.texture_slots[j].second));
					}
				}
			}
		}

		if (model->NumMeshes() > 0)
		{
			XMLNodePtr meshes_chunk = doc.AllocNode(XNT_Element, "meshes_chunk");
			root->AppendNode(meshes_chunk);

			for (uint32_t i = 0; i < model->NumMeshes(); ++ i)
			{
				StaticMesh const & mesh = *model->Mesh(i);

				XMLNodePtr mesh_node = doc.AllocNode(XNT_Element, "mesh");
				meshes_chunk->AppendNode(mesh_node);

				std::string name;
				Convert(name, mesh.Name());

				mesh_node->AppendAttrib(doc.AllocAttribString("name", name));
				mesh_node->AppendAttrib(doc.AllocAttribInt("mtl_id", mesh.MaterialID()));

				XMLNodePtr vertex_elements_chunk = doc.AllocNode(XNT_Element, "vertex_elements_chunk");
				mesh_node->AppendNode(vertex_elements_chunk);

				RenderLayoutPtr const & rl = mesh.GetRenderLayout();
				for (uint32_t j = 0; j < rl->NumVertexStreams(); ++ j)
				{
					vertex_element const & ve = rl->VertexStreamFormat(j)[0];

					XMLNodePtr ve_node = doc.AllocNode(XNT_Element, "vertex_element");
					vertex_elements_chunk->AppendNode(ve_node);

					ve_node->AppendAttrib(doc.AllocAttribUInt("usage", static_cast<uint32_t>(ve.usage)));
					ve_node->AppendAttrib(doc.AllocAttribUInt("usage_index", ve.usage_index));
					ve_node->AppendAttrib(doc.AllocAttribUInt("num_components", NumComponents(ve.format)));
				}

				XMLNodePtr vertices_chunk = doc.AllocNode(XNT_Element, "vertices_chunk");
				mesh_node->AppendNode(vertices_chunk);

				std::vector<std::vector<uint8_t> > buffs(rl->NumVertexStreams());
				for (uint32_t k = 0; k < rl->NumVertexStreams(); ++ k)
				{
					GraphicsBufferPtr const & vb = rl->GetVertexStream(k);
					GraphicsBufferPtr vb_cpu = Context::Instance().RenderFactoryInstance().MakeVertexBuffer(BU_Static, EAH_CPU_Read, NULL);
					vb_cpu->Resize(vb->Size());
					vb->CopyToBuffer(*vb_cpu);

					buffs[k].resize(vb->Size());

					GraphicsBuffer::Mapper mapper(*vb_cpu, BA_Read_Only);
					memcpy(&buffs[k][0], mapper.Pointer<uint8_t>(), vb->Size());
				}

				uint32_t const num_vertices = static_cast<uint32_t const>(mesh.NumVertices());
				for (uint32_t j = 0; j < num_vertices; ++ j)
				{
					XMLNodePtr vertex_node = doc.AllocNode(XNT_Element, "vertex");
					vertices_chunk->AppendNode(vertex_node);

					for (uint32_t k = 0; k < rl->NumVertexStreams(); ++ k)
					{
						vertex_element const & ve = rl->VertexStreamFormat(k)[0];

						switch (ve.usage)
						{
						case VEU_Position:
							{
								float3* p = reinterpret_cast<float3*>(&buffs[k][0]);
								vertex_node->AppendAttrib(doc.AllocAttribFloat("x", p[j].x()));
								vertex_node->AppendAttrib(doc.AllocAttribFloat("y", p[j].y()));
								vertex_node->AppendAttrib(doc.AllocAttribFloat("z", p[j].z()));
							}
							break;

						case VEU_Normal:
							{
								XMLNodePtr normal_node = doc.AllocNode(XNT_Element, "normal");
								vertex_node->AppendNode(normal_node);

								float3* p = reinterpret_cast<float3*>(&buffs[k][0]);
								normal_node->AppendAttrib(doc.AllocAttribFloat("x", p[j].x()));
								normal_node->AppendAttrib(doc.AllocAttribFloat("y", p[j].y()));
								normal_node->AppendAttrib(doc.AllocAttribFloat("z", p[j].z()));
							}
							break;

						case VEU_Diffuse:
							{
								XMLNodePtr diffuse_node = doc.AllocNode(XNT_Element, "diffuse");
								vertex_node->AppendNode(diffuse_node);

								float4* p = reinterpret_cast<float4*>(&buffs[k][0]);
								diffuse_node->AppendAttrib(doc.AllocAttribFloat("r", p[j].x()));
								diffuse_node->AppendAttrib(doc.AllocAttribFloat("g", p[j].y()));
								diffuse_node->AppendAttrib(doc.AllocAttribFloat("b", p[j].z()));
								diffuse_node->AppendAttrib(doc.AllocAttribFloat("a", p[j].w()));
							}
							break;

						case VEU_Specular:
							{
								XMLNodePtr specular_node = doc.AllocNode(XNT_Element, "specular");
								vertex_node->AppendNode(specular_node);

								float4* p = reinterpret_cast<float4*>(&buffs[k][0]);
								specular_node->AppendAttrib(doc.AllocAttribFloat("r", p[j].x()));
								specular_node->AppendAttrib(doc.AllocAttribFloat("g", p[j].y()));
								specular_node->AppendAttrib(doc.AllocAttribFloat("b", p[j].z()));
								specular_node->AppendAttrib(doc.AllocAttribFloat("b", p[j].w()));
							}
							break;

						case VEU_BlendIndex:
							{
								int weight_stream = -1;
								for (uint32_t l = 0; l < rl->NumVertexStreams(); ++ l)
								{
									vertex_element const & ve = rl->VertexStreamFormat(l)[0];
									if (VEU_BlendWeight == ve.usage)
									{
										weight_stream = l;
										break;
									}
								}

								uint32_t num_components = NumComponents(ve.format);
								for (uint32_t c = 0; c < num_components; ++ c)
								{
									XMLNodePtr weight_node = doc.AllocNode(XNT_Element, "weight");
									vertex_node->AppendNode(weight_node);

									uint8_t* p = reinterpret_cast<uint8_t*>(&buffs[k][0]);
									weight_node->AppendAttrib(doc.AllocAttribFloat("bone_index", p[j * num_components + c]));

									if (weight_stream != -1)
									{
										float* p = reinterpret_cast<float*>(&buffs[weight_stream][0]);
										weight_node->AppendAttrib(doc.AllocAttribFloat("weight", p[j * num_components + c]));
									}
								}
							}
							break;

						case VEU_TextureCoord:
							{
								XMLNodePtr tex_coord_node = doc.AllocNode(XNT_Element, "tex_coord");
								vertex_node->AppendNode(tex_coord_node);
								if (ve.usage_index != 0)
								{
									tex_coord_node->AppendAttrib(doc.AllocAttribInt("usage_index", ve.usage_index));
								}

								float* p = reinterpret_cast<float*>(&buffs[k][0]);
								uint32_t num_components = NumComponents(ve.format);
								if (num_components >= 1)
								{
									tex_coord_node->AppendAttrib(doc.AllocAttribFloat("u", p[j * num_components + 0]));
								}
								if (num_components >= 2)
								{
									tex_coord_node->AppendAttrib(doc.AllocAttribFloat("v", p[j * num_components + 1]));
								}
								if (num_components >= 3)
								{
									tex_coord_node->AppendAttrib(doc.AllocAttribFloat("w", p[j * num_components + 2]));
								}
							}
							break;

						case VEU_Tangent:
							{
								XMLNodePtr tangent_node = doc.AllocNode(XNT_Element, "tangent");
								vertex_node->AppendNode(tangent_node);

								float3* p = reinterpret_cast<float3*>(&buffs[k][0]);
								tangent_node->AppendAttrib(doc.AllocAttribFloat("x", p[j].x()));
								tangent_node->AppendAttrib(doc.AllocAttribFloat("y", p[j].y()));
								tangent_node->AppendAttrib(doc.AllocAttribFloat("z", p[j].z()));
							}
							break;

						case VEU_Binormal:
							{
								XMLNodePtr binormal_node = doc.AllocNode(XNT_Element, "binormal");
								vertex_node->AppendNode(binormal_node);

								float3* p = reinterpret_cast<float3*>(&buffs[k][0]);
								binormal_node->AppendAttrib(doc.AllocAttribFloat("x", p[j].x()));
								binormal_node->AppendAttrib(doc.AllocAttribFloat("y", p[j].y()));
								binormal_node->AppendAttrib(doc.AllocAttribFloat("z", p[j].z()));
							}
							break;
						}
					}
				}

				XMLNodePtr triangles_chunk = doc.AllocNode(XNT_Element, "triangles_chunk");
				mesh_node->AppendNode(triangles_chunk);
				{
					GraphicsBufferPtr ib = rl->GetIndexStream();
					GraphicsBufferPtr ib_cpu = Context::Instance().RenderFactoryInstance().MakeIndexBuffer(BU_Static, EAH_CPU_Read, NULL);
					ib_cpu->Resize(ib->Size());
					ib->CopyToBuffer(*ib_cpu);

					GraphicsBuffer::Mapper mapper(*ib_cpu, BA_Read_Only);

					for (uint32_t i = 0; i < mesh.NumTriangles(); ++ i)
					{
						XMLNodePtr triangle_node = doc.AllocNode(XNT_Element, "triangle");
						triangles_chunk->AppendNode(triangle_node);

						triangle_node->AppendAttrib(doc.AllocAttribInt("a", *(mapper.Pointer<uint16_t>() + i * 3 + 0)));
						triangle_node->AppendAttrib(doc.AllocAttribInt("b", *(mapper.Pointer<uint16_t>() + i * 3 + 1)));
						triangle_node->AppendAttrib(doc.AllocAttribInt("c", *(mapper.Pointer<uint16_t>() + i * 3 + 2)));
					}
				}
			}
		}

		if (model->IsSkinned())
		{
			SkinnedModelPtr skinned = checked_pointer_cast<SkinnedModel>(model);
			if (skinned->GetKeyFrames().size() > 0)
			{
				uint32_t num_key_frames = static_cast<uint32_t>(skinned->GetKeyFrames().size());

				XMLNodePtr key_frames_chunk = doc.AllocNode(XNT_Element, "key_frames_chunk");
				root->AppendNode(key_frames_chunk);

				key_frames_chunk->AppendAttrib(doc.AllocAttribUInt("start_frame", skinned->StartFrame()));
				key_frames_chunk->AppendAttrib(doc.AllocAttribUInt("end_frame", skinned->EndFrame()));
				key_frames_chunk->AppendAttrib(doc.AllocAttribUInt("frame_rate", skinned->FrameRate()));

				KeyFramesType const & kfs = checked_pointer_cast<SkinnedModel>(model)->GetKeyFrames();
				KeyFramesType::const_iterator iter = kfs.begin();

				for (uint32_t i = 0; i < num_key_frames; ++ i, ++ iter)
				{
					XMLNodePtr key_frame_node = doc.AllocNode(XNT_Element, "key_frame");
					key_frames_chunk->AppendNode(key_frame_node);

					key_frame_node->AppendAttrib(doc.AllocAttribString("joint", iter->first));

					for (size_t j = 0; j < iter->second.bind_pos.size(); ++ j)
					{
						XMLNodePtr key_node = doc.AllocNode(XNT_Element, "key");
						key_frame_node->AppendNode(key_node);

						XMLNodePtr pos_node = doc.AllocNode(XNT_Element, "pos");
						key_node->AppendNode(pos_node);
						pos_node->AppendAttrib(doc.AllocAttribFloat("x", iter->second.bind_pos[j].x()));
						pos_node->AppendAttrib(doc.AllocAttribFloat("y", iter->second.bind_pos[j].y()));
						pos_node->AppendAttrib(doc.AllocAttribFloat("z", iter->second.bind_pos[j].z()));

						XMLNodePtr quat_node = doc.AllocNode(XNT_Element, "quat");
						key_node->AppendNode(quat_node);
						quat_node->AppendAttrib(doc.AllocAttribFloat("x", iter->second.bind_quat[j].x()));
						quat_node->AppendAttrib(doc.AllocAttribFloat("y", iter->second.bind_quat[j].y()));
						quat_node->AppendAttrib(doc.AllocAttribFloat("z", iter->second.bind_quat[j].z()));
						quat_node->AppendAttrib(doc.AllocAttribFloat("w", iter->second.bind_quat[j].w()));
					}
				}
			}
		}

		std::ofstream file(meshml_name.c_str());
		doc.Print(file);
	}
}
