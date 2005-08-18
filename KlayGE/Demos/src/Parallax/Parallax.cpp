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

#include <KlayGE/D3D9/D3D9RenderFactory.hpp>

#include <KlayGE/OCTree/OCTree.hpp>

#include <KlayGE/Input.hpp>
#include <KlayGE/DInput/DInputFactory.hpp>

#include <vector>
#include <sstream>
#include <ctime>

#include "Parallax.hpp"

using namespace std;
using namespace KlayGE;

namespace
{
	class RenderPolygon : public RenderableHelper
	{
	public:
		RenderPolygon()
		{
			effect_ = Context::Instance().RenderFactoryInstance().LoadEffect("parallax.fx");
			effect_->SetTechnique("Parallax");

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

			vb_.reset(new VertexBuffer(VertexBuffer::BT_TriangleList));

			vb_->AddVertexStream(VST_Positions, sizeof(float), 3, true);
			vb_->AddVertexStream(VST_TextureCoords0, sizeof(float), 2, true);
			vb_->AddVertexStream(VST_TextureCoords1, sizeof(float), 3, true);
			vb_->AddVertexStream(VST_TextureCoords2, sizeof(float), 3, true);
			vb_->GetVertexStream(VST_Positions)->Assign(xyzs, sizeof(xyzs) / sizeof(xyzs[0]));
			vb_->GetVertexStream(VST_TextureCoords0)->Assign(texs, sizeof(texs) / sizeof(texs[0]));
			vb_->GetVertexStream(VST_TextureCoords1)->Assign(t, sizeof(t) / sizeof(t[0]));
			vb_->GetVertexStream(VST_TextureCoords2)->Assign(b, sizeof(b) / sizeof(b[0]));

			vb_->AddIndexStream(true);
			vb_->GetIndexStream()->Assign(indices, sizeof(indices) / sizeof(uint16_t));

			box_ = MathLib::ComputeBoundingBox<float>(&xyzs[0], &xyzs[4]);
		}

		std::wstring const & Name() const
		{
			static const std::wstring name(L"Polygon");
			return name;
		}
	};


	enum
	{
		Exit,
	};

	InputAction actions[] = 
	{
		InputAction(Exit, KS_Escape),
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

	renderPolygon_.reset(new RenderPolygon);
	renderPolygon_->AddToSceneManager();

	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

	renderEngine.ClearColor(Color(0.2f, 0.4f, 0.6f, 1));

	this->LookAt(Vector3(2, 0, -2), Vector3(0, 0, 0));
	this->Proj(0.1f, 100);

	fpcController_.AttachCamera(this->ActiveCamera());
	fpcController_.Scalers(0.005f, 0.1f);

	InputEngine& inputEngine(Context::Instance().InputFactoryInstance().InputEngineInstance());
	KlayGE::InputActionMap actionMap;
	actionMap.AddActions(actions, actions + sizeof(actions) / sizeof(actions[0]));
	action_map_id_ = inputEngine.ActionMap(actionMap, true);
}

void Parallax::Update(uint32_t pass)
{
	fpcController_.Update();

	InputEngine& inputEngine(Context::Instance().InputFactoryInstance().InputEngineInstance());
	InputActionsType actions(inputEngine.Update(action_map_id_));
	for (InputActionsType::iterator iter = actions.begin(); iter != actions.end(); ++ iter)
	{
		switch (iter->first)
		{
		case Exit:
			this->Quit();
			break;
		}
	}

	Matrix4 model = MathLib::RotationX(-0.5f);
	Matrix4 view = this->ActiveCamera().ViewMatrix();
	Matrix4 proj = this->ActiveCamera().ProjMatrix();
	Vector3 eyePos = this->ActiveCamera().EyePos();

	*(renderPolygon_->GetRenderEffect()->ParameterByName("worldviewproj")) = model * view * proj;
	*(renderPolygon_->GetRenderEffect()->ParameterByName("eyePos")) = Vector4(eyePos.x(), eyePos.y(), eyePos.z(), 1);


	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

	float degree(std::clock() / 700.0f);
	Vector3 lightPos(2, 0, -2);
	Matrix4 matRot(MathLib::RotationZ(degree));
	lightPos = MathLib::TransformCoord(lightPos, matRot);
	*(renderPolygon_->GetRenderEffect()->ParameterByName("lightPos")) = Vector4(lightPos.x(), lightPos.y(), lightPos.z(), 1);

	std::wostringstream stream;
	stream << renderEngine.ActiveRenderTarget(0)->FPS();

	font_->RenderText(0, 0, Color(1, 1, 0, 1), L"Parallax²âÊÔ");
	font_->RenderText(0, 18, Color(1, 1, 0, 1), stream.str().c_str());

	SceneManager& sceneMgr(Context::Instance().SceneManagerInstance());
	stream.str(L"");
	stream << sceneMgr.NumObjectsRendered() << " Renderables "
		<< sceneMgr.NumPrimitivesRendered() << " Primitives "
		<< sceneMgr.NumVerticesRendered() << " Vertices";
	font_->RenderText(0, 36, Color(1, 1, 1, 1), stream.str().c_str());
}
