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
#include <KlayGE/Texture.hpp>
#include <KlayGE/RenderSettings.hpp>
#include <KlayGE/Sampler.hpp>

#include <KlayGE/D3D9/D3D9RenderFactory.hpp>

#include <vector>
#include <sstream>

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
		RenderFractal()
			: RenderableHelper(L"Fractal")
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();

			effect_ = rf.LoadEffect("Fractal.fx");

			sampler_.reset(new Sampler);
			sampler_->Filtering(Sampler::TFO_Point);
			sampler_->AddressingMode(Sampler::TAT_Addr_U, Sampler::TAM_Wrap);
			sampler_->AddressingMode(Sampler::TAT_Addr_V, Sampler::TAM_Wrap);
			*(effect_->ParameterByName("fractal_sampler")) = sampler_;

			Vector3 xyzs[] =
			{
				Vector3(-1, +1, 0.5f),
				Vector3(+1, +1, 0.5f),
				Vector3(+1, -1, 0.5f),
				Vector3(-1, -1, 0.5f),
			};

			Vector2 texs[] =
			{
				Vector2(0 + 0.5f / WIDTH, 0 + 0.5f / HEIGHT),
				Vector2(1 + 0.5f / WIDTH, 0 + 0.5f / HEIGHT),
				Vector2(1 + 0.5f / WIDTH, 1 + 0.5f / HEIGHT),
				Vector2(0 + 0.5f / WIDTH, 1 + 0.5f / HEIGHT),
			};

			uint16_t indices[] = 
			{
				0, 1, 2, 2, 3, 0,
			};

			vb_ = rf.MakeVertexBuffer(VertexBuffer::BT_TriangleList);

			VertexStreamPtr pos_vs = rf.MakeVertexStream(boost::make_tuple(vertex_element(VEU_Position, 0, sizeof(float), 3)), true);
			pos_vs->Assign(xyzs, sizeof(xyzs) / sizeof(xyzs[0]));
			VertexStreamPtr tex0_vs = rf.MakeVertexStream(boost::make_tuple(vertex_element(VEU_TextureCoord, 0, sizeof(float), 2)), true);
			tex0_vs->Assign(texs, sizeof(texs) / sizeof(texs[0]));

			vb_->AddVertexStream(pos_vs);
			vb_->AddVertexStream(tex0_vs);

			IndexStreamPtr is = rf.MakeIndexStream(true);
			is->Assign(indices, sizeof(indices) / sizeof(uint16_t));
			vb_->SetIndexStream(is);

			box_ = MathLib::ComputeBoundingBox<float>(&xyzs[0], &xyzs[0] + sizeof(xyzs) / sizeof(xyzs[0]));
		}

		void SetTexture(TexturePtr texture)
		{
			sampler_->SetTexture(texture);
		}

		void OnRenderBegin()
		{
			effect_->ActiveTechnique("Mandelbrot");
		}

	private:
		SamplerPtr sampler_;
	};

	class RenderPlane : public RenderableHelper
	{
	public:
		RenderPlane()
			: RenderableHelper(L"Plane")
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();

			effect_ = rf.LoadEffect("Fractal.fx");

			sampler_.reset(new Sampler);
			sampler_->Filtering(Sampler::TFO_Point);
			sampler_->AddressingMode(Sampler::TAT_Addr_U, Sampler::TAM_Wrap);
			sampler_->AddressingMode(Sampler::TAT_Addr_V, Sampler::TAM_Wrap);
			*(effect_->ParameterByName("fractal_sampler")) = sampler_;

			Vector3 xyzs[] =
			{
				Vector3(-1, +1, 0.5f),
				Vector3(+1, +1, 0.5f),
				Vector3(+1, -1, 0.5f),
				Vector3(-1, -1, 0.5f),
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

			vb_ = rf.MakeVertexBuffer(VertexBuffer::BT_TriangleList);

			VertexStreamPtr pos_vs = rf.MakeVertexStream(boost::make_tuple(vertex_element(VEU_Position, 0, sizeof(float), 3)), true);
			pos_vs->Assign(xyzs, sizeof(xyzs) / sizeof(xyzs[0]));
			VertexStreamPtr tex0_vs = rf.MakeVertexStream(boost::make_tuple(vertex_element(VEU_TextureCoord, 0, sizeof(float), 2)), true);
			tex0_vs->Assign(texs, sizeof(texs) / sizeof(texs[0]));

			vb_->AddVertexStream(pos_vs);
			vb_->AddVertexStream(tex0_vs);

			IndexStreamPtr is = rf.MakeIndexStream(true);
			is->Assign(indices, sizeof(indices) / sizeof(uint16_t));
			vb_->SetIndexStream(is);

			box_ = MathLib::ComputeBoundingBox<float>(&xyzs[0], &xyzs[0] + sizeof(xyzs) / sizeof(xyzs[0]));
		}

		void SetTexture(TexturePtr texture)
		{
			sampler_->SetTexture(texture);
		}

		void OnRenderBegin()
		{
			effect_->ActiveTechnique("Show");
		}

	private:
		SamplerPtr sampler_;
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

	render_buffer_ = Context::Instance().RenderFactoryInstance().MakeRenderTexture();

	screen_buffer_ = renderEngine.ActiveRenderTarget(0);

	render_buffer_->GetViewport().camera = screen_buffer_->GetViewport().camera;

	renderFractal_.reset(new RenderFractal);
	renderPlane_.reset(new RenderPlane);
}

void Fractal::OnResize(uint32_t width, uint32_t height)
{
	rendered_tex_[0] = Context::Instance().RenderFactoryInstance().MakeTexture2D(width, height, 1, PF_GR16F);
	rendered_tex_[1] = Context::Instance().RenderFactoryInstance().MakeTexture2D(width, height, 1, PF_GR16F);

	std::vector<Vector4> data(width * height);
	std::fill(data.begin(), data.end(), Vector4(0, 0, 0, 0));
	rendered_tex_[0]->CopyMemoryToTexture2D(0, &data[0], PF_ABGR32F, width, height, 0, 0, width, height, 0, 0);
	rendered_tex_[1]->CopyMemoryToTexture2D(0, &data[0], PF_ABGR32F, width, height, 0, 0, width, height, 0, 0);
}

uint32_t Fractal::NumPasses() const
{
	return 2;
}

void Fractal::DoUpdate(uint32_t pass)
{
	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
	SceneManager& sceneMgr(Context::Instance().SceneManagerInstance());

	static bool odd = false;

	switch (pass)
	{
	case 0:
		render_buffer_->AttachTexture2D(rendered_tex_[!odd]);

		renderEngine.ActiveRenderTarget(0, render_buffer_);
		checked_cast<RenderFractal*>(renderFractal_.get())->SetTexture(rendered_tex_[odd]);
		renderFractal_->AddToRenderQueue();
		break;

	case 1:
		renderEngine.ActiveRenderTarget(0, screen_buffer_);
		checked_cast<RenderPlane*>(renderPlane_.get())->SetTexture(rendered_tex_[!odd]);
		renderPlane_->AddToRenderQueue();

		odd = !odd;

		std::wostringstream stream;
		stream << renderEngine.ActiveRenderTarget(0)->FPS();

		font_->RenderText(0, 0, Color(1, 1, 0, 1), L"GPU¼ÆËã·ÖÐÎ");
		font_->RenderText(0, 18, Color(1, 1, 0, 1), stream.str());
		break;
	}
}
