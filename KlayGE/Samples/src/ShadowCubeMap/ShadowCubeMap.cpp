#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/GraphicsBuffer.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/Renderable.hpp>
#include <KlayGE/RenderableHelper.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/FrameBuffer.hpp>
#include <KlayGE/SceneManager.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/RenderSettings.hpp>
#include <KlayGE/Mesh.hpp>
#include <KlayGE/KMesh.hpp>
#include <KlayGE/Texture.hpp>
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

#include "ShadowCubeMap.hpp"

using namespace std;
using namespace KlayGE;
using namespace KlayGE::MathLib;

namespace
{
	uint32_t const SHADOW_MAP_SIZE = 512;

	class ShadowMapped
	{
	public:
		ShadowMapped(uint32_t shadow_map_size)
			: shadow_map_size_(shadow_map_size)
		{
		}

		float4x4 LightViewProj() const
		{
			return light_view_ * light_proj_;
		}

		virtual void GenShadowMapPass(bool gen_sm)
		{
			gen_sm_pass_ = gen_sm;
		}

		void LightMatrices(float4x4 const & model)
		{
			light_pos_ = transform_coord(float3(0, 0, 0), model);

			inv_light_model_ = MathLib::inverse(model);

			App3DFramework const & app = Context::Instance().AppInstance();
			light_view_ = app.ActiveCamera().ViewMatrix();
			light_proj_ = app.ActiveCamera().ProjMatrix();
		}

		void ShadowMapTexture(TexturePtr tex)
		{
			sm_tex_ = tex;
		}

		void LampTexture(TexturePtr tex)
		{
			lamp_tex_ = tex;
		}

	protected:
		void OnRenderBegin(float4x4 const & model, RenderEffectPtr effect)
		{
			App3DFramework const & app = Context::Instance().AppInstance();

			*(effect->ParameterByName("model")) = model;
			*(effect->ParameterByName("obj_model_to_light_model")) = model * inv_light_model_;

			if (gen_sm_pass_)
			{
				*(effect->ParameterByName("model_view_proj")) = model * this->LightViewProj();
			}
			else
			{
				float4x4 const & view = app.ActiveCamera().ViewMatrix();
				float4x4 const & proj = app.ActiveCamera().ProjMatrix();

				*(effect->ParameterByName("model_view_proj")) = model * view * proj;
				*(effect->ParameterByName("light_pos")) = light_pos_;

				*(effect->ParameterByName("lamp_sampler")) = lamp_tex_;
				*(effect->ParameterByName("shadow_map_sampler")) = sm_tex_;
			}
		}

	protected:
		uint32_t shadow_map_size_;

		bool gen_sm_pass_;
		TexturePtr sm_tex_;

		float3 light_pos_;
		float4x4 inv_light_model_;
		float4x4 light_view_, light_proj_;

		TexturePtr lamp_tex_;
	};

	class OccluderRenderable : public KMesh, public ShadowMapped
	{
	public:
		OccluderRenderable(RenderModelPtr model, std::wstring const & /*name*/)
			: KMesh(model, L"Occluder"),
				ShadowMapped(SHADOW_MAP_SIZE)
		{
			effect_ = Context::Instance().RenderFactoryInstance().LoadEffect("ShadowCubeMap.kfx");
		}

		void SetModelMatrix(float4x4 const & model)
		{
			model_ = model;
		}

		void GenShadowMapPass(bool gen_sm)
		{
			ShadowMapped::GenShadowMapPass(gen_sm);

			if (gen_sm)
			{
				technique_ = effect_->TechniqueByName("GenShadowMap");
			}
			else
			{
				technique_ = effect_->TechniqueByName("RenderScene");
			}
		}

		void OnRenderBegin()
		{
			model_ = MathLib::translation(0.2f, 0.1f, 0.0f)
				* MathLib::rotation_y(-std::clock() / 1500.0f);
			ShadowMapped::OnRenderBegin(model_, effect_);
		}

	private:
		float4x4 model_;

		RenderEffectPtr effect_;
	};

	class OccluderObject : public SceneObjectHelper
	{
	public:
		OccluderObject()
			: SceneObjectHelper(SOA_Cullable)
		{
			model_ = MathLib::translation(0.0f, 0.2f, 0.0f);

			renderable_ = LoadKModel("teapot.kmodel", EAH_CPU_Write | EAH_GPU_Read, CreateKModelFactory<RenderModel>(), CreateKMeshFactory<OccluderRenderable>())->Mesh(0);
			checked_pointer_cast<OccluderRenderable>(renderable_)->SetModelMatrix(model_);
		}

	private:
		float4x4 model_;
	};

	class GroundRenderable : public RenderableHelper, public ShadowMapped
	{
	public:
		GroundRenderable()
			: RenderableHelper(L"Ground"),
				ShadowMapped(SHADOW_MAP_SIZE)
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();

			effect_ = rf.LoadEffect("ShadowCubeMap.kfx");
			technique_ = effect_->TechniqueByName("RenderScene");

