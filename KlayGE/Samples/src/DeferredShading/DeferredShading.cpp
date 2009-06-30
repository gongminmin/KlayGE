#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/Util.hpp>
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
#include <KlayGE/KMesh.hpp>
#include <KlayGE/SceneObjectHelper.hpp>
#include <KlayGE/PostProcess.hpp>
#include <KlayGE/HDRPostProcess.hpp>
#include <KlayGE/Util.hpp>

#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/InputFactory.hpp>

#include <sstream>
#include <ctime>
#include <boost/bind.hpp>
#include <boost/typeof/typeof.hpp>
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable: 4702)
#endif
#include <boost/lexical_cast.hpp>
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(pop)
#endif

#include "DeferredShading.hpp"

using namespace std;
using namespace KlayGE;

namespace
{
	class RenderTorus : public KMesh
	{
	public:
		RenderTorus(RenderModelPtr const & model, std::wstring const & name)
			: KMesh(model, name),
				gen_sm_pass_(false)
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();

			effect_ = rf.LoadEffect("GBuffer.fxml");
		}

		void BuildMeshInfo()
		{
			std::map<std::string, TexturePtr> tex_pool;

			RenderModel::Material const & mtl = model_.lock()->GetMaterial(this->MaterialID());

			bool has_diffuse_map = false;
			bool has_bump_map = false;
			RenderModel::TextureSlotsType const & texture_slots = mtl.texture_slots;
			for (RenderModel::TextureSlotsType::const_iterator iter = texture_slots.begin();
				iter != texture_slots.end(); ++ iter)
			{
				TexturePtr tex;
				BOOST_AUTO(titer, tex_pool.find(iter->second));
				if (titer != tex_pool.end())
				{
					tex = titer->second;
				}
				else
				{
					tex = LoadTexture(iter->second, EAH_GPU_Read)();
					tex_pool.insert(std::make_pair(iter->second, tex));
				}
				has_diffuse_map = tex;

				if ("Diffuse Color" == iter->first)
				{
					*(effect_->ParameterByName("diffuse_tex")) = tex;
				}
				if ("Bump" == iter->first)
				{
					*(effect_->ParameterByName("bump_tex")) = tex;
					if (tex)
					{
						has_bump_map = true;
					}
				}
			}

			*(effect_->ParameterByName("diffuse_clr")) = float4(mtl.diffuse.x(), mtl.diffuse.y(), mtl.diffuse.z(), has_diffuse_map);

			if (has_bump_map)
			{
				gbuffer_technique_ = effect_->TechniqueByName("GBufferTech");
			}
			else
			{
				gbuffer_technique_ = effect_->TechniqueByName("GBufferNoBumpTech");
			}

			gen_sm_technique_ = effect_->TechniqueByName("GenShadowMap");
		}

		void GenShadowMapPass(bool sm_pass)
		{
			gen_sm_pass_ = sm_pass;
			if (gen_sm_pass_)
			{
				technique_ = gen_sm_technique_;
			}
			else
			{
				technique_ = gbuffer_technique_;
			}
		}

		void LightPosDir(float3 const & pos, float3 const & dir)
		{
			obj_to_light_model_ = MathLib::look_at_lh(pos, pos + dir, float3(0, 1, 0));
		}

		void OnRenderBegin()
		{
			Camera const & camera = Context::Instance().AppInstance().ActiveCamera();

			float4x4 const & view = camera.ViewMatrix();
			float4x4 const & proj = camera.ProjMatrix();

			if (gen_sm_pass_)
			{
				*(effect_->ParameterByName("mvp")) = view * proj;
				*(effect_->ParameterByName("obj_to_light_model")) = obj_to_light_model_;
			}
			else
			{
				*(effect_->ParameterByName("proj")) = proj;
				*(effect_->ParameterByName("model_view")) = view;

				*(effect_->ParameterByName("depth_near_far_invfar")) = float3(camera.NearPlane(), camera.FarPlane(), 1 / camera.FarPlane());
			}
		}

