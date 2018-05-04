#include <KlayGE/KlayGE.hpp>
#include <KFL/CXX17/iterator.hpp>
#include <KFL/ErrorHandling.hpp>
#include <KFL/Util.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/Renderable.hpp>
#include <KlayGE/RenderableHelper.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/FrameBuffer.hpp>
#include <KlayGE/SceneManager.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/RenderSettings.hpp>
#include <KlayGE/Mesh.hpp>
#include <KlayGE/GraphicsBuffer.hpp>
#include <KlayGE/SceneObjectHelper.hpp>
#include <KlayGE/Query.hpp>
#include <KlayGE/PostProcess.hpp>
#include <KlayGE/Camera.hpp>

#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/InputFactory.hpp>

#include <vector>
#include <sstream>

#include "SampleCommon.hpp"
#include "OIT.hpp"

using namespace std;
using namespace KlayGE;

namespace
{
	class RenderPolygon : public StaticMesh
	{
	public:
		RenderPolygon(RenderModelPtr const & model, std::wstring const & name)
			: StaticMesh(model, name),
				no_oit_tech_(nullptr),
				dp_1st_tech_(nullptr), dp_nth_tech_(nullptr), dp_1st_depth_tech_(nullptr), dp_nth_depth_tech_(nullptr),
				wb_effect_(nullptr), wb_render_tech_(nullptr), wb_blit_tech_(nullptr),
				gen_ppll_tech_(nullptr),
				ppll_render_tech_(nullptr),
				at_render_tech_(nullptr),
				gen_rov_ppa_effect_(nullptr), gen_rov_ppa_tech_(nullptr), rov_at_effect_(nullptr), rov_at_render_tech_(nullptr)
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();
			RenderDeviceCaps const & caps = rf.RenderEngineInstance().DeviceCaps();

			no_oit_effect_ = SyncLoadRenderEffect("NoOIT.fxml");
			no_oit_tech_ = no_oit_effect_->TechniqueByName("NoOIT");

			dp_effect_ = SyncLoadRenderEffect("DepthPeeling.fxml");
			dp_1st_tech_ = dp_effect_->TechniqueByName("DepthPeeling1st");
			dp_nth_tech_ = dp_effect_->TechniqueByName("DepthPeelingNth");
			if (!caps.depth_texture_support)
			{
				dp_nth_tech_ = dp_effect_->TechniqueByName("DepthPeelingNthWODepthTexture");
				dp_1st_depth_tech_ = dp_effect_->TechniqueByName("DepthPeeling1stDepth");
				dp_nth_depth_tech_ = dp_effect_->TechniqueByName("DepthPeelingNthDepth");
			}

			if (caps.max_simultaneous_uavs > 0)
			{
				gen_ppll_effect_ = SyncLoadRenderEffect("FragmentList.fxml");
				gen_ppll_tech_ = gen_ppll_effect_->TechniqueByName("GenPerPixelLinkedLists");
				rl_quad_ = rf.MakeRenderLayout();
				rl_quad_->TopologyType(RenderLayout::TT_TriangleStrip);
				rl_quad_->NumVertices(4);

				ppll_effect_ = SyncLoadRenderEffect("PerPixelLinkedLists.fxml");
				ppll_render_tech_ = ppll_effect_->TechniqueByName("RenderPerPixelLinkedLists");

				at_effect_ = SyncLoadRenderEffect("AdaptiveTransparency.fxml");
				at_render_tech_ = at_effect_->TechniqueByName("RenderAdaptiveTransparency");

				if (caps.rovs_support)
				{
					gen_rov_ppa_effect_ = SyncLoadRenderEffect("FragmentArray.fxml");
					gen_rov_ppa_tech_ = gen_rov_ppa_effect_->TechniqueByName("GenPerPixelArrays");

					rov_at_effect_ = SyncLoadRenderEffect("RovAdaptiveTransparency.fxml");
					rov_at_render_tech_ = rov_at_effect_->TechniqueByName("RenderRovAdaptiveTransparency");
				}
			}

			wb_effect_ = SyncLoadRenderEffect("WeightedBlended.fxml");
			wb_render_tech_ = wb_effect_->TechniqueByName("WeightedBlendedRender");
			wb_blit_tech_ = wb_effect_->TechniqueByName("WeightedBlendedBlit");
			
			technique_ = dp_1st_tech_;

			effect_attrs_ = EA_TransparencyFront;
		}

