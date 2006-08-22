// KMesh.cpp
// KlayGE KMesh类 实现文件
// Ver 2.7.1
// 版权所有(C) 龚敏敏, 2005
// Homepage: http://klayge.sourceforge.net
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
#include <KlayGE/Math.hpp>
#include <KlayGE/Texture.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/Sampler.hpp>
#include <KlayGE/App3D.hpp>

#include <boost/assert.hpp>

#include <KlayGE/KMesh.hpp>

namespace KlayGE
{
	KMesh::KMesh(std::wstring const & name, TexturePtr tex)
						: StaticMesh(name),
							sampler_(new Sampler),
							model_(float4x4::Identity())
	{
		// 载入fx
		RenderEffectPtr effect;
		if (!ResLoader::Instance().Locate("KMesh.fx").empty())
		{
			effect = Context::Instance().RenderFactoryInstance().LoadEffect("KMesh.fx");
		}
		else
		{
			effect = RenderEffect::NullObject();
		}

		if (tex)
		{
			technique_ = effect->Technique("KMeshTec");

			sampler_->SetTexture(tex);
			sampler_->Filtering(Sampler::TFO_Bilinear);
			sampler_->AddressingMode(Sampler::TAT_Addr_U, Sampler::TAM_Clamp);
			sampler_->AddressingMode(Sampler::TAT_Addr_V, Sampler::TAM_Clamp);
			*(technique_->Effect().ParameterByName("texSampler")) = sampler_;
		}
		else
		{
			technique_ = effect->Technique("KMeshNoTexTec");
		}
	}

	KMesh::~KMesh()
	{
	}

	void KMesh::OnRenderBegin()
	{
		App3DFramework const & app = Context::Instance().AppInstance();
		Camera const & camera = app.ActiveCamera();

		*(technique_->Effect().ParameterByName("modelviewproj")) = model_ * camera.ViewMatrix() * camera.ProjMatrix();
		*(technique_->Effect().ParameterByName("modelIT")) = MathLib::transpose(MathLib::inverse(model_));
	}

	void KMesh::SetModelMatrix(float4x4 const & model)
	{
		model_ = model;
	}


	RenderModelPtr LoadKModel(std::string const & kmodel_name,
		boost::function<KMeshPtr (std::wstring const &, TexturePtr)> CreateKMeshFactoryFunc)
	{
		BOOST_ASSERT(CreateKMeshFactoryFunc);

		typedef std::vector<StaticMeshPtr> MeshesType;
		MeshesType meshes;

		ResIdentifierPtr file(ResLoader::Instance().Load(kmodel_name));

		char fourcc[4];
		file->read(fourcc, sizeof(fourcc));

		uint32_t version;
		file->read(reinterpret_cast<char*>(&version), sizeof(version));

		uint8_t num_meshes;
		file->read(reinterpret_cast<char*>(&num_meshes), sizeof(num_meshes));
		for (uint8_t i = 0; i < num_meshes; ++ i)
		{
			uint8_t len;
			file->read(reinterpret_cast<char*>(&len), sizeof(len));
			std::vector<char> name(len, 0);
			file->read(&name[0], static_cast<std::streamsize>(name.size()));
			std::wstring wname;
			Convert(wname, std::string(name.begin(), name.end()));

			uint8_t num_vertex_elems;
			file->read(reinterpret_cast<char*>(&num_vertex_elems), sizeof(num_vertex_elems));

			std::vector<vertex_element> vertex_elements;
			for (uint8_t j = 0; j < num_vertex_elems; ++ j)
			{
				vertex_element ve;

				uint8_t usage;
				file->read(reinterpret_cast<char*>(&usage), 1);
				ve.usage = static_cast<VertexElementUsage>(usage);
				file->read(reinterpret_cast<char*>(&ve.usage_index), 1);
				uint8_t num_components;
				file->read(reinterpret_cast<char*>(&num_components), 1);

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
						ve.format = EF_L8;
						break;

					case 2:
						ve.format = EF_AL8;
						break;

					case 3:
						ve.format = EF_RGB8;
						break;

					case 4:
						ve.format = EF_ARGB8;
						break;
					}
				}

				vertex_elements.push_back(ve);
			}

			uint8_t num_textures;
			file->read(reinterpret_cast<char*>(&num_textures), sizeof(num_textures));

			typedef std::vector<std::pair<std::string, std::string> > TextureSlotsType;
			TextureSlotsType texture_slots(num_textures);

			for (uint8_t j = 0; j < num_textures; ++ j)
			{
				uint8_t len;
				file->read(reinterpret_cast<char*>(&len), sizeof(len));
				std::vector<char> texture_type(len, 0);
				file->read(&texture_type[0], static_cast<std::streamsize>(texture_type.size()));
				file->read(reinterpret_cast<char*>(&len), sizeof(len));
				std::vector<char> texture_name(len, 0);
				file->read(&texture_name[0], static_cast<std::streamsize>(texture_name.size()));

				texture_slots[j].first = std::string(texture_type.begin(), texture_type.end());
				texture_slots[j].second = std::string(texture_name.begin(), texture_name.end());
			}

