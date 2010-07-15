#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/ElementFormat.hpp>
#include <KlayGE/Framebuffer.hpp>
#include <KlayGE/RenderLayout.hpp>
#include <KlayGE/Renderable.hpp>
#include <KlayGE/RenderableHelper.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/SceneManager.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/RenderSettings.hpp>
#include <KlayGE/GraphicsBuffer.hpp>
#include <KlayGE/SceneObjectHelper.hpp>
#include <KlayGE/PostProcess.hpp>
#include <KlayGe/Timer.hpp>

#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/InputFactory.hpp>

#include <vector>
#include <sstream>
#include <ctime>
#include <iostream>
#include <boost/tuple/tuple.hpp>
#include <boost/bind.hpp>

#include "OceanSimulator.hpp"

#include "Ocean.hpp"

using namespace std;
using namespace KlayGE;

namespace
{
	int const SUN_FLARENUM = 6;

	class RenderSun : public RenderableHelper
	{
	public:
		RenderSun()
			: RenderableHelper(L"Sun")
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();

			rl_ = rf.MakeRenderLayout();
			rl_->TopologyType(RenderLayout::TT_TriangleList);

			std::vector<float3> vertices;
			for (int i = 0; i < SUN_FLARENUM; ++ i)
			{
				vertices.push_back(float3(-1, +1, i + 0.1f));
				vertices.push_back(float3(+1, +1, i + 0.1f));
				vertices.push_back(float3(-1, -1, i + 0.1f));
				vertices.push_back(float3(+1, -1, i + 0.1f));
			}

			ElementInitData init_data;
			init_data.data = &vertices[0];
			init_data.slice_pitch = init_data.row_pitch = static_cast<uint32_t>(vertices.size() * sizeof(vertices[0]));

			GraphicsBufferPtr pos_vb = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read, &init_data);
			rl_->BindVertexStream(pos_vb, boost::make_tuple(vertex_element(VEU_Position, 0, EF_BGR32F)));

			std::vector<uint32_t> indices;
			for (int i = 0; i < SUN_FLARENUM; ++ i)
			{
				indices.push_back(i * 4 + 2);
				indices.push_back(i * 4 + 0);
				indices.push_back(i * 4 + 1);

				indices.push_back(i * 4 + 1);
				indices.push_back(i * 4 + 3);
				indices.push_back(i * 4 + 2);
			}

			init_data.data = &indices[0];
			init_data.slice_pitch = init_data.row_pitch = static_cast<uint32_t>(indices.size() * sizeof(indices[0]));

			GraphicsBufferPtr ib = rf.MakeIndexBuffer(BU_Static, EAH_GPU_Read, &init_data);
			rl_->BindIndexStream(ib, EF_R32UI);

