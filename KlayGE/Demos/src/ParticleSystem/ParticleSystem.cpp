#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/Util.hpp>
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
#include <KlayGE/RenderableHelper.hpp>
#include <KlayGE/SceneObjectHelper.hpp>
#include <KlayGE/Texture.hpp>

#include <KlayGE/Heightmap/Heightmap.hpp>

#include <KlayGE/D3D9/D3D9RenderFactory.hpp>

#include <KlayGE/Input.hpp>
#include <KlayGE/DInput/DInputFactory.hpp>

#include <vector>
#include <sstream>
#include <ctime>
#include <boost/tuple/tuple.hpp>
#pragma warning(push)
#pragma warning(disable: 4127 4512)
#include <boost/random.hpp>
#pragma warning(pop)
#include <boost/bind.hpp>

#include "ParticleSystem.hpp"

using namespace std;
using namespace KlayGE;

namespace
{
	template <typename ParticleType>
	class GenParticle
	{
	public:
		GenParticle()
			: random_gen_(boost::lagged_fibonacci607(), boost::uniform_real<float>(-0.05f, 0.05f))
		{
		}

		void operator()(ParticleType& par, float4x4 const & mat)
		{
			par.pos = MathLib::transform_coord(float3(0, 0, 0), mat);
			par.vel = MathLib::transform_normal(float3(random_gen_(), 0.2f + abs(random_gen_()) * 3, random_gen_()), mat);
			par.life = 10;
		}

	private:
		boost::variate_generator<boost::lagged_fibonacci607, boost::uniform_real<float> > random_gen_;
	};

	template <typename ParticleType>
	class UpdateParticle
	{
	public:
		UpdateParticle(boost::shared_ptr<HeightImg> hm, std::vector<float3> const & normals)
			: hm_(hm), normals_(normals)
		{
		}

		void operator()(ParticleType& par, float elapse_time)
		{
			par.vel += float3(0, -0.1f, 0) * elapse_time;
			par.pos += par.vel * elapse_time;
			par.life -= elapse_time;

			if (par.pos.y() < (*hm_)(par.pos.x(), par.pos.z()))
			{
				par.pos.y() += 0.01f;
				par.vel = MathLib::reflect(par.vel,
					normals_[hm_->GetImgY(par.pos.z()) * hm_->HMWidth() + hm_->GetImgX(par.pos.x())]) * 0.8f;
			}
		}

	private:
		boost::shared_ptr<HeightImg> hm_;
		std::vector<float3> normals_;
	};

	class TerrainRenderable : public RenderableHelper
	{
	public:
		TerrainRenderable(std::vector<float3> const & vertices, std::vector<uint16_t> const & indices)
			: RenderableHelper(L"Terrain")
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();

			technique_ = rf.LoadEffect("ParticleSystem.fx")->TechniqueByName("Terrain");

			rl_ = rf.MakeRenderLayout(RenderLayout::BT_TriangleList);

			GraphicsBufferPtr pos_vb = rf.MakeVertexBuffer(BU_Static);
			pos_vb->Resize(static_cast<uint32_t>(vertices.size() * sizeof(vertices[0])));
			{
				GraphicsBuffer::Mapper mapper(*pos_vb, BA_Write_Only);
				std::copy(&vertices[0], &vertices[0] + vertices.size(), mapper.Pointer<float3>());
			}
			rl_->BindVertexStream(pos_vb, boost::make_tuple(vertex_element(VEU_Position, 0, EF_BGR32F)));

			GraphicsBufferPtr ib = rf.MakeIndexBuffer(BU_Static);
			ib->Resize(static_cast<uint32_t>(indices.size() * sizeof(indices[0])));
			{
				GraphicsBuffer::Mapper mapper(*ib, BA_Write_Only);
				std::copy(&indices[0], &indices[0] + indices.size(), mapper.Pointer<uint16_t>());
			}
			rl_->BindIndexStream(ib, EF_R16);

			std::vector<float3> normal(vertices.size());
			MathLib::compute_normal<float>(&normal[0],
				&indices[0], &indices[0] + indices.size(),
				&vertices[0], &vertices[0] + vertices.size());

			GraphicsBufferPtr normal_vb = rf.MakeVertexBuffer(BU_Static);
			normal_vb->Resize(static_cast<uint32_t>(normal.size() * sizeof(normal[0])));
			{
				GraphicsBuffer::Mapper mapper(*normal_vb, BA_Write_Only);
				std::copy(&normal[0], &normal[0] + normal.size(), mapper.Pointer<float3>());
			}
			rl_->BindVertexStream(normal_vb, boost::make_tuple(vertex_element(VEU_Normal, 0, EF_BGR32F)));

