#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/VertexBuffer.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/Renderable.hpp>
#include <KlayGE/RenderWindow.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/SceneManager.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/ResLoader.hpp>

#include <KlayGE/D3D9/D3D9RenderSettings.hpp>
#include <KlayGE/D3D9/D3D9RenderFactory.hpp>

#include <KlayGE/OCTree/OCTree.hpp>

#include <sstream>
#include <ctime>

#include "Torus.hpp"
#include "Cartoon.hpp"

using namespace std;
using namespace KlayGE;

namespace
{
	struct RenderTorus : public Renderable
	{
		RenderTorus(TexturePtr const & toonTex, TexturePtr const & edgeTex)
			: vb_(new VertexBuffer(VertexBuffer::BT_TriangleList))
		{
			effect_ = LoadRenderEffect("Cartoon.fx");
			*(effect_->ParameterByName("toon")) = toonTex;
			*(effect_->ParameterByName("edge")) = edgeTex;
			effect_->SetTechnique("cartoonTec");

			MathLib::ComputeBoundingBox(box_, reinterpret_cast<Vector3*>(&Pos[0]),
				reinterpret_cast<Vector3*>(&Pos[0] + sizeof(Pos) / sizeof(float)));

			vb_->AddVertexStream(VST_Positions, sizeof(float), 3, true);
			vb_->AddVertexStream(VST_Normals, sizeof(float), 3, true);

			vb_->GetVertexStream(VST_Positions)->Assign(Pos, sizeof(Pos) / sizeof(float) / 3);
			vb_->GetVertexStream(VST_Normals)->Assign(Normal, sizeof(Normal) / sizeof(float) / 3);

			vb_->AddIndexStream(true);
			vb_->GetIndexStream()->Assign(Index, sizeof(Index) / sizeof(uint16_t));
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
			static const std::wstring name(L"Torus");
			return name;
		}

		VertexBufferPtr vb_;
		RenderEffectPtr effect_;

		Box box_;
	};

	boost::shared_ptr<RenderTorus> renderTorus;
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
		if (caps.PixelShaderVersion < D3DPS_VERSION(1, 1))
		{
			return false;
		}
		return true;
	}
};

int main()
{
	Cartoon app;
	OCTree sceneMgr(Box(Vector3(-10, -10, -10), Vector3(10, 10, 10)), 3);

	Context::Instance().RenderFactoryInstance(D3D9RenderFactoryInstance());
	Context::Instance().SceneManagerInstance(sceneMgr);

	TheRenderSettings settings;
	settings.width = 800;
	settings.height = 600;
	settings.colorDepth = 32;
	settings.fullScreen = false;

	app.Create("¿¨Í¨äÖÈ¾²âÊÔ", settings);
	app.Run();

	return 0;
}

Cartoon::Cartoon()
{
	ResLoader::Instance().AddPath("../media");
	ResLoader::Instance().AddPath("../media/Cartoon");
}

void Cartoon::InitObjects()
{
	font_ = Context::Instance().RenderFactoryInstance().MakeFont("SIMYOU.TTF", 16);

	uint8_t toonData[16] = { 120, 120, 120, 120, 120, 160, 160, 160, 160, 160, 160, 255, 255, 255, 255, 255 };
	TexturePtr toonTex = Context::Instance().RenderFactoryInstance().MakeTexture1D(sizeof(toonData) / sizeof(toonData[0]), 0, PF_L8);
	toonTex->CopyMemoryToTexture1D(0, toonData, PF_L8, 16, 0);

	uint8_t edgeData[4] = { 0, 255, 255, 255 };
	TexturePtr edgeTex = Context::Instance().RenderFactoryInstance().MakeTexture1D(sizeof(edgeData) / sizeof(edgeData[0]), 0, PF_L8);
	edgeTex->CopyMemoryToTexture1D(0, edgeData, PF_L8, 4, 0);

	renderTorus = boost::shared_ptr<RenderTorus>(new RenderTorus(toonTex, edgeTex));

	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

	renderEngine.ClearColor(Color(0.2f, 0.4f, 0.6f, 1));

	this->LookAt(Vector3(0, 0, -6), Vector3(0, 0, 0));
	this->Proj(0.1f, 20.0f);

	view_ = renderEngine.ViewMatrix();
	proj_ = renderEngine.ProjectionMatrix();

	*(renderTorus->GetRenderEffect()->ParameterByName("proj")) = proj_;
	*(renderTorus->GetRenderEffect()->ParameterByName("lightPos")) = Vector4(2, 2, -3, 1);
	*(renderTorus->GetRenderEffect()->ParameterByName("eyePos")) = Vector4(0, 0, -6, 1);

	renderTorus->AddToSceneManager();
}

void Cartoon::Update()
{
	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

	float rotX(std::clock() / 700.0f);
	float rotY(std::clock() / 700.0f);

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

	RenderWindow* rw = static_cast<RenderWindow*>(renderEngine.ActiveRenderTarget()->get());

	font_->RenderText(0, 0, Color(1, 1, 0, 1), L"¿¨Í¨äÖÈ¾²âÊÔ");
	font_->RenderText(0, 18, Color(1, 1, 0, 1), rw->Description());
	font_->RenderText(0, 36, Color(1, 1, 0, 1), stream.str().c_str());

	SceneManager& sceneMgr(Context::Instance().SceneManagerInstance());
	stream.str(L"");
	stream << sceneMgr.NumObjectsRendered() << " Renderables "
		<< sceneMgr.NumPrimitivesRendered() << " Primitives "
		<< sceneMgr.NumVerticesRendered() << " Vertices";
	font_->RenderText(0, 54, Color(1, 1, 1, 1), stream.str().c_str());
}
