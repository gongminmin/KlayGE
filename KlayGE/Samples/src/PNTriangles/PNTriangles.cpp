#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/GraphicsBuffer.hpp>
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
#include <KlayGE/UI.hpp>

#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/InputFactory.hpp>

#include <vector>
#include <sstream>
#include <ctime>
#include <boost/bind.hpp>

#include "PNTriangles.hpp"

using namespace std;
using namespace KlayGE;

namespace
{
	class SubDSkinnedModel;

	class SubDSkinnedMesh : public SkinnedMesh
	{
	public:
		SubDSkinnedMesh(RenderModelPtr model, std::wstring const & name)
			: SkinnedMesh(model, name)
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();
			RenderDeviceCaps const & caps = rf.RenderEngineInstance().DeviceCaps();

			RenderEffectPtr effect = rf.LoadEffect("PNTriangles.fxml");

			if (caps.max_shader_model < 5)
			{
				technique_ = effect->TechniqueByName("NoPNTriangles");
			}
			else
			{
				technique_ = effect->TechniqueByName("PNTriangles");
			}

			if (caps.max_shader_model < 5)
			{
				rl_->TopologyType(RenderLayout::TT_TriangleList);
			}
			else
			{
				rl_->TopologyType(RenderLayout::TT_3_Ctrl_Pt_PatchList);
			}
		}

		void BuildMeshInfo()
		{
			TexturePtr dm;
			RenderModel::Material const & mtl = model_.lock()->GetMaterial(this->MaterialID());
			RenderModel::TextureSlotsType const & texture_slots = mtl.texture_slots;
			for (RenderModel::TextureSlotsType::const_iterator iter = texture_slots.begin();
				iter != texture_slots.end(); ++ iter)
			{
				if (("DiffuseMap" == iter->first) || ("Diffuse Color" == iter->first))
				{
					if (!ResLoader::Instance().Locate(iter->second).empty())
					{
						dm = LoadTexture(iter->second, EAH_GPU_Read)();
					}
				}
			}
			*(technique_->Effect().ParameterByName("diffuse_tex")) = dm;
		}

		void SetModelMatrix(float4x4 model_matrix)
		{
			model_matrix_ = model_matrix;
		}

		void OnRenderBegin()
		{
			App3DFramework const & app = Context::Instance().AppInstance();

			float4x4 const & view = app.ActiveCamera().ViewMatrix();
			float4x4 const & proj = app.ActiveCamera().ProjMatrix();

			*(technique_->Effect().ParameterByName("worldviewproj")) = model_matrix_ * view * proj;
			*(technique_->Effect().ParameterByName("worldview")) = model_matrix_ * view;
			*(technique_->Effect().ParameterByName("eye_pos")) = app.ActiveCamera().EyePos();

			float tess_factor = 5;
			*(technique_->Effect().ParameterByName("tess_factors")) = float4(tess_factor, tess_factor, 1.0f, 9.0f);    

			RenderModelPtr model = model_.lock();
			if (model)
			{
				*(technique_->Effect().ParameterByName("joint_rots")) = checked_pointer_cast<SkinnedModel>(model)->GetBindRotations();
				*(technique_->Effect().ParameterByName("joint_poss")) = checked_pointer_cast<SkinnedModel>(model)->GetBindPositions();
			}
		}

