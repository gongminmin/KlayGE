#include <KlayGE/KlayGE.hpp>

#include <KFL/Math.hpp>
#include <KFL/Util.hpp>
#include <KlayGE/Camera.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/DeferredRenderingLayer.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/FrameBuffer.hpp>
#include <KlayGE/Light.hpp>
#include <KlayGE/Mesh.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/Renderable.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/SceneManager.hpp>
#include <KlayGE/SceneNode.hpp>
#include <KlayGE/SkyBox.hpp>

#include <KlayGE/InputFactory.hpp>
#include <KlayGE/RenderFactory.hpp>

#include <iterator>
#include <sstream>
#include <vector>

#include "DetailedSurfaceDR.hpp"
#include "SampleCommon.hpp"

using namespace std;
using namespace KlayGE;

namespace
{
	enum
	{
		Exit,
	};

	InputActionDefine actions[] = {
		InputActionDefine(Exit, KS_Escape),
	};
} // namespace


int SampleMain()
{
	ContextCfg cfg = Context::Instance().Config();
	cfg.deferred_rendering = true;
	Context::Instance().Config(cfg);

	DetailedSurfaceApp app;
	app.Create();
	app.Run();

	return 0;
}

DetailedSurfaceApp::DetailedSurfaceApp() : App3DFramework("DetailedSurface"), height_scale_(0.06f)
{
	ResLoader::Instance().AddPath("../../Samples/media/DetailedSurface");
	ResLoader::Instance().AddPath("../../Samples/media/DetailedSurfaceDR");
}

