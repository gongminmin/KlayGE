#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>
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
#include <KlayGE/SceneObjectHelper.hpp>

#include <KlayGE/D3D9/D3D9RenderFactory.hpp>

#include <KlayGE/OCTree/OCTree.hpp>

#include <KlayGE/Input.hpp>
#include <KlayGE/DInput/DInputFactory.hpp>

#include <vector>
#include <sstream>
#include <ctime>

#include "DistanceMapping.hpp"

using namespace std;
using namespace KlayGE;

namespace
{
	class RenderPolygon : public RenderableHelper
	{
	public:
		RenderPolygon()
			: RenderableHelper(L"Polygon")
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();

			effect_ = rf.LoadEffect("DistanceMapping.fx");

			if (!effect_->ValidateTechnique("DistanceMapping30"))
			{
				if (!effect_->ValidateTechnique("DistanceMapping2a"))
				{
					effect_->ActiveTechnique("DistanceMapping20");
				}
				else
				{
					effect_->ActiveTechnique("DistanceMapping2a");
				}
			}
			else
			{
				effect_->ActiveTechnique("DistanceMapping30");
			}

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

			SamplerPtr distance_sampler(new Sampler);
			distance_sampler->SetTexture(LoadTexture("distance.dds"));
			distance_sampler->Filtering(Sampler::TFO_Bilinear);
			distance_sampler->AddressingMode(Sampler::TAT_Addr_U, Sampler::TAM_Wrap);
			distance_sampler->AddressingMode(Sampler::TAT_Addr_V, Sampler::TAM_Wrap);
			distance_sampler->AddressingMode(Sampler::TAT_Addr_W, Sampler::TAM_Clamp);
			*(effect_->ParameterByName("distanceMapSampler")) = distance_sampler;

			SamplerPtr normalizer_sampler(new Sampler);
			normalizer_sampler->SetTexture(LoadTexture("normalizer.dds"));
			normalizer_sampler->Filtering(Sampler::TFO_Point);
			normalizer_sampler->AddressingMode(Sampler::TAT_Addr_U, Sampler::TAM_Clamp);
			normalizer_sampler->AddressingMode(Sampler::TAT_Addr_V, Sampler::TAM_Clamp);
			normalizer_sampler->AddressingMode(Sampler::TAT_Addr_W, Sampler::TAM_Clamp);
			*(effect_->ParameterByName("normalizerMapSampler")) = normalizer_sampler;

			Vector3 xyzs[] =
			{
				Vector3(-1, 1,  0),
				Vector3(1,	1,	0),
				Vector3(1,	-1,	0),
				Vector3(-1, -1, 0),
			};

			Vector2 texs[] =
			{
				Vector2(0, 0),
				Vector2(1, 0),
				Vector2(1, 1),
				Vector2(0, 1)
			};

			uint16_t indices[] = 
			{
				0, 1, 2, 2, 3, 0
			};

			Vector3 t[4], b[4];
			MathLib::ComputeTangent<float>(t, b,
				indices, indices + sizeof(indices) / sizeof(indices[0]),
				xyzs, xyzs + sizeof(xyzs) / sizeof(xyzs[0]), texs);

			vb_ = rf.MakeVertexBuffer(VertexBuffer::BT_TriangleList);

			VertexStreamPtr pos_vs = rf.MakeVertexStream(boost::make_tuple(vertex_element(VEU_Position, 0, sizeof(float), 3)), true);
			pos_vs->Assign(xyzs, sizeof(xyzs) / sizeof(xyzs[0]));
			VertexStreamPtr tex0_vs = rf.MakeVertexStream(boost::make_tuple(vertex_element(VEU_TextureCoord, 0, sizeof(float), 2)), true);
			tex0_vs->Assign(texs, sizeof(texs) / sizeof(texs[0]));
			VertexStreamPtr tan_vs = rf.MakeVertexStream(boost::make_tuple(vertex_element(VEU_Tangent, 0, sizeof(float), 3)), true);
			tan_vs->Assign(t, sizeof(t) / sizeof(t[0]));
			VertexStreamPtr binormal_vs = rf.MakeVertexStream(boost::make_tuple(vertex_element(VEU_Binormal, 0, sizeof(float), 3)), true);
			binormal_vs->Assign(b, sizeof(b) / sizeof(b[0]));

			vb_->AddVertexStream(pos_vs);
			vb_->AddVertexStream(tex0_vs);
			vb_->AddVertexStream(tan_vs);
			vb_->AddVertexStream(binormal_vs);

