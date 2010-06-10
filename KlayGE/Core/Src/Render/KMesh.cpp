// KMesh.cpp
// KlayGE KMesh类 实现文件
// Ver 3.10.0
// 版权所有(C) 龚敏敏, 2005-2010
// Homepage: http://www.klayge.org
//
// 3.10.0
// 支持32-bit index (2010.2.28)
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
#include <KlayGE/LZMACodec.hpp>
#include <KlayGE/thread.hpp>

#include <fstream>
#include <sstream>
#include <boost/assert.hpp>
#include <boost/typeof/typeof.hpp>
#include <boost/foreach.hpp>

#include <KlayGE/KMesh.hpp>

namespace
{
	using namespace KlayGE;

	class RenderModelLoader
	{
	private:
		struct ModelDesc
		{
			uint32_t access_hint;
			boost::function<RenderModelPtr (std::wstring const &)> CreateModelFactoryFunc;
			boost::function<StaticMeshPtr (RenderModelPtr, std::wstring const &)> CreateMeshFactoryFunc;

			std::vector<RenderModel::Material> mtls;
			std::vector<std::string> mesh_names;
			std::vector<int32_t> mtl_ids;
			std::vector<std::vector<vertex_element> > ves;
			std::vector<uint32_t> max_num_blends;
			std::vector<std::vector<std::vector<uint8_t> > > buffs;
			std::vector<char> is_index_16_bit;
			std::vector<std::vector<uint8_t> > indices;
			std::vector<Joint> joints;
			boost::shared_ptr<KeyFramesType> kfs;
			int32_t start_frame;
			int32_t end_frame;
			int32_t frame_rate;

			RenderModelPtr model;
		};

	public:
		RenderModelLoader(std::string const & meshml_name, uint32_t access_hint,
			boost::function<RenderModelPtr (std::wstring const &)> CreateModelFactoryFunc,
			boost::function<StaticMeshPtr (RenderModelPtr, std::wstring const &)> CreateMeshFactoryFunc)
		{
			ml_thread_ = GlobalThreadPool()(boost::bind(&RenderModelLoader::LoadKModel, this, meshml_name, access_hint, CreateModelFactoryFunc, CreateMeshFactoryFunc));
		}

		RenderModelPtr operator()()
		{
			if (!model_)
			{
				boost::shared_ptr<ModelDesc> model_desc = ml_thread_();

				if (model_desc->model)
				{
					model_ = model_desc->model;
				}
				else
				{
					model_ = this->CreateModel(model_desc);
				}

				for (uint32_t i = 0; i < model_->NumMeshes(); ++ i)
				{
					model_->Mesh(i)->BuildMeshInfo();
				}
			}

			return model_;
		}

	private:
		boost::shared_ptr<ModelDesc> LoadKModel(std::string const & meshml_name, uint32_t access_hint,
			boost::function<RenderModelPtr (std::wstring const &)> CreateModelFactoryFunc,
			boost::function<StaticMeshPtr (RenderModelPtr, std::wstring const &)> CreateMeshFactoryFunc)
		{
			boost::shared_ptr<ModelDesc> model_desc = MakeSharedPtr<ModelDesc>();
			model_desc->access_hint = access_hint;
			model_desc->CreateModelFactoryFunc = CreateModelFactoryFunc;
			model_desc->CreateMeshFactoryFunc = CreateMeshFactoryFunc;

			LoadModel(meshml_name, model_desc->mtls, model_desc->mesh_names, model_desc->mtl_ids, model_desc->ves, model_desc->max_num_blends,
				model_desc->buffs, model_desc->is_index_16_bit, model_desc->indices, model_desc->joints, model_desc->kfs,
				model_desc->start_frame, model_desc->end_frame, model_desc->frame_rate);

			RenderFactory& rf = Context::Instance().RenderFactoryInstance();
			if (rf.RenderEngineInstance().DeviceCaps().multithread_res_creating_support)
			{
				model_desc->model = this->CreateModel(model_desc);
			}

			return model_desc;
		}

		RenderModelPtr CreateModel(boost::shared_ptr<ModelDesc> const & model_desc)
		{
			std::wstring model_name;
			if (!model_desc->joints.empty())
			{
				model_name = L"KSkinnedMesh";
			}
			else
			{
				model_name = L"KMesh";
			}
			RenderModelPtr model = model_desc->CreateModelFactoryFunc(model_name);

			model->NumMaterials(model_desc->mtls.size());
			for (uint32_t mtl_index = 0; mtl_index < model_desc->mtls.size(); ++ mtl_index)
			{
				model->GetMaterial(mtl_index) = model_desc->mtls[mtl_index];
			}

			std::vector<StaticMeshPtr> meshes(model_desc->mesh_names.size());
			for (uint32_t mesh_index = 0; mesh_index < model_desc->mesh_names.size(); ++ mesh_index)
			{
				std::wstring wname;
				Convert(wname, model_desc->mesh_names[mesh_index]);

				meshes[mesh_index] = model_desc->CreateMeshFactoryFunc(model, wname);
				StaticMeshPtr& mesh = meshes[mesh_index];

				mesh->MaterialID(model_desc->mtl_ids[mesh_index]);

				for (uint32_t ve_index = 0; ve_index < model_desc->buffs[mesh_index].size(); ++ ve_index)
				{
					std::vector<uint8_t>& buff = model_desc->buffs[mesh_index][ve_index];
					mesh->AddVertexStream(&buff[0], static_cast<uint32_t>(buff.size() * sizeof(buff[0])), model_desc->ves[mesh_index][ve_index], model_desc->access_hint);
				}

				mesh->AddIndexStream(&model_desc->indices[mesh_index][0], static_cast<uint32_t>(model_desc->indices[mesh_index].size() * sizeof(model_desc->indices[mesh_index][0])),
					model_desc->is_index_16_bit[mesh_index] ? EF_R16UI : EF_R32UI, model_desc->access_hint);
			}

			if (model_desc->kfs && !model_desc->kfs->empty())
			{
				if (model->IsSkinned())
				{
					SkinnedModelPtr skinned = checked_pointer_cast<SkinnedModel>(model);

					skinned->AssignJoints(model_desc->joints.begin(), model_desc->joints.end());
					skinned->AttachKeyFrames(model_desc->kfs);

					skinned->StartFrame(model_desc->start_frame);
					skinned->EndFrame(model_desc->end_frame);
					skinned->FrameRate(model_desc->frame_rate);
				}
			}

			model->AssignMeshes(meshes.begin(), meshes.end());

			return model;
		}

