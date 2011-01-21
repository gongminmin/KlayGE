// Mesh.cpp
// KlayGE Mesh类 实现文件
// Ver 3.12.0
// 版权所有(C) 龚敏敏, 2004-2011
// Homepage: http://www.klayge.org
//
// 3.4.0
// 增加了AddVertexStream和AddIndexStream (2006.8.21)
//
// 3.2.0
// 增加了SkinnedModel和SkinnedMesh (2006.4.23)
//
// 2.7.0
// 改进了StaticMesh (2005.6.16)
//
// 2.1.2
// 初次建立 (2004.5.26)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/Texture.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/App3D.hpp>
#include <KlayGE/Camera.hpp>
#include <KlayGE/XMLDom.hpp>
#include <KlayGE/LZMACodec.hpp>
#include <KlayGE/thread.hpp>
#include <KlayGE/Light.hpp>

#include <algorithm>
#include <fstream>
#include <sstream>
#include <boost/tuple/tuple.hpp>
#include <boost/typeof/typeof.hpp>
#include <boost/foreach.hpp>

#include <KlayGE/Mesh.hpp>

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
				model_name = L"SkinnedMesh";
			}
			else
			{
				model_name = L"Mesh";
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
	RenderModel::RenderModel(std::wstring const & name)
		: name_(name)
	{
	}

	void RenderModel::AddToRenderQueue()
	{
		BOOST_FOREACH(BOOST_TYPEOF(meshes_)::reference mesh, meshes_)
		{
			mesh->AddToRenderQueue();
		}
	}

	void RenderModel::OnRenderBegin()
	{
		BOOST_FOREACH(BOOST_TYPEOF(meshes_)::reference mesh, meshes_)
		{
			mesh->OnRenderBegin();
		}
	}

	void RenderModel::OnRenderEnd()
	{
		BOOST_FOREACH(BOOST_TYPEOF(meshes_)::reference mesh, meshes_)
		{
			mesh->OnRenderEnd();
		}
	}

	void RenderModel::UpdateBoundBox()
	{
		box_ = Box(float3(0, 0, 0), float3(0, 0, 0));
		BOOST_FOREACH(BOOST_TYPEOF(meshes_)::const_reference mesh, meshes_)
		{
			box_ |= mesh->GetBound();
		}
	}


	StaticMesh::StaticMesh(RenderModelPtr const & model, std::wstring const & name)
		: name_(name), model_(model)
	{
		rl_ = Context::Instance().RenderFactoryInstance().MakeRenderLayout();
		rl_->TopologyType(RenderLayout::TT_TriangleList);
	}

	StaticMesh::~StaticMesh()
	{
	}

	std::wstring const & StaticMesh::Name() const
	{
		return name_;
	}

	Box const & StaticMesh::GetBound() const
	{
		return box_;
	}

	void StaticMesh::AddVertexStream(void const * buf, uint32_t size, vertex_element const & ve, uint32_t access_hint)
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();

		ElementInitData init_data;
		init_data.data = buf;
		init_data.row_pitch = size;
		init_data.slice_pitch = 0;
		GraphicsBufferPtr vb = rf.MakeVertexBuffer(BU_Static, access_hint, &init_data);
		rl_->BindVertexStream(vb, boost::make_tuple(ve));

		if (VEU_Position == ve.usage)
		{
			switch (ve.format)
			{
			case EF_BGR32F:
				box_ = MathLib::compute_bounding_box<float>(static_cast<float3 const *>(buf),
					static_cast<float3 const *>(buf) + size / sizeof(float3));
				break;

			case EF_ABGR32F:
				box_ = MathLib::compute_bounding_box<float>(static_cast<float4 const *>(buf),
					static_cast<float4 const *>(buf) + size / sizeof(float4));
				break;

			default:
				BOOST_ASSERT(false);
				break;
			}
		}
	}

	void StaticMesh::AddIndexStream(void const * buf, uint32_t size, ElementFormat format, uint32_t access_hint)
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();

		ElementInitData init_data;
		init_data.data = buf;
		init_data.row_pitch = size;
		init_data.slice_pitch = 0;
		GraphicsBufferPtr ib = rf.MakeIndexBuffer(BU_Static, access_hint, &init_data);
		rl_->BindIndexStream(ib, format);
	}


	float3 KeyFrames::FramePos(float frame) const
	{
		frame = std::fmod(frame, static_cast<float>(bind_pos.size()));
		int frame0 = static_cast<int>(frame);
		int frame1 = (frame0 + 1) % bind_pos.size();
		return MathLib::lerp(bind_pos[frame0], bind_pos[frame1], frame - frame0);
	}

	Quaternion KeyFrames::FrameQuat(float frame) const
	{
		frame = std::fmod(frame, static_cast<float>(bind_quat.size()));
		int frame0 = static_cast<int>(frame);
		int frame1 = (frame0 + 1) % bind_quat.size();
		return MathLib::slerp(bind_quat[frame0], bind_quat[frame1], frame - frame0);
	}


	SkinnedModel::SkinnedModel(std::wstring const & name)
		: RenderModel(name),
			last_frame_(-1),
			start_frame_(0), end_frame_(1), frame_rate_(0)
	{
	}
	
	void SkinnedModel::BuildBones(float frame)
	{
		BOOST_FOREACH(BOOST_TYPEOF(joints_)::reference joint, joints_)
		{
			KeyFrames const & kf = key_frames_->find(joint.name)->second;
			float3 const & key_pos = kf.FramePos(frame);
			Quaternion const & key_quat = kf.FrameQuat(frame);

			if (joint.parent != -1)
			{
				Joint const & parent(joints_[joint.parent]);

				joint.bind_quat = key_quat * parent.bind_quat;
				joint.bind_pos = MathLib::transform_quat(key_pos, parent.bind_quat) + parent.bind_pos;
			}
			else
			{
				joint.bind_quat = key_quat;
				joint.bind_pos = key_pos;
			}
		}

		this->UpdateBinds();
	}

	void SkinnedModel::UpdateBinds()
	{
		bind_rots_.resize(joints_.size());
		bind_poss_.resize(joints_.size());
		for (size_t i = 0; i < joints_.size(); ++ i)
		{
			Quaternion quat = joints_[i].inverse_origin_quat * joints_[i].bind_quat;
			float3 pos = MathLib::transform_quat(joints_[i].inverse_origin_pos, joints_[i].bind_quat) + joints_[i].bind_pos;
			bind_rots_[i].x() = quat.x();
			bind_rots_[i].y() = quat.y();
			bind_rots_[i].z() = quat.z();
			bind_rots_[i].w() = quat.w();
			bind_poss_[i].x() = pos.x();
			bind_poss_[i].y() = pos.y();
			bind_poss_[i].z() = pos.z();
			bind_poss_[i].w() = 1;
		}
	}

	void SkinnedModel::AttachKeyFrames(boost::shared_ptr<KlayGE::KeyFramesType> const & key_frames)
	{
		key_frames_ = key_frames;
	}

	float SkinnedModel::GetFrame() const
	{
		return last_frame_;
	}

	void SkinnedModel::SetFrame(float frame)
	{
		if (last_frame_ != frame)
		{
			last_frame_ = frame;

			this->BuildBones(frame);
		}
	}

	void SkinnedModel::RebindJoints()
	{
		this->BuildBones(last_frame_);
	}

	void SkinnedModel::UnbindJoints()
	{
		for (size_t i = 0; i < bind_rots_.size(); ++ i)
		{
			bind_rots_[i] = float4(0, 0, 0, 1);
			bind_poss_[i] = float4(0, 0, 0, 1);
		}
	}


	SkinnedMesh::SkinnedMesh(RenderModelPtr const & model, std::wstring const & name)
		: StaticMesh(model, name)
	{
	}


	std::string const jit_ext_name = ".model_bin";

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
		std::string::size_type const pkt_offset(meshml_name.find("//"));
		std::string path_name;
		if (pkt_offset != std::string::npos)
		{
			std::string pkt_name = meshml_name.substr(0, pkt_offset);
			std::string::size_type const password_offset = pkt_name.find("|");
			if (password_offset != std::string::npos)
			{
				pkt_name = pkt_name.substr(0, password_offset - 1);
			}

			std::string::size_type offset = pkt_name.rfind("/");
			if (offset != std::string::npos)
			{
				path_name = pkt_name.substr(0, offset + 1);
			}

			std::string const file_name = meshml_name.substr(pkt_offset + 2);
			path_name += file_name;
		}
		else
		{
			path_name = meshml_name;
		}

		bool jit = false;
		if (ResLoader::Instance().Locate(path_name + jit_ext_name).empty())
		{
			jit = true;
		}
		else
		{
			ResIdentifierPtr lzma_file = ResLoader::Instance().Load(path_name + jit_ext_name);
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

					{
						XMLNodePtr vertices_chunk = mesh_node->FirstNode("vertices_chunk");

						bool has_position = false;
						bool has_normal = false;
						bool has_diffuse = false;
						bool has_specular = false;
						bool has_weight = false;
						std::vector<uint32_t> max_num_tc_components;
						bool has_tangent = false;
						bool has_binormal = false;

						uint32_t num_vertices = 0;
						uint32_t max_num_blend = 0;
						for (XMLNodePtr vertex_node = vertices_chunk->FirstNode("vertex"); vertex_node; vertex_node = vertex_node->NextSibling("vertex"))
						{
							++ num_vertices;

							has_position = true;
						
							XMLNodePtr normal_node = vertex_node->FirstNode("normal");
							if (normal_node)
							{
								has_normal = true;
							}

							XMLNodePtr diffuse_node = vertex_node->FirstNode("diffuse");
							if (diffuse_node)
							{
								has_diffuse = true;
							}

							XMLNodePtr specular_node = vertex_node->FirstNode("specular");
							if (specular_node)
							{
								has_specular = true;
							}

							uint32_t num_blend = 0;
							for (XMLNodePtr weight_node = vertex_node->FirstNode("weight"); weight_node; weight_node = weight_node->NextSibling("weight"))
							{
								has_weight = true;
								++ num_blend;
							}
							max_num_blend = std::max(max_num_blend, num_blend);

							uint32_t num_tex_coord = 0;
							for (XMLNodePtr tex_coord_node = vertex_node->FirstNode("tex_coord"); tex_coord_node; tex_coord_node = tex_coord_node->NextSibling("tex_coord"))
							{
								++ num_tex_coord;

								if (num_tex_coord >= max_num_tc_components.size())
								{
									max_num_tc_components.resize(num_tex_coord, 0);
								}

								uint32_t num_components = 0;
								if (tex_coord_node->Attrib("u"))
								{
									num_components = 1;
								}
								if (tex_coord_node->Attrib("v"))
								{
									num_components = 2;
								}
								if (tex_coord_node->Attrib("w"))
								{
									num_components = 3;
								}
								max_num_tc_components[num_tex_coord - 1] = std::max(max_num_tc_components[num_tex_coord - 1], num_components);
							}

							XMLNodePtr tangent_node = vertex_node->FirstNode("tangent");
							if (tangent_node)
							{
								has_tangent = true;
							}

							XMLNodePtr binormal_node = vertex_node->FirstNode("binormal");
							if (binormal_node)
							{
								has_binormal = true;
							}
						}

						std::vector<float3> position_buf;
						std::vector<float3> normal_buf;
						std::vector<float4> diffuse_buf;
						std::vector<float4> specular_buf;
						std::vector<float> blend_weight_buf;
						std::vector<uint8_t> blend_index_buf;
						std::vector<std::vector<float> > tex_coord_buf;
						std::vector<float3> tangent_buf;
						std::vector<float3> binormal_buf;

						std::vector<vertex_element> vertex_elements;
						{
							vertex_element ve;

							{
								BOOST_ASSERT(has_position);

								ve.usage = VEU_Position;
								ve.usage_index = 0;
								ve.format = EF_BGR32F;
								vertex_elements.push_back(ve);

								position_buf.resize(num_vertices, float3(0, 0, 0));
							}

							if (has_normal)
							{
								ve.usage = VEU_Normal;
								ve.usage_index = 0;
								ve.format = EF_BGR32F;
								vertex_elements.push_back(ve);

								normal_buf.resize(num_vertices, float3(0, 0, 0));
							}

							if (has_diffuse)
							{
								ve.usage = VEU_Diffuse;
								ve.usage_index = 0;
								ve.format = EF_ABGR32F;
								vertex_elements.push_back(ve);

								diffuse_buf.resize(num_vertices, float4(0, 0, 0, 0));
							}

							if (has_specular)
							{
								ve.usage = VEU_Specular;
								ve.usage_index = 0;
								ve.format = EF_ABGR32F;
								vertex_elements.push_back(ve);

								specular_buf.resize(num_vertices, float4(0, 0, 0, 0));
							}

							if (has_weight)
							{
								ve.usage = VEU_BlendWeight;
								ve.usage_index = 0;
								switch (max_num_blend)
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

								default:
									ve.format = EF_ABGR32F;
									break;
								}
								vertex_elements.push_back(ve);

								ve.usage = VEU_BlendIndex;
								ve.usage_index = 0;
								switch (max_num_blend)
								{
								case 1:
									ve.format = EF_R8UI;
									break;

								case 2:
									ve.format = EF_GR8UI;
									break;

								case 3:
									ve.format = EF_BGR8UI;
									break;

								default:
									ve.format = EF_ABGR8UI;
									break;
								}
								vertex_elements.push_back(ve);

								blend_weight_buf.resize(num_vertices * max_num_blend, 0);
								blend_index_buf.resize(num_vertices * max_num_blend, 0);
							}

							tex_coord_buf.resize(max_num_tc_components.size());
							for (uint32_t usage = 0; usage < max_num_tc_components.size(); ++ usage)
							{
								ve.usage = VEU_TextureCoord;
								ve.usage_index = static_cast<uint8_t>(usage);
								switch (max_num_tc_components[usage])
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

								default:
									ve.format = EF_ABGR32F;
									break;
								}
								vertex_elements.push_back(ve);

								tex_coord_buf.back().resize(num_vertices * max_num_tc_components[usage], 0);
							}

							if (has_tangent)
							{
								ve.usage = VEU_Tangent;
								ve.usage_index = 0;
								ve.format = EF_BGR32F;
								vertex_elements.push_back(ve);

								tangent_buf.resize(num_vertices, float3(0, 0, 0));
							}

							if (has_binormal)
							{
								ve.usage = VEU_Binormal;
								ve.usage_index = 0;
								ve.format = EF_BGR32F;
								vertex_elements.push_back(ve);

								binormal_buf.resize(num_vertices, float3(0, 0, 0));
							}
						}

						uint32_t index = 0;
						for (XMLNodePtr vertex_node = vertices_chunk->FirstNode("vertex"); vertex_node; vertex_node = vertex_node->NextSibling("vertex"))
						{
							position_buf[index] = float3(vertex_node->Attrib("x")->ValueFloat(),
								vertex_node->Attrib("y")->ValueFloat(), vertex_node->Attrib("z")->ValueFloat());

							XMLNodePtr normal_node = vertex_node->FirstNode("normal");
							if (normal_node)
							{
								normal_buf[index] = float3(normal_node->Attrib("x")->ValueFloat(),
									normal_node->Attrib("y")->ValueFloat(), normal_node->Attrib("z")->ValueFloat());
							}

							XMLNodePtr diffuse_node = vertex_node->FirstNode("diffuse");
							if (diffuse_node)
							{
								diffuse_buf[index] = float4(diffuse_node->Attrib("r")->ValueFloat(), diffuse_node->Attrib("g")->ValueFloat(),
									diffuse_node->Attrib("b")->ValueFloat(), diffuse_node->Attrib("a")->ValueFloat());
							}

							XMLNodePtr specular_node = vertex_node->FirstNode("diffuse");
							if (specular_node)
							{
								specular_buf[index] = float4(specular_node->Attrib("r")->ValueFloat(), specular_node->Attrib("g")->ValueFloat(),
									specular_node->Attrib("b")->ValueFloat(), specular_node->Attrib("a")->ValueFloat());
							}

							uint32_t num_blend = 0;
							for (XMLNodePtr weight_node = vertex_node->FirstNode("weight"); weight_node; weight_node = weight_node->NextSibling("weight"))
							{
								blend_index_buf[index * max_num_blend + num_blend] = static_cast<uint8_t>(weight_node->Attrib("bone_index")->ValueUInt());
								blend_weight_buf[index * max_num_blend + num_blend] = weight_node->Attrib("weight")->ValueFloat();
								++ num_blend;
							}

							uint32_t usage = 0;
							for (XMLNodePtr tex_coord_node = vertex_node->FirstNode("tex_coord"); tex_coord_node; tex_coord_node = tex_coord_node->NextSibling("tex_coord"))
							{
								XMLAttributePtr attr = tex_coord_node->Attrib("u");
								if (attr)
								{
									tex_coord_buf[usage][index * max_num_tc_components[usage] + 0] = attr->ValueFloat();
								}
								attr = tex_coord_node->Attrib("v");
								if (attr)
								{
									tex_coord_buf[usage][index * max_num_tc_components[usage] + 1] = attr->ValueFloat();
								}
								attr = tex_coord_node->Attrib("w");
								if (attr)
								{
									tex_coord_buf[usage][index * max_num_tc_components[usage] + 2] = attr->ValueFloat();
								}

								++ usage;
							}

							XMLNodePtr tangent_node = vertex_node->FirstNode("tangent");
							if (tangent_node)
							{
								tangent_buf[index] = float3(tangent_node->Attrib("x")->ValueFloat(),
									tangent_node->Attrib("y")->ValueFloat(), tangent_node->Attrib("z")->ValueFloat());
							}

							XMLNodePtr binormal_node = vertex_node->FirstNode("binormal");
							if (binormal_node)
							{
								binormal_buf[index] = float3(binormal_node->Attrib("x")->ValueFloat(),
									binormal_node->Attrib("y")->ValueFloat(), binormal_node->Attrib("z")->ValueFloat());
							}

							++ index;
						}

						uint32_t num_ves = static_cast<uint32_t>(vertex_elements.size());
						ss->write(reinterpret_cast<char*>(&num_ves), sizeof(num_ves));
						for (size_t i = 0; i < vertex_elements.size(); ++ i)
						{
							ss->write(reinterpret_cast<char*>(&vertex_elements[i]), sizeof(vertex_elements[i]));
						}

						ss->write(reinterpret_cast<char*>(&num_vertices), sizeof(num_vertices));
						ss->write(reinterpret_cast<char*>(&max_num_blend), sizeof(max_num_blend));

						ss->write(reinterpret_cast<char*>(&position_buf[0]), position_buf.size() * sizeof(position_buf[0]));
						if (!normal_buf.empty())
						{
							ss->write(reinterpret_cast<char*>(&normal_buf[0]), normal_buf.size() * sizeof(normal_buf[0]));
						}
						if (!diffuse_buf.empty())
						{
							ss->write(reinterpret_cast<char*>(&diffuse_buf[0]), diffuse_buf.size() * sizeof(diffuse_buf[0]));
						}
						if (!specular_buf.empty())
						{
							ss->write(reinterpret_cast<char*>(&specular_buf[0]), specular_buf.size() * sizeof(specular_buf[0]));
						}
						if (!blend_weight_buf.empty())
						{
							ss->write(reinterpret_cast<char*>(&blend_weight_buf[0]), blend_weight_buf.size() * sizeof(blend_weight_buf[0]));
						}
						if (!blend_index_buf.empty())
						{
							ss->write(reinterpret_cast<char*>(&blend_index_buf[0]), blend_index_buf.size() * sizeof(blend_index_buf[0]));
						}
						for (size_t i = 0; i < tex_coord_buf.size(); ++ i)
						{
							ss->write(reinterpret_cast<char*>(&tex_coord_buf[i][0]), tex_coord_buf[i].size() * sizeof(tex_coord_buf[i][0]));
						}
						if (!tangent_buf.empty())
						{
							ss->write(reinterpret_cast<char*>(&tangent_buf[0]), tangent_buf.size() * sizeof(tangent_buf[0]));
						}
						if (!binormal_buf.empty())
						{
							ss->write(reinterpret_cast<char*>(&binormal_buf[0]), binormal_buf.size() * sizeof(binormal_buf[0]));
						}
					}

					{
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

			std::ofstream ofs((path_name + jit_ext_name).c_str(), std::ios_base::binary);
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

			std::string no_packing_name;
			size_t offset = full_meshml_name.rfind("//");
			if (offset != std::string::npos)
			{
				no_packing_name = full_meshml_name.substr(offset + 2);
			}
			else
			{
				no_packing_name = full_meshml_name;
			}
			lzma_file = ResLoader::Instance().Load(no_packing_name + jit_ext_name);
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


	RenderableLightSourceProxy::RenderableLightSourceProxy(RenderModelPtr const & model, std::wstring const & name)
			: StaticMesh(model, name)
	{
		this->Technique(Context::Instance().RenderFactoryInstance().LoadEffect("LightSourceProxy.fxml")->TechniqueByName("LightSourceProxy"));
	}

	void RenderableLightSourceProxy::Technique(RenderTechniquePtr const & tech)
	{
		technique_ = tech;
		if (tech)
		{
			mvp_param_ = technique_->Effect().ParameterByName("mvp");

			light_color_param_ = technique_->Effect().ParameterByName("light_color");
			light_falloff_param_ = technique_->Effect().ParameterByName("light_falloff");
			light_projective_tex_param_ = technique_->Effect().ParameterByName("light_projective_tex");
		}
	}

	void RenderableLightSourceProxy::SetModelMatrix(float4x4 const & mat)
	{
		model_ = mat;
	}

	void RenderableLightSourceProxy::Update()
	{
		if (light_->ProjectiveTexture())
		{
			this->Technique(technique_->Effect().TechniqueByName("LightSourceProxyProjective"));
		}
		else
		{
			this->Technique(technique_->Effect().TechniqueByName("LightSourceProxy"));
		}

		if (light_color_param_)
		{
			*light_color_param_ = light_->Color();
		}
		if (light_falloff_param_)
		{
			*light_falloff_param_ = light_->Falloff();
		}
		if (light_projective_tex_param_)
		{
			*light_projective_tex_param_ = light_->ProjectiveTexture();
		}
	}

	void RenderableLightSourceProxy::OnRenderBegin()
	{
		Camera const & camera = Context::Instance().AppInstance().ActiveCamera();

		float4x4 const & view = camera.ViewMatrix();
		float4x4 const & proj = camera.ProjMatrix();

		float4x4 mv = model_ * view;
		*mvp_param_ = mv * proj;
	}

	void RenderableLightSourceProxy::AttachLightSrc(LightSourcePtr const & light)
	{
		light_ = light;
	}
}
