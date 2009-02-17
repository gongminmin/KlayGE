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
#include <KlayGE/Util.hpp>

#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/InputFactory.hpp>

#include <sstream>
#include <ctime>
#include <boost/bind.hpp>

#include "DeferredShading.hpp"

using namespace std;
using namespace KlayGE;

namespace
{
	class RenderTorus : public KMesh
	{
	public:
		RenderTorus(RenderModelPtr model, std::wstring const & /*name*/)
			: KMesh(model, L"Torus")
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();

			effect_ = rf.LoadEffect("DeferredShading.kfx");
		}

		void BuildMeshInfo()
		{
			bool has_bump_map = false;
			for (StaticMesh::TextureSlotsType::iterator iter = texture_slots_.begin();
				iter != texture_slots_.end(); ++ iter)
			{
				if ("Diffuse Color" == iter->first)
				{
					*(effect_->ParameterByName("diffuse_tex")) = LoadTexture(iter->second, EAH_GPU_Read)();
				}
				if ("Bump" == iter->first)
				{
					TexturePtr bump = LoadTexture(iter->second, EAH_GPU_Read)();
					*(effect_->ParameterByName("bump_tex")) = bump;
					if (bump)
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

			*(technique_->Effect().ParameterByName("model_view_proj")) = view * proj;
			*(technique_->Effect().ParameterByName("model_view")) = view;

			float const depth_range = camera.FarPlane() - camera.NearPlane();
			*(technique_->Effect().ParameterByName("depth_min_invrange_range")) = float3(camera.NearPlane(), 1 / depth_range, depth_range);
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
				*(technique_->Effect().ParameterByName("inv_width")) = 1.0f / tex->Width(0);
				*(technique_->Effect().ParameterByName("inv_height")) = 1.0f / tex->Height(0);
			}
		}

		void ColorTex(TexturePtr const & tex)
		{
			*(technique_->Effect().ParameterByName("color_tex")) = tex;
		}

		void OnRenderBegin()
		{
			PostProcess::OnRenderBegin();

			Camera const & camera = Context::Instance().AppInstance().ActiveCamera();

			float4x4 const & view = camera.ViewMatrix();
			float4x4 const & proj = camera.ProjMatrix();
			float4x4 const inv_proj = MathLib::inverse(proj);

			float const depth_range = camera.FarPlane() - camera.NearPlane();
			*(technique_->Effect().ParameterByName("depth_min_invrange_range")) = float3(camera.NearPlane(), 1 / depth_range, depth_range);
			*(technique_->Effect().ParameterByName("light_in_eye")) = MathLib::transform_coord(float3(2, 10, 0), view);

			*(technique_->Effect().ParameterByName("upper_left")) = MathLib::transform_coord(float3(-1, 1, 1), inv_proj);
			*(technique_->Effect().ParameterByName("upper_right")) = MathLib::transform_coord(float3(1, 1, 1), inv_proj);
			*(technique_->Effect().ParameterByName("lower_left")) = MathLib::transform_coord(float3(-1, -1, 1), inv_proj);
			*(technique_->Effect().ParameterByName("lower_right")) = MathLib::transform_coord(float3(1, -1, 1), inv_proj);
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
			: App3DFramework(name, settings)
{
}

void DeferredShadingApp::InitObjects()
{
	font_ = Context::Instance().RenderFactoryInstance().MakeFont("gkai00mp.kfont");

	torus_.reset(new TorusObject);
	torus_->AddToSceneManager();

	this->LookAt(float3(0, 2, -2), float3(0, 2, 0));
	this->Proj(0.1f, 100.0f);

	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
	g_buffer_ = Context::Instance().RenderFactoryInstance().MakeFrameBuffer();
	g_buffer_->GetViewport().camera = renderEngine.CurFrameBuffer()->GetViewport().camera;

	fpcController_.AttachCamera(this->ActiveCamera());
	fpcController_.Scalers(0.05f, 0.5f);

	InputEngine& inputEngine(Context::Instance().InputFactoryInstance().InputEngineInstance());
	InputActionMap actionMap;
	actionMap.AddActions(actions, actions + sizeof(actions) / sizeof(actions[0]));

	action_handler_t input_handler(new input_signal);
	input_handler->connect(boost::bind(&DeferredShadingApp::InputHandler, this, _1, _2));
	inputEngine.ActionMap(actionMap, input_handler, true);

	deferred_shading_.reset(new DeferredShadingPostProcess);
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

	deferred_shading_->Source(normal_depth_tex_, g_buffer_->RequiresFlipping());
	checked_pointer_cast<DeferredShadingPostProcess>(deferred_shading_)->ColorTex(diffuse_specular_tex_);
	deferred_shading_->Destinate(FrameBufferPtr());

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

uint32_t DeferredShadingApp::DoUpdate(uint32_t pass)
{
	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

	switch (pass)
	{
	case 0:
		UIManager::Instance().HandleInput();

		renderEngine.BindFrameBuffer(g_buffer_);
		renderEngine.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, Color(0.2f, 0.4f, 0.6f, 1), 1.0f, 0);
		return App3DFramework::URV_Need_Flush;

	default:
		renderEngine.BindFrameBuffer(FrameBufferPtr());
		renderEngine.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, Color(0.2f, 0.4f, 0.6f, 1), 1.0f, 0);
		deferred_shading_->Apply();

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