	private:
		KlayGE::RenderEffectPtr effect_;
		bool gen_sm_pass_;
		RenderTechniquePtr gen_sm_technique_;
		RenderTechniquePtr gbuffer_technique_;
		float4x4 obj_to_light_model_;
	};

	class TorusObject : public SceneObjectHelper
	{
	public:
		TorusObject()
			: SceneObjectHelper(SOA_Cullable)
		{
			renderable_ = LoadModel("sponza.meshml", EAH_GPU_Read, CreateKModelFactory<RenderModel>(), CreateKMeshFactory<RenderTorus>());
		}

		void GenShadowMapPass(bool sm_pass)
		{
			RenderModelPtr const & model = checked_pointer_cast<RenderModel>(renderable_);
			for (uint32_t i = 0; i < model->NumMeshes(); ++ i)
			{
				checked_pointer_cast<RenderTorus>(model->Mesh(i))->GenShadowMapPass(sm_pass);
			}
		}

		void LightPosDir(float3 const & pos, float3 const & dir)
		{
			RenderModelPtr const & model = checked_pointer_cast<RenderModel>(renderable_);
			for (uint32_t i = 0; i < model->NumMeshes(); ++ i)
			{
				checked_pointer_cast<RenderTorus>(model->Mesh(i))->LightPosDir(pos, dir);
			}
		}
	};


	class RenderCone : public KMesh
	{
	public:
		RenderCone(RenderModelPtr const & model, std::wstring const & name)
			: KMesh(model, name)
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();

			technique_ = rf.LoadEffect("GBuffer.fxml")->TechniqueByName("GBufferNoTexTech");
		}

		void BuildMeshInfo()
		{
		}

		void ModelMatrix(float4x4 const & mat)
		{
			model_ = mat;
		}

		void OnRenderBegin()
		{
			Camera const & camera = Context::Instance().AppInstance().ActiveCamera();

			float4x4 const & view = camera.ViewMatrix();
			float4x4 const & proj = camera.ProjMatrix();

			*(technique_->Effect().ParameterByName("proj")) = proj;
			*(technique_->Effect().ParameterByName("model_view")) = model_ * view;

			*(technique_->Effect().ParameterByName("depth_near_far_invfar")) = float3(camera.NearPlane(), camera.FarPlane(), 1 / camera.FarPlane());
		}

	private:
		float4x4 model_;
	};

	class ConeObject : public SceneObjectHelper
	{
	public:
		ConeObject(std::string const & model_name, float org_angle, float rot_speed, float height)
			: SceneObjectHelper(SOA_Cullable), rot_speed_(rot_speed), height_(height)
		{
			renderable_ = LoadModel(model_name, EAH_GPU_Read, CreateKModelFactory<RenderModel>(), CreateKMeshFactory<RenderCone>())->Mesh(0);
			model_org_ = MathLib::rotation_x(org_angle);
		}

		void Update()
		{
			model_ = MathLib::scaling(0.1f, 0.1f, 0.1f) * model_org_ * MathLib::rotation_y(std::clock() * rot_speed_) * MathLib::translation(0.0f, height_, 0.0f);
			checked_pointer_cast<RenderCone>(renderable_)->ModelMatrix(model_);
		}

		float4x4 const & ModelMatrix() const
		{
			return model_;
		}

	private:
		float4x4 model_;
		float4x4 model_org_;
		float rot_speed_, height_;
	};


	class DeferredShadingPostProcess : public PostProcess
	{
	public:
		DeferredShadingPostProcess()
			: PostProcess(RenderTechniquePtr()),
				buffer_type_(0)
		{
			if (Context::Instance().RenderFactoryInstance().RenderEngineInstance().DeviceCaps().max_shader_model < 4)
			{
				max_num_lights_a_batch_ = 1;
			}
			else
			{
				max_num_lights_a_batch_ = 8;
			}

			std::pair<std::string, std::string> macros[] = { std::make_pair("MAX_NUM_LIGHTS", ""), std::make_pair("", "") };
			macros[0].second = boost::lexical_cast<std::string>(max_num_lights_a_batch_);
			this->Technique(Context::Instance().RenderFactoryInstance().LoadEffect("DeferredShading.fxml", macros)->TechniqueByName("DeferredShading"));

			technique_wo_blend_ = technique_->Effect().TechniqueByName("DeferredShading");
			technique_w_blend_ = technique_->Effect().TechniqueByName("DeferredShadingBlend");
		}

		int AddPointLight(float3 const & pos, float3 const & clr, float3 const & falloff)
		{
			int id = static_cast<int>(light_clr_type_.size());
			light_enabled_.push_back(1);
			light_clr_type_.push_back(float4(clr.x(), clr.y(), clr.z(), 0.0f));
			light_pos_.push_back(float4(pos.x(), pos.y(), pos.z(), 0));
			light_dir_.push_back(float4(0, 0, 0, 0));
			light_falloff_.push_back(float4(falloff.x(), falloff.y(), falloff.z(), 0));
			light_cos_outer_inner_.push_back(float2(0, 0));
			sm_tex_.push_back(TexturePtr());
			return id;
		}
		int AddDirectionalLight(float3 const & dir, float3 const & clr, float3 const & falloff)
		{
			float3 d = MathLib::normalize(dir);
			int id = static_cast<int>(light_clr_type_.size());
			light_enabled_.push_back(1);
			light_clr_type_.push_back(float4(clr.x(), clr.y(), clr.z(), 1.0f));
			light_pos_.push_back(float4(0, 0, 0, 0));
			light_dir_.push_back(float4(d.x(), d.y(), d.z(), 0));
			light_falloff_.push_back(float4(falloff.x(), falloff.y(), falloff.z(), 0));
			light_cos_outer_inner_.push_back(float2(0, 0));
			sm_tex_.push_back(TexturePtr());
			return id;
		}
		int AddSpotLight(float3 const & pos, float3 const & dir, float cos_outer, float cos_inner, float3 const & clr, float3 const & falloff)
		{
			float3 d = MathLib::normalize(dir);
			int id = static_cast<int>(light_clr_type_.size());
			light_enabled_.push_back(1);
			light_clr_type_.push_back(float4(clr.x(), clr.y(), clr.z(), 2.0f));
			light_pos_.push_back(float4(pos.x(), pos.y(), pos.z(), 0));
			light_dir_.push_back(float4(d.x(), d.y(), d.z(), 0));
			light_falloff_.push_back(float4(falloff.x(), falloff.y(), falloff.z(), 0));
			light_cos_outer_inner_.push_back(float2(cos_outer, cos_inner));
			sm_tex_.push_back(TexturePtr());
			return id;
		}

		void LightColor(int index, float3 const & clr)
		{
			light_clr_type_[index] = float4(clr.x(), clr.y(), clr.z(), light_clr_type_[index].w());
		}
		void LightDir(int index, float3 const & dir)
		{
			float3 d = MathLib::normalize(dir);
			light_dir_[index] = float4(d.x(), d.y(), d.z(), 0);
		}
		void LightPos(int index, float3 const & pos)
		{
			light_pos_[index] = float4(pos.x(), pos.y(), pos.z(), 0);
		}
		void LightFalloff(int index, float3 const & falloff)
		{
			light_falloff_[index] = float4(falloff.x(), falloff.y(), falloff.z(), 0);
		}
		void SpotLightAngle(int index, float cos_outer, float cos_inner)
		{
			light_cos_outer_inner_[index] = float2(cos_outer, cos_inner);
		}
		void ShadowMapTex(int index, TexturePtr const & tex, bool flip)
		{
			sm_tex_[index] = tex;
			*(technique_->Effect().ParameterByName("flip")) = static_cast<int32_t>(flip ? -1 : 1);
		}

		float3 LightColor(int index) const
		{
			return *reinterpret_cast<float3 const *>(&light_clr_type_[index]);
		}
		float3 LightDir(int index) const
		{
			return *reinterpret_cast<float3 const *>(&light_dir_[index]);
		}
		float3 LightPos(int index) const
		{
			return *reinterpret_cast<float3 const *>(&light_pos_[index]);
		}
		float3 LightFalloff(int index) const
		{
			return *reinterpret_cast<float3 const *>(&light_falloff_[index]);
		}
		float2 SpotLightAngle(int index) const
		{
			return light_cos_outer_inner_[index];
		}

		void LightEnabled(int index, bool enable)
		{
			light_enabled_[index] = enable;
		}
		bool LightEnabled(int index) const
		{
			return light_enabled_[index] != 0;
		}

		void Source(TexturePtr const & tex, bool flipping)
		{
			PostProcess::Source(tex, flipping);
			if (tex)
			{
				*(technique_->Effect().ParameterByName("inv_width_height")) = float2(1.0f / tex->Width(0), 1.0f / tex->Height(0));
			}
		}

		void ColorTex(TexturePtr const & tex)
		{
			*(technique_->Effect().ParameterByName("color_tex")) = tex;
		}

		void SSAOTex(TexturePtr const & tex)
		{
			*(technique_->Effect().ParameterByName("ssao_tex")) = tex;
		}

		void SSAOEnabled(bool ssao)
		{
			*(technique_->Effect().ParameterByName("ssao_enabled")) = ssao;
		}

		void BufferType(int buffer_type)
		{
			buffer_type_ = buffer_type;
			switch (buffer_type_)
			{
			case 0:
				technique_wo_blend_ = technique_->Effect().TechniqueByName("DeferredShading");
				technique_w_blend_ = technique_->Effect().TechniqueByName("DeferredShadingBlend");
				break;

			case 1:
				technique_ = technique_->Effect().TechniqueByName("ShowPosition");
				break;

			case 2:
				technique_ = technique_->Effect().TechniqueByName("ShowNormal");
				break;

			case 3:
				technique_ = technique_->Effect().TechniqueByName("ShowDepth");
				break;

			case 4:
				technique_ = technique_->Effect().TechniqueByName("ShowDiffuse");
				break;

			case 5:
				technique_ = technique_->Effect().TechniqueByName("ShowSpecular");
				break;

			case 6:
				technique_ = technique_->Effect().TechniqueByName("ShowEdge");
				break;

			case 7:
				technique_ = technique_->Effect().TechniqueByName("ShowSSAO");
				break;

			default:
				break;
			}
		}

		void Render()
		{
			Camera const & camera = Context::Instance().AppInstance().ActiveCamera();

			float4x4 const & view = camera.ViewMatrix();
			float4x4 const inv_proj = MathLib::inverse(camera.ProjMatrix());
			float4x4 const inv_view = MathLib::inverse(view);

			*(technique_->Effect().ParameterByName("depth_near_far_invfar")) = float3(camera.NearPlane(), camera.FarPlane(), 1 / camera.FarPlane());

			*(technique_->Effect().ParameterByName("upper_left")) = MathLib::transform_coord(float3(-1, 1, 1), inv_proj);
			*(technique_->Effect().ParameterByName("upper_right")) = MathLib::transform_coord(float3(1, 1, 1), inv_proj);
			*(technique_->Effect().ParameterByName("lower_left")) = MathLib::transform_coord(float3(-1, -1, 1), inv_proj);
			*(technique_->Effect().ParameterByName("lower_right")) = MathLib::transform_coord(float3(1, -1, 1), inv_proj);

			if (0 == buffer_type_)
			{
				int32_t num_lights = 0;
				light_clr_type_enabled_.resize(0);
				light_cos_outer_inner_enabled_.resize(0);
				light_falloff_enabled_.resize(0);
				light_pos_enabled_.resize(0);
				light_rotation_enabled_.resize(0);
				sm_tex_enabled_.resize(0);
				for (size_t i = 0; i < light_clr_type_.size(); ++ i)
				{
					if (light_enabled_[i])
					{
						light_clr_type_enabled_.push_back(light_clr_type_[i]);
						light_cos_outer_inner_enabled_.push_back(light_cos_outer_inner_[i]);
						light_falloff_enabled_.push_back(light_falloff_[i]);
						sm_tex_enabled_.push_back(sm_tex_[i]);

						int type = static_cast<int>(light_clr_type_[i].w() + 0.1f);

						float4x4 light_model;
						switch (type)
						{
						case 0:
							{
								float3 eye = *reinterpret_cast<float3*>(&light_pos_[i]);
								light_model = MathLib::translation(-eye);
							}
							break;

						case 1:
							{
								float3 eye(0, 0, 0);
								float3 at = *reinterpret_cast<float3*>(&light_dir_[i]);
								light_model = MathLib::look_at_lh(eye, at, float3(0, 1, 0));
							}
							break;

						case 2:
							{
								float3 eye = *reinterpret_cast<float3*>(&light_pos_[i]);
								float3 at = *reinterpret_cast<float3*>(&light_pos_[i]) + *reinterpret_cast<float3*>(&light_dir_[i]);
								light_model = MathLib::look_at_lh(eye, at, float3(0, 1, 0));
							}
							break;
						}

						float4x4 mat = inv_view * light_model;
						float3 scale, trans;
						Quaternion rot;
						MathLib::decompose(scale, rot, trans, mat);
						light_pos_enabled_.push_back(float4(trans.x(), trans.y(), trans.z(), scale.x()));
						light_rotation_enabled_.push_back(float4(rot.x(), rot.y(), rot.z(), rot.w()));
						if (2 == type)
						{
							float fov = acos(light_cos_outer_inner_[i].x()) * 2;
							light_proj_.push_back(MathLib::perspective_fov_lh(fov, 1.0f, 0.1f, 100.0f));
						}
						else
						{
							light_proj_.push_back(float4x4());
						}

						++ num_lights;
					}
				}

				int32_t start = 0;
				while (num_lights > 0)
				{
					int32_t n = std::min(num_lights, max_num_lights_a_batch_);

					*(technique_->Effect().ParameterByName("num_lights")) = n;
					*(technique_->Effect().ParameterByName("light_clr_type")) = std::vector<float4>(&light_clr_type_enabled_[start], &light_clr_type_enabled_[start] + n);
					*(technique_->Effect().ParameterByName("light_cos_outer_inner")) = std::vector<float2>(&light_cos_outer_inner_enabled_[start], &light_cos_outer_inner_enabled_[start] + n);
					*(technique_->Effect().ParameterByName("light_falloff")) = std::vector<float4>(&light_falloff_enabled_[start], &light_falloff_enabled_[start] + n);
					*(technique_->Effect().ParameterByName("light_pos")) = std::vector<float4>(&light_pos_enabled_[start], &light_pos_enabled_[start] + n);
					*(technique_->Effect().ParameterByName("light_rotation")) = std::vector<float4>(&light_rotation_enabled_[start], &light_rotation_enabled_[start] + n);
					*(technique_->Effect().ParameterByName("light_proj")) = std::vector<float4x4>(&light_proj_[start], &light_proj_[start] + n);

					for (int i = 0; i < n; ++ i)
					{
						std::stringstream ss;
						ss << "shadow_map_" << i << "_tex";
						*(technique_->Effect().ParameterByName(ss.str())) = sm_tex_enabled_[i];
					}

					if (0 == start)
					{
						technique_ = technique_wo_blend_;
					}
					else
					{
						technique_ = technique_w_blend_;
					}

					PostProcess::Render();

					num_lights -= n;
					start += n;
				}
			}
			else
			{
				PostProcess::Render();
			}
		}

	private:
		std::vector<char> light_enabled_;
		std::vector<float4> light_clr_type_;
		std::vector<float4> light_pos_;
		std::vector<float4> light_dir_;
		std::vector<float2> light_cos_outer_inner_;
		std::vector<float4> light_falloff_;
		std::vector<float4x4> light_proj_;
		std::vector<TexturePtr> sm_tex_;

		std::vector<float4> light_clr_type_enabled_;
		std::vector<float2> light_cos_outer_inner_enabled_;
		std::vector<float4> light_falloff_enabled_;
		std::vector<float4> light_pos_enabled_;
		std::vector<float4> light_rotation_enabled_;
		std::vector<TexturePtr> sm_tex_enabled_;

		RenderTechniquePtr technique_wo_blend_;
		RenderTechniquePtr technique_w_blend_;

		int32_t max_num_lights_a_batch_;
		int32_t buffer_type_;
	};

	class AdaptiveAntiAliasPostProcess : public PostProcess
	{
	public:
		AdaptiveAntiAliasPostProcess()
			: PostProcess(Context::Instance().RenderFactoryInstance().LoadEffect("AdaptiveAntiAliasPP.fxml")->TechniqueByName("AdaptiveAntiAlias"))
		{
		}

		void Source(TexturePtr const & tex, bool flipping)
		{
			PostProcess::Source(tex, flipping);
			if (tex)
			{
				*(technique_->Effect().ParameterByName("inv_width_height")) = float2(1.0f / tex->Width(0), 1.0f / tex->Height(0));
			}
		}

		void ColorTex(TexturePtr const & tex)
		{
			*(technique_->Effect().ParameterByName("color_tex")) = tex;
		}
	};

	class SSAOPostProcess : public PostProcess
	{
	public:
		SSAOPostProcess()
			: PostProcess(Context::Instance().RenderFactoryInstance().LoadEffect("SSAOPP.fxml")->TechniqueByName("SSAO"))
		{
			*(technique_->Effect().ParameterByName("ssao_param")) = float4(0.6f, 0.075f, 0.3f, 0.03f);
		}

		void OnRenderBegin()
		{
			PostProcess::OnRenderBegin();

			RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
			Camera const & camera = Context::Instance().AppInstance().ActiveCamera();
			*(technique_->Effect().ParameterByName("depth_near_far_invfar")) = float3(camera.NearPlane(), camera.FarPlane(), 1 / camera.FarPlane());
			*(technique_->Effect().ParameterByName("tex_width_height")) = float2(static_cast<float>(re.CurFrameBuffer()->Width()), static_cast<float>(re.CurFrameBuffer()->Height()));
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

	bool ConfirmDevice()
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		RenderEngine& re = rf.RenderEngineInstance();
		RenderDeviceCaps const & caps = re.DeviceCaps();
		if (caps.max_shader_model < 2)
		{
			return false;
		}
		if (caps.max_simultaneous_rts < 2)
		{
			return false;
		}

		try
		{
			TexturePtr temp_tex = rf.MakeTexture2D(800, 600, 1, EF_ABGR16F, 1, 0, EAH_GPU_Read | EAH_GPU_Write, NULL);
			rf.Make2DRenderView(*temp_tex, 0);
			rf.MakeDepthStencilRenderView(800, 600, EF_D16, 1, 0);
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
	ResLoader::Instance().AddPath("../Samples/media/Common");
	ResLoader::Instance().AddPath("../Samples/media/DeferredShading");

	RenderSettings settings = Context::Instance().LoadCfg("KlayGE.cfg");
	settings.ConfirmDevice = ConfirmDevice;

	DeferredShadingApp app("DeferredShading", settings);
	app.Create();
	app.Run();

	return 0;
}

DeferredShadingApp::DeferredShadingApp(std::string const & name, RenderSettings const & settings)
			: App3DFramework(name, settings),
				anti_alias_enabled_(true), ssao_enabled_(true)
{
}

void DeferredShadingApp::InitObjects()
{
	font_ = Context::Instance().RenderFactoryInstance().MakeFont("gkai00mp.kfont");

	torus_ = MakeSharedPtr<TorusObject>();
	torus_->AddToSceneManager();

	light_src_[0] = MakeSharedPtr<ConeObject>("cone_60.meshml", PI / 2, 1 / 1400.0f, 2.0f);
	light_src_[1] = MakeSharedPtr<ConeObject>("cone_90.meshml", -PI / 2, -1 / 700.0f, 1.7f);
	light_src_[0]->AddToSceneManager();
	light_src_[1]->AddToSceneManager();

	this->LookAt(float3(-2, 2, 0), float3(0, 2, 0));
	this->Proj(0.1f, 100.0f);

	RenderFactory& rf = Context::Instance().RenderFactoryInstance();
	RenderEngine& re = rf.RenderEngineInstance();

	TexturePtr y_cube_map = LoadTexture("Lake_CraterLake03_y.dds", EAH_GPU_Read)();
	TexturePtr c_cube_map = LoadTexture("Lake_CraterLake03_c.dds", EAH_GPU_Read)();
	sky_box_ = MakeSharedPtr<SceneObjectHDRSkyBox>();
	checked_pointer_cast<SceneObjectHDRSkyBox>(sky_box_)->CompressedCubeMap(y_cube_map, c_cube_map);
	sky_box_->AddToSceneManager();
	
	g_buffer_ = rf.MakeFrameBuffer();
	g_buffer_->GetViewport().camera = re.CurFrameBuffer()->GetViewport().camera;

	shaded_buffer_ = rf.MakeFrameBuffer();
	shaded_buffer_->GetViewport().camera = re.CurFrameBuffer()->GetViewport().camera;

	ssao_buffer_ = rf.MakeFrameBuffer();
	ssao_buffer_->GetViewport().camera = re.CurFrameBuffer()->GetViewport().camera;

	blur_ssao_buffer_ = rf.MakeFrameBuffer();
	blur_ssao_buffer_->GetViewport().camera = re.CurFrameBuffer()->GetViewport().camera;

	hdr_buffer_ = rf.MakeFrameBuffer();
	hdr_buffer_->GetViewport().camera = re.CurFrameBuffer()->GetViewport().camera;

	fpcController_.Scalers(0.05f, 0.5f);

	InputEngine& inputEngine(Context::Instance().InputFactoryInstance().InputEngineInstance());
	InputActionMap actionMap;
	actionMap.AddActions(actions, actions + sizeof(actions) / sizeof(actions[0]));

	action_handler_t input_handler = MakeSharedPtr<input_signal>();
	input_handler->connect(boost::bind(&DeferredShadingApp::InputHandler, this, _1, _2));
	inputEngine.ActionMap(actionMap, input_handler, true);

	deferred_shading_ = MakeSharedPtr<DeferredShadingPostProcess>();
	edge_anti_alias_ = MakeSharedPtr<AdaptiveAntiAliasPostProcess>();
	ssao_pp_ = MakeSharedPtr<SSAOPostProcess>();
	blur_pp_ = MakeSharedPtr<BlurPostProcess>(8, 1.0f);
	hdr_pp_ = MakeSharedPtr<HDRPostProcess>(true, false);

	RenderViewPtr ds_view = rf.MakeDepthStencilRenderView(512, 512, EF_D16, 1, 0);
	for (int i = 0; i < 2; ++ i)
	{
		sm_tex_[i] = rf.MakeTexture2D(512, 512, 1, EF_GR16F, 1, 0, EAH_GPU_Read | EAH_GPU_Write, NULL);

		sm_buffer_[i] = rf.MakeFrameBuffer();
		sm_buffer_[i]->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*sm_tex_[i], 0));
		sm_buffer_[i]->Attach(FrameBuffer::ATT_DepthStencil, ds_view);
	}
	sm_buffer_[0]->GetViewport().camera->ProjParams(PI / 6 * 2, 1, 0.1f, 100.0f);
	sm_buffer_[1]->GetViewport().camera->ProjParams(PI / 4 * 2, 1, 0.1f, 100.0f);

	point_light_id_ = checked_pointer_cast<DeferredShadingPostProcess>(deferred_shading_)->AddPointLight(float3(2, 10, 0), float3(0.2f, 0.2f, 0.2f), float3(0, 0.3f, 0));
	spot_light_id_[0] = checked_pointer_cast<DeferredShadingPostProcess>(deferred_shading_)->AddSpotLight(float3(0, 0, 0), float3(0, 0, 0), cos(PI / 6), cos(PI / 8), float3(2, 0, 0), float3(0, 0.3f, 0));
	spot_light_id_[1] = checked_pointer_cast<DeferredShadingPostProcess>(deferred_shading_)->AddSpotLight(float3(0, 0, 0), float3(0, 0, 0), cos(PI / 4), cos(PI / 6), float3(0, 2, 0), float3(0, 0.3f, 0));
	checked_pointer_cast<DeferredShadingPostProcess>(deferred_shading_)->ShadowMapTex(spot_light_id_[0], sm_tex_[0], sm_buffer_[0]->RequiresFlipping());
	checked_pointer_cast<DeferredShadingPostProcess>(deferred_shading_)->ShadowMapTex(spot_light_id_[1], sm_tex_[1], sm_buffer_[1]->RequiresFlipping());

	UIManager::Instance().Load(ResLoader::Instance().Load("DeferredShading.uiml"));
	dialog_ = UIManager::Instance().GetDialogs()[0];

	id_buffer_combo_ = dialog_->IDFromName("BufferCombo");
	id_anti_alias_ = dialog_->IDFromName("AntiAlias");
	id_ssao_ = dialog_->IDFromName("SSAO");
	id_ctrl_camera_ = dialog_->IDFromName("CtrlCamera");

	dialog_->Control<UIComboBox>(id_buffer_combo_)->OnSelectionChangedEvent().connect(boost::bind(&DeferredShadingApp::BufferChangedHandler, this, _1));
	this->BufferChangedHandler(*dialog_->Control<UIComboBox>(id_buffer_combo_));

	dialog_->Control<UICheckBox>(id_anti_alias_)->OnChangedEvent().connect(boost::bind(&DeferredShadingApp::AntiAliasHandler, this, _1));
	DeferredShadingApp::AntiAliasHandler(*dialog_->Control<UICheckBox>(id_anti_alias_));
	dialog_->Control<UICheckBox>(id_ssao_)->OnChangedEvent().connect(boost::bind(&DeferredShadingApp::SSAOHandler, this, _1));
	DeferredShadingApp::SSAOHandler(*dialog_->Control<UICheckBox>(id_ssao_));
	dialog_->Control<UICheckBox>(id_ctrl_camera_)->OnChangedEvent().connect(boost::bind(&DeferredShadingApp::CtrlCameraHandler, this, _1));
	DeferredShadingApp::CtrlCameraHandler(*dialog_->Control<UICheckBox>(id_ctrl_camera_));
}

