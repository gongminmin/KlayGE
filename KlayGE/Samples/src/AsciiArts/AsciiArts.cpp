#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/Texture.hpp>
#include <KlayGE/Renderable.hpp>
#include <KlayGE/RenderableHelper.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/FrameBuffer.hpp>
#include <KlayGE/SceneManager.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/RenderSettings.hpp>
#include <KlayGE/KMesh.hpp>
#include <KlayGE/SceneObjectHelper.hpp>
#include <KlayGE/PostProcess.hpp>
#include <KlayGE/Util.hpp>

#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/InputFactory.hpp>

#include <numeric>
#include <sstream>
#include <boost/assert.hpp>
#include <boost/bind.hpp>

#include "ascii_lums_builder.hpp"
#include "AsciiArts.hpp"

using namespace KlayGE;
using namespace std;

int const CELL_WIDTH = 8;
int const CELL_HEIGHT = 8;
int const INPUT_NUM_ASCII = 128;
size_t const ASCII_WIDTH = 16;
size_t const ASCII_HEIGHT = 16;

size_t const OUTPUT_NUM_ASCII = 32;

namespace
{
	class Downsampler8x8 : public PostProcess
	{
	public:
		Downsampler8x8()
			: PostProcess(Context::Instance().RenderFactoryInstance().LoadEffect("Downsample8x8.kfx")->TechniqueByName("Downsample8x8"))
		{
			tex_coord_offset_ep_ = technique_->Effect().ParameterByName("tex_coord_offset");
		}

		void Source(TexturePtr const & src_tex, bool flipping)
		{
			PostProcess::Source(src_tex, flipping);

			this->GetSampleOffsets8x8(src_tex->Width(0), src_tex->Height(0));
		}

	private:
		void GetSampleOffsets8x8(uint32_t width, uint32_t height)
		{
			std::vector<float4> tex_coord_offset(8);

			float const tu = 1.0f / width;
			float const tv = 1.0f / height;

			// Sample from the 64 surrounding points.
			int index = 0;
			for (int y = -4; y <= 3; y += 2)
			{
				for (int x = -4; x <= 3; x += 4)
				{
					tex_coord_offset[index].x() = (x + 0.5f) * tu;
					tex_coord_offset[index].y() = (y + 0.5f) * tv;
					tex_coord_offset[index].z() = (x + 2.5f) * tu;
					tex_coord_offset[index].w() = (y + 0.5f) * tv;

					++ index;
				}
			}

			*tex_coord_offset_ep_ = tex_coord_offset;
		}

	private:
		RenderEffectParameterPtr tex_coord_offset_ep_;
	};

	class AsciiArts : public PostProcess
	{
	public:
		AsciiArts()
			: PostProcess(Context::Instance().RenderFactoryInstance().LoadEffect("AsciiArts.kfx")->TechniqueByName("AsciiArts"))
		{
			cell_per_row_line_ep_ = technique_->Effect().ParameterByName("cell_per_row_line");
			lums_sampler_ep_ = technique_->Effect().ParameterByName("lums_sampler");
		}

		void SetLumsTex(TexturePtr const & lums_tex)
		{
			*lums_sampler_ep_ = lums_tex;
		}

		void OnRenderBegin()
		{
			PostProcess::OnRenderBegin();

			RenderEngine const & re(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			FrameBuffer const & fb(*re.CurFrameBuffer());

			*cell_per_row_line_ep_ = float2(static_cast<float>(CELL_WIDTH) / fb.Width(), static_cast<float>(CELL_HEIGHT) / fb.Height());
		}

	private:
		RenderEffectParameterPtr cell_per_row_line_ep_;
		RenderEffectParameterPtr lums_sampler_ep_;
	};

