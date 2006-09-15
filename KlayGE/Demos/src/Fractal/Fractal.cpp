#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/GraphicsBuffer.hpp>
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

			technique_ = rf.LoadEffect("Fractal.fx")->TechniqueByName("Mandelbrot");

			sampler_.reset(new Sampler);
			sampler_->Filtering(Sampler::TFO_Point);
			sampler_->AddressingMode(Sampler::TAT_Addr_U, Sampler::TAM_Wrap);
			sampler_->AddressingMode(Sampler::TAT_Addr_V, Sampler::TAM_Wrap);
			*(technique_->Effect().ParameterByName("fractal_sampler")) = sampler_;

			float4 const & offset = rf.RenderEngineInstance().TexelToPixelOffset();

			float3 xyzs[] =
			{
				float3(-1 + offset.x() / WIDTH, +1 + offset.y() / HEIGHT, 0.5f),
				float3(+1 + offset.x() / WIDTH, +1 + offset.y() / HEIGHT, 0.5f),
				float3(+1 + offset.x() / WIDTH, -1 + offset.y() / HEIGHT, 0.5f),
				float3(-1 + offset.x() / WIDTH, -1 + offset.y() / HEIGHT, 0.5f),
			};

			float2 texs[] =
			{
				float2(0, 0),
				float2(1, 0),
				float2(1, 1),
				float2(0, 1),
			};

			rl_ = rf.MakeRenderLayout(RenderLayout::BT_TriangleFan);

			GraphicsBufferPtr pos_vb = rf.MakeVertexBuffer(BU_Static);
			pos_vb->Resize(sizeof(xyzs));
			{
				GraphicsBuffer::Mapper mapper(*pos_vb, BA_Write_Only);
				std::copy(&xyzs[0], &xyzs[0] + sizeof(xyzs) / sizeof(xyzs[0]), mapper.Pointer<float3>());
			}
			GraphicsBufferPtr tex0_vb = rf.MakeVertexBuffer(BU_Static);
			tex0_vb->Resize(sizeof(texs));
			{
				GraphicsBuffer::Mapper mapper(*tex0_vb, BA_Write_Only);
				std::copy(&texs[0], &texs[0] + sizeof(texs) / sizeof(texs[0]), mapper.Pointer<float2>());
			}

			rl_->BindVertexStream(pos_vb, boost::make_tuple(vertex_element(VEU_Position, 0, EF_BGR32F)));
			rl_->BindVertexStream(tex0_vb, boost::make_tuple(vertex_element(VEU_TextureCoord, 0, EF_GR32F)));

			box_ = MathLib::compute_bounding_box<float>(&xyzs[0], &xyzs[0] + sizeof(xyzs) / sizeof(xyzs[0]));
		}

		void SetTexture(TexturePtr texture)
		{
			sampler_->SetTexture(texture);
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

			technique_ = rf.LoadEffect("Fractal.fx")->TechniqueByName("Show");

			sampler_.reset(new Sampler);
			sampler_->Filtering(Sampler::TFO_Point);
			sampler_->AddressingMode(Sampler::TAT_Addr_U, Sampler::TAM_Wrap);
			sampler_->AddressingMode(Sampler::TAT_Addr_V, Sampler::TAM_Wrap);
			*(technique_->Effect().ParameterByName("fractal_sampler")) = sampler_;

			float3 xyzs[] =
			{
				float3(-1, +1, 0.5f),
				float3(+1, +1, 0.5f),
				float3(+1, -1, 0.5f),
				float3(-1, -1, 0.5f),
			};

			float2 texs[] =
			{
				float2(0, 0),
				float2(1, 0),
				float2(1, 1),
				float2(0, 1),
			};

			uint16_t indices[] = 
			{
				0, 1, 2, 2, 3, 0,
			};

			rl_ = rf.MakeRenderLayout(RenderLayout::BT_TriangleList);

			GraphicsBufferPtr pos_vb = rf.MakeVertexBuffer(BU_Static);
			pos_vb->Resize(sizeof(xyzs));
			{
				GraphicsBuffer::Mapper mapper(*pos_vb, BA_Write_Only);
				std::copy(&xyzs[0], &xyzs[0] + sizeof(xyzs) / sizeof(xyzs[0]), mapper.Pointer<float3>());
			}
			GraphicsBufferPtr tex0_vb = rf.MakeVertexBuffer(BU_Static);
			tex0_vb->Resize(sizeof(texs));
			{
				GraphicsBuffer::Mapper mapper(*tex0_vb, BA_Write_Only);
				std::copy(&texs[0], &texs[0] + sizeof(texs) / sizeof(texs[0]), mapper.Pointer<float2>());
			}

			rl_->BindVertexStream(pos_vb, boost::make_tuple(vertex_element(VEU_Position, 0, EF_BGR32F)));
			rl_->BindVertexStream(tex0_vb, boost::make_tuple(vertex_element(VEU_TextureCoord, 0, EF_GR32F)));

			GraphicsBufferPtr ib = rf.MakeIndexBuffer(BU_Static);
			ib->Resize(sizeof(indices));
			{
				GraphicsBuffer::Mapper mapper(*ib, BA_Write_Only);
				std::copy(indices, indices + sizeof(indices) / sizeof(uint16_t), mapper.Pointer<uint16_t>());
			}
			rl_->BindIndexStream(ib, EF_R16);

			box_ = MathLib::compute_bounding_box<float>(&xyzs[0], &xyzs[0] + sizeof(xyzs) / sizeof(xyzs[0]));
		}

		void SetTexture(TexturePtr texture)
		{
			sampler_->SetTexture(texture);
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
	settings.color_fmt = EF_ARGB8;
	settings.full_screen = false;
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

	render_buffer_[0] = Context::Instance().RenderFactoryInstance().MakeFrameBuffer();
	render_buffer_[1] = Context::Instance().RenderFactoryInstance().MakeFrameBuffer();
	render_buffer_[0]->GetViewport().camera
		= render_buffer_[1]->GetViewport().camera = renderEngine.CurRenderTarget()->GetViewport().camera;

	renderFractal_.reset(new RenderFractal);
	renderPlane_.reset(new RenderPlane);
}