void DetailedSurfaceApp::OnCreate()
{
	font_ = SyncLoadFont("gkai00mp.kfont");

	deferred_rendering_ = Context::Instance().DeferredRenderingLayerInstance();

	this->LookAt(float3(-0.18f, 0.24f, -0.18f), float3(0, 0.05f, 0));
	this->Proj(0.01f, 100);

	mtls_[0][static_cast<uint32_t>(DetailTypes::None)] = ASyncLoadRenderMaterial("None.mtlml");
	mtls_[0][static_cast<uint32_t>(DetailTypes::Bump)] = ASyncLoadRenderMaterial("Bump.mtlml");
	mtls_[0][static_cast<uint32_t>(DetailTypes::Parallax)] = ASyncLoadRenderMaterial("Parallax.mtlml");
	mtls_[0][static_cast<uint32_t>(DetailTypes::ParallaxOcclusion)] = ASyncLoadRenderMaterial("ParallaxOcclusion.mtlml");
	mtls_[0][static_cast<uint32_t>(DetailTypes::FlatTessellation)] = ASyncLoadRenderMaterial("FlatTessellation.mtlml");
	mtls_[0][static_cast<uint32_t>(DetailTypes::SmoothTessellation)] = ASyncLoadRenderMaterial("SmoothTessellation.mtlml");
	for (uint32_t i = 0; i < static_cast<uint32_t>(DetailTypes::Count); ++i)
	{
		mtls_[1][i] = mtls_[0][i]->Clone();
		mtls_[1][i]->TextureName(RenderMaterial::TS_Occlusion, "occlusion.dds");
		mtls_[1][i]->LoadTextureSlots();
	}

	polygon_model_ = SyncLoadModel("teapot.glb", EAH_GPU_Read | EAH_Immutable, SceneNode::SOA_Cullable, AddToSceneRootHelper);

	tb_controller_.AttachCamera(this->ActiveCamera());
	tb_controller_.Scalers(0.01f, 0.001f);

	auto& scene_mgr = Context::Instance().SceneManagerInstance();
	auto& root_node = scene_mgr.SceneRootNode();

	auto& rf = Context::Instance().RenderFactoryInstance();
	auto const& caps = rf.RenderEngineInstance().DeviceCaps();

	Color clear_clr(0.2f, 0.4f, 0.6f, 1);
	if (Context::Instance().Config().graphics_cfg.gamma)
	{
		clear_clr.r() = 0.029f;
		clear_clr.g() = 0.133f;
		clear_clr.b() = 0.325f;
	}

	auto const fmt = caps.BestMatchTextureFormat(MakeSpan({EF_ABGR8, EF_ARGB8}));
	BOOST_ASSERT(fmt != EF_Unknown);
	uint32_t texel = ((fmt == EF_ABGR8) ? clear_clr.ABGR() : clear_clr.ARGB());
	ElementInitData init_data[6];
	for (int i = 0; i < 6; ++i)
	{
		init_data[i].data = &texel;
		init_data[i].row_pitch = sizeof(uint32_t);
		init_data[i].slice_pitch = init_data[i].row_pitch;
	}
	auto default_cube_map = rf.MakeTextureCube(1, 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_Immutable, MakeSpan(init_data));

	auto skybox_renderable = MakeSharedPtr<RenderableSkyBox>();
	auto skybox = MakeSharedPtr<SceneNode>(MakeSharedPtr<RenderableComponent>(skybox_renderable), SceneNode::SOA_NotCastShadow);
	skybox_renderable->CubeMap(default_cube_map);
	root_node.AddChild(skybox);

	light_ = MakeSharedPtr<PointLightSource>();
	light_->Attrib(0);
	light_->Color(float3(10, 10, 10));
	light_->Falloff(float3(1, 0, 1.0f));

	auto light_node = MakeSharedPtr<SceneNode>(SceneNode::SOA_Cullable);
	light_node->TransformToParent(MathLib::translation(0.25f, 0.5f, -1.0f));
	light_node->AddComponent(light_);

	auto light_proxy = LoadLightSourceProxyModel(light_);
	light_proxy->RootNode()->TransformToParent(MathLib::scaling(0.01f, 0.01f, 0.01f) * light_proxy->RootNode()->TransformToParent());
	light_node->AddChild(light_proxy->RootNode());

	{
		std::lock_guard<std::mutex> lock(scene_mgr.MutexForUpdate());
		root_node.AddChild(light_node);
	}

	InputEngine& inputEngine(Context::Instance().InputFactoryInstance().InputEngineInstance());
	InputActionMap actionMap;
	actionMap.AddActions(actions, actions + std::size(actions));

	action_handler_t input_handler = MakeSharedPtr<input_signal>();
	input_handler->Connect([this](InputEngine const& sender, InputAction const& action) { this->InputHandler(sender, action); });
	inputEngine.ActionMap(actionMap, input_handler);

	UIManager::Instance().Load(*ResLoader::Instance().Open("DetailedSurfaceDR.uiml"));

	dialog_ = UIManager::Instance().GetDialog("DetailedSurface");
	id_scale_static_ = dialog_->IDFromName("ScaleStatic");
	id_scale_slider_ = dialog_->IDFromName("ScaleSlider");
	id_detail_type_static_ = dialog_->IDFromName("DetailTypeStatic");
	id_detail_type_combo_ = dialog_->IDFromName("DetailTypeCombo");
	id_occlusion_ = dialog_->IDFromName("Occlusion");
	id_na_length_ = dialog_->IDFromName("NaLength");
	id_wireframe_ = dialog_->IDFromName("Wireframe");

	dialog_->Control<UISlider>(id_scale_slider_)->SetValue(static_cast<int>(height_scale_ * 100));
	dialog_->Control<UISlider>(id_scale_slider_)->OnValueChangedEvent().Connect([this](UISlider const& sender) {
		this->ScaleChangedHandler(sender);
	});
	this->ScaleChangedHandler(*dialog_->Control<UISlider>(id_scale_slider_));

	dialog_->Control<UIComboBox>(id_detail_type_combo_)->SetSelectedByIndex(2);
	dialog_->Control<UIComboBox>(id_detail_type_combo_)->OnSelectionChangedEvent().Connect([this](UIComboBox const& sender) {
		this->DetailTypeChangedHandler(sender);
	});
	this->DetailTypeChangedHandler(*dialog_->Control<UIComboBox>(id_detail_type_combo_));

	dialog_->Control<UICheckBox>(id_occlusion_)->OnChangedEvent().Connect([this](UICheckBox const& sender) {
		this->OcclusionHandler(sender);
	});
	this->OcclusionHandler(*dialog_->Control<UICheckBox>(id_occlusion_));
	dialog_->Control<UICheckBox>(id_na_length_)->OnChangedEvent().Connect([this](UICheckBox const& sender) {
		this->NaLengthHandler(sender);
	});
	this->NaLengthHandler(*dialog_->Control<UICheckBox>(id_na_length_));
	dialog_->Control<UICheckBox>(id_wireframe_)->OnChangedEvent().Connect([this](UICheckBox const& sender) {
		this->WireframeHandler(sender);
	});
	this->WireframeHandler(*dialog_->Control<UICheckBox>(id_wireframe_));

	if (!(caps.hs_support && caps.ds_support))
	{
		dialog_->Control<UIComboBox>(id_detail_type_combo_)->RemoveItem(4);
	}
}

