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

#include <KlayGE/OCTree/OCTree.hpp>

#include <KlayGE/Input.hpp>
#include <KlayGE/DInput/DInputFactory.hpp>

#include <vector>
#include <sstream>
#include <ctime>
#include <boost/tuple/tuple.hpp>

#include "Parallax.hpp"

using namespace std;
using namespace KlayGE;

namespace
{
	class RenderPolygon : public KMesh
	{
	public:
		RenderPolygon(std::wstring const & /*name*/, TexturePtr tex)
			: KMesh(L"Polygon", tex)
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();

			effect_ = rf.LoadEffect("parallax.fx");
			effect_->ActiveTechnique("Parallax");

			SamplerPtr diffuse_sampler(new Sampler);
			diffuse_sampler->SetTexture(LoadTexture("diffuse.dds"));
			diffuse_sampler->Filtering(Sampler::TFO_Bilinear);
			diffuse_sampler->AddressingMode(Sampler::TAT_Addr_U, Sampler::TAM_Wrap);
			diffuse_sampler->AddressingMode(Sampler::TAT_Addr_V, Sampler::TAM_Wrap);
			*(effect_->ParameterByName("diffuseMapSampler")) = diffuse_sampler;

			SamplerPtr normal_sampler(new Sampler);
			normal_sampler->SetTexture(LoadTexture("normal.dds"));
			normal_sampler->Filtering(Sampler::TFO_Bilinear);
			normal_sampler->AddressingMode(Sampler::TAT_Addr_U, Sampler::TAM_Wrap);
			normal_sampler->AddressingMode(Sampler::TAT_Addr_V, Sampler::TAM_Wrap);
			*(effect_->ParameterByName("normalMapSampler")) = normal_sampler;

			SamplerPtr height_sampler(new Sampler);
			height_sampler->SetTexture(LoadTexture("height.dds"));
			height_sampler->Filtering(Sampler::TFO_Bilinear);
			height_sampler->AddressingMode(Sampler::TAT_Addr_U, Sampler::TAM_Wrap);
			height_sampler->AddressingMode(Sampler::TAT_Addr_V, Sampler::TAM_Wrap);
			*(effect_->ParameterByName("heightMapSampler")) = height_sampler;

			SamplerPtr normalizer_sampler(new Sampler);
			normalizer_sampler->SetTexture(LoadTexture("normalizer.dds"));
			normalizer_sampler->Filtering(Sampler::TFO_Point);
			normalizer_sampler->AddressingMode(Sampler::TAT_Addr_U, Sampler::TAM_Clamp);
			normalizer_sampler->AddressingMode(Sampler::TAT_Addr_V, Sampler::TAM_Clamp);
			normalizer_sampler->AddressingMode(Sampler::TAT_Addr_W, Sampler::TAM_Clamp);
			*(effect_->ParameterByName("normalizerMapSampler")) = normalizer_sampler;
		}

		void ComputeTB()
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();

			std::vector<Vector3> t(xyzs_.size());
			std::vector<Vector3> b(xyzs_.size());
			MathLib::ComputeTangent<float>(t.begin(), b.begin(),
				indices_.begin(), indices_.end(),
				xyzs_.begin(), xyzs_.end(), multi_tex_coords_[0].begin());

			VertexStreamPtr tan_vs = rf.MakeVertexStream(boost::make_tuple(vertex_element(VEU_Tangent, 0, sizeof(float), 3)), true);
			tan_vs->Assign(&t[0], t.size());
			VertexStreamPtr binormal_vs = rf.MakeVertexStream(boost::make_tuple(vertex_element(VEU_Binormal, 0, sizeof(float), 3)), true);
			binormal_vs->Assign(&b[0], b.size());

			vb_->AddVertexStream(tan_vs);
			vb_->AddVertexStream(binormal_vs);
		}

		void OnRenderBegin()
		{
			App3DFramework const & app = Context::Instance().AppInstance();

			Matrix4 const & model = Matrix4::Identity();
			Matrix4 const & view = app.ActiveCamera().ViewMatrix();
			Matrix4 const & proj = app.ActiveCamera().ProjMatrix();

			*(effect_->ParameterByName("mvp")) = model * view * proj;
			*(effect_->ParameterByName("eyePos")) = app.ActiveCamera().EyePos();
		}
	};

	class PolygonObject : public SceneObjectHelper
	{
	public:
		PolygonObject()
			: SceneObjectHelper(SOA_Cullable)
		{
			renderable_ = LoadKMesh("teapot.kmesh", CreateKMeshFactory<RenderPolygon>())->Mesh(0);
			checked_cast<RenderPolygon*>(renderable_.get())->ComputeTB();
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
	OCTree sceneMgr(Box(Vector3(-5, -5, -5), Vector3(5, 5, 5)), 3);
	Context::Instance().RenderFactoryInstance(D3D9RenderFactoryInstance());
	Context::Instance().SceneManagerInstance(sceneMgr);

	Context::Instance().InputFactoryInstance(DInputFactoryInstance());

	RenderSettings settings;
	settings.width = 800;
	settings.height = 600;
	settings.colorDepth = 32;
	settings.fullScreen = false;
	settings.ConfirmDevice = ConfirmDevice;

	Parallax app;
	app.Create("Parallax", settings);
	app.Run();

	return 0;
}

Parallax::Parallax()
{
	ResLoader::Instance().AddPath("../media/Common");
	ResLoader::Instance().AddPath("../media/Parallax");
}

void Parallax::InitObjects()
{
	// ½¨Á¢×ÖÌå
	font_ = Context::Instance().RenderFactoryInstance().MakeFont("gkai00mp.ttf", 16);

	polygon_.reset(new PolygonObject);
	polygon_->AddToSceneManager();

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
	input_handler += boost::bind(&Parallax::InputHandler, this, _1, _2);
	inputEngine.ActionMap(actionMap, input_handler, true);
}

void Parallax::InputHandler(InputEngine const & sender, InputAction const & action)
{
	switch (action.first)
	{
	case Exit:
		this->Quit();
		break;
	}
}

void Parallax::DoUpdate(uint32_t pass)
{
	fpcController_.Update();

	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
	renderEngine.Clear(RenderEngine::CBM_Color | RenderEngine::CBM_Depth);

	float degree(std::clock() / 700.0f);
	Vector3 lightPos(2, 0, 1);
	Matrix4 matRot(MathLib::RotationZ(degree));
	lightPos = MathLib::TransformCoord(lightPos, matRot);
	*(polygon_->GetRenderable()->GetRenderEffect()->ParameterByName("lightPos")) = lightPos;

	std::wostringstream stream;
	stream << renderEngine.ActiveRenderTarget(0)->FPS();

	font_->RenderText(0, 0, Color(1, 1, 0, 1), L"Parallax²âÊÔ");
	font_->RenderText(0, 18, Color(1, 1, 0, 1), stream.str().c_str());

	SceneManager& sceneMgr(Context::Instance().SceneManagerInstance());
	stream.str(L"");
	stream << sceneMgr.NumRenderablesRendered() << " Renderables "
		<< sceneMgr.NumPrimitivesRendered() << " Primitives "
		<< sceneMgr.NumVerticesRendered() << " Vertices";
	font_->RenderText(0, 36, Color(1, 1, 1, 1), stream.str().c_str());
}