void DeferredShadingApp::OnResize(uint32_t width, uint32_t height)
{
	App3DFramework::OnResize(width, height);

	RenderFactory& rf = Context::Instance().RenderFactoryInstance();
	diffuse_specular_tex_ = rf.MakeTexture2D(width, height, 1, EF_ABGR16F, 1, 0, EAH_GPU_Read | EAH_GPU_Write, NULL);
	normal_depth_tex_ = rf.MakeTexture2D(width, height, 1, EF_ABGR16F, 1, 0, EAH_GPU_Read | EAH_GPU_Write, NULL);
	g_buffer_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*diffuse_specular_tex_, 0));
	g_buffer_->Attach(FrameBuffer::ATT_Color1, rf.Make2DRenderView(*normal_depth_tex_, 0));
	g_buffer_->Attach(FrameBuffer::ATT_DepthStencil, rf.MakeDepthStencilRenderView(width, height, EF_D16, 1, 0));

	try
	{
		ssao_tex_ = rf.MakeTexture2D(width, height, 1, EF_R16F, 1, 0, EAH_GPU_Read | EAH_GPU_Write, NULL);
	}
	catch (...)
	{
		ssao_tex_ = rf.MakeTexture2D(width, height, 1, EF_ABGR16F, 1, 0, EAH_GPU_Read | EAH_GPU_Write, NULL);
	}
	ssao_buffer_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*ssao_tex_, 0));

	blur_ssao_tex_ = rf.MakeTexture2D(width, height, 1, ssao_tex_->Format(), 1, 0, EAH_GPU_Read | EAH_GPU_Write, NULL);
	blur_ssao_buffer_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*blur_ssao_tex_, 0));

	try
	{
		hdr_tex_ = rf.MakeTexture2D(width, height, 1, EF_B10G11R11F, 1, 0, EAH_GPU_Read | EAH_GPU_Write, NULL);
		hdr_buffer_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*hdr_tex_, 0));
	}
	catch (...)
	{
		hdr_tex_ = rf.MakeTexture2D(width, height, 1, EF_ABGR16F, 1, 0, EAH_GPU_Read | EAH_GPU_Write, NULL);
		hdr_buffer_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*hdr_tex_, 0));
	}

	shaded_tex_ = rf.MakeTexture2D(width, height, 1, hdr_tex_->Format(), 1, 0, EAH_GPU_Read | EAH_GPU_Write, NULL);
	shaded_buffer_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*shaded_tex_, 0));

	deferred_shading_->Source(normal_depth_tex_, g_buffer_->RequiresFlipping());
	checked_pointer_cast<DeferredShadingPostProcess>(deferred_shading_)->ColorTex(diffuse_specular_tex_);
	checked_pointer_cast<DeferredShadingPostProcess>(deferred_shading_)->SSAOTex(blur_ssao_tex_);
	deferred_shading_->Destinate(shaded_buffer_);

	edge_anti_alias_->Source(normal_depth_tex_, shaded_buffer_->RequiresFlipping());
	checked_pointer_cast<AdaptiveAntiAliasPostProcess>(edge_anti_alias_)->ColorTex(shaded_tex_);
	edge_anti_alias_->Destinate(hdr_buffer_);
	//edge_anti_alias_->Destinate(FrameBufferPtr());

	hdr_pp_->Source(hdr_tex_, hdr_buffer_->RequiresFlipping());
	hdr_pp_->Destinate(FrameBufferPtr());

	ssao_pp_->Source(normal_depth_tex_, g_buffer_->RequiresFlipping());
	ssao_pp_->Destinate(ssao_buffer_);

	blur_pp_->Source(ssao_tex_, ssao_buffer_->RequiresFlipping());
	blur_pp_->Destinate(blur_ssao_buffer_);

	UIManager::Instance().SettleCtrls(width, height);
}

