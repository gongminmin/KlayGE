#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/Math.hpp>
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
#include <KlayGE/KMesh.hpp>
#include <KlayGE/GraphicsBuffer.hpp>
#include <KlayGE/SceneObjectHelper.hpp>
#include <KlayGE/Query.hpp>
#include <KlayGE/PostProcess.hpp>

#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/InputFactory.hpp>

#include <vector>
#include <sstream>
#include <ctime>
#include <boost/bind.hpp>

#include "DepthPeeling.hpp"

using namespace std;
using namespace KlayGE;

namespace
{
	class RenderPolygon : public KMesh
	{
	public:
		RenderPolygon(RenderModelPtr model, std::wstring const & name)
			: KMesh(model, name)
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();

			technique_ = rf.LoadEffect("DepthPeeling.fxml")->TechniqueByName("DepthPeeling1st");
		}

		void BuildMeshInfo()
		{
		}

		void DepthPeelingEnabled(bool dp)
		{
			if (!dp)
			{
				technique_ = technique_->Effect().TechniqueByName("NoDepthPeeling");
			}
		}

		void FirstPass(bool fp)
		{
			if (fp)
			{
				technique_ = technique_->Effect().TechniqueByName("DepthPeeling1st");
			}
			else
			{
				technique_ = technique_->Effect().TechniqueByName("DepthPeelingNth");
			}
		}

		void LastDepth(TexturePtr depth_tex, bool flip)
		{
			*(technique_->Effect().ParameterByName("last_depth_tex")) = depth_tex;
			*(technique_->Effect().ParameterByName("flip")) = static_cast<int32_t>(flip ? -1 : 1);
		}

		void OnRenderBegin()
		{
			App3DFramework const & app = Context::Instance().AppInstance();

			float4x4 const & model = float4x4::Identity();
			float4x4 const & view = app.ActiveCamera().ViewMatrix();
			float4x4 const & proj = app.ActiveCamera().ProjMatrix();

			*(technique_->Effect().ParameterByName("mvp")) = model * view * proj;
			*(technique_->Effect().ParameterByName("inv_depth_range")) = 1 / app.ActiveCamera().FarPlane();

			RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
			float4 const & texel_to_pixel = re.TexelToPixelOffset() * 2;
			float const x_offset = texel_to_pixel.x() / re.CurFrameBuffer()->Width();
			float const y_offset = texel_to_pixel.y() / re.CurFrameBuffer()->Height();
			*(technique_->Effect().ParameterByName("offset")) = float2(x_offset, y_offset);
		}

		void LightPos(float3 const & light_pos)
		{
			*(technique_->Effect().ParameterByName("light_pos")) = light_pos;
		}

	private:
		bool flip;
	};

	class PolygonObject : public SceneObjectHelper
	{
	public:
		PolygonObject()
			: SceneObjectHelper(SOA_Cullable)
		{
			renderable_ = LoadModel("teapot.meshml", EAH_GPU_Read, CreateKModelFactory<RenderModel>(), CreateKMeshFactory<RenderPolygon>())();
		}

		void LightPos(float3 const & light_pos)
		{
			RenderModelPtr model = checked_pointer_cast<RenderModel>(renderable_);
			for (uint32_t i = 0; i < model->NumMeshes(); ++ i)
			{
				checked_pointer_cast<RenderPolygon>(model->Mesh(i))->LightPos(light_pos);
			}
		}

		void DepthPeelingEnabled(bool dp)
		{
			RenderModelPtr model = checked_pointer_cast<RenderModel>(renderable_);
			for (uint32_t i = 0; i < model->NumMeshes(); ++ i)
			{
				checked_pointer_cast<RenderPolygon>(model->Mesh(i))->DepthPeelingEnabled(dp);
			}
		}

		void FirstPass(bool fp)
		{
			RenderModelPtr model = checked_pointer_cast<RenderModel>(renderable_);
			for (uint32_t i = 0; i < model->NumMeshes(); ++ i)
			{
				checked_pointer_cast<RenderPolygon>(model->Mesh(i))->FirstPass(fp);
			}
		}

		void LastDepth(TexturePtr depth_tex, bool flip)
		{
			RenderModelPtr model = checked_pointer_cast<RenderModel>(renderable_);
			for (uint32_t i = 0; i < model->NumMeshes(); ++ i)
			{
				checked_pointer_cast<RenderPolygon>(model->Mesh(i))->LastDepth(depth_tex, flip);
			}
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

	bool ConfirmDevice()
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		RenderEngine& re = rf.RenderEngineInstance();
		RenderDeviceCaps const & caps = re.DeviceCaps();
		if (caps.max_shader_model < 2)
		{
			return false;
		}
		if (caps.max_simultaneous_rts < 2)
		{
			return false;
		}

		try
		{
			TexturePtr temp_tex = rf.MakeTexture2D(800, 600, 1, 1, EF_R32F, 1, 0, EAH_GPU_Read | EAH_GPU_Write, NULL);
			rf.Make2DRenderView(*temp_tex, 0, 0);
		}
		catch (...)
		{
			return false;
		}

		return true;
	}
}