	std::vector<ascii_tile_type> LoadFromTexture(std::string const & tex_name)
	{
		int const ASCII_IN_A_ROW = 16;

		TexturePtr ascii_tex = LoadTexture(tex_name, EAH_CPU_Read);
		BOOST_ASSERT(EF_L8 == ascii_tex->Format());

		std::vector<ascii_tile_type> ret(INPUT_NUM_ASCII);

		std::vector<uint8_t> ascii_tex_data(INPUT_NUM_ASCII * ASCII_WIDTH * ASCII_HEIGHT);
		{
			Texture::Mapper mapper(*ascii_tex, 0, TMA_Read_Only, 0, 0, ASCII_IN_A_ROW * ASCII_WIDTH, INPUT_NUM_ASCII / ASCII_IN_A_ROW * ASCII_HEIGHT);
			uint8_t* data = mapper.Pointer<uint8_t>();
			for (uint32_t y = 0; y < INPUT_NUM_ASCII / ASCII_IN_A_ROW * ASCII_HEIGHT; ++ y)
			{
				memcpy(&ascii_tex_data[y * ASCII_IN_A_ROW * ASCII_WIDTH], data, ASCII_IN_A_ROW * ASCII_WIDTH);
				data += mapper.RowPitch();
			}
		}

		for (size_t i = 0; i < ret.size(); ++ i)
		{
			ret[i].resize(ASCII_WIDTH * ASCII_HEIGHT);
			for (size_t y = 0; y < ASCII_HEIGHT; ++ y)
			{
				for (size_t x = 0; x < ASCII_WIDTH; ++ x)
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

		ElementInitData init_data;
		init_data.data.resize(OUTPUT_NUM_ASCII * ASCII_WIDTH * ASCII_HEIGHT);
		init_data.row_pitch = OUTPUT_NUM_ASCII * ASCII_WIDTH;
		init_data.slice_pitch = 0;

		for (size_t i = 0; i < OUTPUT_NUM_ASCII; ++ i)
		{
			for (size_t y = 0; y < ASCII_HEIGHT; ++ y)
			{
				for (size_t x = 0; x < ASCII_WIDTH; ++ x)
				{
					init_data.data[y * OUTPUT_NUM_ASCII * ASCII_WIDTH + i * ASCII_WIDTH + x]
						= ascii_lums[i][y * ASCII_WIDTH + x];
				}
			}
		}

		return Context::Instance().RenderFactoryInstance().MakeTexture2D(OUTPUT_NUM_ASCII * ASCII_WIDTH,
			ASCII_HEIGHT, 1, EF_L8, EAH_GPU_Read, &init_data);
	}

	enum
	{
		Switch_AscII
	};

	enum
	{
		Switch,
		Exit,
		FullScreen,
	};

	InputActionDefine actions[] =
	{
		InputActionDefine(Switch, KS_Space),
		InputActionDefine(Exit, KS_Escape),
		InputActionDefine(FullScreen, KS_Enter)
	};

	bool ConfirmDevice()
	{
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		RenderDeviceCaps const & caps = re.DeviceCaps();
		if (caps.max_shader_model < 2)
		{
			return false;
		}
		return true;
	}
}

int main()
{
	ResLoader::Instance().AddPath("../../media/Common");
	ResLoader::Instance().AddPath("../../media/AsciiArts");

	RenderSettings settings;
	SceneManagerPtr sm = Context::Instance().LoadCfg(settings, "KlayGE.cfg");
	settings.ConfirmDevice = ConfirmDevice;

	AsciiArtsApp app("ASCII Arts", settings);
	app.Create();
	app.Run();

	return 0;
}

AsciiArtsApp::AsciiArtsApp(std::string const & name, RenderSettings const & settings)
			: App3DFramework(name, settings),
				show_ascii_(true)
{
}

void AsciiArtsApp::BuildAsciiLumsTex()
{
	ascii_lums_builder builder(INPUT_NUM_ASCII, OUTPUT_NUM_ASCII, ASCII_WIDTH, ASCII_HEIGHT);
	ascii_lums_tex_ = FillTexture(builder.build(LoadFromTexture("font.dds")));
}

void AsciiArtsApp::InitObjects()
{
	font_ = Context::Instance().RenderFactoryInstance().MakeFont("gkai00mp.kfont", 16);

	this->LookAt(float3(0.0f, 0.3f, -0.2f), float3(0.0f, 0.1f, 0.0f));
	this->Proj(0.1f, 100.0f);

	fpcController_.AttachCamera(this->ActiveCamera());
	fpcController_.Scalers(0.05f, 0.1f);

	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

	obj_.reset(new SceneObjectHelper(LoadKModel("teapot.kmodel", EAH_GPU_Read, CreateKModelFactory<RenderModel>(), CreateKMeshFactory<KMesh>()),
		SceneObject::SOA_Cullable));
	obj_->AddToSceneManager();

	this->BuildAsciiLumsTex();

	render_buffer_ = Context::Instance().RenderFactoryInstance().MakeFrameBuffer();
	render_buffer_->GetViewport().camera = renderEngine.CurFrameBuffer()->GetViewport().camera;

	InputEngine& inputEngine(Context::Instance().InputFactoryInstance().InputEngineInstance());
	InputActionMap actionMap;
	actionMap.AddActions(actions, actions + sizeof(actions) / sizeof(actions[0]));

	action_handler_t input_handler(new input_signal);
	input_handler->connect(boost::bind(&AsciiArtsApp::InputHandler, this, _1, _2));
	inputEngine.ActionMap(actionMap, input_handler, true);

	downsampler_.reset(new Downsampler8x8);

	ascii_arts_.reset(new AsciiArts);
	checked_pointer_cast<AsciiArts>(ascii_arts_)->SetLumsTex(ascii_lums_tex_);

	dialog_ = UIManager::Instance().MakeDialog();
	dialog_->AddControl(UIControlPtr(new UICheckBox(dialog_, Switch_AscII, L"AscII filter",
                            60, 550, 350, 24, false, 0, false)));
	dialog_->Control<UICheckBox>(Switch_AscII)->SetChecked(true);
	dialog_->Control<UICheckBox>(Switch_AscII)->OnChangedEvent().connect(boost::bind(&AsciiArtsApp::CheckBoxHandler, this, _1));
}

void AsciiArtsApp::OnResize(uint32_t width, uint32_t height)
{
	App3DFramework::OnResize(width, height);

	RenderFactory& rf = Context::Instance().RenderFactoryInstance();

	rendered_tex_ = rf.MakeTexture2D(width, height, 1, EF_ARGB8, EAH_GPU_Read | EAH_GPU_Write, NULL);
	render_buffer_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*rendered_tex_, 0));
	render_buffer_->Attach(FrameBuffer::ATT_DepthStencil, rf.MakeDepthStencilRenderView(width, height, EF_D16, 0));