			TexturePtr texture;
			if (!texture_slots.empty() && !texture_slots[0].second.empty()
				&& !ResLoader::Instance().Locate(texture_slots[0].second).empty())
			{
				texture = LoadTexture(texture_slots[0].second);
			}

			StaticMeshPtr mesh = CreateKMeshFactoryFunc(wname, texture);

			uint32_t num_vertices;
			file->read(reinterpret_cast<char*>(&num_vertices), sizeof(num_vertices));
			uint32_t num_tex_coords_per_ver;
			file->read(reinterpret_cast<char*>(&num_tex_coords_per_ver), sizeof(num_tex_coords_per_ver));

			for (uint8_t k = 0; k < num_vertex_elems; ++ k)
			{
				std::vector<uint8_t> buf(num_vertices * vertex_elements[k].element_size());
				file->read(reinterpret_cast<char*>(&buf[0]), static_cast<std::streamsize>(sizeof(buf[0]) * buf.size()));

				mesh->AddVertexStream(&buf[0], static_cast<uint32_t>(buf.size()), vertex_elements[k]);
			}

			uint32_t num_triangles;
			file->read(reinterpret_cast<char*>(&num_triangles), sizeof(num_triangles));
			{
				std::vector<uint16_t> indices(num_triangles * 3);
				file->read(reinterpret_cast<char*>(&indices[0]),
					static_cast<std::streamsize>(sizeof(indices[0]) * indices.size()));

				mesh->AddIndexStream(&indices[0], static_cast<uint32_t>(indices.size() * sizeof(indices[0])), EF_R16);
			}

