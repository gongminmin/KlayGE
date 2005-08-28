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
#include <KlayGE/Texture.hpp>
#include <KlayGE/RenderSettings.hpp>
#include <KlayGE/Sampler.hpp>

#include <KlayGE/D3D9/D3D9RenderFactory.hpp>

#include <vector>
#include <sstream>
#include <ctime>

#include "Fractal.hpp"

using namespace std;
using namespace KlayGE;

int const WIDTH = 800;
int const HEIGHT = 600;

namespace
{
	class RenderFractal : public RenderableHelper
	{
	public:
		explicit RenderFractal(TexturePtr texture)
			: RenderableHelper(L"Fractal", true, true)
		{
			effect_ = Context::Instance().RenderFactoryInstance().LoadEffect("Fractal.fx");

			SamplerPtr sampler(new Sampler);
			sampler->Filtering(Sampler::TFO_Point);
			sampler->AddressingMode(Sampler::TAT_Addr_U, Sampler::TAM_Wrap);
			sampler->AddressingMode(Sampler::TAT_Addr_V, Sampler::TAM_Wrap);
			sampler->SetTexture(texture);
			*(effect_->ParameterByName("fractal_sampler")) = sampler;

			Vector3 xyzs[] =
			{
				Vector3(-1, 1, 1),
				Vector3(1, 1, 1),
				Vector3(1, -1, 1),
				Vector3(-1, -1, 1),
			};

			Vector2 texs[] =
			{
				Vector2(0, 0),
				Vector2(1, 0),
				Vector2(1, 1),
				Vector2(0, 1),
			};

			uint16_t indices[] = 
			{
				0, 1, 2, 2, 3, 0,
			};

			vb_.reset(new VertexBuffer(VertexBuffer::BT_TriangleList));

			vb_->AddVertexStream(VST_Positions, sizeof(float), 3);
			vb_->AddVertexStream(VST_TextureCoords0, sizeof(float), 2);
			vb_->GetVertexStream(VST_Positions)->Assign(xyzs, sizeof(xyzs) / sizeof(xyzs[0]));
			vb_->GetVertexStream(VST_TextureCoords0)->Assign(texs, sizeof(texs) / sizeof(texs[0]));

			vb_->AddIndexStream();
			vb_->GetIndexStream()->Assign(indices, sizeof(indices) / sizeof(uint16_t));

			box_ = MathLib::ComputeBoundingBox<float>(&xyzs[0], &xyzs[0] + sizeof(xyzs) / sizeof(xyzs[0]));
		}

		void OnRenderBegin()
		{
			effect_->SetTechnique("Fractal");
		}
	};

	class RenderPlane : public RenderableHelper
	{
	public:
		explicit RenderPlane(TexturePtr texture)
			: RenderableHelper(L"Plane", true, true)
		{
			effect_ = Context::Instance().RenderFactoryInstance().LoadEffect("Fractal.fx");

			SamplerPtr sampler(new Sampler);
			sampler->Filtering(Sampler::TFO_Point);
			sampler->AddressingMode(Sampler::TAT_Addr_U, Sampler::TAM_Wrap);
			sampler->AddressingMode(Sampler::TAT_Addr_V, Sampler::TAM_Wrap);
			sampler->SetTexture(texture);
			*(effect_->ParameterByName("fractal_sampler")) = sampler;

			Vector3 xyzs[] =
			{
				Vector3(-1, 1, 1),
				Vector3(1, 1, 1),
				Vector3(1, -1, 1),
				Vector3(-1, -1, 1),
			};

			Vector2 texs[] =
			{
				Vector2(0, 0),
				Vector2(1, 0),
				Vector2(1, 1),
				Vector2(0, 1),
			};

			uint16_t indices[] = 
			{
				0, 1, 2, 2, 3, 0,
			};

			vb_.reset(new VertexBuffer(VertexBuffer::BT_TriangleList));

			vb_->AddVertexStream(VST_Positions, sizeof(float), 3);
			vb_->AddVertexStream(VST_TextureCoords0, sizeof(float), 2);
			vb_->GetVertexStream(VST_Positions)->Assign(xyzs, sizeof(xyzs) / sizeof(xyzs[0]));
			vb_->GetVertexStream(VST_TextureCoords0)->Assign(texs, sizeof(texs) / sizeof(texs[0]));

			vb_->AddIndexStream();
			vb_->GetIndexStream()->Assign(indices, sizeof(indices) / sizeof(uint16_t));

			box_ = MathLib::ComputeBoundingBox<float>(&xyzs[0], &xyzs[0] + sizeof(xyzs) / sizeof(xyzs[0]));
		}

