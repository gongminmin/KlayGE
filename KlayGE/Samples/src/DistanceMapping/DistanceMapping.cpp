#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/GraphicsBuffer.hpp>
#include <KlayGE/Renderable.hpp>
#include <KlayGE/RenderableHelper.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/Sampler.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/FrameBuffer.hpp>
#include <KlayGE/SceneManager.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/RenderSettings.hpp>
#include <KlayGE/SceneObjectHelper.hpp>

#include <KlayGE/D3D9/D3D9RenderFactory.hpp>
#include <KlayGE/OpenGL/OGLRenderFactory.hpp>

#include <KlayGE/OCTree/OCTree.hpp>

#include <KlayGE/Input.hpp>
#include <KlayGE/DInput/DInputFactory.hpp>

#include <vector>
#include <sstream>
#include <fstream>
#include <ctime>
#include <boost/bind.hpp>
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable: 4251 4275 4512 4702)
#endif
#include <boost/program_options.hpp>
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(pop)
#endif

#include "DistanceMapping.hpp"

using namespace std;
using namespace KlayGE;

namespace
{
	class RenderPolygon : public RenderableHelper
	{
	public:
		RenderPolygon()
			: RenderableHelper(L"Polygon")
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();

			RenderEffectPtr effect = rf.LoadEffect("DistanceMapping.kfx");

			technique_ = effect->TechniqueByName("DistanceMapping2a");
			if (!technique_->Validate())
			{
				technique_ = effect->TechniqueByName("DistanceMapping20");
			}

			*(technique_->Effect().ParameterByName("diffuseMap")) = LoadTexture("diffuse.dds", EAH_CPU_Write | EAH_GPU_Read);
			*(technique_->Effect().ParameterByName("normalMap")) = LoadTexture("normal.dds", EAH_CPU_Write | EAH_GPU_Read);
			*(technique_->Effect().ParameterByName("distanceMap")) = LoadTexture("distance.dds", EAH_CPU_Write | EAH_GPU_Read);

			float3 xyzs[] =
			{
				float3(-1, 1,  0),
				float3(1,	1,	0),
				float3(1,	-1,	0),
				float3(-1, -1, 0),
			};

			float2 texs[] =
			{
				float2(0, 0),
				float2(1, 0),
				float2(1, 1),
				float2(0, 1)
			};

			uint16_t indices[] =
			{
				0, 1, 2, 2, 3, 0
			};

			float3 normal[4];
			MathLib::compute_normal<float>(normal,
				indices, indices + sizeof(indices) / sizeof(indices[0]),
				xyzs, xyzs + sizeof(xyzs) / sizeof(xyzs[0]));

			float3 t[4], b[4];
			MathLib::compute_tangent<float>(t, b,
				indices, indices + sizeof(indices) / sizeof(indices[0]),
				xyzs, xyzs + sizeof(xyzs) / sizeof(xyzs[0]), texs, normal);

			rl_ = rf.MakeRenderLayout();
			rl_->TopologyType(RenderLayout::TT_TriangleList);

			GraphicsBufferPtr pos_vb = rf.MakeVertexBuffer(BU_Static, EAH_CPU_Write | EAH_GPU_Read);
			pos_vb->Resize(sizeof(xyzs));
			{
				GraphicsBuffer::Mapper mapper(*pos_vb, BA_Write_Only);
				std::copy(&xyzs[0], &xyzs[0] + sizeof(xyzs) / sizeof(xyzs[0]), mapper.Pointer<float3>());
			}
			GraphicsBufferPtr tex0_vb = rf.MakeVertexBuffer(BU_Static, EAH_CPU_Write | EAH_GPU_Read);
			tex0_vb->Resize(sizeof(texs));
			{
				GraphicsBuffer::Mapper mapper(*tex0_vb, BA_Write_Only);
				std::copy(&texs[0], &texs[0] + sizeof(texs) / sizeof(texs[0]), mapper.Pointer<float2>());
			}
			GraphicsBufferPtr tan_vb = rf.MakeVertexBuffer(BU_Static, EAH_CPU_Write | EAH_GPU_Read);
			tan_vb->Resize(sizeof(t));
			{
				GraphicsBuffer::Mapper mapper(*tan_vb, BA_Write_Only);
				std::copy(&t[0], &t[0] + sizeof(t) / sizeof(t[0]), mapper.Pointer<float3>());
			}
			GraphicsBufferPtr binormal_vb = rf.MakeVertexBuffer(BU_Static, EAH_CPU_Write | EAH_GPU_Read);
			binormal_vb->Resize(sizeof(b));
			{
				GraphicsBuffer::Mapper mapper(*binormal_vb, BA_Write_Only);
				std::copy(&b[0], &b[0] + sizeof(b) / sizeof(b[0]), mapper.Pointer<float3>());
			}

