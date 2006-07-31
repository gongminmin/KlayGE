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
#include <KlayGE/PostProcess.hpp>

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
			checked_cast<HDRSkyBox*>(renderable_.get())->CompressedCubeMap(y_cube, c_cube);
		}
	};

	class RefractorRenderable : public KMesh
	{
	public:
		RefractorRenderable(std::wstring const & /*name*/, TexturePtr tex)
			: KMesh(L"Refractor", tex),
				y_sampler_(new Sampler), c_sampler_(new Sampler)
		{
			technique_ = Context::Instance().RenderFactoryInstance().LoadEffect("Refract.fx")->Technique("Refract");

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

		void OnRenderBegin()
		{
			App3DFramework const & app = Context::Instance().AppInstance();

			float4x4 const & model = float4x4::Identity();
			float4x4 const & view = app.ActiveCamera().ViewMatrix();
			float4x4 const & proj = app.ActiveCamera().ProjMatrix();

			*(technique_->Effect().ParameterByName("model")) = model;
			*(technique_->Effect().ParameterByName("modelit")) = MathLib::transpose(MathLib::inverse(model));
			*(technique_->Effect().ParameterByName("mvp")) = model * view * proj;

			*(technique_->Effect().ParameterByName("eta_ratio")) = float3(1 / 1.1f, 1 / 1.1f - 0.003f, 1 / 1.1f - 0.006f);

			*(technique_->Effect().ParameterByName("eyePos")) = Context::Instance().AppInstance().ActiveCamera().EyePos();
		}

	private:
		SamplerPtr y_sampler_;
		SamplerPtr c_sampler_;
	};

	class RefractorObject : public SceneObjectHelper
	{
	public:
		RefractorObject(TexturePtr const & y_cube, TexturePtr const & c_cube)
			: SceneObjectHelper(SOA_Cullable)
		{
			renderable_ = LoadKMesh("teapot.kmesh", CreateKMeshFactory<RefractorRenderable>())->Mesh(0);
			checked_cast<RefractorRenderable*>(renderable_.get())->CompressedCubeMap(y_cube, c_cube);	
		}
	};


	int const NUM_TONEMAP_TEXTURES = 3;
	
	class RenderDownsampler : public PostProcess
	{
	public:
		RenderDownsampler()
			: PostProcess(Context::Instance().RenderFactoryInstance().LoadEffect("Downsample.fx")->Technique("Downsample"))
		{
		}
	};

	class RenderBlur : public PostProcess
	{
	public:
		explicit RenderBlur(std::string const & tech)
			: PostProcess(Context::Instance().RenderFactoryInstance().LoadEffect("Blur.fx")->Technique(tech)),
				color_weight_(15), tex_coord_offset_(15)
		{
		}

		virtual ~RenderBlur() = 0
		{
		}

		void OnRenderBegin()
		{
			PostProcess::OnRenderBegin();

			this->CalSampleOffsets(7, src_sampler_->GetTexture()->Width(0), 3, 2);

			*(technique_->Effect().ParameterByName("color_weight")) = color_weight_;
			*(technique_->Effect().ParameterByName("tex_coord_offset")) = tex_coord_offset_;
		}

	private:
		float GaussianDistribution(float x, float y, float rho)
		{
			float g = 1.0f / sqrt(2.0f * PI * rho * rho);
			g *= exp(-(x * x + y * y) / (2 * rho * rho));
			return g;
		}

		void CalSampleOffsets(int length, uint32_t tex_size,
								float deviation, float multiplier)
		{
			color_weight_.resize(length * 2 + 1);
			tex_coord_offset_.resize(length * 2 + 1);

			float tu = 1.0f / tex_size;

			// Fill the center texel
			float weight = 1.0f * this->GaussianDistribution(0, 0, deviation);
			color_weight_[0] = 0;
			tex_coord_offset_[0] = 0.0f;
		    
			// Fill the right side
			for (int i = 1; i < length + 1; ++ i)
			{
				weight = multiplier * this->GaussianDistribution(float(i), 0, deviation);
				color_weight_[i] = weight;

				tex_coord_offset_[i] = float(i) * tu;
			}

			// Copy to the left side
			for (int i = length; i < 2 * length + 1; ++ i)
			{
				color_weight_[i] = color_weight_[i - length];

				tex_coord_offset_[i] = -tex_coord_offset_[i - length];
			}
		}

	private:
		std::vector<float> color_weight_;
		std::vector<float> tex_coord_offset_;
	};

	class RenderBlurX : public RenderBlur
	{
	public:
		RenderBlurX()
			: RenderBlur("BlurX")
		{
		}
	};

	class RenderBlurY : public RenderBlur
	{
	public:
		RenderBlurY()
			: RenderBlur("BlurY")
		{
		}
	};

	class RenderSumLum : public PostProcess
	{
	public:
		explicit RenderSumLum(std::string const & tech)
			: PostProcess(Context::Instance().RenderFactoryInstance().LoadEffect("SumLum.fx")->Technique(tech))
		{
		}

		virtual ~RenderSumLum() = 0
		{
		}

		void Source(TexturePtr const & src_tex, Sampler::TexFilterOp filter)
		{
			PostProcess::Source(src_tex, filter);

			this->GetSampleOffsets4x4(src_tex->Width(0), src_tex->Height(0));
		}

		void OnRenderBegin()
		{
			PostProcess::OnRenderBegin();

			*(technique_->Effect().ParameterByName("tex_coord_offset")) = tex_coord_offset_;
		}

	private:
		void GetSampleOffsets4x4(uint32_t width, uint32_t height)
		{
			tex_coord_offset_.resize(2);

			float tu = 1.0f / width;
			float tv = 1.0f / height;

			// Sample from the 16 surrounding points. 
			int index = 0;
			for (int y = -1; y <= 2; y += 2)
			{
				for (int x = -1; x <= 2; x += 4)
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

	class RenderSumLumLog : public RenderSumLum
	{
	public:
		RenderSumLumLog()
			: RenderSumLum("SumLumLog")
		{
		}
	};

	class RenderSumLumIterative : public RenderSumLum
	{
	public:
		RenderSumLumIterative()
			: RenderSumLum("SumLumIterative")
		{
		}
	};

	class RenderSumLumExp : public RenderSumLum
	{
	public:
		RenderSumLumExp()
			: RenderSumLum("SumLumExp")
		{
		}
	};

	class RenderAdaptedLum : public PostProcess
	{
	public:
		RenderAdaptedLum()
			: PostProcess(Context::Instance().RenderFactoryInstance().LoadEffect("SumLum.fx")->Technique("AdaptedLum")),
				last_index_(false)
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();

			for (int i = 0; i < 2; ++ i)
			{
				adapted_samplers_[i].reset(new Sampler);
				adapted_samplers_[i]->AddressingMode(Sampler::TAT_Addr_U, Sampler::TAM_Clamp);
				adapted_samplers_[i]->AddressingMode(Sampler::TAT_Addr_V, Sampler::TAM_Clamp);
				adapted_samplers_[i]->Filtering(Sampler::TFO_Point);

				TexturePtr tex = Context::Instance().RenderFactoryInstance().MakeTexture2D(1, 1, 1, EF_R32F);
				float data = 0;
				tex->CopyMemoryToTexture2D(0, &data, EF_R32F, 1, 1, 0, 0, 1, 1);
				adapted_samplers_[i]->SetTexture(tex);

				fb_[i] = rf.MakeFrameBuffer();
				fb_[i]->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*adapted_samplers_[i]->GetTexture(), 0));
			}

			this->Destinate(fb_[last_index_]);
		}

		void Apply()
		{
			this->Destinate(fb_[!last_index_]);

			PostProcess::Apply();
		}

		void OnRenderBegin()
		{
			PostProcess::OnRenderBegin();

			*(technique_->Effect().ParameterByName("last_lum_sampler")) = adapted_samplers_[last_index_];
			*(technique_->Effect().ParameterByName("frame_delta")) = float(timer_.elapsed());
			timer_.restart();

			last_index_ = !last_index_;
		}

		TexturePtr AdaptedLum() const
		{
			return adapted_samplers_[last_index_]->GetTexture();
		}

	private:
		KlayGE::FrameBufferPtr fb_[2];
		KlayGE::SamplerPtr adapted_samplers_[2];
		bool last_index_;

		KlayGE::Timer timer_;
	};

	class RenderToneMapping : public PostProcess
	{
	public:
		RenderToneMapping()
			: PostProcess(Context::Instance().RenderFactoryInstance().LoadEffect("ToneMapping.fx")->Technique("ToneMapping")),
				lum_sampler_(new Sampler), bloom_sampler_(new Sampler)
		{
			lum_sampler_->Filtering(Sampler::TFO_Point);
			lum_sampler_->AddressingMode(Sampler::TAT_Addr_U, Sampler::TAM_Clamp);
			lum_sampler_->AddressingMode(Sampler::TAT_Addr_V, Sampler::TAM_Clamp);

			bloom_sampler_->Filtering(Sampler::TFO_Bilinear);
			bloom_sampler_->AddressingMode(Sampler::TAT_Addr_U, Sampler::TAM_Clamp);
			bloom_sampler_->AddressingMode(Sampler::TAT_Addr_V, Sampler::TAM_Clamp);
		}

		void SetTexture(TexturePtr const & lum_tex, TexturePtr const & bloom_tex)
		{
			lum_sampler_->SetTexture(lum_tex);
			bloom_sampler_->SetTexture(bloom_tex);
		}

		void OnRenderBegin()
		{
			PostProcess::OnRenderBegin();

			*(technique_->Effect().ParameterByName("lum_sampler")) = lum_sampler_;
			*(technique_->Effect().ParameterByName("bloom_sampler")) = bloom_sampler_;
		}

	private:
		SamplerPtr lum_sampler_;
		SamplerPtr bloom_sampler_;
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
	refractor_->AddToSceneManager();

	sky_box_.reset(new HDRSceneObjectSkyBox);
	checked_cast<HDRSceneObjectSkyBox*>(sky_box_.get())->CompressedCubeMap(y_cube_map_, c_cube_map_);
	sky_box_->AddToSceneManager();

	RenderFactory& rf = Context::Instance().RenderFactoryInstance();
	RenderEngine& re = rf.RenderEngineInstance();

	re.ClearColor(Color(0.2f, 0.4f, 0.6f, 1));

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

	render_buffer_ = rf.MakeFrameBuffer();
	RenderTargetPtr screen_buffer = re.CurRenderTarget();

	render_buffer_->GetViewport().camera = screen_buffer->GetViewport().camera;


	renderToneMapping_.reset(new RenderToneMapping);
	renderDownsampler_.reset(new RenderDownsampler);
	renderBlurX_.reset(new RenderBlurX);
	renderBlurY_.reset(new RenderBlurY);
	renderSumLums_.resize(NUM_TONEMAP_TEXTURES + 1);
	renderSumLums_[0].reset(new RenderSumLumLog);
	for (int i = 1; i < NUM_TONEMAP_TEXTURES; ++ i)
	{
		renderSumLums_[i].reset(new RenderSumLumIterative);
	}
	renderSumLums_[NUM_TONEMAP_TEXTURES].reset(new RenderSumLumExp);
	renderAdaptedLum_.reset(new RenderAdaptedLum);
}

void Refract::OnResize(uint32_t width, uint32_t height)
{
	RenderFactory& rf = Context::Instance().RenderFactoryInstance();

	rendered_tex_ = rf.MakeTexture2D(width, height, 1, EF_ABGR16F);
	render_buffer_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*rendered_tex_, 0));
	render_buffer_->Attach(FrameBuffer::ATT_DepthStencil, rf.MakeDepthStencilRenderView(width, height, EF_D16, 0));

	downsample_tex_ = rf.MakeTexture2D(width / 2, height / 2, 1, EF_ABGR16F);
	blurx_tex_ = rf.MakeTexture2D(width / 4, height / 4, 1, EF_ABGR16F);
	blury_tex_ = rf.MakeTexture2D(width / 4, height / 4, 1, EF_ABGR16F);

	lum_texs_.clear();
	int len = 4;
	for (int i = 0; i < NUM_TONEMAP_TEXTURES; ++ i)
	{
		lum_texs_.push_back(rf.MakeTexture2D(len, len, 1, EF_GR16F));
		len *= 4;
	}
	std::reverse(lum_texs_.begin(), lum_texs_.end());
	lum_exp_tex_ = rf.MakeTexture2D(1, 1, 1, EF_R32F);

	{
		RenderDownsampler* ppor = checked_cast<RenderDownsampler*>(renderDownsampler_.get());
		ppor->Source(rendered_tex_, Sampler::TFO_Bilinear);
		FrameBufferPtr fb = rf.MakeFrameBuffer();
		fb->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*downsample_tex_, 0));
		ppor->Destinate(fb);
	}

	{
		RenderBlurX* ppor = checked_cast<RenderBlurX*>(renderBlurX_.get());
		ppor->Source(downsample_tex_, Sampler::TFO_Bilinear);
		FrameBufferPtr fb = rf.MakeFrameBuffer();
		fb->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*blurx_tex_, 0));
		ppor->Destinate(fb);
	}
	{
		RenderBlurY* ppor = checked_cast<RenderBlurY*>(renderBlurY_.get());
		ppor->Source(blurx_tex_, Sampler::TFO_Bilinear);
		FrameBufferPtr fb = rf.MakeFrameBuffer();
		fb->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*blury_tex_, 0));
		ppor->Destinate(fb);
	}

	{
		RenderSumLumLog* ppor = checked_cast<RenderSumLumLog*>(renderSumLums_[0].get());
		ppor->Source(rendered_tex_, Sampler::TFO_Bilinear);
		FrameBufferPtr fb = rf.MakeFrameBuffer();
		fb->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*lum_texs_[0], 0));
		ppor->Destinate(fb);
	}
	for (int i = 1; i < NUM_TONEMAP_TEXTURES; ++ i)
	{
		RenderSumLumIterative* ppor = checked_cast<RenderSumLumIterative*>(renderSumLums_[i].get());
		ppor->Source(lum_texs_[i - 1], Sampler::TFO_Bilinear);
		FrameBufferPtr fb = rf.MakeFrameBuffer();
		fb->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*lum_texs_[i], 0));
		ppor->Destinate(fb);
	}
	{
		RenderSumLumExp* ppor = checked_cast<RenderSumLumExp*>(renderSumLums_[NUM_TONEMAP_TEXTURES].get());
		ppor->Source(lum_texs_[NUM_TONEMAP_TEXTURES - 1], Sampler::TFO_Bilinear);
		FrameBufferPtr fb = rf.MakeFrameBuffer();
		fb->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*lum_exp_tex_, 0));
		ppor->Destinate(fb);
	}

	{
		RenderAdaptedLum* ppor = checked_cast<RenderAdaptedLum*>(renderAdaptedLum_.get());
		ppor->Source(lum_exp_tex_, Sampler::TFO_Point);
	}

	{
		RenderToneMapping* ppor = checked_cast<RenderToneMapping*>(renderToneMapping_.get());
		ppor->Source(rendered_tex_, Sampler::TFO_Bilinear);
		ppor->Destinate(RenderTargetPtr());
	}
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

		// 第一遍，正常渲染
		re.BindRenderTarget(render_buffer_);
		re.Clear(RenderEngine::CBM_Color | RenderEngine::CBM_Depth);

		*(refractor_->GetRenderable()->GetRenderTechnique()->Effect().ParameterByName("eyePos")) = this->ActiveCamera().EyePos();
		refractor_->AddToSceneManager();

		sky_box_->AddToSceneManager();
		break;

	case 1:
		sm.Clear();
		
		{
			// 降采样
			RenderDownsampler* ppor = checked_cast<RenderDownsampler*>(renderDownsampler_.get());
			ppor->Apply();
		}
		{
			// Blur X
			RenderBlurX* ppor = checked_cast<RenderBlurX*>(renderBlurX_.get());
			ppor->Apply();
		}
		{
			// Blur Y
			RenderBlurY* ppor = checked_cast<RenderBlurY*>(renderBlurY_.get());
			ppor->Apply();
		}

		{
			// 降采样4x4 log
			RenderSumLum* ppor = checked_cast<RenderSumLum*>(renderSumLums_[0].get());
			ppor->Apply();
		}
		for (size_t i = 1; i < renderSumLums_.size() - 1; ++ i)
		{
			// 降采样4x4
			RenderSumLum* ppor = checked_cast<RenderSumLum*>(renderSumLums_[i].get());
			ppor->Apply();
		}
		{
			// 降采样4x4 exp
			RenderSumLum* ppor = checked_cast<RenderSumLum*>(renderSumLums_[renderSumLums_.size() - 1].get());
			ppor->Apply();
		}
		{
			RenderAdaptedLum* ppor = checked_cast<RenderAdaptedLum*>(renderAdaptedLum_.get());
			ppor->Apply();
		}
		break;

	case 2:
		re.BindRenderTarget(RenderTargetPtr());
		re.Clear(RenderEngine::CBM_Depth);

		{
			// Tone mapping
			RenderToneMapping* ppor = checked_cast<RenderToneMapping*>(renderToneMapping_.get());
			ppor->SetTexture(checked_cast<RenderAdaptedLum*>(renderAdaptedLum_.get())->AdaptedLum(), blury_tex_);
			ppor->Apply();
		}

		std::wostringstream stream;
		stream << this->FPS();

		font_->RenderText(0, 0, Color(1, 1, 0, 1), L"HDR Refract");
		font_->RenderText(0, 18, Color(1, 1, 0, 1), stream.str().c_str());
		break;
	}
}
