#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/GraphicsBuffer.hpp>
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
#include <KlayGE/SceneObjectHelper.hpp>
#include <KlayGE/UI.hpp>
#include <KlayGE/Light.hpp>

#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/InputFactory.hpp>

#include <vector>
#include <sstream>
#include <boost/bind.hpp>

#include "PNTriangles.hpp"

using namespace std;
using namespace KlayGE;

namespace
{
	enum TessMode
	{
		TM_HWTess,
		TM_InstancedTess,
		TM_No
	};

	std::vector<GraphicsBufferPtr> tess_pattern_vbs;
	std::vector<GraphicsBufferPtr> tess_pattern_ibs;

	void InitInstancedTessBuffs()
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();

		tess_pattern_vbs.resize(32);
		tess_pattern_ibs.resize(tess_pattern_vbs.size());

		ElementInitData init_data;
		
		std::vector<float2> vert;
		vert.push_back(float2(0, 0));
		vert.push_back(float2(1, 0));
		vert.push_back(float2(0, 1));
		init_data.row_pitch = static_cast<uint32_t>(vert.size() * sizeof(vert[0]));
		init_data.slice_pitch = 0;
		init_data.data = &vert[0];
		tess_pattern_vbs[0] = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read, &init_data);

		std::vector<uint16_t> index;
		index.push_back(0);
		index.push_back(1);
		index.push_back(2);
		init_data.row_pitch = static_cast<uint32_t>(index.size() * sizeof(index[0]));
		init_data.slice_pitch = 0;
		init_data.data = &index[0];
		tess_pattern_ibs[0] = rf.MakeIndexBuffer(BU_Static, EAH_GPU_Read, &init_data);

		for (size_t i = 1; i < tess_pattern_vbs.size(); ++ i)
		{
			for (size_t j = 0; j < vert.size(); ++ j)
			{
				float f = i / (i + 1.0f);
				vert[j] *= f;
			}

			for (size_t j = 0; j < i + 1; ++ j)
			{
				vert.push_back(float2(1 - j / (i + 1.0f), j / (i + 1.0f)));
			}
			vert.push_back(float2(0, 1));

			uint16_t last_1_row = static_cast<uint16_t>(vert.size() - (i + 2));
			uint16_t last_2_row = static_cast<uint16_t>(last_1_row - (i + 1));

			for (size_t j = 0; j < i; ++ j)
			{
				index.push_back(static_cast<uint16_t>(last_2_row + j));
				index.push_back(static_cast<uint16_t>(last_1_row + j));
				index.push_back(static_cast<uint16_t>(last_1_row + j + 1));

				index.push_back(static_cast<uint16_t>(last_2_row + j));
				index.push_back(static_cast<uint16_t>(last_1_row + j + 1));
				index.push_back(static_cast<uint16_t>(last_2_row + j + 1));
			}
			index.push_back(static_cast<uint16_t>(last_2_row + i));
			index.push_back(static_cast<uint16_t>(last_1_row + i));
			index.push_back(static_cast<uint16_t>(last_1_row + i + 1));

			init_data.row_pitch = static_cast<uint32_t>(vert.size() * sizeof(vert[0]));
			init_data.slice_pitch = 0;
			init_data.data = &vert[0];
			tess_pattern_vbs[i] = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read, &init_data);

			init_data.row_pitch = static_cast<uint32_t>(index.size() * sizeof(index[0]));
			init_data.slice_pitch = 0;
			init_data.data = &index[0];
			tess_pattern_ibs[i] = rf.MakeIndexBuffer(BU_Static, EAH_GPU_Read, &init_data);
		}
	}

	class PNTrianglesSkinnedModel;

	class PNTrianglesSkinnedMesh : public SkinnedMesh
	{
	public:
		PNTrianglesSkinnedMesh(RenderModelPtr const & model, std::wstring const & name)
			: SkinnedMesh(model, name),
				tess_factor_(5), line_mode_(false)
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();
			RenderDeviceCaps const & caps = rf.RenderEngineInstance().DeviceCaps();

			RenderEffectPtr effect = rf.LoadEffect("PNTriangles.fxml");
			if (caps.hs_support && caps.ds_support)
			{
				tess_mode_ = TM_HWTess;

				technique_ = effect->TechniqueByName("PNTriangles");
				if (!technique_->Validate())
				{
					tess_mode_ = TM_InstancedTess;

					technique_ = effect->TechniqueByName("InstTessPNTriangles");
					if (!technique_->Validate())
					{
						tess_mode_ = TM_No;
						technique_ = effect->TechniqueByName("NoPNTriangles");
					}
				}
			}
			else if (caps.max_shader_model >= 4)
			{
				tess_mode_ = TM_InstancedTess;
				
				technique_ = effect->TechniqueByName("InstTessPNTriangles");
				if (!technique_->Validate())
				{
					tess_mode_ = TM_No;
					technique_ = effect->TechniqueByName("NoPNTriangles");
				}
			}
			else
			{
				tess_mode_ = TM_No;
				technique_ = effect->TechniqueByName("NoPNTriangles");
			}

			if (TM_No == tess_mode_)
			{
				pn_enabled_ = false;
				rl_->TopologyType(RenderLayout::TT_TriangleList);
			}
			else
			{
				pn_enabled_ = true;
				if (TM_HWTess == tess_mode_)
				{
					rl_->TopologyType(RenderLayout::TT_3_Ctrl_Pt_PatchList);
				}
			}

			if (tess_mode_ != TM_No)
			{
				tess_pattern_rl_ = rf.MakeRenderLayout();
				tess_pattern_rl_->TopologyType(RenderLayout::TT_TriangleList);

				skinned_pos_vb_ = rf.MakeVertexBuffer(BU_Dynamic, EAH_GPU_Read | EAH_GPU_Write, NULL, EF_ABGR32F);
				skinned_normal_vb_ = rf.MakeVertexBuffer(BU_Dynamic, EAH_GPU_Read | EAH_GPU_Write, NULL, EF_ABGR32F);
				skinned_rl_ = rf.MakeRenderLayout();
				skinned_rl_->TopologyType(RenderLayout::TT_TriangleList);

				point_rl_ = rf.MakeRenderLayout();
				point_rl_->TopologyType(RenderLayout::TT_PointList);

				bindable_ib_ = rf.MakeVertexBuffer(BU_Dynamic, EAH_GPU_Read, NULL, EF_R16UI);

				*(effect->ParameterByName("skinned_pos_buf")) = skinned_pos_vb_;
				*(effect->ParameterByName("skinned_normal_buf")) = skinned_normal_vb_;
				*(effect->ParameterByName("index_buf")) = bindable_ib_;
			}

			mesh_rl_ = rl_;
		}

		void BuildMeshInfo()
		{
			TexturePtr dm;
			RenderModel::Material const & mtl = model_.lock()->GetMaterial(this->MaterialID());
			RenderModel::TextureSlotsType const & texture_slots = mtl.texture_slots;
			for (RenderModel::TextureSlotsType::const_iterator iter = texture_slots.begin();
				iter != texture_slots.end(); ++ iter)
			{
				if (("DiffuseMap" == iter->first) || ("Diffuse Color" == iter->first))
				{
					if (!ResLoader::Instance().Locate(iter->second).empty())
					{
						dm = LoadTexture(iter->second, EAH_GPU_Read)();
					}
				}
			}
			*(technique_->Effect().ParameterByName("diffuse_tex")) = dm;

			if (tess_mode_ != TM_No)
			{
				skinned_pos_vb_->Resize(this->NumVertices() * sizeof(float4));
				skinned_normal_vb_->Resize(this->NumVertices() * sizeof(float4));
				skinned_rl_->BindVertexStream(skinned_pos_vb_, boost::make_tuple(vertex_element(VEU_Position, 0, EF_ABGR32F)));
				skinned_rl_->BindVertexStream(skinned_normal_vb_, boost::make_tuple(vertex_element(VEU_TextureCoord, 0, EF_ABGR32F)));
				skinned_rl_->BindIndexStream(rl_->GetIndexStream(), rl_->IndexStreamFormat());

				for (uint32_t i = 0; i < rl_->NumVertexStreams(); ++ i)
				{
					point_rl_->BindVertexStream(rl_->GetVertexStream(i), rl_->VertexStreamFormat(i));
				}

				bindable_ib_->Resize(rl_->GetIndexStream()->Size());
				rl_->GetIndexStream()->CopyToBuffer(*bindable_ib_);
			}
		}

		void SetModelMatrix(float4x4 const & model_matrix)
		{
			model_matrix_ = model_matrix;
		}

		void LineMode(bool line)
		{
			line_mode_ = line;
			this->UpdateTech();
		}

		void AdaptiveTess(bool adaptive)
		{
			*(technique_->Effect().ParameterByName("adaptive_tess")) = adaptive;
		}

		void SetTessFactor(int32_t tess_factor)
		{
			if (TM_InstancedTess == tess_mode_)
			{
				tess_factor = std::min(tess_factor, static_cast<int32_t>(tess_pattern_vbs.size()));

				tess_pattern_rl_->BindIndexStream(tess_pattern_ibs[tess_factor - 1], EF_R16UI);
				tess_pattern_rl_->BindVertexStream(tess_pattern_vbs[tess_factor - 1], boost::make_tuple(vertex_element(VEU_TextureCoord, 1, EF_GR32F)),
					RenderLayout::ST_Geometry, mesh_rl_->NumIndices() * 3);
			}

			tess_factor_ = static_cast<float>(tess_factor);
		}

		void EnablePNTriangles(bool pn)
		{
			if (tess_mode_ != TM_No)
			{
				pn_enabled_ = pn;
			}
			this->UpdateTech();
		}

		void LightDir(float3 const & dir)
		{
			*(technique_->Effect().ParameterByName("light_dir")) = dir;
		}

		void UpdateTech()
		{
			if (pn_enabled_)
			{
				if (line_mode_)
				{
					technique_ = technique_->Effect().TechniqueByName("PNTrianglesLine");
				}
				else
				{
					technique_ = technique_->Effect().TechniqueByName("PNTriangles");
				}
				mesh_rl_->TopologyType(RenderLayout::TT_3_Ctrl_Pt_PatchList);
				rl_ = mesh_rl_;
			}
			else
			{
				if (line_mode_)
				{
					technique_ = technique_->Effect().TechniqueByName("NoPNTrianglesLine");
				}
				else
				{
					technique_ = technique_->Effect().TechniqueByName("NoPNTriangles");
				}
				mesh_rl_->TopologyType(RenderLayout::TT_TriangleList);
			}
		}

		void OnRenderBegin()
		{
			App3DFramework const & app = Context::Instance().AppInstance();

			float4x4 const & view = app.ActiveCamera().ViewMatrix();
			float4x4 const & proj = app.ActiveCamera().ProjMatrix();

			*(technique_->Effect().ParameterByName("world")) = model_matrix_;
			*(technique_->Effect().ParameterByName("view")) = view;
			*(technique_->Effect().ParameterByName("worldview")) = model_matrix_ * view;
			*(technique_->Effect().ParameterByName("viewproj")) = view * proj;
			*(technique_->Effect().ParameterByName("worldviewproj")) = model_matrix_ * view * proj;
			*(technique_->Effect().ParameterByName("eye_pos")) = app.ActiveCamera().EyePos();

			*(technique_->Effect().ParameterByName("tess_factors")) = float4(tess_factor_, tess_factor_, 1.0f, 9.0f);

			RenderModelPtr model = model_.lock();
			if (model)
			{
				*(technique_->Effect().ParameterByName("joint_rots")) = checked_pointer_cast<SkinnedModel>(model)->GetBindRotations();
				*(technique_->Effect().ParameterByName("joint_poss")) = checked_pointer_cast<SkinnedModel>(model)->GetBindPositions();
			}
		}

		void Render()
		{
			if (pn_enabled_)
			{
				if (TM_HWTess == tess_mode_)
				{
					rl_ = mesh_rl_;
					SkinnedMesh::Render();
				}
				else
				{
					RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
					re.BindSOBuffers(skinned_rl_);
					rl_ = point_rl_;
					technique_ = technique_->Effect().TechniqueByName("SkinnedStreamOut");
					SkinnedMesh::Render();
					re.BindSOBuffers(RenderLayoutPtr());

					if (line_mode_)
					{
						technique_ = technique_->Effect().TechniqueByName("InstTessPNTrianglesLine");
					}
					else
					{
						technique_ = technique_->Effect().TechniqueByName("InstTessPNTriangles");
					}
					rl_ = tess_pattern_rl_;
					SkinnedMesh::Render();
				}
			}
			else
			{
				rl_ = mesh_rl_;
				SkinnedMesh::Render();
			}
		}

	private:
		float4x4 model_matrix_;
		float tess_factor_;
		bool line_mode_;
		bool pn_enabled_;
		TessMode tess_mode_;

		RenderLayoutPtr mesh_rl_;
		RenderLayoutPtr point_rl_;
		RenderLayoutPtr skinned_rl_;
		RenderLayoutPtr tess_pattern_rl_;
		GraphicsBufferPtr skinned_pos_vb_;
		GraphicsBufferPtr skinned_normal_vb_;
		GraphicsBufferPtr bindable_ib_;
	};

	class PNTrianglesSkinnedModel : public SkinnedModel
	{
	public:
		PNTrianglesSkinnedModel(std::wstring const & name)
			: SkinnedModel(name)
		{
		}

		void SetModelMatrix(float4x4 const & model_matrix)
		{
			for (uint32_t i = 0; i < this->NumMeshes(); ++ i)
			{
				checked_pointer_cast<PNTrianglesSkinnedMesh>(Mesh(i))->SetModelMatrix(model_matrix);
			}
		}

		void LineMode(bool line)
		{
			for (uint32_t i = 0; i < this->NumMeshes(); ++ i)
			{
				checked_pointer_cast<PNTrianglesSkinnedMesh>(Mesh(i))->LineMode(line);
			}
		}

		void SetTessFactor(int32_t tess_factor)
		{
			for (uint32_t i = 0; i < this->NumMeshes(); ++ i)
			{
				checked_pointer_cast<PNTrianglesSkinnedMesh>(Mesh(i))->SetTessFactor(tess_factor);
			}
		}

		void AdaptiveTess(bool adaptive)
		{
			for (uint32_t i = 0; i < this->NumMeshes(); ++ i)
			{
				checked_pointer_cast<PNTrianglesSkinnedMesh>(Mesh(i))->AdaptiveTess(adaptive);
			}
		}

		void EnablePNTriangles(bool pn)
		{
			for (uint32_t i = 0; i < this->NumMeshes(); ++ i)
			{
				checked_pointer_cast<PNTrianglesSkinnedMesh>(Mesh(i))->EnablePNTriangles(pn);
			}
		}
		
		void LightDir(float3 const & dir)
		{
			for (uint32_t i = 0; i < this->NumMeshes(); ++ i)
			{
				checked_pointer_cast<PNTrianglesSkinnedMesh>(Mesh(i))->LightDir(dir);
			}
		}
	};

	class PolygonObject : public SceneObjectHelper
	{
	public:
		PolygonObject()
			: SceneObjectHelper(SOA_Cullable)
		{
			renderable_ = LoadModel("archer_attacking.meshml", EAH_GPU_Read, CreateModelFactory<PNTrianglesSkinnedModel>(), CreateMeshFactory<PNTrianglesSkinnedMesh>())();
			model_matrix_ = float4x4::Identity();
			checked_pointer_cast<PNTrianglesSkinnedModel>(renderable_)->SetModelMatrix(model_matrix_);
		}

		float4x4 const & GetModelMatrix() const
		{
			return model_matrix_;
		}

		void SetTessFactor(int32_t tess_factor)
		{
			checked_pointer_cast<PNTrianglesSkinnedModel>(renderable_)->SetTessFactor(tess_factor);
		}

		void LineMode(bool line)
		{
			checked_pointer_cast<PNTrianglesSkinnedModel>(renderable_)->LineMode(line);
		}

		void AdaptiveTess(bool adaptive)
		{
			checked_pointer_cast<PNTrianglesSkinnedModel>(renderable_)->AdaptiveTess(adaptive);
		}

		void EnablePNTriangles(bool pn)
		{
			checked_pointer_cast<PNTrianglesSkinnedModel>(renderable_)->EnablePNTriangles(pn);
		}

		void LightDir(float3 const & dir)
		{
			checked_pointer_cast<PNTrianglesSkinnedModel>(renderable_)->LightDir(dir);
		}

		void SetFrame(float frame)
		{
			checked_pointer_cast<PNTrianglesSkinnedModel>(renderable_)->SetFrame(frame);
		}

		uint32_t FrameRate() const
		{
			return checked_pointer_cast<PNTrianglesSkinnedModel>(renderable_)->FrameRate();
		}

		uint32_t StartFrame() const
		{
			return checked_pointer_cast<PNTrianglesSkinnedModel>(renderable_)->StartFrame();
		}

		uint32_t EndFrame() const
		{
			return checked_pointer_cast<PNTrianglesSkinnedModel>(renderable_)->EndFrame();
		}

	private:
		float4x4 model_matrix_;
	};


	enum
	{
		Exit,
		FullScreen,
	};

	InputActionDefine actions[] =
	{
		InputActionDefine(Exit, KS_Escape),
		InputActionDefine(FullScreen, KS_Enter),
	};
}