			rl_->BindVertexStream(pos_vb, boost::make_tuple(vertex_element(VEU_Position, 0, EF_BGR32F)));
			rl_->BindVertexStream(tex0_vb, boost::make_tuple(vertex_element(VEU_TextureCoord, 0, EF_GR32F)));
			rl_->BindVertexStream(tan_vb, boost::make_tuple(vertex_element(VEU_Tangent, 0, EF_BGR32F)));
			rl_->BindVertexStream(binormal_vb, boost::make_tuple(vertex_element(VEU_Binormal, 0, EF_BGR32F)));

			GraphicsBufferPtr ib = rf.MakeIndexBuffer(BU_Static, EAH_CPU_Write | EAH_GPU_Read);
			ib->Resize(sizeof(indices));
			{
				GraphicsBuffer::Mapper mapper(*ib, BA_Write_Only);
				std::copy(indices, indices + sizeof(indices) / sizeof(uint16_t), mapper.Pointer<uint16_t>());
			}
			rl_->BindIndexStream(ib, EF_R16);

			box_ = MathLib::compute_bounding_box<float>(&xyzs[0], &xyzs[4]);
		}

		void OnRenderBegin()
		{
			App3DFramework const & app = Context::Instance().AppInstance();

			float4x4 model = MathLib::rotation_x(-0.5f);
			float4x4 const & view = app.ActiveCamera().ViewMatrix();
			float4x4 const & proj = app.ActiveCamera().ProjMatrix();

			*(technique_->Effect().ParameterByName("worldviewproj")) = model * view * proj;
			*(technique_->Effect().ParameterByName("eye_pos")) = app.ActiveCamera().EyePos();

			float degree(std::clock() / 700.0f);
			float3 lightPos(2, 0, -2);
			float4x4 matRot(MathLib::rotation_z(degree));
			lightPos = MathLib::transform_coord(lightPos, matRot);
			*(technique_->Effect().ParameterByName("light_pos")) = lightPos;
		}
	};

	class PolygonObject : public SceneObjectHelper
	{
	public:
		PolygonObject()
			: SceneObjectHelper(RenderablePtr(new RenderPolygon), SOA_Cullable)
		{
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
	ResLoader::Instance().AddPath("../../media/Common");
	ResLoader::Instance().AddPath("../../media/DistanceMapping");

	RenderSettings settings;
	SceneManagerPtr sm;

	{
		int octree_depth = 3;
		int width = 800;
		int height = 600;
		int color_fmt = 13; // EF_ARGB8
		bool full_screen = false;

		boost::program_options::options_description desc("Configuration");
		desc.add_options()
			("context.render_factory", boost::program_options::value<std::string>(), "Render Factory")
			("context.input_factory", boost::program_options::value<std::string>(), "Input Factory")
			("context.scene_manager", boost::program_options::value<std::string>(), "Scene Manager")
			("octree.depth", boost::program_options::value<int>(&octree_depth)->default_value(3), "Octree depth")
			("screen.width", boost::program_options::value<int>(&width)->default_value(800), "Screen Width")
			("screen.height", boost::program_options::value<int>(&height)->default_value(600), "Screen Height")
			("screen.color_fmt", boost::program_options::value<int>(&color_fmt)->default_value(13), "Screen Color Format")
			("screen.fullscreen", boost::program_options::value<bool>(&full_screen)->default_value(false), "Full Screen");

		std::ifstream cfg_fs(ResLoader::Instance().Locate("KlayGE.cfg").c_str());
		if (cfg_fs)
		{
			boost::program_options::variables_map vm;
			boost::program_options::store(boost::program_options::parse_config_file(cfg_fs, desc), vm);
			boost::program_options::notify(vm);

			if (vm.count("context.render_factory"))
			{
				std::string rf_name = vm["context.render_factory"].as<std::string>();
				if ("D3D9" == rf_name)
				{
					Context::Instance().RenderFactoryInstance(D3D9RenderFactoryInstance());
				}
				if ("OpenGL" == rf_name)
				{
					Context::Instance().RenderFactoryInstance(OGLRenderFactoryInstance());
				}
			}
			else
			{
				Context::Instance().RenderFactoryInstance(D3D9RenderFactoryInstance());
			}

			if (vm.count("context.input_factory"))
			{
				std::string if_name = vm["context.input_factory"].as<std::string>();
				if ("DInput" == if_name)
				{
					Context::Instance().InputFactoryInstance(DInputFactoryInstance());
				}
			}
			else
			{
				Context::Instance().InputFactoryInstance(DInputFactoryInstance());
			}

			if (vm.count("context.scene_manager"))
			{
				std::string sm_name = vm["context.scene_manager"].as<std::string>();
				if ("Octree" == sm_name)
				{
					sm.reset(new OCTree(octree_depth));
					Context::Instance().SceneManagerInstance(*sm);
				}
			}
		}
		else
		{
			Context::Instance().RenderFactoryInstance(D3D9RenderFactoryInstance());
			Context::Instance().InputFactoryInstance(DInputFactoryInstance());
		}

		settings.width = width;
		settings.height = height;
		settings.color_fmt = static_cast<ElementFormat>(color_fmt);
		settings.full_screen = full_screen;
		settings.ConfirmDevice = ConfirmDevice;
	}

	DistanceMapping app("DistanceMapping", settings);
	app.Create();
	app.Run();

	return 0;
}

DistanceMapping::DistanceMapping(std::string const & name, RenderSettings const & settings)
					: App3DFramework(name, settings)
{
}

void DistanceMapping::InitObjects()
{
	// ½¨Á¢×ÖÌå
	font_ = Context::Instance().RenderFactoryInstance().MakeFont("gkai00mp.kfont", 16);

	polygon_.reset(new PolygonObject);
	polygon_->AddToSceneManager();

	this->LookAt(float3(2, 0, -2), float3(0, 0, 0));
	this->Proj(0.1f, 100);

	fpcController_.AttachCamera(this->ActiveCamera());
	fpcController_.Scalers(0.05f, 0.1f);

	InputEngine& inputEngine(Context::Instance().InputFactoryInstance().InputEngineInstance());
	InputActionMap actionMap;
	actionMap.AddActions(actions, actions + sizeof(actions) / sizeof(actions[0]));

	action_handler_t input_handler(new input_signal);
	input_handler->connect(boost::bind(&DistanceMapping::InputHandler, this, _1, _2));
	inputEngine.ActionMap(actionMap, input_handler, true);
}

void DistanceMapping::InputHandler(InputEngine const & /*sender*/, InputAction const & action)
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

uint32_t DistanceMapping::DoUpdate(uint32_t /*pass*/)
{
	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
	renderEngine.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, Color(0.2f, 0.4f, 0.6f, 1), 1.0f, 0);

	std::wostringstream stream;
	stream << this->FPS() << " FPS";

	font_->RenderText(0, 0, Color(1, 1, 0, 1), L"Distance Mapping");
	font_->RenderText(0, 18, Color(1, 1, 0, 1), stream.str());
	font_->RenderText(0, 36, Color(1, 1, 0, 1), renderEngine.Name());

	return App3DFramework::URV_Need_Flush | App3DFramework::URV_Finished;
}
