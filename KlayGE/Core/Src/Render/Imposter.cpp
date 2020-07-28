/**
 * @file Imposter.cpp
 * @author Minmin Gong
 *
 * @section DESCRIPTION
 *
 * This source file is part of KlayGE
 * For the latest info, see http://www.klayge.org
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * You may alternatively use this source under the terms of
 * the KlayGE Proprietary License (KPL). You can obtained such a license
 * from http://www.klayge.org/licensing/.
 */

#include <KlayGE/KlayGE.hpp>
#include <KFL/XMLDom.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/Texture.hpp>
#include <KFL/Hash.hpp>

#include <KlayGE/Imposter.hpp>

namespace KlayGE
{
	class ImposterLoadingDesc : public ResLoadingDesc
	{
	private:
		struct ImposterDesc
		{
			std::string res_name;

			struct ImposterData
			{
				uint32_t num_azimuth;
				uint32_t num_elevation;
				uint32_t size;

				std::string rt0_name;
				std::string rt1_name;
			};
			std::shared_ptr<ImposterData> imposter_data;

			std::shared_ptr<ImposterPtr> imposter;
		};

	public:
		explicit ImposterLoadingDesc(std::string_view res_name)
		{
			imposter_desc_.res_name = std::string(res_name);
			imposter_desc_.imposter_data = MakeSharedPtr<ImposterDesc::ImposterData>();
			imposter_desc_.imposter = MakeSharedPtr<ImposterPtr>();
		}

		uint64_t Type() const override
		{
			static uint64_t const type = CT_HASH("ImposterLoadingDesc");
			return type;
		}

		bool StateLess() const override
		{
			return true;
		}

		void SubThreadStage() override
		{
			if (*imposter_desc_.imposter)
			{
				return;
			}

			ResIdentifierPtr impml_input = ResLoader::Instance().Open(imposter_desc_.res_name);

			KlayGE::XMLDocument doc;
			XMLNodePtr root = doc.Parse(*impml_input);

			XMLNodePtr azimuth_node = root->FirstNode("azimuth");
			imposter_desc_.imposter_data->num_azimuth = azimuth_node->Attrib("value")->ValueUInt();

			XMLNodePtr elevation_node = root->FirstNode("elevation");
			imposter_desc_.imposter_data->num_elevation = elevation_node->Attrib("value")->ValueUInt();

			XMLNodePtr size_node = root->FirstNode("size");
			imposter_desc_.imposter_data->size = size_node->Attrib("value")->ValueUInt();

			XMLNodePtr rt0_node = root->FirstNode("rt0");
			imposter_desc_.imposter_data->rt0_name = std::string(rt0_node->Attrib("name")->ValueString());

			XMLNodePtr rt1_node = root->FirstNode("rt1");
			imposter_desc_.imposter_data->rt1_name = std::string(rt1_node->Attrib("name")->ValueString());
		}

		void MainThreadStage() override
		{
			if (!*imposter_desc_.imposter)
			{
				TexturePtr rt0_tex = ASyncLoadTexture(imposter_desc_.imposter_data->rt0_name, EAH_GPU_Read | EAH_Immutable);
				TexturePtr rt1_tex = ASyncLoadTexture(imposter_desc_.imposter_data->rt1_name, EAH_GPU_Read | EAH_Immutable);

				*imposter_desc_.imposter = MakeSharedPtr<Imposter>(
					imposter_desc_.imposter_data->num_azimuth, imposter_desc_.imposter_data->num_elevation,
					imposter_desc_.imposter_data->size, rt0_tex, rt1_tex);
			}
		}

		bool HasSubThreadStage() const override
		{
			return true;
		}

		bool Match(ResLoadingDesc const & rhs) const override
		{
			if (this->Type() == rhs.Type())
			{
				ImposterLoadingDesc const & ild = static_cast<ImposterLoadingDesc const &>(rhs);
				return (imposter_desc_.res_name == ild.imposter_desc_.res_name);
			}
			return false;
		}

		void CopyDataFrom(ResLoadingDesc const & rhs) override
		{
			BOOST_ASSERT(this->Type() == rhs.Type());

			ImposterLoadingDesc const & ild = static_cast<ImposterLoadingDesc const &>(rhs);
			imposter_desc_.res_name = ild.imposter_desc_.res_name;
			imposter_desc_.imposter_data = ild.imposter_desc_.imposter_data;
			imposter_desc_.imposter = ild.imposter_desc_.imposter;
		}

		std::shared_ptr<void> CloneResourceFrom(std::shared_ptr<void> const & resource) override
		{
			return resource;
		}

		std::shared_ptr<void> Resource() const override
		{
			return *imposter_desc_.imposter;
		}

	private:
		ImposterDesc imposter_desc_;
	};

	ImposterPtr SyncLoadImposter(std::string_view tex_name)
	{
		return ResLoader::Instance().SyncQueryT<Imposter>(MakeSharedPtr<ImposterLoadingDesc>(tex_name));
	}

	ImposterPtr ASyncLoadImposter(std::string_view tex_name)
	{
		return ResLoader::Instance().ASyncQueryT<Imposter>(MakeSharedPtr<ImposterLoadingDesc>(tex_name));
	}


	Imposter::Imposter(uint32_t num_azimuth, uint32_t num_elevation, uint32_t size, TexturePtr const & rt0_tex, TexturePtr const & rt1_tex)
		: num_azimuth_(num_azimuth), num_elevation_(num_elevation), rt0_tex_(rt0_tex), rt1_tex_(rt1_tex)
	{
		BOOST_ASSERT(rt0_tex_->Width(0) == rt1_tex_->Width(0));
		BOOST_ASSERT(rt0_tex_->Height(0) == rt1_tex_->Height(0));

		azimuth_angle_step_ = PI * 2 / num_azimuth;
		elevation_angle_step_ = PI * 2 / num_elevation;

		size_ = float2(static_cast<float>(size) / rt0_tex_->Width(0), static_cast<float>(size) / rt0_tex_->Height(0));
	}

	float2 Imposter::StartTexCoord(float azimuth, float elevation) const
	{
		float x = floor(MathLib::mod(azimuth, PI * 2) / azimuth_angle_step_ + 0.5f);
		float y = floor(MathLib::mod(elevation, PI * 2) / elevation_angle_step_ + 0.5f);
		return float2(x, y) * size_;
	}

	float2 Imposter::StartTexCoord(float3 const & dir) const
	{
		float3 const dir_norm = MathLib::normalize(dir);
		float elevation = asin(dir_norm.y());
		float azimuth = asin(dir_norm.x() / cos(elevation));
		return this->StartTexCoord(azimuth, elevation);
	}

	float2 Imposter::ImposterSize() const
	{
		return size_;
	}
}
