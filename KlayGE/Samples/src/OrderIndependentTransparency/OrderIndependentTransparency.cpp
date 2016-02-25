#include <KlayGE/KlayGE.hpp>
#include <KFL/ThrowErr.hpp>
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
#include "OrderIndependentTransparency.hpp"

using namespace std;
using namespace KlayGE;

namespace
{
	class RenderPolygon : public StaticMesh
	{
	public:
		RenderPolygon(RenderModelPtr const & model, std::wstring const & name)
			: StaticMesh(model, name)
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();
			RenderDeviceCaps const & caps = rf.RenderEngineInstance().DeviceCaps();

			no_oit_tech_ = SyncLoadRenderEffect("NoOIT.fxml")->TechniqueByName("NoOIT");

			dp_1st_tech_ = SyncLoadRenderEffect("DepthPeeling.fxml")->TechniqueByName("DepthPeeling1st");
			dp_nth_tech_ = dp_1st_tech_->Effect().TechniqueByName("DepthPeelingNth");
			if (!caps.depth_texture_support)
			{
				dp_nth_tech_ = dp_1st_tech_->Effect().TechniqueByName("DepthPeelingNthWODepthTexture");
				dp_1st_depth_tech_ = dp_1st_tech_->Effect().TechniqueByName("DepthPeeling1stDepth");
				dp_nth_depth_tech_ = dp_1st_tech_->Effect().TechniqueByName("DepthPeelingNthDepth");
			}

			if (caps.max_simultaneous_uavs > 0)
			{
				gen_ppll_tech_ = SyncLoadRenderEffect("FragmentList.fxml")->TechniqueByName("GenPerPixelLinkedLists");
				rl_quad_ = rf.MakeRenderLayout();
				rl_quad_->TopologyType(RenderLayout::TT_TriangleStrip);
				rl_quad_->NumVertices(4);

				ppll_render_tech_ = SyncLoadRenderEffect("PerPixelLinkedLists.fxml")->TechniqueByName("RenderPerPixelLinkedLists");

				at_render_tech_ = SyncLoadRenderEffect("AdaptiveTransparency.fxml")->TechniqueByName("RenderAdaptiveTransparency");
			}
			
			technique_ = dp_1st_tech_;

			effect_attrs_ = EA_TransparencyFront;
		}

		virtual void DoBuildMeshInfo() override
		{
			AABBox const & pos_bb = this->PosBound();
			*(no_oit_tech_->Effect().ParameterByName("pos_center")) = pos_bb.Center();
			*(no_oit_tech_->Effect().ParameterByName("pos_extent")) = pos_bb.HalfSize();
			*(dp_1st_tech_->Effect().ParameterByName("pos_center")) = pos_bb.Center();
			*(dp_1st_tech_->Effect().ParameterByName("pos_extent")) = pos_bb.HalfSize();
			if (gen_ppll_tech_)
			{
				*(gen_ppll_tech_->Effect().ParameterByName("pos_center")) = pos_bb.Center();
				*(gen_ppll_tech_->Effect().ParameterByName("pos_extent")) = pos_bb.HalfSize();
			}

			AABBox const & tc_bb = this->TexcoordBound();
			*(no_oit_tech_->Effect().ParameterByName("tc_center")) = float2(tc_bb.Center().x(), tc_bb.Center().y());
			*(no_oit_tech_->Effect().ParameterByName("tc_extent")) = float2(tc_bb.HalfSize().x(), tc_bb.HalfSize().y());
			*(dp_1st_tech_->Effect().ParameterByName("tc_center")) = float2(tc_bb.Center().x(), tc_bb.Center().y());
			*(dp_1st_tech_->Effect().ParameterByName("tc_extent")) = float2(tc_bb.HalfSize().x(), tc_bb.HalfSize().y());
			if (gen_ppll_tech_)
			{
				*(gen_ppll_tech_->Effect().ParameterByName("tc_center")) = float2(tc_bb.Center().x(), tc_bb.Center().y());
				*(gen_ppll_tech_->Effect().ParameterByName("tc_extent")) = float2(tc_bb.HalfSize().x(), tc_bb.HalfSize().y());
			}

			TexturePtr diffuse_tex = ASyncLoadTexture("robot-clean_diffuse.dds", EAH_GPU_Read | EAH_Immutable);
			TexturePtr specular_tex = ASyncLoadTexture("robot-clean_specular.dds", EAH_GPU_Read | EAH_Immutable);
			TexturePtr normal_tex = ASyncLoadTexture("robot-clean_normal.dds", EAH_GPU_Read | EAH_Immutable);
			TexturePtr emit_tex = ASyncLoadTexture("robot-clean_selfillumination.dds", EAH_GPU_Read | EAH_Immutable);

			*(no_oit_tech_->Effect().ParameterByName("diffuse_tex")) = diffuse_tex;
			*(no_oit_tech_->Effect().ParameterByName("specular_tex")) = specular_tex;
			*(no_oit_tech_->Effect().ParameterByName("normal_tex")) = normal_tex;
			*(no_oit_tech_->Effect().ParameterByName("emit_tex")) = emit_tex;
			*(dp_1st_tech_->Effect().ParameterByName("diffuse_tex")) = diffuse_tex;
			*(dp_1st_tech_->Effect().ParameterByName("specular_tex")) = specular_tex;
			*(dp_1st_tech_->Effect().ParameterByName("normal_tex")) = normal_tex;
			*(dp_1st_tech_->Effect().ParameterByName("emit_tex")) = emit_tex;
			if (gen_ppll_tech_)
			{
				*(gen_ppll_tech_->Effect().ParameterByName("diffuse_tex")) = diffuse_tex;
				*(gen_ppll_tech_->Effect().ParameterByName("specular_tex")) = specular_tex;
				*(gen_ppll_tech_->Effect().ParameterByName("normal_tex")) = normal_tex;
				*(gen_ppll_tech_->Effect().ParameterByName("emit_tex")) = emit_tex;
			}
		}

