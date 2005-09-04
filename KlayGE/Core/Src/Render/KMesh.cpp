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
							sampler_(new Sampler)
	{
		// 载入fx
		effect_ = Context::Instance().RenderFactoryInstance().LoadEffect("KMesh.fx");

		if (tex)
		{
			sampler_->SetTexture(tex);
			sampler_->Filtering(Sampler::TFO_Bilinear);
			sampler_->AddressingMode(Sampler::TAT_Addr_U, Sampler::TAM_Clamp);
			sampler_->AddressingMode(Sampler::TAT_Addr_V, Sampler::TAM_Clamp);
			*(effect_->ParameterByName("texSampler")) = sampler_;

			effect_->ActiveTechnique("KMeshTec");
		}
		else
		{
			effect_->ActiveTechnique("KMeshNoTexTec");
		}
	}

	KMesh::~KMesh()
	{
	}

	void KMesh::OnRenderBegin()
	{
		App3DFramework const & app = Context::Instance().AppInstance();
		Camera const & camera = app.ActiveCamera();

		*(effect_->ParameterByName("modelviewproj")) = model_ * camera.ViewMatrix() * camera.ProjMatrix();
		*(effect_->ParameterByName("modelIT")) = MathLib::Transpose(MathLib::Inverse(model_));
	}

	StaticMeshPtr LoadKMesh(const std::string& kmeshName,
		boost::function<KMeshPtr (std::wstring const &, TexturePtr)> CreateFactoryFunc)
	{
		BOOST_ASSERT(CreateFactoryFunc);

		typedef std::vector<StaticMeshPtr> MeshesType;
		MeshesType meshes;

		ResIdentifierPtr file(ResLoader::Instance().Load(kmeshName));

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
			std::string name(len, 0);
			file->read(&name[0], static_cast<std::streamsize>(name.size()));
			std::wstring wname;
			Convert(wname, name);

			uint8_t num_textures;
			file->read(reinterpret_cast<char*>(&num_textures), sizeof(num_textures));

			typedef std::vector<std::pair<std::string, std::string> > TextureSlotsType;
			TextureSlotsType texture_slots(num_textures);

			for (uint8_t j = 0; j < num_textures; ++ j)
			{
				std::string& texture_type = texture_slots[j].first;
				std::string& texture_name = texture_slots[j].second;

				uint8_t len;
				file->read(reinterpret_cast<char*>(&len), sizeof(len));
				texture_type.resize(len, 0);
				file->read(&texture_type[0], static_cast<std::streamsize>(texture_type.size()));
				file->read(reinterpret_cast<char*>(&len), sizeof(len));
				texture_name.resize(len, 0);
				file->read(&texture_name[0], static_cast<std::streamsize>(texture_name.size()));
			}

			TexturePtr texture;
			if (!texture_slots.empty() && !texture_slots[0].second.empty())
			{
				texture = LoadTexture(texture_slots[0].second);
			}

			StaticMeshPtr mesh = CreateFactoryFunc(wname, texture);

			uint32_t num_vertices;
			file->read(reinterpret_cast<char*>(&num_vertices), sizeof(num_vertices));
			uint32_t num_tex_coords_per_ver;
			file->read(reinterpret_cast<char*>(&num_tex_coords_per_ver), sizeof(num_tex_coords_per_ver));

			StaticMesh::XYZsType positions(num_vertices);
			StaticMesh::MultiTexCoordsType multi_tex_coords(num_tex_coords_per_ver);
			for (uint32_t k = 0; k < num_tex_coords_per_ver; ++ k)
			{
				multi_tex_coords[k].resize(num_vertices);
			}

			for (uint32_t j = 0; j < num_vertices; ++ j)
			{
				file->read(reinterpret_cast<char*>(&positions[j]), sizeof(positions[j]));

				for (uint32_t k = 0; k < num_tex_coords_per_ver; ++ k)
				{
					file->read(reinterpret_cast<char*>(&multi_tex_coords[k][j]), sizeof(multi_tex_coords[k][j]));
				}
			}

			uint32_t num_triangles;
			file->read(reinterpret_cast<char*>(&num_triangles), sizeof(num_triangles));
			StaticMesh::IndicesType indices(num_triangles * 3);
			file->read(reinterpret_cast<char*>(&indices[0]),
				static_cast<std::streamsize>(sizeof(indices[0]) * indices.size()));

			mesh->AssignXYZs(positions.begin(), positions.end());
			mesh->AssignMultiTexs(multi_tex_coords.begin(), multi_tex_coords.end());
			mesh->AssignIndices(indices.begin(), indices.end());

			mesh->ComputeNormal();

			meshes.push_back(mesh);
		}

		StaticMeshPtr ret(new StaticMesh(L"KMesh"));
		ret->AssignChildren(meshes.begin(), meshes.end());
		return ret;
	}
}
