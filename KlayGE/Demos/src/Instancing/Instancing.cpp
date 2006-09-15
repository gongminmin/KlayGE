#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/Renderable.hpp>
#include <KlayGE/RenderableHelper.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/SceneManager.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/RenderSettings.hpp>
#include <KlayGE/Sampler.hpp>
#include <KlayGE/KMesh.hpp>
#include <KlayGE/SceneObjectHelper.hpp>

#include <KlayGE/D3D9/D3D9RenderFactory.hpp>

#include <KlayGE/Input.hpp>
#include <KlayGE/DInput/DInputFactory.hpp>

#include <vector>
#include <sstream>
#include <ctime>
#include <boost/bind.hpp>

#include "Instancing.hpp"

using namespace std;
using namespace KlayGE;

namespace
{
	int const NUM_INSTANCE = 400;

	class Teapot : public SceneObjectHelper
	{
	private:
		struct InstData
		{
			float4 col[3];
			Color clr;
		};

	public:
		Teapot()
			: SceneObjectHelper(SOA_Cullable)
		{
			instance_format_.push_back(vertex_element(VEU_TextureCoord, 1, EF_ABGR32F));
			instance_format_.push_back(vertex_element(VEU_TextureCoord, 2, EF_ABGR32F));
			instance_format_.push_back(vertex_element(VEU_TextureCoord, 3, EF_ABGR32F));
			instance_format_.push_back(vertex_element(VEU_Diffuse, 0, EF_ABGR32F));
		}

		void Instance(float4x4 const & mat, Color const & clr)
		{
			float4x4 matT = MathLib::transpose(mat);

			inst_.col[0] = matT.Row(0);
			inst_.col[1] = matT.Row(1);
			inst_.col[2] = matT.Row(2);
			inst_.clr = clr;
		}

		void const * InstanceData() const
		{
			return &inst_;
		}

		void SetRenderable(RenderablePtr ra)
		{
			renderable_ = ra;
		}

	private:
		InstData inst_;
	};

	class RenderInstance : public KMesh
	{
	public:
		RenderInstance(RenderModelPtr model, std::wstring const & /*name*/)
			: KMesh(model, L"Instance")
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();

			technique_ = rf.LoadEffect("Instancing.fx")->TechniqueByName("Instance");
		}

		void BuildMeshInfo()
		{
		}

		void OnRenderBegin()
		{
			App3DFramework const & app = Context::Instance().AppInstance();

			float4x4 const & model = float4x4::Identity();
			float4x4 const & view = app.ActiveCamera().ViewMatrix();
			float4x4 const & proj = app.ActiveCamera().ProjMatrix();

			*(technique_->Effect().ParameterByName("ViewProj")) = model * view * proj;
			*(technique_->Effect().ParameterByName("lightPos")) = float4(-1, 0, -1, 1);
		}
	};

	class RenderNormalMesh : public KMesh
	{
	private:
		struct InstData
		{
			float4 col[3];
			Color clr;
		};

	public:
		RenderNormalMesh(RenderModelPtr model, std::wstring const & /*name*/)
			: KMesh(model, L"NormalMesh")
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();

			technique_ = rf.LoadEffect("Instancing.fx")->TechniqueByName("NormalMesh");
		}

		void OnRenderBegin()
		{
			App3DFramework const & app = Context::Instance().AppInstance();

			float4x4 const & view = app.ActiveCamera().ViewMatrix();
			float4x4 const & proj = app.ActiveCamera().ProjMatrix();

			*(technique_->Effect().ParameterByName("ViewProj")) = view * proj;
			*(technique_->Effect().ParameterByName("lightPos")) = float4(-1, 0, -1, 1);
		}

		void OnInstanceBegin(uint32_t id)
		{
			InstData const * data = static_cast<InstData const *>(instances_[id].lock()->InstanceData());

			float4x4 model;
			model.Col(0, data->col[0]);
			model.Col(1, data->col[1]);
			model.Col(2, data->col[2]);
			model.Col(3, float4(0, 0, 0, 1));

			*(technique_->Effect().ParameterByName("modelmat")) = model;
			*(technique_->Effect().ParameterByName("color")) = float4(data->clr.r(), data->clr.g(), data->clr.b(), data->clr.a());
		}

	private:
		void UpdateInstanceStream()
		{
		}
	};

	enum
	{
		UseInstance,
		UseNormal,

		Exit
	};

	InputActionDefine actions[] = 
	{
		InputActionDefine(UseInstance, KS_1),
		InputActionDefine(UseNormal, KS_2),
		InputActionDefine(Exit, KS_Escape)
	};

	bool ConfirmDevice(RenderDeviceCaps const & caps)
	{
		if (caps.max_shader_model < 2)
		{
			return false;
		}
		return true;
	}
}


