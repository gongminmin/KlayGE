#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/Renderable.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/SceneManager.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/Texture.hpp>
#include <KlayGE/RenderableHelper.hpp>
#include <KlayGE/RenderSettings.hpp>
#include <KlayGE/KMesh.hpp>
#include <KlayGE/Sampler.hpp>
#include <KlayGE/SceneObjectHelper.hpp>
#include <KlayGE/HDRPostProcess.hpp>

#include <KlayGE/D3D9/D3D9RenderFactory.hpp>

#include <KlayGE/OCTree/OCTree.hpp>

#include <KlayGE/Input.hpp>
#include <KlayGE/DInput/DInputFactory.hpp>

#include <vector>
#include <sstream>
#include <ctime>
#include <boost/bind.hpp>

#include "Refract.hpp"

using namespace std;
using namespace KlayGE;

namespace
{
	class HDRSkyBox : public RenderableSkyBox
	{
	public:
		HDRSkyBox()
			: y_sampler_(new Sampler), c_sampler_(new Sampler)
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();
			technique_ = rf.LoadEffect("HDRSkyBox.fx")->Technique("HDRSkyBoxTec");

			y_sampler_->Filtering(Sampler::TFO_Bilinear);
			y_sampler_->AddressingMode(Sampler::TAT_Addr_U, Sampler::TAM_Clamp);
			y_sampler_->AddressingMode(Sampler::TAT_Addr_V, Sampler::TAM_Clamp);
			y_sampler_->AddressingMode(Sampler::TAT_Addr_W, Sampler::TAM_Clamp);
			*(technique_->Effect().ParameterByName("skybox_YcubeMapSampler")) = y_sampler_;

			c_sampler_->Filtering(Sampler::TFO_Bilinear);
			c_sampler_->AddressingMode(Sampler::TAT_Addr_U, Sampler::TAM_Clamp);
			c_sampler_->AddressingMode(Sampler::TAT_Addr_V, Sampler::TAM_Clamp);
			c_sampler_->AddressingMode(Sampler::TAT_Addr_W, Sampler::TAM_Clamp);
			*(technique_->Effect().ParameterByName("skybox_CcubeMapSampler")) = c_sampler_;
		}

		void CompressedCubeMap(TexturePtr const & y_cube, TexturePtr const & c_cube)
		{
			y_sampler_->SetTexture(y_cube);
			c_sampler_->SetTexture(c_cube);
		}