		void DoBuildMeshInfo() override
		{
			StaticMesh::DoBuildMeshInfo();

			AABBox const & pos_bb = this->PosBound();
			*(no_oit_effect_->ParameterByName("pos_center")) = pos_bb.Center();
			*(no_oit_effect_->ParameterByName("pos_extent")) = pos_bb.HalfSize();
			*(dp_effect_->ParameterByName("pos_center")) = pos_bb.Center();
			*(dp_effect_->ParameterByName("pos_extent")) = pos_bb.HalfSize();
			*(wb_effect_->ParameterByName("pos_center")) = pos_bb.Center();
			*(wb_effect_->ParameterByName("pos_extent")) = pos_bb.HalfSize();
			if (gen_ppll_tech_)
			{
				*(gen_ppll_effect_->ParameterByName("pos_center")) = pos_bb.Center();
				*(gen_ppll_effect_->ParameterByName("pos_extent")) = pos_bb.HalfSize();
			}
			if (gen_rov_ppa_tech_)
			{
				*(gen_rov_ppa_effect_->ParameterByName("pos_center")) = pos_bb.Center();
				*(gen_rov_ppa_effect_->ParameterByName("pos_extent")) = pos_bb.HalfSize();
			}

			AABBox const & tc_bb = this->TexcoordBound();
			*(no_oit_effect_->ParameterByName("tc_center")) = float2(tc_bb.Center().x(), tc_bb.Center().y());
			*(no_oit_effect_->ParameterByName("tc_extent")) = float2(tc_bb.HalfSize().x(), tc_bb.HalfSize().y());
			*(dp_effect_->ParameterByName("tc_center")) = float2(tc_bb.Center().x(), tc_bb.Center().y());
			*(dp_effect_->ParameterByName("tc_extent")) = float2(tc_bb.HalfSize().x(), tc_bb.HalfSize().y());
			*(wb_effect_->ParameterByName("tc_center")) = float2(tc_bb.Center().x(), tc_bb.Center().y());
			*(wb_effect_->ParameterByName("tc_extent")) = float2(tc_bb.HalfSize().x(), tc_bb.HalfSize().y());
			if (gen_ppll_tech_)
			{
				*(gen_ppll_effect_->ParameterByName("tc_center")) = float2(tc_bb.Center().x(), tc_bb.Center().y());
				*(gen_ppll_effect_->ParameterByName("tc_extent")) = float2(tc_bb.HalfSize().x(), tc_bb.HalfSize().y());
			}
			if (gen_rov_ppa_tech_)
			{
				*(gen_rov_ppa_effect_->ParameterByName("tc_center")) = float2(tc_bb.Center().x(), tc_bb.Center().y());
				*(gen_rov_ppa_effect_->ParameterByName("tc_extent")) = float2(tc_bb.HalfSize().x(), tc_bb.HalfSize().y());
			}

			*(no_oit_effect_->ParameterByName("albedo_clr")) = mtl_->albedo;
			*(no_oit_effect_->ParameterByName("metalness_clr")) = float2(mtl_->metalness,
				textures_[RenderMaterial::TS_Metalness].get() ? 1.0f : 0.0f);
			*(no_oit_effect_->ParameterByName("glossiness_clr")) = float2(mtl_->glossiness,
				textures_[RenderMaterial::TS_Glossiness].get() ? 1.0f : 0.0f);
			*(no_oit_effect_->ParameterByName("emissive_clr")) = float4(mtl_->emissive.x(), mtl_->emissive.y(), mtl_->emissive.z(),
				textures_[RenderMaterial::TS_Emissive].get() ? 1.0f : 0.0f);
			*(no_oit_effect_->ParameterByName("albedo_map_enabled"))
				= static_cast<int32_t>(textures_[RenderMaterial::TS_Albedo].get() ? 1 : 0);
			*(no_oit_effect_->ParameterByName("normal_map_enabled"))
				= static_cast<int32_t>(textures_[RenderMaterial::TS_Normal].get() ? 1 : 0);
			*(no_oit_effect_->ParameterByName("albedo_tex")) = textures_[RenderMaterial::TS_Albedo];
			*(no_oit_effect_->ParameterByName("metalness_tex")) = textures_[RenderMaterial::TS_Metalness];
			*(no_oit_effect_->ParameterByName("glossiness_tex")) = textures_[RenderMaterial::TS_Glossiness];
			*(no_oit_effect_->ParameterByName("emissive_tex")) = textures_[RenderMaterial::TS_Emissive];
			*(no_oit_effect_->ParameterByName("normal_tex")) = textures_[RenderMaterial::TS_Normal];

			*(dp_effect_->ParameterByName("albedo_clr")) = mtl_->albedo;
			*(dp_effect_->ParameterByName("metalness_clr")) = float2(mtl_->metalness,
				textures_[RenderMaterial::TS_Metalness].get() ? 1.0f : 0.0f);
			*(dp_effect_->ParameterByName("glossiness_clr")) = float2(mtl_->glossiness,
				textures_[RenderMaterial::TS_Glossiness].get() ? 1.0f : 0.0f);
			*(dp_effect_->ParameterByName("emissive_clr")) = float4(mtl_->emissive.x(), mtl_->emissive.y(), mtl_->emissive.z(),
				textures_[RenderMaterial::TS_Emissive].get() ? 1.0f : 0.0f);
			*(dp_effect_->ParameterByName("albedo_map_enabled")) 
				= static_cast<int32_t>(textures_[RenderMaterial::TS_Albedo].get() ? 1 : 0);
			*(dp_effect_->ParameterByName("normal_map_enabled"))
				= static_cast<int32_t>(textures_[RenderMaterial::TS_Normal].get() ? 1 : 0);
			*(dp_effect_->ParameterByName("albedo_tex")) = textures_[RenderMaterial::TS_Albedo];
			*(dp_effect_->ParameterByName("metalness_tex")) = textures_[RenderMaterial::TS_Metalness];
			*(dp_effect_->ParameterByName("glossiness_tex")) = textures_[RenderMaterial::TS_Glossiness];
			*(dp_effect_->ParameterByName("emissive_tex")) = textures_[RenderMaterial::TS_Emissive];
			*(dp_effect_->ParameterByName("normal_tex")) = textures_[RenderMaterial::TS_Normal];

			*(wb_effect_->ParameterByName("albedo_clr")) = mtl_->albedo;
			*(wb_effect_->ParameterByName("metalness_clr")) = float2(mtl_->metalness,
				textures_[RenderMaterial::TS_Metalness].get() ? 1.0f : 0.0f);
			*(wb_effect_->ParameterByName("glossiness_clr")) = float2(mtl_->glossiness,
				textures_[RenderMaterial::TS_Glossiness].get() ? 1.0f : 0.0f);
			*(wb_effect_->ParameterByName("emissive_clr")) = float4(mtl_->emissive.x(), mtl_->emissive.y(), mtl_->emissive.z(),
				textures_[RenderMaterial::TS_Emissive].get() ? 1.0f : 0.0f);
			*(wb_effect_->ParameterByName("albedo_map_enabled"))
				= static_cast<int32_t>(textures_[RenderMaterial::TS_Albedo].get() ? 1 : 0);
			*(wb_effect_->ParameterByName("normal_map_enabled"))
				= static_cast<int32_t>(textures_[RenderMaterial::TS_Normal].get() ? 1 : 0);
			*(wb_effect_->ParameterByName("albedo_tex")) = textures_[RenderMaterial::TS_Albedo];
			*(wb_effect_->ParameterByName("metalness_tex")) = textures_[RenderMaterial::TS_Metalness];
			*(wb_effect_->ParameterByName("glossiness_tex")) = textures_[RenderMaterial::TS_Glossiness];
			*(wb_effect_->ParameterByName("emissive_tex")) = textures_[RenderMaterial::TS_Emissive];
			*(wb_effect_->ParameterByName("normal_tex")) = textures_[RenderMaterial::TS_Normal];

			if (gen_ppll_tech_)
			{
				*(gen_ppll_effect_->ParameterByName("albedo_clr")) = mtl_->albedo;
				*(gen_ppll_effect_->ParameterByName("metalness_clr")) = float2(mtl_->metalness,
					textures_[RenderMaterial::TS_Metalness].get() ? 1.0f : 0.0f);
				*(gen_ppll_effect_->ParameterByName("glossiness_clr")) = float2(mtl_->glossiness,
					textures_[RenderMaterial::TS_Glossiness].get() ? 1.0f : 0.0f);
				*(gen_ppll_effect_->ParameterByName("emissive_clr")) = float4(mtl_->emissive.x(), mtl_->emissive.y(), mtl_->emissive.z(),
					textures_[RenderMaterial::TS_Emissive].get() ? 1.0f : 0.0f);
				*(gen_ppll_effect_->ParameterByName("albedo_map_enabled"))
					= static_cast<int32_t>(textures_[RenderMaterial::TS_Albedo].get() ? 1 : 0);
				*(gen_ppll_effect_->ParameterByName("normal_map_enabled"))
					= static_cast<int32_t>(textures_[RenderMaterial::TS_Normal].get() ? 1 : 0);
				*(gen_ppll_effect_->ParameterByName("albedo_tex")) = textures_[RenderMaterial::TS_Albedo];
				*(gen_ppll_effect_->ParameterByName("metalness_tex")) = textures_[RenderMaterial::TS_Metalness];
				*(gen_ppll_effect_->ParameterByName("glossiness_tex")) = textures_[RenderMaterial::TS_Glossiness];
				*(gen_ppll_effect_->ParameterByName("emissive_tex")) = textures_[RenderMaterial::TS_Emissive];
				*(gen_ppll_effect_->ParameterByName("normal_tex")) = textures_[RenderMaterial::TS_Normal];
			}
			if (gen_rov_ppa_tech_)
			{
				*(gen_rov_ppa_effect_->ParameterByName("albedo_clr")) = mtl_->albedo;
				*(gen_rov_ppa_effect_->ParameterByName("metalness_clr")) = float2(mtl_->metalness,
					textures_[RenderMaterial::TS_Metalness].get() ? 1.0f : 0.0f);
				*(gen_rov_ppa_effect_->ParameterByName("glossiness_clr")) = float2(mtl_->glossiness,
					textures_[RenderMaterial::TS_Glossiness].get() ? 1.0f : 0.0f);
				*(gen_rov_ppa_effect_->ParameterByName("emissive_clr")) = float4(mtl_->emissive.x(), mtl_->emissive.y(), mtl_->emissive.z(),
					textures_[RenderMaterial::TS_Emissive].get() ? 1.0f : 0.0f);
				*(gen_rov_ppa_effect_->ParameterByName("albedo_map_enabled"))
					= static_cast<int32_t>(textures_[RenderMaterial::TS_Albedo].get() ? 1 : 0);
				*(gen_rov_ppa_effect_->ParameterByName("normal_map_enabled"))
					= static_cast<int32_t>(textures_[RenderMaterial::TS_Normal].get() ? 1 : 0);
				*(gen_rov_ppa_effect_->ParameterByName("albedo_tex")) = textures_[RenderMaterial::TS_Albedo];
				*(gen_rov_ppa_effect_->ParameterByName("metalness_tex")) = textures_[RenderMaterial::TS_Metalness];
				*(gen_rov_ppa_effect_->ParameterByName("glossiness_tex")) = textures_[RenderMaterial::TS_Glossiness];
				*(gen_rov_ppa_effect_->ParameterByName("emissive_tex")) = textures_[RenderMaterial::TS_Emissive];
				*(gen_rov_ppa_effect_->ParameterByName("normal_tex")) = textures_[RenderMaterial::TS_Normal];
			}
		}