void DeferredShadingApp::InputHandler(InputEngine const & /*sender*/, InputAction const & action)
{
	switch (action.first)
	{
	case Exit:
		this->Quit();
		break;
	}
}

void DeferredShadingApp::BufferChangedHandler(KlayGE::UIComboBox const & sender)
{
	buffer_type_ = sender.GetSelectedIndex();
	checked_pointer_cast<DeferredShadingPostProcess>(deferred_shading_)->BufferType(buffer_type_);

	if (buffer_type_ != 0)
	{
		dialog_->Control<UICheckBox>(id_anti_alias_)->SetChecked(false);
		anti_alias_enabled_ = false;
		deferred_shading_->Destinate(FrameBufferPtr());
	}
}

void DeferredShadingApp::AntiAliasHandler(KlayGE::UICheckBox const & sender)
{
	if (0 == buffer_type_)
	{
		anti_alias_enabled_ = sender.GetChecked();
		if (anti_alias_enabled_)
		{
			deferred_shading_->Destinate(shaded_buffer_);
			edge_anti_alias_->Destinate(hdr_buffer_);
			//edge_anti_alias_->Destinate(FrameBufferPtr());
		}
		else
		{
			deferred_shading_->Destinate(hdr_buffer_);
			//deferred_shading_->Destinate(FrameBufferPtr());
		}
	}
}

