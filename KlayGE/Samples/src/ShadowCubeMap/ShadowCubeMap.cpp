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
#include <KlayGE/Mesh.hpp>
#include <KlayGE/Texture.hpp>
#include <KlayGE/SceneObjectHelper.hpp>
#include <KlayGE/PostProcess.hpp>
#include <KlayGE/Light.hpp>

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

		void LightSrc(LightSourcePtr const & light_src)
		{
			light_pos_ = light_src->Position();

			float4x4 light_model = MathLib::to_matrix(light_src->Rotation()) * MathLib::translation(light_src->Position());
			inv_light_model_ = MathLib::inverse(light_model);

			App3DFramework const & app = Context::Instance().AppInstance();
			light_view_ = app.ActiveCamera().ViewMatrix();
			light_proj_ = app.ActiveCamera().ProjMatrix();

			light_falloff_ = light_src->Falloff();
		}

		void ShadowMapTexture(TexturePtr const & cube_tex)
		{
			sm_cube_tex_ = cube_tex;
		}

		void LampTexture(TexturePtr const & tex)
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
				*(effect->ParameterByName("mvp")) = model * this->LightViewProj();
			}
			else
			{
				float4x4 const & view = app.ActiveCamera().ViewMatrix();
				float4x4 const & proj = app.ActiveCamera().ProjMatrix();

				*(effect->ParameterByName("mvp")) = model * view * proj;
				*(effect->ParameterByName("light_pos")) = light_pos_;

				RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
				*(effect->ParameterByName("flipping")) = static_cast<int32_t>(re.CurFrameBuffer()->RequiresFlipping() ? -1 : +1);

				*(effect->ParameterByName("light_projective_tex")) = lamp_tex_;
				*(effect->ParameterByName("shadow_cube_tex")) = sm_cube_tex_;

				*(effect->ParameterByName("light_falloff")) = light_falloff_;
			}
		}

	protected:
		uint32_t shadow_map_size_;

		bool gen_sm_pass_;
		TexturePtr sm_cube_tex_;

		float3 light_pos_;
		float4x4 inv_light_model_;
		float4x4 light_view_, light_proj_;
		float3 light_falloff_;

		TexturePtr lamp_tex_;
	};

	class OccluderRenderable : public StaticMesh, public ShadowMapped
	{
	public:
		OccluderRenderable(RenderModelPtr model, std::wstring const & /*name*/)
			: StaticMesh(model, L"Occluder"),
				ShadowMapped(SHADOW_MAP_SIZE)
		{
			effect_ = Context::Instance().RenderFactoryInstance().LoadEffect("ShadowCubeMap.fxml");
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
			: SceneObjectHelper(SOA_Cullable | SOA_Moveable)
		{
			model_ = MathLib::translation(0.0f, 0.2f, 0.0f);

			renderable_ = LoadModel("teapot.meshml", EAH_GPU_Read, CreateModelFactory<RenderModel>(), CreateMeshFactory<OccluderRenderable>())()->Mesh(0);
			checked_pointer_cast<OccluderRenderable>(renderable_)->SetModelMatrix(model_);
		}

		void Update()
		{
			model_ = MathLib::translation(0.2f, 0.1f, 0.0f) * MathLib::rotation_y(-std::clock() / 1500.0f);
			checked_pointer_cast<OccluderRenderable>(renderable_)->SetModelMatrix(model_);
		}

		float4x4 const & GetModelMatrix() const
		{
			return model_;
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

			effect_ = rf.LoadEffect("ShadowCubeMap.fxml");
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
			: SceneObjectHelper(MakeSharedPtr<GroundRenderable>(), SOA_Cullable)
		{
			model_ = MathLib::translation(0.0f, -0.2f, 0.0f);
			checked_pointer_cast<GroundRenderable>(renderable_)->SetModelMatrix(model_);
		}
		
		float4x4 const & GetModelMatrix() const
		{
			return model_;
		}

	private:
		float4x4 model_;
	};


	class PointLightSourceUpdate
	{
	public:
		void operator()(float4x4& model)
		{
			model = MathLib::rotation_z(0.4f)
				* MathLib::rotation_y(static_cast<float>(timer_.current_time()) / 1.4f)
				* MathLib::translation(0.1f, 0.6f, 0.2f);
		}

	private:
		Timer timer_;
	};


	enum
	{
		Exit,
	};

	InputActionDefine actions[] =
	{
		InputActionDefine(Exit, KS_Escape),
	};
}


int main()
{
	ResLoader::Instance().AddPath("../Samples/media/Common");

	Context::Instance().LoadCfg("KlayGE.cfg");

	ShadowCubeMap app;
	app.Create();
	app.Run();

	return 0;
}

ShadowCubeMap::ShadowCubeMap()
				: App3DFramework("ShadowCubeMap")
{
	ResLoader::Instance().AddPath("../Samples/media/ShadowCubeMap");
}

bool ShadowCubeMap::ConfirmDevice() const
{
	RenderDeviceCaps const & caps = Context::Instance().RenderFactoryInstance().RenderEngineInstance().DeviceCaps();
	if (caps.max_shader_model < 2)
	{
		return false;
	}
	if (!(caps.rendertarget_format_support(EF_D16, 1, 0)
		&& (caps.rendertarget_format_support(EF_GR16F, 1, 0) || caps.rendertarget_format_support(EF_ABGR16F, 1, 0))))
	{
		return false;
	}

	return true;
}

void ShadowCubeMap::InitObjects()
{
	RenderFactory& rf = Context::Instance().RenderFactoryInstance();

	// ½¨Á¢×ÖÌå
	font_ = rf.MakeFont("gkai00mp.kfont");

	ground_ = MakeSharedPtr<GroundObject>();
	ground_->AddToSceneManager();

	mesh_ = MakeSharedPtr<OccluderObject>();
	mesh_->AddToSceneManager();

	this->LookAt(float3(1.3f, 0.5f, -0.7f), float3(0, 0, 0));
	this->Proj(0.01f, 100);

	lamp_tex_ = LoadTexture("lamp.dds", EAH_GPU_Read)();

	checked_pointer_cast<OccluderRenderable>(mesh_->GetRenderable())->LampTexture(lamp_tex_);
	checked_pointer_cast<GroundRenderable>(ground_->GetRenderable())->LampTexture(lamp_tex_);

	RenderViewPtr depth_view = rf.Make2DDepthStencilRenderView(SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, EF_D16, 1, 0);
	shadow_buffer_ = rf.MakeFrameBuffer();
	ElementFormat fmt;
	if (rf.RenderEngineInstance().DeviceCaps().rendertarget_format_support(EF_GR16F, 1, 0))
	{
		fmt = EF_GR16F;
	}
	else
	{
		BOOST_ASSERT(rf.RenderEngineInstance().DeviceCaps().rendertarget_format_support(EF_ABGR16F, 1, 0));

		fmt = EF_ABGR16F;
	}
	shadow_tex_ = rf.MakeTexture2D(SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write, NULL);
	shadow_buffer_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*shadow_tex_, 0, 0));
	shadow_buffer_->Attach(FrameBuffer::ATT_DepthStencil, depth_view);

	shadow_cube_tex_ = rf.MakeTextureCube(SHADOW_MAP_SIZE, 1, 1, shadow_tex_->Format(), 1, 0, EAH_GPU_Read | EAH_GPU_Write, NULL);

	light_ = MakeSharedPtr<PointLightSource>();
	light_->Attrib(0);
	light_->Color(float3(1, 1, 1));
	light_->Falloff(float3(0.01f, 0, 0.5f));
	light_->ProjectiveTexture(lamp_tex_);

	light_proxy_ = MakeSharedPtr<SceneObjectLightSourceProxy>(light_, 0.05f, PointLightSourceUpdate());
	light_proxy_->AddToSceneManager();

	for (int i = 0; i < 6; ++ i)
	{
		sm_filter_pps_[i] = MakeSharedPtr<BlurPostProcess<SeparableGaussianFilterPostProcess> >(3, 1.0f);
	
		sm_filter_pps_[i]->InputPin(0, shadow_tex_);
		sm_filter_pps_[i]->OutputPin(0, shadow_cube_tex_, 0, 0, i);
		if (!shadow_buffer_->RequiresFlipping())
		{
			switch (i)
			{
			case Texture::CF_Positive_Y:
				sm_filter_pps_[i]->OutputPin(0, shadow_cube_tex_, 0, 0, Texture::CF_Negative_Y);
				break;

			case Texture::CF_Negative_Y:
				sm_filter_pps_[i]->OutputPin(0, shadow_cube_tex_, 0, 0, Texture::CF_Positive_Y);
				break;

			default:
				break;
			}
		}
	}

	fpcController_.Scalers(0.05f, 0.1f);

	InputEngine& inputEngine(Context::Instance().InputFactoryInstance().InputEngineInstance());
	InputActionMap actionMap;
	actionMap.AddActions(actions, actions + sizeof(actions) / sizeof(actions[0]));

	action_handler_t input_handler = MakeSharedPtr<input_signal>();
	input_handler->connect(boost::bind(&ShadowCubeMap::InputHandler, this, _1, _2));
	inputEngine.ActionMap(actionMap, input_handler, true);

	UIManager::Instance().Load(ResLoader::Instance().Load("ShadowCubemap.uiml"));
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
	App3DFramework::OnResize(width, height);

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