int main()
{
	ResLoader::Instance().AddPath("../../Samples/media/Common");

	Context::Instance().LoadCfg("KlayGE.cfg");

	PNTrianglesApp app;
	app.Create();
	app.Run();

	return 0;
}

PNTrianglesApp::PNTrianglesApp()
					: App3DFramework("PNTriangles"),
						tess_factor_(5)
{
	ResLoader::Instance().AddPath("../../Samples/media/PNTriangles");
}

bool PNTrianglesApp::ConfirmDevice() const
{
	RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
	RenderDeviceCaps const & caps = re.DeviceCaps();
	if (caps.max_shader_model < 2)
	{
		return false;
	}
	return true;
}

void PNTrianglesApp::InitObjects()
{
	InitInstancedTessBuffs();

	// ½¨Á¢×ÖÌå
	font_ = Context::Instance().RenderFactoryInstance().MakeFont("gkai00mp.kfont");

	polygon_ = MakeSharedPtr<PolygonObject>();
	checked_pointer_cast<PolygonObject>(polygon_)->SetFrame(0);
	polygon_->AddToSceneManager();

	this->LookAt(float3(-1.3f, 3, -2.2f), float3(0, 1.5f, 0));
	this->Proj(0.1f, 100);

	fpcController_.Scalers(0.05f, 0.1f);

	light_ = MakeSharedPtr<DirectionalLightSource>();
	light_->Attrib(0);
	light_->Color(float3(1, 1, 1));
	light_->Direction(-float3(1, 2, -1));
	light_->AddToSceneManager();

	light_proxy_ = MakeSharedPtr<SceneObjectLightSourceProxy>(light_);
	checked_pointer_cast<SceneObjectLightSourceProxy>(light_proxy_)->Scaling(0.5f, 0.5f, 0.5f);
	checked_pointer_cast<SceneObjectLightSourceProxy>(light_proxy_)->Translation(2.0f, 4.0f, -2.0f);
	light_proxy_->AddToSceneManager();

	InputEngine& inputEngine(Context::Instance().InputFactoryInstance().InputEngineInstance());
	InputActionMap actionMap;
	actionMap.AddActions(actions, actions + sizeof(actions) / sizeof(actions[0]));

	action_handler_t input_handler = MakeSharedPtr<input_signal>();
	input_handler->connect(boost::bind(&PNTrianglesApp::InputHandler, this, _1, _2));
	inputEngine.ActionMap(actionMap, input_handler, true);

	UIManager::Instance().Load(ResLoader::Instance().Load("PNTriangles.uiml"));
	dialog_params_ = UIManager::Instance().GetDialog("Parameters");
	id_warning_static_ = dialog_params_->IDFromName("WarningStatic");
	id_tess_static_ = dialog_params_->IDFromName("TessStatic");
	id_tess_slider_ = dialog_params_->IDFromName("TessSlider");
	id_line_mode_ = dialog_params_->IDFromName("LineModeCheck");
	id_adaptive_tess_ = dialog_params_->IDFromName("AdaptiveTess");
	id_enable_pn_triangles_ = dialog_params_->IDFromName("EnablePNTriangles");
	id_animation_ = dialog_params_->IDFromName("Animation");
	id_fps_camera_ = dialog_params_->IDFromName("FPSCamera");

	dialog_params_->Control<UISlider>(id_tess_slider_)->OnValueChangedEvent().connect(boost::bind(&PNTrianglesApp::TessChangedHandler, this, _1));
	this->TessChangedHandler(*dialog_params_->Control<UISlider>(id_tess_slider_));

	dialog_params_->Control<UICheckBox>(id_line_mode_)->OnChangedEvent().connect(boost::bind(&PNTrianglesApp::LineModeHandler, this, _1));
	dialog_params_->Control<UICheckBox>(id_adaptive_tess_)->OnChangedEvent().connect(boost::bind(&PNTrianglesApp::AdaptiveTessHandler, this, _1));
	this->AdaptiveTessHandler(*dialog_params_->Control<UICheckBox>(id_adaptive_tess_));
	dialog_params_->Control<UICheckBox>(id_enable_pn_triangles_)->OnChangedEvent().connect(boost::bind(&PNTrianglesApp::EnablePNTrianglesHandler, this, _1));
	dialog_params_->Control<UICheckBox>(id_animation_)->OnChangedEvent().connect(boost::bind(&PNTrianglesApp::AnimationHandler, this, _1));
	this->AnimationHandler(*dialog_params_->Control<UICheckBox>(id_animation_));
	dialog_params_->Control<UICheckBox>(id_fps_camera_)->OnChangedEvent().connect(boost::bind(&PNTrianglesApp::FPSCameraHandler, this, _1));

	RenderFactory& rf = Context::Instance().RenderFactoryInstance();
	RenderDeviceCaps const & caps = rf.RenderEngineInstance().DeviceCaps();
	bool tess_support = (caps.max_shader_model >= 4);

	dialog_params_->Control<UIStatic>(id_warning_static_)->SetVisible(!tess_support);

	dialog_params_->Control<UISlider>(id_tess_slider_)->SetEnabled(tess_support);
	dialog_params_->Control<UIStatic>(id_tess_static_)->SetEnabled(tess_support);

	dialog_params_->Control<UICheckBox>(id_adaptive_tess_)->SetChecked(tess_support);
	dialog_params_->Control<UICheckBox>(id_adaptive_tess_)->SetEnabled(tess_support);

	dialog_params_->Control<UICheckBox>(id_enable_pn_triangles_)->SetChecked(tess_support);
	dialog_params_->Control<UICheckBox>(id_enable_pn_triangles_)->SetEnabled(tess_support);
}

