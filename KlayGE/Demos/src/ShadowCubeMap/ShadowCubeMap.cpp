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
#include <KlayGE/SceneManager.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/RenderSettings.hpp>
#include <KlayGE/Sampler.hpp>
#include <KlayGE/Mesh.hpp>
#include <KlayGE/KMesh.hpp>
#include <KlayGE/Texture.hpp>
#include <KlayGE/SceneObjectHelper.hpp>

#include <KlayGE/D3D9/D3D9RenderFactory.hpp>

#include <KlayGE/Input.hpp>
#include <KlayGE/DInput/DInputFactory.hpp>

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
				sm_sampler_(new Sampler),
				lamp_sampler_(new Sampler)
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
			sm_sampler_->SetTexture(tex);
			sm_sampler_->Filtering(Sampler::TFO_Bilinear);
			sm_sampler_->AddressingMode(Sampler::TAT_Addr_U, Sampler::TAM_Wrap);
			sm_sampler_->AddressingMode(Sampler::TAT_Addr_V, Sampler::TAM_Wrap);
		}

		void LampTexture(TexturePtr tex)
		{
			lamp_sampler_->SetTexture(tex);
			lamp_sampler_->Filtering(Sampler::TFO_Bilinear);
			lamp_sampler_->AddressingMode(Sampler::TAT_Addr_U, Sampler::TAM_Wrap);
			lamp_sampler_->AddressingMode(Sampler::TAT_Addr_V, Sampler::TAM_Wrap);
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

				*(effect->ParameterByName("lamp_sampler")) = lamp_sampler_;
				*(effect->ParameterByName("shadow_map_sampler")) = sm_sampler_;
			}
		}

	protected:
		uint32_t shadow_map_size_;

		bool gen_sm_pass_;
		SamplerPtr sm_sampler_;

		float3 light_pos_;
		float4x4 inv_light_model_;
		float4x4 light_view_, light_proj_;

		SamplerPtr lamp_sampler_;
	};

	class OccluderRenderable : public KMesh, public ShadowMapped
	{
	public:
		OccluderRenderable(std::wstring const & /*name*/, TexturePtr tex)
			: KMesh(L"Occluder", tex),
				ShadowMapped(SHADOW_MAP_SIZE)
		{
			effect_ = Context::Instance().RenderFactoryInstance().LoadEffect("ShadowCubeMap.fx");
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
				technique_ = effect_->Technique("GenShadowMap");
			}
			else
			{
				technique_ = effect_->Technique("RenderScene");
			}
		}

		void OnRenderBegin()
		{
			model_ = MathLib::translation(0.2f, 0.1f, 0.0f)
				* MathLib::rotation_y(-std::clock() / 1000.0f);
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

			renderable_ = LoadKMesh("teapot.kmesh", CreateKMeshFactory<OccluderRenderable>())->Mesh(0);
			checked_cast<OccluderRenderable*>(renderable_.get())->SetModelMatrix(model_);
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

			effect_ = rf.LoadEffect("ShadowCubeMap.fx");
			technique_ = effect_->Technique("RenderScene");

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

			rl_ = rf.MakeRenderLayout(RenderLayout::BT_TriangleList);

			GraphicsBufferPtr pos_vb = rf.MakeVertexBuffer(BU_Static);
			pos_vb->Resize(sizeof(xyzs));
			{
				GraphicsBuffer::Mapper mapper(*pos_vb, BA_Write_Only);
				std::copy(&xyzs[0], &xyzs[0] + sizeof(xyzs) / sizeof(xyzs[0]), mapper.Pointer<float3>());
			}
			rl_->BindVertexStream(pos_vb, boost::make_tuple(vertex_element(VEU_Position, 0, EF_BGR32F)));

			GraphicsBufferPtr ib = rf.MakeIndexBuffer(BU_Static);
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

			GraphicsBufferPtr normal_vb = rf.MakeVertexBuffer(BU_Static);
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
				technique_ = effect_->Technique("GenShadowMap");
			}
			else
			{
				technique_ = effect_->Technique("RenderScene");
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

			checked_cast<GroundRenderable*>(renderable_.get())->SetModelMatrix(model_);
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
	Context::Instance().InputFactoryInstance(DInputFactoryInstance());

	RenderSettings settings;
	settings.width = 800;
	settings.height = 600;
	settings.color_fmt = EF_ARGB8;
	settings.full_screen = false;
	settings.ConfirmDevice = ConfirmDevice;

	ShadowCubeMap app;
	app.Create("ShadowCubeMap", settings);
	app.Run();

	return 0;
}