		void SetOITMode(OITMode mode)
		{
			mode_ = mode;
		}

		void SetAlpha(float alpha)
		{
			*(no_oit_effect_->ParameterByName("alpha")) = alpha;
			*(dp_effect_->ParameterByName("alpha")) = alpha;
			*(wb_effect_->ParameterByName("alpha")) = alpha;
			if (gen_ppll_tech_)
			{
				*(gen_ppll_effect_->ParameterByName("alpha")) = alpha;
			}
			if (gen_rov_ppa_tech_)
			{
				*(gen_rov_ppa_effect_->ParameterByName("alpha")) = alpha;
			}
		}

		void FirstPass(bool fp)
		{
			first_pass_ = fp;
			if (fp)
			{
				switch (mode_)
				{
				case OM_No:
					effect_ = no_oit_effect_;
					technique_ = no_oit_tech_;
					break;

				case OM_DepthPeeling:
					effect_ = dp_effect_;
					technique_ = dp_1st_tech_;
					break;

				case OM_WeightedBlended:
					effect_ = wb_effect_;
					technique_ = wb_render_tech_;
					break;

				case OM_PerPixelLinkedLists:
				case OM_AdaptiveTransparency:
					effect_ = gen_ppll_effect_;
					technique_ = gen_ppll_tech_;
					break;

				case OM_RovAdaptiveTransparency:
					effect_ = gen_rov_ppa_effect_;
					technique_ = gen_rov_ppa_tech_;
					break;

				default:
					KFL_UNREACHABLE("Invalid OIT mode");
				}
			}
			else
			{
				switch (mode_)
				{
				case OM_No:
					effect_ = no_oit_effect_;
					technique_ = no_oit_tech_;
					break;

				case OM_DepthPeeling:
					effect_ = dp_effect_;
					technique_ = dp_nth_tech_;
					break;

				case OM_PerPixelLinkedLists:
					effect_ = ppll_effect_;
					technique_ = ppll_render_tech_;
					break;

				case OM_AdaptiveTransparency:
					effect_ = at_effect_;
					technique_ = at_render_tech_;
					break;

				case OM_RovAdaptiveTransparency:
					effect_ = rov_at_effect_;
					technique_ = rov_at_render_tech_;
					break;

				default:
					KFL_UNREACHABLE("Invalid OIT mode");
				}
			}
		}

		void DepthPass(bool dp)
		{
			BOOST_ASSERT(OM_DepthPeeling == mode_);

			effect_ = dp_effect_;
			if (dp)
			{
				if (first_pass_)
				{
					technique_ = dp_1st_depth_tech_;
				}
				else
				{
					technique_ = dp_nth_depth_tech_;
				}
			}
			else
			{
				if (first_pass_)
				{
					technique_ = dp_1st_tech_;
				}
				else
				{
					technique_ = dp_nth_tech_;
				}
			}
		}

		void LastDepth(TexturePtr const & depth_tex)
		{
			if (dp_nth_tech_)
			{
				*(dp_effect_->ParameterByName("last_depth_tex")) = depth_tex;
			}
		}

		void BackgroundTex(TexturePtr const & bg_tex)
		{
			if (ppll_render_tech_)
			{
				*(ppll_effect_->ParameterByName("bg_tex")) = bg_tex;
			}
		}

		void LinkedListBuffer(GraphicsBufferPtr const & fragment_link_buf, GraphicsBufferPtr const & start_offset_buf)
		{
			if (gen_ppll_tech_)
			{
				*(gen_ppll_effect_->ParameterByName("rw_frags_buffer")) = fragment_link_buf;
				*(gen_ppll_effect_->ParameterByName("rw_start_offset_buffer")) = start_offset_buf;
			}
			if (gen_rov_ppa_tech_)
			{
				*(gen_rov_ppa_effect_->ParameterByName("rw_frags_buffer")) = fragment_link_buf;
				*(gen_rov_ppa_effect_->ParameterByName("rw_frag_length_buffer")) = start_offset_buf;
			}

			if (ppll_render_tech_)
			{
				*(ppll_effect_->ParameterByName("frags_buffer")) = fragment_link_buf;
				*(ppll_effect_->ParameterByName("start_offset_buffer")) = start_offset_buf;
			}
			if (at_render_tech_)
			{
				*(at_effect_->ParameterByName("frags_buffer")) = fragment_link_buf;
				*(at_effect_->ParameterByName("start_offset_buffer")) = start_offset_buf;
			}
			if (rov_at_render_tech_)
			{
				*(rov_at_effect_->ParameterByName("frags_buffer")) = fragment_link_buf;
				*(rov_at_effect_->ParameterByName("frag_length_buffer")) = start_offset_buf;
			}
		}

