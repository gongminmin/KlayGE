#include <KlayGE/KlayGE.hpp>
#include <KFL/CXX17/iterator.hpp>
#include <KFL/Util.hpp>
#include <KFL/Math.hpp>
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
#include <KlayGE/GraphicsBuffer.hpp>
#include <KlayGE/SceneObjectHelper.hpp>
#include <KlayGE/UI.hpp>
#include <KlayGE/Camera.hpp>
#include <KlayGE/DeferredRenderingLayer.hpp>
#include <KFL/XMLDom.hpp>
#include <KlayGE/Window.hpp>

#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/InputFactory.hpp>
#include <KlayGE/ScriptFactory.hpp>

#include <vector>
#include <sstream>
#include <fstream>
#if defined(KLAYGE_COMPILER_CLANGC2)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable" // Ignore unused variable (mpl_assertion_in_line_xxx) in boost
#endif
#include <boost/lexical_cast.hpp>
#if defined(KLAYGE_COMPILER_CLANGC2)
#pragma clang diagnostic pop
#endif
#if defined(KLAYGE_COMPILER_CLANGC2)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable" // Ignore unused variable (mpl_assertion_in_line_xxx) in boost
#endif
#include <boost/algorithm/string/split.hpp>
#if defined(KLAYGE_COMPILER_CLANGC2)
#pragma clang diagnostic pop
#endif
#include <boost/algorithm/string/trim.hpp>

#include "SampleCommon.hpp"
#include "ScenePlayer.hpp"

using namespace std;
using namespace KlayGE;

namespace
{
	class PyScriptUpdate
	{
	public:
		explicit PyScriptUpdate(std::string const & script)
		{
			ScriptEngine& scriptEngine = Context::Instance().ScriptFactoryInstance().ScriptEngineInstance();
			module_ = scriptEngine.CreateModule("");
			module_->RunString("from ScenePlayer import *");

			script_ = MakeSharedPtr<std::string>();
			*script_ = boost::algorithm::trim_copy(script);
		}

		virtual ~PyScriptUpdate()
		{
		}

		std::any Run(float app_time, float elapsed_time)
		{
			module_->RunString(*script_);

			return module_->Call("update", { app_time, elapsed_time });
		}

	private:
		std::shared_ptr<ScriptModule> module_;
		std::shared_ptr<std::string> script_;
	};

	class LightSourceUpdate : public PyScriptUpdate
	{
	public:
		explicit LightSourceUpdate(std::string const & script)
			: PyScriptUpdate(script)
		{
		}

		void operator()(LightSource& light, float app_time, float elapsed_time)
		{
			std::any py_ret = this->Run(app_time, elapsed_time);
			if (std::any_cast<std::vector<std::any>>(&py_ret) != nullptr)
			{
				std::vector<std::any> ret = std::any_cast<std::vector<std::any>>(py_ret);
				size_t s = ret.size();

				if (s > 0)
				{
					std::any py_mat = ret[0];
					if (std::any_cast<std::vector<std::any>>(&py_mat) != nullptr)
					{
						std::vector<std::any> mat = std::any_cast<std::vector<std::any>>(py_mat);
						if (!mat.empty())
						{
							float4x4 light_mat;
							for (int i = 0; i < 16; ++ i)
							{
								light_mat[i] = std::any_cast<float>(mat[i]);
							}
							light.ModelMatrix(light_mat);
						}
					}
				}
				if (s > 1)
				{
					std::any py_clr = ret[1];
					if (std::any_cast<std::vector<std::any>>(&py_clr) != nullptr)
					{
						std::vector<std::any> clr = std::any_cast<std::vector<std::any>>(py_clr);
						if (!clr.empty())
						{
							float3 light_clr;
							for (int i = 0; i < 3; ++ i)
							{
								light_clr[i] = std::any_cast<float>(clr[i]);
							}
							light.Color(light_clr);
						}
					}
				}				
				if (s > 2)
				{
					std::any py_fo = ret[2];
					if (std::any_cast<std::vector<std::any>>(&py_fo) != nullptr)
					{
						std::vector<std::any> fo = std::any_cast<std::vector<std::any>>(py_fo);
						if (!fo.empty())
						{
							float3 light_fall_off;
							for (int i = 0; i < 3; ++ i)
							{
								light_fall_off[i] = std::any_cast<float>(fo[i]);
							}
							light.Falloff(light_fall_off);
						}
					}
				}
				if (s > 3)
				{
					std::any py_oi = ret[3];
					if (std::any_cast<std::vector<std::any>>(&py_oi) != nullptr)
					{
						std::vector<std::any> oi = std::any_cast<std::vector<std::any>>(py_oi);
						if (!oi.empty())
						{
							float2 light_outer_inner;
							for (int i = 0; i < 2; ++ i)
							{
								light_outer_inner[i] = std::any_cast<float>(oi[i]);
							}
							light.OuterAngle(light_outer_inner.x());
							light.InnerAngle(light_outer_inner.y());
						}
					}
				}
			}
		}
	};

