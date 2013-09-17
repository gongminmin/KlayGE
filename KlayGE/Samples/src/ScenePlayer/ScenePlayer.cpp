#include <KlayGE/KlayGE.hpp>
#include <KFL/ThrowErr.hpp>
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
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable: 4702)
#endif
#include <boost/lexical_cast.hpp>
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(pop)
#endif
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable: 4127 6328)
#endif
#include <boost/tokenizer.hpp>
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(pop)
#endif

#include "SampleCommon.hpp"
#include "ScenePlayer.hpp"

using namespace std;
using namespace KlayGE;

namespace
{
	class PyScriptUpdate
	{
	public:
		PyScriptUpdate(std::string const & script)
		{
			ScriptEngine& scriptEngine = Context::Instance().ScriptFactoryInstance().ScriptEngineInstance();
			module_ = scriptEngine.CreateModule("");
			module_->RunString("from ScenePlayer import *");

			script_ = MakeSharedPtr<std::string>(script);
		}

		virtual ~PyScriptUpdate()
		{
		}

		boost::any Run(float app_time, float elapsed_time)
		{
			module_->RunString(*script_);

			return module_->Call("update", KlayGE::make_tuple(app_time, elapsed_time));
		}

	private:
		KlayGE::shared_ptr<ScriptModule> module_;
		KlayGE::shared_ptr<std::string> script_;
	};

	class LightSourceUpdate : public PyScriptUpdate
	{
	public:
		LightSourceUpdate(std::string const & script)
			: PyScriptUpdate(script)
		{
		}