	private:
		joiner<boost::shared_ptr<ModelDesc> > ml_thread_;
		RenderModelPtr model_;
	};
}

namespace KlayGE
{
	std::string const jit_ext_name = ".model_bin";

	KMesh::KMesh(RenderModelPtr const & model, std::wstring const & name)
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
		bool jit = false;
		if (ResLoader::Instance().Locate(meshml_name + jit_ext_name).empty())
		{
			jit = true;
		}
		else
		{
			ResIdentifierPtr lzma_file = ResLoader::Instance().Load(meshml_name + jit_ext_name);
			uint32_t fourcc;
			lzma_file->read(&fourcc, sizeof(fourcc));
			if (fourcc != MakeFourCC<'K', 'L', 'M', '1'>::value)
			{
				jit = true;
			}
		}

		if (jit)
		{
			boost::shared_ptr<std::stringstream> ss = MakeSharedPtr<std::stringstream>();

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
				ss->write(reinterpret_cast<char*>(&num_mtls), sizeof(num_mtls));
			}
			else
			{
				uint32_t num_mtls = 0;
				ss->write(reinterpret_cast<char*>(&num_mtls), sizeof(num_mtls));
			}

			XMLNodePtr meshes_chunk = root->FirstNode("meshes_chunk");
			if (meshes_chunk)
			{
				uint32_t num_meshes = 0;
				for (XMLNodePtr mesh_node = meshes_chunk->FirstNode("mesh"); mesh_node; mesh_node = mesh_node->NextSibling("mesh"))
				{
					++ num_meshes;
				}
				ss->write(reinterpret_cast<char*>(&num_meshes), sizeof(num_meshes));
			}
			else
			{
				uint32_t num_meshes = 0;
				ss->write(reinterpret_cast<char*>(&num_meshes), sizeof(num_meshes));
			}

			XMLNodePtr bones_chunk = root->FirstNode("bones_chunk");
			if (bones_chunk)
			{
				uint32_t num_joints = 0;
				for (XMLNodePtr bone_node = bones_chunk->FirstNode("bone"); bone_node; bone_node = bone_node->NextSibling("bone"))
				{
					++ num_joints;
				}
				ss->write(reinterpret_cast<char*>(&num_joints), sizeof(num_joints));
			}
			else
			{
				uint32_t num_joints = 0;
				ss->write(reinterpret_cast<char*>(&num_joints), sizeof(num_joints));
			}

