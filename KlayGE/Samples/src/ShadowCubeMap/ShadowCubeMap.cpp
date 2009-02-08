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

#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/InputFactory.hpp>

#include <vector>
#include <sstream>
#include <ctime>
#include <boost/bind.hpp>

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
			: shadow_map_size_(shadow_map_size),
				light_view_proj_mat_(6)
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

		void LightMatrices(int pass, float4x4 const & model)
		{
			light_pos_ = transform_coord(float3(0, 0, 0), model);

			inv_light_model_ = MathLib::inverse(model);

			App3DFramework const & app = Context::Instance().AppInstance();
			light_view_ = app.ActiveCamera().ViewMatrix();
			light_proj_ = app.ActiveCamera().ProjMatrix();

			light_view_proj_mat_[pass] = light_view_ * light_proj_;
		}

		void ShadowMapTexture(TexturePtr tex[6], bool flip)
		{
			sm_tex_.assign(&tex[0], &tex[6]);
			flip_ = flip;
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
				*(effect->ParameterByName("flip")) = flip_ ? -1 : 1;

				*(effect->ParameterByName("lamp_tex")) = lamp_tex_;
				*(effect->ParameterByName("shadow_map_x_pos_tex")) = sm_tex_[0];
				*(effect->ParameterByName("shadow_map_x_neg_tex")) = sm_tex_[1];
				*(effect->ParameterByName("shadow_map_y_pos_tex")) = sm_tex_[2];
				*(effect->ParameterByName("shadow_map_y_neg_tex")) = sm_tex_[3];
				*(effect->ParameterByName("shadow_map_z_pos_tex")) = sm_tex_[4];
				*(effect->ParameterByName("shadow_map_z_neg_tex")) = sm_tex_[5];

				*(effect->ParameterByName("light_view_proj")) = light_view_proj_mat_;
			}
		}

	protected:
		uint32_t shadow_map_size_;

		bool gen_sm_pass_;
		std::vector<TexturePtr> sm_tex_;
		std::vector<float4x4> light_view_proj_mat_;
		bool flip_;

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

		void MinVariance(float min_variance)
		{
			*(effect_->ParameterByName("min_variance")) = min_variance;
		}

		void BleedingReduce(float bleeding_reduce)
		{
			*(effect_->ParameterByName("bleeding_reduce")) = bleeding_reduce;
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

			renderable_ = LoadKModel("teapot.kmodel", EAH_GPU_Read, CreateKModelFactory<RenderModel>(), CreateKMeshFactory<OccluderRenderable>())->Mesh(0);
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

			ElementInitData init_data;
			init_data.row_pitch = sizeof(xyzs);
			init_data.slice_pitch = 0;
			init_data.data = xyzs;
			GraphicsBufferPtr pos_vb = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read, &init_data);
			rl_->BindVertexStream(pos_vb, boost::make_tuple(vertex_element(VEU_Position, 0, EF_BGR32F)));

			init_data.row_pitch = sizeof(indices);
			init_data.slice_pitch = 0;
			init_data.data = indices;
			GraphicsBufferPtr ib = rf.MakeIndexBuffer(BU_Static, EAH_GPU_Read, &init_data);
			rl_->BindIndexStream(ib, EF_R16UI);

			float3 normal[sizeof(xyzs) / sizeof(xyzs[0])];
			MathLib::compute_normal<float>(&normal[0],
				&indices[0], &indices[sizeof(indices) / sizeof(uint16_t)],
				&xyzs[0], &xyzs[sizeof(xyzs) / sizeof(xyzs[0])]);

			init_data.row_pitch = sizeof(normal);
			init_data.slice_pitch = 0;
			init_data.data = normal;
			GraphicsBufferPtr normal_vb = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read, &init_data);
			rl_->BindVertexStream(normal_vb, boost::make_tuple(vertex_element(VEU_Normal, 0, EF_BGR32F)));

			box_ = MathLib::compute_bounding_box<float>(&xyzs[0], &xyzs[4]);
		}

		void MinVariance(float min_variance)
		{
			*(effect_->ParameterByName("min_variance")) = min_variance;
		}

		void BleedingReduce(float bleeding_reduce)
		{
			*(effect_->ParameterByName("bleeding_reduce")) = bleeding_reduce;
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
			rf.MakeDepthStencilRenderView(SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, EF_D16, 1, 0);
		}
		catch (...)
		{
			return false;
		}

		try
		{
			rf.MakeTextureCube(SHADOW_MAP_SIZE, 1, EF_GR16F, 1, 0, EAH_GPU_Read | EAH_GPU_Write, NULL);
		}
		catch (...)
		{
			try
			{
				rf.MakeTextureCube(SHADOW_MAP_SIZE, 1, EF_ABGR16F, 1, 0, EAH_GPU_Read | EAH_GPU_Write, NULL);
			}
			catch (...)
			{
				return false;
			}
		}

		return true;
	}
}