			IndexStreamPtr is = rf.MakeIndexStream(true);
			is->Assign(indices, sizeof(indices) / sizeof(uint16_t));
			vb_->SetIndexStream(is);

			box_ = MathLib::ComputeBoundingBox<float>(&xyzs[0], &xyzs[4]);
		}

		void OnRenderBegin()
		{
			App3DFramework const & app = Context::Instance().AppInstance();

			Matrix4 model = MathLib::RotationX(-0.5f);
			Matrix4 const & view = app.ActiveCamera().ViewMatrix();
			Matrix4 const & proj = app.ActiveCamera().ProjMatrix();

			*(effect_->ParameterByName("worldviewproj")) = model * view * proj;
			*(effect_->ParameterByName("eyePos")) = app.ActiveCamera().EyePos();

			float degree(std::clock() / 700.0f);
			Vector3 lightPos(2, 0, -2);
			Matrix4 matRot(MathLib::RotationZ(degree));
			lightPos = MathLib::TransformCoord(lightPos, matRot);
			*(effect_->ParameterByName("lightPos")) = lightPos;
		}
	};

	class PolygonObject : public SceneObjectHelper
	{
	public:
		PolygonObject()
			: SceneObjectHelper(RenderablePtr(new RenderPolygon), SOA_Cullable)
		{
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
	OCTree sceneMgr(Box(Vector3(-20, -20, -20), Vector3(20, 20, 20)), 3);

	Context::Instance().RenderFactoryInstance(D3D9RenderFactoryInstance());
	Context::Instance().SceneManagerInstance(sceneMgr);

	Context::Instance().InputFactoryInstance(DInputFactoryInstance());

	RenderSettings settings;
	settings.width = 800;
	settings.height = 600;
	settings.colorDepth = 32;
	settings.fullScreen = false;
	settings.ConfirmDevice = ConfirmDevice;

	DistanceMapping app;
	app.Create("DistanceMapping", settings);
	app.Run();

	return 0;
}

DistanceMapping::DistanceMapping()
{
	ResLoader::Instance().AddPath("../media/Common");
	ResLoader::Instance().AddPath("../media/DistanceMapping");
}

void DistanceMapping::InitObjects()
{
	// ½¨Á¢×ÖÌå
	font_ = Context::Instance().RenderFactoryInstance().MakeFont("gkai00mp.ttf", 16);

	polygon_.reset(new PolygonObject);
	polygon_->AddToSceneManager();

	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

	renderEngine.ClearColor(Color(0.2f, 0.4f, 0.6f, 1));

	this->LookAt(Vector3(2, 0, -2), Vector3(0, 0, 0));
	this->Proj(0.1f, 100);

	fpcController_.AttachCamera(this->ActiveCamera());
	fpcController_.Scalers(0.05f, 0.1f);

	InputEngine& inputEngine(Context::Instance().InputFactoryInstance().InputEngineInstance());
	KlayGE::InputActionMap actionMap;
	actionMap.AddActions(actions, actions + sizeof(actions) / sizeof(actions[0]));

	action_handler_t input_handler(inputEngine);
	input_handler += boost::bind(&DistanceMapping::InputHandler, this, _1, _2);
	inputEngine.ActionMap(actionMap, input_handler, true);
}

void DistanceMapping::InputHandler(KlayGE::InputEngine const & sender, KlayGE::InputAction const & action)
{
	switch (action.first)
	{
	case Exit:
		this->Quit();
		break;
	}
}

void DistanceMapping::DoUpdate(KlayGE::uint32_t pass)
{
	fpcController_.Update();

	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
	renderEngine.Clear(RenderEngine::CBM_Color | RenderEngine::CBM_Depth);

	std::wostringstream stream;
	stream << renderEngine.ActiveRenderTarget(0)->FPS();

	font_->RenderText(0, 0, Color(1, 1, 0, 1), L"DistanceMapping²âÊÔ");
	font_->RenderText(0, 18, Color(1, 1, 0, 1), stream.str().c_str());

	SceneManager& sceneMgr(Context::Instance().SceneManagerInstance());
	stream.str(L"");
	stream << sceneMgr.NumRenderablesRendered() << " Renderables "
		<< sceneMgr.NumPrimitivesRendered() << " Primitives "
		<< sceneMgr.NumVerticesRendered() << " Vertices";
	font_->RenderText(0, 36, Color(1, 1, 1, 1), stream.str().c_str());
}
