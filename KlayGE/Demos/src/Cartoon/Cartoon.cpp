#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/RenderBuffer.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/Renderable.hpp>
#include <KlayGE/RenderWindow.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/SceneManager.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/ResLocator.hpp>

#include <KlayGE/D3D9/D3D9RenderSettings.hpp>
#include <KlayGE/D3D9/D3D9RenderFactory.hpp>

#include <KlayGE/OCTree/OCTree.hpp>

#include <iostream>
#include <sstream>

#include "Torus.hpp"
#include "Cartoon.hpp"

using namespace std;
using namespace KlayGE;

namespace
{
	struct RenderTorus : public Renderable
	{
		RenderTorus(const TexturePtr& texture0, const TexturePtr& texture1)
			: rb_(new RenderBuffer(RenderBuffer::BT_TriangleList))
		{
			effect_ = LoadRenderEffect("Cartoon.fx");
			*(effect_->ParameterByName("toon")) = texture0;
			*(effect_->ParameterByName("edge")) = texture1;
			effect_->SetTechnique("cartoonTec");

			float minx(Pos[0]), miny(Pos[1]), minz(Pos[2]);
			for (size_t i = 0; i < sizeof(Pos) / sizeof(float); i += 3)
			{
				if (Pos[i + 0] < minx)
				{
					minx = Pos[i + 0];
				}
				if (Pos[i + 1] < miny)
				{
					miny = Pos[i + 1];
				}
				if (Pos[i + 2] < minz)
				{
					minz = Pos[i + 2];
				}
			}

			float maxx(Pos[0]), maxy(Pos[1]), maxz(Pos[2]);
			for (size_t i = 0; i < sizeof(Pos) / sizeof(float); i += 3)
			{
				if (Pos[i + 0] > maxx)
				{
					maxx = Pos[i + 0];
				}
				if (Pos[i + 1] > maxy)
				{
					maxy = Pos[i + 1];
				}
				if (Pos[i + 2] > maxz)
				{
					maxz = Pos[i + 2];
				}
			}

			box_ = Box(Vector3(minx, miny, minz), Vector3(maxx, maxy, maxz));

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

		Box GetBound() const
			{ return box_; }

		const std::wstring& Name() const
		{
			static std::wstring name(L"Torus");
			return name;
		}

		KlayGE::RenderBufferPtr rb_;
		KlayGE::RenderEffectPtr effect_;

		Box box_;
	};

	boost::shared_ptr<RenderTorus> renderTorus;
}


int main()
{
	Cartoon app;
	OCTree sceneMgr(Box(Vector3(-20, -20, -20), Vector3(20, 20, 20)));

	Context::Instance().RenderFactoryInstance(D3D9RenderFactoryInstance());
	Context::Instance().SceneManagerInstance(sceneMgr);

	D3D9RenderSettings settings;
	settings.width = 800;
	settings.height = 600;
	settings.colorDepth = 32;
	settings.fullScreen = false;

	app.Create("¿¨Í¨äÖÈ¾²âÊÔ", settings);
	app.Run();

	return 0;
}

Cartoon::Cartoon()
			: rotX(0), rotY(0)
{
	ResLocator::Instance().AddPath("../media");
	ResLocator::Instance().AddPath("../media/Cartoon");
}

void Cartoon::InitObjects()
{
	font_ = Context::Instance().RenderFactoryInstance().MakeFont(L"ËÎÌå", 16);

	U8 cartoolShadeData0[16] = { 120, 120, 120, 120, 120, 160, 160, 160, 160, 160, 160, 255, 255, 255, 255, 255 };
	TexturePtr texture0 = Context::Instance().RenderFactoryInstance().MakeTexture(sizeof(cartoolShadeData0) / sizeof(cartoolShadeData0[0]), 1, 0, PF_L8);
	texture0->CopyMemoryToTexture(cartoolShadeData0, PF_L8);

	U8 cartoolShadeData1[4] = { 0, 255, 255, 255 };
	TexturePtr texture1 = Context::Instance().RenderFactoryInstance().MakeTexture(sizeof(cartoolShadeData1) / sizeof(cartoolShadeData1[0]), 1, 0, PF_L8);
	texture1->CopyMemoryToTexture(cartoolShadeData1, PF_L8);

	renderTorus = boost::shared_ptr<RenderTorus>(new RenderTorus(texture0, texture1));

	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

	renderEngine.ClearColor(Color(0.2f, 0.4f, 0.6f, 1));

	MathLib::LookAtLH(view_, Vector3(0, 0, -6), Vector3(0, 5, 0));
	MathLib::PerspectiveFovLH(proj_, PI / 4, 800.0f / 600, 0.1f, 20.0f);

	*(renderTorus->GetRenderEffect()->ParameterByName("proj")) = proj_;
	*(renderTorus->GetRenderEffect()->ParameterByName("lightPos")) = Vector4(2, 2, -3, 1);
	*(renderTorus->GetRenderEffect()->ParameterByName("eyePos")) = Vector4(0, 0, -6, 1);
}

void Cartoon::Update()
{
	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
	SceneManager& sceneMgr(Context::Instance().SceneManagerInstance());

	rotX += 0.003f;
	rotY += 0.003f;

	Matrix4 mat, matY;
	MathLib::RotationX(mat, rotX);
	MathLib::RotationY(matY, rotY);
	mat *= matY;
	mat *= view_;

	*(renderTorus->GetRenderEffect()->ParameterByName("worldview")) = mat;

	MathLib::Transpose(mat, mat);
	MathLib::Inverse(mat, mat);
	*(renderTorus->GetRenderEffect()->ParameterByName("worldviewIT")) = mat;

	std::wostringstream stream;
	stream << (*renderEngine.ActiveRenderTarget())->FPS();

	sceneMgr.PushRenderable(renderTorus);

	sceneMgr.PushRenderable(font_->RenderText(0, 0, Color(1, 1, 0, 1), L"¿¨Í¨äÖÈ¾²âÊÔ"));
	sceneMgr.PushRenderable(font_->RenderText(0, 18, Color(1, 1, 0, 1), stream.str().c_str()));
}