	private:
		SamplerPtr y_sampler_;
		SamplerPtr c_sampler_;
	};

	class HDRSceneObjectSkyBox : public SceneObjectSkyBox
	{
	public:
		HDRSceneObjectSkyBox()
		{
			renderable_.reset(new HDRSkyBox);
		}

		void CompressedCubeMap(TexturePtr const & y_cube, TexturePtr const & c_cube)
		{
			checked_pointer_cast<HDRSkyBox>(renderable_)->CompressedCubeMap(y_cube, c_cube);
		}
	};

	class RefractorRenderable : public KMesh
	{
	public:
		RefractorRenderable(RenderModelPtr model, std::wstring const & /*name*/)
			: KMesh(model, L"Refractor"),
				y_sampler_(new Sampler), c_sampler_(new Sampler),
				bf_sampler_(new Sampler)
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();

			RenderEffectPtr effect = rf.LoadEffect("Refract.fx");
			back_face_tech_ = effect->Technique("RefractBackFace");

			if (!effect->ValidateTechnique("Refract2x"))
			{
				front_face_tech_ = effect->Technique("Refract20");
			}
			else
			{
				front_face_tech_ = effect->Technique("Refract2x");
			}

			technique_ = back_face_tech_;

			y_sampler_->Filtering(Sampler::TFO_Bilinear);
			y_sampler_->AddressingMode(Sampler::TAT_Addr_U, Sampler::TAM_Clamp);
			y_sampler_->AddressingMode(Sampler::TAT_Addr_V, Sampler::TAM_Clamp);
			y_sampler_->AddressingMode(Sampler::TAT_Addr_W, Sampler::TAM_Clamp);
			*(technique_->Effect().ParameterByName("skybox_YcubeMapSampler")) = y_sampler_;

			c_sampler_->Filtering(Sampler::TFO_Bilinear);
			c_sampler_->AddressingMode(Sampler::TAT_Addr_U, Sampler::TAM_Clamp);
			c_sampler_->AddressingMode(Sampler::TAT_Addr_V, Sampler::TAM_Clamp);
			c_sampler_->AddressingMode(Sampler::TAT_Addr_W, Sampler::TAM_Clamp);
			*(technique_->Effect().ParameterByName("skybox_CcubeMapSampler")) = c_sampler_;

			bf_sampler_->Filtering(Sampler::TFO_Bilinear);
			bf_sampler_->AddressingMode(Sampler::TAT_Addr_U, Sampler::TAM_Clamp);
			bf_sampler_->AddressingMode(Sampler::TAT_Addr_V, Sampler::TAM_Clamp);
			*(technique_->Effect().ParameterByName("BackFace_Sampler")) = bf_sampler_;
		}

		void BuildMeshInfo()
		{
		}

		void Pass(int pass)
		{
			switch (pass)
			{
			case 0:
				technique_ = back_face_tech_;
				break;

			case 1:
				technique_ = front_face_tech_;
				break;

			default:
				BOOST_ASSERT(false);
				break;
			}
		}

		void BackFaceTexture(TexturePtr const & bf_tex)
		{
			bf_sampler_->SetTexture(bf_tex);
		}

		void CompressedCubeMap(TexturePtr const & y_cube, TexturePtr const & c_cube)
		{
			y_sampler_->SetTexture(y_cube);
			c_sampler_->SetTexture(c_cube);
		}

		void OnRenderBegin()
		{
			App3DFramework const & app = Context::Instance().AppInstance();

			float4x4 const & model = float4x4::Identity();
			float4x4 const & view = app.ActiveCamera().ViewMatrix();
			float4x4 const & proj = app.ActiveCamera().ProjMatrix();

			*(technique_->Effect().ParameterByName("model")) = model;
			*(technique_->Effect().ParameterByName("modelit")) = MathLib::transpose(MathLib::inverse(model));
			*(technique_->Effect().ParameterByName("mvp")) = model * view * proj;
			*(technique_->Effect().ParameterByName("eyePos")) = app.ActiveCamera().EyePos();

			*(technique_->Effect().ParameterByName("inv_vp")) = MathLib::inverse(view * proj);

			*(technique_->Effect().ParameterByName("eta_ratio")) = float3(1 / 1.1f, 1 / 1.1f - 0.003f, 1 / 1.1f - 0.006f);
		}

	private:
		SamplerPtr y_sampler_;
		SamplerPtr c_sampler_;

		SamplerPtr bf_sampler_;

		RenderTechniquePtr back_face_tech_;
		RenderTechniquePtr front_face_tech_;
	};

	class RefractorObject : public SceneObjectHelper
	{
	public:
		RefractorObject(TexturePtr const & y_cube, TexturePtr const & c_cube)
			: SceneObjectHelper(SOA_Cullable)
		{
			renderable_ = LoadKModel("teapot.kmodel", CreateKModelFactory<RenderModel>(), CreateKMeshFactory<RefractorRenderable>())->Mesh(0);
			checked_pointer_cast<RefractorRenderable>(renderable_)->CompressedCubeMap(y_cube, c_cube);	
		}

		void Pass(int pass)
		{
			checked_pointer_cast<RefractorRenderable>(renderable_)->Pass(pass);
		}

		void BackFaceTexture(TexturePtr const & bf_tex)
		{
			checked_pointer_cast<RefractorRenderable>(renderable_)->BackFaceTexture(bf_tex);
		}
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

	Refract app;
	app.Create("Refract", settings);
	app.Run();

	return 0;
}

Refract::Refract()
{
	ResLoader::Instance().AddPath("../media/Common");
	ResLoader::Instance().AddPath("../media/Refract");
}