			technique_ = rf.LoadEffect("Sun.fxml")->TechniqueByName("SunFlare");
		}

		void Direction(float3 const & dir)
		{
			*(technique_->Effect().ParameterByName("sun_dir")) = -dir;
		}

		void FlareParam(std::vector<float3> const & param, float alpha_fac)
		{
			*(technique_->Effect().ParameterByName("flare_param")) = param;
			*(technique_->Effect().ParameterByName("alpha_fac")) = alpha_fac;
		}

		void OnRenderBegin()
		{
			App3DFramework const & app = Context::Instance().AppInstance();
			Camera const & camera = app.ActiveCamera();
			
			*(technique_->Effect().ParameterByName("eye_pos")) = camera.EyePos();

			RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
			*(technique_->Effect().ParameterByName("scale")) = static_cast<float>(re.CurFrameBuffer()->Width()) / re.CurFrameBuffer()->Height();
		}
	};
	
	class SunObject : public SceneObjectHelper
	{
	public:
		SunObject()
			: SceneObjectHelper(0)
		{
			renderable_.reset(new RenderSun);
			this->Direction(float3(0.286024f, 0.75062f, 0.592772f));
		}

		void Direction(float3 const & dir)
		{
			dir_ = dir;
			checked_pointer_cast<RenderSun>(renderable_)->Direction(dir_);
		}

		float3 const & Direction() const
		{
			return dir_;
		}

		void Update()
		{
			float const FLARE_RENDERANGLE = 0.9f;
			float const FLARE_SCALEAMOUNT = 0.2f;

			App3DFramework const & app = Context::Instance().AppInstance();
			Camera const & camera = app.ActiveCamera();

			float4x4 const & view = camera.ViewMatrix();
			float4x4 const & proj = camera.ProjMatrix();

			float3 sun_vec = MathLib::normalize(dir_);
			float3 const & view_vec = camera.ViewVec();

			float angle = MathLib::dot(view_vec, sun_vec);

			// update flare
			if (angle > FLARE_RENDERANGLE)
			{
				this->Visible(true);

				// get angle amount by current angle
				float angle_amount = 1 - (1 - angle) / (1 - FLARE_RENDERANGLE);	// convert angle to percent 
				float inv_angle_amount = std::max(0.85f, (1 - angle) / (1 - FLARE_RENDERANGLE));

				float alpha_fac;
				if (angle_amount < 0.5f)
				{
					alpha_fac = angle_amount;
				}
				else
				{
					alpha_fac = 1 - angle_amount;
				}

				// calculate flare pos
				float2 center_pos(0, 0);
				float3 sun_vec_es = MathLib::transform_normal(dir_, view);
				float3 sun_pos_es = camera.FarPlane() / sun_vec_es.z() * sun_vec_es;
				float2 axis_vec = MathLib::transform_coord(sun_pos_es, proj);

				// update flare pos and scale matrix by pos and angle amount
				std::vector<float3> flare_param(SUN_FLARENUM);
				for (int flare = 0; flare < SUN_FLARENUM; ++ flare)
				{
					float2 flare_pos = center_pos + (flare - SUN_FLARENUM * 0.2f) / ((SUN_FLARENUM - 1.0f) * 1.5f) * axis_vec;
					float scale_fac = FLARE_SCALEAMOUNT * inv_angle_amount * ((SUN_FLARENUM - flare) / (SUN_FLARENUM - 1.0f));

					flare_param[flare] = float3(flare_pos.x(), flare_pos.y(), scale_fac);
				}

				checked_pointer_cast<RenderSun>(renderable_)->FlareParam(flare_param, alpha_fac);
			}
			else
			{
				this->Visible(false);
			}
		}

	private:
		float3 dir_;
	};

	class RenderInfFlatObject : public RenderableHelper
	{
	public:
		RenderInfFlatObject(std::wstring const & name)
			: RenderableHelper(name)
		{
			int const NX = 256;
			int const NY = 256;

			RenderFactory& rf = Context::Instance().RenderFactoryInstance();

			rl_ = rf.MakeRenderLayout();
			rl_->TopologyType(RenderLayout::TT_TriangleList);

			App3DFramework const & app = Context::Instance().AppInstance();
			Camera const & camera = app.ActiveCamera();

			float angle = atan(tan(camera.FOV() / 2) * camera.Aspect());
			x_dir_ = float2(-sin(angle), cos(angle));
			y_dir_ = float2(-x_dir_.x(), x_dir_.y());

			float2 addr(0, 0);
			float2 increment(1, 1);
			std::vector<float2> vertices;
			for (int y = 0; y < NY - 1; ++ y, addr.y() += increment.y())
			{
				increment.x() = 1;
				addr.x() = 0;
				for (int x = 0; x < NX - 1; ++ x, addr.x() += increment.x())
				{
					float2 p(addr.x() * x_dir_ * 0.5f + addr.y() * y_dir_ * 0.5f);
					vertices.push_back(p);
					increment.x() *= 1.012f;
				}
				{
					float2 p((addr.x() + 1000.0f) * x_dir_ * 0.5f + addr.y() * y_dir_ * 0.5f);
					vertices.push_back(p);
				}

				increment.y() *= 1.012f;
			}
			{
				increment.x() = 1;
				addr.x() = 0;
				for (int x = 0; x < NX - 1; ++ x, addr.x() += increment.x())
				{
					float2 p(addr.x() * x_dir_ * 0.5f + (addr.y() + 1000.0f) * y_dir_ * 0.5f);
					vertices.push_back(p);

					increment.x() *= 1.012f;
				}
				{
					float2 p((addr.x() + 1000.0f) * x_dir_ * 0.5f + (addr.y() + 1000.0f) * y_dir_ * 0.5f);
					vertices.push_back(p);
				}
			}

			ElementInitData init_data;
			init_data.data = &vertices[0];
			init_data.slice_pitch = init_data.row_pitch = vertices.size() * sizeof(vertices[0]);

			GraphicsBufferPtr pos_vb = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read, &init_data);
			rl_->BindVertexStream(pos_vb, boost::make_tuple(vertex_element(VEU_Position, 0, EF_GR32F)));

			std::vector<uint32_t> indices;
			for (uint32_t y = 0; y < NY - 1; ++ y)
			{
				for (uint32_t x = 0; x < NX - 1; ++ x)
				{
					indices.push_back((y + 0) * NX + (x + 0));
					indices.push_back((y + 0) * NX + (x + 1));
					indices.push_back((y + 1) * NX + (x + 0));

					indices.push_back((y + 1) * NX + (x + 0));
					indices.push_back((y + 0) * NX + (x + 1));
					indices.push_back((y + 1) * NX + (x + 1));
				}
			}

			init_data.data = &indices[0];
			init_data.slice_pitch = init_data.row_pitch = indices.size() * sizeof(indices[0]);

			GraphicsBufferPtr ib = rf.MakeIndexBuffer(BU_Static, EAH_GPU_Read, &init_data);
			rl_->BindIndexStream(ib, EF_R32UI);
		}

		virtual ~RenderInfFlatObject()
		{
		}

		float2 const & XDir() const
		{
			return x_dir_;
		}

		float2 const & YDir() const
		{
			return y_dir_;
		}

		void SetStretch(float stretch)
		{
			*(technique_->Effect().ParameterByName("stretch")) = stretch;
		}

		void SetBaseLevel(float base_level)
		{
			*(technique_->Effect().ParameterByName("base_level")) = base_level;
		}

		void OffsetY(float y)
		{
			*(technique_->Effect().ParameterByName("offset_y")) = y;
		}

		void OnRenderBegin()
		{
			App3DFramework const & app = Context::Instance().AppInstance();
			Camera const & camera = app.ActiveCamera();

			float4x4 const & view = camera.ViewMatrix();
			float4x4 const & proj = camera.ProjMatrix();

			float3 look_at_vec = float3(camera.LookAt().x() - camera.EyePos().x(), 0, camera.LookAt().z() - camera.EyePos().z());
			if (MathLib::dot(look_at_vec, look_at_vec) < 1e-6f)
			{
				look_at_vec = float3(0, 0, 1);
			}
			float4x4 virtual_view = MathLib::look_at_lh(camera.EyePos(), camera.EyePos() + look_at_vec);
			float4x4 inv_virtual_view = MathLib::inverse(virtual_view);

			float4x4 vp = view * proj;
			*(technique_->Effect().ParameterByName("mvp")) = vp;
			*(technique_->Effect().ParameterByName("inv_virtual_view")) = inv_virtual_view;
			*(technique_->Effect().ParameterByName("eye_pos")) = camera.EyePos();
		}

	protected:
		float2 x_dir_, y_dir_;
	};

	class InfFlatObject : public SceneObjectHelper
	{
	public:
		InfFlatObject()
			: SceneObjectHelper(SOA_Moveable)
		{
		}

		virtual ~InfFlatObject()
		{
		}

		void Update()
		{
			App3DFramework const & app = Context::Instance().AppInstance();
			Camera const & camera = app.ActiveCamera();

			float3 look_at_vec = float3(camera.LookAt().x() - camera.EyePos().x(), 0, camera.LookAt().z() - camera.EyePos().z());
			if (MathLib::dot(look_at_vec, look_at_vec) < 1e-6f)
			{
				look_at_vec = float3(0, 0, 1);
			}
			float4x4 virtual_view = MathLib::look_at_lh(camera.EyePos(), camera.EyePos() + look_at_vec);
			float4x4 inv_virtual_view = MathLib::inverse(virtual_view);

			float4x4 const & view = camera.ViewMatrix();
			float4x4 const & proj = camera.ProjMatrix();
			float4x4 proj_to_virtual_view = MathLib::inverse(view * proj) * virtual_view;

			float2 const & x_dir_2d = checked_pointer_cast<RenderInfFlatObject>(renderable_)->XDir();
			float2 const & y_dir_2d = checked_pointer_cast<RenderInfFlatObject>(renderable_)->YDir();
			float3 x_dir(x_dir_2d.x(), -camera.EyePos().y(), x_dir_2d.y());
			float3 y_dir(y_dir_2d.x(), -camera.EyePos().y(), y_dir_2d.y());

			float3 const frustum[8] = 
			{
				MathLib::transform_coord(float3(-1, +1, 1), proj_to_virtual_view),
				MathLib::transform_coord(float3(+1, +1, 1), proj_to_virtual_view),
				MathLib::transform_coord(float3(-1, -1, 1), proj_to_virtual_view),
				MathLib::transform_coord(float3(+1, -1, 1), proj_to_virtual_view),
				MathLib::transform_coord(float3(-1, +1, 0), proj_to_virtual_view),
				MathLib::transform_coord(float3(+1, +1, 0), proj_to_virtual_view),
				MathLib::transform_coord(float3(-1, -1, 0), proj_to_virtual_view),
				MathLib::transform_coord(float3(+1, -1, 0), proj_to_virtual_view)
			};

			int const view_cube[24] =
			{
				0, 1, 1, 3, 3, 2, 2, 0,
				4, 5, 5, 7, 7, 6, 6, 4,
				0, 4, 1, 5, 3, 7, 2, 6
			};

			Plane const lower_bound = MathLib::from_point_normal(float3(0, base_level_ - camera.EyePos().y() - strength_, 0), float3(0, 1, 0));

			bool intersect = false;
			float sy = 0;
			for (int i = 0; i < 12; ++ i)
			{
				int src = view_cube[i * 2 + 0];
				int dst = view_cube[i * 2 + 1];
				if (MathLib::dot_coord(lower_bound, frustum[src]) / MathLib::dot_coord(lower_bound, frustum[dst]) < 0)
				{
					float t = MathLib::intersect_ray(lower_bound, frustum[src], frustum[dst] - frustum[src]);
					float3 p = MathLib::lerp(frustum[src], frustum[dst], t);
					sy = std::max(sy, std::max((x_dir.z() * p.x() - x_dir.x() * p.z()) / x_dir.x(),
						(y_dir.z() * p.x() - y_dir.x() * p.z()) / y_dir.x()));
					intersect = true;
				}
			}
			checked_pointer_cast<RenderInfFlatObject>(renderable_)->OffsetY(sy);

			this->Visible(intersect);
		}

	protected:
		float base_level_;
		float strength_;
	};

	class RenderTerrain : public RenderInfFlatObject
	{
	public:
		RenderTerrain()
			: RenderInfFlatObject(L"Terrain")
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();
			technique_ = rf.LoadEffect("InfTerrain.fxml")->TechniqueByName("Terrain");
		}

		void SunDirection(float3 const & dir)
		{
			*(technique_->Effect().ParameterByName("sun_dir")) = -dir;
		}

		void ReflectionPass(bool ref)
		{
			if (ref)
			{
				technique_ = technique_->Effect().TechniqueByName("TerrainReflection");
			}
			else
			{
				technique_ = technique_->Effect().TechniqueByName("Terrain");
			}
		}
	};

	class TerrainObject : public InfFlatObject
	{
	public:
		TerrainObject()
		{
			base_level_ = -10;
			strength_ = 50;

			renderable_.reset(new RenderTerrain);
			checked_pointer_cast<RenderTerrain>(renderable_)->SetStretch(strength_);
			checked_pointer_cast<RenderTerrain>(renderable_)->SetBaseLevel(base_level_);
		}

		void SunDirection(float3 const & dir)
		{
			checked_pointer_cast<RenderTerrain>(renderable_)->SunDirection(dir);
		}

		void ReflectionPass(bool ref)
		{
			checked_pointer_cast<RenderTerrain>(renderable_)->ReflectionPass(ref);
		}
	};

	class RenderOcean : public RenderInfFlatObject
	{
	public:
		RenderOcean()
			: RenderInfFlatObject(L"Ocean")
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();
			technique_ = rf.LoadEffect("Ocean.fxml")->TechniqueByName("Ocean");
		}

		void SunDirection(float3 const & dir)
		{
			*(technique_->Effect().ParameterByName("sun_dir")) = -dir;
		}

		void PatchLength(float patch_length)
		{
			*(technique_->Effect().ParameterByName("patch_length")) = patch_length;
		}

		void DisplacementMap(TexturePtr const & tex)
		{
			*(technique_->Effect().ParameterByName("displacement_tex")) = tex;
		}

		void GradientMap(TexturePtr const & tex)
		{
			*(technique_->Effect().ParameterByName("gradient_tex")) = tex;
		}

		void RefractionTex(TexturePtr const & tex)
		{
			*(technique_->Effect().ParameterByName("refraction_tex")) = tex;
		}

		void ReflectionTex(TexturePtr const & tex)
		{
			*(technique_->Effect().ParameterByName("reflection_tex")) = tex;
		}
	};

	class OceanObject : public InfFlatObject
	{
	public:
		OceanObject()
		{
			base_level_ = 0;
			strength_ = 10;

			renderable_.reset(new RenderOcean);
			checked_pointer_cast<RenderOcean>(renderable_)->SetStretch(strength_);
			checked_pointer_cast<RenderOcean>(renderable_)->SetBaseLevel(base_level_);

			Plane ocean_plane;
			ocean_plane = MathLib::from_point_normal(float3(0, base_level_, 0), float3(0, 1, 0));
			reflect_mat_ = MathLib::reflect(ocean_plane);

			// The size of displacement map. In this sample, it's fixed to 512.
			ocean_param_.dmap_dim			= 512;
			// The side length (world space) of square sized patch
			ocean_param_.patch_length		= 42;
			// Adjust this parameter to control the simulation speed
			ocean_param_.time_scale			= 0.8f;
			// A scale to control the amplitude. Not the world space height
			ocean_param_.wave_amplitude		= 0.003f;
			// 2D wind direction. No need to be normalized
			// The bigger the wind speed, the larger scale of wave crest.
			// But the wave scale can be no larger than patch_length
			ocean_param_.wind_speed			= float2(0.8f, 0.6f) * 6;
			// Damp out the components opposite to wind direction.
			// The smaller the value, the higher wind dependency
			ocean_param_.wind_dependency	= 0.1f;
			// Control the scale of horizontal movement. Higher value creates
			// pointy crests.
			ocean_param_.choppy_scale		= 1.1f;

			dirty_ = true;

			ocean_simulator_.reset(new OceanSimulator);
		}

		void SunDirection(float3 const & dir)
		{
			checked_pointer_cast<RenderOcean>(renderable_)->SunDirection(dir);
		}

		void Update()
		{
			if (dirty_)
			{
				ocean_simulator_->Parameters(ocean_param_);
				checked_pointer_cast<RenderOcean>(renderable_)->DisplacementMap(ocean_simulator_->DisplacementTex());
				checked_pointer_cast<RenderOcean>(renderable_)->GradientMap(ocean_simulator_->GradientTex());
				checked_pointer_cast<RenderOcean>(renderable_)->PatchLength(ocean_param_.patch_length);

				dirty_ = false;
			}

			InfFlatObject::Update();

			if (this->Visible())
			{
				ocean_simulator_->Update();
			}
		}

		void RefractionTex(TexturePtr const & tex)
		{
			checked_pointer_cast<RenderOcean>(renderable_)->RefractionTex(tex);
		}

		void ReflectionTex(TexturePtr const & tex)
		{
			checked_pointer_cast<RenderOcean>(renderable_)->ReflectionTex(tex);
		}

		void ReflectViewParams(float3& reflect_eye, float3& reflect_at, float3& reflect_up,
			float3 const & eye, float3 const & at, float3 const & up)
		{
			reflect_eye = MathLib::transform_coord(eye, reflect_mat_);
			reflect_at = MathLib::transform_coord(at, reflect_mat_);
			reflect_up = MathLib::transform_normal(up, reflect_mat_);
			reflect_up *= -1.0f;
		}

		int DMapDim() const
		{
			return ocean_param_.dmap_dim;
		}
		void DMapDim(int dmap_dim)
		{
			ocean_param_.dmap_dim = dmap_dim;
			dirty_ = true;
		}

		float PatchLength() const
		{
			return ocean_param_.patch_length;
		}
		void PatchLength(float patch_length)
		{
			ocean_param_.patch_length = patch_length;
			dirty_ = true;
		}

		float TimeScale() const
		{
			return ocean_param_.time_scale;
		}
		void TimeScale(float time_scale)
		{
			ocean_param_.time_scale = time_scale;
			dirty_ = true;
		}

		float WaveAmplitude() const
		{
			return ocean_param_.wave_amplitude;
		}
		void WaveAmplitude(float amp)
		{
			ocean_param_.wave_amplitude = amp;
			dirty_ = true;
		}

		float WindSpeedX() const
		{
			return ocean_param_.wind_speed.x();
		}
		void WindSpeedX(float speed)
		{
			ocean_param_.wind_speed.x() = speed;
			dirty_ = true;
		}

		float WindSpeedY() const
		{
			return ocean_param_.wind_speed.y();
		}
		void WindSpeedY(float speed)
		{
			ocean_param_.wind_speed.y() = speed;
			dirty_ = true;
		}

		float WindDependency() const
		{
			return ocean_param_.wind_dependency;
		}
		void WindDependency(float dep)
		{
			ocean_param_.wind_dependency = dep;
			dirty_ = true;
		}

		float ChoppyScale() const
		{
			return ocean_param_.choppy_scale;
		}
		void ChoppyScale(float choppy)
		{
			ocean_param_.choppy_scale = choppy;
			dirty_ = true;
		}

	private:
		OceanParameter ocean_param_;
		bool dirty_;

		float4x4 reflect_mat_;

		boost::shared_ptr<OceanSimulator> ocean_simulator_;
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
		if ((caps.max_shader_model < 3) || !caps.cs_support)
		{
			return false;
		}
		return true;
	}
}