void PNTrianglesApp::OnResize(uint32_t width, uint32_t height)
{
	UIManager::Instance().SettleCtrls(width, height);
}

void PNTrianglesApp::InputHandler(InputEngine const & /*sender*/, InputAction const & action)
{
	switch (action.first)
	{
	case FullScreen:
		{
			RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			renderEngine.EndFrame();
			ContextCfg const & cfg = Context::Instance().Config();
			renderEngine.Resize(cfg.graphics_cfg.width, cfg.graphics_cfg.height);
			renderEngine.FullScreen(!renderEngine.FullScreen());
			renderEngine.BeginFrame();
		}
		break;

	case Exit:
		this->Quit();
		break;
	}
}

void PNTrianglesApp::TessChangedHandler(UISlider const & sender)
{
	tess_factor_ = sender.GetValue();
	checked_pointer_cast<PolygonObject>(polygon_)->SetTessFactor(tess_factor_);

	std::wostringstream stream;
	stream << L"Tessellation factor: " << tess_factor_;
	dialog_params_->Control<UIStatic>(id_tess_static_)->SetText(stream.str());
}

void PNTrianglesApp::LineModeHandler(UICheckBox const & sender)
{
	checked_pointer_cast<PolygonObject>(polygon_)->LineMode(sender.GetChecked());
}