		void operator()(LightSource& light, float app_time, float elapsed_time)
		{
			boost::any py_ret = this->Run(app_time, elapsed_time);
			if (typeid(std::vector<boost::any>) == py_ret.type())
			{
				std::vector<boost::any> ret = boost::any_cast<std::vector<boost::any> >(py_ret);
				size_t s = ret.size();

				if (s > 0)
				{
					boost::any py_mat = ret[0];
					if (typeid(std::vector<boost::any>) == py_mat.type())
					{
						std::vector<boost::any> mat = boost::any_cast<std::vector<boost::any> >(py_mat);
						if (!mat.empty())
						{
							float4x4 light_mat;
							for (int i = 0; i < 16; ++ i)
							{
								light_mat[i] = boost::any_cast<float>(mat[i]);
							}
							light.ModelMatrix(light_mat);
						}
					}
				}
				if (s > 1)
				{
					boost::any py_clr = ret[1];
					if (typeid(std::vector<boost::any>) == py_clr.type())
					{
						std::vector<boost::any> clr = boost::any_cast<std::vector<boost::any> >(py_clr);
						if (!clr.empty())
						{
							float3 light_clr;
							for (int i = 0; i < 3; ++ i)
							{
								light_clr[i] = boost::any_cast<float>(clr[i]);
							}
							light.Color(light_clr);
						}
					}
				}				
				if (s > 2)
				{
					boost::any py_fo = ret[2];
					if (typeid(std::vector<boost::any>) == py_fo.type())
					{
						std::vector<boost::any> fo = boost::any_cast<std::vector<boost::any> >(py_fo);
						if (!fo.empty())
						{
							float3 light_fall_off;
							for (int i = 0; i < 3; ++ i)
							{
								light_fall_off[i] = boost::any_cast<float>(fo[i]);
							}
							light.Falloff(light_fall_off);
						}
					}
				}
				if (s > 3)
				{
					boost::any py_oi = ret[3];
					if (typeid(std::vector<boost::any>) == py_oi.type())
					{
						std::vector<boost::any> oi = boost::any_cast<std::vector<boost::any> >(py_oi);
						if (!oi.empty())
						{
							float2 light_outer_inner;
							for (int i = 0; i < 2; ++ i)
							{
								light_outer_inner[i] = boost::any_cast<float>(oi[i]);
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
		SceneObjectUpdate(std::string const & script)
			: PyScriptUpdate(script)
		{
		}

		void operator()(SceneObject& obj, float app_time, float elapsed_time)
		{
			boost::any py_ret = this->Run(app_time, elapsed_time);
			if (typeid(std::vector<boost::any>) == py_ret.type())
			{
				std::vector<boost::any> ret = boost::any_cast<std::vector<boost::any> >(py_ret);
				size_t s = ret.size();

				if (s > 0)
				{
					boost::any py_mat = ret[0];
					if (typeid(std::vector<boost::any>) == py_mat.type())
					{
						std::vector<boost::any> mat = boost::any_cast<std::vector<boost::any> >(py_mat);
						if (!mat.empty())
						{
							float4x4 obj_mat;
							for (int i = 0; i < 16; ++ i)
							{
								obj_mat[i] = boost::any_cast<float>(mat[i]);
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
		CameraUpdate(std::string const & script)
			: PyScriptUpdate(script)
		{
		}

		void operator()(Camera& camera, float app_time, float elapsed_time)
		{
			boost::any py_ret = this->Run(app_time, elapsed_time);
			if (typeid(std::vector<boost::any>) == py_ret.type())
			{
				std::vector<boost::any> ret = boost::any_cast<std::vector<boost::any> >(py_ret);
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
					boost::any py_eye = ret[0];
					if (typeid(std::vector<boost::any>) == py_eye.type())
					{
						std::vector<boost::any> eye = boost::any_cast<std::vector<boost::any> >(py_eye);
						if (!eye.empty())
						{
							for (int i = 0; i < 3; ++ i)
							{
								cam_eye[i] = boost::any_cast<float>(eye[i]);
							}
						}
					}
				}
				if (s > 1)
				{
					boost::any py_lookat = ret[1];
					if (typeid(std::vector<boost::any>) == py_lookat.type())
					{
						std::vector<boost::any> lookat = boost::any_cast<std::vector<boost::any> >(py_lookat);
						if (!lookat.empty())
						{
							for (int i = 0; i < 3; ++ i)
							{
								cam_lookat[i] = boost::any_cast<float>(lookat[i]);
							}
						}
					}
				}
				if (s > 2)
				{
					boost::any py_up = ret[2];
					if (typeid(std::vector<boost::any>) == py_up.type())
					{
						std::vector<boost::any> up = boost::any_cast<std::vector<boost::any> >(py_up);
						if (!up.empty())
						{
							for (int i = 0; i < 3; ++ i)
							{
								cam_up[i] = boost::any_cast<float>(up[i]);
							}
						}
					}
				}
				if (s > 3)
				{
					boost::any py_np = ret[3];
					if (typeid(float) == py_np.type())
					{
						cam_np = boost::any_cast<float>(py_np);
					}
				}
				if (s > 4)
				{
					boost::any py_fp = ret[3];
					if (typeid(float) == py_fp.type())
					{
						cam_fp = boost::any_cast<float>(py_fp);
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
	cfg.script_factory_name = "Python";
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

bool ScenePlayerApp::ConfirmDevice() const
{
	RenderDeviceCaps const & caps = Context::Instance().RenderFactoryInstance().RenderEngineInstance().DeviceCaps();
	if (caps.max_shader_model < 2)
	{
		return false;
	}
	return true;
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

	for (XMLNodePtr light_node = root->FirstNode("light"); light_node; light_node = light_node->NextSibling("light"))
	{
		LightSourcePtr light;

		uint32_t light_attr = 0;
		float3 light_clr(0, 0, 0);
		float3 fall_off(1, 0, 0);
		std::string update_script;
		TexturePtr projective;

		XMLNodePtr attribute_node = light_node->FirstNode("attribute");
		if (attribute_node)
		{
			XMLAttributePtr attr = attribute_node->Attrib("value");
			if (attr)
			{
				try
				{
					light_attr = attr->ValueInt();
				}
				catch (boost::bad_lexical_cast const &)
				{
					light_attr = 0;

					std::string attribute_str = attr->ValueString();
			
					boost::char_separator<char> sep("", " \t|");
					boost::tokenizer<boost::char_separator<char> > tok(attribute_str, sep);
					std::string this_token;
					for (KLAYGE_AUTO(beg, tok.begin()); beg != tok.end(); ++ beg)
					{
						this_token = *beg;
						if ("noshadow" == this_token)
						{
							light_attr |= LightSource::LSA_NoShadow;
						}
						else if ("nodiffuse" == this_token)
						{
							light_attr |= LightSource::LSA_NoDiffuse;
						}
						else if ("nospecular" == this_token)
						{
							light_attr |= LightSource::LSA_NoSpecular;
						}
						else if ("indirect" == this_token)
						{
							light_attr |= LightSource::LSA_IndirectLighting;
						}
					}
				}
			}
		}

		XMLNodePtr color_node = light_node->FirstNode("color");
		if (color_node)
		{
			XMLAttributePtr attr = color_node->Attrib("x");
			if (attr)
			{
				light_clr.x() = attr->ValueFloat();
			}

			attr = color_node->Attrib("y");
			if (attr)
			{
				light_clr.y() = attr->ValueFloat();
			}

			attr = color_node->Attrib("z");
			if (attr)
			{
				light_clr.z() = attr->ValueFloat();
			}
		}

		XMLNodePtr fall_off_node = light_node->FirstNode("fall_off");
		if (fall_off_node)
		{
			XMLAttributePtr attr = fall_off_node->Attrib("x");
			if (attr)
			{
				fall_off.x() = attr->ValueFloat();
			}

			attr = fall_off_node->Attrib("y");
			if (attr)
			{
				fall_off.y() = attr->ValueFloat();
			}

			attr = fall_off_node->Attrib("z");
			if (attr)
			{
				fall_off.z() = attr->ValueFloat();
			}
		}

		XMLNodePtr update_node = light_node->FirstNode("update");
		if (update_node)
		{
			update_node = update_node->FirstNode();
			if (update_node && (XNT_CData == update_node->Type()))
			{
				update_script = update_node->ValueString();
			}
		}

		XMLNodePtr projective_node = light_node->FirstNode("projective");
		if (projective_node)
		{
			XMLAttributePtr attr = projective_node->Attrib("name");
			if (attr)
			{
				projective = SyncLoadTexture(attr->ValueString(), EAH_GPU_Read | EAH_Immutable);
			}
		}

		XMLAttributePtr attr = light_node->Attrib("type");
		BOOST_ASSERT(attr);

		std::string type = attr->ValueString();
		if ("ambient" == type)
		{
			light = MakeSharedPtr<AmbientLightSource>();
			light->Color(light_clr);
		}
		else if ("point" == type)
		{
			float3 light_pos(0, 0, 0);

			XMLNodePtr pos_node = light_node->FirstNode("position");
			if (pos_node)
			{
				light_pos.x() = pos_node->Attrib("x")->ValueFloat();
				light_pos.y() = pos_node->Attrib("y")->ValueFloat();
				light_pos.z() = pos_node->Attrib("z")->ValueFloat();
			}

			light = MakeSharedPtr<PointLightSource>();
			light->Attrib(light_attr);
			light->Position(light_pos);
			light->Color(light_clr);
			light->Falloff(fall_off);
		}
		else if ("spot" == type)
		{
			float3 light_pos(0, 0, 0);
			float3 light_dir(0, 0, 1);
			float outer_angle = PI / 4;
			float inner_angle = PI / 6;

			XMLNodePtr pos_node = light_node->FirstNode("position");
			if (pos_node)
			{
				light_pos.x() = pos_node->Attrib("x")->ValueFloat();
				light_pos.y() = pos_node->Attrib("y")->ValueFloat();
				light_pos.z() = pos_node->Attrib("z")->ValueFloat();
			}

			XMLNodePtr dir_node = light_node->FirstNode("direction");
			if (dir_node)
			{
				light_dir.x() = dir_node->Attrib("x")->ValueFloat();
				light_dir.y() = dir_node->Attrib("y")->ValueFloat();
				light_dir.z() = dir_node->Attrib("z")->ValueFloat();
			}

			XMLNodePtr angle_node = light_node->FirstNode("angle");
			if (angle_node)
			{
				outer_angle = angle_node->Attrib("outer")->ValueFloat();
				inner_angle = angle_node->Attrib("inner")->ValueFloat();
			}

			light = MakeSharedPtr<SpotLightSource>();
			light->Attrib(light_attr);
			light->Position(light_pos);
			light->Direction(light_dir);
			light->Color(light_clr);
			light->Falloff(fall_off);
			light->OuterAngle(outer_angle);
			light->InnerAngle(inner_angle);
		}
		else
		{
			BOOST_ASSERT("directional" == type);

			float3 light_dir(0, 0, 1);

			XMLNodePtr dir_node = light_node->FirstNode("direction");
			if (dir_node)
			{
				light_dir.x() = dir_node->Attrib("x")->ValueFloat();
				light_dir.y() = dir_node->Attrib("y")->ValueFloat();
				light_dir.z() = dir_node->Attrib("z")->ValueFloat();
			}

			light = MakeSharedPtr<DirectionalLightSource>();
			light->Attrib(light_attr);
			light->Direction(light_dir);
			light->Color(light_clr);
			light->Falloff(fall_off);
		}

		if (!update_script.empty())
		{
			light->BindUpdateFunc(LightSourceUpdate(update_script));
		}

		light->ProjectiveTexture(projective);

		light->AddToSceneManager();
		lights_.push_back(light);

		XMLNodePtr proxy_node = light_node->FirstNode("proxy");
		if (proxy_node)
		{
			float scale = 1;
			XMLAttributePtr attr = proxy_node->Attrib("scale");
			if (attr)
			{
				scale = attr->ValueFloat();
			}

			SceneObjectPtr light_proxy = MakeSharedPtr<SceneObjectLightSourceProxy>(light);
			checked_pointer_cast<SceneObjectLightSourceProxy>(light_proxy)->Scaling(scale, scale, scale);
			light_proxy->AddToSceneManager();

			light_proxies_.push_back(light_proxy);
		}
	}

	for (XMLNodePtr model_node = root->FirstNode("model"); model_node; model_node = model_node->NextSibling("model"))
	{
		uint32_t obj_attr = SceneObject::SOA_Cullable;
		float4x4 obj_mat = float4x4::Identity();

		XMLNodePtr transform_node = model_node->FirstNode("transform");
		if (transform_node)
		{
			for (XMLNodePtr node = transform_node->FirstNode(); node; node = node->NextSibling())
			{
				if ("translation" == node->Name())
				{
					float x = node->Attrib("x")->ValueFloat();
					float y = node->Attrib("y")->ValueFloat();
					float z = node->Attrib("z")->ValueFloat();
					obj_mat *= MathLib::translation(x, y, z);
				}
				else if ("scaling" == node->Name())
				{
					float x = node->Attrib("x")->ValueFloat();
					float y = node->Attrib("y")->ValueFloat();
					float z = node->Attrib("z")->ValueFloat();
					obj_mat *= MathLib::scaling(x, y, z);
				}
			}
		}

		XMLNodePtr attribute_node = model_node->FirstNode("attribute");
		if (attribute_node)
		{
			XMLAttributePtr attr = attribute_node->Attrib("value");
			if (attr)
			{
				try
				{
					obj_attr = attr->ValueInt();
				}
				catch (boost::bad_lexical_cast const &)
				{
					obj_attr = SceneObject::SOA_Cullable;

					std::string attribute_str = attr->ValueString();
			
					boost::char_separator<char> sep("", " \t|");
					boost::tokenizer<boost::char_separator<char> > tok(attribute_str, sep);
					std::string this_token;
					for (KLAYGE_AUTO(beg, tok.begin()); beg != tok.end(); ++ beg)
					{
						this_token = *beg;
						if ("cullable" == this_token)
						{
							obj_attr |= SceneObject::SOA_Cullable;
						}
						else if ("overlay" == this_token)
						{
							obj_attr |= SceneObject::SOA_Overlay;
						}
						else if ("moveable" == this_token)
						{
							obj_attr |= SceneObject::SOA_Moveable;
						}
						else if ("invisible" == this_token)
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

		XMLAttributePtr attr = model_node->Attrib("name");
		BOOST_ASSERT(attr);

		RenderModelPtr model = SyncLoadModel(attr->ValueString(), EAH_GPU_Read | EAH_Immutable);
		scene_models_.push_back(model);
		for (size_t i = 0; i < model->NumMeshes(); ++ i)
		{
			SceneObjectPtr scene_obj = MakeSharedPtr<SceneObjectHelper>(model->Mesh(i), obj_attr);
			scene_obj->ModelMatrix(obj_mat);
			if (!update_script.empty())
			{
				scene_obj->BindSubThreadUpdateFunc(SceneObjectUpdate(update_script));
			}
			scene_objs_.push_back(scene_obj);
			scene_obj->AddToSceneManager();
		}
	}

	{
		XMLNodePtr skybox_node = root->FirstNode("skybox");

		XMLAttributePtr y_cube_attr = skybox_node->Attrib("y_cube");
		if (y_cube_attr)
		{
			XMLAttributePtr c_cube_attr = skybox_node->Attrib("c_cube");
			BOOST_ASSERT(c_cube_attr);

			sky_box_ = MakeSharedPtr<SceneObjectSkyBox>();
			checked_pointer_cast<SceneObjectSkyBox>(sky_box_)->CompressedCubeMap(
				SyncLoadTexture(y_cube_attr->ValueString(), EAH_GPU_Read | EAH_Immutable),
				SyncLoadTexture(c_cube_attr->ValueString(), EAH_GPU_Read | EAH_Immutable));
		}
		else
		{
			XMLAttributePtr cube_attr = skybox_node->Attrib("cube");
			if (cube_attr)
			{
				sky_box_ = MakeSharedPtr<SceneObjectSkyBox>();
				checked_pointer_cast<SceneObjectSkyBox>(sky_box_)->CubeMap(
					SyncLoadTexture(cube_attr->ValueString(), EAH_GPU_Read | EAH_Immutable));
			}
			else
			{
				Color color(0, 0, 0, 1);

				XMLAttributePtr r_attr = skybox_node->Attrib("r");
				if (r_attr)
				{
					color.r() = r_attr->ValueFloat();
				}
				XMLAttributePtr g_attr = skybox_node->Attrib("g");
				if (g_attr)
				{
					color.g() = g_attr->ValueFloat();
				}
				XMLAttributePtr b_attr = skybox_node->Attrib("b");
				if (b_attr)
				{
					color.b() = b_attr->ValueFloat();
				}

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

				sky_box_ = MakeSharedPtr<SceneObjectSkyBox>();
				checked_pointer_cast<SceneObjectSkyBox>(sky_box_)->CubeMap(rf.MakeTextureCube(1, 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_Immutable, init_data));
			}
		}

		sky_box_->AddToSceneManager();
	}

	{
		float3 eye_pos(0, 0, -1);
		float3 look_at(0, 0, 0);
		float3 up(0, 1, 0);
		float near_plane = 0.1f;
		float far_plane = 500;

		XMLNodePtr camera_node = root->FirstNode("camera");

		XMLAttributePtr x_attr = camera_node->Attrib("x");
		if (x_attr)
		{
			eye_pos.x() = x_attr->ValueFloat();
		}

		XMLAttributePtr y_attr = camera_node->Attrib("y");
		if (y_attr)
		{
			eye_pos.y() = y_attr->ValueFloat();
		}

		XMLAttributePtr z_attr = camera_node->Attrib("z");
		if (z_attr)
		{
			eye_pos.z() = z_attr->ValueFloat();
		}

		XMLNodePtr look_at_node = camera_node->FirstNode("look_at");
		if (look_at_node)
		{
			XMLAttributePtr x_attr = look_at_node->Attrib("x");
			if (x_attr)
			{
				look_at.x() = x_attr->ValueFloat();
			}

			XMLAttributePtr y_attr = look_at_node->Attrib("y");
			if (y_attr)
			{
				look_at.y() = y_attr->ValueFloat();
			}

			XMLAttributePtr z_attr = look_at_node->Attrib("z");
			if (z_attr)
			{
				look_at.z() = z_attr->ValueFloat();
			}
		}

		XMLNodePtr up_node = camera_node->FirstNode("up");
		if (up_node)
		{
			XMLAttributePtr x_attr = up_node->Attrib("x");
			if (x_attr)
			{
				up.x() = x_attr->ValueFloat();
			}

			XMLAttributePtr y_attr = up_node->Attrib("y");
			if (y_attr)
			{
				up.y() = y_attr->ValueFloat();
			}

			XMLAttributePtr z_attr = up_node->Attrib("z");
			if (z_attr)
			{
				up.z() = z_attr->ValueFloat();
			}
		}

		XMLNodePtr near_node = camera_node->FirstNode("near_plane");
		if (near_node)
		{
			XMLAttributePtr val_attr = near_node->Attrib("value");
			if (val_attr)
			{
				near_plane = val_attr->ValueFloat();
			}
		}

		XMLNodePtr far_node = camera_node->FirstNode("far_plane");
		if (far_node)
		{
			XMLAttributePtr val_attr = far_node->Attrib("value");
			if (val_attr)
			{
				far_plane = val_attr->ValueFloat();
			}
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

		if (!update_script.empty())
		{
			this->ActiveCamera().BindUpdateFunc(CameraUpdate(update_script));
		}

		this->LookAt(eye_pos, look_at, up);
		this->Proj(near_plane, far_plane);
	}
}

void ScenePlayerApp::InitObjects()
{
	this->LoadScene("DeferredRendering.scene");

	font_ = SyncLoadFont("gkai00mp.kfont");

	deferred_rendering_ = Context::Instance().DeferredRenderingLayerInstance();

	fpcController_.Scalers(0.05f, 0.5f);

	InputEngine& inputEngine(Context::Instance().InputFactoryInstance().InputEngineInstance());
	InputActionMap actionMap;
	actionMap.AddActions(actions, actions + sizeof(actions) / sizeof(actions[0]));

	action_handler_t input_handler = MakeSharedPtr<input_signal>();
	input_handler->connect(KlayGE::bind(&ScenePlayerApp::InputHandler, this, KlayGE::placeholders::_1, KlayGE::placeholders::_2));
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

	dialog_->Control<UIButton>(id_open_)->OnClickedEvent().connect(KlayGE::bind(&ScenePlayerApp::OpenHandler, this, KlayGE::placeholders::_1));

	dialog_->Control<UIComboBox>(id_illum_combo_)->OnSelectionChangedEvent().connect(KlayGE::bind(&ScenePlayerApp::IllumChangedHandler, this, KlayGE::placeholders::_1));
	this->IllumChangedHandler(*dialog_->Control<UIComboBox>(id_illum_combo_));

	dialog_->Control<UISlider>(id_il_scale_slider_)->SetValue(static_cast<int>(il_scale_ * 10));
	dialog_->Control<UISlider>(id_il_scale_slider_)->OnValueChangedEvent().connect(KlayGE::bind(&ScenePlayerApp::ILScaleChangedHandler, this, KlayGE::placeholders::_1));
	this->ILScaleChangedHandler(*dialog_->Control<UISlider>(id_il_scale_slider_));

	dialog_->Control<UICheckBox>(id_ssgi_)->OnChangedEvent().connect(KlayGE::bind(&ScenePlayerApp::SSGIHandler, this, KlayGE::placeholders::_1));
	this->SSGIHandler(*dialog_->Control<UICheckBox>(id_ssgi_));

	dialog_->Control<UICheckBox>(id_ssvo_)->OnChangedEvent().connect(KlayGE::bind(&ScenePlayerApp::SSVOHandler, this, KlayGE::placeholders::_1));
	this->SSVOHandler(*dialog_->Control<UICheckBox>(id_ssvo_));

	dialog_->Control<UICheckBox>(id_hdr_)->OnChangedEvent().connect(KlayGE::bind(&ScenePlayerApp::HDRHandler, this, KlayGE::placeholders::_1));
	this->HDRHandler(*dialog_->Control<UICheckBox>(id_hdr_));

	dialog_->Control<UICheckBox>(id_aa_)->OnChangedEvent().connect(KlayGE::bind(&ScenePlayerApp::AAHandler, this, KlayGE::placeholders::_1));
	this->AAHandler(*dialog_->Control<UICheckBox>(id_aa_));

	dialog_->Control<UICheckBox>(id_cg_)->OnChangedEvent().connect(KlayGE::bind(&ScenePlayerApp::ColorGradingHandler, this, KlayGE::placeholders::_1));
	this->ColorGradingHandler(*dialog_->Control<UICheckBox>(id_cg_));

	dialog_->Control<UICheckBox>(id_ctrl_camera_)->OnChangedEvent().connect(KlayGE::bind(&ScenePlayerApp::CtrlCameraHandler, this, KlayGE::placeholders::_1));
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
#if defined KLAYGE_PLATFORM_WINDOWS
	OPENFILENAMEA ofn;
	char fn[260];
	HWND hwnd = this->MainWnd()->HWnd();

	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFile = fn;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(fn);
	ofn.lpstrFilter = "Scene File\0*.scene\0All\0*.*\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = nullptr;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = nullptr;
	ofn.Flags = OFN_NOCHANGEDIR | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	if (GetOpenFileNameA(&ofn))
	{
		this->LoadScene(fn);
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