int main()
{
	Context::Instance().RenderFactoryInstance(D3D9RenderFactoryInstance());
	Context::Instance().InputFactoryInstance(DInputFactoryInstance());

	RenderSettings settings;
	settings.width = 800;
	settings.height = 600;
	settings.color_fmt = EF_ARGB8;
	settings.full_screen = false;
	settings.ConfirmDevice = ConfirmDevice;

	Instancing app;
	app.Create("Instance", settings);
	app.Run();

	return 0;
}

Instancing::Instancing()
{
	ResLoader::Instance().AddPath("../media/Common");
	ResLoader::Instance().AddPath("../media/Instancing");
}

void Instancing::InitObjects()
{
	// 建立字体
	font_ = Context::Instance().RenderFactoryInstance().MakeFont("gkai00mp.ttf", 16);

	renderInstance_ = LoadKModel("teapot.kmodel", CreateKModelFactory<RenderModel>(), CreateKMeshFactory<RenderInstance>())->Mesh(0);
	for (int i = 0; i < 10; ++ i)
	{
		for (int j = 0; j < NUM_INSTANCE / 10; ++ j)
		{
			float const s = sin(2 * PI * j / (NUM_INSTANCE / 10));
			float const c = cos(2 * PI * j / (NUM_INSTANCE / 10));

			SceneObjectPtr so(new Teapot);
			checked_pointer_cast<Teapot>(so)->Instance(
				MathLib::translation(s, i / 10.0f, c), Color(s, c, 0, 1));

			checked_pointer_cast<Teapot>(so)->SetRenderable(renderInstance_);
			so->AddToSceneManager();
			scene_objs_.push_back(so);
		}
	}
	use_instance_ = true;

	renderMesh_ = LoadKModel("teapot.kmodel", CreateKModelFactory<RenderModel>(), CreateKMeshFactory<RenderNormalMesh>())->Mesh(0);

	this->LookAt(float3(-1.8f, 1.9f, -1.8f), float3(0, 0, 0));
	this->Proj(0.1f, 100);

	fpcController_.AttachCamera(this->ActiveCamera());
	fpcController_.Scalers(0.05f, 0.1f);

	InputEngine& inputEngine(Context::Instance().InputFactoryInstance().InputEngineInstance());
	InputActionMap actionMap;
	actionMap.AddActions(actions, actions + sizeof(actions) / sizeof(actions[0]));

	action_handler_t input_handler(inputEngine);
	input_handler += boost::bind(&Instancing::InputHandler, this, _1, _2);
	inputEngine.ActionMap(actionMap, input_handler, true);
}

void Instancing::InputHandler(InputEngine const & /*sender*/, InputAction const & action)
{
	switch (action.first)
	{
	case UseInstance:
		if (!use_instance_)
		{
			for (int i = 0; i < NUM_INSTANCE; ++ i)
			{
				checked_pointer_cast<Teapot>(scene_objs_[i])->SetRenderable(renderInstance_);
			}

			use_instance_ = true;
		}
		break;

	case UseNormal:
		if (use_instance_)
		{
			for (int i = 0; i < NUM_INSTANCE; ++ i)
			{
				checked_pointer_cast<Teapot>(scene_objs_[i])->SetRenderable(renderMesh_);
			}

			use_instance_ = false;
		}
		break;

	case Exit:
		this->Quit();
		break;
	}
}

void Instancing::DoUpdate(uint32_t /*pass*/)
{
	fpcController_.Update();

	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
	renderEngine.Clear(RenderEngine::CBM_Color | RenderEngine::CBM_Depth, Color(0.2f, 0.4f, 0.6f, 1), 1, 0);

	std::wostringstream stream;
	stream << this->FPS();

	font_->RenderText(0, 0, Color(1, 1, 0, 1), L"几何体实例化");
	font_->RenderText(0, 18, Color(1, 1, 0, 1), stream.str().c_str());

	SceneManager& sceneMgr(Context::Instance().SceneManagerInstance());
	stream.str(L"");
	stream << sceneMgr.NumObjectsRendered() << " Scene objects "
		<< sceneMgr.NumRenderablesRendered() << " Renderables "
		<< sceneMgr.NumPrimitivesRendered() << " Primitives "
		<< sceneMgr.NumVerticesRendered() << " Vertices";
	font_->RenderText(0, 36, Color(1, 1, 1, 1), stream.str().c_str());

	if (use_instance_)
	{
		font_->RenderText(0, 54, Color(1, 1, 1, 1), L"Instancing is enabled");
	}
	else
	{
		font_->RenderText(0, 54, Color(1, 1, 1, 1), L"Instancing is disabled");
	}

	font_->RenderText(0, 72, Color(1, 1, 1, 1), L"Press '1' to enable instancing, '2' to disable it");
}