int main()
{
	ResLoader::Instance().AddPath("../Samples/media/Common");

	ContextCfg context_cfg = Context::Instance().LoadCfg("KlayGE.cfg");
	context_cfg.graphics_cfg.ConfirmDevice = ConfirmDevice;

	Context::Instance().Config(context_cfg);

	DepthPeelingApp app;
	app.Create();
	app.Run();

	return 0;
}

DepthPeelingApp::DepthPeelingApp()
			: App3DFramework("DepthPeeling"),
				num_layers_(0)
{
	ResLoader::Instance().AddPath("../Samples/media/DepthPeeling");
}

void DepthPeelingApp::InitObjects()
{
	// ½¨Á¢×ÖÌå
	font_ = Context::Instance().RenderFactoryInstance().MakeFont("gkai00mp.kfont");

	polygon_ = MakeSharedPtr<PolygonObject>();
	checked_pointer_cast<PolygonObject>(polygon_)->LightPos(float3(0, 2, -1));
	polygon_->AddToSceneManager();

	this->LookAt(float3(-0.3f, 0.4f, -0.3f), float3(0, 0, 0));
	this->Proj(0.01f, 100);

	RenderFactory& rf = Context::Instance().RenderFactoryInstance();
	RenderEngine& re = rf.RenderEngineInstance();

	peeling_fbs_.resize(9);
	for (size_t i = 0; i < peeling_fbs_.size(); ++ i)
	{
		peeling_fbs_[i] = rf.MakeFrameBuffer();
		peeling_fbs_[i]->GetViewport().camera = re.CurFrameBuffer()->GetViewport().camera;
	}
	peeled_texs_.resize(peeling_fbs_.size());
	peeled_views_.resize(peeled_texs_.size());

	for (size_t i = 0; i < oc_queries_.size(); ++ i)
	{
		oc_queries_[i] = rf.MakeConditionalRender();
	}

	fpcController_.Scalers(0.05f, 0.01f);

	InputEngine& inputEngine(Context::Instance().InputFactoryInstance().InputEngineInstance());
	InputActionMap actionMap;
	actionMap.AddActions(actions, actions + sizeof(actions) / sizeof(actions[0]));

	action_handler_t input_handler = MakeSharedPtr<input_signal>();
	input_handler->connect(boost::bind(&DepthPeelingApp::InputHandler, this, _1, _2));
	inputEngine.ActionMap(actionMap, input_handler, true);

	blend_pp_ = LoadPostProcess(ResLoader::Instance().Load("Blend.ppml"), "blend");

	UIManager::Instance().Load(ResLoader::Instance().Load("DepthPeeling.uiml"));
	dialog_peeling_ = UIManager::Instance().GetDialogs()[0];
	dialog_layer_ = UIManager::Instance().GetDialogs()[1];

	id_use_depth_peeling_ = dialog_peeling_->IDFromName("UseDepthPeeling");
	id_ctrl_camera_ = dialog_peeling_->IDFromName("CtrlCamera");
	id_layer_combo_ = dialog_layer_->IDFromName("LayerCombo");
	id_layer_tex_ = dialog_layer_->IDFromName("LayerTexButton");

	dialog_peeling_->Control<UICheckBox>(id_use_depth_peeling_)->OnChangedEvent().connect(boost::bind(&DepthPeelingApp::UsePeelingHandler, this, _1));
	this->UsePeelingHandler(*dialog_peeling_->Control<UICheckBox>(id_use_depth_peeling_));
	dialog_peeling_->Control<UICheckBox>(id_ctrl_camera_)->OnChangedEvent().connect(boost::bind(&DepthPeelingApp::CtrlCameraHandler, this, _1));
	this->CtrlCameraHandler(*dialog_peeling_->Control<UICheckBox>(id_ctrl_camera_));

	dialog_layer_->Control<UIComboBox>(id_layer_combo_)->OnSelectionChangedEvent().connect(boost::bind(&DepthPeelingApp::LayerChangedHandler, this, _1));
	this->LayerChangedHandler(*dialog_layer_->Control<UIComboBox>(id_layer_combo_));

	for (uint32_t i = 0; i < peeled_texs_.size(); ++ i)
	{
		std::wostringstream stream;
		stream << i << " Layer";
		dialog_layer_->Control<UIComboBox>(id_layer_combo_)->AddItem(stream.str());
	}
}