int main()
{
	ResLoader::Instance().AddPath("../Samples/media/Common");
	ResLoader::Instance().AddPath("../Samples/media/ShadowCubeMap");

	RenderSettings settings = Context::Instance().LoadCfg("KlayGE.cfg");
	settings.ConfirmDevice = ConfirmDevice;

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
	font_ = rf.MakeFont("gkai00mp.kfont");

	ground_.reset(new GroundObject);
	ground_->AddToSceneManager();

	mesh_.reset(new OccluderObject);
	mesh_->AddToSceneManager();

	this->LookAt(float3(1.3f, 0.5f, -0.7f), float3(0, 0, 0));
	this->Proj(0.01f, 100);

	lamp_tex_ = LoadTexture("lamp.dds", EAH_GPU_Read)();

	checked_pointer_cast<OccluderRenderable>(mesh_->GetRenderable())->LampTexture(lamp_tex_);
	checked_pointer_cast<GroundRenderable>(ground_->GetRenderable())->LampTexture(lamp_tex_);

	RenderViewPtr depth_view = rf.MakeDepthStencilRenderView(SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, EF_D16, 1, 0);
	for (int i = 0; i < 6; ++ i)
	{
		shadow_buffers_[i] = rf.MakeFrameBuffer();
		try
		{
			shadow_tex_[i] = rf.MakeTexture2D(SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, 1, EF_GR16F, 1, 0, EAH_GPU_Read | EAH_GPU_Write, NULL);
			shadow_buffers_[i]->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*shadow_tex_[i], 0));
		}
		catch (...)
		{
			shadow_tex_[i] = rf.MakeTexture2D(SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, 1, EF_ABGR16F, 1, 0, EAH_GPU_Read | EAH_GPU_Write, NULL);
			shadow_buffers_[i]->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*shadow_tex_[i], 0));
		}

		shadow_buffers_[i]->Attach(FrameBuffer::ATT_DepthStencil, depth_view);

		CameraPtr camera = shadow_buffers_[i]->GetViewport().camera;
		camera->ProjParams(PI / 2.0f, 1.0f, 0.01f, 10.0f);
	}

	fpcController_.Scalers(0.05f, 0.1f);

	InputEngine& inputEngine(Context::Instance().InputFactoryInstance().InputEngineInstance());
	InputActionMap actionMap;
	actionMap.AddActions(actions, actions + sizeof(actions) / sizeof(actions[0]));

	action_handler_t input_handler(new input_signal);
	input_handler->connect(boost::bind(&ShadowCubeMap::InputHandler, this, _1, _2));
	inputEngine.ActionMap(actionMap, input_handler, true);

	UIManager::Instance().Load(ResLoader::Instance().Load("ShadowCubemap.kui"));
	dialog_ = UIManager::Instance().GetDialogs()[0];

	id_min_variance_static_ = dialog_->IDFromName("MinVarianceStatic");
	id_min_variance_slider_ = dialog_->IDFromName("MinVarianceSlider");
	id_bleeding_reduce_static_ = dialog_->IDFromName("BleedingReduceStatic");
	id_bleeding_reduce_slider_ = dialog_->IDFromName("BleedingReduceSlider");
	id_ctrl_camera_ = dialog_->IDFromName("CtrlCamera");

	dialog_->Control<UISlider>(id_min_variance_slider_)->OnValueChangedEvent().connect(boost::bind(&ShadowCubeMap::MinVarianceChangedHandler, this, _1));
	dialog_->Control<UISlider>(id_bleeding_reduce_slider_)->OnValueChangedEvent().connect(boost::bind(&ShadowCubeMap::BleedingReduceChangedHandler, this, _1));
	dialog_->Control<UICheckBox>(id_ctrl_camera_)->OnChangedEvent().connect(boost::bind(&ShadowCubeMap::CtrlCameraHandler, this, _1));

	this->MinVarianceChangedHandler(*dialog_->Control<UISlider>(id_min_variance_slider_));
	this->BleedingReduceChangedHandler(*dialog_->Control<UISlider>(id_bleeding_reduce_slider_));
}