void PNTrianglesApp::AdaptiveTessHandler(UICheckBox const & sender)
{
	checked_pointer_cast<PolygonObject>(polygon_)->AdaptiveTess(sender.GetChecked());
}

void PNTrianglesApp::EnablePNTrianglesHandler(UICheckBox const & sender)
{
	checked_pointer_cast<PolygonObject>(polygon_)->EnablePNTriangles(sender.GetChecked());
}

void PNTrianglesApp::AnimationHandler(KlayGE::UICheckBox const & sender)
{
	animation_ = sender.GetChecked();
}

void PNTrianglesApp::FPSCameraHandler(UICheckBox const & sender)
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

void PNTrianglesApp::DoUpdateOverlay()
{
	UIManager::Instance().Render();

	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

	std::wostringstream stream;
	stream.precision(2);
	stream << std::fixed << this->FPS() << " FPS";

	font_->RenderText(0, 0, Color(1, 1, 0, 1), L"PNTriangles", 16);
	font_->RenderText(0, 18, Color(1, 1, 0, 1), stream.str(), 16);
	font_->RenderText(0, 36, Color(1, 1, 0, 1), renderEngine.Name(), 16);
}

uint32_t PNTrianglesApp::DoUpdate(uint32_t /*pass*/)
{
	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
	renderEngine.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, Color(0.2f, 0.4f, 0.6f, 1), 1.0f, 0);

	boost::shared_ptr<PolygonObject> obj = checked_pointer_cast<PolygonObject>(polygon_);

	if (animation_)
	{
		float this_time = static_cast<float>(ani_timer_.elapsed());
		obj->SetFrame(this_time * obj->FrameRate());
	}
	obj->LightDir(MathLib::transform_coord(light_->Direction(), this->ActiveCamera().ViewMatrix()));

	return App3DFramework::URV_Need_Flush | App3DFramework::URV_Finished;
}