void DepthPeelingApp::OnResize(uint32_t width, uint32_t height)
{
	App3DFramework::OnResize(width, height);

	RenderFactory& rf = Context::Instance().RenderFactoryInstance();

	ElementFormat peel_format;
	try
	{
		depth_texs_[0] = rf.MakeTexture2D(width, height, 1, 1, EF_R32F, 1, 0, EAH_GPU_Read | EAH_GPU_Write, NULL);
		depth_view_[0] = rf.Make2DRenderView(*depth_texs_[0], 0, 0);
		if (rf.RenderEngineInstance().DeviceCaps().argb8_support)
		{
			peel_format = EF_ARGB8;
		}
		else
		{
			peel_format = EF_ABGR8;
		}
	}
	catch (...)
	{
		depth_texs_[0] = rf.MakeTexture2D(width, height, 1, 1, EF_ABGR32F, 1, 0, EAH_GPU_Read | EAH_GPU_Write, NULL);
		depth_view_[0] = rf.Make2DRenderView(*depth_texs_[0], 0, 0);
		peel_format = EF_ABGR32F;
	}
	depth_texs_[1] = rf.MakeTexture2D(width, height, 1, 1, depth_texs_[0]->Format(), 1, 0, EAH_GPU_Read | EAH_GPU_Write, NULL);
	depth_view_[1] = rf.Make2DRenderView(*depth_texs_[1], 0, 0);

	peeled_depth_view_ = rf.Make2DDepthStencilRenderView(width, height, EF_D16, 1, 0);

	for (size_t i = 0; i < peeling_fbs_.size(); ++ i)
	{
		peeled_texs_[i] = rf.MakeTexture2D(width, height, 1, 1, peel_format, 1, 0, EAH_GPU_Read | EAH_GPU_Write, NULL);
		peeled_views_[i] = rf.Make2DRenderView(*peeled_texs_[i], 0, 0);

		peeling_fbs_[i]->Attach(FrameBuffer::ATT_Color0, peeled_views_[i]);
		peeling_fbs_[i]->Attach(FrameBuffer::ATT_Color1, depth_view_[i % 2]);
		peeling_fbs_[i]->Attach(FrameBuffer::ATT_DepthStencil, peeled_depth_view_);
	}

	UIManager::Instance().SettleCtrls(width, height);
}

void DepthPeelingApp::InputHandler(InputEngine const & /*sender*/, InputAction const & action)
{
	switch (action.first)
	{
	case Exit:
		this->Quit();
		break;
	}
}

void DepthPeelingApp::UsePeelingHandler(UICheckBox const & sender)
{
	use_depth_peeling_ = sender.GetChecked();
}

void DepthPeelingApp::CtrlCameraHandler(KlayGE::UICheckBox const & sender)
{
	if (sender.GetChecked())
	{
		fpcController_.AttachCamera(this->ActiveCamera());
	}
	else
	{
		fpcController_.DetachCamera();
	}
}