void DeferredShadingApp::SSAOHandler(KlayGE::UICheckBox const & sender)
{
	if (0 == buffer_type_)
	{
		ssao_enabled_ = sender.GetChecked();
		checked_pointer_cast<DeferredShadingPostProcess>(deferred_shading_)->SSAOEnabled(ssao_enabled_);
	}
}

void DeferredShadingApp::CtrlCameraHandler(KlayGE::UICheckBox const & sender)
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

void DeferredShadingApp::DoUpdateOverlay()
{
	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

	UIManager::Instance().Render();

	FrameBuffer& rw = *checked_pointer_cast<FrameBuffer>(renderEngine.CurFrameBuffer());

	font_->RenderText(0, 0, Color(1, 1, 0, 1), L"Deferred Shading", 16);
	font_->RenderText(0, 18, Color(1, 1, 0, 1), rw.Description(), 16);

	std::wostringstream stream;
	stream.precision(2);
	stream << fixed << this->FPS() << " FPS";
	font_->RenderText(0, 36, Color(1, 1, 0, 1), stream.str(), 16);
}

uint32_t DeferredShadingApp::DoUpdate(uint32_t pass)
{
	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

	boost::shared_ptr<DeferredShadingPostProcess> const & ds = checked_pointer_cast<DeferredShadingPostProcess>(deferred_shading_);

	switch (pass)
	{
	case 0:
		checked_pointer_cast<TorusObject>(torus_)->GenShadowMapPass(false);

		light_src_[0]->Visible(true);
		light_src_[1]->Visible(true);

		renderEngine.BindFrameBuffer(g_buffer_);
		renderEngine.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, Color(0, 0, 1, 0), 1.0f, 0);
		return App3DFramework::URV_Need_Flush;

	case 1:
	case 2:
		{
			float4x4 model_mat = checked_pointer_cast<ConeObject>(light_src_[pass - 1])->ModelMatrix();
			float3 p = MathLib::transform_coord(float3(0, 0, 0), model_mat);
			float3 d = MathLib::normalize(MathLib::transform_normal(float3(0, -1, 0), model_mat));
			ds->LightPos(spot_light_id_[pass - 1], p);
			ds->LightDir(spot_light_id_[pass - 1], d);

			sm_buffer_[pass - 1]->GetViewport().camera->ViewParams(p, p + d, float3(0, 1, 0));
		}

		checked_pointer_cast<TorusObject>(torus_)->GenShadowMapPass(true);
		checked_pointer_cast<TorusObject>(torus_)->LightPosDir(ds->LightPos(spot_light_id_[pass - 1]), ds->LightDir(spot_light_id_[pass - 1]));

		light_src_[0]->Visible(false);
		light_src_[1]->Visible(false);

		renderEngine.BindFrameBuffer(sm_buffer_[pass - 1]);
		renderEngine.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, Color(0, 0, 0, 0), 1.0f, 0);
		return App3DFramework::URV_Need_Flush;

	default:
		renderEngine.BindFrameBuffer(FrameBufferPtr());
		renderEngine.CurFrameBuffer()->Attached(FrameBuffer::ATT_DepthStencil)->Clear(1.0f);
		if (((0 == buffer_type_) && ssao_enabled_) || (7 == buffer_type_))
		{
			ssao_pp_->Apply();
			blur_pp_->Apply();
		}
		deferred_shading_->Apply();
		if ((0 == buffer_type_) && anti_alias_enabled_)
		{
			edge_anti_alias_->Apply();
		}
		if (0 == buffer_type_)
		{
			hdr_pp_->Apply();
		}

		return App3DFramework::URV_Finished;
	}
}