void Fractal::OnResize(uint32_t width, uint32_t height)
{
	RenderFactory& rf = Context::Instance().RenderFactoryInstance();

	rendered_tex_[0] = rf.MakeTexture2D(width, height, 1, EF_GR16F);
	rendered_tex_[1] = rf.MakeTexture2D(width, height, 1, EF_GR16F);

	std::vector<float4> data(width * height);
	std::fill(data.begin(), data.end(), float4(0, 0, 0, 0));
	rendered_tex_[0]->CopyMemoryToTexture2D(0, &data[0], EF_ABGR32F, width, height, 0, 0, width, height);
	rendered_tex_[1]->CopyMemoryToTexture2D(0, &data[0], EF_ABGR32F, width, height, 0, 0, width, height);

	render_buffer_[0]->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*rendered_tex_[0], 0));
	render_buffer_[1]->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*rendered_tex_[1], 0));
}

uint32_t Fractal::NumPasses() const
{
	return 2;
}

void Fractal::DoUpdate(uint32_t pass)
{
	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

	static bool odd = false;

	switch (pass)
	{
	case 0:
		renderEngine.BindRenderTarget(render_buffer_[!odd]);
		renderEngine.Clear(RenderEngine::CBM_Color, Color(0.2f, 0.4f, 0.6f, 1), 1, 0);

		checked_pointer_cast<RenderFractal>(renderFractal_)->SetTexture(rendered_tex_[odd]);
		renderFractal_->AddToRenderQueue();
		break;

	case 1:
		renderEngine.BindRenderTarget(RenderTargetPtr());
		renderEngine.Clear(RenderEngine::CBM_Color | RenderEngine::CBM_Depth, Color(0.2f, 0.4f, 0.6f, 1), 1, 0);

		checked_pointer_cast<RenderPlane>(renderPlane_)->SetTexture(rendered_tex_[!odd]);
		renderPlane_->AddToRenderQueue();

		odd = !odd;

		std::wostringstream stream;
		stream << this->FPS();

		font_->RenderText(0, 0, Color(1, 1, 0, 1), L"GPU¼ÆËã·ÖÐÎ");
		font_->RenderText(0, 18, Color(1, 1, 0, 1), stream.str());
		break;
	}
}
