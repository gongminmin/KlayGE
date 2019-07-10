#include <KlayGE/KlayGE.hpp>
#include <KFL/CXX17/iterator.hpp>
#include <KFL/Util.hpp>
#include <KFL/Math.hpp>
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
#include <KlayGE/Light.hpp>
#include <KlayGE/SceneNode.hpp>
#include <KlayGE/JudaTexture.hpp>
#include <KlayGE/Camera.hpp>

#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/InputFactory.hpp>

#include <vector>
#include <sstream>

#include "SampleCommon.hpp"
#include "DetailedSurface.hpp"

using namespace std;
using namespace KlayGE;

namespace
{
	enum
	{
		DT_None,
		DT_Bump,
		DT_Parallax,
		DT_ParallaxOcclusion,
		DT_FlatTessellation,
		DT_SmoothTessellation,
	};

	class RenderPolygon : public StaticMesh
	{
	public:
		explicit RenderPolygon(std::wstring_view name)
			: StaticMesh(name),
				detail_type_(DT_Parallax), wireframe_(false)
		{
			effect_ = SyncLoadRenderEffect("DetailedSurface.fxml");
			technique_ = effect_->TechniqueByName("Parallax");

			tile_bb_[0] = int4(0, 0, 4, 4);
			tile_bb_[1] = int4(4, 0, 4, 4);
			tile_bb_[2] = int4(0, 4, 4, 4);
			tile_bb_[3] = int4(4, 4, 4, 4);

			*(effect_->ParameterByName("diffuse_tex_bb")) = tile_bb_[0];
			*(effect_->ParameterByName("normal_tex_bb")) = tile_bb_[1];
			*(effect_->ParameterByName("height_tex_bb")) = tile_bb_[2];
			*(effect_->ParameterByName("occlusion_tex_bb")) = tile_bb_[3];
			*(effect_->ParameterByName("tex_size")) = int2(512, 512);
			*(effect_->ParameterByName("na_length_tex")) = ASyncLoadTexture("na_length.dds", EAH_GPU_Read | EAH_Immutable);
		}

		void OnRenderBegin()
		{
			StaticMesh::OnRenderBegin();

			App3DFramework const & app = Context::Instance().AppInstance();
			Camera const & camera = app.ActiveCamera();

			float4x4 const & model = float4x4::Identity();

			*(effect_->ParameterByName("mvp")) = model * camera.ViewProjMatrix();
			*(effect_->ParameterByName("model_view")) = model * camera.ViewMatrix();
			*(effect_->ParameterByName("world")) = model;
			*(effect_->ParameterByName("eye_pos")) = camera.EyePos();
			*(effect_->ParameterByName("forward_vec")) = camera.ForwardVec();

			RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
			FrameBufferPtr const & fb = re.CurFrameBuffer();
			*(effect_->ParameterByName("frame_size")) = int2(fb->Width(), fb->Height());
		}

		void LightPos(float3 const & light_pos)
		{
			*(effect_->ParameterByName("light_pos")) = light_pos;
		}

		void LightColor(float3 const & light_color)
		{
			*(effect_->ParameterByName("light_color")) = light_color;
		}

		void LightFalloff(float3 const & light_falloff)
		{
			*(effect_->ParameterByName("light_falloff")) = light_falloff;
		}

		void HeightScale(float scale)
		{
			*(effect_->ParameterByName("height_scale")) = scale;
		}

		void BindJudaTexture(JudaTexturePtr const & juda_tex)
		{
			juda_tex->SetParams(*effect_);

			tile_ids_.clear();
			uint32_t level = juda_tex->TreeLevels() - 1;
			for (size_t i = 0; i < std::size(tile_bb_); ++ i)
			{
				for (int32_t y = 0; y < tile_bb_[i].w(); ++ y)
				{
					for (int32_t x = 0; x < tile_bb_[i].z(); ++ x)
					{
						tile_ids_.push_back(juda_tex->EncodeTileID(level, tile_bb_[i].x() + x, tile_bb_[i].y() + y));
					}
				}
			}
		}

		std::vector<uint32_t> const & JudaTexTileIDs() const
		{
			return tile_ids_;
		}

		void DetailType(uint32_t dt)
		{
			detail_type_ = dt;
			this->UpdateTech();
		}

		void NaLength(bool len)
		{
			*(effect_->ParameterByName("use_na_length")) = len;
		}