void ShadowCubeMap::OnResize(uint32_t width, uint32_t height)
{
	UIManager::Instance().SettleCtrls(width, height);
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

void ShadowCubeMap::MinVarianceChangedHandler(KlayGE::UISlider const & sender)
{
	checked_pointer_cast<OccluderRenderable>(mesh_->GetRenderable())->MinVariance(sender.GetValue() / 2000.0f + 0.001f);
	checked_pointer_cast<GroundRenderable>(ground_->GetRenderable())->MinVariance(sender.GetValue() / 2000.0f + 0.001f);
}

void ShadowCubeMap::BleedingReduceChangedHandler(KlayGE::UISlider const & sender)
{
	checked_pointer_cast<OccluderRenderable>(mesh_->GetRenderable())->BleedingReduce(sender.GetValue() / 500.0f + 0.45f);
	checked_pointer_cast<GroundRenderable>(ground_->GetRenderable())->BleedingReduce(sender.GetValue() / 500.0f + 0.45f);
}

void ShadowCubeMap::CtrlCameraHandler(KlayGE::UICheckBox const & sender)
{
	if (sender.GetChecked())
	{
		fpcController_.AttachCamera(this->ActiveCamera());
	}
	else
	{
		fpcController_.DetachCamera();
	}
}

uint32_t ShadowCubeMap::DoUpdate(uint32_t pass)
{
	RenderEngine& renderEngine = Context::Instance().RenderFactoryInstance().RenderEngineInstance();

	switch (pass)
	{
	case 0:
		{
			UIManager::Instance().HandleInput();

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

			checked_pointer_cast<OccluderRenderable>(mesh_->GetRenderable())->LightMatrices(pass, light_model_);
			checked_pointer_cast<GroundRenderable>(ground_->GetRenderable())->LightMatrices(pass, light_model_);
		}
		return App3DFramework::URV_Need_Flush;

	default:
		{
			renderEngine.BindFrameBuffer(FrameBufferPtr());
			renderEngine.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, Color(0.2f, 0.4f, 0.6f, 1), 1.0f, 0);

			//SaveTexture(shadow_cube_tex_, "shadow_tex.dds");

			checked_pointer_cast<OccluderRenderable>(mesh_->GetRenderable())->ShadowMapTexture(shadow_tex_, shadow_buffers_[0]->RequiresFlipping());
			checked_pointer_cast<GroundRenderable>(ground_->GetRenderable())->ShadowMapTexture(shadow_tex_, shadow_buffers_[0]->RequiresFlipping());

			checked_pointer_cast<OccluderRenderable>(mesh_->GetRenderable())->GenShadowMapPass(false);
			checked_pointer_cast<GroundRenderable>(ground_->GetRenderable())->GenShadowMapPass(false);

			UIManager::Instance().Render();

			std::wostringstream stream;
			stream << this->FPS();

			font_->RenderText(0, 0, Color(1, 1, 0, 1), L"ShadowCubeMap", 16);
			font_->RenderText(0, 18, Color(1, 1, 0, 1), stream.str(), 16);
		}
		return App3DFramework::URV_Need_Flush | App3DFramework::URV_Finished;
	}
}