	private:
		float4x4 model_matrix_;
	};

	class SubDSkinnedModel : public SkinnedModel
	{
	public:
		SubDSkinnedModel(std::wstring const & name)
			: SkinnedModel(name)
		{
		}

		void SetModelMatrix(float4x4 model_matrix)
		{
			for (uint32_t i = 0; i < this->NumMeshes(); ++ i)
			{
				checked_pointer_cast<SubDSkinnedMesh>(Mesh(i))->SetModelMatrix(model_matrix);
			}
		}
	};

	class PolygonObject : public SceneObjectHelper
	{
	public:
		PolygonObject()
			: SceneObjectHelper(SOA_Cullable)
		{
			renderable_ = LoadModel("archer_attacking.meshml", EAH_GPU_Read, CreateKModelFactory<SubDSkinnedModel>(), CreateKMeshFactory<SubDSkinnedMesh>())();
			model_matrix_ = MathLib::scaling(0.01f, 0.01f, 0.01f);
			checked_pointer_cast<SubDSkinnedModel>(renderable_)->SetModelMatrix(model_matrix_);
		}

		float4x4 const & GetModelMatrix() const
		{
			return model_matrix_;
		}

		void SetFrame(uint32_t frame)
		{
			checked_pointer_cast<SubDSkinnedModel>(renderable_)->SetFrame(frame);
		}

	private:
		float4x4 model_matrix_;
	};


	enum
	{
		Exit,
		FullScreen,
	};

	InputActionDefine actions[] =
	{
		InputActionDefine(Exit, KS_Escape),
		InputActionDefine(FullScreen, KS_Enter),
	};

	bool ConfirmDevice()
	{
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		RenderDeviceCaps const & caps = re.DeviceCaps();
		if (caps.max_shader_model < 5)
		{
			return false;
		}
		return true;
	}
}

int main()
{
	ResLoader::Instance().AddPath("../Samples/media/Common");
	ResLoader::Instance().AddPath("../Samples/media/PNTriangles");

	RenderSettings settings = Context::Instance().LoadCfg("KlayGE.cfg");
	settings.ConfirmDevice = ConfirmDevice;

	SubDApp app("SubDApp", settings);
	app.Create();
	app.Run();

	return 0;
}

SubDApp::SubDApp(std::string const & name, RenderSettings const & settings)
					: App3DFramework(name, settings)
{
}

void SubDApp::InitObjects()
{
	// ½¨Á¢×ÖÌå
	font_ = Context::Instance().RenderFactoryInstance().MakeFont("gkai00mp.kfont");

	polygon_ = MakeSharedPtr<PolygonObject>();
	polygon_->AddToSceneManager();

	this->LookAt(float3(2, 3, -2), float3(0, 1, 0));
	this->Proj(0.1f, 100);

	fpcController_.AttachCamera(this->ActiveCamera());
	fpcController_.Scalers(0.05f, 0.1f);

	InputEngine& inputEngine(Context::Instance().InputFactoryInstance().InputEngineInstance());
	InputActionMap actionMap;
	actionMap.AddActions(actions, actions + sizeof(actions) / sizeof(actions[0]));

	action_handler_t input_handler = MakeSharedPtr<input_signal>();
	input_handler->connect(boost::bind(&SubDApp::InputHandler, this, _1, _2));
	inputEngine.ActionMap(actionMap, input_handler, true);

	UIManager::Instance().Load(ResLoader::Instance().Load("PNTriangles.uiml"));
}

void SubDApp::OnResize(uint32_t width, uint32_t height)
{
	UIManager::Instance().SettleCtrls(width, height);
}

void SubDApp::InputHandler(InputEngine const & /*sender*/, InputAction const & action)
{
	switch (action.first)
	{
	case FullScreen:
		{
			RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			renderEngine.EndFrame();
			renderEngine.Resize(800, 600);
			renderEngine.FullScreen(!renderEngine.FullScreen());
			renderEngine.BeginFrame();
		}
		break;

	case Exit:
		this->Quit();
		break;
	}
}

void SubDApp::DoUpdateOverlay()
{
	UIManager::Instance().Render();

	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

	std::wostringstream stream;
	stream << this->FPS() << " FPS";

	font_->RenderText(0, 0, Color(1, 1, 0, 1), L"PNTriangles", 16);
	font_->RenderText(0, 18, Color(1, 1, 0, 1), stream.str(), 16);
	font_->RenderText(0, 36, Color(1, 1, 0, 1), renderEngine.Name(), 16);
}

uint32_t SubDApp::DoUpdate(uint32_t /*pass*/)
{
	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
	renderEngine.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, Color(0.2f, 0.4f, 0.6f, 1), 1.0f, 0);
	checked_pointer_cast<PolygonObject>(polygon_)->SetFrame(0);
	return App3DFramework::URV_Need_Flush | App3DFramework::URV_Finished;
}