		void UseOcclusionMap(bool om)
		{
			*(effect_->ParameterByName("use_occlusion_map")) = om;
		}

		void Wireframe(bool wf)
		{
			wireframe_ = wf;
			this->UpdateTech();
		}

	private:
		void UpdateTech()
		{
			std::string tech_name;
			switch (detail_type_)
			{
			case DT_None:
				tech_name = "None";
				break;

			case DT_Bump:
				tech_name = "Bump";
				break;

			case DT_Parallax:
				tech_name = "Parallax";
				break;

			case DT_ParallaxOcclusion:
				tech_name = "ParallaxOcclusion";
				break;

			case DT_FlatTessellation:
				tech_name = "FlatTessellation";
				break;

			case DT_SmoothTessellation:
				tech_name = "SmoothTessellation";
				break;

			default:
				tech_name = "None";
				break;
			}

			if (wireframe_)
			{
				tech_name += "Wireframe";
			}

			technique_ = effect_->TechniqueByName(tech_name);

			if ((DT_FlatTessellation == detail_type_) || (DT_SmoothTessellation == detail_type_))
			{
				for (auto const & rl : rls_)
				{
					rl->TopologyType(RenderLayout::TT_3_Ctrl_Pt_PatchList);
				}
			}
			else
			{
				for (auto const & rl : rls_)
				{
					rl->TopologyType(RenderLayout::TT_TriangleList);
				}
			}
		}

	private:
		int4 tile_bb_[4];
		std::vector<uint32_t> tile_ids_;
		uint32_t detail_type_;
		bool wireframe_;
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
	DetailedSurfaceApp app;
	app.Create();
	app.Run();

