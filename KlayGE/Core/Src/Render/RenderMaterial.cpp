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
#include <KFL/ErrorHandling.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KFL/XMLDom.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KFL/Hash.hpp>
#include <KFL/CXX17/filesystem.hpp>

#include <fstream>

#if defined(KLAYGE_COMPILER_CLANGC2)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable" // Ignore unused variable (mpl_assertion_in_line_xxx) in boost
#endif
#include <boost/algorithm/string/split.hpp>
#if defined(KLAYGE_COMPILER_CLANGC2)
#pragma clang diagnostic pop
#endif
#include <boost/algorithm/string/trim.hpp>
#if defined(KLAYGE_COMPILER_CLANGC2)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable" // Ignore unused variable (mpl_assertion_in_line_xxx) in boost
#endif
#include <boost/lexical_cast.hpp>
#if defined(KLAYGE_COMPILER_CLANGC2)
#pragma clang diagnostic pop
#endif

#include <KlayGE/RenderMaterial.hpp>

namespace
{
	using namespace KlayGE;

	template <int N>
	void ExtractFVector(std::string_view value_str, float* v)
	{
		std::vector<std::string> strs;
		boost::algorithm::split(strs, value_str, boost::is_any_of(" "));
		for (size_t i = 0; i < N; ++ i)
		{
			if (i < strs.size())
			{
				boost::algorithm::trim(strs[i]);
				v[i] = static_cast<float>(atof(strs[i].c_str()));
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

				std::array<std::string, RenderMaterial::TS_NumTextureSlots> tex_names;

				RenderMaterial::SurfaceDetailMode detail_mode;
				float2 height_offset_scale;
				float4 tess_factors;
			};
			std::shared_ptr<RenderMaterialData> mtl_data;

			std::shared_ptr<RenderMaterialPtr> mtl;
		};

	public:
		explicit RenderMaterialLoadingDesc(std::string const & res_name)
		{
			mtl_desc_.res_name = res_name;
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
			XMLNodePtr root = doc.Parse(mtl_input);

			{
				XMLAttributePtr attr = root->Attrib("name");
				if (attr)
				{
					mtl_desc_.mtl_data->name = attr->ValueString();
				}
				else
				{
					std::filesystem::path res_path(mtl_desc_.res_name);
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

			mtl_desc_.mtl_data->detail_mode = RenderMaterial::SDM_Parallax;
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
					mtl_desc_.mtl_data->tex_names[RenderMaterial::TS_Albedo] = attr->ValueString();
				}
			}

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
					mtl_desc_.mtl_data->tex_names[RenderMaterial::TS_Metalness] = attr->ValueString();
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
					mtl_desc_.mtl_data->tex_names[RenderMaterial::TS_Glossiness] = attr->ValueString();
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
					mtl_desc_.mtl_data->tex_names[RenderMaterial::TS_Emissive] = attr->ValueString();
				}
			}

			XMLNodePtr normal_node = root->FirstNode("normal");
			if (normal_node)
			{
				XMLAttributePtr attr = normal_node->Attrib("texture");
				if (attr)
				{
					mtl_desc_.mtl_data->tex_names[RenderMaterial::TS_Normal] = attr->ValueString();
				}
			}