	downsample_tex_ = rf.MakeTexture2D(width / CELL_WIDTH, height / CELL_HEIGHT,
		1, EF_ARGB8, EAH_GPU_Read | EAH_GPU_Write, NULL);

	FrameBufferPtr fb = rf.MakeFrameBuffer();
	fb->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*downsample_tex_, 0));
	downsampler_->Source(rendered_tex_, render_buffer_->RequiresFlipping());
	downsampler_->Destinate(fb);

	ascii_arts_->Source(downsample_tex_, fb->RequiresFlipping());
	ascii_arts_->Destinate(FrameBufferPtr());

	dialog_->GetControl(Switch_AscII)->SetLocation(60, height - 50);
}

void AsciiArtsApp::InputHandler(InputEngine const & /*sender*/, InputAction const & action)
{
	switch (action.first)
	{
	case Switch:
		show_ascii_ = !show_ascii_;
		dialog_->Control<UICheckBox>(Switch_AscII)->SetChecked(show_ascii_);
		KlayGE::Sleep(150);
		break;

	case FullScreen:
		{
			RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			renderEngine.EndFrame();
			renderEngine.Resize(800, 600);
			renderEngine.FullScreen(!renderEngine.FullScreen());
			renderEngine.BeginFrame();
		}
		break;

	case Exit:
		this->Quit();
		break;
	}
}

void AsciiArtsApp::CheckBoxHandler(UICheckBox const & /*sender*/)
{
	show_ascii_ = dialog_->Control<UICheckBox>(Switch_AscII)->GetChecked();
}

uint32_t AsciiArtsApp::DoUpdate(uint32_t pass)
{
	if (0 == pass)
	{
		UIManager::Instance().HandleInput();
	}

	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
	SceneManager& sceneMgr(Context::Instance().SceneManagerInstance());

	if (show_ascii_)
	{
		switch (pass)
		{
		case 0:
			// Õý³£äÖÈ¾
			renderEngine.BindFrameBuffer(render_buffer_);
			renderEngine.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, Color(0.2f, 0.4f, 0.6f, 1), 1.0f, 0);
			return App3DFramework::URV_Need_Flush;

		case 1:
			//SaveTexture(rendered_tex_, "rendered_tex.dds");

			// ½µ²ÉÑù
			downsampler_->Apply();

			//SaveTexture(downsample_tex_, "downsample_tex.dds");

			// Æ¥Åä£¬×îÖÕäÖÈ¾
			renderEngine.BindFrameBuffer(FrameBufferPtr());
			renderEngine.CurFrameBuffer()->Attached(FrameBuffer::ATT_DepthStencil)->Clear(1.0f);
			ascii_arts_->Apply();
			break;
		}
	}
	else
	{
		renderEngine.BindFrameBuffer(FrameBufferPtr());
		renderEngine.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, Color(0.2f, 0.4f, 0.6f, 1), 1.0f, 0);
	}

	UIManager::Instance().Render();

	std::wostringstream stream;
	stream << this->FPS();

	font_->RenderText(0, 0, Color(1, 1, 0, 1), L"ASCII Arts");
	font_->RenderText(0, 18, Color(1, 1, 0, 1), stream.str());

	stream.str(L"");
	stream << sceneMgr.NumRenderablesRendered() << " Renderables "
		<< sceneMgr.NumPrimitivesRendered() << " Primitives "
		<< sceneMgr.NumVerticesRendered() << " Vertices";
	font_->RenderText(0, 36, Color(1, 1, 1, 1), stream.str());

	if (!show_ascii_ && (0 == pass))
	{
		return App3DFramework::URV_Need_Flush | App3DFramework::URV_Finished;
	}
	else
	{
		return App3DFramework::URV_Only_New_Objs | App3DFramework::URV_Need_Flush | App3DFramework::URV_Finished;
	}
}