void Refract::InitObjects()
{
	// 建立字体
	font_ = Context::Instance().RenderFactoryInstance().MakeFont("gkai00mp.ttf", 16);

	y_cube_map_ = LoadTexture("uffizi_cross_y.dds");
	c_cube_map_ = LoadTexture("uffizi_cross_c.dds");

	refractor_.reset(new RefractorObject(y_cube_map_, c_cube_map_));

	sky_box_.reset(new HDRSceneObjectSkyBox);
	checked_pointer_cast<HDRSceneObjectSkyBox>(sky_box_)->CompressedCubeMap(y_cube_map_, c_cube_map_);

	RenderFactory& rf = Context::Instance().RenderFactoryInstance();
	RenderEngine& re = rf.RenderEngineInstance();

	re.Clear(RenderEngine::CBM_Depth, Color(0.2f, 0.4f, 0.6f, 1), 1, 0);

	this->LookAt(float3(-0.05f, -0.01f, -0.5f), float3(0, 0.05f, 0));
	this->Proj(0.05f, 100);

	fpcController_.AttachCamera(this->ActiveCamera());
	fpcController_.Scalers(0.05f, 0.05f);

	InputEngine& inputEngine(Context::Instance().InputFactoryInstance().InputEngineInstance());
	InputActionMap actionMap;
	actionMap.AddActions(actions, actions + sizeof(actions) / sizeof(actions[0]));

	action_handler_t input_handler(inputEngine);
	input_handler += boost::bind(&Refract::InputHandler, this, _1, _2);
	inputEngine.ActionMap(actionMap, input_handler, true);

	back_face_buffer_ = rf.MakeFrameBuffer();
	render_buffer_ = rf.MakeFrameBuffer();
	RenderTargetPtr screen_buffer = re.CurRenderTarget();
	back_face_buffer_->GetViewport().camera = render_buffer_->GetViewport().camera = screen_buffer->GetViewport().camera;

	hdr_.reset(new HDRPostProcess);
	hdr_->Destinate(RenderTargetPtr());
}

void Refract::OnResize(uint32_t width, uint32_t height)
{
	RenderFactory& rf = Context::Instance().RenderFactoryInstance();

	back_face_tex_ = rf.MakeTexture2D(width, height, 1, EF_ARGB8);
	back_face_buffer_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*back_face_tex_, 0));
	back_face_buffer_->Attach(FrameBuffer::ATT_DepthStencil, rf.MakeDepthStencilRenderView(width, height, EF_D16, 0));

	rendered_tex_ = rf.MakeTexture2D(width, height, 1, EF_ABGR16F);
	render_buffer_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*rendered_tex_, 0));
	render_buffer_->Attach(FrameBuffer::ATT_DepthStencil, rf.MakeDepthStencilRenderView(width, height, EF_D16, 0));

	hdr_->Source(rendered_tex_, Sampler::TFO_Bilinear, Sampler::TAM_Clamp);
}

void Refract::InputHandler(InputEngine const & /*sender*/, InputAction const & action)
{
	switch (action.first)
	{
	case Exit:
		this->Quit();
		break;
	}
}

uint32_t Refract::NumPasses() const
{
	return 3;
}

void Refract::DoUpdate(uint32_t pass)
{
	RenderEngine& re(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
	SceneManager& sm(Context::Instance().SceneManagerInstance());

	switch (pass)
	{
	case 0:
		fpcController_.Update();

		// 第一遍，渲染背面
		re.BindRenderTarget(back_face_buffer_);
		re.Clear(RenderEngine::CBM_Color | RenderEngine::CBM_Depth, Color(0, 0, 0, 1), 0, 0);

		checked_pointer_cast<RefractorObject>(refractor_)->Pass(0);
		refractor_->AddToSceneManager();
		break;

	case 1:
		// 第二遍，渲染正面
		re.BindRenderTarget(render_buffer_);
		re.Clear(RenderEngine::CBM_Color | RenderEngine::CBM_Depth, Color(0.2f, 0.4f, 0.6f, 1), 1, 0);

		checked_pointer_cast<RefractorObject>(refractor_)->Pass(1);
		checked_pointer_cast<RefractorObject>(refractor_)->BackFaceTexture(back_face_tex_);

		sky_box_->AddToSceneManager();
		break;

	case 2:
		sm.Clear();

		hdr_->Apply();

		re.BindRenderTarget(RenderTargetPtr());

		std::wostringstream stream;
		stream << this->FPS();

		font_->RenderText(0, 0, Color(1, 1, 0, 1), L"HDR Refract");
		font_->RenderText(0, 18, Color(1, 1, 0, 1), stream.str().c_str());
		break;
	}
}