			meshes.push_back(mesh);
		}

		RenderModelPtr ret(new RenderModel(L"KMesh"));
		ret->AssignMeshes(meshes.begin(), meshes.end());
		return ret;
	}

	SkinnedModelPtr LoadKSkinnedModel(std::string const & kmodel_name,
		boost::function<KMeshPtr (std::wstring const &, TexturePtr)> CreateKMeshFactoryFunc)
	{
		BOOST_ASSERT(CreateKMeshFactoryFunc);

		typedef std::vector<StaticMeshPtr> MeshesType;
		MeshesType meshes;

		ResIdentifierPtr file(ResLoader::Instance().Load(kmodel_name));

		char fourcc[4];
		file->read(fourcc, sizeof(fourcc));

		uint32_t version;
		file->read(reinterpret_cast<char*>(&version), sizeof(version));

		uint8_t num_meshes;
		file->read(reinterpret_cast<char*>(&num_meshes), sizeof(num_meshes));
		for (uint8_t i = 0; i < num_meshes; ++ i)
		{
			uint8_t len;
			file->read(reinterpret_cast<char*>(&len), sizeof(len));
			std::vector<char> name(len, 0);
			file->read(&name[0], static_cast<std::streamsize>(name.size()));
			std::wstring wname;
			Convert(wname, std::string(name.begin(), name.end()));

			uint8_t num_vertex_elems;
			file->read(reinterpret_cast<char*>(&num_vertex_elems), sizeof(num_vertex_elems));

			std::vector<vertex_element> vertex_elements;
			for (uint8_t j = 0; j < num_vertex_elems; ++ j)
			{
				vertex_element ve;

				uint8_t usage;
				file->read(reinterpret_cast<char*>(&usage), 1);
				ve.usage = static_cast<VertexElementUsage>(usage);
				file->read(reinterpret_cast<char*>(&ve.usage_index), 1);
				uint8_t num_components;
				file->read(reinterpret_cast<char*>(&num_components), 1);

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
						ve.format = EF_L8;
						break;

					case 2:
						ve.format = EF_AL8;
						break;

					case 3:
						ve.format = EF_RGB8;
						break;

					case 4:
						ve.format = EF_ARGB8;
						break;
					}
				}

				vertex_elements.push_back(ve);
			}

			uint8_t num_textures;
			file->read(reinterpret_cast<char*>(&num_textures), sizeof(num_textures));

			typedef std::vector<std::pair<std::string, std::string> > TextureSlotsType;
			TextureSlotsType texture_slots(num_textures);

			for (uint8_t j = 0; j < num_textures; ++ j)
			{
				uint8_t len;
				file->read(reinterpret_cast<char*>(&len), sizeof(len));
				std::vector<char> texture_type(len, 0);
				file->read(&texture_type[0], static_cast<std::streamsize>(texture_type.size()));
				file->read(reinterpret_cast<char*>(&len), sizeof(len));
				std::vector<char> texture_name(len, 0);
				file->read(&texture_name[0], static_cast<std::streamsize>(texture_name.size()));

				texture_slots[j].first = std::string(texture_type.begin(), texture_type.end());
				texture_slots[j].second = std::string(texture_name.begin(), texture_name.end());
			}

			TexturePtr texture;
			if (!texture_slots.empty() && !texture_slots[0].second.empty()
				&& !ResLoader::Instance().Locate(texture_slots[0].second).empty())
			{
				texture = LoadTexture(texture_slots[0].second);
			}

			StaticMeshPtr mesh = CreateKMeshFactoryFunc(wname, texture);

			uint32_t num_vertices;
			file->read(reinterpret_cast<char*>(&num_vertices), sizeof(num_vertices));
			uint32_t num_tex_coords_per_ver;
			file->read(reinterpret_cast<char*>(&num_tex_coords_per_ver), sizeof(num_tex_coords_per_ver));

			for (uint8_t k = 0; k < num_vertex_elems; ++ k)
			{
				std::vector<uint8_t> buf(num_vertices * vertex_elements[k].element_size());
				file->read(reinterpret_cast<char*>(&buf[0]), static_cast<std::streamsize>(sizeof(buf[0]) * buf.size()));

				mesh->AddVertexStream(&buf[0], static_cast<uint32_t>(buf.size()), vertex_elements[k]);
			}

			uint32_t num_triangles;
			file->read(reinterpret_cast<char*>(&num_triangles), sizeof(num_triangles));
			{
				std::vector<uint16_t> indices(num_triangles * 3);
				file->read(reinterpret_cast<char*>(&indices[0]),
					static_cast<std::streamsize>(sizeof(indices[0]) * indices.size()));

				mesh->AddIndexStream(&indices[0], static_cast<uint32_t>(indices.size() * sizeof(indices[0])), EF_R16);
			}

			meshes.push_back(mesh);
		}

		std::vector<Joint> joints;
		uint8_t num_joints;
		file->read(reinterpret_cast<char*>(&num_joints), sizeof(num_joints));
		for (uint8_t i = 0; i < num_joints; ++ i)
		{
			uint8_t len;
			file->read(reinterpret_cast<char*>(&len), sizeof(len));
			std::vector<char> name(len, 0);
			file->read(&name[0], static_cast<std::streamsize>(name.size()));

			Joint joint;
			joint.name = std::string(name.begin(), name.end());
			file->read(reinterpret_cast<char*>(&joint.parent), sizeof(joint.parent));
			file->read(reinterpret_cast<char*>(&joint.bind_pos), sizeof(joint.bind_pos));
			file->read(reinterpret_cast<char*>(&joint.bind_quat), sizeof(joint.bind_quat));

			float4x4 origin_mat = MathLib::to_matrix(joint.bind_quat);
			origin_mat *= MathLib::translation(joint.bind_pos);
			float4x4 inverse_origin_mat = MathLib::inverse(origin_mat);
			joint.inverse_origin_quat = MathLib::to_quaternion(inverse_origin_mat);
			joint.inverse_origin_pos = float3(inverse_origin_mat(3, 0), inverse_origin_mat(3, 1), inverse_origin_mat(3, 2));

			joints.push_back(joint);
		}

		uint8_t num_key_frames;
		file->read(reinterpret_cast<char*>(&num_key_frames), sizeof(num_key_frames));
		uint32_t start_frame;
		uint32_t end_frame;
		uint32_t frame_rate;
		file->read(reinterpret_cast<char*>(&start_frame), sizeof(start_frame));
		file->read(reinterpret_cast<char*>(&end_frame), sizeof(end_frame));
		file->read(reinterpret_cast<char*>(&frame_rate), sizeof(frame_rate));

		boost::shared_ptr<KeyFramesType> kfs(new KeyFramesType);
		for (uint8_t i = 0; i < num_key_frames; ++ i)
		{
			uint8_t len;
			file->read(reinterpret_cast<char*>(&len), sizeof(len));
			std::vector<char> name(len, 0);
			file->read(&name[0], static_cast<std::streamsize>(name.size()));

			KeyFrames kf;
			kf.bind_pos.resize(end_frame - start_frame);
			kf.bind_quat.resize(end_frame - start_frame);

			file->read(reinterpret_cast<char*>(&kf.bind_pos[0]),
				static_cast<std::streamsize>(sizeof(kf.bind_pos[0]) * kf.bind_pos.size()));
			file->read(reinterpret_cast<char*>(&kf.bind_quat[0]),
				static_cast<std::streamsize>(sizeof(kf.bind_quat[0]) * kf.bind_quat.size()));

			kfs->insert(std::make_pair(std::string(name.begin(), name.end()), kf));
		}

		SkinnedModelPtr ret(new SkinnedModel(L"KSkinnedMesh", start_frame, end_frame, frame_rate));
		ret->AssignMeshes(meshes.begin(), meshes.end());
		ret->AssignJoints(joints.begin(), joints.end());
		ret->AttachKeyFrames(kfs);

		return ret;
	}
}