		void SetOITMode(OITMode mode)
		{
			mode_ = mode;
		}

		void SetAlpha(float alpha)
		{
			*(no_oit_tech_->Effect().ParameterByName("alpha")) = alpha;
			*(dp_1st_tech_->Effect().ParameterByName("alpha")) = alpha;
			if (gen_ppll_tech_)
			{
				*(gen_ppll_tech_->Effect().ParameterByName("alpha")) = alpha;
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
					technique_ = no_oit_tech_;
					break;

				case OM_DepthPeeling:
					technique_ = dp_1st_tech_;
					break;

				case OM_PerPixelLinkedLists:
				case OM_AdaptiveTransparency:
					technique_ = gen_ppll_tech_;
					break;

				default:
					BOOST_ASSERT(false);
					break;
				}
			}
			else
			{
				switch (mode_)
				{
				case OM_No:
					technique_ = no_oit_tech_;
					break;

				case OM_DepthPeeling:
					technique_ = dp_nth_tech_;
					break;

				case OM_PerPixelLinkedLists:
					technique_ = ppll_render_tech_;
					break;

				case OM_AdaptiveTransparency:
					technique_ = at_render_tech_;
					break;

				default:
					BOOST_ASSERT(false);
					break;
				}
			}
		}

		void DepthPass(bool dp)
		{
			BOOST_ASSERT(OM_DepthPeeling == mode_);

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
				*(dp_nth_tech_->Effect().ParameterByName("last_depth_tex")) = depth_tex;
			}
		}

		void BackgroundTex(TexturePtr const & bg_tex)
		{
			if (ppll_render_tech_)
			{
				*(ppll_render_tech_->Effect().ParameterByName("bg_tex")) = bg_tex;
			}
		}
		void LinkedListBuffer(GraphicsBufferPtr const & fragment_link_buf, GraphicsBufferPtr const & start_offset_buf)
		{
			if (gen_ppll_tech_)
			{
				*(gen_ppll_tech_->Effect().ParameterByName("rw_frags_buffer")) = fragment_link_buf;
				*(gen_ppll_tech_->Effect().ParameterByName("rw_start_offset_buffer")) = start_offset_buf;
			}

			if (ppll_render_tech_)
			{
				*(ppll_render_tech_->Effect().ParameterByName("frags_buffer")) = fragment_link_buf;
				*(ppll_render_tech_->Effect().ParameterByName("start_offset_buffer")) = start_offset_buf;
			}
			if (at_render_tech_)
			{
				*(at_render_tech_->Effect().ParameterByName("frags_buffer")) = fragment_link_buf;
				*(at_render_tech_->Effect().ParameterByName("start_offset_buffer")) = start_offset_buf;
			}
		}

		void RenderQuad()
		{
			RenderTechniquePtr tech;
			RenderLayoutPtr rl;
			if (OM_PerPixelLinkedLists == mode_)
			{
				tech = ppll_render_tech_;
				rl = rl_quad_;
			}
			else if (OM_AdaptiveTransparency == mode_)
			{
				tech = at_render_tech_;
				rl = rl_quad_;
			}

			RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
			*(tech->Effect().ParameterByName("frame_width")) = static_cast<int32_t>(re.CurFrameBuffer()->GetViewport()->width);

			re.Render(*tech, *rl);
		}

		void OnRenderBegin()
		{
			App3DFramework const & app = Context::Instance().AppInstance();
			Camera const & camera = app.ActiveCamera();

			float4x4 const & model = float4x4::Identity();

			*(technique_->Effect().ParameterByName("mvp")) = model * camera.ViewProjMatrix();
			*(technique_->Effect().ParameterByName("eye_pos")) = camera.EyePos();

			switch (mode_)
			{
			case OM_No:
				break;

			case OM_DepthPeeling:
				{
					float q = camera.FarPlane() / (camera.FarPlane() - camera.NearPlane());
					*(technique_->Effect().ParameterByName("near_q")) = float2(camera.NearPlane() * q, q);
					*(technique_->Effect().ParameterByName("far_plane")) = float2(camera.FarPlane(), 1.0f / camera.FarPlane());
					break;
				}
			
			case OM_PerPixelLinkedLists:
			case OM_AdaptiveTransparency:
				{
					RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
					*(technique_->Effect().ParameterByName("frame_width")) = static_cast<int32_t>(re.CurFrameBuffer()->GetViewport()->width);
					break;
				}

			default:
				BOOST_ASSERT(false);
				break;
			}
		}

		void LightPos(float3 const & light_pos)
		{
			*(no_oit_tech_->Effect().ParameterByName("light_pos")) = light_pos;
			*(dp_1st_tech_->Effect().ParameterByName("light_pos")) = light_pos;
			if (gen_ppll_tech_)
			{
				*(gen_ppll_tech_->Effect().ParameterByName("light_pos")) = light_pos;
			}
		}

	private:
		OITMode mode_;
		bool first_pass_;
		
		RenderTechniquePtr no_oit_tech_;

		RenderTechniquePtr dp_1st_tech_;
		RenderTechniquePtr dp_nth_tech_;
		RenderTechniquePtr dp_1st_depth_tech_;
		RenderTechniquePtr dp_nth_depth_tech_;

		RenderTechniquePtr gen_ppll_tech_;
		RenderLayoutPtr rl_quad_;

		RenderTechniquePtr ppll_render_tech_;
		
		RenderTechniquePtr at_render_tech_;
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
	OrderIndependentTransparencyApp app;
	app.Create();
	app.Run();

	return 0;
}

