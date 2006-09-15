#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/Texture.hpp>
#include <KlayGE/Renderable.hpp>
#include <KlayGE/RenderableHelper.hpp>
#include <KlayGE/RenderWindow.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/FrameBuffer.hpp>
#include <KlayGE/SceneManager.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/RenderSettings.hpp>
#include <KlayGE/Sampler.hpp>
#include <KlayGE/KMesh.hpp>
#include <KlayGE/SceneObjectHelper.hpp>
#include <KlayGE/PostProcess.hpp>
#include <KlayGE/Util.hpp>

#include <KlayGE/D3D9/D3D9RenderFactory.hpp>

#include <KlayGE/Input.hpp>
#include <KlayGE/DInput/DInputFactory.hpp>

#include <KlayGE/OCTree/OCTree.hpp>

#include <numeric>
#include <sstream>
#include <boost/assert.hpp>
#include <boost/bind.hpp>

#include "ascii_lums_builder.hpp"
#include "AsciiArts.hpp"

using namespace KlayGE;
using namespace std;

int const WIDTH = 800;
int const HEIGHT = 600;
int const CELL_WIDTH = 8;
int const CELL_HEIGHT = 8;
int const INPUT_NUM_ASCII = 128;
int const ASCII_WIDTH = 16;
int const ASCII_HEIGHT = 16;

int const OUTPUT_NUM_ASCII = 32;

namespace
{
	class Downsampler8x8 : public PostProcess
	{
	public:
		Downsampler8x8()
			: PostProcess(Context::Instance().RenderFactoryInstance().LoadEffect("AsciiArts.fx")->TechniqueByName("Downsample8x8"))
		{
		}

		void Source(TexturePtr const & src_tex, Sampler::TexFilterOp filter, Sampler::TexAddressingMode am)
		{
			PostProcess::Source(src_tex, filter, am);

			this->GetSampleOffsets8x8(src_tex->Width(0), src_tex->Height(0));
		}

		void OnRenderBegin()
		{
			PostProcess::OnRenderBegin();

			*(technique_->Effect().ParameterByName("tex_coord_offset")) = tex_coord_offset_;
		}

	private:
		void GetSampleOffsets8x8(uint32_t width, uint32_t height)
		{
			tex_coord_offset_.resize(8);

			float const tu = 1.0f / width;
			float const tv = 1.0f / height;

			// Sample from the 64 surrounding points. 
			int index = 0;
			for (int y = -3; y <= 4; y += 2)
			{
				for (int x = -3; x <= 4; x += 4)
				{
					tex_coord_offset_[index].x() = (x + 0) * tu;
					tex_coord_offset_[index].y() = y * tv;
					tex_coord_offset_[index].z() = (x + 2) * tu;
					tex_coord_offset_[index].w() = y * tv;

					++ index;
				}
			}
		}

	private:
		std::vector<float4> tex_coord_offset_;
	};

	class AsciiArts : public PostProcess
	{
	public:
		AsciiArts()
			: PostProcess(Context::Instance().RenderFactoryInstance().LoadEffect("AsciiArts.fx")->TechniqueByName("AsciiArts")),
				lums_sampler_(new Sampler)
		{
			src_sampler_->Filtering(Sampler::TFO_Point);
			src_sampler_->AddressingMode(Sampler::TAT_Addr_U, Sampler::TAM_Clamp);
			src_sampler_->AddressingMode(Sampler::TAT_Addr_V, Sampler::TAM_Clamp);
			*(technique_->Effect().ParameterByName("src_sampler")) = src_sampler_;
			lums_sampler_->Filtering(Sampler::TFO_Bilinear);
			lums_sampler_->AddressingMode(Sampler::TAT_Addr_U, Sampler::TAM_Clamp);
			lums_sampler_->AddressingMode(Sampler::TAT_Addr_V, Sampler::TAM_Clamp);
			*(technique_->Effect().ParameterByName("lums_sampler")) = lums_sampler_;
		}

		void SetLumsTex(TexturePtr const & lums_tex)
		{
			lums_sampler_->SetTexture(lums_tex);
		}

		void OnRenderBegin()
		{
			PostProcess::OnRenderBegin();

			RenderEngine const & renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			RenderTarget const & renderTarget(*renderEngine.CurRenderTarget());

			*(technique_->Effect().ParameterByName("cell_per_row_line")) =
				float2(static_cast<float>(CELL_WIDTH) / renderTarget.Width(),
						static_cast<float>(CELL_HEIGHT) / renderTarget.Height());
		}

	private:
		SamplerPtr lums_sampler_;
	};