int main()
{
	ResLoader::Instance().AddPath("../Samples/media/Common");
	ResLoader::Instance().AddPath("../Samples/media/Ocean");

	RenderSettings settings = Context::Instance().LoadCfg("KlayGE.cfg");
	settings.ConfirmDevice = ConfirmDevice;

	OceanApp app("Ocean", settings);
	app.Create();
	app.Run();

	return 0;
}

OceanApp::OceanApp(std::string const & name, RenderSettings const & settings)
					: App3DFramework(name, settings)
{
}

void OceanApp::InitObjects()
{
	this->LookAt(float3(0, 20, 0), float3(0, 19.8f, 1));
	this->Proj(0.01f, 3000);

	// ½¨Á¢×ÖÌå
	font_ = Context::Instance().RenderFactoryInstance().MakeFont("gkai00mp.kfont");

	TexturePtr skybox_tex = LoadTexture("Langholmen.dds", EAH_GPU_Read)();

	terrain_.reset(new TerrainObject);
	terrain_->AddToSceneManager();
	ocean_.reset(new OceanObject);
	ocean_->AddToSceneManager();
	sun_flare_.reset(new SunObject);
	sun_flare_->AddToSceneManager();

	checked_pointer_cast<TerrainObject>(terrain_)->SunDirection(checked_pointer_cast<SunObject>(sun_flare_)->Direction());
	checked_pointer_cast<OceanObject>(ocean_)->SunDirection(checked_pointer_cast<SunObject>(sun_flare_)->Direction());

	RenderFactory& rf = Context::Instance().RenderFactoryInstance();

	sky_box_ = MakeSharedPtr<SceneObjectSkyBox>();
	checked_pointer_cast<SceneObjectSkyBox>(sky_box_)->Technique(rf.LoadEffect("Ocean.fxml")->TechniqueByName("FoggySkyBox"));
	checked_pointer_cast<SceneObjectSkyBox>(sky_box_)->CubeMap(skybox_tex);
	sky_box_->AddToSceneManager();

	fpcController_.Scalers(0.05f, 1.0f);

	InputEngine& inputEngine(Context::Instance().InputFactoryInstance().InputEngineInstance());
	InputActionMap actionMap;
	actionMap.AddActions(actions, actions + sizeof(actions) / sizeof(actions[0]));

	action_handler_t input_handler(new input_signal);
	input_handler->connect(boost::bind(&OceanApp::InputHandler, this, _1, _2));
	inputEngine.ActionMap(actionMap, input_handler, true);

	copy_pp_ = LoadPostProcess(ResLoader::Instance().Load("Copy.ppml"), "copy");

	refraction_fb_ = rf.MakeFrameBuffer();
	refraction_fb_->GetViewport().camera = rf.RenderEngineInstance().CurFrameBuffer()->GetViewport().camera;

	Camera& scene_camera = this->ActiveCamera();
	reflection_fb_ = rf.MakeFrameBuffer();
	reflection_fb_->GetViewport().camera->ProjParams(scene_camera.FOV(), scene_camera.Aspect(),
			scene_camera.NearPlane(), scene_camera.FarPlane());

	blur_y_ = MakeSharedPtr<BlurYPostProcess<SeparableGaussianFilterPostProcess> >(8, 1.0f);

	UIManager::Instance().Load(ResLoader::Instance().Load("Ocean.uiml"));
	dialog_params_ = UIManager::Instance().GetDialog("Parameters");
	id_dmap_dim_static_ = dialog_params_->IDFromName("DMapDimStatic");
	id_dmap_dim_slider_ = dialog_params_->IDFromName("DMapDimSlider");
	id_patch_length_static_ = dialog_params_->IDFromName("PatchLengthStatic");
	id_patch_length_slider_ = dialog_params_->IDFromName("PatchLengthSlider");
	id_time_scale_static_ = dialog_params_->IDFromName("TimeScaleStatic");
	id_time_scale_slider_ = dialog_params_->IDFromName("TimeScaleSlider");
	id_wave_amplitude_static_ = dialog_params_->IDFromName("WaveAmplitudeStatic");
	id_wave_amplitude_slider_ = dialog_params_->IDFromName("WaveAmplitudeSlider");
	id_wind_speed_x_static_ = dialog_params_->IDFromName("WindSpeedXStatic");
	id_wind_speed_x_slider_ = dialog_params_->IDFromName("WindSpeedXSlider");
	id_wind_speed_y_static_ = dialog_params_->IDFromName("WindSpeedYStatic");
	id_wind_speed_y_slider_ = dialog_params_->IDFromName("WindSpeedYSlider");
	id_wind_dependency_static_ = dialog_params_->IDFromName("WindDependencyStatic");
	id_wind_dependency_slider_ = dialog_params_->IDFromName("WindDependencySlider");
	id_choppy_scale_static_ = dialog_params_->IDFromName("ChoppyScaleStatic");
	id_choppy_scale_slider_ = dialog_params_->IDFromName("ChoppyScaleSlider");
	id_fps_camera_ = dialog_params_->IDFromName("FPSCamera");

	dialog_params_->Control<UISlider>(id_dmap_dim_slider_)->OnValueChangedEvent().connect(boost::bind(&OceanApp::DMapDimChangedHandler, this, _1));
	this->DMapDimChangedHandler(*dialog_params_->Control<UISlider>(id_dmap_dim_slider_));

	dialog_params_->Control<UISlider>(id_patch_length_slider_)->OnValueChangedEvent().connect(boost::bind(&OceanApp::PatchLengthChangedHandler, this, _1));
	this->PatchLengthChangedHandler(*dialog_params_->Control<UISlider>(id_patch_length_slider_));

	dialog_params_->Control<UISlider>(id_time_scale_slider_)->OnValueChangedEvent().connect(boost::bind(&OceanApp::TimeScaleChangedHandler, this, _1));
	this->TimeScaleChangedHandler(*dialog_params_->Control<UISlider>(id_time_scale_slider_));

	dialog_params_->Control<UISlider>(id_wave_amplitude_slider_)->OnValueChangedEvent().connect(boost::bind(&OceanApp::WaveAmplitudeChangedHandler, this, _1));
	this->WaveAmplitudeChangedHandler(*dialog_params_->Control<UISlider>(id_wave_amplitude_slider_));

	dialog_params_->Control<UISlider>(id_wind_speed_x_slider_)->OnValueChangedEvent().connect(boost::bind(&OceanApp::WindSpeedXChangedHandler, this, _1));
	this->WindSpeedXChangedHandler(*dialog_params_->Control<UISlider>(id_wind_speed_x_slider_));

	dialog_params_->Control<UISlider>(id_wind_speed_y_slider_)->OnValueChangedEvent().connect(boost::bind(&OceanApp::WindSpeedYChangedHandler, this, _1));
	this->WindSpeedYChangedHandler(*dialog_params_->Control<UISlider>(id_wind_speed_y_slider_));

	dialog_params_->Control<UISlider>(id_wind_dependency_slider_)->OnValueChangedEvent().connect(boost::bind(&OceanApp::WindDependencyChangedHandler, this, _1));
	this->WindDependencyChangedHandler(*dialog_params_->Control<UISlider>(id_wind_dependency_slider_));

	dialog_params_->Control<UISlider>(id_choppy_scale_slider_)->OnValueChangedEvent().connect(boost::bind(&OceanApp::ChoppyScaleChangedHandler, this, _1));
	this->ChoppyScaleChangedHandler(*dialog_params_->Control<UISlider>(id_choppy_scale_slider_));

	dialog_params_->Control<UICheckBox>(id_fps_camera_)->OnChangedEvent().connect(boost::bind(&OceanApp::FPSCameraHandler, this, _1));
}

