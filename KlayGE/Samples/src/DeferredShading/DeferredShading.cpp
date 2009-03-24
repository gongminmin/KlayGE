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
#include <KlayGE/SceneObjectHelper.hpp>
#include <KlayGE/PostProcess.hpp>
#include <KlayGE/HDRPostProcess.hpp>
#include <KlayGE/Util.hpp>

#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/InputFactory.hpp>

#include <sstream>
#include <ctime>
#include <boost/bind.hpp>
#include <boost/typeof/typeof.hpp>

#include "DeferredShading.hpp"

using namespace std;
using namespace KlayGE;

namespace
{
	class RenderTorus : public KMesh
	{
	public:
		RenderTorus(RenderModelPtr model, std::wstring const & name)
			: KMesh(model, name)
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();

			effect_ = rf.LoadEffect("DeferredShading.kfx");
		}

		void BuildMeshInfo()
		{
			std::map<std::string, TexturePtr> tex_pool;

			bool has_bump_map = false;
			for (StaticMesh::TextureSlotsType::iterator iter = texture_slots_.begin();
				iter != texture_slots_.end(); ++ iter)
			{
				TexturePtr tex;
				BOOST_AUTO(titer, tex_pool.find(iter->second));
				if (titer != tex_pool.end())
				{
					tex = titer->second;
				}
				else
				{
					tex = LoadTexture(iter->second, EAH_GPU_Read)();
					tex_pool.insert(std::make_pair(iter->second, tex));
				}

				if ("Diffuse Color" == iter->first)
				{
					*(effect_->ParameterByName("diffuse_tex")) = tex;
				}
				if ("Bump" == iter->first)
				{
					*(effect_->ParameterByName("bump_tex")) = tex;
					if (tex)
					{
						has_bump_map = true;
					}
				}
			}

			if (has_bump_map)
			{
				technique_ = effect_->TechniqueByName("GBufferTech");
			}
			else
			{
				technique_ = effect_->TechniqueByName("GBufferNoBumpTech");
			}
		}

		void OnRenderBegin()
		{
			Camera const & camera = Context::Instance().AppInstance().ActiveCamera();

			float4x4 const & view = camera.ViewMatrix();
			float4x4 const & proj = camera.ProjMatrix();

			*(technique_->Effect().ParameterByName("proj")) = proj;
			*(technique_->Effect().ParameterByName("model_view")) = view;

			*(technique_->Effect().ParameterByName("depth_near_far_invfar")) = float3(camera.NearPlane(), camera.FarPlane(), 1 / camera.FarPlane());
			*(technique_->Effect().ParameterByName("light_in_eye")) = MathLib::transform_coord(float3(2, 2, -3), view);
		}