		void AccumWeightTextures(TexturePtr const & accum_tex, TexturePtr const & weight_tex)
		{
			*(wb_effect_->ParameterByName("accum_tex")) = accum_tex;
			*(wb_effect_->ParameterByName("weight_tex")) = weight_tex;
		}

		void RenderQuad()
		{
			RenderEffect* effect = nullptr;
			RenderTechnique* tech = nullptr;
			RenderLayout* rl = nullptr;
			switch (mode_)
			{
			case OM_WeightedBlended:
				effect = wb_effect_.get();
				tech = wb_blit_tech_;
				rl = rl_quad_.get();
				break;

			case OM_PerPixelLinkedLists:
				effect = ppll_effect_.get();
				tech = ppll_render_tech_;
				rl = rl_quad_.get();
				break;

			case OM_AdaptiveTransparency:
				effect = at_effect_.get();
				tech = at_render_tech_;
				rl = rl_quad_.get();
				break;

			case OM_RovAdaptiveTransparency:
				effect = rov_at_effect_.get();
				tech = rov_at_render_tech_;
				rl = rl_quad_.get();
				break;

			default:
				KFL_UNREACHABLE("Invalid OIT mode");
			}

			BOOST_ASSERT(effect != nullptr);
			BOOST_ASSERT(tech != nullptr);
			BOOST_ASSERT(rl != nullptr);

			RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
			if ((OM_PerPixelLinkedLists == mode_) || (OM_AdaptiveTransparency == mode_) || (OM_RovAdaptiveTransparency == mode_))
			{
				*(effect->ParameterByName("frame_width")) = static_cast<int32_t>(re.CurFrameBuffer()->GetViewport()->width);
			}

			re.Render(*effect, *tech, *rl);
		}

		void OnRenderBegin()
		{
			App3DFramework const & app = Context::Instance().AppInstance();
			Camera const & camera = app.ActiveCamera();

			float4x4 const & model = float4x4::Identity();

			*(effect_->ParameterByName("mvp")) = model * camera.ViewProjMatrix();
			*(effect_->ParameterByName("eye_pos")) = camera.EyePos();

			switch (mode_)
			{
			case OM_No:
				break;

			case OM_DepthPeeling:
				{
					float q = camera.FarPlane() / (camera.FarPlane() - camera.NearPlane());
					*(effect_->ParameterByName("near_q")) = float2(camera.NearPlane() * q, q);
					*(effect_->ParameterByName("far_plane")) = float2(camera.FarPlane(), 1.0f / camera.FarPlane());
					break;
				}

			case OM_WeightedBlended:
				{
					*(effect_->ParameterByName("near_far")) = float2(camera.NearPlane(), camera.FarPlane());
					break;
				}
			
			case OM_PerPixelLinkedLists:
			case OM_AdaptiveTransparency:
			case OM_RovAdaptiveTransparency:
				{
					RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
					*(effect_->ParameterByName("frame_width")) = static_cast<int32_t>(re.CurFrameBuffer()->GetViewport()->width);
					break;
				}

			default:
				KFL_UNREACHABLE("Invalid OIT mode");
			}
		}

		void LightPos(float3 const & light_pos)
		{
			*(no_oit_effect_->ParameterByName("light_pos")) = light_pos;
			*(dp_effect_->ParameterByName("light_pos")) = light_pos;
			*(wb_effect_->ParameterByName("light_pos")) = light_pos;
			if (gen_ppll_tech_)
			{
				*(gen_ppll_effect_->ParameterByName("light_pos")) = light_pos;
			}
			if (gen_rov_ppa_tech_)
			{
				*(gen_rov_ppa_effect_->ParameterByName("light_pos")) = light_pos;
			}
		}

	private:
		OITMode mode_;
		bool first_pass_;
		
		RenderEffectPtr no_oit_effect_;
		RenderTechnique* no_oit_tech_;

		RenderEffectPtr dp_effect_;
		RenderTechnique* dp_1st_tech_;
		RenderTechnique* dp_nth_tech_;
		RenderTechnique* dp_1st_depth_tech_;
		RenderTechnique* dp_nth_depth_tech_;

		RenderEffectPtr wb_effect_;
		RenderTechnique* wb_render_tech_;
		RenderTechnique* wb_blit_tech_;

		RenderEffectPtr gen_ppll_effect_;
		RenderTechnique* gen_ppll_tech_;
		RenderLayoutPtr rl_quad_;

		RenderEffectPtr ppll_effect_;
		RenderTechnique* ppll_render_tech_;

		RenderEffectPtr at_effect_;
		RenderTechnique* at_render_tech_;

		RenderEffectPtr gen_rov_ppa_effect_;
		RenderTechnique* gen_rov_ppa_tech_;