void DetailedSurfaceApp::OnResize(uint32_t width, uint32_t height)
{
	App3DFramework::OnResize(width, height);

	RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
	deferred_rendering_->SetupViewport(0, re.CurFrameBuffer(), 0);

	UIManager::Instance().SettleCtrls();
}

void DetailedSurfaceApp::InputHandler(InputEngine const& /*sender*/, InputAction const& action)
{
	switch (action.first)
	{
	case Exit:
		this->Quit();
		break;
	}
}

void DetailedSurfaceApp::UpdateMaterial()
{
	auto const& mtl = mtls_[use_occlusion_map_][material_index_];
	mtl->HeightScale(height_scale_);

	polygon_model_->GetMaterial(0) = mtl;
	polygon_model_->ForEachMesh([&mtl](Renderable& mesh) { mesh.Material(mtl); });
}

void DetailedSurfaceApp::ScaleChangedHandler(KlayGE::UISlider const& sender)
{
	height_scale_ = sender.GetValue() / 100.0f;
	this->UpdateMaterial();

	std::wostringstream stream;
	stream << L"Scale: " << height_scale_;
	dialog_->Control<UIStatic>(id_scale_static_)->SetText(stream.str());
}

void DetailedSurfaceApp::DetailTypeChangedHandler(KlayGE::UIComboBox const& sender)
{
	material_index_ = sender.GetSelectedIndex();
	this->UpdateMaterial();
}

void DetailedSurfaceApp::OcclusionHandler(KlayGE::UICheckBox const& sender)
{
	use_occlusion_map_ = sender.GetChecked();
	this->UpdateMaterial();
}

void DetailedSurfaceApp::NaLengthHandler(KlayGE::UICheckBox const& sender)
{
	// Na Length textures are not supported by deferred rendering layer, yet
	KFL_UNUSED(sender);
}

void DetailedSurfaceApp::WireframeHandler(KlayGE::UICheckBox const& sender)
{
	bool const wf = sender.GetChecked();
	deferred_rendering_->ForceLineMode(wf);
}

void DetailedSurfaceApp::DoUpdateOverlay()
{
	UIManager::Instance().Render();

	std::wostringstream stream;
	stream.precision(2);
	stream << std::fixed << this->FPS() << " FPS";

	font_->RenderText(0, 0, Color(1, 1, 0, 1), L"Detailed Surface with Deferred Rendering", 16);
	font_->RenderText(0, 18, Color(1, 1, 0, 1), stream.str(), 16);

	SceneManager& sceneMgr(Context::Instance().SceneManagerInstance());
	stream.str(L"");
	stream << sceneMgr.NumRenderablesRendered() << " Renderables " << sceneMgr.NumPrimitivesRendered() << " Primitives "
		   << sceneMgr.NumVerticesRendered() << " Vertices";
	font_->RenderText(0, 36, Color(1, 1, 1, 1), stream.str(), 16);

	uint32_t const num_loading_res = ResLoader::Instance().NumLoadingResources();
	if (num_loading_res > 0)
	{
		stream.str(L"");
		stream << "Loading " << num_loading_res << " resources...";
		font_->RenderText(100, 300, Color(1, 0, 0, 1), stream.str(), 48);
	}
}

uint32_t DetailedSurfaceApp::DoUpdate(uint32_t pass)
{
	float3 light_pos(0.25f, 0.5f, -1.0f);
	light_pos = MathLib::transform_coord(light_pos, this->ActiveCamera().InverseViewMatrix());
	light_pos = MathLib::normalize(light_pos);
	light_->BoundSceneNode()->TransformToParent(MathLib::translation(light_pos));

	return deferred_rendering_->Update(pass);
}