	std::vector<ascii_tile_type> LoadFromTexture(std::string const & tex_name)
	{
		int const ASCII_IN_A_ROW = 16;

		TexturePtr ascii_tex = LoadTexture(tex_name);
		BOOST_ASSERT(EF_L8 == ascii_tex->Format());

		std::vector<ascii_tile_type> ret(INPUT_NUM_ASCII);

		std::vector<uint8_t> ascii_tex_data(INPUT_NUM_ASCII * ASCII_WIDTH * ASCII_HEIGHT);
		ascii_tex->CopyToMemory2D(0, &ascii_tex_data[0]);

		for (size_t i = 0; i < ret.size(); ++ i)
		{
			ret[i].resize(ASCII_WIDTH * ASCII_HEIGHT);
			for (int y = 0; y < ASCII_HEIGHT; ++ y)
			{
				for (int x = 0; x < ASCII_WIDTH; ++ x)
				{
					ret[i][y * ASCII_WIDTH + x]
						= ascii_tex_data[((i / ASCII_IN_A_ROW) * ASCII_HEIGHT + y) * ASCII_IN_A_ROW * ASCII_WIDTH
							+ (i % ASCII_IN_A_ROW) * ASCII_WIDTH + x];
				}
			}
		}

		return ret;
	}

	TexturePtr FillTexture(ascii_tiles_type const & ascii_lums)
	{
		BOOST_ASSERT(OUTPUT_NUM_ASCII == ascii_lums.size());

		std::vector<uint8_t> temp_data(OUTPUT_NUM_ASCII * ASCII_WIDTH * ASCII_HEIGHT);

		for (size_t i = 0; i < OUTPUT_NUM_ASCII; ++ i)
		{
			for (size_t y = 0; y < ASCII_HEIGHT; ++ y)
			{
				for (size_t x = 0; x < ASCII_WIDTH; ++ x)
				{
					temp_data[y * OUTPUT_NUM_ASCII * ASCII_WIDTH + i * ASCII_WIDTH + x]
						= ascii_lums[i][y * ASCII_WIDTH + x];
				}
			}
		}

		TexturePtr ret = Context::Instance().RenderFactoryInstance().MakeTexture2D(OUTPUT_NUM_ASCII * ASCII_WIDTH,
			ASCII_HEIGHT, 1, EF_L8);
		ret->CopyMemoryToTexture2D(0, &temp_data[0], EF_L8, OUTPUT_NUM_ASCII * ASCII_WIDTH, ASCII_HEIGHT, 0, 0,
			OUTPUT_NUM_ASCII * ASCII_WIDTH, ASCII_HEIGHT);
		return ret;
	}

	enum
	{
		Switch,
		Exit,
	};

