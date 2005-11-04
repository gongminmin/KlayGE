#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/VertexBuffer.hpp>
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
				sm_sampler_(new Sampler)
		{
		}

		Matrix4 LightViewProj() const
		{
			return light_view_ * light_proj_;
		}

		void GenShadowMapPass(bool gen_sm)
		{
			gen_sm_pass_ = gen_sm;
		}

		void LightMatrices(Matrix4 const & model, Matrix4 const & view, Matrix4 const & proj)
		{
			light_pos_ = Transform(Vector3(0, 0, 0), model);
			light_pos_ /= light_pos_.w();

			inv_light_model_ = MathLib::Inverse(model);
			light_view_ = view;
			light_proj_ = proj;
		}

		void ShadowMapTexture(TexturePtr tex)
		{
			sm_sampler_->SetTexture(tex);
			sm_sampler_->Filtering(Sampler::TFO_Point);
			sm_sampler_->AddressingMode(Sampler::TAT_Addr_U, Sampler::TAM_Wrap);
			sm_sampler_->AddressingMode(Sampler::TAT_Addr_V, Sampler::TAM_Wrap);
		}

	protected:
		uint32_t shadow_map_size_;

		bool gen_sm_pass_;
		SamplerPtr sm_sampler_;

		Vector4 light_pos_;
		Matrix4 inv_light_model_;
		Matrix4 light_view_, light_proj_;
	};

	class OccluderRenderable : public KMesh, public ShadowMapped
	{
	public:
		OccluderRenderable(std::wstring const & /*name*/, TexturePtr tex)
			: KMesh(L"Occluder", tex),
				ShadowMapped(SHADOW_MAP_SIZE),
				lamp_sampler_(new Sampler)
		{
			effect_ = Context::Instance().RenderFactoryInstance().LoadEffect("ShadowCubeMap.fx");
			effect_->ActiveTechnique("RenderScene");
		}

		void SetModelMatrix(Matrix4 const & model)
		{
			model_ = model;
		}

		void OnRenderBegin()
		{
			App3DFramework const & app = Context::Instance().AppInstance();

			Matrix4 view = app.ActiveCamera().ViewMatrix();
			Matrix4 proj = app.ActiveCamera().ProjMatrix();
			Matrix4 light_mat = model_ * this->LightViewProj();

			*(effect_->ParameterByName("World")) = model_;
			*(effect_->ParameterByName("InvLightWorld")) = inv_light_model_;

			if (gen_sm_pass_)
			{
				effect_->ActiveTechnique("GenShadowMap");
				*(effect_->ParameterByName("WorldViewProj")) = light_mat;
			}
			else
			{
				effect_->ActiveTechnique("RenderScene");

				*(effect_->ParameterByName("LampSampler")) = lamp_sampler_;
				*(effect_->ParameterByName("ShadowMapSampler")) = sm_sampler_;
				*(effect_->ParameterByName("WorldViewProj")) = model_ * view * proj;
				*(effect_->ParameterByName("light_pos")) = light_pos_;
			}
		}

		void LampTexture(TexturePtr tex)
		{
			lamp_sampler_->SetTexture(tex);
			lamp_sampler_->Filtering(Sampler::TFO_Bilinear);
			lamp_sampler_->AddressingMode(Sampler::TAT_Addr_U, Sampler::TAM_Wrap);
			lamp_sampler_->AddressingMode(Sampler::TAT_Addr_V, Sampler::TAM_Wrap);
		}

	private:
		SamplerPtr lamp_sampler_;
		Matrix4 model_;
	};

	class OccluderObject : public SceneObjectHelper
	{
	public:
		OccluderObject()
			: SceneObjectHelper(SOA_Cullable)
		{
			model_ = MathLib::Translation(0.0f, 0.2f, 0.0f);

			renderable_ = LoadKMesh("teapot.kmesh", CreateFactory<OccluderRenderable>)->Mesh(0);
			checked_cast<OccluderRenderable*>(renderable_.get())->SetModelMatrix(model_);
		}

	private:
		Matrix4 model_;
	};

	class GroundRenderable : public RenderableHelper, public ShadowMapped
	{
	public:
		GroundRenderable()
			: RenderableHelper(L"Ground"),
				ShadowMapped(SHADOW_MAP_SIZE),
				lamp_sampler_(new Sampler)
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();

			effect_ = rf.LoadEffect("ShadowCubeMap.fx");
			effect_->ActiveTechnique("RenderScene");

			Vector3 xyzs[] =
			{
				Vector3(-1, 0, 1),
				Vector3(1, 0, 1),
				Vector3(1, 0, -1),
				Vector3(-1, 0, -1),
			};

			uint16_t indices[] = 
			{
				0, 1, 2, 2, 3, 0
			};

			vb_ = rf.MakeVertexBuffer(VertexBuffer::BT_TriangleList);

			VertexStreamPtr pos_vs = rf.MakeVertexStream(boost::make_tuple(vertex_element(VEU_Position, 0, sizeof(float), 3)), true);
			pos_vs->Assign(xyzs, sizeof(xyzs) / sizeof(xyzs[0]));
			vb_->AddVertexStream(pos_vs);

			IndexStreamPtr is = rf.MakeIndexStream(true);
			is->Assign(indices, sizeof(indices) / sizeof(uint16_t));
			vb_->SetIndexStream(is);

			Vector3 normal[sizeof(xyzs) / sizeof(xyzs[0])];
			MathLib::ComputeNormal<float>(&normal[0],
				&indices[0], &indices[sizeof(indices) / sizeof(uint16_t)],
				&xyzs[0], &xyzs[sizeof(xyzs) / sizeof(xyzs[0])]);

			VertexStreamPtr normal_vs = rf.MakeVertexStream(boost::make_tuple(vertex_element(VEU_Normal, 0, sizeof(float), 3)), true);
			normal_vs->Assign(normal, sizeof(normal) / sizeof(normal[0]));
			vb_->AddVertexStream(normal_vs);

			box_ = MathLib::ComputeBoundingBox<float>(&xyzs[0], &xyzs[4]);
		}

		void SetModelMatrix(Matrix4 const & model)
		{
			model_ = model;
		}

		void OnRenderBegin()
		{
			App3DFramework const & app = Context::Instance().AppInstance();

			Matrix4 view = app.ActiveCamera().ViewMatrix();
			Matrix4 proj = app.ActiveCamera().ProjMatrix();
			Matrix4 light_mat = model_ * this->LightViewProj();

			*(effect_->ParameterByName("World")) = model_;
			*(effect_->ParameterByName("InvLightWorld")) = inv_light_model_;

			if (gen_sm_pass_)
			{
				effect_->ActiveTechnique("GenShadowMap");
				*(effect_->ParameterByName("WorldViewProj")) = light_mat;
			}
			else
			{
				effect_->ActiveTechnique("RenderScene");

				*(effect_->ParameterByName("LampSampler")) = lamp_sampler_;
				*(effect_->ParameterByName("ShadowMapSampler")) = sm_sampler_;
				*(effect_->ParameterByName("WorldViewProj")) = model_ * view * proj;
				*(effect_->ParameterByName("light_pos")) = light_pos_;
			}
		}

		void LampTexture(TexturePtr tex)
		{
			lamp_sampler_->SetTexture(tex);
			lamp_sampler_->Filtering(Sampler::TFO_Bilinear);
			lamp_sampler_->AddressingMode(Sampler::TAT_Addr_U, Sampler::TAM_Wrap);
			lamp_sampler_->AddressingMode(Sampler::TAT_Addr_V, Sampler::TAM_Wrap);
		}

	private:
		SamplerPtr lamp_sampler_;
		Matrix4 model_;
	};

	class GroundObject : public SceneObjectHelper
	{
	public:
		GroundObject()
			: SceneObjectHelper(RenderablePtr(new GroundRenderable), SOA_Cullable)
		{
			model_ = MathLib::Translation(0.0f, -0.2f, 0.0f);

			checked_cast<GroundRenderable*>(renderable_.get())->SetModelMatrix(model_);
		}

	private:
		Matrix4 model_;
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
	settings.colorDepth = 32;
	settings.fullScreen = false;
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
	// ½¨Á¢×ÖÌå
	font_ = Context::Instance().RenderFactoryInstance().MakeFont("gkai00mp.ttf", 16);

	ground_.reset(new GroundObject);
	ground_->AddToSceneManager();

	mesh_.reset(new OccluderObject);
	mesh_->AddToSceneManager();

	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

	renderEngine.ClearColor(Color(0.2f, 0.4f, 0.6f, 1));

	this->LookAt(Vector3(2, 0, -1), Vector3(0, 0, 0));
	this->Proj(0.01f, 100);

	lamp_tex_ = LoadTexture("lamp.dds");

	checked_cast<OccluderRenderable*>(mesh_->GetRenderable().get())->LampTexture(lamp_tex_);
	checked_cast<GroundRenderable*>(ground_->GetRenderable().get())->LampTexture(lamp_tex_);

	shadow_tex_ = Context::Instance().RenderFactoryInstance().MakeTextureCube(SHADOW_MAP_SIZE, 1, PF_R32F);
	shadow_buffer_ = Context::Instance().RenderFactoryInstance().MakeRenderTexture();

	screen_buffer_ = renderEngine.ActiveRenderTarget(0);


	fpcController_.AttachCamera(this->ActiveCamera());
	fpcController_.Scalers(0.05f, 0.1f);

	InputEngine& inputEngine(Context::Instance().InputFactoryInstance().InputEngineInstance());
	KlayGE::InputActionMap actionMap;
	actionMap.AddActions(actions, actions + sizeof(actions) / sizeof(actions[0]));

	action_handler_t input_handler(inputEngine);
	input_handler += boost::bind(&ShadowCubeMap::InputHandler, this, _1, _2);
	inputEngine.ActionMap(actionMap, input_handler, true);
}