		RenderEffectPtr rov_at_effect_;
		RenderTechnique* rov_at_render_tech_;
	};

	class PolygonObject : public SceneObjectHelper
	{
	public:
		PolygonObject()
			: SceneObjectHelper(SOA_Cullable)
		{
			renderable_ = SyncLoadModel("robot_clean.meshml", EAH_GPU_Read | EAH_Immutable, CreateModelFactory<RenderModel>(), CreateMeshFactory<RenderPolygon>());
		}

		void LightPos(float3 const & light_pos)
		{
			RenderModelPtr model = checked_pointer_cast<RenderModel>(renderable_);
			for (uint32_t i = 0; i < model->NumSubrenderables(); ++ i)
			{
				checked_pointer_cast<RenderPolygon>(model->Subrenderable(i))->LightPos(light_pos);
			}
		}

		void SetOITMode(OITMode mode)
		{
			RenderModelPtr model = checked_pointer_cast<RenderModel>(renderable_);
			for (uint32_t i = 0; i < model->NumSubrenderables(); ++ i)
			{
				checked_pointer_cast<RenderPolygon>(model->Subrenderable(i))->SetOITMode(mode);
			}
		}

		void SetAlpha(float alpha)
		{
			RenderModelPtr model = checked_pointer_cast<RenderModel>(renderable_);
			for (uint32_t i = 0; i < model->NumSubrenderables(); ++ i)
			{
				checked_pointer_cast<RenderPolygon>(model->Subrenderable(i))->SetAlpha(alpha);
			}
		}

		void FirstPass(bool fp)
		{
			RenderModelPtr model = checked_pointer_cast<RenderModel>(renderable_);
			for (uint32_t i = 0; i < model->NumSubrenderables(); ++ i)
			{
				checked_pointer_cast<RenderPolygon>(model->Subrenderable(i))->FirstPass(fp);
			}
		}

		void DepthPass(bool dp)
		{
			RenderModelPtr model = checked_pointer_cast<RenderModel>(renderable_);
			for (uint32_t i = 0; i < model->NumSubrenderables(); ++ i)
			{
				checked_pointer_cast<RenderPolygon>(model->Subrenderable(i))->DepthPass(dp);
			}
		}

		void LastDepth(TexturePtr const & depth_tex)
		{
			RenderModelPtr model = checked_pointer_cast<RenderModel>(renderable_);
			for (uint32_t i = 0; i < model->NumSubrenderables(); ++ i)
			{
				checked_pointer_cast<RenderPolygon>(model->Subrenderable(i))->LastDepth(depth_tex);
			}
		}

		void BackgroundTex(TexturePtr const & bg_tex)
		{
			RenderModelPtr model = checked_pointer_cast<RenderModel>(renderable_);
			for (uint32_t i = 0; i < model->NumSubrenderables(); ++ i)
			{
				checked_pointer_cast<RenderPolygon>(model->Subrenderable(i))->BackgroundTex(bg_tex);
			}
		}

		void LinkedListBuffer(GraphicsBufferPtr const & fragment_link_buf, GraphicsBufferPtr const & start_offset_buf)
		{
			RenderModelPtr model = checked_pointer_cast<RenderModel>(renderable_);
			for (uint32_t i = 0; i < model->NumSubrenderables(); ++ i)
			{
				checked_pointer_cast<RenderPolygon>(model->Subrenderable(i))->LinkedListBuffer(fragment_link_buf, start_offset_buf);
			}
		}

		void AccumWeightTextures(TexturePtr const & accum_tex, TexturePtr const & weight_tex)
		{
			RenderModelPtr model = checked_pointer_cast<RenderModel>(renderable_);
			for (uint32_t i = 0; i < model->NumSubrenderables(); ++i)
			{
				checked_pointer_cast<RenderPolygon>(model->Subrenderable(i))->AccumWeightTextures(accum_tex, weight_tex);
			}
		}

		void RenderQuad()
		{
			RenderModelPtr model = checked_pointer_cast<RenderModel>(renderable_);
			checked_pointer_cast<RenderPolygon>(model->Subrenderable(0))->RenderQuad();
		}
	};


	enum
	{
		Exit,
	};

	InputActionDefine actions[] =
	{
		InputActionDefine(Exit, KS_Escape),
	};
}


int SampleMain()
{
	OITApp app;
	app.Create();
	app.Run();

	return 0;
}

OITApp::OITApp()
			: App3DFramework("Order Independent Transparency"),
				num_layers_(0)
{
	ResLoader::Instance().AddPath("../../Samples/media/OIT");
}

void OITApp::OnCreate()
{
	font_ = SyncLoadFont("gkai00mp.kfont");

	polygon_ = MakeSharedPtr<PolygonObject>();
	checked_pointer_cast<PolygonObject>(polygon_)->LightPos(float3(-1, 2, 1));
	polygon_->AddToSceneManager();

	this->LookAt(float3(-2.0f, 2.0f, 2.0f), float3(0, 1, 0));
	this->Proj(0.1f, 10);

	RenderFactory& rf = Context::Instance().RenderFactoryInstance();
	RenderEngine& re = rf.RenderEngineInstance();
	RenderDeviceCaps const & caps = re.DeviceCaps();

	TexturePtr y_cube_map = ASyncLoadTexture("uffizi_cross_filtered_y.dds", EAH_GPU_Read | EAH_Immutable);
	TexturePtr c_cube_map = ASyncLoadTexture("uffizi_cross_filtered_c.dds", EAH_GPU_Read | EAH_Immutable);
	sky_box_ = MakeSharedPtr<SceneObjectSkyBox>(0);
	checked_pointer_cast<SceneObjectSkyBox>(sky_box_)->CompressedCubeMap(y_cube_map, c_cube_map);
	sky_box_->AddToSceneManager();

	depth_texture_support_ = caps.depth_texture_support;

	peeling_fbs_.resize(17);
	for (size_t i = 0; i < peeling_fbs_.size(); ++ i)
	{
		peeling_fbs_[i] = rf.MakeFrameBuffer();
		peeling_fbs_[i]->GetViewport()->camera = re.CurFrameBuffer()->GetViewport()->camera;
	}
	peeled_texs_.resize(peeling_fbs_.size());

	if (!depth_texture_support_)
	{
		for (size_t i = 0; i < depth_fbs_.size(); ++ i)
		{
			depth_fbs_[i] = rf.MakeFrameBuffer();
			depth_fbs_[i]->GetViewport()->camera = re.CurFrameBuffer()->GetViewport()->camera;
		}
	}

	for (size_t i = 0; i < oc_queries_.size(); ++ i)
	{
		oc_queries_[i] = checked_pointer_cast<ConditionalRender>(rf.MakeConditionalRender());
	}

	weighted_fb_ = rf.MakeFrameBuffer();
	weighted_fb_->GetViewport()->camera = re.CurFrameBuffer()->GetViewport()->camera;

	if (caps.max_simultaneous_uavs > 0)
	{
		opaque_bg_fb_ = rf.MakeFrameBuffer();
		opaque_bg_fb_->GetViewport()->camera = re.CurFrameBuffer()->GetViewport()->camera;
		linked_list_fb_ = rf.MakeFrameBuffer();
		linked_list_fb_->GetViewport()->camera = re.CurFrameBuffer()->GetViewport()->camera;
	}

	tb_controller_.AttachCamera(this->ActiveCamera());
	tb_controller_.Scalers(0.003f, 0.003f);

	InputEngine& inputEngine(Context::Instance().InputFactoryInstance().InputEngineInstance());
	InputActionMap actionMap;
	actionMap.AddActions(actions, actions + std::size(actions));

	action_handler_t input_handler = MakeSharedPtr<input_signal>();
	input_handler->connect(
		[this](InputEngine const & sender, InputAction const & action)
		{
			this->InputHandler(sender, action);
		});
	inputEngine.ActionMap(actionMap, input_handler);

	blend_pp_ = SyncLoadPostProcess("Blend.ppml", "blend");

	UIManager::Instance().Load(ResLoader::Instance().Open("OIT.uiml"));
	dialog_oit_ = UIManager::Instance().GetDialogs()[0];
	dialog_layer_ = UIManager::Instance().GetDialogs()[1];

	id_oit_mode_ = dialog_oit_->IDFromName("OITMode");
	id_alpha_static_ = dialog_oit_->IDFromName("AlphaStatic");
	id_alpha_slider_ = dialog_oit_->IDFromName("Alpha");
	id_layer_combo_ = dialog_layer_->IDFromName("LayerCombo");
	id_layer_tex_ = dialog_layer_->IDFromName("LayerTexButton");

	if ((0 == caps.max_simultaneous_uavs) || !caps.rovs_support)
	{
		dialog_oit_->Control<UIComboBox>(id_oit_mode_)->RemoveItem(5);
	}
	if (0 == caps.max_simultaneous_uavs)
	{
		dialog_oit_->Control<UIComboBox>(id_oit_mode_)->RemoveItem(4);
		dialog_oit_->Control<UIComboBox>(id_oit_mode_)->RemoveItem(3);
	}

	dialog_oit_->Control<UIComboBox>(id_oit_mode_)->OnSelectionChangedEvent().connect(
		[this](UIComboBox const & sender)
		{
			this->OITModeHandler(sender);
		});
	this->OITModeHandler(*dialog_oit_->Control<UIComboBox>(id_oit_mode_));
	dialog_oit_->Control<UISlider>(id_alpha_slider_)->OnValueChangedEvent().connect(
		[this](UISlider const & sender)
		{
			this->AlphaHandler(sender);
		});
	this->AlphaHandler(*dialog_oit_->Control<UISlider>(id_alpha_slider_));

	dialog_layer_->Control<UIComboBox>(id_layer_combo_)->OnSelectionChangedEvent().connect(
		[this](UIComboBox const & sender)
		{
			this->LayerChangedHandler(sender);
		});
	this->LayerChangedHandler(*dialog_layer_->Control<UIComboBox>(id_layer_combo_));

	for (uint32_t i = 0; i < peeled_texs_.size(); ++ i)
	{
		std::wostringstream stream;
		stream << "Layer " << i;
		dialog_layer_->Control<UIComboBox>(id_layer_combo_)->AddItem(stream.str());
	}
}