void OceanApp::OnResize(uint32_t width, uint32_t height)
{
	App3DFramework::OnResize(width, height);

	RenderFactory& rf = Context::Instance().RenderFactoryInstance();

	refraction_tex_ = rf.MakeTexture2D(width, height, 1, 1, EF_ABGR16F, 1, 0, EAH_GPU_Read | EAH_GPU_Write, NULL);
	refraction_fb_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*refraction_tex_, 0, 0));
	refraction_fb_->Attach(FrameBuffer::ATT_DepthStencil, rf.RenderEngineInstance().CurFrameBuffer()->Attached(FrameBuffer::ATT_DepthStencil));
	copy_pp_->InputPin(0, refraction_tex_);

	reflection_tex_ = rf.MakeTexture2D(width / 2, height / 2, 1, 1, EF_ABGR16F, 1, 0, EAH_GPU_Read | EAH_GPU_Write, NULL);
	reflection_blur_tex_ = rf.MakeTexture2D(width / 2, height / 2, 1, 1, EF_ABGR16F, 1, 0, EAH_GPU_Read | EAH_GPU_Write, NULL);
	reflection_fb_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*reflection_tex_, 0, 0));
	reflection_fb_->Attach(FrameBuffer::ATT_DepthStencil, rf.Make2DDepthStencilRenderView(width / 2, height / 2, EF_D16, 1, 0));
	reflection_fb_->GetViewport().left = 0;
	reflection_fb_->GetViewport().top = 0;
	reflection_fb_->GetViewport().width = width / 2;
	reflection_fb_->GetViewport().height = height / 2;

	blur_y_->InputPin(0, reflection_tex_);
	blur_y_->OutputPin(0, reflection_blur_tex_);

	checked_pointer_cast<OceanObject>(ocean_)->RefractionTex(refraction_tex_);
	checked_pointer_cast<OceanObject>(ocean_)->ReflectionTex(reflection_blur_tex_);

	UIManager::Instance().SettleCtrls(width, height);
}

