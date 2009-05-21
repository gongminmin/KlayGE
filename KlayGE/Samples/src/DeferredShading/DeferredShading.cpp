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
		RenderTorus(RenderModelPtr const & model, std::wstring const & name)
			: KMesh(model, name)
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();

			effect_ = rf.LoadEffect("DeferredShading.fxml");
		}

		void BuildMeshInfo()
		{
			std::map<std::string, TexturePtr> tex_pool;

			RenderModel::Material const & mtl = model_.lock()->GetMaterial(this->MaterialID());

			bool has_diffuse_map = false;
			bool has_bump_map = false;
			RenderModel::TextureSlotsType const & texture_slots = mtl.texture_slots;
			for (RenderModel::TextureSlotsType::const_iterator iter = texture_slots.begin();
				iter != texture_slots.end(); ++ iter)
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
				has_diffuse_map = tex;

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

			*(effect_->ParameterByName("diffuse_clr")) = float4(mtl.diffuse.x(), mtl.diffuse.y(), mtl.diffuse.z(), has_diffuse_map);

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
			renderable_ = LoadModel("sponza.meshml", EAH_GPU_Read, CreateKModelFactory<RenderModel>(), CreateKMeshFactory<RenderTorus>());
		}
	};


	class RenderCone : public KMesh
	{
	public:
		RenderCone(RenderModelPtr const & model, std::wstring const & name)
			: KMesh(model, name)
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();

			technique_ = rf.LoadEffect("DeferredShading.fxml")->TechniqueByName("GBufferNoTexTech");
		}

		void BuildMeshInfo()
		{
		}

		void ModelMatrix(float4x4 const & mat)
		{
			model_ = mat;
		}

		void OnRenderBegin()
		{
			Camera const & camera = Context::Instance().AppInstance().ActiveCamera();

			float4x4 const & view = camera.ViewMatrix();
			float4x4 const & proj = camera.ProjMatrix();

			*(technique_->Effect().ParameterByName("proj")) = proj;
			*(technique_->Effect().ParameterByName("model_view")) = model_ * view;

			*(technique_->Effect().ParameterByName("depth_near_far_invfar")) = float3(camera.NearPlane(), camera.FarPlane(), 1 / camera.FarPlane());
		}

	private:
		float4x4 model_;
	};

	class ConeObject : public SceneObjectHelper
	{
	public:
		ConeObject(std::string const & model_name, float org_angle)
			: SceneObjectHelper(SOA_Cullable)
		{
			renderable_ = LoadModel(model_name, EAH_GPU_Read, CreateKModelFactory<RenderModel>(), CreateKMeshFactory<RenderCone>())->Mesh(0);
			model_org_ = MathLib::rotation_x(org_angle);
		}

		void Update()
		{
			model_ = MathLib::scaling(0.1f, 0.1f, 0.1f) * model_org_ * MathLib::rotation_y(std::clock() / 700.0f) * MathLib::translation(0.0f, 2.0f, 0.0f);
			checked_pointer_cast<RenderCone>(renderable_)->ModelMatrix(model_);
		}

		float4x4 const & ModelMatrix() const
		{
			return model_;
		}

	private:
		float4x4 model_;
		float4x4 model_org_;
	};


	class DeferredShadingPostProcess : public PostProcess
	{
	public:
		DeferredShadingPostProcess()
			: PostProcess(Context::Instance().RenderFactoryInstance().LoadEffect("DeferredShading.fxml")->TechniqueByName("DeferredShading")),
				spot_light_clr_(16), spot_light_pos_(16), spot_light_dir_(16), spot_light_cos_cone_(16), num_spot_lights_(0)
		{
		}

		void SetSpotLight(int32_t index, float4x4 const & model_mat, float cos_outer, float cos_inner, float3 const & clr)
		{
			num_spot_lights_ = std::max(num_spot_lights_, index + 1);

			spot_light_clr_[index] = clr;
			spot_light_pos_[index] = MathLib::transform_coord(float3(0, 0, 0), model_mat);
			spot_light_dir_[index] = MathLib::normalize(MathLib::transform_normal(float3(0, -1, 0), model_mat));
			spot_light_cos_cone_[index] = float2(cos_outer, cos_inner);
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

			std::vector<float3> spot_light_pos_eye(spot_light_pos_.size());
			std::vector<float3> spot_light_dir_eye(spot_light_dir_.size());
			for (int32_t i = 0; i < num_spot_lights_; ++ i)
			{
				spot_light_pos_eye[i] = MathLib::transform_coord(spot_light_pos_[i], view);
				spot_light_dir_eye[i] = MathLib::transform_normal(spot_light_dir_[i], view);
			}
			*(technique_->Effect().ParameterByName("num_spot_lights")) = num_spot_lights_;
			*(technique_->Effect().ParameterByName("spot_light_clr")) = spot_light_clr_;
			*(technique_->Effect().ParameterByName("spot_light_pos")) = spot_light_pos_eye;
			*(technique_->Effect().ParameterByName("spot_light_dir")) = spot_light_dir_eye;
			*(technique_->Effect().ParameterByName("spot_light_cos_cone")) = spot_light_cos_cone_;
		}

	private:
		int32_t num_spot_lights_;
		std::vector<float3> spot_light_clr_;
		std::vector<float3> spot_light_pos_;
		std::vector<float3> spot_light_dir_;
		std::vector<float2> spot_light_cos_cone_;
	};

	class AntiAliasPostProcess : public PostProcess
	{
	public:
		AntiAliasPostProcess()
			: PostProcess(Context::Instance().RenderFactoryInstance().LoadEffect("DeferredShading.fxml")->TechniqueByName("AntiAlias"))
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
			: PostProcess(Context::Instance().RenderFactoryInstance().LoadEffect("DeferredShading.fxml")->TechniqueByName("SSAO"))
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

	torus_ = MakeSharedPtr<TorusObject>();
	torus_->AddToSceneManager();

	light_src_[0] = MakeSharedPtr<ConeObject>("cone_60.meshml", PI / 2);
	light_src_[1] = MakeSharedPtr<ConeObject>("cone_90.meshml", -PI / 2);
	light_src_[0]->AddToSceneManager();
	light_src_[1]->AddToSceneManager();

	this->LookAt(float3(-2, 2, 0), float3(0, 2, 0));
	this->Proj(0.1f, 100.0f);

	RenderFactory& rf = Context::Instance().RenderFactoryInstance();
	RenderEngine& re = rf.RenderEngineInstance();

	TexturePtr y_cube_map = LoadTexture("Lake_CraterLake03_y.dds", EAH_GPU_Read)();
	TexturePtr c_cube_map = LoadTexture("Lake_CraterLake03_c.dds", EAH_GPU_Read)();
	sky_box_ = MakeSharedPtr<SceneObjectHDRSkyBox>();
	checked_pointer_cast<SceneObjectHDRSkyBox>(sky_box_)->CompressedCubeMap(y_cube_map, c_cube_map);
	sky_box_->AddToSceneManager();
	
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

	action_handler_t input_handler = MakeSharedPtr<input_signal>();
	input_handler->connect(boost::bind(&DeferredShadingApp::InputHandler, this, _1, _2));
	inputEngine.ActionMap(actionMap, input_handler, true);

	deferred_shading_ = MakeSharedPtr<DeferredShadingPostProcess>();
	edge_anti_alias_ = MakeSharedPtr<AntiAliasPostProcess>();
	ssao_pp_ = MakeSharedPtr<SSAOPostProcess>();
	blur_pp_ = MakeSharedPtr<BlurPostProcess>(8, 1.0f);
	hdr_pp_ = MakeSharedPtr<HDRPostProcess>(false, false);

	UIManager::Instance().Load(ResLoader::Instance().Load("DeferredShading.uiml"));
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

void DeferredShadingApp::DoUpdateOverlay()
{
	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

	UIManager::Instance().Render();

	FrameBuffer& rw = *checked_pointer_cast<FrameBuffer>(renderEngine.CurFrameBuffer());

	font_->RenderText(0, 0, Color(1, 1, 0, 1), L"Deferred Shading", 16);
	font_->RenderText(0, 18, Color(1, 1, 0, 1), rw.Description(), 16);

	std::wostringstream stream;
	stream.precision(2);
	stream << fixed << this->FPS() << " FPS";
	font_->RenderText(0, 36, Color(1, 1, 0, 1), stream.str(), 16);
}

uint32_t DeferredShadingApp::DoUpdate(uint32_t pass)
{
	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

	switch (pass)
	{
	case 0:
		renderEngine.BindFrameBuffer(g_buffer_);
		renderEngine.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, Color(0, 0, 1, 0), 1.0f, 0);
		return App3DFramework::URV_Need_Flush;

	default:
		checked_pointer_cast<DeferredShadingPostProcess>(deferred_shading_)->SetSpotLight(0,
			checked_pointer_cast<ConeObject>(light_src_[0])->ModelMatrix(), cos(PI / 6), cos(PI / 8), float3(1, 0, 0));
		checked_pointer_cast<DeferredShadingPostProcess>(deferred_shading_)->SetSpotLight(1,
			checked_pointer_cast<ConeObject>(light_src_[1])->ModelMatrix(), cos(PI / 4), cos(PI / 6), float3(0, 1, 0));

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

		return App3DFramework::URV_Finished;
	}
}