void OITApp::OnResize(uint32_t width, uint32_t height)
{
	App3DFramework::OnResize(width, height);

	RenderFactory& rf = Context::Instance().RenderFactoryInstance();
	RenderDeviceCaps const & caps = rf.RenderEngineInstance().DeviceCaps();

	ElementFormat ds_format;
	if (caps.rendertarget_format_support(EF_D24S8, 1, 0))
	{
		ds_format = EF_D24S8;
	}
	else
	{
		BOOST_ASSERT(caps.rendertarget_format_support(EF_D16, 1, 0));
		ds_format = EF_D16;
	}
	if (depth_texture_support_)
	{
		for (size_t i = 0; i < depth_texs_.size(); ++ i)
		{
			depth_texs_[i] = rf.MakeTexture2D(width, height, 1, 1, ds_format, 1, 0, EAH_GPU_Read | EAH_GPU_Write);
			depth_views_[i] = rf.Make2DDepthStencilRenderView(*depth_texs_[i], 0, 1, 0);
		}
	}
	else
	{
		ElementFormat depth_format;
		if (caps.rendertarget_format_support(EF_ABGR8, 1, 0))
		{
			depth_format = EF_ABGR8;
		}
		else
		{
			BOOST_ASSERT(caps.rendertarget_format_support(EF_ARGB8, 1, 0));
			depth_format = EF_ARGB8;
		}
		for (size_t i = 0; i < depth_texs_.size(); ++ i)
		{
			depth_texs_[i] = rf.MakeTexture2D(width, height, 1, 1, depth_format, 1, 0, EAH_GPU_Read | EAH_GPU_Write);
			depth_views_[i] = rf.Make2DDepthStencilRenderView(width, height, ds_format, 1, 0);
		}
	}

	ElementFormat peel_format;
	if (caps.rendertarget_format_support(EF_ABGR16F, 1, 0))
	{
		peel_format = EF_ABGR16F;
	}
	else if (caps.rendertarget_format_support(EF_ABGR8, 1, 0))
	{
		peel_format = EF_ABGR8;
	}
	else
	{
		BOOST_ASSERT(caps.rendertarget_format_support(EF_ARGB8, 1, 0));
		peel_format = EF_ARGB8;
	}
	for (size_t i = 0; i < peeling_fbs_.size(); ++ i)
	{
		peeled_texs_[i] = rf.MakeTexture2D(width, height, 1, 1, peel_format, 1, 0, EAH_GPU_Read | EAH_GPU_Write);

		peeling_fbs_[i]->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*peeled_texs_[i], 0, 1, 0));
		peeling_fbs_[i]->Attach(FrameBuffer::ATT_DepthStencil, depth_views_[i % 2]);
	}
	if (!depth_texture_support_)
	{
		for (size_t i = 0; i < depth_fbs_.size(); ++ i)
		{
			depth_fbs_[i]->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*depth_texs_[i], 0, 1, 0));
			depth_fbs_[i]->Attach(FrameBuffer::ATT_DepthStencil, depth_views_[i]);
		}
	}

	{
		accum_tex_ = rf.MakeTexture2D(width, height, 1, 1, EF_ABGR16F, 1, 0, EAH_GPU_Read | EAH_GPU_Write);
		weight_tex_ = rf.MakeTexture2D(width, height, 1, 1, caps.mrt_independent_bit_depths_support ? EF_R16F : EF_ABGR16F,
			1, 0, EAH_GPU_Read | EAH_GPU_Write);
		weighted_fb_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*accum_tex_, 0, 1, 0));
		weighted_fb_->Attach(FrameBuffer::ATT_Color1, rf.Make2DRenderView(*weight_tex_, 0, 1, 0));
	}

	if (caps.max_simultaneous_uavs > 0)
	{
		ElementFormat opaque_bg_format;
		if (caps.rendertarget_format_support(EF_B10G11R11F, 1, 0))
		{
			opaque_bg_format = EF_B10G11R11F;
		}
		else
		{
			opaque_bg_format = peel_format;
		}
		opaque_bg_tex_ = rf.MakeTexture2D(width, height, 1, 1, opaque_bg_format, 1, 0, EAH_GPU_Read | EAH_GPU_Write);
		opaque_bg_fb_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*opaque_bg_tex_, 0, 1, 0));
		opaque_bg_fb_->Attach(FrameBuffer::ATT_DepthStencil, rf.Make2DDepthStencilRenderView(width, height, ds_format, 1, 0));
		frag_link_buf_ = rf.MakeVertexBuffer(BU_Dynamic,
			EAH_GPU_Read | EAH_GPU_Write | EAH_GPU_Unordered | EAH_GPU_Structured | EAH_Counter,
			width * height * 8 * sizeof(float4), nullptr, EF_ABGR32F);
		start_offset_buf_ = rf.MakeVertexBuffer(BU_Dynamic,
			EAH_GPU_Read | EAH_GPU_Write | EAH_GPU_Unordered | EAH_Raw,
			width * height * sizeof(uint32_t), nullptr, EF_R32UI);
		frag_link_uav_ = rf.MakeGraphicsBufferUnorderedAccessView(*frag_link_buf_, EF_ABGR32F);
		start_offset_uav_ = rf.MakeGraphicsBufferUnorderedAccessView(*start_offset_buf_, EF_R32UI);
		linked_list_fb_->AttachUAV(0, frag_link_uav_);
		linked_list_fb_->AttachUAV(1, start_offset_uav_);
		linked_list_fb_->GetViewport()->width = width;
		linked_list_fb_->GetViewport()->height = height;

		checked_pointer_cast<PolygonObject>(polygon_)->BackgroundTex(opaque_bg_tex_);

		if (caps.rovs_support)
		{
			frag_length_buf_ = rf.MakeVertexBuffer(BU_Dynamic,
				EAH_GPU_Read | EAH_GPU_Write | EAH_GPU_Unordered,
				width * height * sizeof(uint32_t), nullptr, EF_R32UI);
			frag_length_uav_ = rf.MakeGraphicsBufferUnorderedAccessView(*frag_length_buf_, EF_R32UI);
		}
	}

	UIManager::Instance().SettleCtrls();
}