void OceanApp::InputHandler(InputEngine const & /*sender*/, InputAction const & action)
{
	switch (action.first)
	{
	case Exit:
		this->Quit();
		break;
	}
}

void OceanApp::DMapDimChangedHandler(UISlider const & sender)
{
	int dmap_dim = 1UL << (3 * sender.GetValue());

	std::wostringstream stream;
	stream << L"DMap dim: " << dmap_dim;
	dialog_params_->Control<UIStatic>(id_dmap_dim_static_)->SetText(stream.str());

	checked_pointer_cast<OceanObject>(ocean_)->DMapDim(dmap_dim);
}

void OceanApp::PatchLengthChangedHandler(UISlider const & sender)
{
	float patch_length = sender.GetValue() * 0.5f;

	std::wostringstream stream;
	stream << L"Patch length: " << patch_length;
	dialog_params_->Control<UIStatic>(id_patch_length_static_)->SetText(stream.str());

	checked_pointer_cast<OceanObject>(ocean_)->PatchLength(patch_length);
}

void OceanApp::TimeScaleChangedHandler(UISlider const & sender)
{
	float time_scale = sender.GetValue() * 0.1f;

	std::wostringstream stream;
	stream << L"Time scale: " << time_scale;
	dialog_params_->Control<UIStatic>(id_time_scale_static_)->SetText(stream.str());

	checked_pointer_cast<OceanObject>(ocean_)->TimeScale(time_scale);
}