void DepthPeelingApp::LayerChangedHandler(KlayGE::UIComboBox const & sender)
{
	if (sender.GetSelectedIndex() >= 0)
	{
		dialog_layer_->Control<UITexButton>(id_layer_tex_)->SetTexture(peeled_texs_[sender.GetSelectedIndex()]);
	}
}

void DepthPeelingApp::DoUpdateOverlay()
{
	SceneManager& sceneMgr(Context::Instance().SceneManagerInstance());

	UIManager::Instance().Render();

	std::wostringstream stream;
	stream << this->FPS();

	font_->RenderText(0, 0, Color(1, 1, 0, 1), L"Depth Peeling", 16);
	font_->RenderText(0, 18, Color(1, 1, 0, 1), stream.str(), 16);

	stream.str(L"");
	stream << sceneMgr.NumRenderablesRendered() << " Renderables "
		<< sceneMgr.NumPrimitivesRendered() << " Primitives "
		<< sceneMgr.NumVerticesRendered() << " Vertices";
	font_->RenderText(0, 36, Color(1, 1, 1, 1), stream.str(), 16);

	stream.str(L"");
	stream << num_layers_ << " Layers";
	font_->RenderText(0, 54, Color(1, 1, 0, 1), stream.str(), 16);
}

uint32_t DepthPeelingApp::DoUpdate(uint32_t pass)
{
	RenderEngine& re(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

	if (use_depth_peeling_)
	{
		switch (pass)
		{
		case 0:
			num_layers_ = 1;

			checked_pointer_cast<PolygonObject>(polygon_)->FirstPass(true);
			re.BindFrameBuffer(peeling_fbs_[0]);
			peeling_fbs_[0]->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, Color(0, 0, 0, 0), 1, 0);
			depth_view_[0]->ClearColor(Color(1, 0, 0, 0));
			return App3DFramework::URV_Need_Flush;

		default:
			if (1 == pass)
			{
				checked_pointer_cast<PolygonObject>(polygon_)->FirstPass(false);
			}

			{
				bool finished = false;

				size_t layer_batch = (pass - 1) / oc_queries_.size() * oc_queries_.size() + 1;
				size_t oc_index = (pass - 1) % oc_queries_.size();
				size_t layer = layer_batch + oc_index;
				if (oc_index > 0)
				{
					oc_queries_[oc_index - 1]->End();
				}
				if ((oc_index == 0) && (layer_batch > 1))
				{
					oc_queries_.back()->End();
					for (size_t j = 0; j < oc_queries_.size(); ++ j)
					{
						if (!checked_pointer_cast<ConditionalRender>(oc_queries_[j])->AnySamplesPassed())
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
						checked_pointer_cast<PolygonObject>(polygon_)->LastDepth(depth_texs_[(layer - 1) % 2], peeling_fbs_[layer - 1]->RequiresFlipping());

						re.BindFrameBuffer(peeling_fbs_[layer]);
						peeling_fbs_[layer]->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, Color(0, 0, 0, 0), 1, 0);
						depth_view_[layer % 2]->ClearColor(Color(1, 0, 0, 0));

						oc_queries_[oc_index]->Begin();
					}
				}
				else
				{
					finished = true;
				}

				if (finished)
				{
					re.BindFrameBuffer(FrameBufferPtr());
					re.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, Color(0.2f, 0.4f, 0.6f, 1), 1, 0);
					for (size_t i = 0; i < num_layers_; ++ i)
					{
						blend_pp_->InputPin(0, peeled_texs_[num_layers_ - 1 - i]);
						blend_pp_->Apply();
					}

					return App3DFramework::URV_Finished;
				}
				else
				{
					return App3DFramework::URV_Need_Flush;
				}
			}
		}
	}
	else
	{
		checked_pointer_cast<PolygonObject>(polygon_)->DepthPeelingEnabled(false);

		re.BindFrameBuffer(FrameBufferPtr());
		re.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, Color(0.2f, 0.4f, 0.6f, 1), 1, 0);
		return App3DFramework::URV_Need_Flush | App3DFramework::URV_Finished;
	}
}