void OITApp::InputHandler(InputEngine const & /*sender*/, InputAction const & action)
{
	switch (action.first)
	{
	case Exit:
		this->Quit();
		break;
	}
}

void OITApp::OITModeHandler(KlayGE::UIComboBox const & sender)
{
	oit_mode_ = static_cast<OITMode>(sender.GetSelectedIndex());
	checked_pointer_cast<PolygonObject>(polygon_)->SetOITMode(oit_mode_);
	dialog_layer_->SetVisible(OM_DepthPeeling == oit_mode_);
}

void OITApp::AlphaHandler(KlayGE::UISlider const & sender)
{
	float alpha = sender.GetValue() * 0.01f;
	checked_pointer_cast<PolygonObject>(polygon_)->SetAlpha(alpha);
	std::wostringstream stream;
	stream.precision(2);
	stream << std::fixed << "Alpha: " << alpha;
	dialog_oit_->Control<UIStatic>(id_alpha_static_)->SetText(stream.str());
}

void OITApp::LayerChangedHandler(KlayGE::UIComboBox const & sender)
{
	if (sender.GetSelectedIndex() >= 0)
	{
		dialog_layer_->Control<UITexButton>(id_layer_tex_)->SetTexture(peeled_texs_[sender.GetSelectedIndex()]);
	}
}

void OITApp::DoUpdateOverlay()
{
	SceneManager& sceneMgr(Context::Instance().SceneManagerInstance());

	UIManager::Instance().Render();

	std::wostringstream stream;
	stream.precision(2);
	stream << std::fixed << this->FPS() << " FPS";

	font_->RenderText(0, 0, Color(1, 1, 0, 1), L"Order Independent Transparency", 16);
	font_->RenderText(0, 18, Color(1, 1, 0, 1), stream.str(), 16);

	stream.str(L"");
	stream << sceneMgr.NumRenderablesRendered() << " Renderables "
		<< sceneMgr.NumPrimitivesRendered() << " Primitives "
		<< sceneMgr.NumVerticesRendered() << " Vertices";
	font_->RenderText(0, 36, Color(1, 1, 1, 1), stream.str(), 16);

	if (OM_DepthPeeling == oit_mode_)
	{
		stream.str(L"");
		stream << num_layers_ << " Layers";
		font_->RenderText(0, 54, Color(1, 1, 0, 1), stream.str(), 16);
	}
}