ShadowCubeMap::ShadowCubeMap()
{
	ResLoader::Instance().AddPath("../media/Common");
	ResLoader::Instance().AddPath("../media/ShadowCubeMap");
}

void ShadowCubeMap::InitObjects()
{
	RenderFactory& rf = Context::Instance().RenderFactoryInstance();

	// ½¨Á¢×ÖÌå
	font_ = rf.MakeFont("gkai00mp.ttf", 16);

	ground_.reset(new GroundObject);
	ground_->AddToSceneManager();

	mesh_.reset(new OccluderObject);
	mesh_->AddToSceneManager();

	RenderEngine& renderEngine(rf.RenderEngineInstance());

	renderEngine.ClearColor(Color(0.2f, 0.4f, 0.6f, 1));

	this->LookAt(float3(1.3f, 0.5f, -0.7f), float3(0, 0, 0));
	this->Proj(0.01f, 100);

	lamp_tex_ = LoadTexture("lamp.dds");

	checked_cast<OccluderRenderable*>(mesh_->GetRenderable().get())->LampTexture(lamp_tex_);
	checked_cast<GroundRenderable*>(ground_->GetRenderable().get())->LampTexture(lamp_tex_);

	RenderViewPtr depth_view = rf.MakeDepthStencilRenderView(SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, EF_D16, 0);
	shadow_tex_ = rf.MakeTextureCube(SHADOW_MAP_SIZE, 1, EF_GR16F);
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

	action_handler_t input_handler(inputEngine);
	input_handler += boost::bind(&ShadowCubeMap::InputHandler, this, _1, _2);
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

uint32_t ShadowCubeMap::NumPasses() const
{
	return 7;
}

void ShadowCubeMap::DoUpdate(uint32_t pass)
{
	RenderEngine& renderEngine = Context::Instance().RenderFactoryInstance().RenderEngineInstance();

	switch (pass)
	{
	case 0:
		{
			fpcController_.Update();

			checked_cast<OccluderRenderable*>(mesh_->GetRenderable().get())->GenShadowMapPass(true);
			checked_cast<GroundRenderable*>(ground_->GetRenderable().get())->GenShadowMapPass(true);

			light_model_ = MathLib::rotation_z(0.4f) * MathLib::rotation_y(std::clock() / 700.0f)
				* MathLib::translation(0.1f, 0.7f, 0.2f);

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
			renderEngine.BindRenderTarget(shadow_buffers_[pass]);
			renderEngine.Clear(RenderEngine::CBM_Color | RenderEngine::CBM_Depth);

			checked_cast<OccluderRenderable*>(mesh_->GetRenderable().get())->LightMatrices(light_model_);
			checked_cast<GroundRenderable*>(ground_->GetRenderable().get())->LightMatrices(light_model_);
		}
		break;

	case 6:
		{
			renderEngine.BindRenderTarget(RenderTargetPtr());
			renderEngine.Clear(RenderEngine::CBM_Color | RenderEngine::CBM_Depth);

			//SaveTexture(shadow_cube_tex_, "shadow_tex.dds");

			checked_cast<OccluderRenderable*>(mesh_->GetRenderable().get())->ShadowMapTexture(shadow_tex_);
			checked_cast<GroundRenderable*>(ground_->GetRenderable().get())->ShadowMapTexture(shadow_tex_);

			checked_cast<OccluderRenderable*>(mesh_->GetRenderable().get())->GenShadowMapPass(false);
			checked_cast<GroundRenderable*>(ground_->GetRenderable().get())->GenShadowMapPass(false);

			std::wostringstream stream;
			stream << this->FPS();

			font_->RenderText(0, 0, Color(1, 1, 0, 1), L"ShadowCubeMap");
			font_->RenderText(0, 18, Color(1, 1, 0, 1), stream.str().c_str());
		}
		break;
	}
}