			XMLNodePtr key_frames_chunk = root->FirstNode("key_frames_chunk");
			if (key_frames_chunk)
			{
				uint32_t num_kfs = 0;
				for (XMLNodePtr kf_node = key_frames_chunk->FirstNode("key_frame"); kf_node; kf_node = kf_node->NextSibling("key_frame"))
				{
					++ num_kfs;
				}
				ss->write(reinterpret_cast<char*>(&num_kfs), sizeof(num_kfs));
			}
			else
			{
				uint32_t num_kfs = 0;
				ss->write(reinterpret_cast<char*>(&num_kfs), sizeof(num_kfs));
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

					ss->write(reinterpret_cast<char*>(&ambient_r), sizeof(ambient_r));
					ss->write(reinterpret_cast<char*>(&ambient_g), sizeof(ambient_g));
					ss->write(reinterpret_cast<char*>(&ambient_b), sizeof(ambient_b));
					ss->write(reinterpret_cast<char*>(&diffuse_r), sizeof(diffuse_r));
					ss->write(reinterpret_cast<char*>(&diffuse_g), sizeof(diffuse_g));
					ss->write(reinterpret_cast<char*>(&diffuse_b), sizeof(diffuse_b));
					ss->write(reinterpret_cast<char*>(&specular_r), sizeof(specular_r));
					ss->write(reinterpret_cast<char*>(&specular_g), sizeof(specular_g));
					ss->write(reinterpret_cast<char*>(&specular_b), sizeof(specular_b));
					ss->write(reinterpret_cast<char*>(&emit_r), sizeof(emit_r));
					ss->write(reinterpret_cast<char*>(&emit_g), sizeof(emit_g));
					ss->write(reinterpret_cast<char*>(&emit_b), sizeof(emit_b));
					ss->write(reinterpret_cast<char*>(&opacity), sizeof(opacity));
					ss->write(reinterpret_cast<char*>(&specular_level), sizeof(specular_level));
					ss->write(reinterpret_cast<char*>(&shininess), sizeof(shininess));

					XMLNodePtr textures_chunk = mtl_node->FirstNode("textures_chunk");
					if (textures_chunk)
					{
						uint32_t num_texs = 0;
						for (XMLNodePtr tex_node = textures_chunk->FirstNode("texture"); tex_node; tex_node = tex_node->NextSibling("texture"))
						{
							++ num_texs;
						}
						ss->write(reinterpret_cast<char*>(&num_texs), sizeof(num_texs));

						for (XMLNodePtr tex_node = textures_chunk->FirstNode("texture"); tex_node; tex_node = tex_node->NextSibling("texture"))
						{
							WriteShortString(*ss, tex_node->Attrib("type")->ValueString());
							WriteShortString(*ss, tex_node->Attrib("name")->ValueString());
						}
					}
					else
					{
						uint32_t num_texs = 0;
						ss->write(reinterpret_cast<char*>(&num_texs), sizeof(num_texs));
					}
				}
			}

			if (meshes_chunk)
			{
				for (XMLNodePtr mesh_node = meshes_chunk->FirstNode("mesh"); mesh_node; mesh_node = mesh_node->NextSibling("mesh"))
				{
					WriteShortString(*ss, mesh_node->Attrib("name")->ValueString());

					int32_t mtl_id = mesh_node->Attrib("mtl_id")->ValueInt();
					ss->write(reinterpret_cast<char*>(&mtl_id), sizeof(mtl_id));

					XMLNodePtr vertex_elements_chunk = mesh_node->FirstNode("vertex_elements_chunk");

					uint32_t num_ves = 0;
					for (XMLNodePtr ve_node = vertex_elements_chunk->FirstNode("vertex_element"); ve_node; ve_node = ve_node->NextSibling("vertex_element"))
					{
						++ num_ves;
					}
					ss->write(reinterpret_cast<char*>(&num_ves), sizeof(num_ves));

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

						ss->write(reinterpret_cast<char*>(&ve), sizeof(ve));
						vertex_elements.push_back(ve);
					}

					XMLNodePtr vertices_chunk = mesh_node->FirstNode("vertices_chunk");

					uint32_t num_vertices = 0;
					for (XMLNodePtr vertex_node = vertices_chunk->FirstNode("vertex"); vertex_node; vertex_node = vertex_node->NextSibling("vertex"))
					{
						++ num_vertices;
					}
					ss->write(reinterpret_cast<char*>(&num_vertices), sizeof(num_vertices));

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
					ss->write(reinterpret_cast<char*>(&max_num_blend), sizeof(max_num_blend));

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

								ss->write(reinterpret_cast<char*>(&buf[0]), buf.size() * sizeof(buf[0]));
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

								ss->write(reinterpret_cast<char*>(&buf[0]), buf.size() * sizeof(buf[0]));
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

								ss->write(reinterpret_cast<char*>(&buf[0]), buf.size() * sizeof(buf[0]));
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

								ss->write(reinterpret_cast<char*>(&buf[0]), buf.size() * sizeof(buf[0]));
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

								ss->write(reinterpret_cast<char*>(&buf[0]), buf.size() * sizeof(buf[0]));
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

								ss->write(reinterpret_cast<char*>(&buf[0]), buf.size() * sizeof(buf[0]));
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

								ss->write(reinterpret_cast<char*>(&buf[0]), buf.size() * sizeof(buf[0]));
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

								ss->write(reinterpret_cast<char*>(&buf[0]), buf.size() * sizeof(buf[0]));
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

								ss->write(reinterpret_cast<char*>(&buf[0]), buf.size() * sizeof(buf[0]));
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
					ss->write(reinterpret_cast<char*>(&num_triangles), sizeof(num_triangles));

					char is_index_16 = true;
					std::vector<uint32_t> indices;
					for (XMLNodePtr tri_node = triangles_chunk->FirstNode("triangle"); tri_node; tri_node = tri_node->NextSibling("triangle"))
					{
						uint32_t a = static_cast<uint32_t>(tri_node->Attrib("a")->ValueUInt());
						uint32_t b = static_cast<uint32_t>(tri_node->Attrib("b")->ValueUInt());
						uint32_t c = static_cast<uint32_t>(tri_node->Attrib("c")->ValueUInt());
						indices.push_back(a);
						indices.push_back(b);
						indices.push_back(c);

						if ((a > 0xFFFF) || (b > 0xFFFF) || (c > 0xFFFF))
						{
							is_index_16 = false;
						}
					}
					ss->write(&is_index_16, sizeof(is_index_16));
					if (is_index_16)
					{
						std::vector<uint16_t> indices_16(indices.size());
						for (size_t i = 0; i < indices.size(); ++ i)
						{
							indices_16[i] = static_cast<uint16_t>(indices[i]);
						}
						ss->write(reinterpret_cast<char*>(&indices_16[0]), static_cast<uint32_t>(indices_16.size() * sizeof(indices_16[0])));
					}
					else
					{
						ss->write(reinterpret_cast<char*>(&indices[0]), static_cast<uint32_t>(indices.size() * sizeof(indices[0])));
					}
				}
			}

			if (bones_chunk)
			{
				for (XMLNodePtr bone_node = bones_chunk->FirstNode("bone"); bone_node; bone_node = bone_node->NextSibling("bone"))
				{
					WriteShortString(*ss, bone_node->Attrib("name")->ValueString());

					int16_t joint_parent = static_cast<int16_t>(bone_node->Attrib("parent")->ValueInt());
					ss->write(reinterpret_cast<char*>(&joint_parent), sizeof(joint_parent));

					XMLNodePtr bind_pos_node = bone_node->FirstNode("bind_pos");
					float3 bind_pos(bind_pos_node->Attrib("x")->ValueFloat(), bind_pos_node->Attrib("y")->ValueFloat(),
						bind_pos_node->Attrib("z")->ValueFloat());
					ss->write(reinterpret_cast<char*>(&bind_pos), sizeof(bind_pos));

					XMLNodePtr bind_quat_node = bone_node->FirstNode("bind_quat");
					Quaternion bind_quat(bind_quat_node->Attrib("x")->ValueFloat(), bind_quat_node->Attrib("y")->ValueFloat(),
						bind_quat_node->Attrib("z")->ValueFloat(), bind_quat_node->Attrib("w")->ValueFloat());
					ss->write(reinterpret_cast<char*>(&bind_quat), sizeof(bind_quat));
				}
			}

			if (key_frames_chunk)
			{
				int32_t start_frame = key_frames_chunk->Attrib("start_frame")->ValueInt();
				int32_t end_frame = key_frames_chunk->Attrib("end_frame")->ValueInt();
				int32_t frame_rate = key_frames_chunk->Attrib("frame_rate")->ValueInt();
				ss->write(reinterpret_cast<char*>(&start_frame), sizeof(start_frame));
				ss->write(reinterpret_cast<char*>(&end_frame), sizeof(end_frame));
				ss->write(reinterpret_cast<char*>(&frame_rate), sizeof(frame_rate));

				boost::shared_ptr<KeyFramesType> kfs = MakeSharedPtr<KeyFramesType>();
				for (XMLNodePtr kf_node = key_frames_chunk->FirstNode("key_frame"); kf_node; kf_node = kf_node->NextSibling("key_frame"))
				{
					WriteShortString(*ss, kf_node->Attrib("joint")->ValueString());

					uint32_t num_kf = 0;
					for (XMLNodePtr key_node = kf_node->FirstNode("key"); key_node; key_node = key_node->NextSibling("key"))
					{
						++ num_kf;
					}
					ss->write(reinterpret_cast<char*>(&num_kf), sizeof(num_kf));

					for (XMLNodePtr key_node = kf_node->FirstNode("key"); key_node; key_node = key_node->NextSibling("key"))
					{
						XMLNodePtr pos_node = key_node->FirstNode("pos");
						float3 bind_pos(pos_node->Attrib("x")->ValueFloat(), pos_node->Attrib("y")->ValueFloat(),
							pos_node->Attrib("z")->ValueFloat());
						ss->write(reinterpret_cast<char*>(&bind_pos), sizeof(bind_pos));

						XMLNodePtr quat_node = key_node->FirstNode("quat");
						Quaternion bind_quat(quat_node->Attrib("x")->ValueFloat(), quat_node->Attrib("y")->ValueFloat(),
							quat_node->Attrib("z")->ValueFloat(), quat_node->Attrib("w")->ValueFloat());
						ss->write(reinterpret_cast<char*>(&bind_quat), sizeof(bind_quat));
					}
				}
			}

			std::ofstream ofs((meshml_name + jit_ext_name).c_str(), std::ios_base::binary);
			BOOST_ASSERT(ofs);
			uint32_t fourcc = MakeFourCC<'K', 'L', 'M', '1'>::value;
			ofs.write(reinterpret_cast<char*>(&fourcc), sizeof(fourcc));

			uint64_t original_len = ss->str().size();
			ofs.write(reinterpret_cast<char*>(&original_len), sizeof(original_len));

			std::ofstream::pos_type p = ofs.tellp();
			uint64_t len = 0;
			ofs.write(reinterpret_cast<char*>(&len), sizeof(len));

			LZMACodec lzma;
			len = lzma.Encode(ofs, ss->str().c_str(), ss->str().size());

			ofs.seekp(p, std::ios_base::beg);
			ofs.write(reinterpret_cast<char*>(&len), sizeof(len));
		}
	}

	void LoadModel(std::string const & meshml_name, std::vector<RenderModel::Material>& mtls,
		std::vector<std::string>& mesh_names, std::vector<int32_t>& mtl_ids, std::vector<std::vector<vertex_element> >& ves,
		std::vector<uint32_t>& max_num_blends, std::vector<std::vector<std::vector<uint8_t> > >& buffs,
		std::vector<char>& is_index_16_bit, std::vector<std::vector<uint8_t> >& indices,
		std::vector<Joint>& joints, boost::shared_ptr<KeyFramesType>& kfs,
		int32_t& start_frame, int32_t& end_frame, int32_t& frame_rate)
	{
		ResIdentifierPtr lzma_file;
		if (meshml_name.rfind(jit_ext_name) + jit_ext_name.size() == meshml_name.size())
		{
			lzma_file = ResLoader::Instance().Load(meshml_name);
		}
		else
		{
			std::string full_meshml_name = ResLoader::Instance().Locate(meshml_name);
			ModelJIT(full_meshml_name);

			lzma_file = ResLoader::Instance().Load(full_meshml_name + jit_ext_name);
		}
		uint32_t fourcc;
		lzma_file->read(&fourcc, sizeof(fourcc));
		BOOST_ASSERT((fourcc == MakeFourCC<'K', 'L', 'M', '1'>::value));

		boost::shared_ptr<std::stringstream> ss = MakeSharedPtr<std::stringstream>();

		uint64_t original_len, len;
		lzma_file->read(&original_len, sizeof(original_len));
		lzma_file->read(&len, sizeof(len));

		LZMACodec lzma;
		lzma.Decode(*ss, lzma_file, len, original_len);

		ResIdentifierPtr decoded = MakeSharedPtr<ResIdentifier>(lzma_file->ResName(), ss);

		uint32_t num_mtls;
		decoded->read(&num_mtls, sizeof(num_mtls));
		uint32_t num_meshes;
		decoded->read(&num_meshes, sizeof(num_meshes));
		uint32_t num_joints;
		decoded->read(&num_joints, sizeof(num_joints));
		uint32_t num_kfs;
		decoded->read(&num_kfs, sizeof(num_kfs));

		mtls.resize(num_mtls);
		for (uint32_t mtl_index = 0; mtl_index < num_mtls; ++ mtl_index)
		{
			RenderModel::Material& mtl = mtls[mtl_index];
			decoded->read(&mtl.ambient.x(), sizeof(float));
			decoded->read(&mtl.ambient.y(), sizeof(float));
			decoded->read(&mtl.ambient.z(), sizeof(float));
			decoded->read(&mtl.diffuse.x(), sizeof(float));
			decoded->read(&mtl.diffuse.y(), sizeof(float));
			decoded->read(&mtl.diffuse.z(), sizeof(float));
			decoded->read(&mtl.specular.x(), sizeof(float));
			decoded->read(&mtl.specular.y(), sizeof(float));
			decoded->read(&mtl.specular.z(), sizeof(float));
			decoded->read(&mtl.emit.x(), sizeof(float));
			decoded->read(&mtl.emit.y(), sizeof(float));
			decoded->read(&mtl.emit.z(), sizeof(float));
			decoded->read(&mtl.opacity, sizeof(float));
			decoded->read(&mtl.specular_level, sizeof(float));
			decoded->read(&mtl.shininess, sizeof(float));

			uint32_t num_texs;
			decoded->read(&num_texs, sizeof(num_texs));

			for (uint32_t tex_index = 0; tex_index < num_texs; ++ tex_index)
			{
				std::string type, name;
				ReadShortString(decoded, type);
				ReadShortString(decoded, name);
				mtl.texture_slots.push_back(std::make_pair(type, name));
			}
		}

		mesh_names.resize(num_meshes);
		mtl_ids.resize(num_meshes);
		ves.resize(num_meshes),
		max_num_blends.resize(num_meshes);
		buffs.resize(num_meshes);
		is_index_16_bit.resize(num_meshes);
		indices.resize(num_meshes);
		for (uint32_t mesh_index = 0; mesh_index < num_meshes; ++ mesh_index)
		{
			ReadShortString(decoded, mesh_names[mesh_index]);

			decoded->read(&mtl_ids[mesh_index], sizeof(mtl_ids[mesh_index]));

			uint32_t num_ves;
			decoded->read(&num_ves, sizeof(num_ves));
			ves[mesh_index].resize(num_ves);

			std::vector<vertex_element>& vertex_elements = ves[mesh_index];
			for (uint32_t ve_index = 0; ve_index < num_ves; ++ ve_index)
			{
				decoded->read(&vertex_elements[ve_index], sizeof(vertex_elements[ve_index]));
			}

			uint32_t num_vertices;
			decoded->read(&num_vertices, sizeof(num_vertices));

			uint32_t& max_num_blend = max_num_blends[mesh_index];
			decoded->read(&max_num_blend, sizeof(max_num_blend));

			buffs[mesh_index].resize(num_ves);
			for (uint32_t ve_index = 0; ve_index < num_ves; ++ ve_index)
			{
				std::vector<uint8_t>& buff = buffs[mesh_index][ve_index];
				vertex_element const & ve = vertex_elements[ve_index];
				switch (ve.usage)
				{
				case VEU_Position:
				case VEU_Normal:
				case VEU_Tangent:
				case VEU_Binormal:
					buff.resize(num_vertices * sizeof(float3));
					break;

				case VEU_Diffuse:
				case VEU_Specular:
					buff.resize(num_vertices * sizeof(float4));
					break;

				case VEU_BlendIndex:
					buff.resize(num_vertices * max_num_blend);
					break;

				case VEU_BlendWeight:
					buff.resize(num_vertices * max_num_blend * sizeof(float));
					break;

				case VEU_TextureCoord:
					buff.resize(num_vertices * NumComponents(ve.format) * sizeof(float));
					break;
				}

				decoded->read(&buff[0], buff.size() * sizeof(buff[0]));
			}

			uint32_t num_triangles;
			decoded->read(&num_triangles, sizeof(num_triangles));

			decoded->read(&is_index_16_bit[mesh_index], sizeof(is_index_16_bit[mesh_index]));

			indices[mesh_index].resize((is_index_16_bit[mesh_index] ? 2 : 4) * num_triangles * 3);
			decoded->read(&indices[mesh_index][0], indices[mesh_index].size() * sizeof(indices[mesh_index][0]));
		}

		joints.resize(num_joints);
		for (uint32_t joint_index = 0; joint_index < num_joints; ++ joint_index)
		{
			Joint& joint = joints[joint_index];

			ReadShortString(decoded, joint.name);
			decoded->read(&joint.parent, sizeof(joint.parent));

			decoded->read(&joint.bind_pos, sizeof(joint.bind_pos));
			decoded->read(&joint.bind_quat, sizeof(joint.bind_quat));

			joint.inverse_origin_quat = MathLib::inverse(joint.bind_quat);
			joint.inverse_origin_pos = MathLib::transform_quat(-joint.bind_pos, joint.inverse_origin_quat);
		}

		if (num_kfs > 0)
		{
			decoded->read(&start_frame, sizeof(start_frame));
			decoded->read(&end_frame, sizeof(end_frame));
			decoded->read(&frame_rate, sizeof(frame_rate));

			kfs = MakeSharedPtr<KeyFramesType>();
			for (uint32_t kf_index = 0; kf_index < num_kfs; ++ kf_index)
			{
				std::string name;
				ReadShortString(decoded, name);

				uint32_t num_kf;
				decoded->read(&num_kf, sizeof(num_kf));

				KeyFrames kf;
				kf.bind_pos.resize(num_kf);
				kf.bind_quat.resize(num_kf);
				for (uint32_t k_index = 0; k_index < num_kf; ++ k_index)
				{
					decoded->read(&kf.bind_pos[k_index], sizeof(kf.bind_pos[k_index]));
					decoded->read(&kf.bind_quat[k_index], sizeof(kf.bind_quat[k_index]));
				}

				kfs->insert(std::make_pair(name, kf));
			}
		}
	}

	boost::function<RenderModelPtr()> LoadModel(std::string const & meshml_name, uint32_t access_hint,
		boost::function<RenderModelPtr (std::wstring const &)> CreateModelFactoryFunc,
		boost::function<StaticMeshPtr (RenderModelPtr, std::wstring const &)> CreateMeshFactoryFunc)
	{
		BOOST_ASSERT(CreateModelFactoryFunc);
		BOOST_ASSERT(CreateMeshFactoryFunc);

		return RenderModelLoader(meshml_name, access_hint, CreateModelFactoryFunc, CreateMeshFactoryFunc);
	}

	void SaveModel(std::string const & meshml_name, std::vector<RenderModel::Material> const & mtls,
		std::vector<std::string> const & mesh_names, std::vector<int32_t> const & mtl_ids, std::vector<std::vector<vertex_element> > const & ves,
		std::vector<std::vector<std::vector<uint8_t> > > const & buffs,
		std::vector<char> const & is_index_16_bit, std::vector<std::vector<uint8_t> > const & indices,
		std::vector<Joint> const & joints, boost::shared_ptr<KeyFramesType> const & kfs,
		int32_t start_frame, int32_t end_frame, int32_t frame_rate)
	{
		KlayGE::XMLDocument doc;

		XMLNodePtr root = doc.AllocNode(XNT_Element, "model");
		doc.RootNode(root);
		root->AppendAttrib(doc.AllocAttribUInt("version", 4));

		if (kfs)
		{
			XMLNodePtr bones_chunk = doc.AllocNode(XNT_Element, "bones_chunk");
			root->AppendNode(bones_chunk);

			uint32_t num_joints = static_cast<uint32_t>(joints.size());
			for (uint32_t i = 0; i < num_joints; ++ i)
			{
				XMLNodePtr bone_node = doc.AllocNode(XNT_Element, "bone");
				bones_chunk->AppendNode(bone_node);

				Joint const & joint = joints[i];

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

		if (!mtls.empty())
		{
			XMLNodePtr materials_chunk = doc.AllocNode(XNT_Element, "materials_chunk");
			root->AppendNode(materials_chunk);

			for (uint32_t i = 0; i < mtls.size(); ++ i)
			{
				RenderModel::Material const & mtl = mtls[i];

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

		if (!mesh_names.empty())
		{
			XMLNodePtr meshes_chunk = doc.AllocNode(XNT_Element, "meshes_chunk");
			root->AppendNode(meshes_chunk);

			for (uint32_t mesh_index = 0; mesh_index < static_cast<uint32_t>(mesh_names.size()); ++ mesh_index)
			{
				XMLNodePtr mesh_node = doc.AllocNode(XNT_Element, "mesh");
				meshes_chunk->AppendNode(mesh_node);

				mesh_node->AppendAttrib(doc.AllocAttribString("name", mesh_names[mesh_index]));
				mesh_node->AppendAttrib(doc.AllocAttribInt("mtl_id", mtl_ids[mesh_index]));

				XMLNodePtr vertex_elements_chunk = doc.AllocNode(XNT_Element, "vertex_elements_chunk");
				mesh_node->AppendNode(vertex_elements_chunk);

				uint32_t num_vertices = 0;
				for (uint32_t j = 0; j < ves[mesh_index].size(); ++ j)
				{
					vertex_element const & ve = ves[mesh_index][j];

					XMLNodePtr ve_node = doc.AllocNode(XNT_Element, "vertex_element");
					vertex_elements_chunk->AppendNode(ve_node);

					ve_node->AppendAttrib(doc.AllocAttribUInt("usage", static_cast<uint32_t>(ve.usage)));
					ve_node->AppendAttrib(doc.AllocAttribUInt("usage_index", ve.usage_index));
					ve_node->AppendAttrib(doc.AllocAttribUInt("num_components", NumComponents(ve.format)));

					switch (ve.usage)
					{
					case VEU_Position:
					case VEU_Normal:
						num_vertices = static_cast<uint32_t>(buffs[mesh_index][j].size() / sizeof(float3));
						break;

					case VEU_Diffuse:
					case VEU_Specular:
						num_vertices = static_cast<uint32_t>(buffs[mesh_index][j].size() / sizeof(float4));
						break;

					case VEU_TextureCoord:
						num_vertices = static_cast<uint32_t>(buffs[mesh_index][j].size() / NumComponents(ve.format) / sizeof(float));
						break;

					default:
						break;
					}
				}

				XMLNodePtr vertices_chunk = doc.AllocNode(XNT_Element, "vertices_chunk");
				mesh_node->AppendNode(vertices_chunk);

				for (uint32_t j = 0; j < num_vertices; ++ j)
				{
					XMLNodePtr vertex_node = doc.AllocNode(XNT_Element, "vertex");
					vertices_chunk->AppendNode(vertex_node);

					for (size_t k = 0; k < ves[mesh_index].size(); ++ k)
					{
						vertex_element const & ve = ves[mesh_index][k];

						switch (ve.usage)
						{
						case VEU_Position:
							{
								float3 const * p = reinterpret_cast<float3 const *>(&buffs[mesh_index][k][0]);
								vertex_node->AppendAttrib(doc.AllocAttribFloat("x", p[j].x()));
								vertex_node->AppendAttrib(doc.AllocAttribFloat("y", p[j].y()));
								vertex_node->AppendAttrib(doc.AllocAttribFloat("z", p[j].z()));
							}
							break;

						case VEU_Normal:
							{
								XMLNodePtr normal_node = doc.AllocNode(XNT_Element, "normal");
								vertex_node->AppendNode(normal_node);

								float3 const * p = reinterpret_cast<float3 const *>(&buffs[mesh_index][k][0]);
								normal_node->AppendAttrib(doc.AllocAttribFloat("x", p[j].x()));
								normal_node->AppendAttrib(doc.AllocAttribFloat("y", p[j].y()));
								normal_node->AppendAttrib(doc.AllocAttribFloat("z", p[j].z()));
							}
							break;

						case VEU_Diffuse:
							{
								XMLNodePtr diffuse_node = doc.AllocNode(XNT_Element, "diffuse");
								vertex_node->AppendNode(diffuse_node);

								float4 const * p = reinterpret_cast<float4 const *>(&buffs[mesh_index][k][0]);
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

								float4 const * p = reinterpret_cast<float4 const *>(&buffs[mesh_index][k][0]);
								specular_node->AppendAttrib(doc.AllocAttribFloat("r", p[j].x()));
								specular_node->AppendAttrib(doc.AllocAttribFloat("g", p[j].y()));
								specular_node->AppendAttrib(doc.AllocAttribFloat("b", p[j].z()));
								specular_node->AppendAttrib(doc.AllocAttribFloat("b", p[j].w()));
							}
							break;

						case VEU_BlendIndex:
							{
								int weight_stream = -1;
								for (uint32_t l = 0; l < ves[mesh_index].size(); ++ l)
								{
									vertex_element const & other_ve = ves[mesh_index][l];
									if (VEU_BlendWeight == other_ve.usage)
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

									uint8_t const * bone_indices = &buffs[mesh_index][k][0];
									weight_node->AppendAttrib(doc.AllocAttribFloat("bone_index", bone_indices[j * num_components + c]));

									if (weight_stream != -1)
									{
										float const * weights = reinterpret_cast<float const *>(&buffs[mesh_index][weight_stream][0]);
										weight_node->AppendAttrib(doc.AllocAttribFloat("weight", weights[j * num_components + c]));
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

								float const * p = reinterpret_cast<float const *>(&buffs[mesh_index][k][0]);
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

								float3 const * p = reinterpret_cast<float3 const *>(&buffs[mesh_index][k][0]);
								tangent_node->AppendAttrib(doc.AllocAttribFloat("x", p[j].x()));
								tangent_node->AppendAttrib(doc.AllocAttribFloat("y", p[j].y()));
								tangent_node->AppendAttrib(doc.AllocAttribFloat("z", p[j].z()));
							}
							break;

						case VEU_Binormal:
							{
								XMLNodePtr binormal_node = doc.AllocNode(XNT_Element, "binormal");
								vertex_node->AppendNode(binormal_node);

								float3 const * p = reinterpret_cast<float3 const *>(&buffs[mesh_index][k][0]);
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
					if (is_index_16_bit[mesh_index])
					{
						size_t size = indices[mesh_index].size() / 2;
						uint16_t const * indices_16 = reinterpret_cast<uint16_t const *>(&indices[mesh_index][0]);
						for (size_t j = 0; j < size; j += 3)
						{
							XMLNodePtr triangle_node = doc.AllocNode(XNT_Element, "triangle");
							triangles_chunk->AppendNode(triangle_node);

							triangle_node->AppendAttrib(doc.AllocAttribInt("a", indices_16[j + 0]));
							triangle_node->AppendAttrib(doc.AllocAttribInt("b", indices_16[j + 1]));
							triangle_node->AppendAttrib(doc.AllocAttribInt("c", indices_16[j + 2]));
						}
					}
					else
					{
						size_t size = indices[mesh_index].size() / 4;
						uint32_t const * indices_32 = reinterpret_cast<uint32_t const *>(&indices[mesh_index][0]);
						for (size_t j = 0; j < size; j += 3)
						{
							XMLNodePtr triangle_node = doc.AllocNode(XNT_Element, "triangle");
							triangles_chunk->AppendNode(triangle_node);

							triangle_node->AppendAttrib(doc.AllocAttribInt("a", indices_32[j + 0]));
							triangle_node->AppendAttrib(doc.AllocAttribInt("b", indices_32[j + 1]));
							triangle_node->AppendAttrib(doc.AllocAttribInt("c", indices_32[j + 2]));
						}
					}
				}
			}
		}

		if (kfs)
		{
			if (!kfs->empty())
			{
				uint32_t num_key_frames = static_cast<uint32_t>(kfs->size());

				XMLNodePtr key_frames_chunk = doc.AllocNode(XNT_Element, "key_frames_chunk");
				root->AppendNode(key_frames_chunk);

				key_frames_chunk->AppendAttrib(doc.AllocAttribUInt("start_frame", start_frame));
				key_frames_chunk->AppendAttrib(doc.AllocAttribUInt("end_frame", end_frame));
				key_frames_chunk->AppendAttrib(doc.AllocAttribUInt("frame_rate", frame_rate));

				KeyFramesType::const_iterator iter = kfs->begin();

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

	void SaveModel(RenderModelPtr const & model, std::string const & meshml_name)
	{
		std::vector<RenderModel::Material> mtls(model->NumMaterials());
		if (!mtls.empty())
		{
			for (uint32_t i = 0; i < mtls.size(); ++ i)
			{
				mtls[i] = model->GetMaterial(i);
			}
		}

		std::vector<std::string> mesh_names(model->NumMeshes());
		std::vector<int32_t> mtl_ids(mesh_names.size());
		std::vector<std::vector<vertex_element> > ves(mesh_names.size());
		std::vector<std::vector<std::vector<uint8_t> > > buffs(mesh_names.size());
		std::vector<char> is_index_16_bit(mesh_names.size());
		std::vector<std::vector<uint8_t> > indices(mesh_names.size());
		if (!mesh_names.empty())
		{
			for (uint32_t mesh_index = 0; mesh_index < mesh_names.size(); ++ mesh_index)
			{
				StaticMesh const & mesh = *model->Mesh(mesh_index);

				Convert(mesh_names[mesh_index], mesh.Name());
				mtl_ids[mesh_index] = mesh.MaterialID();

				RenderLayoutPtr const & rl = mesh.GetRenderLayout();
				ves[mesh_index].resize(rl->NumVertexStreams());
				for (uint32_t j = 0; j < rl->NumVertexStreams(); ++ j)
				{
					ves[mesh_index][j] = rl->VertexStreamFormat(j)[0];
				}

				buffs[mesh_index].resize(ves[mesh_index].size());
				for (uint32_t j = 0; j < rl->NumVertexStreams(); ++ j)
				{
					GraphicsBufferPtr const & vb = rl->GetVertexStream(j);
					GraphicsBufferPtr vb_cpu = Context::Instance().RenderFactoryInstance().MakeVertexBuffer(BU_Static, EAH_CPU_Read, NULL);
					vb_cpu->Resize(vb->Size());
					vb->CopyToBuffer(*vb_cpu);

					buffs[mesh_index][j].resize(vb->Size());

					GraphicsBuffer::Mapper mapper(*vb_cpu, BA_Read_Only);
					memcpy(&buffs[mesh_index][j][0], mapper.Pointer<uint8_t>(), vb->Size());
				}

				if (EF_R16UI == rl->IndexStreamFormat())
				{
					is_index_16_bit[mesh_index] = true;
				}
				else
				{
					BOOST_ASSERT(EF_R32UI == rl->IndexStreamFormat());
					is_index_16_bit[mesh_index] = false;
				}

				indices[mesh_index].resize((is_index_16_bit[mesh_index] ? 2 : 4) * mesh.NumTriangles() * 3);
				{
					GraphicsBufferPtr ib = rl->GetIndexStream();
					GraphicsBufferPtr ib_cpu = Context::Instance().RenderFactoryInstance().MakeIndexBuffer(BU_Static, EAH_CPU_Read, NULL);
					ib_cpu->Resize(ib->Size());
					ib->CopyToBuffer(*ib_cpu);

					GraphicsBuffer::Mapper mapper(*ib_cpu, BA_Read_Only);
					memcpy(&indices[mesh_index][0], mapper.Pointer<uint8_t>(), ib->Size());
				}
			}
		}

		std::vector<Joint> joints;
		boost::shared_ptr<KeyFramesType> kfs;
		int32_t start_frame = 0;
		int32_t end_frame = 0;
		int32_t frame_rate = 0;
		if (model->IsSkinned())
		{
			SkinnedModelPtr skinned = checked_pointer_cast<SkinnedModel>(model);
			uint32_t num_joints = skinned->NumJoints();
			joints.resize(num_joints);
			for (uint32_t i = 0; i < num_joints; ++ i)
			{
				joints[i] = checked_pointer_cast<SkinnedModel>(model)->GetJoint(i);
			}

			start_frame = skinned->StartFrame();
			end_frame = skinned->EndFrame();
			frame_rate = skinned->FrameRate();

			kfs = skinned->GetKeyFrames();
		}

		SaveModel(meshml_name, mtls, mesh_names, mtl_ids, ves, buffs, is_index_16_bit, indices, joints, kfs, start_frame, end_frame, frame_rate);
	}
}