OrderIndependentTransparencyApp::OrderIndependentTransparencyApp()
			: App3DFramework("Order Independent Transparency"),
				num_layers_(0)
{
	ResLoader::Instance().AddPath("../../Samples/media/OrderIndependentTransparency");
}

bool OrderIndependentTransparencyApp::ConfirmDevice() const
{
	return true;
}

void OrderIndependentTransparencyApp::OnCreate()
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
	actionMap.AddActions(actions, actions + sizeof(actions) / sizeof(actions[0]));

	action_handler_t input_handler = MakeSharedPtr<input_signal>();
	input_handler->connect(std::bind(&OrderIndependentTransparencyApp::InputHandler, this, std::placeholders::_1, std::placeholders::_2));
	inputEngine.ActionMap(actionMap, input_handler);

	blend_pp_ = SyncLoadPostProcess("Blend.ppml", "blend");

	UIManager::Instance().Load(ResLoader::Instance().Open("OrderIndependentTransparency.uiml"));
	dialog_oit_ = UIManager::Instance().GetDialogs()[0];
	dialog_layer_ = UIManager::Instance().GetDialogs()[1];

	id_oit_mode_ = dialog_oit_->IDFromName("OITMode");
	id_alpha_static_ = dialog_oit_->IDFromName("AlphaStatic");
	id_alpha_slider_ = dialog_oit_->IDFromName("Alpha");
	id_layer_combo_ = dialog_layer_->IDFromName("LayerCombo");
	id_layer_tex_ = dialog_layer_->IDFromName("LayerTexButton");

	if (0 == caps.max_simultaneous_uavs)
	{
		dialog_oit_->Control<UIComboBox>(id_oit_mode_)->RemoveItem(3);
		dialog_oit_->Control<UIComboBox>(id_oit_mode_)->RemoveItem(2);
	}

	dialog_oit_->Control<UIComboBox>(id_oit_mode_)->OnSelectionChangedEvent().connect(std::bind(&OrderIndependentTransparencyApp::OITModeHandler, this, std::placeholders::_1));
	this->OITModeHandler(*dialog_oit_->Control<UIComboBox>(id_oit_mode_));
	dialog_oit_->Control<UISlider>(id_alpha_slider_)->OnValueChangedEvent().connect(std::bind(&OrderIndependentTransparencyApp::AlphaHandler, this, std::placeholders::_1));
	this->AlphaHandler(*dialog_oit_->Control<UISlider>(id_alpha_slider_));

	dialog_layer_->Control<UIComboBox>(id_layer_combo_)->OnSelectionChangedEvent().connect(std::bind(&OrderIndependentTransparencyApp::LayerChangedHandler, this, std::placeholders::_1));
	this->LayerChangedHandler(*dialog_layer_->Control<UIComboBox>(id_layer_combo_));

	for (uint32_t i = 0; i < peeled_texs_.size(); ++ i)
	{
		std::wostringstream stream;
		stream << "Layer " << i;
		dialog_layer_->Control<UIComboBox>(id_layer_combo_)->AddItem(stream.str());
	}
}