void OceanApp::WaveAmplitudeChangedHandler(UISlider const & sender)
{
	float wave_amp = sender.GetValue() * 0.0001f;

	std::wostringstream stream;
	stream << L"Wave amplitude: " << wave_amp;
	dialog_params_->Control<UIStatic>(id_wave_amplitude_static_)->SetText(stream.str());

	checked_pointer_cast<OceanObject>(ocean_)->WaveAmplitude(wave_amp);
}

void OceanApp::WindSpeedXChangedHandler(UISlider const & sender)
{
	float wind_speed = sender.GetValue() * 0.05f;

	std::wostringstream stream;
	stream << L"Wind speed X: " << wind_speed;
	dialog_params_->Control<UIStatic>(id_wind_speed_x_static_)->SetText(stream.str());

	checked_pointer_cast<OceanObject>(ocean_)->WindSpeedX(wind_speed);
}

void OceanApp::WindSpeedYChangedHandler(UISlider const & sender)
{
	float wind_speed = sender.GetValue() * 0.05f;

	std::wostringstream stream;
	stream << L"Wind speed Y: " << wind_speed;
	dialog_params_->Control<UIStatic>(id_wind_speed_y_static_)->SetText(stream.str());

	checked_pointer_cast<OceanObject>(ocean_)->WindSpeedY(wind_speed);
}