		void OnRenderBegin()
		{
			effect_->SetTechnique("Show");
		}
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
	SceneManager sceneMgr;
	Context::Instance().RenderFactoryInstance(D3D9RenderFactoryInstance());
	Context::Instance().SceneManagerInstance(sceneMgr);

	RenderSettings settings;
	settings.width = WIDTH;
	settings.height = HEIGHT;
	settings.colorDepth = 32;
	settings.fullScreen = false;
	settings.ConfirmDevice = ConfirmDevice;

	Fractal app;
	app.Create("Fractal", settings);
	app.Run();

	return 0;
}

Fractal::Fractal()
{
	ResLoader::Instance().AddPath("../media/Common");
	ResLoader::Instance().AddPath("../media/Fractal");
}

void Fractal::InitObjects()
{
	font_ = Context::Instance().RenderFactoryInstance().MakeFont("gkai00mp.ttf", 16);

	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
	renderEngine.ClearColor(Color(0.2f, 0.4f, 0.6f, 1));

	rendered_tex_[0] = Context::Instance().RenderFactoryInstance().MakeTexture2D(WIDTH, HEIGHT, 1,
		PF_A32B32G32R32F, Texture::TU_RenderTarget);
	rendered_tex_[1] = Context::Instance().RenderFactoryInstance().MakeTexture2D(WIDTH, HEIGHT, 1,
		PF_A32B32G32R32F, Texture::TU_RenderTarget);

	std::vector<Vector4> data(WIDTH * HEIGHT);
	std::fill(data.begin(), data.end(), Vector4(0, 0, 0, 0));
	rendered_tex_[0]->CopyMemoryToTexture2D(0, &data[0], PF_A32B32G32R32F, WIDTH, HEIGHT, 0, 0);
	rendered_tex_[1]->CopyMemoryToTexture2D(0, &data[0], PF_A32B32G32R32F, WIDTH, HEIGHT, 0, 0);

	render_buffer_ = Context::Instance().RenderFactoryInstance().MakeRenderTexture();
	render_buffer_->AttachTexture2D(rendered_tex_[0]);

	screen_buffer_ = renderEngine.ActiveRenderTarget(0);

	render_buffer_->GetViewport().camera = screen_buffer_->GetViewport().camera;

	renderFractal_.reset(new RenderFractal(rendered_tex_[1]));
	renderPlane_.reset(new RenderPlane(rendered_tex_[1]));
}

uint32_t Fractal::NumPasses() const
{
	return 2;
}

void Fractal::Update(uint32_t pass)
{
	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
	SceneManager& sceneMgr(Context::Instance().SceneManagerInstance());

	static int i = 0;
	switch (pass)
	{
	case 0:
		if (i < 30)
		{
			renderEngine.ActiveRenderTarget(0, render_buffer_);
			renderFractal_->AddToSceneManager();
			++ i;
		}
		break;

	case 1:
		rendered_tex_[0]->CopyToTexture(*rendered_tex_[1]);

		renderEngine.ActiveRenderTarget(0, screen_buffer_);
		renderPlane_->AddToSceneManager();

		std::wostringstream stream;
		stream << renderEngine.ActiveRenderTarget(0)->FPS();

		font_->RenderText(0, 0, Color(1, 1, 0, 1), L"GPU¼ÆËã·ÖÐÎ");
		font_->RenderText(0, 18, Color(1, 1, 0, 1), stream.str());
		break;
	}
}
