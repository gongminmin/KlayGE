#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/VertexBuffer.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/Renderable.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/SceneManager.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/ResLoader.hpp>

#include <KlayGE/D3D9/D3D9RenderSettings.hpp>
#include <KlayGE/D3D9/D3D9RenderFactory.hpp>

#include <KlayGE/OCTree/OCTree.hpp>

#include <KlayGE/Input.hpp>
#include <KlayGE/DInput/DInputFactory.hpp>

#include <vector>
#include <sstream>
#include <ctime>

#include "Displacement.hpp"

using namespace std;
using namespace KlayGE;

namespace
{
	class RenderPolygon : public Renderable
	{
	public:
		RenderPolygon()
			: vb_(new VertexBuffer(VertexBuffer::BT_TriangleList))
		{
			effect_ = LoadRenderEffect("Displacement.fx");
			*(effect_->ParameterByName("diffusemap")) = LoadTexture("diffuse.dds");
			*(effect_->ParameterByName("normalmap")) = LoadTexture("normal.dds");
			*(effect_->ParameterByName("distancemap")) = LoadTexture("distance.dds");
			effect_->SetTechnique("Displacement");

			Vector3 xyzs[] =
			{
				Vector3(-1, 1,  0),
				Vector3(1,	1,	0),
				Vector3(1,	-1,	1),
				Vector3(-1, -1, 1),
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

			MathLib::ComputeBoundingBox(box_, &xyzs[0], &xyzs[4]);

			Vector3 t[4], b[4];
			MathLib::ComputeTangent(t, b,
				indices, indices + sizeof(indices) / sizeof(indices[0]),
				xyzs, xyzs + sizeof(xyzs) / sizeof(xyzs[0]), texs);

			vb_->AddVertexStream(VST_Positions, sizeof(float), 3);
			vb_->AddVertexStream(VST_TextureCoords0, sizeof(float), 2);
			vb_->AddVertexStream(VST_TextureCoords1, sizeof(float), 3);
			vb_->AddVertexStream(VST_TextureCoords2, sizeof(float), 3);
			vb_->GetVertexStream(VST_Positions)->Assign(xyzs, sizeof(xyzs) / sizeof(xyzs[0]));
			vb_->GetVertexStream(VST_TextureCoords0)->Assign(texs, sizeof(texs) / sizeof(texs[0]));
			vb_->GetVertexStream(VST_TextureCoords1)->Assign(t, sizeof(t) / sizeof(t[0]));
			vb_->GetVertexStream(VST_TextureCoords2)->Assign(b, sizeof(b) / sizeof(b[0]));

			vb_->AddIndexStream();
			vb_->GetIndexStream()->Assign(indices, sizeof(indices) / sizeof(uint16_t));
		}

		RenderEffectPtr GetRenderEffect() const
		{
			return effect_;
		}

		VertexBufferPtr GetVertexBuffer() const
		{
			return vb_;
		}

		Box GetBound() const
		{
			return box_;
		}

		std::wstring const & Name() const
		{
			static const std::wstring name(L"Polygon");
			return name;
		}

		KlayGE::VertexBufferPtr vb_;
		KlayGE::RenderEffectPtr effect_;

		Box box_;
	};

	boost::shared_ptr<RenderPolygon> renderPolygon;


	enum
	{
		TurnLeftRight,
		TurnUpDown,

		Forward,
		Backward,
		MoveLeft,
		MoveRight,
	};

