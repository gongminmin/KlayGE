#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/SharedPtr.hpp>
#include <KlayGE/RenderBuffer.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/Renderable.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/alloc.hpp>
#include <KlayGE/SceneManager.hpp>
#include <KlayGE/Engine.hpp>

#include <KlayGE/D3D9/D3D9RenderSettings.hpp>
#include <KlayGE/D3D9/D3D9RenderFactory.hpp>

#include <sstream>

#include "Torus.hpp"
#include "Cartoon.hpp"

using namespace KlayGE;

namespace
{
	struct RenderTorus : public Renderable
	{
		RenderTorus(const TexturePtr& texture)
			: rb_(new RenderBuffer(RenderBuffer::BT_TriangleList))
		{
			effect_ = LoadRenderEffect(L"Cartoon.fx");
			effect_->SetTexture("cartoonTex", texture);
			effect_->SetTechnique("cartoonTec");

			rb_->AddVertexStream(VST_Positions, sizeof(float), 3, true);
			rb_->AddVertexStream(VST_Normals, sizeof(float), 3, true);

			rb_->GetVertexStream(VST_Positions)->Assign(Pos, sizeof(Pos) / sizeof(float) / 3);
			rb_->GetVertexStream(VST_Normals)->Assign(Normal, sizeof(Normal) / sizeof(float) / 3);

			rb_->AddIndexStream(true);
			rb_->GetIndexStream()->Assign(Index, sizeof(Index) / sizeof(U16));
		}

		RenderEffectPtr GetRenderEffect() const
			{ return effect_; }

		RenderBufferPtr GetRenderBuffer() const
			{ return rb_; }

		const WString& Name() const
		{
			static WString name(L"Torus");
			return name;
		}

		KlayGE::RenderBufferPtr rb_;
		KlayGE::RenderEffectPtr effect_;
	};

	SharedPtr<RenderTorus> renderTorus;
}


int main()
{
	Cartoon app;

	Engine::RenderFactoryInstance(D3D9RenderFactoryInstance());

	D3D9RenderSettings settings;
	settings.width = 800;
	settings.height = 600;
	settings.colorDepth = 32;
	settings.fullScreen = false;

	app.Create("Cartoon", settings);
	app.Run();

	return 0;
}

Cartoon::Cartoon()
			: rotX(0), rotY(0)
{
}

void Cartoon::InitObjects()
{
	font_ = Engine::RenderFactoryInstance().MakeFont(L"ËÎÌå", 16);

	TexturePtr texture = Engine::RenderFactoryInstance().MakeTexture(16, 1, 0, PF_X8R8G8B8);
	U32 cartoolShadeData[16] = {
		(255 << 24) + (0 << 16) + (0 << 8) + 0, 
		(255 << 24) + (0 << 16) + (0 << 8) + 0, 
		(255 << 24) + (0 << 16) + (0 << 8) + 0, 

		(255 << 24) + (120 << 16) + (120 << 8) + 120, 
		(255 << 24) + (120 << 16) + (120 << 8) + 120, 
		(255 << 24) + (120 << 16) + (120 << 8) + 120, 

		(255 << 24) + (160 << 16) + (160 << 8) + 160, 
		(255 << 24) + (160 << 16) + (160 << 8) + 160, 
		(255 << 24) + (160 << 16) + (160 << 8) + 160,
		(255 << 24) + (160 << 16) + (160 << 8) + 160,
		(255 << 24) + (160 << 16) + (160 << 8) + 160,
		(255 << 24) + (160 << 16) + (160 << 8) + 160,

		(255 << 24) + (255 << 16) + (255 << 8) + 255,
		(255 << 24) + (255 << 16) + (255 << 8) + 255,
		(255 << 24) + (255 << 16) + (255 << 8) + 255,
		(255 << 24) + (255 << 16) + (255 << 8) + 255,
	};
	texture->CopyMemoryToTexture(cartoolShadeData, PF_X8R8G8B8);


	renderTorus = SharedPtr<RenderTorus>(new RenderTorus(texture));

	RenderEngine& renderEngine(Engine::RenderFactoryInstance().RenderEngineInstance());

	renderEngine.ClearColor(Color(0.2f, 0.4f, 0.6f, 1));

	this->LookAt(MakeVector(0.0f, 0.0f, -6.0f), MakeVector(0.0f, 0.0f, 0.0f));
	this->Proj(0.1f, 20.0f);

	renderTorus->GetRenderEffect()->SetMatrix("proj", renderEngine.ProjectionMatrix());
}

void Cartoon::Update()
{
	RenderEngine& renderEngine(Engine::RenderFactoryInstance().RenderEngineInstance());
	MathLib& math(Engine::MathInstance());

	rotX += 0.003f;
	rotY += 0.003f;

	Matrix4 mat, matY;
	math.RotationX(mat, rotX);
	math.RotationY(matY, rotY);
	mat *= matY;

	renderTorus->GetRenderEffect()->SetMatrix("worldview", mat * renderEngine.ViewMatrix());

	std::wostringstream stream;
	stream << (*renderEngine.ActiveRenderTarget())->FPS();

	Engine::SceneManagerInstance().PushRenderable(renderTorus);
	Engine::SceneManagerInstance().PushRenderable(font_->RenderText(0, 0, Color(1, 1, 0, 1), L"¿¨Í¨äÖÈ¾²âÊÔ"));
	Engine::SceneManagerInstance().PushRenderable(font_->RenderText(0, 18, Color(1, 1, 0, 1), stream.str().c_str()));
}
