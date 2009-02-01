#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/GraphicsBuffer.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/Renderable.hpp>
#include <KlayGE/RenderableHelper.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/RenderLayout.hpp>
#include <KlayGE/FrameBuffer.hpp>
#include <KlayGE/SceneManager.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/Texture.hpp>
#include <KlayGE/RenderSettings.hpp>
#include <KlayGE/PerlinNoise.hpp>

#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/InputFactory.hpp>

#include <vector>
#include <sstream>
#include <ctime>
#include <boost/bind.hpp>

#include "Electro.hpp"

using namespace std;
using namespace KlayGE;

namespace
{
	class RenderElectro : public RenderableHelper
	{
	public:
		RenderElectro()
			: RenderableHelper(L"Electro")
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();

			MathLib::PerlinNoise<float>& pn = MathLib::PerlinNoise<float>::Instance();

			int const XSIZE = 128;
			int const YSIZE = 32;
			int const ZSIZE = 32;
			float const XSCALE = 0.04f;
			float const YSCALE = 0.08f;
			float const ZSCALE = 0.08f;

			std::vector<uint8_t> data_v;
			data_v.reserve(XSIZE * YSIZE * ZSIZE);
			uint16_t min = 255, max = 0;
			for (int z = 0; z < ZSIZE; ++ z)
			{
				for (int y = 0; y < YSIZE; ++ y)
				{
					for (int x = 0; x < XSIZE; ++ x)
					{
						uint8_t t = static_cast<uint8_t>(127 * (1
							+ pn.tileable_turbulence(XSCALE * x, YSCALE * y, ZSCALE * z,
								XSIZE * XSCALE, YSIZE * YSCALE, ZSIZE * ZSCALE, 16)));
						if (t > max)
						{
							max = t;
						}
						if (t < min)
						{
							min = t;
						}

						data_v.push_back(t);
					}
				}
			}
			ElementInitData init_data;
			init_data.data = &data_v[0];
			init_data.row_pitch = XSIZE;
			init_data.slice_pitch = XSIZE * YSIZE;

			for (uint32_t i = 0; i < XSIZE * YSIZE * ZSIZE; ++ i)
			{
				data_v[i] = static_cast<uint8_t>((255 * (data_v[i] - min)) / (max - min));
			}

			TexturePtr electro_tex = rf.MakeTexture3D(XSIZE, YSIZE, ZSIZE, 1, EF_L8, 1, 0, EAH_GPU_Read, &init_data);

			technique_ = rf.LoadEffect("Electro.kfx")->TechniqueByName("Electro");
			*(technique_->Effect().ParameterByName("electroSampler")) = electro_tex;

			float3 xyzs[] =
			{
				float3(-0.8f, 0.8f, 0.5f),
				float3(0.8f, 0.8f, 0.5f),
				float3(-0.8f, -0.8f, 0.5f),
				float3(0.8f, -0.8f, 0.5f),
			};

			float3 texs[] =
			{
				float3(-1, 0, 0),
				float3(1, 0, 0),
				float3(-1, -1, 0),
				float3(1, -1, 0),
			};

			rl_ = rf.MakeRenderLayout();
			rl_->TopologyType(RenderLayout::TT_TriangleStrip);

			init_data.row_pitch = sizeof(xyzs);
			init_data.slice_pitch = 0;
			init_data.data = xyzs;
			GraphicsBufferPtr pos_vb = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read, &init_data);
			
			init_data.row_pitch = sizeof(texs);
			init_data.slice_pitch = 0;
			init_data.data = texs;
			GraphicsBufferPtr tex0_vb = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read, &init_data);
			tex0_vb->Resize(sizeof(texs));

			rl_->BindVertexStream(pos_vb, boost::make_tuple(vertex_element(VEU_Position, 0, EF_BGR32F)));
			rl_->BindVertexStream(tex0_vb, boost::make_tuple(vertex_element(VEU_TextureCoord, 0, EF_BGR32F)));

			box_ = MathLib::compute_bounding_box<float>(&xyzs[0], &xyzs[0] + sizeof(xyzs) / sizeof(xyzs[0]));
		}

		void OnRenderBegin()
		{
			float const t = std::clock() * 0.0002f;

			*(technique_->Effect().ParameterByName("y")) = t * 2;
			*(technique_->Effect().ParameterByName("z")) = t;
		}
	};


	enum
	{
		Exit,
		FullScreen,
	};

	InputActionDefine actions[] =
	{
		InputActionDefine(Exit, KS_Escape),
		InputActionDefine(FullScreen, KS_Enter),
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
	ResLoader::Instance().AddPath("../Samples/media/Common");
	ResLoader::Instance().AddPath("../Samples/media/Electro");

	RenderSettings settings = Context::Instance().LoadCfg("KlayGE.cfg");
	settings.ConfirmDevice = ConfirmDevice;

	Electro app("Electro", settings);
	app.Create();
	app.Run();

	return 0;
}

Electro::Electro(std::string const & name, RenderSettings const & settings)
			: App3DFramework(name, settings)
{
}

void Electro::InitObjects()
{
	font_ = Context::Instance().RenderFactoryInstance().MakeFont("gkai00mp.kfont", 16);

	renderElectro_.reset(new RenderElectro);

	InputEngine& inputEngine(Context::Instance().InputFactoryInstance().InputEngineInstance());
	InputActionMap actionMap;
	actionMap.AddActions(actions, actions + sizeof(actions) / sizeof(actions[0]));

	action_handler_t input_handler(new input_signal);
	input_handler->connect(boost::bind(&Electro::InputHandler, this, _1, _2));
	inputEngine.ActionMap(actionMap, input_handler, true);
}

void Electro::InputHandler(InputEngine const & /*sender*/, InputAction const & action)
{
	switch (action.first)
	{
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

uint32_t Electro::DoUpdate(uint32_t /*pass*/)
{
	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

	renderEngine.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, Color(0.2f, 0.4f, 0.6f, 1), 1.0f, 0);

	renderElectro_->AddToRenderQueue();

	std::wostringstream stream;
	stream << this->FPS();

	font_->RenderText(0, 0, Color(1, 1, 0, 1), L"Electro Effect");
	font_->RenderText(0, 18, Color(1, 1, 0, 1), stream.str());

	return App3DFramework::URV_Need_Flush | App3DFramework::URV_Finished;
}