uint32_t OITApp::DoUpdate(uint32_t pass)
{
	RenderEngine& re(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

	if ((OM_PerPixelLinkedLists == oit_mode_) || (OM_AdaptiveTransparency == oit_mode_) || (OM_RovAdaptiveTransparency == oit_mode_))
	{
		checked_pointer_cast<PolygonObject>(polygon_)->LinkedListBuffer(frag_link_buf_,
			(OM_RovAdaptiveTransparency == oit_mode_) ? frag_length_buf_ : start_offset_buf_);

		switch (pass)
		{
		case 0:
			if (OM_PerPixelLinkedLists == oit_mode_)
			{
				re.BindFrameBuffer(opaque_bg_fb_);
			}
			else
			{
				re.BindFrameBuffer(FrameBufferPtr());
			}
			re.CurFrameBuffer()->Attached(FrameBuffer::ATT_DepthStencil)->ClearDepthStencil(1, 0);
			return App3DFramework::URV_OpaqueOnly | App3DFramework::URV_NeedFlush;

		case 1:
			checked_pointer_cast<PolygonObject>(polygon_)->FirstPass(true);
			
			re.BindFrameBuffer(linked_list_fb_);
			{
				if (OM_RovAdaptiveTransparency == oit_mode_)
				{
					linked_list_fb_->AttachUAV(1, frag_length_uav_);
					frag_length_uav_->Clear(uint4(0, 0, 0, 0));
				}
				else
				{
					linked_list_fb_->AttachUAV(1, start_offset_uav_);
					start_offset_uav_->Clear(uint4(0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF));
				}
			}
			return App3DFramework::URV_TransparencyFrontOnly | App3DFramework::URV_NeedFlush;

		default:
			checked_pointer_cast<PolygonObject>(polygon_)->FirstPass(false);

			re.BindFrameBuffer(FrameBufferPtr());
			if (OM_PerPixelLinkedLists == oit_mode_)
			{
				re.CurFrameBuffer()->Attached(FrameBuffer::ATT_DepthStencil)->ClearDepthStencil(1, 0);
			}
			checked_pointer_cast<PolygonObject>(polygon_)->RenderQuad();
			return App3DFramework::URV_Finished;
		}
	}
	else if (OM_WeightedBlended == oit_mode_)
	{
		switch (pass)
		{
		case 0:
			re.BindFrameBuffer(FrameBufferPtr());
			re.CurFrameBuffer()->Attached(FrameBuffer::ATT_DepthStencil)->ClearDepthStencil(1, 0);
			return App3DFramework::URV_OpaqueOnly | App3DFramework::URV_NeedFlush;

		case 1:
			checked_pointer_cast<PolygonObject>(polygon_)->FirstPass(true);

			re.BindFrameBuffer(weighted_fb_);
			re.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color, Color(0, 0, 0, 1), 1, 0);
			return App3DFramework::URV_TransparencyFrontOnly | App3DFramework::URV_NeedFlush;

		default:
			re.BindFrameBuffer(FrameBufferPtr());
			checked_pointer_cast<PolygonObject>(polygon_)->AccumWeightTextures(accum_tex_, weight_tex_);
			checked_pointer_cast<PolygonObject>(polygon_)->RenderQuad();
			return App3DFramework::URV_Finished;
		}
	}
	else if (OM_DepthPeeling == oit_mode_)
	{
		if (0 == pass)
		{
			re.BindFrameBuffer(FrameBufferPtr());
			re.CurFrameBuffer()->Attached(FrameBuffer::ATT_DepthStencil)->ClearDepthStencil(1, 0);
			return App3DFramework::URV_OpaqueOnly | App3DFramework::URV_NeedFlush;
		}
		else
		{
			if (depth_texture_support_)
			{
				if (1 == pass)
				{
					num_layers_ = 1;

					checked_pointer_cast<PolygonObject>(polygon_)->FirstPass(true);
					re.BindFrameBuffer(peeling_fbs_[0]);
					re.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, Color(0, 0, 0, 0), 1, 0);
					return App3DFramework::URV_TransparencyFrontOnly | App3DFramework::URV_NeedFlush;
				}
				else
				{
					checked_pointer_cast<PolygonObject>(polygon_)->FirstPass(false);

					bool finished = false;

					size_t layer_batch = (pass - 2) / oc_queries_.size() * oc_queries_.size() + 1;
					size_t oc_index = (pass - 2) % oc_queries_.size();
					size_t layer = layer_batch + oc_index;
					if (oc_index > 0)
					{
						if (oc_queries_[oc_index - 1])
						{
							oc_queries_[oc_index - 1]->End();
						}
					}
					if ((0 == oc_index) && (layer_batch > 1))
					{
						if (oc_queries_.back())
						{
							oc_queries_.back()->End();
						}
						for (size_t j = 0; j < oc_queries_.size(); ++ j)
						{
							if (oc_queries_[j] && !oc_queries_[j]->AnySamplesPassed())
							{
								finished = true;
							}
							else
							{
								++ num_layers_;
							}
						}
					}
					if (layer_batch < peeled_texs_.size())
					{
						if (!finished)
						{
							checked_pointer_cast<PolygonObject>(polygon_)->LastDepth(depth_texs_[(layer - 1) % 2]);

							re.BindFrameBuffer(peeling_fbs_[layer]);
							peeling_fbs_[layer]->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, Color(0, 0, 0, 0), 1, 0);

							if (oc_queries_[oc_index])
							{
								oc_queries_[oc_index]->Begin();
							}
						}
					}
					else
					{
						finished = true;
					}

					if (finished)
					{
						re.BindFrameBuffer(FrameBufferPtr());
						for (size_t i = 0; i < num_layers_; ++ i)
						{
							blend_pp_->InputPin(0, peeled_texs_[num_layers_ - 1 - i]);
							blend_pp_->Apply();
						}

						return App3DFramework::URV_Finished;
					}
					else
					{
						return App3DFramework::URV_TransparencyFrontOnly | App3DFramework::URV_NeedFlush;
					}
				}
			}
			else
			{
				bool depth_pass = (pass & 1);
				if ((1 == pass) || (2 == pass))
				{
					num_layers_ = 1;

					checked_pointer_cast<PolygonObject>(polygon_)->FirstPass(true);
					checked_pointer_cast<PolygonObject>(polygon_)->DepthPass(depth_pass);
					if (depth_pass)
					{
						re.BindFrameBuffer(depth_fbs_[0]);
						re.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, Color(1, 1, 1, 1), 1, 0);
					}
					else
					{
						re.BindFrameBuffer(peeling_fbs_[0]);
						re.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, Color(0, 0, 0, 0), 1, 0);
					}
					return App3DFramework::URV_TransparencyFrontOnly | App3DFramework::URV_NeedFlush;
				}
				else
				{
					checked_pointer_cast<PolygonObject>(polygon_)->FirstPass(false);
					checked_pointer_cast<PolygonObject>(polygon_)->DepthPass(depth_pass);

					bool finished = false;

					size_t layer_batch = ((pass - 3) / 2) / oc_queries_.size() * oc_queries_.size() + 1;
					size_t oc_index = ((pass - 3) / 2) % oc_queries_.size();
					size_t layer = layer_batch + oc_index;

					if (!depth_pass)
					{
						if (oc_index > 0)
						{
							if (oc_queries_[oc_index - 1])
							{
								oc_queries_[oc_index - 1]->End();
							}
						}
						if ((0 == oc_index) && (layer_batch > 1))
						{
							if (oc_queries_.back())
							{
								oc_queries_.back()->End();
							}
							for (size_t j = 0; j < oc_queries_.size(); ++ j)
							{
								if (oc_queries_[j] && !oc_queries_[j]->AnySamplesPassed())
								{
									finished = true;
								}
								else
								{
									++ num_layers_;
								}
							}
						}
					}
					if (layer_batch < peeled_texs_.size())
					{
						if (!finished)
						{
							checked_pointer_cast<PolygonObject>(polygon_)->LastDepth(depth_texs_[(layer - 1) % 2]);

							if (depth_pass)
							{
								re.BindFrameBuffer(depth_fbs_[layer % 2]);
								re.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, Color(1, 1, 1, 1), 1, 0);
							}
							else
							{
								re.BindFrameBuffer(peeling_fbs_[layer]);
								re.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, Color(0, 0, 0, 0), 1, 0);
							}

							if (!depth_pass)
							{
								if (oc_queries_[oc_index])
								{
									oc_queries_[oc_index]->Begin();
								}
							}
						}
					}
					else
					{
						if (!depth_pass)
						{
							finished = true;
						}
					}

					if (finished)
					{
						re.BindFrameBuffer(FrameBufferPtr());
						for (size_t i = 0; i < num_layers_; ++ i)
						{
							blend_pp_->InputPin(0, peeled_texs_[num_layers_ - 1 - i]);
							blend_pp_->Apply();
						}

						return App3DFramework::URV_Finished;
					}
					else
					{
						return App3DFramework::URV_TransparencyFrontOnly | App3DFramework::URV_NeedFlush;
					}
				}
			}
		}
	}
	else
	{
		checked_pointer_cast<PolygonObject>(polygon_)->FirstPass(true);

		re.BindFrameBuffer(FrameBufferPtr());
		re.CurFrameBuffer()->Attached(FrameBuffer::ATT_DepthStencil)->ClearDepthStencil(1, 0);
		return App3DFramework::URV_NeedFlush | App3DFramework::URV_Finished;
	}
}