	private:
		KlayGE::RenderEffectPtr effect_;
	};

	class TorusObject : public SceneObjectHelper
	{
	public:
		TorusObject()
			: SceneObjectHelper(SOA_Cullable)
		{
			renderable_ = LoadKModel("sponza.kmodel", EAH_GPU_Read, CreateKModelFactory<RenderModel>(), CreateKMeshFactory<RenderTorus>());
		}
	};


	class DeferredShadingPostProcess : public PostProcess
	{
	public:
		DeferredShadingPostProcess()
			: PostProcess(Context::Instance().RenderFactoryInstance().LoadEffect("DeferredShading.kfx")->TechniqueByName("DeferredShading"))
		{
		}

		void Source(TexturePtr const & tex, bool flipping)
		{
			PostProcess::Source(tex, flipping);
			if (tex)
			{
				*(technique_->Effect().ParameterByName("inv_width_height")) = float2(1.0f / tex->Width(0), 1.0f / tex->Height(0));
			}
		}

		void ColorTex(TexturePtr const & tex)
		{
			*(technique_->Effect().ParameterByName("color_tex")) = tex;
		}

		void SSAOTex(TexturePtr const & tex)
		{
			*(technique_->Effect().ParameterByName("ssao_tex")) = tex;
		}

		void SSAOEnabled(bool ssao)
		{
			*(technique_->Effect().ParameterByName("ssao_enabled")) = ssao;
		}

		void BufferType(int buffer_type)
		{
			switch (buffer_type)
			{
			case 0:
				technique_ = technique_->Effect().TechniqueByName("DeferredShading");
				break;

			case 1:
				technique_ = technique_->Effect().TechniqueByName("ShowPosition");
				break;

			case 2:
				technique_ = technique_->Effect().TechniqueByName("ShowNormal");
				break;

			case 3:
				technique_ = technique_->Effect().TechniqueByName("ShowDepth");
				break;

			case 4:
				technique_ = technique_->Effect().TechniqueByName("ShowDiffuse");
				break;

			case 5:
				technique_ = technique_->Effect().TechniqueByName("ShowSpecular");
				break;

			case 6:
				technique_ = technique_->Effect().TechniqueByName("ShowEdge");
				break;

			case 7:
				technique_ = technique_->Effect().TechniqueByName("ShowSSAO");
				break;

			default:
				break;
			}
		}

		void OnRenderBegin()
		{
			PostProcess::OnRenderBegin();

			Camera const & camera = Context::Instance().AppInstance().ActiveCamera();

			float4x4 const & view = camera.ViewMatrix();
			float4x4 const & proj = camera.ProjMatrix();
			float4x4 const inv_proj = MathLib::inverse(proj);

			*(technique_->Effect().ParameterByName("proj")) = proj;

			*(technique_->Effect().ParameterByName("depth_near_far_invfar")) = float3(camera.NearPlane(), camera.FarPlane(), 1 / camera.FarPlane());
			*(technique_->Effect().ParameterByName("light_in_eye")) = MathLib::transform_coord(float3(2, 10, 0), view);

			*(technique_->Effect().ParameterByName("upper_left")) = MathLib::transform_coord(float3(-1, 1, 1), inv_proj);
			*(technique_->Effect().ParameterByName("upper_right")) = MathLib::transform_coord(float3(1, 1, 1), inv_proj);
			*(technique_->Effect().ParameterByName("lower_left")) = MathLib::transform_coord(float3(-1, -1, 1), inv_proj);
			*(technique_->Effect().ParameterByName("lower_right")) = MathLib::transform_coord(float3(1, -1, 1), inv_proj);
		}
	};

	class AntiAliasPostProcess : public PostProcess
	{
	public:
		AntiAliasPostProcess()
			: PostProcess(Context::Instance().RenderFactoryInstance().LoadEffect("DeferredShading.kfx")->TechniqueByName("AntiAlias"))
		{
		}

		void Source(TexturePtr const & tex, bool flipping)
		{
			PostProcess::Source(tex, flipping);
			if (tex)
			{
				*(technique_->Effect().ParameterByName("inv_width_height")) = float2(1.0f / tex->Width(0), 1.0f / tex->Height(0));
			}
		}

		void ColorTex(TexturePtr const & tex)
		{
			*(technique_->Effect().ParameterByName("color_tex")) = tex;
		}
	};

	class SSAOPostProcess : public PostProcess
	{
	public:
		SSAOPostProcess()
			: PostProcess(Context::Instance().RenderFactoryInstance().LoadEffect("DeferredShading.kfx")->TechniqueByName("SSAO"))
		{
			*(technique_->Effect().ParameterByName("ssao_param")) = float4(0.6f, 0.075f, 0.3f, 0.03f);
		}

		void OnRenderBegin()
		{
			PostProcess::OnRenderBegin();

			Camera const & camera = Context::Instance().AppInstance().ActiveCamera();
			*(technique_->Effect().ParameterByName("depth_near_far_invfar")) = float3(camera.NearPlane(), camera.FarPlane(), 1 / camera.FarPlane());
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
			TexturePtr temp_tex = rf.MakeTexture2D(800, 600, 1, EF_ABGR16F, 1, 0, EAH_GPU_Read | EAH_GPU_Write, NULL);
			rf.Make2DRenderView(*temp_tex, 0);
			rf.MakeDepthStencilRenderView(800, 600, EF_D16, 1, 0);
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
	ResLoader::Instance().AddPath("../Samples/media/DeferredShading");

	RenderSettings settings = Context::Instance().LoadCfg("KlayGE.cfg");
	settings.ConfirmDevice = ConfirmDevice;

	DeferredShadingApp app("DeferredShading", settings);
	app.Create();
	app.Run();

	return 0;
}

DeferredShadingApp::DeferredShadingApp(std::string const & name, RenderSettings const & settings)
			: App3DFramework(name, settings),
				anti_alias_enabled_(true), ssao_enabled_(true)
{
}

void DeferredShadingApp::InitObjects()
{
	font_ = Context::Instance().RenderFactoryInstance().MakeFont("gkai00mp.kfont");

	torus_.reset(new TorusObject);
	torus_->AddToSceneManager();

	this->LookAt(float3(-2, 2, 0), float3(0, 2, 0));
	this->Proj(0.1f, 100.0f);

	RenderFactory& rf = Context::Instance().RenderFactoryInstance();
	RenderEngine& re = rf.RenderEngineInstance();
	
	g_buffer_ = rf.MakeFrameBuffer();
	g_buffer_->GetViewport().camera = re.CurFrameBuffer()->GetViewport().camera;

	shaded_buffer_ = rf.MakeFrameBuffer();
	shaded_buffer_->GetViewport().camera = re.CurFrameBuffer()->GetViewport().camera;

	ssao_buffer_ = rf.MakeFrameBuffer();
	ssao_buffer_->GetViewport().camera = re.CurFrameBuffer()->GetViewport().camera;

	blur_ssao_buffer_ = rf.MakeFrameBuffer();
	blur_ssao_buffer_->GetViewport().camera = re.CurFrameBuffer()->GetViewport().camera;

	hdr_buffer_ = rf.MakeFrameBuffer();
	hdr_buffer_->GetViewport().camera = re.CurFrameBuffer()->GetViewport().camera;

	fpcController_.Scalers(0.05f, 0.5f);

	InputEngine& inputEngine(Context::Instance().InputFactoryInstance().InputEngineInstance());
	InputActionMap actionMap;
	actionMap.AddActions(actions, actions + sizeof(actions) / sizeof(actions[0]));

	action_handler_t input_handler(new input_signal);
	input_handler->connect(boost::bind(&DeferredShadingApp::InputHandler, this, _1, _2));
	inputEngine.ActionMap(actionMap, input_handler, true);

	deferred_shading_.reset(new DeferredShadingPostProcess);
	edge_anti_alias_.reset(new AntiAliasPostProcess);
	ssao_pp_.reset(new SSAOPostProcess);
	blur_pp_.reset(new BlurPostProcess(8, 1));
	hdr_pp_.reset(new HDRPostProcess);

	UIManager::Instance().Load(ResLoader::Instance().Load("DeferredShading.kui"));
	dialog_ = UIManager::Instance().GetDialogs()[0];

	id_buffer_combo_ = dialog_->IDFromName("BufferCombo");
	id_anti_alias_ = dialog_->IDFromName("AntiAlias");
	id_ssao_ = dialog_->IDFromName("SSAO");
	id_ctrl_camera_ = dialog_->IDFromName("CtrlCamera");

	dialog_->Control<UIComboBox>(id_buffer_combo_)->OnSelectionChangedEvent().connect(boost::bind(&DeferredShadingApp::BufferChangedHandler, this, _1));
	this->BufferChangedHandler(*dialog_->Control<UIComboBox>(id_buffer_combo_));

	dialog_->Control<UICheckBox>(id_anti_alias_)->OnChangedEvent().connect(boost::bind(&DeferredShadingApp::AntiAliasHandler, this, _1));
	DeferredShadingApp::AntiAliasHandler(*dialog_->Control<UICheckBox>(id_anti_alias_));
	dialog_->Control<UICheckBox>(id_ssao_)->OnChangedEvent().connect(boost::bind(&DeferredShadingApp::SSAOHandler, this, _1));
	DeferredShadingApp::SSAOHandler(*dialog_->Control<UICheckBox>(id_ssao_));
	dialog_->Control<UICheckBox>(id_ctrl_camera_)->OnChangedEvent().connect(boost::bind(&DeferredShadingApp::CtrlCameraHandler, this, _1));
	DeferredShadingApp::CtrlCameraHandler(*dialog_->Control<UICheckBox>(id_ctrl_camera_));
}

void DeferredShadingApp::OnResize(uint32_t width, uint32_t height)
{
	App3DFramework::OnResize(width, height);

	RenderFactory& rf = Context::Instance().RenderFactoryInstance();
	diffuse_specular_tex_ = rf.MakeTexture2D(width, height, 1, EF_ABGR16F, 1, 0, EAH_GPU_Read | EAH_GPU_Write, NULL);
	normal_depth_tex_ = rf.MakeTexture2D(width, height, 1, EF_ABGR16F, 1, 0, EAH_GPU_Read | EAH_GPU_Write, NULL);
	g_buffer_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*diffuse_specular_tex_, 0));
	g_buffer_->Attach(FrameBuffer::ATT_Color1, rf.Make2DRenderView(*normal_depth_tex_, 0));
	g_buffer_->Attach(FrameBuffer::ATT_DepthStencil, rf.MakeDepthStencilRenderView(width, height, EF_D16, 1, 0));

	shaded_tex_ = rf.MakeTexture2D(width, height, 1, EF_ABGR16F, 1, 0, EAH_GPU_Read | EAH_GPU_Write, NULL);
	shaded_buffer_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*shaded_tex_, 0));

	try
	{
		ssao_tex_ = rf.MakeTexture2D(width, height, 1, EF_R16F, 1, 0, EAH_GPU_Read | EAH_GPU_Write, NULL);
	}
	catch (...)
	{
		ssao_tex_ = rf.MakeTexture2D(width, height, 1, EF_ABGR16F, 1, 0, EAH_GPU_Read | EAH_GPU_Write, NULL);
	}
	ssao_buffer_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*ssao_tex_, 0));

	blur_ssao_tex_ = rf.MakeTexture2D(width, height, 1, ssao_tex_->Format(), 1, 0, EAH_GPU_Read | EAH_GPU_Write, NULL);
	blur_ssao_buffer_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*blur_ssao_tex_, 0));

	hdr_tex_ = rf.MakeTexture2D(width, height, 1, EF_ABGR16F, 1, 0, EAH_GPU_Read | EAH_GPU_Write, NULL);
	hdr_buffer_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*hdr_tex_, 0));

	deferred_shading_->Source(normal_depth_tex_, g_buffer_->RequiresFlipping());
	checked_pointer_cast<DeferredShadingPostProcess>(deferred_shading_)->ColorTex(diffuse_specular_tex_);
	checked_pointer_cast<DeferredShadingPostProcess>(deferred_shading_)->SSAOTex(blur_ssao_tex_);
	deferred_shading_->Destinate(shaded_buffer_);

	edge_anti_alias_->Source(normal_depth_tex_, shaded_buffer_->RequiresFlipping());
	checked_pointer_cast<AntiAliasPostProcess>(edge_anti_alias_)->ColorTex(shaded_tex_);
	edge_anti_alias_->Destinate(hdr_buffer_);
	//edge_anti_alias_->Destinate(FrameBufferPtr());

	hdr_pp_->Source(hdr_tex_, hdr_buffer_->RequiresFlipping());
	hdr_pp_->Destinate(FrameBufferPtr());

	ssao_pp_->Source(normal_depth_tex_, g_buffer_->RequiresFlipping());
	ssao_pp_->Destinate(ssao_buffer_);

	blur_pp_->Source(ssao_tex_, ssao_buffer_->RequiresFlipping());
	blur_pp_->Destinate(blur_ssao_buffer_);

	UIManager::Instance().SettleCtrls(width, height);
}