	class SceneObjectUpdate : public PyScriptUpdate
	{
	public:
		explicit SceneObjectUpdate(std::string const & script)
			: PyScriptUpdate(script)
		{
		}

		void operator()(SceneObject& obj, float app_time, float elapsed_time)
		{
			std::any py_ret = this->Run(app_time, elapsed_time);
			if (std::any_cast<std::vector<std::any>>(&py_ret) != nullptr)
			{
				std::vector<std::any> ret = std::any_cast<std::vector<std::any>>(py_ret);
				size_t s = ret.size();

				if (s > 0)
				{
					std::any py_mat = ret[0];
					if (std::any_cast<std::vector<std::any>>(&py_mat) != nullptr)
					{
						std::vector<std::any> mat = std::any_cast<std::vector<std::any>>(py_mat);
						if (!mat.empty())
						{
							float4x4 obj_mat;
							for (int i = 0; i < 16; ++ i)
							{
								obj_mat[i] = std::any_cast<float>(mat[i]);
							}
							obj.ModelMatrix(obj_mat);
						}
					}
				}
			}
		}
	};

	class CameraUpdate : public PyScriptUpdate
	{
	public:
		explicit CameraUpdate(std::string const & script)
			: PyScriptUpdate(script)
		{
		}

		void operator()(Camera& camera, float app_time, float elapsed_time)
		{
			std::any py_ret = this->Run(app_time, elapsed_time);
			if (std::any_cast<std::vector<std::any>>(&py_ret) != nullptr)
			{
				std::vector<std::any> ret = std::any_cast<std::vector<std::any>>(py_ret);
				size_t s = ret.size();

				float3 cam_eye = camera.EyePos();
				float3 cam_lookat = camera.LookAt();
				float3 cam_up = camera.UpVec();
				float cam_fov = camera.FOV();
				float cam_aspect = camera.Aspect();
				float cam_np = camera.NearPlane();
				float cam_fp = camera.FarPlane();
				
				if (s > 0)
				{
					std::any py_eye = ret[0];
					if (std::any_cast<std::vector<std::any>>(&py_eye) != nullptr)
					{
						std::vector<std::any> eye = std::any_cast<std::vector<std::any>>(py_eye);
						if (!eye.empty())
						{
							for (int i = 0; i < 3; ++ i)
							{
								cam_eye[i] = std::any_cast<float>(eye[i]);
							}
						}
					}
				}
				if (s > 1)
				{
					std::any py_lookat = ret[1];
					if (std::any_cast<std::vector<std::any>>(&py_lookat) != nullptr)
					{
						std::vector<std::any> lookat = std::any_cast<std::vector<std::any>>(py_lookat);
						if (!lookat.empty())
						{
							for (int i = 0; i < 3; ++ i)
							{
								cam_lookat[i] = std::any_cast<float>(lookat[i]);
							}
						}
					}
				}
				if (s > 2)
				{
					std::any py_up = ret[2];
					if (std::any_cast<std::vector<std::any>>(&py_up) != nullptr)
					{
						std::vector<std::any> up = std::any_cast<std::vector<std::any>>(py_up);
						if (!up.empty())
						{
							for (int i = 0; i < 3; ++ i)
							{
								cam_up[i] = std::any_cast<float>(up[i]);
							}
						}
					}
				}
				if (s > 3)
				{
					std::any py_np = ret[3];
					if (std::any_cast<float>(&py_np) != nullptr)
					{
						cam_np = std::any_cast<float>(py_np);
					}
				}
				if (s > 4)
				{
					std::any py_fp = ret[3];
					if (std::any_cast<float>(&py_fp) != nullptr)
					{
						cam_fp = std::any_cast<float>(py_fp);
					}
				}

				camera.ViewParams(cam_eye, cam_lookat, cam_up);
				camera.ProjParams(cam_fov, cam_aspect, cam_np, cam_fp);
			}
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
}

int SampleMain()
{
	ContextCfg cfg = Context::Instance().Config();
	cfg.deferred_rendering = true;
	cfg.graphics_cfg.fft_lens_effects = true;
	Context::Instance().Config(cfg);

	ScenePlayerApp app;
	app.Create();
	app.Run();

	return 0;
}

ScenePlayerApp::ScenePlayerApp()
			: App3DFramework("ScenePlayer"),
				il_scale_(1.0f)
{
	ResLoader::Instance().AddPath("../../Samples/media/ScenePlayer");
}

void ScenePlayerApp::LoadScene(std::string const & name)
{
	Context& context = Context::Instance();
	SceneManager& sceneMgr(context.SceneManagerInstance());
	sceneMgr.ClearLight();
	sceneMgr.ClearObject();

	RenderFactory& rf = context.RenderFactoryInstance();

	scene_models_.clear();
	scene_objs_.clear();
	sky_box_.reset();

	lights_.clear();
	light_proxies_.clear();

	ResIdentifierPtr ifs = ResLoader::Instance().Open(name.c_str());

	KlayGE::XMLDocument doc;
	XMLNodePtr root = doc.Parse(ifs);

	{
		XMLAttributePtr attr = root->Attrib("skybox");
		if (attr)
		{
			sky_box_ = MakeSharedPtr<SceneObjectSkyBox>();

			std::string const skybox_name = std::string(attr->ValueString());
			if (!ResLoader::Instance().Locate(skybox_name).empty())
			{
				checked_pointer_cast<SceneObjectSkyBox>(sky_box_)->CubeMap(ASyncLoadTexture(skybox_name,
					EAH_GPU_Read | EAH_Immutable));
			}
			else if (!ResLoader::Instance().Locate(skybox_name + ".dds").empty())
			{
				checked_pointer_cast<SceneObjectSkyBox>(sky_box_)->CubeMap(ASyncLoadTexture(skybox_name + ".dds",
					EAH_GPU_Read | EAH_Immutable));
			}
			else if (!ResLoader::Instance().Locate(skybox_name + "_y.dds").empty())
			{
				checked_pointer_cast<SceneObjectSkyBox>(sky_box_)->CompressedCubeMap(
					ASyncLoadTexture(skybox_name + "_y.dds", EAH_GPU_Read | EAH_Immutable),
					ASyncLoadTexture(skybox_name + "_c.dds", EAH_GPU_Read | EAH_Immutable));
			}
			else
			{
				std::istringstream attr_ss(skybox_name);
				Color color(0, 0, 0, 1);
				attr_ss >> color.r() >> color.g() >> color.b();

				uint32_t texel;
				ElementFormat fmt;
				if (rf.RenderEngineInstance().DeviceCaps().texture_format_support(EF_ABGR8))
				{
					fmt = EF_ABGR8;
					texel = color.ABGR();
				}
				else
				{
					BOOST_ASSERT(rf.RenderEngineInstance().DeviceCaps().texture_format_support(EF_ARGB8));

					fmt = EF_ARGB8;
					texel = color.ARGB();
				}
				ElementInitData init_data[6];
				for (int i = 0; i < 6; ++ i)
				{
					init_data[i].data = &texel;
					init_data[i].row_pitch = sizeof(uint32_t);
					init_data[i].slice_pitch = init_data[i].row_pitch;
				}

				checked_pointer_cast<SceneObjectSkyBox>(sky_box_)->CubeMap(rf.MakeTextureCube(1, 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_Immutable, init_data));
			}

			sky_box_->AddToSceneManager();
		}
	}

	for (XMLNodePtr light_node = root->FirstNode("light"); light_node; light_node = light_node->NextSibling("light"))
	{
		LightSourcePtr light;

		std::string_view const lt_str = light_node->Attrib("type")->ValueString();
		if ("ambient" == lt_str)
		{
			light = MakeSharedPtr<AmbientLightSource>();
		}
		else if ("directional" == lt_str)
		{
			light = MakeSharedPtr<DirectionalLightSource>();
		}
		else if ("point" == lt_str)
		{
			light = MakeSharedPtr<PointLightSource>();
		}
		else if ("spot" == lt_str)
		{
			light = MakeSharedPtr<SpotLightSource>();
		}
		else if ("sphere_area" == lt_str)
		{
			light = MakeSharedPtr<SphereAreaLightSource>();
		}
		else
		{
			BOOST_ASSERT("tube_area" == lt_str);
			light = MakeSharedPtr<TubeAreaLightSource>();
		}

		uint32_t light_attr = 0;
		XMLNodePtr attr_node = light_node->FirstNode("attr");
		if (attr_node)
		{
			std::string_view const attr_str = attr_node->Attrib("value")->ValueString();
			std::vector<std::string> tokens;
			boost::algorithm::split(tokens, attr_str, boost::is_any_of(" \t|"));
			for (auto& token : tokens)
			{
				boost::algorithm::trim(token);

				if ("no_shadow" == token)
				{
					light_attr |= LightSource::LSA_NoShadow;
				}
				else if ("no_diffuse" == token)
				{
					light_attr |= LightSource::LSA_NoDiffuse;
				}
				else if ("no_specular" == token)
				{
					light_attr |= LightSource::LSA_NoSpecular;
				}
				else if ("indirect" == token)
				{
					light_attr |= LightSource::LSA_IndirectLighting;
				}
			}
		}
		light->Attrib(light_attr);

		XMLNodePtr color_node = light_node->FirstNode("color");
		if (color_node)
		{
			std::istringstream attr_ss(std::string(color_node->Attrib("v")->ValueString()));
			float3 color;
			attr_ss >> color.x() >> color.y() >> color.z();
			light->Color(color);
		}


		if (light->Type() != LightSource::LT_Ambient)
		{
			XMLNodePtr dir_node = light_node->FirstNode("dir");
			if (dir_node)
			{
				std::istringstream attr_ss(std::string(dir_node->Attrib("v")->ValueString()));
				float3 dir;
				attr_ss >> dir.x() >> dir.y() >> dir.z();
				light->Direction(dir);
			}
		}
		if ((LightSource::LT_Point == light->Type()) || (LightSource::LT_Spot == light->Type())
			|| (LightSource::LT_SphereArea == light->Type()) || (LightSource::LT_TubeArea == light->Type()))
		{
			XMLNodePtr pos_node = light_node->FirstNode("pos");
			if (pos_node)
			{
				std::istringstream attr_ss(std::string(pos_node->Attrib("v")->ValueString()));
				float3 pos;
				attr_ss >> pos.x() >> pos.y() >> pos.z();
				light->Position(pos);
			}

			XMLNodePtr fall_off_node = light_node->FirstNode("fall_off");
			if (fall_off_node)
			{
				std::istringstream attr_ss(std::string(fall_off_node->Attrib("v")->ValueString()));
				float3 fall_off;
				attr_ss >> fall_off.x() >> fall_off.y() >> fall_off.z();
				light->Falloff(fall_off);
			}

			if ((LightSource::LT_Point == light->Type()) || (LightSource::LT_Spot == light->Type()))
			{
				XMLNodePtr projective_node = light_node->FirstNode("projective");
				if (projective_node)
				{
					XMLAttributePtr attr = projective_node->Attrib("name");
					if (attr)
					{
						TexturePtr projective = ASyncLoadTexture(std::string(attr->ValueString()), EAH_GPU_Read | EAH_Immutable);
						light->ProjectiveTexture(projective);
					}
				}

				if (LightSource::LT_Spot == light->Type())
				{
					XMLNodePtr angle_node = light_node->FirstNode("angle");
					if (angle_node)
					{
						light->InnerAngle(angle_node->Attrib("inner")->ValueFloat());
						light->OuterAngle(angle_node->Attrib("outer")->ValueFloat());
					}
				}
			}

			// TODO: sphere area light and tube area light
		}

		XMLNodePtr update_node = light_node->FirstNode("update");
		if (update_node)
		{
			update_node = update_node->FirstNode();
			if (update_node && (XNT_CData == update_node->Type()))
			{
				std::string const update_script = std::string(update_node->ValueString());
				if (!update_script.empty())
				{
					light->BindUpdateFunc(LightSourceUpdate(update_script));
				}
			}
		}

		light->AddToSceneManager();
		lights_.push_back(light);

		XMLNodePtr scale_node = light_node->FirstNode("scale");
		if (scale_node)
		{
			float3 scale(1, 1, 1);
			{
				std::istringstream attr_ss(std::string(scale_node->Attrib("v")->ValueString()));
				attr_ss >> scale.x() >> scale.y() >> scale.z();
			}

			SceneObjectPtr light_proxy = MakeSharedPtr<SceneObjectLightSourceProxy>(light);
			checked_pointer_cast<SceneObjectLightSourceProxy>(light_proxy)->Scaling(scale);
			light_proxy->AddToSceneManager();

			light_proxies_.push_back(light_proxy);
		}
	}

	for (XMLNodePtr model_node = root->FirstNode("model"); model_node; model_node = model_node->NextSibling("model"))
	{
		uint32_t obj_attr = SceneObject::SOA_Cullable;
		float4x4 obj_mat = float4x4::Identity();

		float3 scale(1, 1, 1);
		Quaternion rotate = Quaternion::Identity();
		float3 translate(0, 0, 0);

		XMLNodePtr scale_node = model_node->FirstNode("scale");
		if (scale_node)
		{
			std::istringstream attr_ss(std::string(scale_node->Attrib("v")->ValueString()));
			attr_ss >> scale.x() >> scale.y() >> scale.z();
		}
		
		XMLNodePtr rotate_node = model_node->FirstNode("rotate");
		if (!!rotate_node)
		{
			std::istringstream attr_ss(std::string(rotate_node->Attrib("v")->ValueString()));
			attr_ss >> rotate.x() >> rotate.y() >> rotate.z() >> rotate.w();
		}

		XMLNodePtr translate_node = model_node->FirstNode("translate");
		if (scale_node)
		{
			std::istringstream attr_ss(std::string(translate_node->Attrib("v")->ValueString()));
			attr_ss >> translate.x() >> translate.y() >> translate.z();
		}

		obj_mat = MathLib::transformation<float>(nullptr, nullptr, &scale, nullptr, &rotate, &translate);

		XMLNodePtr attribute_node = model_node->FirstNode("attr");
		if (attribute_node)
		{
			XMLAttributePtr attr = attribute_node->Attrib("value");
			if (attr)
			{
				if (!attr->TryConvert(obj_attr))
				{
					obj_attr = SceneObject::SOA_Cullable;

					std::string_view const attr_str = attr->ValueString();
					std::vector<std::string> tokens;
					boost::algorithm::split(tokens, attr_str, boost::is_any_of(" \t|"));
					for (auto& token : tokens)
					{
						boost::algorithm::trim(token);
						if ("cullable" == token)
						{
							obj_attr |= SceneObject::SOA_Cullable;
						}
						else if ("overlay" == token)
						{
							obj_attr |= SceneObject::SOA_Overlay;
						}
						else if ("moveable" == token)
						{
							obj_attr |= SceneObject::SOA_Moveable;
						}
						else if ("invisible" == token)
						{
							obj_attr |= SceneObject::SOA_Invisible;
						}
					}
				}
			}
		}

		std::string update_script;
		XMLNodePtr update_node = model_node->FirstNode("update");
		if (update_node)
		{
			update_node = update_node->FirstNode();
			if (update_node && (XNT_CData == update_node->Type()))
			{
				update_script = update_node->ValueString();
			}
		}

		XMLAttributePtr attr = model_node->Attrib("meshml");
		BOOST_ASSERT(attr);

		RenderModelPtr model = ASyncLoadModel(std::string(attr->ValueString()), EAH_GPU_Read | EAH_Immutable);
		scene_models_.push_back(model);
		SceneObjectPtr scene_obj = MakeSharedPtr<SceneObjectHelper>(model, obj_attr);
		scene_obj->ModelMatrix(obj_mat);
		if (!update_script.empty())
		{
			scene_obj->BindSubThreadUpdateFunc(SceneObjectUpdate(update_script));
		}
		scene_objs_.push_back(scene_obj);
		scene_obj->AddToSceneManager();
	}

	{
		float3 eye_pos(0, 0, -1);
		float3 look_at(0, 0, 0);
		float3 up(0, 1, 0);
		float fov = PI / 4;
		float near_plane = 1;
		float far_plane = 1000;

		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		FrameBuffer& fb = *re.CurFrameBuffer();
		float aspect = static_cast<float>(fb.Width()) / fb.Height();

		XMLNodePtr camera_node = root->FirstNode("camera");

		XMLNodePtr eye_pos_node = camera_node->FirstNode("eye_pos");
		if (eye_pos_node)
		{
			std::istringstream attr_ss(std::string(eye_pos_node->Attrib("v")->ValueString()));
			attr_ss >> eye_pos.x() >> eye_pos.y() >> eye_pos.z();
		}
		XMLNodePtr look_at_node = camera_node->FirstNode("look_at");
		if (look_at_node)
		{
			std::istringstream attr_ss(std::string(look_at_node->Attrib("v")->ValueString()));
			attr_ss >> look_at.x() >> look_at.y() >> look_at.z();
		}
		XMLNodePtr up_node = camera_node->FirstNode("up");
		if (up_node)
		{
			std::istringstream attr_ss(std::string(up_node->Attrib("v")->ValueString()));
			attr_ss >> up.x() >> up.y() >> up.z();
		}

		XMLNodePtr fov_node = camera_node->FirstNode("fov");
		if (fov_node)
		{
			fov = fov_node->Attrib("s")->ValueFloat();
		}
		XMLNodePtr aspect_node = camera_node->FirstNode("aspect");
		if (aspect_node)
		{
			aspect = aspect_node->Attrib("s")->ValueFloat();
		}
		XMLNodePtr near_plane_node = camera_node->FirstNode("near");
		if (near_plane_node)
		{
			near_plane = near_plane_node->Attrib("s")->ValueFloat();
		}
		XMLNodePtr far_plane_node = camera_node->FirstNode("far");
		if (far_plane_node)
		{
			far_plane = far_plane_node->Attrib("s")->ValueFloat();
		}

		std::string update_script;

		XMLNodePtr update_node = camera_node->FirstNode("update");
		if (update_node)
		{
			update_node = update_node->FirstNode();
			if (update_node && (XNT_CData == update_node->Type()))
			{
				update_script = update_node->ValueString();
			}
		}

		auto& camera = this->ActiveCamera();
		camera.ViewParams(eye_pos, look_at, up);
		camera.ProjParams(fov, aspect, near_plane, far_plane);
		if (!update_script.empty())
		{
			camera.BindUpdateFunc(CameraUpdate(update_script));
		}
	}
}

void ScenePlayerApp::OnCreate()
{
	this->LoadScene("DeferredRendering.kges");

	font_ = SyncLoadFont("gkai00mp.kfont");

	deferred_rendering_ = Context::Instance().DeferredRenderingLayerInstance();

	fpcController_.Scalers(0.05f, 0.5f);

	InputEngine& inputEngine(Context::Instance().InputFactoryInstance().InputEngineInstance());
	InputActionMap actionMap;
	actionMap.AddActions(actions, actions + std::size(actions));

	action_handler_t input_handler = MakeSharedPtr<input_signal>();
	input_handler->connect(
		[this](InputEngine const & sender, InputAction const & action)
		{
			this->InputHandler(sender, action);
		});
	inputEngine.ActionMap(actionMap, input_handler);

	UIManager::Instance().Load(ResLoader::Instance().Open("ScenePlayer.uiml"));
	dialog_ = UIManager::Instance().GetDialogs()[0];

	id_open_ = dialog_->IDFromName("Open");
	id_illum_combo_ = dialog_->IDFromName("IllumCombo");
	id_il_scale_static_ = dialog_->IDFromName("ILScaleStatic");
	id_il_scale_slider_ = dialog_->IDFromName("ILScaleSlider");
	id_ssgi_ = dialog_->IDFromName("SSGI");
	id_ssvo_ = dialog_->IDFromName("SSVO");
	id_hdr_ = dialog_->IDFromName("HDR");
	id_aa_ = dialog_->IDFromName("AA");
	id_cg_ = dialog_->IDFromName("CG");
	id_ctrl_camera_ = dialog_->IDFromName("CtrlCamera");

	dialog_->Control<UIButton>(id_open_)->OnClickedEvent().connect(
		[this](UIButton const & sender)
		{
			this->OpenHandler(sender);
		});

	dialog_->Control<UIComboBox>(id_illum_combo_)->OnSelectionChangedEvent().connect(
		[this](UIComboBox const & sender)
		{
			this->IllumChangedHandler(sender);
		});
	this->IllumChangedHandler(*dialog_->Control<UIComboBox>(id_illum_combo_));

	dialog_->Control<UISlider>(id_il_scale_slider_)->SetValue(static_cast<int>(il_scale_ * 10));
	dialog_->Control<UISlider>(id_il_scale_slider_)->OnValueChangedEvent().connect(
		[this](UISlider const & sender)
		{
			this->ILScaleChangedHandler(sender);
		});
	this->ILScaleChangedHandler(*dialog_->Control<UISlider>(id_il_scale_slider_));

	dialog_->Control<UICheckBox>(id_ssgi_)->OnChangedEvent().connect(
		[this](UICheckBox const & sender)
		{
			this->SSGIHandler(sender);
		});
	this->SSGIHandler(*dialog_->Control<UICheckBox>(id_ssgi_));

	dialog_->Control<UICheckBox>(id_ssvo_)->OnChangedEvent().connect(
		[this](UICheckBox const & sender)
		{
			this->SSVOHandler(sender);
		});
	this->SSVOHandler(*dialog_->Control<UICheckBox>(id_ssvo_));

	dialog_->Control<UICheckBox>(id_hdr_)->OnChangedEvent().connect(
		[this](UICheckBox const & sender)
		{
			this->HDRHandler(sender);
		});
	this->HDRHandler(*dialog_->Control<UICheckBox>(id_hdr_));

	dialog_->Control<UICheckBox>(id_aa_)->OnChangedEvent().connect(
		[this](UICheckBox const & sender)
		{
			this->AAHandler(sender);
		});
	this->AAHandler(*dialog_->Control<UICheckBox>(id_aa_));

	dialog_->Control<UICheckBox>(id_cg_)->OnChangedEvent().connect(
		[this](UICheckBox const & sender)
		{
			this->ColorGradingHandler(sender);
		});
	this->ColorGradingHandler(*dialog_->Control<UICheckBox>(id_cg_));

	dialog_->Control<UICheckBox>(id_ctrl_camera_)->OnChangedEvent().connect(
		[this](UICheckBox const & sender)
		{
			this->CtrlCameraHandler(sender);
		});
}

void ScenePlayerApp::OnResize(uint32_t width, uint32_t height)
{
	App3DFramework::OnResize(width, height);

	RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
	deferred_rendering_->SetupViewport(0, re.CurFrameBuffer(), 0);

	UIManager::Instance().SettleCtrls();
}

void ScenePlayerApp::InputHandler(InputEngine const & /*sender*/, InputAction const & action)
{
	switch (action.first)
	{
	case Exit:
		this->Quit();
		break;
	}
}

void ScenePlayerApp::OpenHandler(UIButton const & /*sender*/)
{
#if defined KLAYGE_PLATFORM_WINDOWS_DESKTOP
	OPENFILENAMEA ofn;
	char fn[260];
	HWND hwnd = this->MainWnd()->HWnd();

	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFile = fn;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(fn);
	ofn.lpstrFilter = "Scene File\0*.kges\0All\0*.*\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = nullptr;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = nullptr;
	ofn.Flags = OFN_NOCHANGEDIR | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

	if (GetOpenFileNameA(&ofn))
	{
		HCURSOR cur = GetCursor();
		SetCursor(LoadCursor(nullptr, IDC_WAIT));

		if (last_file_path_.empty())
		{
			ResLoader::Instance().DelPath(last_file_path_);
		}

		std::string file_name = fn;
		last_file_path_ = file_name.substr(0, file_name.find_last_of('\\'));
		ResLoader::Instance().AddPath(last_file_path_);

		this->LoadScene(file_name);

		SetCursor(cur);
	}
#endif
}

void ScenePlayerApp::IllumChangedHandler(UIComboBox const & sender)
{
	deferred_rendering_->DisplayIllum(sender.GetSelectedIndex());
}

void ScenePlayerApp::ILScaleChangedHandler(KlayGE::UISlider const & sender)
{
	il_scale_ = sender.GetValue() / 10.0f;
	deferred_rendering_->IndirectScale(il_scale_);

	std::wostringstream stream;
	stream << L"Scale: " << il_scale_ << " x";
	dialog_->Control<UIStatic>(id_il_scale_static_)->SetText(stream.str());
}

void ScenePlayerApp::SSGIHandler(UICheckBox const & sender)
{
	deferred_rendering_->SSGIEnabled(0, sender.GetChecked());
}

void ScenePlayerApp::SSVOHandler(UICheckBox const & sender)
{
	deferred_rendering_->SSVOEnabled(0, sender.GetChecked());
}

void ScenePlayerApp::HDRHandler(UICheckBox const & sender)
{
	Context::Instance().RenderFactoryInstance().RenderEngineInstance().HDREnabled(sender.GetChecked());
}

void ScenePlayerApp::AAHandler(UICheckBox const & sender)
{
	Context::Instance().RenderFactoryInstance().RenderEngineInstance().PPAAEnabled(sender.GetChecked() ? 1 : 0);
}

void ScenePlayerApp::ColorGradingHandler(UICheckBox const & sender)
{
	Context::Instance().RenderFactoryInstance().RenderEngineInstance().ColorGradingEnabled(sender.GetChecked());
}

void ScenePlayerApp::CtrlCameraHandler(UICheckBox const & sender)
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

void ScenePlayerApp::DoUpdateOverlay()
{
	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

	UIManager::Instance().Render();

	font_->RenderText(0, 0, Color(1, 1, 0, 1), L"Scene Player", 16);
	font_->RenderText(0, 18, Color(1, 1, 0, 1), renderEngine.ScreenFrameBuffer()->Description(), 16);

	std::wostringstream stream;
	stream.precision(2);
	stream << std::fixed << this->FPS() << " FPS";
	font_->RenderText(0, 36, Color(1, 1, 0, 1), stream.str(), 16);
}

uint32_t ScenePlayerApp::DoUpdate(uint32_t pass)
{
	return deferred_rendering_->Update(pass);
}
