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


	void ReadShortString(ResIdentifierPtr& file, std::string& str)
	{
		uint8_t len;
		file->read(reinterpret_cast<char*>(&len), sizeof(len));
		str.resize(len);
		file->read(reinterpret_cast<char*>(&str[0]), len * sizeof(str[0]));
	}

	void WriteShortString(std::ostream& os, std::string const & str)
	{
		BOOST_ASSERT(str.size() < 256);

		uint8_t len = static_cast<uint8_t>(str.length());
		os.write(reinterpret_cast<char const *>(&len), sizeof(len));
		os.write(reinterpret_cast<char const *>(&str[0]), str.size() * sizeof(str[0]));
	}

	void ModelJIT(std::string const & meshml_name)
	{
		std::ofstream ofs((meshml_name + ".model_bin").c_str(), std::ios_base::binary);
		uint32_t fourcc = MakeFourCC<'K', 'L', 'M', 'O'>::value;
		ofs.write(reinterpret_cast<char*>(&fourcc), sizeof(fourcc));

		ResIdentifierPtr file = ResLoader::Instance().Load(meshml_name);
		XMLDocument doc;
		XMLNodePtr root = doc.Parse(file);

		XMLNodePtr materials_chunk = root->FirstNode("materials_chunk");
		if (materials_chunk)
		{
			uint32_t num_mtls = 0;
			for (XMLNodePtr mtl_node = materials_chunk->FirstNode("material"); mtl_node; mtl_node = mtl_node->NextSibling("material"))
			{
				++ num_mtls;
			}
			ofs.write(reinterpret_cast<char*>(&num_mtls), sizeof(num_mtls));
		}
		else
		{
			uint32_t num_mtls = 0;
			ofs.write(reinterpret_cast<char*>(&num_mtls), sizeof(num_mtls));
		}

		XMLNodePtr meshes_chunk = root->FirstNode("meshes_chunk");
		if (meshes_chunk)
		{
			uint32_t num_meshes = 0;
			for (XMLNodePtr mesh_node = meshes_chunk->FirstNode("mesh"); mesh_node; mesh_node = mesh_node->NextSibling("mesh"))
			{
				++ num_meshes;
			}
			ofs.write(reinterpret_cast<char*>(&num_meshes), sizeof(num_meshes));
		}
		else
		{
			uint32_t num_meshes = 0;
			ofs.write(reinterpret_cast<char*>(&num_meshes), sizeof(num_meshes));
		}

		XMLNodePtr bones_chunk = root->FirstNode("bones_chunk");
		if (bones_chunk)
		{
			uint32_t num_joints = 0;
			for (XMLNodePtr bone_node = bones_chunk->FirstNode("bone"); bone_node; bone_node = bone_node->NextSibling("bone"))
			{
				++ num_joints;
			}
			ofs.write(reinterpret_cast<char*>(&num_joints), sizeof(num_joints));
		}
		else
		{
			uint32_t num_joints = 0;
			ofs.write(reinterpret_cast<char*>(&num_joints), sizeof(num_joints));
		}

		XMLNodePtr key_frames_chunk = root->FirstNode("key_frames_chunk");
		if (key_frames_chunk)
		{
			uint32_t num_kfs = 0;
			for (XMLNodePtr kf_node = key_frames_chunk->FirstNode("key_frame"); kf_node; kf_node = kf_node->NextSibling("key_frame"))
			{
				++ num_kfs;
			}
			ofs.write(reinterpret_cast<char*>(&num_kfs), sizeof(num_kfs));
		}
		else
		{
			uint32_t num_kfs = 0;
			ofs.write(reinterpret_cast<char*>(&num_kfs), sizeof(num_kfs));
		}

		if (materials_chunk)
		{
			uint32_t mtl_index = 0;
			for (XMLNodePtr mtl_node = materials_chunk->FirstNode("material"); mtl_node; mtl_node = mtl_node->NextSibling("material"), ++ mtl_index)
			{
				RenderModel::Material mtl;
				float ambient_r = mtl_node->Attrib("ambient_r")->ValueFloat();
				float ambient_g = mtl_node->Attrib("ambient_g")->ValueFloat();
				float ambient_b = mtl_node->Attrib("ambient_b")->ValueFloat();
				float diffuse_r = mtl_node->Attrib("diffuse_r")->ValueFloat();
				float diffuse_g = mtl_node->Attrib("diffuse_g")->ValueFloat();
				float diffuse_b = mtl_node->Attrib("diffuse_b")->ValueFloat();
				float specular_r = mtl_node->Attrib("specular_r")->ValueFloat();
				float specular_g = mtl_node->Attrib("specular_g")->ValueFloat();
				float specular_b = mtl_node->Attrib("specular_b")->ValueFloat();
				float emit_r = mtl_node->Attrib("emit_r")->ValueFloat();
				float emit_g = mtl_node->Attrib("emit_g")->ValueFloat();
				float emit_b = mtl_node->Attrib("emit_b")->ValueFloat();
				float opacity = mtl_node->Attrib("opacity")->ValueFloat();
				float specular_level = mtl_node->Attrib("specular_level")->ValueFloat();
				float shininess = mtl_node->Attrib("shininess")->ValueFloat();

				ofs.write(reinterpret_cast<char*>(&ambient_r), sizeof(ambient_r));
				ofs.write(reinterpret_cast<char*>(&ambient_g), sizeof(ambient_g));
				ofs.write(reinterpret_cast<char*>(&ambient_b), sizeof(ambient_b));
				ofs.write(reinterpret_cast<char*>(&diffuse_r), sizeof(diffuse_r));
				ofs.write(reinterpret_cast<char*>(&diffuse_g), sizeof(diffuse_g));
				ofs.write(reinterpret_cast<char*>(&diffuse_b), sizeof(diffuse_b));
				ofs.write(reinterpret_cast<char*>(&specular_r), sizeof(specular_r));
				ofs.write(reinterpret_cast<char*>(&specular_g), sizeof(specular_g));
				ofs.write(reinterpret_cast<char*>(&specular_b), sizeof(specular_b));
				ofs.write(reinterpret_cast<char*>(&emit_r), sizeof(emit_r));
				ofs.write(reinterpret_cast<char*>(&emit_g), sizeof(emit_g));
				ofs.write(reinterpret_cast<char*>(&emit_b), sizeof(emit_b));
				ofs.write(reinterpret_cast<char*>(&opacity), sizeof(opacity));
				ofs.write(reinterpret_cast<char*>(&specular_level), sizeof(specular_level));
				ofs.write(reinterpret_cast<char*>(&shininess), sizeof(shininess));

				XMLNodePtr textures_chunk = mtl_node->FirstNode("textures_chunk");
				if (textures_chunk)
				{
					uint32_t num_texs = 0;
					for (XMLNodePtr tex_node = textures_chunk->FirstNode("texture"); tex_node; tex_node = tex_node->NextSibling("texture"))
					{
						++ num_texs;
					}
					ofs.write(reinterpret_cast<char*>(&num_texs), sizeof(num_texs));

					for (XMLNodePtr tex_node = textures_chunk->FirstNode("texture"); tex_node; tex_node = tex_node->NextSibling("texture"))
					{
						WriteShortString(ofs, tex_node->Attrib("type")->ValueString());
						WriteShortString(ofs, tex_node->Attrib("name")->ValueString());
					}
				}
				else
				{
					uint32_t num_texs = 0;
					ofs.write(reinterpret_cast<char*>(&num_texs), sizeof(num_texs));
				}
			}
		}

		if (meshes_chunk)
		{
			for (XMLNodePtr mesh_node = meshes_chunk->FirstNode("mesh"); mesh_node; mesh_node = mesh_node->NextSibling("mesh"))
			{
				WriteShortString(ofs, mesh_node->Attrib("name")->ValueString());

				int32_t mtl_id = mesh_node->Attrib("mtl_id")->ValueInt();
				ofs.write(reinterpret_cast<char*>(&mtl_id), sizeof(mtl_id));

				XMLNodePtr vertex_elements_chunk = mesh_node->FirstNode("vertex_elements_chunk");

				uint32_t num_ves = 0;
				for (XMLNodePtr ve_node = vertex_elements_chunk->FirstNode("vertex_element"); ve_node; ve_node = ve_node->NextSibling("vertex_element"))
				{
					++ num_ves;
				}
				ofs.write(reinterpret_cast<char*>(&num_ves), sizeof(num_ves));

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

					ofs.write(reinterpret_cast<char*>(&ve), sizeof(ve));
					vertex_elements.push_back(ve);
				}

				XMLNodePtr vertices_chunk = mesh_node->FirstNode("vertices_chunk");
				
				uint32_t num_vertices = 0;
				for (XMLNodePtr vertex_node = vertices_chunk->FirstNode("vertex"); vertex_node; vertex_node = vertex_node->NextSibling("vertex"))
				{
					++ num_vertices;
				}
				ofs.write(reinterpret_cast<char*>(&num_vertices), sizeof(num_vertices));

				uint32_t max_num_blend = 0;
				for (XMLNodePtr vertex_node = vertices_chunk->FirstNode("vertex"); vertex_node; vertex_node = vertex_node->NextSibling("vertex"))
				{
					uint32_t num_blend = 0;
					for (XMLNodePtr weight_node = vertex_node->FirstNode("weight"); weight_node; weight_node = weight_node->NextSibling("weight"))
					{
						++ num_blend;
					}
					max_num_blend = std::max(max_num_blend, num_blend);
				}
				ofs.write(reinterpret_cast<char*>(&max_num_blend), sizeof(max_num_blend));

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

							ofs.write(reinterpret_cast<char*>(&buf[0]), buf.size() * sizeof(buf[0]));
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

							ofs.write(reinterpret_cast<char*>(&buf[0]), buf.size() * sizeof(buf[0]));
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

							ofs.write(reinterpret_cast<char*>(&buf[0]), buf.size() * sizeof(buf[0]));
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

							ofs.write(reinterpret_cast<char*>(&buf[0]), buf.size() * sizeof(buf[0]));
						}
						break;

					case VEU_BlendIndex:
						{
							std::vector<uint8_t> buf;
							for (XMLNodePtr vertex_node = vertices_chunk->FirstNode("vertex"); vertex_node; vertex_node = vertex_node->NextSibling("vertex"))
							{
								uint32_t num_blend = 0;
								for (XMLNodePtr weight_node = vertex_node->FirstNode("weight"); weight_node; weight_node = weight_node->NextSibling("weight"))
								{
									buf.push_back(static_cast<uint8_t>(weight_node->Attrib("bone_index")->ValueUInt()));
									++ num_blend;
								}
								for (uint32_t i = num_blend; i < max_num_blend; ++ i)
								{
									buf.push_back(0);
								}
							}

							ofs.write(reinterpret_cast<char*>(&buf[0]), buf.size() * sizeof(buf[0]));
						}
						break;

					case VEU_BlendWeight:
						{
							std::vector<float> buf;
							for (XMLNodePtr vertex_node = vertices_chunk->FirstNode("vertex"); vertex_node; vertex_node = vertex_node->NextSibling("vertex"))
							{
								uint32_t num_blend = 0;
								for (XMLNodePtr weight_node = vertex_node->FirstNode("weight"); weight_node; weight_node = weight_node->NextSibling("weight"))
								{
									buf.push_back(weight_node->Attrib("weight")->ValueFloat());
									++ num_blend;
								}
								for (uint32_t i = num_blend; i < max_num_blend; ++ i)
								{
									buf.push_back(0);
								}
							}

							ofs.write(reinterpret_cast<char*>(&buf[0]), buf.size() * sizeof(buf[0]));
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

							ofs.write(reinterpret_cast<char*>(&buf[0]), buf.size() * sizeof(buf[0]));
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

							ofs.write(reinterpret_cast<char*>(&buf[0]), buf.size() * sizeof(buf[0]));
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

							ofs.write(reinterpret_cast<char*>(&buf[0]), buf.size() * sizeof(buf[0]));
						}
						break;
					}
				}

				XMLNodePtr triangles_chunk = mesh_node->FirstNode("triangles_chunk");

				uint32_t num_triangles = 0;
				for (XMLNodePtr tri_node = triangles_chunk->FirstNode("triangle"); tri_node; tri_node = tri_node->NextSibling("triangle"))
				{
					++ num_triangles;
				}
				ofs.write(reinterpret_cast<char*>(&num_triangles), sizeof(num_triangles));

				std::vector<uint16_t> indices;
				for (XMLNodePtr tri_node = triangles_chunk->FirstNode("triangle"); tri_node; tri_node = tri_node->NextSibling("triangle"))
				{
					indices.push_back(static_cast<uint16_t>(tri_node->Attrib("a")->ValueUInt()));
					indices.push_back(static_cast<uint16_t>(tri_node->Attrib("b")->ValueUInt()));
					indices.push_back(static_cast<uint16_t>(tri_node->Attrib("c")->ValueUInt()));
				}
				ofs.write(reinterpret_cast<char*>(&indices[0]), static_cast<uint32_t>(indices.size() * sizeof(indices[0])));
			}
		}

		if (bones_chunk)
		{
			for (XMLNodePtr bone_node = bones_chunk->FirstNode("bone"); bone_node; bone_node = bone_node->NextSibling("bone"))
			{
				WriteShortString(ofs, bone_node->Attrib("name")->ValueString());

				int16_t joint_parent = static_cast<int16_t>(bone_node->Attrib("parent")->ValueInt());
				ofs.write(reinterpret_cast<char*>(&joint_parent), sizeof(joint_parent));

				XMLNodePtr bind_pos_node = bone_node->FirstNode("bind_pos");
				float3 bind_pos(bind_pos_node->Attrib("x")->ValueFloat(), bind_pos_node->Attrib("y")->ValueFloat(),
					bind_pos_node->Attrib("z")->ValueFloat());
				ofs.write(reinterpret_cast<char*>(&bind_pos), sizeof(bind_pos));

				XMLNodePtr bind_quat_node = bone_node->FirstNode("bind_quat");
				Quaternion bind_quat(bind_quat_node->Attrib("x")->ValueFloat(), bind_quat_node->Attrib("y")->ValueFloat(),
					bind_quat_node->Attrib("z")->ValueFloat(), bind_quat_node->Attrib("w")->ValueFloat());
				ofs.write(reinterpret_cast<char*>(&bind_quat), sizeof(bind_quat));
			}
		}

		if (key_frames_chunk)
		{
			int32_t start_frame = key_frames_chunk->Attrib("start_frame")->ValueInt();
			int32_t end_frame = key_frames_chunk->Attrib("end_frame")->ValueInt();
			int32_t frame_rate = key_frames_chunk->Attrib("frame_rate")->ValueInt();
			ofs.write(reinterpret_cast<char*>(&start_frame), sizeof(start_frame));
			ofs.write(reinterpret_cast<char*>(&end_frame), sizeof(end_frame));
			ofs.write(reinterpret_cast<char*>(&frame_rate), sizeof(frame_rate));

			boost::shared_ptr<KeyFramesType> kfs = MakeSharedPtr<KeyFramesType>();
			for (XMLNodePtr kf_node = key_frames_chunk->FirstNode("key_frame"); kf_node; kf_node = kf_node->NextSibling("key_frame"))
			{
				WriteShortString(ofs, kf_node->Attrib("joint")->ValueString());

				uint32_t num_kf = 0;
				for (XMLNodePtr key_node = kf_node->FirstNode("key"); key_node; key_node = key_node->NextSibling("key"))
				{
					++ num_kf;
				}
				ofs.write(reinterpret_cast<char*>(&num_kf), sizeof(num_kf));

				for (XMLNodePtr key_node = kf_node->FirstNode("key"); key_node; key_node = key_node->NextSibling("key"))
				{
					XMLNodePtr pos_node = key_node->FirstNode("pos");
					float3 bind_pos(pos_node->Attrib("x")->ValueFloat(), pos_node->Attrib("y")->ValueFloat(),
						pos_node->Attrib("z")->ValueFloat());
					ofs.write(reinterpret_cast<char*>(&bind_pos), sizeof(bind_pos));

					XMLNodePtr quat_node = key_node->FirstNode("quat");
					Quaternion bind_quat(quat_node->Attrib("x")->ValueFloat(), quat_node->Attrib("y")->ValueFloat(),
						quat_node->Attrib("z")->ValueFloat(), quat_node->Attrib("w")->ValueFloat());
					ofs.write(reinterpret_cast<char*>(&bind_quat), sizeof(bind_quat));
				}
			}
		}
	}

	RenderModelPtr LoadModel(std::string const & meshml_name, uint32_t access_hint,
		boost::function<RenderModelPtr (std::wstring const &)> CreateModelFactoryFunc,
		boost::function<StaticMeshPtr (RenderModelPtr, std::wstring const &)> CreateMeshFactoryFunc)
	{
		BOOST_ASSERT(CreateModelFactoryFunc);
		BOOST_ASSERT(CreateMeshFactoryFunc);

		std::string full_meshml_name = ResLoader::Instance().Locate(meshml_name);
		if (ResLoader::Instance().Locate(full_meshml_name + ".model_bin").empty())
		{
			ModelJIT(full_meshml_name);
		}

		ResIdentifierPtr file = ResLoader::Instance().Load(full_meshml_name + ".model_bin");
		uint32_t fourcc;
		file->read(&fourcc, sizeof(fourcc));
		BOOST_ASSERT((fourcc == MakeFourCC<'K', 'L', 'M', 'O'>::value));

		uint32_t num_mtls;
		file->read(&num_mtls, sizeof(num_mtls));
		uint32_t num_meshes;
		file->read(&num_meshes, sizeof(num_meshes));
		uint32_t num_joints;
		file->read(&num_joints, sizeof(num_joints));
		uint32_t num_kfs;
		file->read(&num_kfs, sizeof(num_kfs));

		std::wstring model_name;
		if (num_joints > 0)
		{
			model_name = L"KSkinnedMesh";
		}
		else
		{
			model_name = L"KMesh";
		}
		RenderModelPtr ret = CreateModelFactoryFunc(model_name);

		ret->NumMaterials(num_mtls);
		for (uint32_t mtl_index = 0; mtl_index < num_mtls; ++ mtl_index)
		{
			RenderModel::Material& mtl = ret->GetMaterial(mtl_index);
			file->read(&mtl.ambient.x(), sizeof(float));
			file->read(&mtl.ambient.y(), sizeof(float));
			file->read(&mtl.ambient.z(), sizeof(float));
			file->read(&mtl.diffuse.x(), sizeof(float));
			file->read(&mtl.diffuse.y(), sizeof(float));
			file->read(&mtl.diffuse.z(), sizeof(float));
			file->read(&mtl.specular.x(), sizeof(float));
			file->read(&mtl.specular.y(), sizeof(float));
			file->read(&mtl.specular.z(), sizeof(float));
			file->read(&mtl.emit.x(), sizeof(float));
			file->read(&mtl.emit.y(), sizeof(float));
			file->read(&mtl.emit.z(), sizeof(float));
			file->read(&mtl.opacity, sizeof(float));
			file->read(&mtl.specular_level, sizeof(float));
			file->read(&mtl.shininess, sizeof(float));

			uint32_t num_texs;
			file->read(&num_texs, sizeof(num_texs));
			
			for (uint32_t tex_index = 0; tex_index < num_texs; ++ tex_index)
			{
				std::string type, name;
				ReadShortString(file, type);
				ReadShortString(file, name);
				mtl.texture_slots.push_back(std::make_pair(type, name));
			}
		}

		std::vector<StaticMeshPtr> meshes(num_meshes);
		for (uint32_t mesh_index = 0; mesh_index < num_meshes; ++ mesh_index)
		{
			std::string name;
			ReadShortString(file, name);

			std::wstring wname;
			Convert(wname, name);

			meshes[mesh_index] = CreateMeshFactoryFunc(ret, wname);
			StaticMeshPtr& mesh = meshes[mesh_index];

			int32_t mtl_id;
			file->read(&mtl_id, sizeof(mtl_id));
			mesh->MaterialID(mtl_id);

			uint32_t num_ves;
			file->read(&num_ves, sizeof(num_ves));

			std::vector<vertex_element> vertex_elements(num_ves);
			for (uint32_t ve_index = 0; ve_index < num_ves; ++ ve_index)
			{
				file->read(&vertex_elements[ve_index], sizeof(vertex_elements[ve_index]));
			}

			uint32_t num_vertices;
			file->read(&num_vertices, sizeof(num_vertices));

			uint32_t max_num_blend;
			file->read(&max_num_blend, sizeof(max_num_blend));

			BOOST_FOREACH(BOOST_TYPEOF(vertex_elements)::const_reference ve, vertex_elements)
			{
				switch (ve.usage)
				{
				case VEU_Position:
					{
						std::vector<float3> buf(num_vertices);
						file->read(&buf[0], buf.size() * sizeof(buf[0]));
						mesh->AddVertexStream(&buf[0], static_cast<uint32_t>(buf.size() * sizeof(buf[0])), ve, access_hint);
					}
					break;

				case VEU_Normal:
					{
						std::vector<float3> buf(num_vertices);
						file->read(&buf[0], buf.size() * sizeof(buf[0]));
						mesh->AddVertexStream(&buf[0], static_cast<uint32_t>(buf.size() * sizeof(buf[0])), ve, access_hint);
					}
					break;

				case VEU_Diffuse:
					{
						std::vector<float4> buf(num_vertices);
						file->read(&buf[0], buf.size() * sizeof(buf[0]));
						mesh->AddVertexStream(&buf[0], static_cast<uint32_t>(buf.size() * sizeof(buf[0])), ve, access_hint);
					}
					break;

				case VEU_Specular:
					{
						std::vector<float4> buf(num_vertices);
						file->read(&buf[0], buf.size() * sizeof(buf[0]));
						mesh->AddVertexStream(&buf[0], static_cast<uint32_t>(buf.size() * sizeof(buf[0])), ve, access_hint);
					}
					break;

				case VEU_BlendIndex:
					{
						std::vector<uint8_t> buf(num_vertices * max_num_blend);
						file->read(&buf[0], buf.size() * sizeof(buf[0]));
						mesh->AddVertexStream(&buf[0], static_cast<uint32_t>(buf.size() * sizeof(buf[0])), ve, access_hint);
					}
					break;

				case VEU_BlendWeight:
					{
						std::vector<float> buf(num_vertices * max_num_blend);
						file->read(&buf[0], buf.size() * sizeof(buf[0]));
						mesh->AddVertexStream(&buf[0], static_cast<uint32_t>(buf.size() * sizeof(buf[0])), ve, access_hint);
					}
					break;

				case VEU_TextureCoord:
					{
						std::vector<float> buf(num_vertices * NumComponents(ve.format));
						file->read(&buf[0], buf.size() * sizeof(buf[0]));
						mesh->AddVertexStream(&buf[0], static_cast<uint32_t>(buf.size() * sizeof(buf[0])), ve, access_hint);
					}
					break;

				case VEU_Tangent:
					{
						std::vector<float3> buf(num_vertices);
						file->read(&buf[0], buf.size() * sizeof(buf[0]));
						mesh->AddVertexStream(&buf[0], static_cast<uint32_t>(buf.size() * sizeof(buf[0])), ve, access_hint);
					}
					break;

				case VEU_Binormal:
					{
						std::vector<float3> buf(num_vertices);
						file->read(&buf[0], buf.size() * sizeof(buf[0]));
						mesh->AddVertexStream(&buf[0], static_cast<uint32_t>(buf.size() * sizeof(buf[0])), ve, access_hint);
					}
					break;
				}
			}

			uint32_t num_triangles;
			file->read(&num_triangles, sizeof(num_triangles));

			std::vector<uint16_t> indices(num_triangles * 3);
			file->read(&indices[0], indices.size() * sizeof(indices[0]));
			mesh->AddIndexStream(&indices[0], static_cast<uint32_t>(indices.size() * sizeof(indices[0])), EF_R16UI, access_hint);
		}

		std::vector<Joint> joints(num_joints);
		for (uint32_t joint_index = 0; joint_index < num_joints; ++ joint_index)
		{
			Joint& joint = joints[joint_index];

			ReadShortString(file, joint.name);
			file->read(&joint.parent, sizeof(joint.parent));

			file->read(&joint.bind_pos, sizeof(joint.bind_pos));
			file->read(&joint.bind_quat, sizeof(joint.bind_quat));

			joint.inverse_origin_quat = MathLib::inverse(joint.bind_quat);
			joint.inverse_origin_pos = MathLib::transform_quat(-joint.bind_pos, joint.inverse_origin_quat);
		}

		if (num_kfs > 0)
		{
			int32_t start_frame;
			int32_t end_frame;
			int32_t frame_rate;
			file->read(&start_frame, sizeof(start_frame));
			file->read(&end_frame, sizeof(end_frame));
			file->read(&frame_rate, sizeof(frame_rate));

			boost::shared_ptr<KeyFramesType> kfs = MakeSharedPtr<KeyFramesType>();
			for (uint32_t kf_index = 0; kf_index < num_kfs; ++ kf_index)
			{
				std::string name;
				ReadShortString(file, name);

				uint32_t num_kf;
				file->read(&num_kf, sizeof(num_kf));

				KeyFrames kf;
				kf.bind_pos.resize(num_kf);
				kf.bind_quat.resize(num_kf);
				for (uint32_t k_index = 0; k_index < num_kf; ++ k_index)
				{
					file->read(&kf.bind_pos[k_index], sizeof(kf.bind_pos[k_index]));
					file->read(&kf.bind_quat[k_index], sizeof(kf.bind_quat[k_index]));
				}

				kfs->insert(std::make_pair(name, kf));
			}

			if (ret->IsSkinned())
			{
				SkinnedModelPtr skinned = checked_pointer_cast<SkinnedModel>(ret);

				skinned->AssignJoints(joints.begin(), joints.end());
				skinned->AttachKeyFrames(kfs);

				skinned->StartFrame(start_frame);
				skinned->EndFrame(end_frame);
				skinned->FrameRate(frame_rate);
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

						case VEU_BlendWeight:
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
