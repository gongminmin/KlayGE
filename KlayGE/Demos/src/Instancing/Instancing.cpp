#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/VertexBuffer.hpp>
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
#include <boost/tuple/tuple.hpp>

#include "Instancing.hpp"

using namespace std;
using namespace KlayGE;

namespace
{
	int const NUM_INSTANCE = 20;

	class Teapot : public SceneObjectHelper
	{
	private:
		struct InstData
		{
			Vector4 col[3];
			Color clr;
		};

	public:
		Teapot()
			: SceneObjectHelper(SOA_Cullable)
		{
			instance_format_.push_back(vertex_element(VEU_TextureCoord, 1, sizeof(float), 4));
			instance_format_.push_back(vertex_element(VEU_TextureCoord, 2, sizeof(float), 4));
			instance_format_.push_back(vertex_element(VEU_TextureCoord, 3, sizeof(float), 4));
			instance_format_.push_back(vertex_element(VEU_Diffuse, 0, sizeof(float), 4));
		}

		void Instance(Matrix4 const & mat, Color const & clr)
		{
			Matrix4 matT = MathLib::Transpose(mat);

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
		RenderInstance(std::wstring const & /*name*/, TexturePtr tex)
			: KMesh(L"Instance", tex)
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();

			effect_ = rf.LoadEffect("Instancing.fx");
		}

		void OnRenderBegin()
		{
			effect_->ActiveTechnique("Instance");

			App3DFramework const & app = Context::Instance().AppInstance();

			Matrix4 const & model = Matrix4::Identity();
			Matrix4 const & view = app.ActiveCamera().ViewMatrix();
			Matrix4 const & proj = app.ActiveCamera().ProjMatrix();

			*(effect_->ParameterByName("ViewProj")) = view * proj;
			*(effect_->ParameterByName("lightPos")) = Vector4(-1, 0, -1, 1);
		}
	};

	class RenderNormalMesh : public KMesh
	{
	private:
		struct InstData
		{
			Vector4 col[3];
			Color clr;
		};

	public:
		RenderNormalMesh(std::wstring const & name, TexturePtr tex)
			: KMesh(L"NormalMesh", tex)
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();

			effect_ = rf.LoadEffect("Instancing.fx");
		}

		void OnRenderBegin()
		{
			App3DFramework const & app = Context::Instance().AppInstance();

			Matrix4 const & view = app.ActiveCamera().ViewMatrix();
			Matrix4 const & proj = app.ActiveCamera().ProjMatrix();

			effect_->ActiveTechnique("NormalMesh");

			*(effect_->ParameterByName("ViewProj")) = view * proj;
			*(effect_->ParameterByName("lightPos")) = Vector4(-1, 0, -1, 1);
		}

		void OnInstanceBegin(uint32_t id)
		{
			InstData const * data = static_cast<InstData const *>(instances_[id].lock()->InstanceData());

			Matrix4 model;
			model.Col(0, data->col[0]);
			model.Col(1, data->col[1]);
			model.Col(2, data->col[2]);
			model.Col(3, Vector4(0, 0, 0, 1));

			*(effect_->ParameterByName("modelmat")) = model;
			*(effect_->ParameterByName("color")) = Vector4(data->clr.r(), data->clr.g(), data->clr.b(), data->clr.a());
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
	settings.colorDepth = 32;
	settings.fullScreen = false;
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

	renderInstance_ = LoadKMesh("teapot.kmesh", CreateKMeshFactory<RenderInstance>())->Mesh(0);
	for (int i = 0; i < NUM_INSTANCE; ++ i)
	{
		SceneObjectPtr so(new Teapot);
		checked_cast<Teapot*>(so.get())->Instance(
			MathLib::Translation((i / 10) / 10.0f, (i % 10) / 10.0f, 0.0f),
			Color((i % 10) / 10.0f, (i / 10) / 10.0f, 0, 1));

		checked_cast<Teapot*>(so.get())->SetRenderable(renderInstance_);
		so->AddToSceneManager();
		scene_objs_.push_back(so);
	}
	use_instance_ = true;

	renderMesh_ = LoadKMesh("teapot.kmesh", CreateKMeshFactory<RenderNormalMesh>())->Mesh(0);

	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

	renderEngine.ClearColor(Color(0.2f, 0.4f, 0.6f, 1));

	this->LookAt(Vector3(-0.3f, 0.4f, -0.3f), Vector3(0, 0, 0));
	this->Proj(0.1f, 100);

	fpcController_.AttachCamera(this->ActiveCamera());
	fpcController_.Scalers(0.05f, 0.1f);

	InputEngine& inputEngine(Context::Instance().InputFactoryInstance().InputEngineInstance());
	KlayGE::InputActionMap actionMap;
	actionMap.AddActions(actions, actions + sizeof(actions) / sizeof(actions[0]));

	action_handler_t input_handler(inputEngine);
	input_handler += boost::bind(&Instancing::InputHandler, this, _1, _2);
	inputEngine.ActionMap(actionMap, input_handler, true);
}

void Instancing::InputHandler(InputEngine const & sender, InputAction const & action)
{
	SceneManager& sceneMgr(Context::Instance().SceneManagerInstance());

	switch (action.first)
	{
	case UseInstance:
		if (!use_instance_)
		{
			for (int i = 0; i < NUM_INSTANCE; ++ i)
			{
				checked_cast<Teapot*>(scene_objs_[i].get())->SetRenderable(renderInstance_);
			}

			use_instance_ = true;
		}
		break;

	case UseNormal:
		if (use_instance_)
		{
			for (int i = 0; i < NUM_INSTANCE; ++ i)
			{
				checked_cast<Teapot*>(scene_objs_[i].get())->SetRenderable(renderMesh_);
			}

			use_instance_ = false;
		}
		break;

	case Exit:
		this->Quit();
		break;
	}
}

void Instancing::DoUpdate(uint32_t pass)
{
	fpcController_.Update();

	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
	renderEngine.Clear(RenderEngine::CBM_Color | RenderEngine::CBM_Depth);

	std::wostringstream stream;
	stream << renderEngine.ActiveRenderTarget(0)->FPS();

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
		font_->RenderText(0, 52, Color(1, 1, 1, 1), L"Instancing is enabled");
	}
	else
	{
		font_->RenderText(0, 52, Color(1, 1, 1, 1), L"Instancing is disabled");
	}
}