			float3 xyzs[] =
			{
				float3(-1, 0, 1),
				float3(1, 0, 1),
				float3(1, 0, -1),
				float3(-1, 0, -1),
			};

			uint16_t indices[] =
			{
				0, 1, 2, 2, 3, 0
			};

			rl_ = rf.MakeRenderLayout();
			rl_->TopologyType(RenderLayout::TT_TriangleList);

			GraphicsBufferPtr pos_vb = rf.MakeVertexBuffer(BU_Static, EAH_CPU_Write | EAH_GPU_Read);
			pos_vb->Resize(sizeof(xyzs));
			{
				GraphicsBuffer::Mapper mapper(*pos_vb, BA_Write_Only);
				std::copy(&xyzs[0], &xyzs[0] + sizeof(xyzs) / sizeof(xyzs[0]), mapper.Pointer<float3>());
			}
			rl_->BindVertexStream(pos_vb, boost::make_tuple(vertex_element(VEU_Position, 0, EF_BGR32F)));

			GraphicsBufferPtr ib = rf.MakeIndexBuffer(BU_Static, EAH_CPU_Write | EAH_GPU_Read);
			ib->Resize(sizeof(indices));
			{
				GraphicsBuffer::Mapper mapper(*ib, BA_Write_Only);
				std::copy(indices, indices + sizeof(indices) / sizeof(uint16_t), mapper.Pointer<uint16_t>());
			}
			rl_->BindIndexStream(ib, EF_R16);

			float3 normal[sizeof(xyzs) / sizeof(xyzs[0])];
			MathLib::compute_normal<float>(&normal[0],
				&indices[0], &indices[sizeof(indices) / sizeof(uint16_t)],
				&xyzs[0], &xyzs[sizeof(xyzs) / sizeof(xyzs[0])]);

			GraphicsBufferPtr normal_vb = rf.MakeVertexBuffer(BU_Static, EAH_CPU_Write | EAH_GPU_Read);
			normal_vb->Resize(sizeof(normal));
			{
				GraphicsBuffer::Mapper mapper(*normal_vb, BA_Write_Only);
				std::copy(&normal[0], &normal[0] + sizeof(normal) / sizeof(normal[0]), mapper.Pointer<float3>());
			}
			rl_->BindVertexStream(normal_vb, boost::make_tuple(vertex_element(VEU_Normal, 0, EF_BGR32F)));

			box_ = MathLib::compute_bounding_box<float>(&xyzs[0], &xyzs[4]);
		}

		void GenShadowMapPass(bool gen_sm)
		{
			ShadowMapped::GenShadowMapPass(gen_sm);

			if (gen_sm)
			{
				technique_ = effect_->TechniqueByName("GenShadowMap");
			}
			else
			{
				technique_ = effect_->TechniqueByName("RenderScene");
			}
		}

		void SetModelMatrix(float4x4 const & model)
		{
			model_ = model;
		}

		void OnRenderBegin()
		{
			ShadowMapped::OnRenderBegin(model_, effect_);
		}

	private:
		float4x4 model_;

		RenderEffectPtr effect_;
	};

	class GroundObject : public SceneObjectHelper
	{
	public:
		GroundObject()
			: SceneObjectHelper(RenderablePtr(new GroundRenderable), SOA_Cullable)
		{
			model_ = MathLib::translation(0.0f, -0.2f, 0.0f);

			checked_pointer_cast<GroundRenderable>(renderable_)->SetModelMatrix(model_);
		}

	private:
		float4x4 model_;
	};


	enum
	{
		Exit,
	};

	InputActionDefine actions[] =
	{
		InputActionDefine(Exit, KS_Escape),
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
			rf.MakeDepthStencilRenderView(SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, EF_D16, 0);
			rf.MakeTextureCube(SHADOW_MAP_SIZE, 1, EF_GR16F, EAH_GPU_Read | EAH_GPU_Write);
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
	ResLoader::Instance().AddPath("../../media/Common");
	ResLoader::Instance().AddPath("../../media/ShadowCubeMap");

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

	ShadowCubeMap app("ShadowCubeMap", settings);
	app.Create();
	app.Run();

	return 0;
}

ShadowCubeMap::ShadowCubeMap(std::string const & name, RenderSettings const & settings)
				: App3DFramework(name, settings)
{
}