void ShadowCubeMap::DoUpdateOverlay()
{
	UIManager::Instance().Render();

	std::wostringstream stream;
	stream.precision(2);
	stream << std::fixed << this->FPS() << " FPS";

	font_->RenderText(0, 0, Color(1, 1, 0, 1), L"ShadowCubeMap", 16);
	font_->RenderText(0, 18, Color(1, 1, 0, 1), stream.str(), 16);
}

uint32_t ShadowCubeMap::DoUpdate(uint32_t pass)
{
	RenderEngine& renderEngine = Context::Instance().RenderFactoryInstance().RenderEngineInstance();

	if (pass > 0)
	{
		sm_filter_pps_[pass - 1]->Apply();
	}

	switch (pass)
	{
	case 0:
		{
			checked_pointer_cast<OccluderRenderable>(mesh_->GetRenderable())->GenShadowMapPass(true);
			checked_pointer_cast<GroundRenderable>(ground_->GetRenderable())->GenShadowMapPass(true);
		}

	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
		{
			renderEngine.BindFrameBuffer(shadow_buffer_);
			renderEngine.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, Color(0.2f, 0.4f, 0.6f, 1), 1.0f, 0);

			shadow_buffer_->GetViewport().camera = light_->SMCamera(pass);

			checked_pointer_cast<OccluderRenderable>(mesh_->GetRenderable())->LightSrc(light_);
			checked_pointer_cast<GroundRenderable>(ground_->GetRenderable())->LightSrc(light_);
		}
		return App3DFramework::URV_Need_Flush;

	default:
		{
			renderEngine.BindFrameBuffer(FrameBufferPtr());
			renderEngine.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, Color(0.2f, 0.4f, 0.6f, 1), 1.0f, 0);

			checked_pointer_cast<OccluderRenderable>(mesh_->GetRenderable())->ShadowMapTexture(shadow_cube_tex_);
			checked_pointer_cast<GroundRenderable>(ground_->GetRenderable())->ShadowMapTexture(shadow_cube_tex_);

			checked_pointer_cast<OccluderRenderable>(mesh_->GetRenderable())->GenShadowMapPass(false);
			checked_pointer_cast<GroundRenderable>(ground_->GetRenderable())->GenShadowMapPass(false);
		}
		return App3DFramework::URV_Need_Flush | App3DFramework::URV_Finished;
	}
}