	return 0;
}

DetailedSurfaceApp::DetailedSurfaceApp()
			: App3DFramework("DetailedSurface"),
				height_scale_(0.06f)
{
	ResLoader::Instance().AddPath("../../Samples/media/DetailedSurface");
}

void DetailedSurfaceApp::OnCreate()
{
	font_ = SyncLoadFont("gkai00mp.kfont");
	UIManager::Instance().Load(ResLoader::Instance().Open("DetailedSurface.uiml"));

	this->LookAt(float3(-0.18f, 0.24f, -0.18f), float3(0, 0.05f, 0));
	this->Proj(0.01f, 100);

	RenderFactory& rf = Context::Instance().RenderFactoryInstance();
	juda_tex_ = LoadJudaTexture("DetailedSurface.jdt");

	auto const fmt = rf.RenderEngineInstance().DeviceCaps().BestMatchTextureFormat(MakeSpan({EF_BC1, EF_ABGR8, EF_ARGB8}));
	BOOST_ASSERT(fmt != EF_Unknown);
	juda_tex_->CacheProperty(1024, fmt, 4);

	loading_percentage_ = 0;
}

void DetailedSurfaceApp::OnResize(uint32_t width, uint32_t height)
{
	App3DFramework::OnResize(width, height);

	UIManager::Instance().SettleCtrls();
}

void DetailedSurfaceApp::InputHandler(InputEngine const & /*sender*/, InputAction const & action)
{
	switch (action.first)
	{
	case Exit:
		this->Quit();
		break;
	}
}

void DetailedSurfaceApp::ScaleChangedHandler(KlayGE::UISlider const & sender)
{
	height_scale_ = sender.GetValue() / 100.0f;
	polygon_model_->ForEachMesh([this](Renderable& mesh)
		{
			checked_cast<RenderPolygon&>(mesh).HeightScale(height_scale_);
		});

	std::wostringstream stream;
	stream << L"Scale: " << height_scale_;
	dialog_->Control<UIStatic>(id_scale_static_)->SetText(stream.str());
}

void DetailedSurfaceApp::DetailTypeChangedHandler(KlayGE::UIComboBox const & sender)
{
	int const index = sender.GetSelectedIndex();
	polygon_model_->ForEachMesh([index](Renderable& mesh)
		{
			checked_cast<RenderPolygon&>(mesh).DetailType(index);
		});
}

void DetailedSurfaceApp::OcclusionHandler(KlayGE::UICheckBox const& sender)
{
	bool const om = sender.GetChecked();
	polygon_model_->ForEachMesh([om](Renderable& mesh) { checked_cast<RenderPolygon&>(mesh).UseOcclusionMap(om); });
}

void DetailedSurfaceApp::NaLengthHandler(KlayGE::UICheckBox const & sender)
{
	bool const na = sender.GetChecked();
	polygon_model_->ForEachMesh([na](Renderable& mesh)
		{
			checked_cast<RenderPolygon&>(mesh).NaLength(na);
		});
}

void DetailedSurfaceApp::WireframeHandler(KlayGE::UICheckBox const & sender)
{
	bool const wf = sender.GetChecked();
	polygon_model_->ForEachMesh([wf](Renderable& mesh)
		{
			checked_cast<RenderPolygon&>(mesh).Wireframe(wf);
		});
}

void DetailedSurfaceApp::DoUpdateOverlay()
{
	UIManager::Instance().Render();

	std::wostringstream stream;
	stream.precision(2);
	stream << std::fixed << this->FPS() << " FPS";

	font_->RenderText(0, 0, Color(1, 1, 0, 1), L"Detailed Surface", 16);
	font_->RenderText(0, 18, Color(1, 1, 0, 1), stream.str(), 16);

	SceneManager& sceneMgr(Context::Instance().SceneManagerInstance());
	stream.str(L"");
	stream << sceneMgr.NumRenderablesRendered() << " Renderables "
		<< sceneMgr.NumPrimitivesRendered() << " Primitives "
		<< sceneMgr.NumVerticesRendered() << " Vertices";
	font_->RenderText(0, 36, Color(1, 1, 1, 1), stream.str(), 16);
}

uint32_t DetailedSurfaceApp::DoUpdate(uint32_t /*pass*/)
{
	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
	if (loading_percentage_ < 100)
	{
		UIDialogPtr const & dialog_loading = UIManager::Instance().GetDialog("Loading");
		UIStaticPtr const & msg = dialog_loading->Control<UIStatic>(dialog_loading->IDFromName("Msg"));
		UIProgressBarPtr const & progress_bar = dialog_loading->Control<UIProgressBar>(dialog_loading->IDFromName("Progress"));

		if (loading_percentage_ < 20)
		{
			dialog_ = UIManager::Instance().GetDialog("DetailedSurface");
			dialog_->SetVisible(false);

			dialog_loading->SetVisible(true);

			loading_percentage_ = 20;
			progress_bar->SetValue(loading_percentage_);
			msg->SetText(L"Loading Geometry");
		}
		else if (loading_percentage_ < 60)
		{
			polygon_model_ = SyncLoadModel("teapot.glb", EAH_GPU_Read | EAH_Immutable,
				SceneNode::SOA_Cullable, AddToSceneRootHelper,
				CreateModelFactory<RenderModel>, CreateMeshFactory<RenderPolygon>);
			polygon_model_->ForEachMesh([this](Renderable& mesh)
				{
					checked_cast<RenderPolygon&>(mesh).BindJudaTexture(juda_tex_);
				});
			juda_tex_->UpdateCache(checked_pointer_cast<RenderPolygon>(polygon_model_->Mesh(0))->JudaTexTileIDs());

			tb_controller_.AttachCamera(this->ActiveCamera());
			tb_controller_.Scalers(0.01f, 0.001f);

			loading_percentage_ = 60;
			progress_bar->SetValue(loading_percentage_);
			msg->SetText(L"Loading Light");
		}
		else if (loading_percentage_ < 80)
		{
			light_ = MakeSharedPtr<PointLightSource>();
			light_->Attrib(0);
			light_->Color(float3(2, 2, 2));
			light_->Falloff(float3(1, 0, 1.0f));

			auto light_node = MakeSharedPtr<SceneNode>(SceneNode::SOA_Cullable);
			light_node->TransformToParent(MathLib::translation(0.25f, 0.5f, -1.0f));
			light_node->AddComponent(light_);

			auto light_proxy = LoadLightSourceProxyModel(light_);
			light_proxy->RootNode()->TransformToParent(
				MathLib::scaling(0.01f, 0.01f, 0.01f) * light_proxy->RootNode()->TransformToParent());
			light_node->AddChild(light_proxy->RootNode());

			{
				auto& scene_mgr = Context::Instance().SceneManagerInstance();
				std::lock_guard<std::mutex> lock(scene_mgr.MutexForUpdate());
				scene_mgr.SceneRootNode().AddChild(light_node);
			}

			loading_percentage_ = 80;
			progress_bar->SetValue(loading_percentage_);
			msg->SetText(L"Initalizing Input System");
		}
		else if (loading_percentage_ < 90)
		{
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

			loading_percentage_ = 90;
			progress_bar->SetValue(loading_percentage_);
			msg->SetText(L"Initalizing UI");
		}
		else
		{
			id_scale_static_ = dialog_->IDFromName("ScaleStatic");
			id_scale_slider_ = dialog_->IDFromName("ScaleSlider");
			id_detail_type_static_ = dialog_->IDFromName("DetailTypeStatic");
			id_detail_type_combo_ = dialog_->IDFromName("DetailTypeCombo");
			id_occlusion_ = dialog_->IDFromName("Occlusion");
			id_na_length_ = dialog_->IDFromName("NaLength");
			id_wireframe_ = dialog_->IDFromName("Wireframe");

			dialog_->Control<UISlider>(id_scale_slider_)->SetValue(static_cast<int>(height_scale_ * 100));
			dialog_->Control<UISlider>(id_scale_slider_)->OnValueChangedEvent().Connect(
				[this](UISlider const & sender)
				{
					this->ScaleChangedHandler(sender);
				});
			this->ScaleChangedHandler(*dialog_->Control<UISlider>(id_scale_slider_));

			dialog_->Control<UIComboBox>(id_detail_type_combo_)->SetSelectedByIndex(2);
			dialog_->Control<UIComboBox>(id_detail_type_combo_)->OnSelectionChangedEvent().Connect(
				[this](UIComboBox const & sender)
				{
					this->DetailTypeChangedHandler(sender);
				});
			this->DetailTypeChangedHandler(*dialog_->Control<UIComboBox>(id_detail_type_combo_));

			dialog_->Control<UICheckBox>(id_occlusion_)->OnChangedEvent().Connect([this](UICheckBox const& sender) {
				this->OcclusionHandler(sender);
			});
			this->OcclusionHandler(*dialog_->Control<UICheckBox>(id_occlusion_));
			dialog_->Control<UICheckBox>(id_na_length_)->OnChangedEvent().Connect(
				[this](UICheckBox const & sender)
				{
					this->NaLengthHandler(sender);
				});
			this->NaLengthHandler(*dialog_->Control<UICheckBox>(id_na_length_));
			dialog_->Control<UICheckBox>(id_wireframe_)->OnChangedEvent().Connect(
				[this](UICheckBox const & sender)
				{
					this->WireframeHandler(sender);
				});
			this->WireframeHandler(*dialog_->Control<UICheckBox>(id_wireframe_));

			loading_percentage_ = 100;
			progress_bar->SetValue(loading_percentage_);
			msg->SetText(L"DONE");

			dialog_->SetVisible(true);
			dialog_loading->SetVisible(false);

			RenderDeviceCaps const & caps = Context::Instance().RenderFactoryInstance().RenderEngineInstance().DeviceCaps();
			if (!(caps.hs_support && caps.ds_support))
			{
				dialog_->Control<UIComboBox>(id_detail_type_combo_)->RemoveItem(4);
			}
		}

		renderEngine.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, Color(0.0f, 0.0f, 0.0f, 1), 1.0f, 0);
		return App3DFramework::URV_SkipPostProcess | App3DFramework::URV_Finished;
	}
	else
	{
		Color clear_clr(0.2f, 0.4f, 0.6f, 1);
		if (Context::Instance().Config().graphics_cfg.gamma)
		{
			clear_clr.r() = 0.029f;
			clear_clr.g() = 0.133f;
			clear_clr.b() = 0.325f;
		}
		renderEngine.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, clear_clr, 1.0f, 0);

		/*float degree(std::clock() / 700.0f);
		float3 lightPos(2, 0, 1);
		float4x4 matRot(MathLib::rotation_y(degree));
		lightPos = MathLib::transform_coord(lightPos, matRot);*/

		float3 light_pos(0.25f, 0.5f, -1.0f);
		light_pos = MathLib::transform_coord(light_pos, this->ActiveCamera().InverseViewMatrix());
		light_pos = MathLib::normalize(light_pos);
		light_->BoundSceneNode()->TransformToParent(MathLib::translation(light_pos));

		polygon_model_->ForEachMesh([this, &light_pos](Renderable& mesh)
			{
				auto& polygon_mesh = checked_cast<RenderPolygon&>(mesh);

				polygon_mesh.LightPos(light_pos);
				polygon_mesh.LightColor(light_->Color());
				polygon_mesh.LightFalloff(light_->Falloff());
			});

		return App3DFramework::URV_NeedFlush | App3DFramework::URV_Finished;
	}
}