			box_ = MathLib::compute_bounding_box<float>(&vertices[0], &vertices[0] + vertices.size());
		}

		void OnRenderBegin()
		{
			App3DFramework const & app = Context::Instance().AppInstance();

			float4x4 view = app.ActiveCamera().ViewMatrix();
			float4x4 proj = app.ActiveCamera().ProjMatrix();

			*(technique_->Effect().ParameterByName("View")) = view;
			*(technique_->Effect().ParameterByName("Proj")) = proj;
		}

		void DepthPass(bool depth)
		{
			if (depth)
			{
				technique_ = technique_->Effect().TechniqueByName("DepthPass");
			}
			else
			{
				technique_ = technique_->Effect().TechniqueByName("Terrain");
			}
		}

	private:
		float4x4 model_;
	};

	class TerrainObject : public SceneObjectHelper
	{
	public:
		TerrainObject(std::vector<float3> const & vertices, std::vector<uint16_t> const & indices)
			: SceneObjectHelper(RenderablePtr(new TerrainRenderable(vertices, indices)), SOA_Cullable)
		{
		}

		void DepthPass(bool depth)
		{
			checked_pointer_cast<TerrainRenderable>(renderable_)->DepthPass(depth);
		}
	};

	int const NUM_PARTICLE = 8192;

	class PointSpriteObject : public SceneObjectHelper
	{
	public:
		PointSpriteObject()
			: SceneObjectHelper(SOA_Cullable | SOA_ShortAge)
		{
			instance_format_.push_back(vertex_element(VEU_Position, 0, EF_ABGR32F));
		}

		void Instance(Particle const & particle)
		{
			par_info_.x() = particle.pos.x();
			par_info_.y() = particle.pos.y();
			par_info_.z() = particle.pos.z();
			par_info_.w() = particle.life;
		}

		void const * InstanceData() const
		{
			return &par_info_;
		}

		void SetRenderable(RenderablePtr ra)
		{
			renderable_ = ra;
		}

	private:
		float4 par_info_;
	};

	class RenderPointSprite : public RenderableHelper
	{
	public:
		RenderPointSprite()
			: RenderableHelper(L"PointSprite"),
				particle_sampler_(new Sampler),
				depth_sampler_(new Sampler)
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();

			rl_ = rf.MakeRenderLayout(RenderLayout::BT_TriangleFan);
			rl_->BindVertexStream(rf.MakeVertexBuffer(BU_Static), boost::make_tuple(vertex_element(VEU_TextureCoord, 0, EF_GR32F)));
			rl_->BindIndexStream(rf.MakeIndexBuffer(BU_Static), EF_R16);

			float2 texs[] =
			{
				float2(0.0f, 0.0f),
				float2(1.0f, 0.0f),
				float2(1.0f, 1.0f),
				float2(0.0f, 1.0f),
			};

			uint16_t indices[] =
			{
				0, 1, 2, 3,
			};

			rl_->GetVertexStream(0)->Resize(sizeof(texs));
			{
				GraphicsBuffer::Mapper mapper(*rl_->GetVertexStream(0), BA_Write_Only);
				std::copy(&texs[0], &texs[4], mapper.Pointer<float2>());
			}

			rl_->GetIndexStream()->Resize(sizeof(indices));
			{
				GraphicsBuffer::Mapper mapper(*rl_->GetIndexStream(), BA_Write_Only);
				std::copy(&indices[0], &indices[4], mapper.Pointer<uint16_t>());
			}

			technique_ = rf.LoadEffect("ParticleSystem.fx")->TechniqueByName("PointSprite");

			particle_sampler_->SetTexture(LoadTexture("particle.dds"));
			particle_sampler_->Filtering(Sampler::TFO_Bilinear);
			particle_sampler_->AddressingMode(Sampler::TAT_Addr_U, Sampler::TAM_Wrap);
			particle_sampler_->AddressingMode(Sampler::TAT_Addr_V, Sampler::TAM_Wrap);

			depth_sampler_->Filtering(Sampler::TFO_Point);
			depth_sampler_->AddressingMode(Sampler::TAT_Addr_U, Sampler::TAM_Clamp);
			depth_sampler_->AddressingMode(Sampler::TAT_Addr_V, Sampler::TAM_Clamp);
		}

		void DepthTexture(TexturePtr tex)
		{
			depth_sampler_->SetTexture(tex);
		}

		void OnRenderBegin()
		{
			*(technique_->Effect().ParameterByName("particle_sampler")) = particle_sampler_;
			*(technique_->Effect().ParameterByName("depth_sampler")) = depth_sampler_;

			App3DFramework const & app = Context::Instance().AppInstance();

			float4x4 const & view = app.ActiveCamera().ViewMatrix();
			float4x4 const & proj = app.ActiveCamera().ProjMatrix();

			*(technique_->Effect().ParameterByName("View")) = view;
			*(technique_->Effect().ParameterByName("Proj")) = proj;
			*(technique_->Effect().ParameterByName("WVP")) = view * proj;
			*(technique_->Effect().ParameterByName("InvWVP")) = MathLib::inverse(view * proj);
			*(technique_->Effect().ParameterByName("InvProj")) = MathLib::inverse(proj);

			*(technique_->Effect().ParameterByName("PointRadius")) = 0.04f;

			RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
			float4 const & texel_to_pixel = re.TexelToPixelOffset();
			float const x_offset = texel_to_pixel.x() / re.CurRenderTarget()->Width();
			float const y_offset = texel_to_pixel.y() / re.CurRenderTarget()->Height();
			*(technique_->Effect().ParameterByName("offset")) = float2(x_offset, y_offset);
		}

	private:
		SamplerPtr particle_sampler_;
		SamplerPtr depth_sampler_; 
	};

	class CopyPostProcess : public PostProcess
	{
	public:
		CopyPostProcess()
			: PostProcess(Context::Instance().RenderFactoryInstance().LoadEffect("ParticleSystem.fx")->TechniqueByName("Copy")),
				depth_sampler_(new Sampler)
		{
			depth_sampler_->Filtering(Sampler::TFO_Point);
			depth_sampler_->AddressingMode(Sampler::TAT_Addr_U, Sampler::TAM_Clamp);
			depth_sampler_->AddressingMode(Sampler::TAT_Addr_V, Sampler::TAM_Clamp);
		}

		void DepthTexture(TexturePtr depth)
		{
			depth_sampler_->SetTexture(depth);
		}

		void OnRenderBegin()
		{
			PostProcess::OnRenderBegin();

			*(technique_->Effect().ParameterByName("depth_sampler")) = depth_sampler_;
		}

	private:
		SamplerPtr depth_sampler_;
	};

	enum
	{
		Exit
	};

	InputActionDefine actions[] = 
	{
		InputActionDefine(Exit, KS_Escape)
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

	ParticleSystemApp app;
	app.Create("Particle System", settings);
	app.Run();

	return 0;
}