			XMLNodePtr height_node = root->FirstNode("height");
			if (height_node)
			{
				XMLAttributePtr attr = height_node->Attrib("texture");
				if (attr)
				{
					mtl_desc_.mtl_data->tex_names[RenderMaterial::TS_Height] = attr->ValueString();
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

			XMLNodePtr detail_node = root->FirstNode("detail");
			if (detail_node)
			{
				XMLAttributePtr attr = detail_node->Attrib("mode");
				if (attr)
				{
					std::string_view const mode_str = attr->ValueString();
					size_t const mode_hash = HashRange(mode_str.begin(), mode_str.end());
					if (CT_HASH("Flat Tessellation") == mode_hash)
					{
						mtl_desc_.mtl_data->detail_mode = RenderMaterial::SDM_FlatTessellation;
					}
					else if (CT_HASH("Smooth Tessellation") == mode_hash)
					{
						mtl_desc_.mtl_data->detail_mode = RenderMaterial::SDM_SmoothTessellation;
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

			RenderFactory& rf = Context::Instance().RenderFactoryInstance();
			RenderDeviceCaps const & caps = rf.RenderEngineInstance().DeviceCaps();
			if (caps.multithread_res_creating_support)
			{
				this->MainThreadStageNoLock();
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

				mtl->albedo = mtl_desc_.mtl_data->albedo;
				mtl->metalness = mtl_desc_.mtl_data->metalness;
				mtl->glossiness = mtl_desc_.mtl_data->glossiness;
				mtl->emissive = mtl_desc_.mtl_data->emissive;

				mtl->transparent = mtl_desc_.mtl_data->transparent;
				mtl->alpha_test = mtl_desc_.mtl_data->alpha_test;
				mtl->sss = mtl_desc_.mtl_data->sss;
				mtl->two_sided = mtl_desc_.mtl_data->two_sided;

				mtl->tex_names = mtl_desc_.mtl_data->tex_names;

				mtl->detail_mode = mtl_desc_.mtl_data->detail_mode;
				mtl->height_offset_scale = mtl_desc_.mtl_data->height_offset_scale;
				mtl->tess_factors = mtl_desc_.mtl_data->tess_factors;

				*mtl_desc_.mtl = mtl;
			}
		}

	private:
		RenderMaterialDesc mtl_desc_;
		std::mutex main_thread_stage_mutex_;
	};
}

namespace KlayGE
{
	RenderMaterialPtr SyncLoadRenderMaterial(std::string const & mtlml_name)
	{
		return ResLoader::Instance().SyncQueryT<RenderMaterial>(MakeSharedPtr<RenderMaterialLoadingDesc>(mtlml_name));
	}

	RenderMaterialPtr ASyncLoadRenderMaterial(std::string const & mtlml_name)
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

			std::string color_str = boost::lexical_cast<std::string>(mtl->albedo.x())
				+ ' ' + boost::lexical_cast<std::string>(mtl->albedo.y())
				+ ' ' + boost::lexical_cast<std::string>(mtl->albedo.z())
				+ ' ' + boost::lexical_cast<std::string>(mtl->albedo.w());
			albedo_node->AppendAttrib(doc.AllocAttribString("color", color_str));

			if (!mtl->tex_names[RenderMaterial::TS_Albedo].empty())
			{
				albedo_node->AppendAttrib(doc.AllocAttribString("texture", mtl->tex_names[RenderMaterial::TS_Albedo]));
			}

			root->AppendNode(albedo_node);
		}

		if ((mtl->metalness > 0) || !mtl->tex_names[RenderMaterial::TS_Metalness].empty())
		{
			XMLNodePtr metalness_node = doc.AllocNode(XNT_Element, "metalness");

			if (mtl->metalness > 0)
			{
				metalness_node->AppendAttrib(doc.AllocAttribFloat("value", mtl->metalness));
			}
			if (!mtl->tex_names[RenderMaterial::TS_Metalness].empty())
			{
				metalness_node->AppendAttrib(doc.AllocAttribString("texture", mtl->tex_names[RenderMaterial::TS_Metalness]));
			}

			root->AppendNode(metalness_node);
		}

		if ((mtl->glossiness > 0) || !mtl->tex_names[RenderMaterial::TS_Glossiness].empty())
		{
			XMLNodePtr glossiness_node = doc.AllocNode(XNT_Element, "glossiness");

			if (mtl->glossiness > 0)
			{
				glossiness_node->AppendAttrib(doc.AllocAttribFloat("value", mtl->glossiness));
			}
			if (!mtl->tex_names[RenderMaterial::TS_Glossiness].empty())
			{
				glossiness_node->AppendAttrib(doc.AllocAttribString("texture", mtl->tex_names[RenderMaterial::TS_Glossiness]));
			}

			root->AppendNode(glossiness_node);
		}

		if ((mtl->emissive.x() > 0) || (mtl->emissive.y() > 0) || (mtl->emissive.z() > 0)
			|| (!mtl->tex_names[RenderMaterial::TS_Emissive].empty()))
		{
			XMLNodePtr emissive_node = doc.AllocNode(XNT_Element, "emissive");

			if ((mtl->emissive.x() > 0) || (mtl->emissive.y() > 0) || (mtl->emissive.z() > 0))
			{
				std::string color_str = boost::lexical_cast<std::string>(mtl->emissive.x())
					+ ' ' + boost::lexical_cast<std::string>(mtl->emissive.y())
					+ ' ' + boost::lexical_cast<std::string>(mtl->emissive.z());
				emissive_node->AppendAttrib(doc.AllocAttribString("color", color_str));
			}
			if (!mtl->tex_names[RenderMaterial::TS_Emissive].empty())
			{
				emissive_node->AppendAttrib(doc.AllocAttribString("texture", mtl->tex_names[RenderMaterial::TS_Emissive]));
			}

			root->AppendNode(emissive_node);
		}

		if (!mtl->tex_names[RenderMaterial::TS_Normal].empty())
		{
			XMLNodePtr normal_node = doc.AllocNode(XNT_Element, "normal");

			normal_node->AppendAttrib(doc.AllocAttribString("texture", mtl->tex_names[RenderMaterial::TS_Normal]));

			root->AppendNode(normal_node);
		}

		if (!mtl->tex_names[RenderMaterial::TS_Height].empty())
		{
			XMLNodePtr height_node = doc.AllocNode(XNT_Element, "height");

			height_node->AppendAttrib(doc.AllocAttribString("texture", mtl->tex_names[RenderMaterial::TS_Height]));
			height_node->AppendAttrib(doc.AllocAttribFloat("offset", mtl->height_offset_scale.x()));
			height_node->AppendAttrib(doc.AllocAttribFloat("scale", mtl->height_offset_scale.y()));

			root->AppendNode(height_node);
		}

		if (mtl->detail_mode != RenderMaterial::SDM_Parallax)
		{
			XMLNodePtr detail_node = doc.AllocNode(XNT_Element, "detail");

			std::string detail_mode_str;
			switch (mtl->detail_mode)
			{
			case RenderMaterial::SDM_FlatTessellation:
				detail_mode_str = "Flat Tessellation";
				break;

			case RenderMaterial::SDM_SmoothTessellation:
				detail_mode_str = "Smooth Tessellation";
				break;

			default:
				KFL_UNREACHABLE("Invalid surface detail mode");
			}
			detail_node->AppendAttrib(doc.AllocAttribString("mode", detail_mode_str));

			{
				XMLNodePtr tess_node = doc.AllocNode(XNT_Element, "tess");
				tess_node->AppendAttrib(doc.AllocAttribFloat("edge_hint", mtl->tess_factors.x()));
				tess_node->AppendAttrib(doc.AllocAttribFloat("inside_hint", mtl->tess_factors.y()));
				tess_node->AppendAttrib(doc.AllocAttribFloat("min", mtl->tess_factors.z()));
				tess_node->AppendAttrib(doc.AllocAttribFloat("max", mtl->tess_factors.w()));
				detail_node->AppendNode(tess_node);
			}

			root->AppendNode(detail_node);
		}

		if (mtl->transparent)
		{
			XMLNodePtr transparent_node = doc.AllocNode(XNT_Element, "transparent");

			transparent_node->AppendAttrib(doc.AllocAttribString("value", "1"));

			root->AppendNode(transparent_node);
		}

		if (mtl->alpha_test > 0)
		{
			XMLNodePtr alpha_test_node = doc.AllocNode(XNT_Element, "alpha_test");

			alpha_test_node->AppendAttrib(doc.AllocAttribFloat("value", mtl->alpha_test));

			root->AppendNode(alpha_test_node);
		}

		if (mtl->sss)
		{
			XMLNodePtr sss_node = doc.AllocNode(XNT_Element, "sss");

			sss_node->AppendAttrib(doc.AllocAttribString("value", "1"));

			root->AppendNode(sss_node);
		}

		if (mtl->two_sided)
		{
			XMLNodePtr two_sided_node = doc.AllocNode(XNT_Element, "two_sided");

			two_sided_node->AppendAttrib(doc.AllocAttribString("value", "1"));

			root->AppendNode(two_sided_node);
		}

		std::ofstream ofs(mtlml_name.c_str());
		if (!ofs)
		{
			ofs.open((ResLoader::Instance().LocalFolder() + mtlml_name).c_str());
		}
		doc.Print(ofs);
	}
}