void OceanApp::WindDependencyChangedHandler(UISlider const & sender)
{
	float dep = sender.GetValue() * 0.01f;

	std::wostringstream stream;
	stream << L"Wind dependency: " << dep;
	dialog_params_->Control<UIStatic>(id_wind_dependency_static_)->SetText(stream.str());

	checked_pointer_cast<OceanObject>(ocean_)->WindDependency(dep);
}

void OceanApp::ChoppyScaleChangedHandler(UISlider const & sender)
{
	float choppy = sender.GetValue() * 0.1f;

	std::wostringstream stream;
	stream << L"Choppy scale: " << choppy;
	dialog_params_->Control<UIStatic>(id_choppy_scale_static_)->SetText(stream.str());

	checked_pointer_cast<OceanObject>(ocean_)->ChoppyScale(choppy);
}

void OceanApp::FPSCameraHandler(UICheckBox const & sender)
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

void OceanApp::DoUpdateOverlay()
{
	UIManager::Instance().Render();

	SceneManager& sceneMgr(Context::Instance().SceneManagerInstance());

	std::wostringstream stream;
	stream << this->FPS();

	font_->RenderText(0, 0, Color(1, 1, 0, 1), L"Ocean", 16);
	font_->RenderText(0, 18, Color(1, 1, 0, 1), stream.str(), 16);

	stream.str(L"");
	stream << sceneMgr.NumRenderablesRendered() << " Renderables "
		<< sceneMgr.NumPrimitivesRendered() << " Primitives "
		<< sceneMgr.NumVerticesRendered() << " Vertices";
	font_->RenderText(0, 36, Color(1, 1, 1, 1), stream.str(), 16);
}

