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
#include <KlayGE/FrameBuffer.hpp>
#include <KlayGE/RenderLayout.hpp>
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

			technique_ = rf.LoadEffect("Fractal.kfx")->TechniqueByName("Mandelbrot");

			float4 const & offset = rf.RenderEngineInstance().TexelToPixelOffset() * 2;

			float3 xyzs[] =
			{
				float3(-1 + offset.x() / WIDTH, +1 + offset.y() / HEIGHT, 0.5f),
				float3(+1 + offset.x() / WIDTH, +1 + offset.y() / HEIGHT, 0.5f),
				float3(-1 + offset.x() / WIDTH, -1 + offset.y() / HEIGHT, 0.5f),
				float3(+1 + offset.x() / WIDTH, -1 + offset.y() / HEIGHT, 0.5f),
			};

			float2 texs[] =
			{
				float2(0, 0),
				float2(1, 0),
				float2(0, 1),
				float2(1, 1),
			};

			rl_ = rf.MakeRenderLayout();
			rl_->TopologyType(RenderLayout::TT_TriangleStrip);

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
			*(technique_->Effect().ParameterByName("fractal_sampler")) = texture;
		}
	};

	class RenderPlane : public RenderableHelper
	{
	public:
		RenderPlane()
			: RenderableHelper(L"Plane")
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();

			technique_ = rf.LoadEffect("Fractal.kfx")->TechniqueByName("Show");

			float3 xyzs[] =
			{
				float3(-1, +1, 0.5f),
				float3(+1, +1, 0.5f),
				float3(-1, -1, 0.5f),
				float3(+1, -1, 0.5f),
			};

			float2 texs[] =
			{
				float2(0, 0),
				float2(1, 0),
				float2(0, 1),
				float2(1, 1),
			};

			rl_ = rf.MakeRenderLayout();
			rl_->TopologyType(RenderLayout::TT_TriangleStrip);

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
			*(technique_->Effect().ParameterByName("fractal_sampler")) = texture;
		}
	};

	bool ConfirmDevice()
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		RenderEngine& re = rf.RenderEngineInstance();
		RenderDeviceCaps const & caps = re.DeviceCaps();
		if (caps.max_shader_model < 2)
		{
			return false;
		}

		try
		{
			TexturePtr temp_tex = rf.MakeTexture2D(800, 600, 1, EF_GR16F);
			rf.Make2DRenderView(*temp_tex, 0);
		}
		catch (...)
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

	Fractal app("Fractal", settings);
	app.Create();
	app.Run();

	return 0;
}

Fractal::Fractal(std::string const & name, RenderSettings const & settings)
			: App3DFramework(name, settings)
{
	ResLoader::Instance().AddPath("../../media/Common");
	ResLoader::Instance().AddPath("../../media/Fractal");
}

void Fractal::InitObjects()
{
	font_ = Context::Instance().RenderFactoryInstance().MakeFont("gkai00mp.kfont", 16);

	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

	render_buffer_[0] = Context::Instance().RenderFactoryInstance().MakeFrameBuffer();
	render_buffer_[1] = Context::Instance().RenderFactoryInstance().MakeFrameBuffer();
	render_buffer_[0]->GetViewport().camera
		= render_buffer_[1]->GetViewport().camera = renderEngine.CurFrameBuffer()->GetViewport().camera;

	renderFractal_.reset(new RenderFractal);
	renderPlane_.reset(new RenderPlane);
}

void Fractal::OnResize(uint32_t width, uint32_t height)
{
	App3DFramework::OnResize(width, height);

	RenderFactory& rf = Context::Instance().RenderFactoryInstance();

	for (int i = 0; i < 2; ++ i)
	{
		rendered_tex_[i] = rf.MakeTexture2D(width, height, 1, EF_GR16F);

		Texture::Mapper mapper(*rendered_tex_[i], 0, TMA_Write_Only, 0, 0, width, height);
		uint8_t* data = mapper.Pointer<uint8_t>();
		for (uint32_t y = 0; y < height; ++ y)
		{
			for (uint32_t x = 0; x < width * 4; ++ x)
			{
				data[x] = 0;
			}
			data += mapper.RowPitch();
		}
	}

	render_buffer_[0]->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*rendered_tex_[0], 0));
	render_buffer_[1]->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*rendered_tex_[1], 0));
}

uint32_t Fractal::DoUpdate(uint32_t pass)
{
	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

	static bool odd = false;

	switch (pass)
	{
	case 0:
		renderEngine.BindFrameBuffer(render_buffer_[!odd]);
		renderEngine.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color, Color(0.2f, 0.4f, 0.6f, 1), 1.0f, 0);

		checked_pointer_cast<RenderFractal>(renderFractal_)->SetTexture(rendered_tex_[odd]);
		renderFractal_->AddToRenderQueue();
		return App3DFramework::URV_Need_Flush;

	default:
		renderEngine.BindFrameBuffer(FrameBufferPtr());
		renderEngine.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, Color(0.2f, 0.4f, 0.6f, 1), 1.0f, 0);

		checked_pointer_cast<RenderPlane>(renderPlane_)->SetTexture(rendered_tex_[!odd]);
		renderPlane_->AddToRenderQueue();

		odd = !odd;

		std::wostringstream stream;
		stream << this->FPS();

		font_->RenderText(0, 0, Color(1, 1, 0, 1), L"GPU¼ÆËã·ÖÐÎ");
		font_->RenderText(0, 18, Color(1, 1, 0, 1), stream.str());
		return App3DFramework::URV_Need_Flush | App3DFramework::URV_Finished;
	}
}