ParticleSystemApp::ParticleSystemApp()
{
	ResLoader::Instance().AddPath("../media/Common");
	ResLoader::Instance().AddPath("../media/ParticleSystem");
}

void ParticleSystemApp::InitObjects()
{
	// ½¨Á¢×ÖÌå
	font_ = Context::Instance().RenderFactoryInstance().MakeFont("gkai00mp.ttf", 16);

	this->LookAt(float3(-1.5f, 1.5f, -1.5f), float3(0, 0, 0));
	this->Proj(0.01f, 100);

	fpcController_.AttachCamera(this->ActiveCamera());
	fpcController_.Scalers(0.05f, 0.1f);

	InputEngine& inputEngine(Context::Instance().InputFactoryInstance().InputEngineInstance());
	InputActionMap actionMap;
	actionMap.AddActions(actions, actions + sizeof(actions) / sizeof(actions[0]));

	action_handler_t input_handler(inputEngine);
	input_handler += boost::bind(&ParticleSystemApp::InputHandler, this, _1, _2);
	inputEngine.ActionMap(actionMap, input_handler, true);

	height_img_.reset(new HeightImg(-2, -2, 2, 2, LoadTexture("grcanyon.dds"), 0.3f));

	renderInstance_.reset(new RenderPointSprite);
	for (int i = 0; i < NUM_PARTICLE; ++ i)
	{
		SceneObjectPtr so(new PointSpriteObject);
		checked_pointer_cast<PointSpriteObject>(so)->SetRenderable(renderInstance_);
		scene_objs_.push_back(so);
	}

	std::vector<float3> vertices;
	std::vector<uint16_t> indices;
	HeightMap hm;
	hm.BuildTerrain(-2, -2, 2, 2, 4.0f / 64, 4.0f / 64, vertices, indices, *height_img_);
	terrain_.reset(new TerrainObject(vertices, indices));


	std::vector<float3> normal(vertices.size());
	MathLib::compute_normal<float>(&normal[0],
		&indices[0], &indices[0] + indices.size(),
		&vertices[0], &vertices[0] + vertices.size());

	ps_.reset(new ParticleSystem<Particle>(NUM_PARTICLE,
		GenParticle<Particle>(), UpdateParticle<Particle>(height_img_, normal)));

	ps_->AutoEmit(128);

	RenderFactory& rf = Context::Instance().RenderFactoryInstance();
	RenderEngine& re = rf.RenderEngineInstance();

	depth_buffer_ = rf.MakeFrameBuffer();
	RenderTargetPtr screen_buffer = re.CurRenderTarget();
	depth_buffer_->GetViewport().camera = screen_buffer->GetViewport().camera;
}

