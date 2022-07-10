#include <KlayGE/KlayGE.hpp>
#include <KFL/CustomizedStreamBuf.hpp>
#include <KFL/Util.hpp>
#include <KFL/Math.hpp>
#include <KFL/StringUtil.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/Renderable.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/FrameBuffer.hpp>
#include <KlayGE/SceneManager.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/RenderSettings.hpp>
#include <KlayGE/Mesh.hpp>
#include <KlayGE/GraphicsBuffer.hpp>
#include <KlayGE/SceneNode.hpp>
#include <KlayGE/SkyBox.hpp>
#include <KlayGE/UI.hpp>
#include <KlayGE/Camera.hpp>
#include <KlayGE/DeferredRenderingLayer.hpp>
#include <KFL/XMLDom.hpp>
#include <KlayGE/Window.hpp>

#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/InputFactory.hpp>
#include <KlayGE/ScriptFactory.hpp>

#if defined KLAYGE_PLATFORM_WINDOWS_DESKTOP
#include <commdlg.h>
#endif

#include <fstream>
#include <iterator>
#include <sstream>
#include <vector>

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
			*script_ = std::string(StringUtil::Trim(script));
		}

		virtual ~PyScriptUpdate() = default;

		ScriptVariablePtr Run(float app_time, float elapsed_time)
		{
			module_->RunString(*script_);
			return module_->Call(
				"update", MakeSpan<ScriptVariablePtr>({module_->MakeVariable(app_time), module_->MakeVariable(elapsed_time)}));
		}

	private:
		std::shared_ptr<ScriptModule> module_;
		std::shared_ptr<std::string> script_;
	};

	class LightSourceNodeUpdate : public PyScriptUpdate
	{
	public:
		explicit LightSourceNodeUpdate(std::string const & script)
			: PyScriptUpdate(script)
		{
		}

		void operator()(SceneComponent& component, float app_time, float elapsed_time)
		{
			LightSource& light = checked_cast<LightSource&>(component);

			ScriptVariablePtr py_ret = this->Run(app_time, elapsed_time);
			std::vector<ScriptVariablePtr> ret;
			if (py_ret->TryValue(ret))
			{
				size_t s = ret.size();

				if (s > 0)
				{
					ScriptVariablePtr const& py_mat = ret[0];
					std::vector<ScriptVariablePtr> mat;
					if (py_mat->TryValue(mat))
					{
						if (!mat.empty())
						{
							float4x4 light_mat;
							for (int i = 0; i < 16; ++ i)
							{
								mat[i]->Value(light_mat[i]);
							}
							light.BoundSceneNode()->TransformToParent(light_mat);
						}
					}
				}
				if (s > 1)
				{
					ScriptVariablePtr const& py_clr = ret[1];
					std::vector<ScriptVariablePtr> clr;
					if (py_clr->TryValue(clr))
					{
						if (!clr.empty())
						{
							float3 light_clr;
							for (int i = 0; i < 3; ++ i)
							{
								clr[i]->Value(light_clr[i]);
							}
							light.Color(light_clr);
						}
					}
				}				
				if (s > 2)
				{
					ScriptVariablePtr const& py_fo = ret[2];
					std::vector<ScriptVariablePtr> fo;
					if (py_fo->TryValue(fo))
					{
						if (!fo.empty())
						{
							float3 light_fall_off;
							for (int i = 0; i < 3; ++ i)
							{
								fo[i]->Value(light_fall_off[i]);
							}
							light.Falloff(light_fall_off);
						}
					}
				}
				if (s > 3)
				{
					ScriptVariablePtr const& py_oi = ret[3];
					std::vector<ScriptVariablePtr> oi;
					if (py_oi->TryValue(oi))
					{
						if (!oi.empty())
						{
							float2 light_outer_inner;
							for (int i = 0; i < 2; ++ i)
							{
								oi[i]->Value(light_outer_inner[i]);
							}
							light.OuterAngle(light_outer_inner.x());
							light.InnerAngle(light_outer_inner.y());
						}
					}
				}
			}
		}
	};

	class SceneNodeUpdate : public PyScriptUpdate
	{
	public:
		explicit SceneNodeUpdate(std::string const & script)
			: PyScriptUpdate(script)
		{
		}

		void operator()(SceneNode& node, float app_time, float elapsed_time)
		{
			ScriptVariablePtr py_ret = this->Run(app_time, elapsed_time);
			std::vector<ScriptVariablePtr> ret;
			if (py_ret->TryValue(ret))
			{
				size_t s = ret.size();

				if (s > 0)
				{
					ScriptVariablePtr const& py_mat = ret[0];
					std::vector<ScriptVariablePtr> mat;
					if (py_mat->TryValue(mat))
					{
						if (!mat.empty())
						{
							float4x4 obj_mat;
							for (int i = 0; i < 16; ++ i)
							{
								mat[i]->Value(obj_mat[i]);
							}
							node.TransformToParent(obj_mat);
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

		void operator()(SceneComponent& component, float app_time, float elapsed_time)
		{
			Camera& camera = checked_cast<Camera&>(component);

			ScriptVariablePtr py_ret = this->Run(app_time, elapsed_time);
			std::vector<ScriptVariablePtr> ret;
			if (py_ret->TryValue(ret))
			{
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
					ScriptVariablePtr const& py_eye = ret[0];
					std::vector<ScriptVariablePtr> eye;
					if (py_eye->TryValue(eye))
					{
						if (!eye.empty())
						{
							for (int i = 0; i < 3; ++ i)
							{
								eye[i]->Value(cam_eye[i]);
							}
						}
					}
				}
				if (s > 1)
				{
					ScriptVariablePtr const& py_lookat = ret[1];
					std::vector<ScriptVariablePtr> lookat;
					if (py_lookat->TryValue(lookat))
					{
						if (!lookat.empty())
						{
							for (int i = 0; i < 3; ++ i)
							{
								lookat[i]->Value(cam_lookat[i]);
							}
						}
					}
				}
				if (s > 2)
				{
					ScriptVariablePtr const& py_up = ret[2];
					std::vector<ScriptVariablePtr> up;
					if (py_up->TryValue(up))
					{
						if (!up.empty())
						{
							for (int i = 0; i < 3; ++ i)
							{
								up[i]->Value(cam_up[i]);
							}
						}
					}
				}
				if (s > 3)
				{
					ScriptVariablePtr const& py_np = ret[3];
					float np;
					if (py_np->TryValue(np))
					{
						cam_np = np;
					}
				}
				if (s > 4)
				{
					ScriptVariablePtr const& py_fp = ret[4];
					float fp;
					if (py_fp->TryValue(fp))
					{
						cam_fp = fp;
					}
				}

				camera.LookAtDist(MathLib::length(cam_lookat - cam_eye));
				camera.BoundSceneNode()->TransformToWorld(MathLib::inverse(MathLib::look_at_lh(cam_eye, cam_lookat, cam_up)));
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
	sceneMgr.ClearObject();

	RenderFactory& rf = context.RenderFactoryInstance();

	scene_models_.clear();
	scene_objs_.clear();
	skybox_.reset();

	lights_.clear();

	ResIdentifierPtr ifs = ResLoader::Instance().Open(name.c_str());

	std::unique_ptr<KlayGE::XMLDocument> doc = LoadXml(*ifs);
	XMLNode const* root = doc->RootNode();

	if (XMLAttribute const* attr = root->Attrib("skybox"))
	{
		auto skybox_renderable = MakeSharedPtr<RenderableSkyBox>();
		skybox_ = MakeSharedPtr<SceneNode>(MakeSharedPtr<RenderableComponent>(skybox_renderable), SceneNode::SOA_NotCastShadow);

		std::string const skybox_name = std::string(attr->ValueString());
		if (!ResLoader::Instance().Locate(skybox_name).empty())
		{
			skybox_renderable->CubeMap(ASyncLoadTexture(skybox_name, EAH_GPU_Read | EAH_Immutable));
		}
		else if (!ResLoader::Instance().Locate(skybox_name + ".dds").empty())
		{
			skybox_renderable->CubeMap(ASyncLoadTexture(skybox_name + ".dds", EAH_GPU_Read | EAH_Immutable));
		}
		else if (!ResLoader::Instance().Locate(skybox_name + "_y.dds").empty())
		{
			skybox_renderable->CompressedCubeMap(ASyncLoadTexture(skybox_name + "_y.dds", EAH_GPU_Read | EAH_Immutable),
				ASyncLoadTexture(skybox_name + "_c.dds", EAH_GPU_Read | EAH_Immutable));
		}
		else
		{
			Color color(0, 0, 0, 1);
			MemInputStreamBuf stream_buff(skybox_name.data(), skybox_name.size());
			std::istream(&stream_buff) >> color.r() >> color.g() >> color.b();

			auto const fmt = rf.RenderEngineInstance().DeviceCaps().BestMatchTextureFormat(MakeSpan({EF_ABGR8, EF_ARGB8}));
			BOOST_ASSERT(fmt != EF_Unknown);
			uint32_t texel = (fmt == EF_ABGR8) ? color.ABGR() : color.ARGB();
			ElementInitData init_data[6];
			for (int i = 0; i < 6; ++ i)
			{
				init_data[i].data = &texel;
				init_data[i].row_pitch = sizeof(uint32_t);
				init_data[i].slice_pitch = init_data[i].row_pitch;
			}

			skybox_renderable->CubeMap(rf.MakeTextureCube(1, 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_Immutable, MakeSpan(init_data)));
		}

		Context::Instance().SceneManagerInstance().SceneRootNode().AddChild(skybox_);
	}

	for (XMLNode const* light_node = root->FirstNode("light"); light_node; light_node = light_node->NextSibling("light"))
	{
		LightSourcePtr light;
		SceneNodePtr scene_node = MakeSharedPtr<SceneNode>(SceneNode::SOA_Cullable | SceneNode::SOA_Moveable);

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
		if (XMLNode const* attr_node = light_node->FirstNode("attr"))
		{
			std::string_view const attr_str = attr_node->Attrib("value")->ValueString();
			std::vector<std::string_view> tokens = StringUtil::Split(attr_str, StringUtil::IsAnyOf(" \t|"));
			for (auto& token : tokens)
			{
				token = StringUtil::Trim(token);

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

		if (XMLNode const* color_node = light_node->FirstNode("color"))
		{
			float3 color;
			auto v = color_node->Attrib("v")->ValueString();
			MemInputStreamBuf stream_buff(v.data(), v.size());
			std::istream(&stream_buff) >> color.x() >> color.y() >> color.z();
			light->Color(color);
		}

		float3 light_pos(0, 0, 0);
		float3 light_dir(0, 0, 1);
		if (light->Type() != LightSource::LT_Ambient)
		{
			if (XMLNode const* dir_node = light_node->FirstNode("dir"))
			{
				auto v = dir_node->Attrib("v")->ValueString();
				MemInputStreamBuf stream_buff(v.data(), v.size());
				std::istream(&stream_buff) >> light_dir.x() >> light_dir.y() >> light_dir.z();
			}
		}
		if ((LightSource::LT_Point == light->Type()) || (LightSource::LT_Spot == light->Type())
			|| (LightSource::LT_SphereArea == light->Type()) || (LightSource::LT_TubeArea == light->Type()))
		{
			if (XMLNode const* pos_node = light_node->FirstNode("pos"))
			{
				auto v = pos_node->Attrib("v")->ValueString();
				MemInputStreamBuf stream_buff(v.data(), v.size());
				std::istream(&stream_buff) >> light_pos.x() >> light_pos.y() >> light_pos.z();
			}

			if (XMLNode const* fall_off_node = light_node->FirstNode("fall_off"))
			{
				float3 fall_off;
				auto v = fall_off_node->Attrib("v")->ValueString();
				MemInputStreamBuf stream_buff(v.data(), v.size());
				std::istream(&stream_buff) >> fall_off.x() >> fall_off.y() >> fall_off.z();
				light->Falloff(fall_off);
			}

			if ((LightSource::LT_Point == light->Type()) || (LightSource::LT_Spot == light->Type()))
			{
				if (XMLNode const* projective_node = light_node->FirstNode("projective"))
				{
					if (XMLAttribute const* attr = projective_node->Attrib("name"))
					{
						TexturePtr projective = ASyncLoadTexture(std::string(attr->ValueString()), EAH_GPU_Read | EAH_Immutable);
						light->ProjectiveTexture(projective);
					}
				}

				if (LightSource::LT_Spot == light->Type())
				{
					if (XMLNode const* angle_node = light_node->FirstNode("angle"))
					{
						light->InnerAngle(angle_node->Attrib("inner")->ValueFloat());
						light->OuterAngle(angle_node->Attrib("outer")->ValueFloat());
					}
				}
			}

			// TODO: sphere area light and tube area light
		}

		scene_node->TransformToParent(
			MathLib::to_matrix(MathLib::axis_to_axis(float3(0, 0, 1), light_dir)) * MathLib::translation(light_pos));
		scene_node->AddComponent(light);

		if (XMLNode const* update_node = light_node->FirstNode("update"))
		{
			update_node = update_node->FirstNode();
			if (update_node && (XMLNodeType::CData == update_node->Type()))
			{
				std::string const update_script = std::string(update_node->ValueString());
				if (!update_script.empty())
				{
					light->OnMainThreadUpdate().Connect(LightSourceNodeUpdate(update_script));
				}
			}
		}

		Context::Instance().SceneManagerInstance().SceneRootNode().AddChild(scene_node);
		lights_.push_back(light);

		if (XMLNode const* scale_node = light_node->FirstNode("scale"))
		{
			float3 scale(1, 1, 1);
			{
				auto v = scale_node->Attrib("v")->ValueString();
				MemInputStreamBuf stream_buff(v.data(), v.size());
				std::istream(&stream_buff) >> scale.x() >> scale.y() >> scale.z();
			}

			auto light_proxy = LoadLightSourceProxyModel(light);
			light_proxy->RootNode()->TransformToParent(MathLib::scaling(scale) * light_proxy->RootNode()->TransformToParent());
			scene_node->AddChild(light_proxy->RootNode());
		}
	}

	for (XMLNode const* model_node = root->FirstNode("model"); model_node; model_node = model_node->NextSibling("model"))
	{
		uint32_t obj_attr = SceneNode::SOA_Cullable;
		float4x4 obj_mat = float4x4::Identity();

		float3 scale(1, 1, 1);
		Quaternion rotate = Quaternion::Identity();
		float3 translate(0, 0, 0);

		if (XMLNode const* scale_node = model_node->FirstNode("scale"))
		{
			auto v = scale_node->Attrib("v")->ValueString();
			MemInputStreamBuf stream_buff(v.data(), v.size());
			std::istream(&stream_buff) >> scale.x() >> scale.y() >> scale.z();
		}
		
		if (XMLNode const* rotate_node = model_node->FirstNode("rotate"))
		{
			auto v = rotate_node->Attrib("v")->ValueString();
			MemInputStreamBuf stream_buff(v.data(), v.size());
			std::istream(&stream_buff) >> rotate.x() >> rotate.y() >> rotate.z() >> rotate.w();
		}

		if (XMLNode const* translate_node = model_node->FirstNode("translate"))
		{
			auto v = translate_node->Attrib("v")->ValueString();
			MemInputStreamBuf stream_buff(v.data(), v.size());
			std::istream(&stream_buff) >> translate.x() >> translate.y() >> translate.z();
		}

		obj_mat = MathLib::transformation<float>(nullptr, nullptr, &scale, nullptr, &rotate, &translate);

		if (XMLNode const* attribute_node = model_node->FirstNode("attr"))
		{
			if (XMLAttribute const* attr = attribute_node->Attrib("value"))
			{
				if (!attr->TryConvertValue(obj_attr))
				{
					obj_attr = SceneNode::SOA_Cullable;

					std::string_view const attr_str = attr->ValueString();
					std::vector<std::string_view> tokens = StringUtil::Split(attr_str, StringUtil::IsAnyOf(" \t|"));
					for (auto& token : tokens)
					{
						token = StringUtil::Trim(token);
						if ("cullable" == token)
						{
							obj_attr |= SceneNode::SOA_Cullable;
						}
						else if ("overlay" == token)
						{
							obj_attr |= SceneNode::SOA_Overlay;
						}
						else if ("moveable" == token)
						{
							obj_attr |= SceneNode::SOA_Moveable;
						}
						else if ("invisible" == token)
						{
							obj_attr |= SceneNode::SOA_Invisible;
						}
					}
				}
			}
		}

		std::string update_script;
		if (XMLNode const* update_node = model_node->FirstNode("update"))
		{
			update_node = update_node->FirstNode();
			if (update_node && (XMLNodeType::CData == update_node->Type()))
			{
				update_script = std::string(update_node->ValueString());
			}
		}

		XMLAttribute const* attr = model_node->Attrib("model");
		BOOST_ASSERT(attr);

		auto& scene_obj = scene_objs_.emplace_back(MakeSharedPtr<SceneNode>(SceneNode::SOA_Cullable));
		scene_obj->TransformToParent(obj_mat);
		Context::Instance().SceneManagerInstance().SceneRootNode().AddChild(scene_obj);
		scene_models_.emplace_back(ASyncLoadModel(attr->ValueString(), EAH_GPU_Read | EAH_Immutable,
			obj_attr,
			[scene_obj](RenderModel& model)
			{
				AddToSceneHelper(*scene_obj, model);
			}));
		if (!update_script.empty())
		{
			scene_obj->OnSubThreadUpdate().Connect(SceneNodeUpdate(update_script));
		}
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

		XMLNode const* camera_node = root->FirstNode("camera");

		if (XMLNode const* eye_pos_node = camera_node->FirstNode("eye_pos"))
		{
			auto v = eye_pos_node->Attrib("v")->ValueString();
			MemInputStreamBuf stream_buff(v.data(), v.size());
			std::istream(&stream_buff) >> eye_pos.x() >> eye_pos.y() >> eye_pos.z();
		}
		if (XMLNode const* look_at_node = camera_node->FirstNode("look_at"))
		{
			auto v = look_at_node->Attrib("v")->ValueString();
			MemInputStreamBuf stream_buff(v.data(), v.size());
			std::istream(&stream_buff) >> look_at.x() >> look_at.y() >> look_at.z();
		}
		if (XMLNode const* up_node = camera_node->FirstNode("up"))
		{
			auto v = up_node->Attrib("v")->ValueString();
			MemInputStreamBuf stream_buff(v.data(), v.size());
			std::istream(&stream_buff) >> up.x() >> up.y() >> up.z();
		}

		if (XMLNode const* fov_node = camera_node->FirstNode("fov"))
		{
			fov = fov_node->Attrib("s")->ValueFloat();
		}
		if (XMLNode const* aspect_node = camera_node->FirstNode("aspect"))
		{
			aspect = aspect_node->Attrib("s")->ValueFloat();
		}
		if (XMLNode const* near_plane_node = camera_node->FirstNode("near"))
		{
			near_plane = near_plane_node->Attrib("s")->ValueFloat();
		}
		if (XMLNode const* far_plane_node = camera_node->FirstNode("far"))
		{
			far_plane = far_plane_node->Attrib("s")->ValueFloat();
		}

		std::string update_script;

		if (XMLNode const* update_node = camera_node->FirstNode("update"))
		{
			update_node = update_node->FirstNode();
			if (update_node && (XMLNodeType::CData == update_node->Type()))
			{
				update_script = std::string(update_node->ValueString());
			}
		}

		auto& camera = this->ActiveCamera();
		camera.LookAtDist(MathLib::length(look_at - eye_pos));
		camera.BoundSceneNode()->TransformToWorld(MathLib::inverse(MathLib::look_at_lh(eye_pos, look_at, up)));
		camera.ProjParams(fov, aspect, near_plane, far_plane);
		if (!update_script.empty())
		{
			camera.OnMainThreadUpdate().Connect(CameraUpdate(update_script));
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
	input_handler->Connect(
		[this](InputEngine const & sender, InputAction const & action)
		{
			this->InputHandler(sender, action);
		});
	inputEngine.ActionMap(actionMap, input_handler);

	UIManager::Instance().Load(*ResLoader::Instance().Open("ScenePlayer.uiml"));
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

	dialog_->Control<UIButton>(id_open_)->OnClickedEvent().Connect(
		[this](UIButton const & sender)
		{
			this->OpenHandler(sender);
		});

	dialog_->Control<UIComboBox>(id_illum_combo_)->OnSelectionChangedEvent().Connect(
		[this](UIComboBox const & sender)
		{
			this->IllumChangedHandler(sender);
		});
	this->IllumChangedHandler(*dialog_->Control<UIComboBox>(id_illum_combo_));

	dialog_->Control<UISlider>(id_il_scale_slider_)->SetValue(static_cast<int>(il_scale_ * 10));
	dialog_->Control<UISlider>(id_il_scale_slider_)->OnValueChangedEvent().Connect(
		[this](UISlider const & sender)
		{
			this->ILScaleChangedHandler(sender);
		});
	this->ILScaleChangedHandler(*dialog_->Control<UISlider>(id_il_scale_slider_));

	dialog_->Control<UICheckBox>(id_ssgi_)->OnChangedEvent().Connect(
		[this](UICheckBox const & sender)
		{
			this->SSGIHandler(sender);
		});
	this->SSGIHandler(*dialog_->Control<UICheckBox>(id_ssgi_));

	dialog_->Control<UICheckBox>(id_ssvo_)->OnChangedEvent().Connect(
		[this](UICheckBox const & sender)
		{
			this->SSVOHandler(sender);
		});
	this->SSVOHandler(*dialog_->Control<UICheckBox>(id_ssvo_));

	dialog_->Control<UICheckBox>(id_hdr_)->OnChangedEvent().Connect(
		[this](UICheckBox const & sender)
		{
			this->HDRHandler(sender);
		});
	this->HDRHandler(*dialog_->Control<UICheckBox>(id_hdr_));

	dialog_->Control<UICheckBox>(id_aa_)->OnChangedEvent().Connect(
		[this](UICheckBox const & sender)
		{
			this->AAHandler(sender);
		});
	this->AAHandler(*dialog_->Control<UICheckBox>(id_aa_));

	dialog_->Control<UICheckBox>(id_cg_)->OnChangedEvent().Connect(
		[this](UICheckBox const & sender)
		{
			this->ColorGradingHandler(sender);
		});
	this->ColorGradingHandler(*dialog_->Control<UICheckBox>(id_cg_));

	dialog_->Control<UICheckBox>(id_ctrl_camera_)->OnChangedEvent().Connect(
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
	ofn.Flags = OFN_NOCHANGEDIR | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

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

	uint32_t const num_loading_res = ResLoader::Instance().NumLoadingResources();
	if (num_loading_res > 0)
	{
		stream.str(L"");
		stream << "Loading " << num_loading_res << " resources...";
		font_->RenderText(100, 300, Color(1, 0, 0, 1), stream.str(), 48);
	}
}

uint32_t ScenePlayerApp::DoUpdate(uint32_t pass)
{
	return deferred_rendering_->Update(pass);
}