void DeferredShadingApp::InputHandler(InputEngine const & /*sender*/, InputAction const & action)
{
	switch (action.first)
	{
	case Exit:
		this->Quit();
		break;
	}
}

void DeferredShadingApp::BufferChangedHandler(KlayGE::UIComboBox const & sender)
{
	buffer_type_ = sender.GetSelectedIndex();
	checked_pointer_cast<DeferredShadingPostProcess>(deferred_shading_)->BufferType(buffer_type_);

	if (buffer_type_ != 0)
	{
		dialog_->Control<UICheckBox>(id_anti_alias_)->SetChecked(false);
		anti_alias_enabled_ = false;
		deferred_shading_->Destinate(FrameBufferPtr());
	}
}

void DeferredShadingApp::AntiAliasHandler(KlayGE::UICheckBox const & sender)
{
	if (0 == buffer_type_)
	{
		anti_alias_enabled_ = sender.GetChecked();
		if (anti_alias_enabled_)
		{
			deferred_shading_->Destinate(shaded_buffer_);
			edge_anti_alias_->Destinate(hdr_buffer_);
			//edge_anti_alias_->Destinate(FrameBufferPtr());
		}
		else
		{
			deferred_shading_->Destinate(hdr_buffer_);
			//deferred_shading_->Destinate(FrameBufferPtr());
		}
	}
}