	InputActionDefine actions[] = 
	{
		InputActionDefine(Switch, KS_Space),
		InputActionDefine(Exit, KS_Escape),
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
	OCTree sceneMgr(Box(float3(-1, -1, -1), float3(1, 1, 1)), 3);

	Context::Instance().RenderFactoryInstance(D3D9RenderFactoryInstance());
	Context::Instance().SceneManagerInstance(sceneMgr);

	Context::Instance().InputFactoryInstance(DInputFactoryInstance());

	RenderSettings settings;
	settings.width = WIDTH;
	settings.height = HEIGHT;
	settings.color_fmt = EF_ARGB8;
	settings.full_screen = false;
	settings.ConfirmDevice = ConfirmDevice;

	AsciiArtsApp app;
	app.Create("ASCII Arts", settings);
	app.Run();

	return 0;
}

AsciiArtsApp::AsciiArtsApp()
			: show_ascii_(true)
{
	ResLoader::Instance().AddPath("../media/Common");
	ResLoader::Instance().AddPath("../media/AsciiArts");
}

void AsciiArtsApp::BuildAsciiLumsTex()
{
	ascii_lums_builder builder(INPUT_NUM_ASCII, OUTPUT_NUM_ASCII, ASCII_WIDTH, ASCII_HEIGHT);
	ascii_lums_tex_ = FillTexture(builder.build(LoadFromTexture("font.dds")));
}

void AsciiArtsApp::InitObjects()
{
	font_ = Context::Instance().RenderFactoryInstance().MakeFont("gkai00mp.ttf", 16);

	this->LookAt(float3(0.0f, 0.3f, -0.2f), float3(0.0f, 0.1f, 0.0f));
	this->Proj(0.1f, 100.0f);

	fpcController_.AttachCamera(this->ActiveCamera());
	fpcController_.Scalers(0.05f, 0.1f);

	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

	renderEngine.Clear(RenderEngine::CBM_Color | RenderEngine::CBM_Depth, Color(0.2f, 0.4f, 0.6f, 1), 1, 0);

	obj_.reset(new SceneObjectHelper(LoadKModel("teapot.kmodel", CreateKModelFactory<RenderModel>(), CreateKMeshFactory<KMesh>()),
		SceneObject::SOA_Cullable | SceneObject::SOA_ShortAge));

	this->BuildAsciiLumsTex();

	render_buffer_ = Context::Instance().RenderFactoryInstance().MakeFrameBuffer();
	render_buffer_->GetViewport().camera = renderEngine.CurRenderTarget()->GetViewport().camera;

	InputEngine& inputEngine(Context::Instance().InputFactoryInstance().InputEngineInstance());
	InputActionMap actionMap;
	actionMap.AddActions(actions, actions + sizeof(actions) / sizeof(actions[0]));

	action_handler_t input_handler(inputEngine);
	input_handler += boost::bind(&AsciiArtsApp::InputHandler, this, _1, _2);
	inputEngine.ActionMap(actionMap, input_handler, true);

	downsampler_.reset(new Downsampler8x8);

	ascii_arts_.reset(new AsciiArts);
	ascii_arts_->Destinate(RenderTargetPtr());
	checked_pointer_cast<AsciiArts>(ascii_arts_)->SetLumsTex(ascii_lums_tex_);
}

void AsciiArtsApp::OnResize(uint32_t width, uint32_t height)
{
	RenderFactory& rf = Context::Instance().RenderFactoryInstance();

	rendered_tex_ = rf.MakeTexture2D(width, height, 1, EF_ARGB8);
	render_buffer_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*rendered_tex_, 0));
	render_buffer_->Attach(FrameBuffer::ATT_DepthStencil, rf.MakeDepthStencilRenderView(width, height, EF_D16, 0));

	downsample_tex_ = rf.MakeTexture2D(width / CELL_WIDTH, height / CELL_HEIGHT,
		1, EF_ARGB8);

	FrameBufferPtr fb = rf.MakeFrameBuffer();
	fb->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*downsample_tex_, 0));
	downsampler_->Source(rendered_tex_, Sampler::TFO_Bilinear, Sampler::TAM_Clamp);
	downsampler_->Destinate(fb);

	ascii_arts_->Source(downsample_tex_, Sampler::TFO_Point, Sampler::TAM_Clamp);
}

uint32_t AsciiArtsApp::NumPasses() const
{
	if (show_ascii_)
	{
		return 2;
	}
	else
	{
		return 1;
	}
}

void AsciiArtsApp::InputHandler(InputEngine const & /*sender*/, InputAction const & action)
{
	switch (action.first)
	{
	case Switch:
		show_ascii_ = !show_ascii_;
		KlayGE::Sleep(150);
		break;

	case Exit:
		this->Quit();
		break;
	}
}

void AsciiArtsApp::DoUpdate(uint32_t pass)
{
	if (0 == pass)
	{
		fpcController_.Update();
	}

	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
	SceneManager& sceneMgr(Context::Instance().SceneManagerInstance());

	if (show_ascii_)
	{
		switch (pass)
		{
		case 0:
			// 正常渲染
			renderEngine.BindRenderTarget(render_buffer_);
			renderEngine.Clear(RenderEngine::CBM_Color | RenderEngine::CBM_Depth, Color(0.2f, 0.4f, 0.6f, 1), 1, 0);

			obj_->AddToSceneManager();
			break;

		case 1:
			// 降采样
			downsampler_->Apply();

			// 匹配，最终渲染
			ascii_arts_->Apply();
			break;
		}
	}
	else
	{
		renderEngine.BindRenderTarget(RenderTargetPtr());
		renderEngine.Clear(RenderEngine::CBM_Color | RenderEngine::CBM_Depth, Color(0.2f, 0.4f, 0.6f, 1), 1, 0);

		sceneMgr.Clear();
		obj_->AddToSceneManager();
	}

	if ((!show_ascii_ && (0 == pass))
		|| (show_ascii_ && (1 == pass)))
	{
		renderEngine.BindRenderTarget(RenderTargetPtr());

		std::wostringstream stream;
		stream << this->FPS();

		font_->RenderText(0, 0, Color(1, 1, 0, 1), L"ASCII艺术");
		font_->RenderText(0, 18, Color(1, 1, 0, 1), stream.str().c_str());

		stream.str(L"");
		stream << sceneMgr.NumRenderablesRendered() << " Renderables "
			<< sceneMgr.NumPrimitivesRendered() << " Primitives "
			<< sceneMgr.NumVerticesRendered() << " Vertices";
		font_->RenderText(0, 36, Color(1, 1, 1, 1), stream.str().c_str());
	}
}