uint32_t OceanApp::DoUpdate(uint32_t pass)
{
	RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();

	switch (pass)
	{
	case 0:
		re.BindFrameBuffer(refraction_fb_);
		re.CurFrameBuffer()->Clear(FrameBuffer::CBM_Depth, Color(0, 0, 0, 1), 1, 0);
		checked_pointer_cast<TerrainObject>(terrain_)->ReflectionPass(false);
		terrain_->Visible(true);
		sky_box_->Visible(true);
		ocean_->Visible(false);
		return App3DFramework::URV_Need_Flush;

	case 1:
		{
			Camera& scene_camera = this->ActiveCamera();

			re.BindFrameBuffer(reflection_fb_);
			re.CurFrameBuffer()->Clear(FrameBuffer::CBM_Depth, Color(0, 0, 0, 1), 1, 0);
			checked_pointer_cast<TerrainObject>(terrain_)->ReflectionPass(true);
			terrain_->Visible(true);
			sky_box_->Visible(true);
			ocean_->Visible(false);

			float3 reflect_eye, reflect_at, reflect_up;
			checked_pointer_cast<OceanObject>(ocean_)->ReflectViewParams(reflect_eye, reflect_at, reflect_up,
				scene_camera.EyePos(), scene_camera.LookAt(), scene_camera.UpVec());
			reflection_fb_->GetViewport().camera->ViewParams(reflect_eye, reflect_at, reflect_up);
		}

		return App3DFramework::URV_Need_Flush;

	default:
		blur_y_->Apply();
		re.BindFrameBuffer(FrameBufferPtr());
		copy_pp_->Apply();
		terrain_->Visible(false);
		sky_box_->Visible(false);
		ocean_->Visible(true);

		return App3DFramework::URV_Need_Flush | App3DFramework::URV_Finished;
	}
}