void ParticleSystemApp::OnResize(uint32_t width, uint32_t height)
{
	RenderFactory& rf = Context::Instance().RenderFactoryInstance();
	
	depth_tex_ = rf.MakeTexture2D(width, height, 1, EF_R32F);
	depth_buffer_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*depth_tex_, 0));

	checked_pointer_cast<RenderPointSprite>(renderInstance_)->DepthTexture(depth_tex_);
}

void ParticleSystemApp::InputHandler(InputEngine const & /*sender*/, InputAction const & action)
{
	switch (action.first)
	{
	case Exit:
		this->Quit();
		break;
	}
}

uint32_t ParticleSystemApp::NumPasses() const
{
	return 2;
}

bool particle_cmp(Particle const & lhs, Particle const & rhs)
{
	App3DFramework const & app = Context::Instance().AppInstance();
	float4x4 view = app.ActiveCamera().ViewMatrix();

	float3 l_v = MathLib::transform_coord(lhs.pos, view);
	float3 r_v = MathLib::transform_coord(rhs.pos, view);

	return l_v.z() > r_v.z();
}

void ParticleSystemApp::DoUpdate(uint32_t pass)
{
	RenderFactory& rf = Context::Instance().RenderFactoryInstance();
	RenderEngine& re = rf.RenderEngineInstance();
	SceneManager& sm = Context::Instance().SceneManagerInstance();

	switch (pass)
	{
	case 0:
		fpcController_.Update();

		sm.Clear();

		re.BindRenderTarget(depth_buffer_);
		re.Clear(RenderEngine::CBM_Color, Color(1, 1, 1, 1), 1, 0);
		
		checked_pointer_cast<TerrainObject>(terrain_)->DepthPass(true);
		terrain_->AddToSceneManager();
		break;

	case 1:
		sm.Clear();

		re.BindRenderTarget(RenderTargetPtr());
		re.Clear(RenderEngine::CBM_Color | RenderEngine::CBM_Depth, Color(0.2f, 0.4f, 0.6f, 1), 1, 0);

		checked_pointer_cast<TerrainObject>(terrain_)->DepthPass(false);
		terrain_->AddToSceneManager();

		std::vector<Particle> active_particles;
		for (uint32_t i = 0; i < ps_->NumParticles(); ++ i)
		{
			if (ps_->GetParticle(i).life > 0)
			{
				active_particles.push_back(ps_->GetParticle(i));
			}
		}
		std::sort(active_particles.begin(), active_particles.end(), particle_cmp);
		for (uint32_t i = 0; i < active_particles.size(); ++ i)
		{
			checked_pointer_cast<PointSpriteObject>(scene_objs_[i])->Instance(active_particles[i]);
			scene_objs_[i]->AddToSceneManager();
		}

		float4x4 mat = MathLib::rotation_x(PI / 6) * MathLib::rotation_y(clock() / 300.0f) * MathLib::translation(0.0f, 0.5f, 0.0f);
		ps_->ModelMatrix(mat);

		ps_->Update(static_cast<float>(timer_.elapsed()));
		timer_.restart();

		std::wostringstream stream;
		stream << this->FPS();

		font_->RenderText(0, 0, Color(1, 1, 0, 1), L"Particle System");
		font_->RenderText(0, 18, Color(1, 1, 0, 1), stream.str());
		break;
	}
}