	InputAction actions[] = 
	{
		InputAction(TurnLeftRight, MS_X),
		InputAction(TurnUpDown, MS_Y),

		InputAction(Forward, KS_W),
		InputAction(Backward, KS_S),
		InputAction(MoveLeft, KS_A),
		InputAction(MoveRight, KS_D),
	};
}


class TheRenderSettings : public D3D9RenderSettings
{
private:
	bool DoConfirmDevice(D3DCAPS9 const & caps, uint32_t behavior, D3DFORMAT format) const
	{
		if (caps.VertexShaderVersion < D3DVS_VERSION(1, 1))
		{
			return false;
		}
		if (caps.PixelShaderVersion < D3DPS_VERSION(2, 0))
		{
			return false;
		}
		return true;
	}
};

int main()
{
	Displacement app;
	OCTree sceneMgr(Box(Vector3(-10, -10, -10), Vector3(10, 10, 10)));

	Context::Instance().RenderFactoryInstance(D3D9RenderFactoryInstance());
	Context::Instance().SceneManagerInstance(sceneMgr);

	Context::Instance().InputFactoryInstance(DInputFactoryInstance());

	TheRenderSettings settings;
	settings.width = 800;
	settings.height = 600;
	settings.colorDepth = 32;
	settings.fullScreen = false;

	app.Create("Displacement", settings);
	app.Run();

	return 0;
}

Displacement::Displacement()
{
	ResLoader::Instance().AddPath("../media");
	ResLoader::Instance().AddPath("../media/Displacement");
}

void Displacement::InitObjects()
{
	// ½¨Á¢×ÖÌå
	font_ = Context::Instance().RenderFactoryInstance().MakeFont("SIMYOU.TTF", 16);

	renderPolygon = boost::shared_ptr<RenderPolygon>(new RenderPolygon);

	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

	renderEngine.ClearColor(Color(0.2f, 0.4f, 0.6f, 1));

	fpcController_.AttachCamera(this->ActiveCamera());
	fpcController_.Scalers(0.005f, 0.1f);

	this->LookAt(Vector3(2, 0, -2), Vector3(0, 0, 0));
	this->Proj(0.1f, 20.0f);

	InputEngine& inputEngine(Context::Instance().InputFactoryInstance().InputEngineInstance());
	KlayGE::InputActionMap actionMap;
	actionMap.AddActions(actions, actions + sizeof(actions) / sizeof(actions[0]));
	inputEngine.ActionMap(actionMap, true);
}

void Displacement::Update()
{
	static clock_t lastTime(std::clock());
	clock_t curTime(std::clock());
	if (curTime - lastTime > 5)
	{
		float scaler = (curTime - lastTime) / 100.0f;

		lastTime = curTime;

		InputEngine& inputEngine(Context::Instance().InputFactoryInstance().InputEngineInstance());
		InputActionsType actions(inputEngine.Update());
		for (InputActionsType::iterator iter = actions.begin(); iter != actions.end(); ++ iter)
		{
			switch (iter->first)
			{
			case TurnLeftRight:
				fpcController_.Rotate(iter->second * scaler, 0, 0);
				break;

			case TurnUpDown:
				fpcController_.Rotate(0, iter->second * scaler, 0);
				break;

			case Forward:
				fpcController_.Move(0, 0, scaler);
				break;

			case Backward:
				fpcController_.Move(0, 0, -scaler);
				break;

			case MoveLeft:
				fpcController_.Move(-scaler, 0, 0);
				break;

			case MoveRight:
				fpcController_.Move(scaler, 0, 0);
				break;
			}
		}
	}
	else
	{
		fpcController_.Update();
	}

	Matrix4 view = this->ActiveCamera().ViewMatrix();
	Matrix4 proj = this->ActiveCamera().ProjMatrix();
	Vector3 eyePos = this->ActiveCamera().EyePos();

	*(renderPolygon->effect_->ParameterByName("worldviewproj")) = view * proj;
	*(renderPolygon->effect_->ParameterByName("eyePos")) = Vector4(eyePos.x(), eyePos.y(), eyePos.z(), 0.0f);


	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

	float degree(std::clock() / 700.0f);
	Vector3 lightPos(2, 0, -2);
	Matrix4 matRot;
	MathLib::RotationZ(matRot, degree);
	MathLib::TransformCoord(lightPos, lightPos, matRot);
	*(renderPolygon->effect_->ParameterByName("lightPos")) = Vector4(lightPos.x(), lightPos.y(), lightPos.z(), 1);

	std::wostringstream stream;
	stream << (*renderEngine.ActiveRenderTarget())->FPS();

	renderPolygon->Render();
	font_->RenderText(0, 0, Color(1, 1, 0, 1), L"Displacement²âÊÔ");
	font_->RenderText(0, 18, Color(1, 1, 0, 1), stream.str().c_str());
}