void OrderIndependentTransparencyApp::OnResize(uint32_t width, uint32_t height)
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
			depth_texs_[i] = rf.MakeTexture2D(width, height, 1, 1, ds_format, 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);
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
			depth_texs_[i] = rf.MakeTexture2D(width, height, 1, 1, depth_format, 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);
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
		peeled_texs_[i] = rf.MakeTexture2D(width, height, 1, 1, peel_format, 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);

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

	if (caps.max_simultaneous_uavs > 0)
	{
		ElementFormat opaque_bg_format;
		if (rf.RenderEngineInstance().DeviceCaps().rendertarget_format_support(EF_B10G11R11F, 1, 0))
		{
			opaque_bg_format = EF_B10G11R11F;
		}
		else
		{
			opaque_bg_format = peel_format;
		}
		opaque_bg_tex_ = rf.MakeTexture2D(width, height, 1, 1, opaque_bg_format, 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);
		opaque_bg_fb_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*opaque_bg_tex_, 0, 1, 0));
		opaque_bg_fb_->Attach(FrameBuffer::ATT_DepthStencil, rf.Make2DDepthStencilRenderView(width, height, ds_format, 1, 0));
		frag_link_buf_ = rf.MakeVertexBuffer(BU_Dynamic,
			EAH_GPU_Read | EAH_GPU_Write | EAH_GPU_Unordered | EAH_GPU_Structured | EAH_Counter,
			width * height * 4 * sizeof(float4), nullptr, EF_ABGR32F);
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
	}

	UIManager::Instance().SettleCtrls();
}

void OrderIndependentTransparencyApp::InputHandler(InputEngine const & /*sender*/, InputAction const & action)
{
	switch (action.first)
	{
	case Exit:
		this->Quit();
		break;
	}
}

void OrderIndependentTransparencyApp::OITModeHandler(KlayGE::UIComboBox const & sender)
{
	oit_mode_ = static_cast<OITMode>(sender.GetSelectedIndex());
	checked_pointer_cast<PolygonObject>(polygon_)->SetOITMode(oit_mode_);
	dialog_layer_->SetVisible(OM_DepthPeeling == oit_mode_);
}

void OrderIndependentTransparencyApp::AlphaHandler(KlayGE::UISlider const & sender)
{
	float alpha = sender.GetValue() * 0.01f;
	checked_pointer_cast<PolygonObject>(polygon_)->SetAlpha(alpha);
	std::wostringstream stream;
	stream.precision(2);
	stream << std::fixed << "Alpha: " << alpha;
	dialog_oit_->Control<UIStatic>(id_alpha_static_)->SetText(stream.str());
}

void OrderIndependentTransparencyApp::LayerChangedHandler(KlayGE::UIComboBox const & sender)
{
	if (sender.GetSelectedIndex() >= 0)
	{
		dialog_layer_->Control<UITexButton>(id_layer_tex_)->SetTexture(peeled_texs_[sender.GetSelectedIndex()]);
	}
}

void OrderIndependentTransparencyApp::DoUpdateOverlay()
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

uint32_t OrderIndependentTransparencyApp::DoUpdate(uint32_t pass)
{
	RenderEngine& re(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

	if ((OM_PerPixelLinkedLists == oit_mode_) || (OM_AdaptiveTransparency == oit_mode_))
	{
		checked_pointer_cast<PolygonObject>(polygon_)->LinkedListBuffer(frag_link_buf_, start_offset_buf_);

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
			start_offset_uav_->Clear(uint4(0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF));
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