void DeferredShadingApp::SSAOHandler(KlayGE::UICheckBox const & sender)
{
	if (0 == buffer_type_)
	{
		ssao_enabled_ = sender.GetChecked();
		checked_pointer_cast<DeferredShadingPostProcess>(deferred_shading_)->SSAOEnabled(ssao_enabled_);
	}
}

void DeferredShadingApp::CtrlCameraHandler(KlayGE::UICheckBox const & sender)
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

uint32_t DeferredShadingApp::DoUpdate(uint32_t pass)
{
	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

	switch (pass)
	{
	case 0:
		renderEngine.BindFrameBuffer(g_buffer_);
		renderEngine.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, Color(0, 0, 1, 1), 1.0f, 0);
		return App3DFramework::URV_Need_Flush;

	default:
		renderEngine.BindFrameBuffer(FrameBufferPtr());
		renderEngine.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, Color(0.2f, 0.4f, 0.6f, 1), 1.0f, 0);
		if (((0 == buffer_type_) && ssao_enabled_) || (7 == buffer_type_))
		{
			ssao_pp_->Apply();
			blur_pp_->Apply();
		}
		deferred_shading_->Apply();
		if ((0 == buffer_type_) && anti_alias_enabled_)
		{
			edge_anti_alias_->Apply();
		}
		if (0 == buffer_type_)
		{
			hdr_pp_->Apply();
		}

		UIManager::Instance().Render();

		{
			FrameBuffer& rw = *checked_pointer_cast<FrameBuffer>(renderEngine.CurFrameBuffer());

			font_->RenderText(0, 0, Color(1, 1, 0, 1), L"Deferred Shading", 16);
			font_->RenderText(0, 18, Color(1, 1, 0, 1), rw.Description(), 16);

			std::wostringstream stream;
			stream.precision(2);
			stream << fixed << this->FPS() << " FPS";
			font_->RenderText(0, 36, Color(1, 1, 0, 1), stream.str(), 16);
		}
	
		return App3DFramework::URV_Only_New_Objs | App3DFramework::URV_Need_Flush | App3DFramework::URV_Finished;
	}
}
