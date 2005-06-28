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
#include <KlayGE/Texture.hpp>
#include <KlayGE/RenderableHelper.hpp>

#include <KlayGE/D3D9/D3D9RenderSettings.hpp>
#include <KlayGE/D3D9/D3D9RenderFactory.hpp>

#include <KlayGE/OCTree/OCTree.hpp>

#include <KlayGE/Input.hpp>
#include <KlayGE/DInput/DInputFactory.hpp>

#include <vector>
#include <sstream>
#include <ctime>

#include "Refract.hpp"

using namespace std;
using namespace KlayGE;

namespace
{
	class RenderBox : public Renderable
	{
	public:
		RenderBox(Box const & box)
			: box_(box),
				vb_(new VertexBuffer(VertexBuffer::BT_TriangleList))
		{
			effect_ = LoadRenderEffect("Refract.fx");
			effect_->SetTechnique("Refract");

			Vector3 xyzs[] =
			{
				box[0], box[1], box[2], box[3], box[4], box[5], box[6], box[7]
			};

			uint16_t indices[] =
			{
				0, 1, 2, 2, 3, 0,
				7, 6, 5, 5, 4, 7,
				4, 0, 3, 3, 7, 4,
				4, 5, 1, 1, 0, 4,
				1, 5, 6, 6, 2, 1,
				3, 2, 6, 6, 7, 3,
			};

			Vector3 nors[sizeof(xyzs) / sizeof(xyzs[0])];
			MathLib::ComputeNormal<float>(nors, indices, indices + sizeof(indices) / sizeof(indices[0]),
				xyzs, xyzs + sizeof(xyzs) / sizeof(xyzs[0]));

			vb_->AddVertexStream(VST_Positions, sizeof(float), 3, true);
			vb_->GetVertexStream(VST_Positions)->Assign(xyzs, sizeof(xyzs) / sizeof(xyzs[0]));
			vb_->AddVertexStream(VST_Normals, sizeof(float), 3, true);
			vb_->GetVertexStream(VST_Normals)->Assign(nors, sizeof(nors) / sizeof(nors[0]));
			
			vb_->AddIndexStream();
			vb_->GetIndexStream()->Assign(indices, sizeof(indices) / sizeof(indices[0]));
		}

		void CubeMap(TexturePtr const & texture)
		{
			*(effect_->ParameterByName("cubemap")) = texture;	
		}

		void MVPMatrix(Matrix4 const & model, Matrix4 const & view, Matrix4 const & proj)
		{
			*(effect_->ParameterByName("model")) = model;
			*(effect_->ParameterByName("modelit")) = MathLib::Transpose(MathLib::Inverse(model));
			*(effect_->ParameterByName("mvp")) = model * view * proj;
		}

		RenderEffectPtr GetRenderEffect() const
		{
			return effect_;
		}

		VertexBufferPtr GetVertexBuffer() const
		{
			return vb_;
		}

		std::wstring const & Name() const
		{
			static std::wstring const name(L"Box");
			return name;
		}

		Box GetBound() const
		{
			return box_;
		}

	private:
		VertexBufferPtr vb_;
		RenderEffectPtr effect_;

		Box box_;
	};


	enum
	{
		Exit,
	};

	InputAction actions[] = 
	{
		InputAction(Exit, KS_Escape),
	};

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
}

int main()
{
	SceneManager sceneMgr;
	Context::Instance().RenderFactoryInstance(D3D9RenderFactoryInstance());
	Context::Instance().SceneManagerInstance(sceneMgr);

	Context::Instance().InputFactoryInstance(DInputFactoryInstance());

	TheRenderSettings settings;
	settings.width = 800;
	settings.height = 600;
	settings.colorDepth = 32;
	settings.fullScreen = false;

	Refract app;
	app.Create("Refract", settings);
	app.Run();

	return 0;
}

Refract::Refract()
{
	ResLoader::Instance().AddPath("../media");
	ResLoader::Instance().AddPath("../media/Refract");
}

void Refract::InitObjects()
{
	// ½¨Á¢×ÖÌå
	font_ = Context::Instance().RenderFactoryInstance().MakeFont("gbsn00lp.ttf", 16);

	renderBox_.reset(new RenderBox(Box(Vector3(-1, -1, -1), Vector3(1, 1, 1))));
	static_cast<RenderBox*>(renderBox_.get())->CubeMap(LoadTexture("Glacier2.dds"));
	renderBox_->AddToSceneManager();

	renderSkyBox_.reset(new RenderableSkyBox);
	static_cast<RenderableSkyBox*>(renderSkyBox_.get())->CubeMap(LoadTexture("Glacier2.dds"));
	renderSkyBox_->AddToSceneManager();

	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

	renderEngine.ClearColor(Color(0.2f, 0.4f, 0.6f, 1));

	this->LookAt(Vector3(4, 0, -4), Vector3(0, 0, 0));
	this->Proj(0.1f, 100);

	fpcController_.AttachCamera(this->ActiveCamera());
	fpcController_.Scalers(0.05f, 1);

	InputEngine& inputEngine(Context::Instance().InputFactoryInstance().InputEngineInstance());
	KlayGE::InputActionMap actionMap;
	actionMap.AddActions(actions, actions + sizeof(actions) / sizeof(actions[0]));
	action_map_id_ = inputEngine.ActionMap(actionMap, true);
}

void Refract::Update()
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

	Matrix4 view = this->ActiveCamera().ViewMatrix();
	Matrix4 proj = this->ActiveCamera().ProjMatrix();
	static_cast<RenderableSkyBox*>(renderSkyBox_.get())->MVPMatrix(Matrix4::Identity(), view, proj);
	static_cast<RenderBox*>(renderBox_.get())->MVPMatrix(Matrix4::Identity(), view, proj);

	*(renderBox_->GetRenderEffect()->ParameterByName("eyePos"))
		= Vector4(this->ActiveCamera().EyePos().x(), this->ActiveCamera().EyePos().y(),
			this->ActiveCamera().EyePos().z(), 1);

	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
	std::wostringstream stream;
	stream << (*renderEngine.ActiveRenderTarget())->FPS();

	font_->RenderText(0, 0, Color(1, 1, 0, 1), L"Refract");
	font_->RenderText(0, 18, Color(1, 1, 0, 1), stream.str().c_str());
}