void ShadowCubeMap::InputHandler(InputEngine const & sender, InputAction const & action)
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
	SceneManager& sceneMgr = Context::Instance().SceneManagerInstance();

	switch (pass)
	{
	case 0:
		{
			fpcController_.Update();

			light_model_ = MathLib::RotationZ(0.4f) * MathLib::RotationY(std::clock() / 700.0f)
				* MathLib::Translation(0.1f, 0.7f, 0.2f);
		}

	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
		{
			Texture::CubeFaces face = static_cast<Texture::CubeFaces>(Texture::CF_Positive_X + pass);

			std::pair<Vector3, Vector3> lookat_up = CubeMapViewVector<float>(face);

			Vector3 le = TransformCoord(Vector3(0, 0, 0), light_model_);
			Vector3 lla = TransformCoord(Vector3(0, 0, 0) + lookat_up.first, light_model_);
			Vector3 lu = TransformNormal(Vector3(0, 0, 0) + lookat_up.second, light_model_);

			Matrix4 light_view = LookAtLH(le, lla, lu);
			Matrix4 light_proj = PerspectiveFovLH(PI / 2.0f, 1.0f, 0.01f, 10.0f);
			checked_cast<OccluderRenderable*>(mesh_->GetRenderable().get())->LightMatrices(light_model_, light_view, light_proj);
			checked_cast<GroundRenderable*>(ground_->GetRenderable().get())->LightMatrices(light_model_, light_view, light_proj);

			checked_cast<OccluderRenderable*>(mesh_->GetRenderable().get())->GenShadowMapPass(true);
			checked_cast<GroundRenderable*>(ground_->GetRenderable().get())->GenShadowMapPass(true);

			shadow_buffer_->AttachTextureCube(shadow_tex_, face);
			renderEngine.ActiveRenderTarget(0, shadow_buffer_);
		}
		break;

	case 6:
		{
			renderEngine.ActiveRenderTarget(0, screen_buffer_);

			//SaveToFile(shadow_tex_, "shadow_tex.dds");

			checked_cast<OccluderRenderable*>(mesh_->GetRenderable().get())->ShadowMapTexture(shadow_tex_);
			checked_cast<GroundRenderable*>(ground_->GetRenderable().get())->ShadowMapTexture(shadow_tex_);

			checked_cast<OccluderRenderable*>(mesh_->GetRenderable().get())->GenShadowMapPass(false);
			checked_cast<GroundRenderable*>(ground_->GetRenderable().get())->GenShadowMapPass(false);

			std::wostringstream stream;
			stream << renderEngine.ActiveRenderTarget(0)->FPS();

			font_->RenderText(0, 0, Color(1, 1, 0, 1), L"ShadowCubeMap");
			font_->RenderText(0, 18, Color(1, 1, 0, 1), stream.str().c_str());
		}
		break;
	}
}
