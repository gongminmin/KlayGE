/**
 * @file RenderMaterial.cpp
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

#include <KFL/CXX20/format.hpp>
#include <KFL/ErrorHandling.hpp>
#include <KFL/StringUtil.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KFL/XMLDom.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KFL/Hash.hpp>
#include <KFL/CXX17/filesystem.hpp>

#include <fstream>
#include <string>

#include <KlayGE/RenderMaterial.hpp>

namespace
{
	using namespace KlayGE;

	template <int N>
	void ExtractFVector(std::string_view value_str, float* v)
	{
		std::vector<std::string_view> strs = StringUtil::Split(value_str, StringUtil::EqualTo(' '));
		for (size_t i = 0; i < N; ++ i)
		{
			if (i < strs.size())
			{
				strs[i] = StringUtil::Trim(strs[i]);
				v[i] = stof(std::string(strs[i]));
			}
			else
			{
				v[i] = 0;
			}
		}
	}

	class RenderMaterialLoadingDesc : public ResLoadingDesc
	{
	private:
		struct RenderMaterialDesc
		{
			std::string res_name;

			struct RenderMaterialData
			{
				std::string name;

				float4 albedo;
				float metalness;
				float glossiness;
				float3 emissive;

				bool transparent;
				float alpha_test;
				bool sss;
				bool two_sided;

				float normal_scale;
				float occlusion_strength;

				std::array<std::string, RenderMaterial::TS_NumTextureSlots> tex_names;

				RenderMaterial::SurfaceDetailMode detail_mode;
				float2 height_offset_scale;
				float4 tess_factors;
			};
			std::shared_ptr<RenderMaterialData> mtl_data;

			std::shared_ptr<RenderMaterialPtr> mtl;
		};

	public:
		explicit RenderMaterialLoadingDesc(std::string_view res_name)
		{
			mtl_desc_.res_name = std::string(res_name);
			mtl_desc_.mtl_data = MakeSharedPtr<RenderMaterialDesc::RenderMaterialData>();
			mtl_desc_.mtl = MakeSharedPtr<RenderMaterialPtr>();
		}

		uint64_t Type() const override
		{
			static uint64_t const type = CT_HASH("RenderMaterialLoadingDesc");
			return type;
		}

		bool StateLess() const override
		{
			return true;
		}

		void SubThreadStage() override
		{
			std::lock_guard<std::mutex> lock(main_thread_stage_mutex_);

			if (*mtl_desc_.mtl)
			{
				return;
			}

			ResIdentifierPtr mtl_input = ResLoader::Instance().Open(mtl_desc_.res_name);

			KlayGE::XMLDocument doc;
			XMLNodePtr root = doc.Parse(*mtl_input);

			{
				XMLAttributePtr attr = root->Attrib("name");
				if (attr)
				{
					mtl_desc_.mtl_data->name = std::string(attr->ValueString());
				}
				else
				{
					FILESYSTEM_NS::path res_path(mtl_desc_.res_name);
					mtl_desc_.mtl_data->name = res_path.stem().string();
				}
			}

			mtl_desc_.mtl_data->albedo = float4(0, 0, 0, 1);
			mtl_desc_.mtl_data->metalness = 0;
			mtl_desc_.mtl_data->glossiness = 0;
			mtl_desc_.mtl_data->emissive = float3(0, 0, 0);
			mtl_desc_.mtl_data->transparent = false;
			mtl_desc_.mtl_data->alpha_test = 0;
			mtl_desc_.mtl_data->sss = false;
			mtl_desc_.mtl_data->two_sided = false;

			mtl_desc_.mtl_data->normal_scale = 1;
			mtl_desc_.mtl_data->occlusion_strength = 1;

			mtl_desc_.mtl_data->detail_mode = RenderMaterial::SurfaceDetailMode::ParallaxMapping;
			mtl_desc_.mtl_data->height_offset_scale = float2(-0.5f, 0.06f);
			mtl_desc_.mtl_data->tess_factors = float4(5, 5, 1, 9);

			XMLNodePtr albedo_node = root->FirstNode("albedo");
			if (albedo_node)
			{
				XMLAttributePtr attr = albedo_node->Attrib("color");
				if (attr)
				{
					ExtractFVector<4>(attr->ValueString(), &mtl_desc_.mtl_data->albedo[0]);
				}
				attr = albedo_node->Attrib("texture");
				if (attr)
				{
					mtl_desc_.mtl_data->tex_names[RenderMaterial::TS_Albedo] = std::string(attr->ValueString());
				}
			}

			XMLNodePtr metalness_glossiness_node = root->FirstNode("metalness_glossiness");
			if (metalness_glossiness_node)
			{
				XMLAttributePtr attr = metalness_glossiness_node->Attrib("metalness");
				if (attr)
				{
					mtl_desc_.mtl_data->metalness = attr->ValueFloat();
				}
				attr = metalness_glossiness_node->Attrib("glossiness");
				if (attr)
				{
					mtl_desc_.mtl_data->glossiness = attr->ValueFloat();
				}
				attr = metalness_glossiness_node->Attrib("texture");
				if (attr)
				{
					mtl_desc_.mtl_data->tex_names[RenderMaterial::TS_MetalnessGlossiness] = std::string(attr->ValueString());
				}
			}
			else
			{
				XMLNodePtr metalness_node = root->FirstNode("metalness");
				if (metalness_node)
				{
					XMLAttributePtr attr = metalness_node->Attrib("value");
					if (attr)
					{
						mtl_desc_.mtl_data->metalness = attr->ValueFloat();
					}
					attr = metalness_node->Attrib("texture");
					if (attr)
					{
						mtl_desc_.mtl_data->tex_names[RenderMaterial::TS_MetalnessGlossiness] = std::string(attr->ValueString());
					}
				}

				XMLNodePtr glossiness_node = root->FirstNode("glossiness");
				if (glossiness_node)
				{
					XMLAttributePtr attr = glossiness_node->Attrib("value");
					if (attr)
					{
						mtl_desc_.mtl_data->glossiness = attr->ValueFloat();
					}
					attr = glossiness_node->Attrib("texture");
					if (attr)
					{
						mtl_desc_.mtl_data->tex_names[RenderMaterial::TS_MetalnessGlossiness] = std::string(attr->ValueString());
					}
				}
			}

			XMLNodePtr emissive_node = root->FirstNode("emissive");
			if (emissive_node)
			{
				XMLAttributePtr attr = emissive_node->Attrib("color");
				if (attr)
				{
					ExtractFVector<3>(attr->ValueString(), &mtl_desc_.mtl_data->emissive[0]);
				}
				attr = emissive_node->Attrib("texture");
				if (attr)
				{
					mtl_desc_.mtl_data->tex_names[RenderMaterial::TS_Emissive] = std::string(attr->ValueString());
				}
			}

			XMLNodePtr normal_node = root->FirstNode("normal");
			if (normal_node)
			{
				XMLAttributePtr attr = normal_node->Attrib("texture");
				if (attr)
				{
					mtl_desc_.mtl_data->tex_names[RenderMaterial::TS_Normal] = std::string(attr->ValueString());
				}

				attr = normal_node->Attrib("scale");
				if (attr)
				{
					mtl_desc_.mtl_data->normal_scale = attr->ValueFloat();
				}
			}

			XMLNodePtr height_node = root->FirstNode("height");
			if (height_node)
			{
				XMLAttributePtr attr = height_node->Attrib("texture");
				if (attr)
				{
					mtl_desc_.mtl_data->tex_names[RenderMaterial::TS_Height] = std::string(attr->ValueString());
				}

				attr = height_node->Attrib("offset");
				if (attr)
				{
					mtl_desc_.mtl_data->height_offset_scale.x() = attr->ValueFloat();
				}

				attr = height_node->Attrib("scale");
				if (attr)
				{
					mtl_desc_.mtl_data->height_offset_scale.y() = attr->ValueFloat();
				}
			}

			XMLNodePtr occlusion_node = root->FirstNode("occlusion");
			if (occlusion_node)
			{
				XMLAttributePtr attr = occlusion_node->Attrib("texture");
				if (attr)
				{
					mtl_desc_.mtl_data->tex_names[RenderMaterial::TS_Occlusion] = std::string(attr->ValueString());
				}

				attr = occlusion_node->Attrib("strength");
				if (attr)
				{
					mtl_desc_.mtl_data->occlusion_strength = attr->ValueFloat();
				}
			}

			XMLNodePtr detail_node = root->FirstNode("detail");
			if (detail_node)
			{
				XMLAttributePtr attr = detail_node->Attrib("mode");
				if (attr)
				{
					std::string_view const mode_str = attr->ValueString();
					size_t const mode_hash = HashRange(mode_str.begin(), mode_str.end());
					if (CT_HASH("Parallax Occlusion Mapping") == mode_hash)
					{
						mtl_desc_.mtl_data->detail_mode = RenderMaterial::SurfaceDetailMode::ParallaxOcclusionMapping;
					}
					else if (CT_HASH("Flat Tessellation") == mode_hash)
					{
						mtl_desc_.mtl_data->detail_mode = RenderMaterial::SurfaceDetailMode::FlatTessellation;
					}
					else if (CT_HASH("Smooth Tessellation") == mode_hash)
					{
						mtl_desc_.mtl_data->detail_mode = RenderMaterial::SurfaceDetailMode::SmoothTessellation;
					}
				}

				XMLNodePtr tess_node = detail_node->FirstNode("tess");
				if (tess_node)
				{
					attr = tess_node->Attrib("edge_hint");
					if (attr)
					{
						mtl_desc_.mtl_data->tess_factors.x() = attr->ValueFloat();
					}
					attr = tess_node->Attrib("inside_hint");
					if (attr)
					{
						mtl_desc_.mtl_data->tess_factors.y() = attr->ValueFloat();
					}
					attr = tess_node->Attrib("min");
					if (attr)
					{
						mtl_desc_.mtl_data->tess_factors.z() = attr->ValueFloat();
					}
					attr = tess_node->Attrib("max");
					if (attr)
					{
						mtl_desc_.mtl_data->tess_factors.w() = attr->ValueFloat();
					}
				}
			}

			XMLNodePtr transparent_node = root->FirstNode("transparent");
			if (transparent_node)
			{
				XMLAttributePtr attr = transparent_node->Attrib("value");
				if (attr)
				{
					mtl_desc_.mtl_data->transparent = attr->ValueInt() ? true : false;
				}
			}

			XMLNodePtr alpha_test_node = root->FirstNode("alpha_test");
			if (alpha_test_node)
			{
				XMLAttributePtr attr = alpha_test_node->Attrib("value");
				if (attr)
				{
					mtl_desc_.mtl_data->alpha_test = attr->ValueFloat();
				}
			}

			XMLNodePtr sss_node = root->FirstNode("sss");
			if (sss_node)
			{
				XMLAttributePtr attr = sss_node->Attrib("value");
				if (attr)
				{
					mtl_desc_.mtl_data->sss = attr->ValueInt() ? true : false;
				}
			}

			XMLNodePtr two_sided_node = root->FirstNode("two_sided");
			if (two_sided_node)
			{
				XMLAttributePtr attr = two_sided_node->Attrib("value");
				if (attr)
				{
					mtl_desc_.mtl_data->two_sided = attr->ValueInt() ? true : false;
				}
			}

			if (Context::Instance().RenderFactoryValid())
			{
				RenderFactory& rf = Context::Instance().RenderFactoryInstance();
				RenderDeviceCaps const& caps = rf.RenderEngineInstance().DeviceCaps();
				if (caps.multithread_res_creating_support)
				{
					this->MainThreadStageNoLock();
				}
			}
		}

		void MainThreadStage() override
		{
			std::lock_guard<std::mutex> lock(main_thread_stage_mutex_);
			this->MainThreadStageNoLock();
		}

		bool HasSubThreadStage() const override
		{
			return true;
		}

		bool Match(ResLoadingDesc const & rhs) const override
		{
			if (this->Type() == rhs.Type())
			{
				RenderMaterialLoadingDesc const & mtlld = static_cast<RenderMaterialLoadingDesc const &>(rhs);
				return (mtl_desc_.res_name == mtlld.mtl_desc_.res_name);
			}
			return false;
		}

		void CopyDataFrom(ResLoadingDesc const & rhs) override
		{
			BOOST_ASSERT(this->Type() == rhs.Type());

			RenderMaterialLoadingDesc const & mtlld = static_cast<RenderMaterialLoadingDesc const &>(rhs);
			mtl_desc_.res_name = mtlld.mtl_desc_.res_name;
			mtl_desc_.mtl_data = mtlld.mtl_desc_.mtl_data;
			mtl_desc_.mtl = mtlld.mtl_desc_.mtl;
		}

		std::shared_ptr<void> CloneResourceFrom(std::shared_ptr<void> const & resource) override
		{
			return resource;
		}

		std::shared_ptr<void> Resource() const override
		{
			return *mtl_desc_.mtl;
		}

	private:
		void MainThreadStageNoLock()
		{
			if (!*mtl_desc_.mtl)
			{
				RenderMaterialPtr mtl = MakeSharedPtr<RenderMaterial>();

				mtl->Name(mtl_desc_.mtl_data->name);

				mtl->Albedo(mtl_desc_.mtl_data->albedo);
				mtl->Metalness(mtl_desc_.mtl_data->metalness);
				mtl->Glossiness(mtl_desc_.mtl_data->glossiness);
				mtl->Emissive(mtl_desc_.mtl_data->emissive);

				mtl->Transparent(mtl_desc_.mtl_data->transparent);
				mtl->AlphaTestThreshold(mtl_desc_.mtl_data->alpha_test);
				mtl->Sss(mtl_desc_.mtl_data->sss);
				mtl->TwoSided(mtl_desc_.mtl_data->two_sided);

				mtl->NormalScale(mtl_desc_.mtl_data->normal_scale);
				mtl->OcclusionStrength(mtl_desc_.mtl_data->occlusion_strength);

				for (size_t i = 0; i < RenderMaterial::TS_NumTextureSlots; ++i)
				{
					mtl->TextureName(static_cast<RenderMaterial::TextureSlot>(i), mtl_desc_.mtl_data->tex_names[i]);
				}

				mtl->DetailMode(mtl_desc_.mtl_data->detail_mode);
				mtl->HeightOffset(mtl_desc_.mtl_data->height_offset_scale.x());
				mtl->HeightScale(mtl_desc_.mtl_data->height_offset_scale.y());
				mtl->EdgeTessHint(mtl_desc_.mtl_data->tess_factors.x());
				mtl->InsideTessHint(mtl_desc_.mtl_data->tess_factors.y());
				mtl->MinTessFactor(mtl_desc_.mtl_data->tess_factors.z());
				mtl->MaxTessFactor(mtl_desc_.mtl_data->tess_factors.w());

				mtl->LoadTextureSlots();

				*mtl_desc_.mtl = mtl;
			}
		}

	private:
		RenderMaterialDesc mtl_desc_;
		std::mutex main_thread_stage_mutex_;
	};

	// TODO: Need refactors

	uint32_t constexpr sw_albedo_clr_offset_ = 0;
	uint32_t constexpr sw_metalness_glossiness_factor_offset_ = sw_albedo_clr_offset_ + sizeof(float4);
	uint32_t constexpr sw_emissive_clr_offset_ = sw_metalness_glossiness_factor_offset_ + sizeof(float4);
	uint32_t constexpr sw_albedo_map_enabled_offset_ = sw_emissive_clr_offset_ + sizeof(float4);
	uint32_t constexpr sw_normal_map_enabled_offset_ = sw_albedo_map_enabled_offset_ + sizeof(uint32_t);
	uint32_t constexpr sw_height_map_parallax_enabled_offset_ = sw_normal_map_enabled_offset_ + sizeof(uint32_t);
	uint32_t constexpr sw_height_map_tess_enabled_offset_ = sw_height_map_parallax_enabled_offset_ + sizeof(uint32_t);
	uint32_t constexpr sw_occlusion_map_enabled_offset_ = sw_height_map_tess_enabled_offset_ + sizeof(uint32_t);
	uint32_t constexpr sw_alpha_test_threshold_offset_ = sw_occlusion_map_enabled_offset_ + sizeof(uint32_t);
	uint32_t constexpr sw_normal_scale_offset_ = sw_alpha_test_threshold_offset_ + sizeof(uint32_t);
	uint32_t constexpr sw_occlusion_strength_offset_ = sw_normal_scale_offset_ + sizeof(float);
	uint32_t constexpr sw_height_offset_scale_offset_ = sw_occlusion_strength_offset_ + sizeof(float);
	uint32_t constexpr sw_tess_factors_offset_ = sw_height_offset_scale_offset_ + sizeof(uint32_t);
	uint32_t constexpr sw_pdmc_size = sw_tess_factors_offset_ + sizeof(float4);

	RenderEngine::PredefinedMaterialCBuffer const& PredefinedMaterialCBufferInstance()
	{
		return Context::Instance().RenderFactoryInstance().RenderEngineInstance().PredefinedMaterialCBufferInstance();
	}
}

namespace KlayGE
{
	RenderMaterial::RenderMaterial()
	{
		if (Context::Instance().RenderFactoryValid())
		{
			auto* curr_cbuff = PredefinedMaterialCBufferInstance().CBuffer();
			cbuffer_ = curr_cbuff->Clone(curr_cbuff->OwnerEffect());
			is_sw_mode_ = false;
		}
		else
		{
			sw_cbuffer_.resize(sw_pdmc_size);
			is_sw_mode_ = true;
		}
	}

	RenderMaterialPtr RenderMaterial::Clone() const
	{
		RenderMaterialPtr ret = MakeSharedPtr<RenderMaterial>();

		ret->Name(this->Name());
		ret->is_sw_mode_ = is_sw_mode_;
		if (is_sw_mode_)
		{
			ret->sw_cbuffer_ = sw_cbuffer_;
		}
		else
		{
			ret->cbuffer_ = cbuffer_->Clone(cbuffer_->OwnerEffect());
		}

		ret->transparent_ = transparent_;
		ret->sss_ = sss_;
		ret->two_sided_ = two_sided_;
		ret->detail_mode_ = detail_mode_;
		ret->textures_ = textures_;

		return ret;
	}

	void RenderMaterial::Albedo(float4 const& value)
	{
		if (is_sw_mode_)
		{
			reinterpret_cast<float4&>(sw_cbuffer_[sw_albedo_clr_offset_]) = value;
		}
		else
		{
			PredefinedMaterialCBufferInstance().AlbedoClr(*cbuffer_) = value;
			cbuffer_->Dirty(true);
		}
	}

	float4 const& RenderMaterial::Albedo() const
	{
		if (is_sw_mode_)
		{
			return reinterpret_cast<float4 const&>(sw_cbuffer_[sw_albedo_clr_offset_]);
		}
		else
		{
			return PredefinedMaterialCBufferInstance().AlbedoClr(*cbuffer_);
		}
	}

	void RenderMaterial::Metalness(float value)
	{
		if (is_sw_mode_)
		{
			reinterpret_cast<float2&>(sw_cbuffer_[sw_metalness_glossiness_factor_offset_]).x() = value;
		}
		else
		{
			PredefinedMaterialCBufferInstance().MetalnessGlossinessFactor(*cbuffer_).x() = value;
			cbuffer_->Dirty(true);
		}
	}

	float RenderMaterial::Metalness() const
	{
		if (is_sw_mode_)
		{
			return reinterpret_cast<float2 const&>(sw_cbuffer_[sw_metalness_glossiness_factor_offset_]).x();
		}
		else
		{
			return PredefinedMaterialCBufferInstance().MetalnessGlossinessFactor(*cbuffer_).x();
		}
	}

	void RenderMaterial::Glossiness(float value)
	{
		if (is_sw_mode_)
		{
			reinterpret_cast<float2&>(sw_cbuffer_[sw_metalness_glossiness_factor_offset_]).y() = value;
		}
		else
		{
			PredefinedMaterialCBufferInstance().MetalnessGlossinessFactor(*cbuffer_).y() = MathLib::clamp(value, 1e-6f, 0.999f);
			cbuffer_->Dirty(true);
		}
	}

	float RenderMaterial::Glossiness() const
	{
		if (is_sw_mode_)
		{
			return reinterpret_cast<float2 const&>(sw_cbuffer_[sw_metalness_glossiness_factor_offset_]).y();
		}
		else
		{
			return PredefinedMaterialCBufferInstance().MetalnessGlossinessFactor(*cbuffer_).y();
		}
	}

	void RenderMaterial::Emissive(float3 const& value)
	{
		if (is_sw_mode_)
		{
			reinterpret_cast<float3&>(sw_cbuffer_[sw_emissive_clr_offset_]) = value;
		}
		else
		{
			reinterpret_cast<float3&>(PredefinedMaterialCBufferInstance().EmissiveClr(*cbuffer_)) = value;
			cbuffer_->Dirty(true);
		}
	}

	float3 const& RenderMaterial::Emissive() const
	{
		if (is_sw_mode_)
		{
			return reinterpret_cast<float3 const&>(sw_cbuffer_[sw_emissive_clr_offset_]);
		}
		else
		{
			return reinterpret_cast<float3 const&>(PredefinedMaterialCBufferInstance().EmissiveClr(*cbuffer_));
		}
	}

	void RenderMaterial::AlphaTestThreshold(float value)
	{
		if (is_sw_mode_)
		{
			reinterpret_cast<float&>(sw_cbuffer_[sw_alpha_test_threshold_offset_]) = value;
		}
		else
		{
			PredefinedMaterialCBufferInstance().AlphaTestThreshold(*cbuffer_) = value;
			cbuffer_->Dirty(true);
		}
	}

	float RenderMaterial::AlphaTestThreshold() const
	{
		if (is_sw_mode_)
		{
			return reinterpret_cast<float const&>(sw_cbuffer_[sw_alpha_test_threshold_offset_]);
		}
		else
		{
			return PredefinedMaterialCBufferInstance().AlphaTestThreshold(*cbuffer_);
		}
	}

	void RenderMaterial::NormalScale(float value)
	{
		if (is_sw_mode_)
		{
			reinterpret_cast<float&>(sw_cbuffer_[sw_normal_scale_offset_]) = value;
		}
		else
		{
			PredefinedMaterialCBufferInstance().NormalScale(*cbuffer_) = value;
			cbuffer_->Dirty(true);
		}
	}

	float RenderMaterial::NormalScale() const
	{
		if (is_sw_mode_)
		{
			return reinterpret_cast<float const&>(sw_cbuffer_[sw_normal_scale_offset_]);
		}
		else
		{
			return PredefinedMaterialCBufferInstance().NormalScale(*cbuffer_);
		}
	}

	void RenderMaterial::OcclusionStrength(float value)
	{
		if (is_sw_mode_)
		{
			reinterpret_cast<float&>(sw_cbuffer_[sw_occlusion_strength_offset_]) = value;
		}
		else
		{
			PredefinedMaterialCBufferInstance().OcclusionStrength(*cbuffer_) = value;
			cbuffer_->Dirty(true);
		}
	}

	float RenderMaterial::OcclusionStrength() const
	{
		if (is_sw_mode_)
		{
			return reinterpret_cast<float const&>(sw_cbuffer_[sw_occlusion_strength_offset_]);
		}
		else
		{
			return PredefinedMaterialCBufferInstance().OcclusionStrength(*cbuffer_);
		}
	}

	void RenderMaterial::TextureName(TextureSlot slot, std::string_view name)
	{
		textures_[slot].first = std::string(name);
	}

	void RenderMaterial::Texture(TextureSlot slot, ShaderResourceViewPtr srv)
	{
		auto const& pmcb = PredefinedMaterialCBufferInstance();
		switch (slot)
		{
		case TS_Albedo:
			if (is_sw_mode_)
			{
				reinterpret_cast<uint32_t&>(sw_cbuffer_[sw_albedo_map_enabled_offset_]) = srv ? 1 : 0;
			}
			else
			{
				pmcb.AlbedoMapEnabled(*cbuffer_) = srv ? 1 : 0;
			}
			break;

		case TS_MetalnessGlossiness:
			if (is_sw_mode_)
			{
				reinterpret_cast<float3&>(sw_cbuffer_[sw_metalness_glossiness_factor_offset_]).z() = srv ? 1.0f : 0.0f;
			}
			else
			{
				pmcb.MetalnessGlossinessFactor(*cbuffer_).z() = srv ? 1.0f : 0.0f;
			}
			break;

		case TS_Emissive:
			if (is_sw_mode_)
			{
				reinterpret_cast<float4&>(sw_cbuffer_[sw_emissive_clr_offset_]).w() = srv ? 1.0f : 0.0f;
			}
			else
			{
				pmcb.EmissiveClr(*cbuffer_).w() = srv ? 1.0f : 0.0f;
			}
			break;

		case TS_Normal:
			if (is_sw_mode_)
			{
				reinterpret_cast<uint32_t&>(sw_cbuffer_[sw_normal_map_enabled_offset_]) = srv ? 1 : 0;
			}
			else
			{
				pmcb.NormalMapEnabled(*cbuffer_) = srv ? 1 : 0;
			}
			break;

		case TS_Height:
			if ((detail_mode_ == SurfaceDetailMode::ParallaxMapping) || (detail_mode_ == SurfaceDetailMode::ParallaxOcclusionMapping))
			{
				if (is_sw_mode_)
				{
					reinterpret_cast<uint32_t&>(sw_cbuffer_[sw_height_map_parallax_enabled_offset_]) = srv ? 1 : 0;
					reinterpret_cast<uint32_t&>(sw_cbuffer_[sw_height_map_tess_enabled_offset_]) = 0;
				}
				else
				{
					pmcb.HeightMapParallaxEnabled(*cbuffer_) = srv ? 1 : 0;
					pmcb.HeightMapTessEnabled(*cbuffer_) = 0;
				}
			}
			else
			{
				if (is_sw_mode_)
				{
					reinterpret_cast<uint32_t&>(sw_cbuffer_[sw_height_map_parallax_enabled_offset_]) = 0;
					reinterpret_cast<uint32_t&>(sw_cbuffer_[sw_height_map_tess_enabled_offset_]) = srv ? 1 : 0;
				}
				else
				{
					pmcb.HeightMapParallaxEnabled(*cbuffer_) = 0;
					pmcb.HeightMapTessEnabled(*cbuffer_) = srv ? 1 : 0;
				}
			}
			break;

		case TS_Occlusion:
			if (is_sw_mode_)
			{
				reinterpret_cast<uint32_t&>(sw_cbuffer_[sw_occlusion_map_enabled_offset_]) = 0;
			}
			else
			{
				pmcb.OcclusionMapEnabled(*cbuffer_) = srv ? 1 : 0;
			}
			break;

		default:
			KFL_UNREACHABLE("Invalid texture slot");
			break;
		}

		cbuffer_->Dirty(true);

		textures_[slot].second = std::move(srv);
	}

	void RenderMaterial::HeightOffset(float value)
	{
		if (is_sw_mode_)
		{
			reinterpret_cast<float2&>(sw_cbuffer_[sw_height_offset_scale_offset_]).x() = value;
		}
		else
		{
			PredefinedMaterialCBufferInstance().HeightOffsetScale(*cbuffer_).x() = value;
			cbuffer_->Dirty(true);
		}
	}

	float RenderMaterial::HeightOffset() const
	{
		if (is_sw_mode_)
		{
			return reinterpret_cast<float2 const&>(sw_cbuffer_[sw_height_offset_scale_offset_]).x();
		}
		else
		{
			return PredefinedMaterialCBufferInstance().HeightOffsetScale(*cbuffer_).x();
		}
	}

	void RenderMaterial::HeightScale(float value)
	{
		if (is_sw_mode_)
		{
			reinterpret_cast<float2&>(sw_cbuffer_[sw_height_offset_scale_offset_]).y() = value;
		}
		else
		{
			PredefinedMaterialCBufferInstance().HeightOffsetScale(*cbuffer_).y() = value;
			cbuffer_->Dirty(true);
		}
	}

	float RenderMaterial::HeightScale() const
	{
		if (is_sw_mode_)
		{
			return reinterpret_cast<float2 const&>(sw_cbuffer_[sw_height_offset_scale_offset_]).y();
		}
		else
		{
			return PredefinedMaterialCBufferInstance().HeightOffsetScale(*cbuffer_).y();
		}
	}

	void RenderMaterial::EdgeTessHint(float value)
	{
		if (is_sw_mode_)
		{
			reinterpret_cast<float4&>(sw_cbuffer_[sw_tess_factors_offset_]).x() = value;
		}
		else
		{
			PredefinedMaterialCBufferInstance().TessFactors(*cbuffer_).x() = value;
			cbuffer_->Dirty(true);
		}
	}

	float RenderMaterial::EdgeTessHint() const
	{
		if (is_sw_mode_)
		{
			return reinterpret_cast<float4 const&>(sw_cbuffer_[sw_tess_factors_offset_]).x();
		}
		else
		{
			return PredefinedMaterialCBufferInstance().TessFactors(*cbuffer_).x();
		}
	}

	void RenderMaterial::InsideTessHint(float value)
	{
		if (is_sw_mode_)
		{
			reinterpret_cast<float4&>(sw_cbuffer_[sw_tess_factors_offset_]).y() = value;
		}
		else
		{
			PredefinedMaterialCBufferInstance().TessFactors(*cbuffer_).y() = value;
			cbuffer_->Dirty(true);
		}
	}

	float RenderMaterial::InsideTessHint() const
	{
		if (is_sw_mode_)
		{
			return reinterpret_cast<float4 const&>(sw_cbuffer_[sw_tess_factors_offset_]).y();
		}
		else
		{
			return PredefinedMaterialCBufferInstance().TessFactors(*cbuffer_).y();
		}
	}

	void RenderMaterial::MinTessFactor(float value)
	{
		if (is_sw_mode_)
		{
			reinterpret_cast<float4&>(sw_cbuffer_[sw_tess_factors_offset_]).z() = value;
		}
		else
		{
			PredefinedMaterialCBufferInstance().TessFactors(*cbuffer_).z() = value;
			cbuffer_->Dirty(true);
		}
	}

	float RenderMaterial::MinTessFactor() const
	{
		if (is_sw_mode_)
		{
			return reinterpret_cast<float4 const&>(sw_cbuffer_[sw_tess_factors_offset_]).z();
		}
		else
		{
			return PredefinedMaterialCBufferInstance().TessFactors(*cbuffer_).z();
		}
	}

	void RenderMaterial::MaxTessFactor(float value)
	{
		if (is_sw_mode_)
		{
			reinterpret_cast<float4&>(sw_cbuffer_[sw_tess_factors_offset_]).w() = value;
		}
		else
		{
			PredefinedMaterialCBufferInstance().TessFactors(*cbuffer_).w() = value;
			cbuffer_->Dirty(true);
		}
	}

	float RenderMaterial::MaxTessFactor() const
	{
		if (is_sw_mode_)
		{
			return reinterpret_cast<float4 const&>(sw_cbuffer_[sw_tess_factors_offset_]).w();
		}
		else
		{
			return PredefinedMaterialCBufferInstance().TessFactors(*cbuffer_).w();
		}
	}

	void RenderMaterial::Active(RenderEffect& effect)
	{
		if (&cbuffer_->OwnerEffect() != &effect)
		{
			albedo_tex_param_ = effect.ParameterByName("albedo_tex");
			metalness_glossiness_tex_param_ = effect.ParameterByName("metalness_glossiness_tex");
			emissive_tex_param_ = effect.ParameterByName("emissive_tex");
			normal_tex_param_ = effect.ParameterByName("normal_tex");
			height_tex_param_ = effect.ParameterByName("height_tex");
			occlusion_tex_param_ = effect.ParameterByName("occlusion_tex");
		}

		uint32_t const index = effect.FindCBuffer("klayge_material");
		if (index != static_cast<uint32_t>(-1) && (effect.CBufferByIndex(index)->Size() > 0))
		{
			if (&cbuffer_->OwnerEffect() != &effect)
			{
				cbuffer_ = cbuffer_->Clone(effect);
			}

			effect.BindCBufferByIndex(index, cbuffer_);
		}

		if (albedo_tex_param_)
		{
			*albedo_tex_param_ = this->Texture(RenderMaterial::TS_Albedo);
		}
		if (metalness_glossiness_tex_param_)
		{
			*metalness_glossiness_tex_param_ = this->Texture(RenderMaterial::TS_MetalnessGlossiness);
		}
		if (emissive_tex_param_)
		{
			*emissive_tex_param_ = this->Texture(RenderMaterial::TS_Emissive);
		}
		if (normal_tex_param_)
		{
			*normal_tex_param_ = this->Texture(RenderMaterial::TS_Normal);
		}
		if (height_tex_param_)
		{
			*height_tex_param_ = this->Texture(RenderMaterial::TS_Height);
		}
		if (occlusion_tex_param_)
		{
			*occlusion_tex_param_ = this->Texture(RenderMaterial::TS_Occlusion);
		}
	}

	void RenderMaterial::LoadTextureSlots()
	{
		if (Context::Instance().RenderFactoryValid())
		{
			auto& rf = Context::Instance().RenderFactoryInstance();
			for (size_t i = 0; i < RenderMaterial::TS_NumTextureSlots; ++i)
			{
				auto slot = static_cast<RenderMaterial::TextureSlot>(i);
				auto const& tex_name = textures_[slot].first;
				if (!tex_name.empty())
				{
					if (!ResLoader::Instance().Locate(tex_name).empty()
						|| !ResLoader::Instance().Locate(tex_name + ".dds").empty())
					{
						this->Texture(slot, rf.MakeTextureSrv(ASyncLoadTexture(tex_name, EAH_GPU_Read | EAH_Immutable)));
					}
				}
			}
		}
	}

	RenderMaterialPtr SyncLoadRenderMaterial(std::string_view mtlml_name)
	{
		return ResLoader::Instance().SyncQueryT<RenderMaterial>(MakeSharedPtr<RenderMaterialLoadingDesc>(mtlml_name));
	}

	RenderMaterialPtr ASyncLoadRenderMaterial(std::string_view mtlml_name)
	{
		// TODO: Make it really async
		return ResLoader::Instance().SyncQueryT<RenderMaterial>(MakeSharedPtr<RenderMaterialLoadingDesc>(mtlml_name));
	}

	void SaveRenderMaterial(RenderMaterialPtr const & mtl, std::string const & mtlml_name)
	{
		KlayGE::XMLDocument doc;

		XMLNodePtr root = doc.AllocNode(XNT_Element, "material");
		doc.RootNode(root);

		{
			XMLNodePtr albedo_node = doc.AllocNode(XNT_Element, "albedo");

			float4 const& albedo = mtl->Albedo();
			albedo_node->AppendAttrib(
				doc.AllocAttribString("color", std::format("{} {} {} {}", albedo.x(), albedo.y(), albedo.z(), albedo.w())));

			if (!mtl->TextureName(RenderMaterial::TS_Albedo).empty())
			{
				albedo_node->AppendAttrib(doc.AllocAttribString("texture", mtl->TextureName(RenderMaterial::TS_Albedo)));
			}

			root->AppendNode(albedo_node);
		}

		if ((mtl->Metalness() > 0) || (mtl->Glossiness() > 0) || !mtl->TextureName(RenderMaterial::TS_MetalnessGlossiness).empty())
		{
			XMLNodePtr metalness_glossiness_node = doc.AllocNode(XNT_Element, "metalness_glossiness");

			if (mtl->Metalness() > 0)
			{
				metalness_glossiness_node->AppendAttrib(doc.AllocAttribFloat("value", mtl->Metalness()));
			}
			if (mtl->Glossiness() > 0)
			{
				metalness_glossiness_node->AppendAttrib(doc.AllocAttribFloat("value", mtl->Glossiness()));
			}
			if (!mtl->TextureName(RenderMaterial::TS_MetalnessGlossiness).empty())
			{
				metalness_glossiness_node->AppendAttrib(
					doc.AllocAttribString("texture", mtl->TextureName(RenderMaterial::TS_MetalnessGlossiness)));
			}

			root->AppendNode(metalness_glossiness_node);
		}

		float3 const& emissive = mtl->Emissive();
		if ((emissive.x() > 0) || (emissive.y() > 0) || (emissive.z() > 0)
			|| (!mtl->TextureName(RenderMaterial::TS_Emissive).empty()))
		{
			XMLNodePtr emissive_node = doc.AllocNode(XNT_Element, "emissive");

			if ((emissive.x() > 0) || (emissive.y() > 0) || (emissive.z() > 0))
			{
				emissive_node->AppendAttrib(
					doc.AllocAttribString("color", std::format("{} {} {}", emissive.x(), emissive.y(), emissive.z())));
			}
			if (!mtl->TextureName(RenderMaterial::TS_Emissive).empty())
			{
				emissive_node->AppendAttrib(doc.AllocAttribString("texture", mtl->TextureName(RenderMaterial::TS_Emissive)));
			}

			root->AppendNode(emissive_node);
		}

		if (!mtl->TextureName(RenderMaterial::TS_Normal).empty())
		{
			XMLNodePtr normal_node = doc.AllocNode(XNT_Element, "normal");

			normal_node->AppendAttrib(doc.AllocAttribString("texture", mtl->TextureName(RenderMaterial::TS_Normal)));
			normal_node->AppendAttrib(doc.AllocAttribFloat("scale", mtl->NormalScale()));

			root->AppendNode(normal_node);
		}

		if (!mtl->TextureName(RenderMaterial::TS_Height).empty())
		{
			XMLNodePtr height_node = doc.AllocNode(XNT_Element, "height");

			height_node->AppendAttrib(doc.AllocAttribString("texture", mtl->TextureName(RenderMaterial::TS_Height)));
			height_node->AppendAttrib(doc.AllocAttribFloat("offset", mtl->HeightOffset()));
			height_node->AppendAttrib(doc.AllocAttribFloat("scale", mtl->HeightScale()));

			root->AppendNode(height_node);
		}

		if (!mtl->TextureName(RenderMaterial::TS_Occlusion).empty())
		{
			XMLNodePtr occlusion_node = doc.AllocNode(XNT_Element, "occlusion");

			occlusion_node->AppendAttrib(doc.AllocAttribString("texture", mtl->TextureName(RenderMaterial::TS_Occlusion)));
			occlusion_node->AppendAttrib(doc.AllocAttribFloat("strength", mtl->OcclusionStrength()));

			root->AppendNode(occlusion_node);
		}

		if (mtl->DetailMode() != RenderMaterial::SurfaceDetailMode::ParallaxMapping)
		{
			XMLNodePtr detail_node = doc.AllocNode(XNT_Element, "detail");

			std::string detail_mode_str;
			switch (mtl->DetailMode())
			{
			case RenderMaterial::SurfaceDetailMode::ParallaxOcclusionMapping:
				detail_mode_str = "Parallax Occlusion Mapping";
				break;

			case RenderMaterial::SurfaceDetailMode::FlatTessellation:
				detail_mode_str = "Flat Tessellation";
				break;

			case RenderMaterial::SurfaceDetailMode::SmoothTessellation:
				detail_mode_str = "Smooth Tessellation";
				break;

			default:
				KFL_UNREACHABLE("Invalid surface detail mode");
			}
			detail_node->AppendAttrib(doc.AllocAttribString("mode", detail_mode_str));

			if ((mtl->DetailMode() == RenderMaterial::SurfaceDetailMode::FlatTessellation) ||
				(mtl->DetailMode() == RenderMaterial::SurfaceDetailMode::SmoothTessellation))
			{
				XMLNodePtr tess_node = doc.AllocNode(XNT_Element, "tess");
				tess_node->AppendAttrib(doc.AllocAttribFloat("edge_hint", mtl->EdgeTessHint()));
				tess_node->AppendAttrib(doc.AllocAttribFloat("inside_hint", mtl->InsideTessHint()));
				tess_node->AppendAttrib(doc.AllocAttribFloat("min", mtl->MinTessFactor()));
				tess_node->AppendAttrib(doc.AllocAttribFloat("max", mtl->MaxTessFactor()));
				detail_node->AppendNode(tess_node);
			}

			root->AppendNode(detail_node);
		}

		if (mtl->Transparent())
		{
			XMLNodePtr transparent_node = doc.AllocNode(XNT_Element, "transparent");

			transparent_node->AppendAttrib(doc.AllocAttribString("value", "1"));

			root->AppendNode(transparent_node);
		}

		if (mtl->AlphaTestThreshold() > 0)
		{
			XMLNodePtr alpha_test_node = doc.AllocNode(XNT_Element, "alpha_test");

			alpha_test_node->AppendAttrib(doc.AllocAttribFloat("value", mtl->AlphaTestThreshold()));

			root->AppendNode(alpha_test_node);
		}

		if (mtl->Sss())
		{
			XMLNodePtr sss_node = doc.AllocNode(XNT_Element, "sss");

			sss_node->AppendAttrib(doc.AllocAttribString("value", "1"));

			root->AppendNode(sss_node);
		}

		if (mtl->TwoSided())
		{
			XMLNodePtr two_sided_node = doc.AllocNode(XNT_Element, "two_sided");

			two_sided_node->AppendAttrib(doc.AllocAttribString("value", "1"));

			root->AppendNode(two_sided_node);
		}

		std::ofstream ofs(std::string(mtlml_name).c_str());
		if (!ofs)
		{
			ofs.open((ResLoader::Instance().LocalFolder() + mtlml_name).c_str());
		}
		doc.Print(ofs);
	}
} // namespace KlayGE