void ShadowCubeMap::InitObjects()
{
	RenderFactory& rf = Context::Instance().RenderFactoryInstance();

	// ½¨Á¢×ÖÌå
	font_ = rf.MakeFont("gkai00mp.kfont", 16);

	ground_.reset(new GroundObject);
	ground_->AddToSceneManager();

	mesh_.reset(new OccluderObject);
	mesh_->AddToSceneManager();

	this->LookAt(float3(1.3f, 0.5f, -0.7f), float3(0, 0, 0));
	this->Proj(0.01f, 100);

	lamp_tex_ = LoadTexture("lamp.dds", EAH_CPU_Write | EAH_GPU_Read);

	checked_pointer_cast<OccluderRenderable>(mesh_->GetRenderable())->LampTexture(lamp_tex_);
	checked_pointer_cast<GroundRenderable>(ground_->GetRenderable())->LampTexture(lamp_tex_);

	RenderViewPtr depth_view = rf.MakeDepthStencilRenderView(SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, EF_D16, 0);
	shadow_tex_ = rf.MakeTextureCube(SHADOW_MAP_SIZE, 1, EF_GR16F, EAH_GPU_Read | EAH_GPU_Write);
	for (int i = 0; i < 6; ++ i)
	{
		shadow_buffers_[i] = rf.MakeFrameBuffer();
		shadow_buffers_[i]->Attach(FrameBuffer::ATT_Color0,
			rf.Make2DRenderView(*shadow_tex_, static_cast<Texture::CubeFaces>(i), 0));
		shadow_buffers_[i]->Attach(FrameBuffer::ATT_DepthStencil, depth_view);

		CameraPtr camera = shadow_buffers_[i]->GetViewport().camera;
		camera->ProjParams(PI / 2.0f, 1.0f, 0.01f, 10.0f);
	}

	fpcController_.AttachCamera(this->ActiveCamera());
	fpcController_.Scalers(0.05f, 0.1f);

	InputEngine& inputEngine(Context::Instance().InputFactoryInstance().InputEngineInstance());
	InputActionMap actionMap;
	actionMap.AddActions(actions, actions + sizeof(actions) / sizeof(actions[0]));

	action_handler_t input_handler(new input_signal);
	input_handler->connect(boost::bind(&ShadowCubeMap::InputHandler, this, _1, _2));
	inputEngine.ActionMap(actionMap, input_handler, true);
}

void ShadowCubeMap::InputHandler(InputEngine const & /*sender*/, InputAction const & action)
{
	switch (action.first)
	{
	case Exit:
		this->Quit();
		break;
	}
}

uint32_t ShadowCubeMap::DoUpdate(uint32_t pass)
{
	RenderEngine& renderEngine = Context::Instance().RenderFactoryInstance().RenderEngineInstance();

	switch (pass)
	{
	case 0:
		{
			checked_pointer_cast<OccluderRenderable>(mesh_->GetRenderable())->GenShadowMapPass(true);
			checked_pointer_cast<GroundRenderable>(ground_->GetRenderable())->GenShadowMapPass(true);

			light_model_ = MathLib::rotation_z(0.4f) * MathLib::rotation_y(std::clock() / 1400.0f)
				* MathLib::translation(0.1f, 0.4f, 0.2f);

			for (int i = 0; i < 6; ++ i)
			{
				CameraPtr camera = shadow_buffers_[i]->GetViewport().camera;

				Texture::CubeFaces face = static_cast<Texture::CubeFaces>(Texture::CF_Positive_X + i);

				std::pair<float3, float3> lookat_up = CubeMapViewVector<float>(face);

				float3 le = transform_coord(float3(0, 0, 0), light_model_);
				float3 lla = transform_coord(float3(0, 0, 0) + lookat_up.first, light_model_);
				float3 lu = transform_normal(float3(0, 0, 0) + lookat_up.second, light_model_);

				camera->ViewParams(le, lla, lu);
			}
		}

	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
		{
			renderEngine.BindFrameBuffer(shadow_buffers_[pass]);
			renderEngine.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, Color(0.2f, 0.4f, 0.6f, 1), 1.0f, 0);

			checked_pointer_cast<OccluderRenderable>(mesh_->GetRenderable())->LightMatrices(light_model_);
			checked_pointer_cast<GroundRenderable>(ground_->GetRenderable())->LightMatrices(light_model_);
		}
		return App3DFramework::URV_Need_Flush;

	default:
		{
			renderEngine.BindFrameBuffer(FrameBufferPtr());
			renderEngine.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, Color(0.2f, 0.4f, 0.6f, 1), 1.0f, 0);

			//SaveTexture(shadow_cube_tex_, "shadow_tex.dds");

			checked_pointer_cast<OccluderRenderable>(mesh_->GetRenderable())->ShadowMapTexture(shadow_tex_);
			checked_pointer_cast<GroundRenderable>(ground_->GetRenderable())->ShadowMapTexture(shadow_tex_);

			checked_pointer_cast<OccluderRenderable>(mesh_->GetRenderable())->GenShadowMapPass(false);
			checked_pointer_cast<GroundRenderable>(ground_->GetRenderable())->GenShadowMapPass(false);

			std::wostringstream stream;
			stream << this->FPS();

			font_->RenderText(0, 0, Color(1, 1, 0, 1), L"ShadowCubeMap");
			font_->RenderText(0, 18, Color(1, 1, 0, 1), stream.str());
		}
		return App3DFramework::URV_Need_Flush | App3DFramework::URV_Finished;
	}
}
