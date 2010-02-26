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
			float3 view_vec = camera.ViewVec();

			float angle = MathLib::dot(view_vec, sun_vec);

			// calculate flare pos

			float2 center_pos(0, 0);
			float3 sun_vec_es = MathLib::transform_normal(dir_, view);
			float3 sun_pos_es = camera.FarPlane() / sun_vec_es.z() * sun_vec_es;
			float2 axis_vec = MathLib::transform_coord(sun_pos_es, proj);

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

				// update flare pos and scale matrix by pos and angle amount
				std::vector<float3> flare_param(SUN_FLARENUM);
				for (int iFlare = 0; iFlare < SUN_FLARENUM; ++ iFlare)
				{
					float2 flare_pos = center_pos + (iFlare - SUN_FLARENUM * 0.2f) / ((SUN_FLARENUM - 1.0f) * 1.2f) * axis_vec;
					float scale_fac = FLARE_SCALEAMOUNT * inv_angle_amount * ((SUN_FLARENUM - iFlare) / (SUN_FLARENUM - 1.0f));

					flare_param[iFlare] = float3(flare_pos.x(), flare_pos.y(), scale_fac);
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

	class RenderOcean : public RenderableHelper
	{
	public:
		RenderOcean()
			: RenderableHelper(L"Ocean")
		{
			int const NX = 512;
			int const NY = 256;

			RenderFactory& rf = Context::Instance().RenderFactoryInstance();

			rl_ = rf.MakeRenderLayout();
			rl_->TopologyType(RenderLayout::TT_TriangleList);

			std::vector<float2> vertices;
			for (int y = 0; y < NY; ++ y)
			{
				for (int x = 0; x < NX; ++ x)
				{
					vertices.push_back(float2(static_cast<float>(x) / (NX - 1),
						static_cast<float>(y) / (NY - 1)));
				}
			}

			ElementInitData init_data;
			init_data.data = &vertices[0];
			init_data.slice_pitch = init_data.row_pitch = static_cast<uint32_t>(vertices.size() * sizeof(vertices[0]));

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
			init_data.slice_pitch = init_data.row_pitch = static_cast<uint32_t>(indices.size() * sizeof(indices[0]));

			GraphicsBufferPtr ib = rf.MakeIndexBuffer(BU_Static, EAH_GPU_Read, &init_data);
			rl_->BindIndexStream(ib, EF_R32UI);

			technique_ = rf.LoadEffect("Ocean.fxml")->TechniqueByName("Ocean");
		}

		void SunDirection(float3 const & dir)
		{
			*(technique_->Effect().ParameterByName("sun_dir")) = -dir;
		}

		void SetBox(Box const & box)
		{
			box_ = box;
		}

		void SetCorners(std::vector<float4> const & corners)
		{
			*(technique_->Effect().ParameterByName("corners")) = corners;
		}

		void DisplacementMap(TexturePtr const & tex)
		{
			*(technique_->Effect().ParameterByName("displacement_tex")) = tex;
		}

		void GradientMap(TexturePtr const & tex)
		{
			*(technique_->Effect().ParameterByName("gradient_tex")) = tex;
		}

		void SkyboxTex(TexturePtr const & tex)
		{
			*(technique_->Effect().ParameterByName("skybox_tex")) = tex;
		}

		void ReflectionTex(TexturePtr const & tex)
		{
			*(technique_->Effect().ParameterByName("reflection_tex")) = tex;
		}

		void OnRenderBegin()
		{
			App3DFramework const & app = Context::Instance().AppInstance();
			Camera const & camera = app.ActiveCamera();

			float4x4 const & view = camera.ViewMatrix();
			float4x4 const & proj = camera.ProjMatrix();

			float4x4 vp = view * proj;
			*(technique_->Effect().ParameterByName("mvp")) = vp;
			*(technique_->Effect().ParameterByName("eye_pos")) = camera.EyePos();
		}
	};

	class OceanObject : public SceneObjectHelper
	{
	public:
		OceanObject()
			: SceneObjectHelper(SOA_Cullable | SOA_Moveable),
				strength_(20)
		{
			renderable_.reset(new RenderOcean);

			float y0 = 0;
			ocean_plane_ = MathLib::from_point_normal(float3(0, y0, 0), float3(0, 1, 0));
			reflect_mat_ = MathLib::reflect(ocean_plane_);

			OceanParameter ocean_param;

			// The size of displacement map. In this sample, it's fixed to 512.
			ocean_param.dmap_dim			= 512;
			// The side length (world space) of square sized patch
			ocean_param.patch_length		= 20;
			// Adjust this parameter to control the simulation speed
			ocean_param.time_scale			= 0.8f;
			// A scale to control the amplitude. Not the world space height
			ocean_param.wave_amplitude		= 0.0035f;
			// 2D wind direction. No need to be normalized
			ocean_param.wind_dir			= float2(0.8f, 0.6f);
			// The bigger the wind speed, the larger scale of wave crest.
			// But the wave scale can be no larger than patch_length
			ocean_param.wind_speed			= 6;
			// Damp out the components opposite to wind direction.
			// The smaller the value, the higher wind dependency
			ocean_param.wind_dependency		= 0.07f;
			// Control the scale of horizontal movement. Higher value creates
			// pointy crests.
			ocean_param.choppy_scale		= 1.3f;

			ocean_simulator_.reset(new OceanSimulator(ocean_param));

			checked_pointer_cast<RenderOcean>(renderable_)->DisplacementMap(ocean_simulator_->DisplacementTex());
			checked_pointer_cast<RenderOcean>(renderable_)->GradientMap(ocean_simulator_->GradientTex());
		}

		void SunDirection(float3 const & dir)
		{
			checked_pointer_cast<RenderOcean>(renderable_)->SunDirection(dir);
		}

		void Update()
		{
			App3DFramework const & app = Context::Instance().AppInstance();
			Camera const & camera = app.ActiveCamera();

			float4x4 const & view = camera.ViewMatrix();
			float4x4 const & proj = camera.ProjMatrix();

			float4x4 vp = view * proj;
			float4x4 inv_vp = MathLib::inverse(vp);
			float4x4 inv_view = MathLib::inverse(view);

			Plane upper_bound = ocean_plane_;
			upper_bound.d() += strength_;
			Plane lower_bound = ocean_plane_;
			lower_bound.d() -= strength_;

			float3 frustum[8];
			frustum[0] = MathLib::transform_coord(float3(-1, +1, 1), inv_vp);
			frustum[1] = MathLib::transform_coord(float3(+1, +1, 1), inv_vp);
			frustum[2] = MathLib::transform_coord(float3(-1, -1, 1), inv_vp);
			frustum[3] = MathLib::transform_coord(float3(+1, -1, 1), inv_vp);
			frustum[4] = MathLib::transform_coord(float3(-1, +1, 0), inv_vp);
			frustum[5] = MathLib::transform_coord(float3(+1, +1, 0), inv_vp);
			frustum[6] = MathLib::transform_coord(float3(-1, -1, 0), inv_vp);
			frustum[7] = MathLib::transform_coord(float3(+1, -1, 0), inv_vp);

			int const view_cube[24] =
			{
				0, 1, 1, 3, 3, 2, 2, 0,
				4, 5, 5, 7, 7, 6, 6, 4,
				0, 4, 1, 5, 3, 7, 2, 6
			};

			float3 min_p(1e10f, 1e10f, 1e10f), max_p(-1e10f, -1e10f, -1e10f);
			std::vector<float3> intersect;
			for (int i = 0; i < 12; ++ i)
			{
				int src = view_cube[i * 2 + 0];
				int dst = view_cube[i * 2 + 1];
				if (MathLib::dot_coord(upper_bound, frustum[src]) / MathLib::dot_coord(upper_bound, frustum[dst]) < 0)
				{
					float t = MathLib::intersect_ray(upper_bound, frustum[src], frustum[dst] - frustum[src]);
					float3 p = MathLib::lerp(frustum[src], frustum[dst], t);
					min_p = MathLib::minimize(min_p, p);
					max_p = MathLib::maximize(max_p, p);
					intersect.push_back(p);
				}
				if (MathLib::dot_coord(lower_bound, frustum[src]) / MathLib::dot_coord(lower_bound, frustum[dst]) < 0)
				{
					float t = MathLib::intersect_ray(lower_bound, frustum[src], frustum[dst] - frustum[src]);
					float3 p = MathLib::lerp(frustum[src], frustum[dst], t);
					min_p = MathLib::minimize(min_p, p);
					max_p = MathLib::maximize(max_p, p);
					intersect.push_back(p);
				}
			}
			for (int i = 0; i < 8; ++ i)
			{
				if (MathLib::dot_coord(upper_bound, frustum[i]) / MathLib::dot_coord(lower_bound, frustum[i]) < 0)
				{
					intersect.push_back(frustum[i]);
				}
			}

			if (!intersect.empty())
			{
				ocean_simulator_->Update();

				float3 eye = camera.EyePos();
				float3 forward = camera.ViewVec();
				float3 virtual_eye = eye;

				float height_in_plane = MathLib::dot_normal(lower_bound, eye);
				bool underwater = (height_in_plane < ocean_plane_.d());
				if (height_in_plane < strength_)
				{
					virtual_eye += lower_bound.Normal() * (strength_ - height_in_plane * (underwater ? 2 : 1));
				}

				// aim the projector at the point where the camera view-vector intersects the plane
				// if the camera is aimed away from the plane, mirror it's view-vector against the plane
				float3 aim_point;
				if ((MathLib::dot_normal(ocean_plane_, forward) < 0) ^ (MathLib::dot_coord(ocean_plane_, eye) < 0))
				{
					float t = MathLib::intersect_ray(ocean_plane_, eye, forward);
					aim_point = eye + t * forward;
				}
				else
				{
					float3 flipped = forward - 2 * ocean_plane_.Normal() * MathLib::dot(forward, ocean_plane_.Normal());
					float t = MathLib::intersect_ray(ocean_plane_, eye, flipped);
					aim_point = eye + t * flipped;
				}

				// force the point the camera is looking at in a plane, and have the projector look at it
				// works well against horizon, even when camera is looking upwards
				// doesn't work straight down/up
				float af = abs(MathLib::dot_normal(ocean_plane_, forward));
				float3 aim_point2 = eye + 10 * forward;
				aim_point2 = aim_point2 - ocean_plane_.Normal() * MathLib::dot(aim_point2, ocean_plane_.Normal());

				// fade between aim_point & aim_point2 depending on view angle

				aim_point = MathLib::lerp(aim_point2, aim_point, af);

				float3 up = MathLib::transform_normal(float3(0, 1, 0), inv_view);

				float4x4 virtual_view = MathLib::look_at_lh(virtual_eye, aim_point, up);
				float4x4 virtual_vp = virtual_view * proj;
				float4x4 inv_virtual_vp = MathLib::inverse(virtual_vp);

				Box box(min_p, max_p);
				box.Max().y() += strength_;

				float x_min = 1e10f, y_min = 1e10f, x_max = -1e10f, y_max = -1e10f;
				for (size_t i = 0; i < intersect.size(); ++ i)
				{
					intersect[i] -= ocean_plane_.Normal() * MathLib::dot_normal(ocean_plane_, intersect[i]);
					float3 pt = MathLib::transform_coord(intersect[i], virtual_vp);
					x_min = std::min(x_min, pt.x());
					x_max = std::max(x_max, pt.x());
					y_min = std::min(y_min, pt.y());
					y_max = std::max(y_max, pt.y());
				}

				std::vector<float4> corners(4);
				corners[0] = float4(x_min, y_max, 0, 1);
				corners[1] = float4(x_max, y_max, 0, 1);
				corners[2] = float4(x_min, y_min, 0, 1);
				corners[3] = float4(x_max, y_min, 0, 1);
				for (int i = 0; i < 4; ++ i)
				{
					float4 origin = MathLib::transform(float4(corners[i].x(), corners[i].y(), -1, 1), inv_virtual_vp);
					float4 direction = MathLib::transform(float4(0, 0, 2, 0), inv_virtual_vp);

					corners[i] = origin + direction * (ocean_plane_.d() - origin.y()) / direction.y();
				}

				checked_pointer_cast<RenderOcean>(renderable_)->SetBox(box);
				checked_pointer_cast<RenderOcean>(renderable_)->SetCorners(corners);
			}
		}

		void SkyboxTex(TexturePtr const & tex)
		{
			checked_pointer_cast<RenderOcean>(renderable_)->SkyboxTex(tex);
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

	private:
		float strength_;
		Plane ocean_plane_;
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
	// ½¨Á¢×ÖÌå
	font_ = Context::Instance().RenderFactoryInstance().MakeFont("gkai00mp.kfont");

	TexturePtr skybox_tex = LoadTexture("Langholmen.dds", EAH_GPU_Read)();

	ocean_.reset(new OceanObject);
	checked_pointer_cast<OceanObject>(ocean_)->SkyboxTex(skybox_tex);
	ocean_->AddToSceneManager();
	sun_flare_.reset(new SunObject);
	sun_flare_->AddToSceneManager();

	checked_pointer_cast<OceanObject>(ocean_)->SunDirection(checked_pointer_cast<SunObject>(sun_flare_)->Direction());

	this->LookAt(float3(0, 20, 0), float3(0, 19.8f, 1));
	this->Proj(0.01f, 3000);

	RenderFactory& rf = Context::Instance().RenderFactoryInstance();

	sky_box_ = MakeSharedPtr<SceneObjectSkyBox>();
	checked_pointer_cast<SceneObjectSkyBox>(sky_box_)->Technique(rf.LoadEffect("Ocean.fxml")->TechniqueByName("FoggySkyBox"));
	checked_pointer_cast<SceneObjectSkyBox>(sky_box_)->CubeMap(skybox_tex);
	sky_box_->AddToSceneManager();

	fpcController_.AttachCamera(this->ActiveCamera());
	fpcController_.Scalers(0.05f, 1.0f);

	InputEngine& inputEngine(Context::Instance().InputFactoryInstance().InputEngineInstance());
	InputActionMap actionMap;
	actionMap.AddActions(actions, actions + sizeof(actions) / sizeof(actions[0]));

	action_handler_t input_handler(new input_signal);
	input_handler->connect(boost::bind(&OceanApp::InputHandler, this, _1, _2));
	inputEngine.ActionMap(actionMap, input_handler, true);

	Camera& scene_camera = this->ActiveCamera();
	reflection_fb_ = rf.MakeFrameBuffer();
	reflection_fb_->GetViewport().camera->ProjParams(scene_camera.FOV(), scene_camera.Aspect(),
			scene_camera.NearPlane(), scene_camera.FarPlane());

	UIManager::Instance().Load(ResLoader::Instance().Load("Ocean.uiml"));
}

void OceanApp::OnResize(uint32_t width, uint32_t height)
{
	App3DFramework::OnResize(width, height);

	RenderFactory& rf = Context::Instance().RenderFactoryInstance();
	reflection_tex_ = rf.MakeTexture2D(width, height, 1, 1, EF_ABGR16F, 1, 0, EAH_GPU_Read | EAH_GPU_Write, NULL);
	reflection_fb_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*reflection_tex_, 0, 0));
	reflection_fb_->Attach(FrameBuffer::ATT_DepthStencil, rf.MakeDepthStencilRenderView(width, height, EF_D16, 1, 0));
	reflection_fb_->GetViewport().left = 0;
	reflection_fb_->GetViewport().top = 0;
	reflection_fb_->GetViewport().width = width;
	reflection_fb_->GetViewport().height = height;

	checked_pointer_cast<OceanObject>(ocean_)->ReflectionTex(reflection_tex_);

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

void OceanApp::DoUpdateOverlay()
{
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

	UIManager::Instance().Render();
}

uint32_t OceanApp::DoUpdate(uint32_t pass)
{
	RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();

	switch (pass)
	{
	case 0:
		{
			Camera& scene_camera = this->ActiveCamera();

			re.BindFrameBuffer(reflection_fb_);
			re.CurFrameBuffer()->Clear(FrameBuffer::CBM_Depth, Color(0, 0, 0, 1), 1, 0);
			ocean_->Visible(false);

			float3 reflect_eye, reflect_at, reflect_up;
			checked_pointer_cast<OceanObject>(ocean_)->ReflectViewParams(reflect_eye, reflect_at, reflect_up,
				scene_camera.EyePos(), scene_camera.LookAt(), scene_camera.UpVec());
			reflection_fb_->GetViewport().camera->ViewParams(reflect_eye, reflect_at, reflect_up);
		}

		return App3DFramework::URV_Need_Flush;

	default:
		re.BindFrameBuffer(FrameBufferPtr());
		re.CurFrameBuffer()->Clear(FrameBuffer::CBM_Depth, Color(0, 0, 0, 1), 1, 0);
		ocean_->Visible(true);

		return App3DFramework::URV_Need_Flush | App3DFramework::URV_Finished;
	}
}
