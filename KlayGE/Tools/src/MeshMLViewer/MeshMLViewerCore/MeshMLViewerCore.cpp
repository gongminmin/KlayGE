#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/FrameBuffer.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/RenderableHelper.hpp>
#include <KlayGE/Camera.hpp>
#include <KlayGE/SceneObjectHelper.hpp>
#include <KlayGE/DeferredRenderingLayer.hpp>
#include <KlayGE/UI.hpp>
#include <KlayGE/Mesh.hpp>
#include <KlayGE/Window.hpp>
#include <KlayGE/SceneManager.hpp>
#include <KlayGE/Input.hpp>
#include <KlayGE/InputFactory.hpp>

#include <sstream>
#include <fstream>

#include "MeshMLViewerCore.hpp"

using namespace std;
using namespace KlayGE;

namespace
{
	class RenderAxis : public RenderableHelper
	{
	public:
		RenderAxis()
			: RenderableHelper(L"Axis")
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();

			RenderEffectPtr effect = SyncLoadRenderEffect("MVUtil.fxml");
			simple_forward_tech_ = effect->TechniqueByName("AxisTech");
			mvp_param_ = effect->ParameterByName("mvp");

			float4 xyzs[] =
			{
				float4(0, 0, 0, 0),
				float4(1, 0, 0, 0),
				float4(0, 0, 0, 1),
				float4(0, 1, 0, 1),
				float4(0, 0, 0, 2),
				float4(0, 0, 1, 2),
			};

			rl_ = rf.MakeRenderLayout();
			rl_->TopologyType(RenderLayout::TT_LineList);

			GraphicsBufferPtr pos_vb = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, sizeof(xyzs), xyzs);

			rl_->BindVertexStream(pos_vb, std::make_tuple(vertex_element(VEU_Position, 0, EF_ABGR32F)));

			pos_aabb_ = MathLib::compute_aabbox(&xyzs[0], &xyzs[sizeof(xyzs) / sizeof(xyzs[0])]);
			tc_aabb_ = AABBox(float3(0, 0, 0), float3(0, 0, 0));

			effect_attrs_ |= EA_SimpleForward;
		}

		void OnRenderBegin()
		{
			Camera const & camera = Context::Instance().AppInstance().ActiveCamera();
			*mvp_param_ = model_mat_ * camera.ViewProjMatrix();
		}
	};

	class RenderGrid : public RenderableHelper
	{
	public:
		RenderGrid()
			: RenderableHelper(L"Grid")
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();

			RenderEffectPtr effect = SyncLoadRenderEffect("MVUtil.fxml");
			simple_forward_tech_ = effect->TechniqueByName("GridTech");
			mvp_param_ = effect->ParameterByName("mvp");

			float3 xyzs[(21 + 21) * 2];
			for (int i = 0; i < 21; ++ i)
			{
				xyzs[i * 2 + 0] = float3(-10.0f + i, 0, -10);
				xyzs[i * 2 + 1] = float3(-10.0f + i, 0, +10);

				xyzs[(i + 21) * 2 + 0] = float3(-10, 0, -10.0f + i);
				xyzs[(i + 21) * 2 + 1] = float3(+10, 0, -10.0f + i);
			}

			rl_ = rf.MakeRenderLayout();
			rl_->TopologyType(RenderLayout::TT_LineList);

			GraphicsBufferPtr pos_vb = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, sizeof(xyzs), xyzs);

			rl_->BindVertexStream(pos_vb, std::make_tuple(vertex_element(VEU_Position, 0, EF_BGR32F)));

			pos_aabb_ = MathLib::compute_aabbox(&xyzs[0], &xyzs[sizeof(xyzs) / sizeof(xyzs[0])]);
			tc_aabb_ = AABBox(float3(0, 0, 0), float3(0, 0, 0));

			effect_attrs_ |= EA_SimpleForward;
		}

		void OnRenderBegin()
		{
			Camera const & camera = Context::Instance().AppInstance().ActiveCamera();
			*mvp_param_ = model_mat_ * camera.ViewProjMatrix();
		}
	};

	class ModelObject : public SceneObjectHelper
	{
	public:
		explicit ModelObject(std::string const & name)
			: SceneObjectHelper(0)
		{
			renderable_ = SyncLoadModel(name, EAH_GPU_Read | EAH_Immutable,
				CreateModelFactory<DetailedSkinnedModel>(), CreateMeshFactory<DetailedSkinnedMesh>());
			checked_pointer_cast<DetailedSkinnedModel>(renderable_)->SetTime(0);
		}

		virtual ~ModelObject()
		{
			ResLoader::Instance().Unload(renderable_);
		}

		uint32_t NumFrames() const
		{
			return checked_pointer_cast<DetailedSkinnedModel>(renderable_)->NumFrames();
		}

		uint32_t FrameRate() const
		{
			return checked_pointer_cast<DetailedSkinnedModel>(renderable_)->FrameRate();
		}

		RenderablePtr const & Mesh(size_t id) const
		{
			return checked_pointer_cast<DetailedSkinnedModel>(renderable_)->Subrenderable(id);
		}

		uint32_t NumMeshes() const
		{
			return checked_pointer_cast<DetailedSkinnedModel>(renderable_)->NumSubrenderables();
		}

		void RebindJoints()
		{
			checked_pointer_cast<DetailedSkinnedModel>(renderable_)->RebindJoints();
		}

		void UnbindJoints()
		{
			checked_pointer_cast<DetailedSkinnedModel>(renderable_)->UnbindJoints();
		}

		RenderMaterialPtr const & GetMaterial(int32_t i) const
		{
			return checked_pointer_cast<DetailedSkinnedModel>(renderable_)->GetMaterial(i);
		}

		void SetTime(float time)
		{
			checked_pointer_cast<DetailedSkinnedModel>(renderable_)->SetTime(time);
		}

		void SetFrame(float frame)
		{
			checked_pointer_cast<DetailedSkinnedModel>(renderable_)->SetFrame(frame);
		}

		void VisualizeLighting()
		{
			checked_pointer_cast<DetailedSkinnedModel>(renderable_)->VisualizeLighting();
		}

		void VisualizeVertex(VertexElementUsage usage, uint8_t usage_index)
		{
			checked_pointer_cast<DetailedSkinnedModel>(renderable_)->VisualizeVertex(usage, usage_index);
		}

		void VisualizeTexture(int slot)
		{
			checked_pointer_cast<DetailedSkinnedModel>(renderable_)->VisualizeTexture(slot);
		}

		void SmoothMesh(bool smooth)
		{
			checked_pointer_cast<DetailedSkinnedModel>(renderable_)->SmoothMesh(smooth);
		}

		void SetTessFactor(int32_t tess_factor)
		{
			checked_pointer_cast<DetailedSkinnedModel>(renderable_)->SetTessFactor(tess_factor);
		}
	};

	class LightSourceUpdate
	{
	public:
		void operator()(LightSource& light, float /*app_time*/, float /*elapsed_time*/)
		{
			light.Direction(Context::Instance().AppInstance().ActiveCamera().ForwardVec());
		}
	};
}

namespace KlayGE
{
	MeshMLViewerCore::MeshMLViewerCore(void* native_wnd)
				: App3DFramework("MeshMLViewer", native_wnd),
					fps_controller_(false), tb_controller_(false), is_fps_camera_(false),
					skinning_(true), curr_frame_(0), mouse_down_in_wnd_(false), mouse_tracking_mode_(false),
					update_selective_buffer_(false), selected_obj_(0)
	{
		ResLoader::Instance().AddPath("../../Tools/media/MeshMLViewer");
	}

	bool MeshMLViewerCore::ConfirmDevice() const
	{
		return true;
	}

	void MeshMLViewerCore::Resize(uint32_t width, uint32_t height)
	{
		Context::Instance().RenderFactoryInstance().RenderEngineInstance().Resize(width, height);

		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		RenderEngine& re = rf.RenderEngineInstance();
		deferred_rendering_->SetupViewport(0, re.CurFrameBuffer(), 0);

		ElementFormat fmt;
		if (re.DeviceCaps().texture_format_support(EF_ABGR8))
		{
			fmt = EF_ABGR8;
		}
		else
		{
			BOOST_ASSERT(re.DeviceCaps().texture_format_support(EF_ABGR8));

			fmt = EF_ARGB8;
		}

		selective_tex_ = rf.MakeTexture2D(width, height, 1, 1, fmt, 1, 0, EAH_GPU_Write, NULL);
		selective_cpu_tex_ = rf.MakeTexture2D(width, height, 1, 1, fmt, 1, 0, EAH_CPU_Read, NULL);
		selective_fb_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*selective_tex_, 0, 1, 0));
		selective_fb_->Attach(FrameBuffer::ATT_DepthStencil, rf.Make2DDepthStencilRenderView(width, height, EF_D24S8, 1, 0));

		update_selective_buffer_ = true;
	}

	void MeshMLViewerCore::OnCreate()
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		RenderEngine& re = rf.RenderEngineInstance();

		font_ = SyncLoadFont("gkai00mp.kfont");

		deferred_rendering_ = Context::Instance().DeferredRenderingLayerInstance();
		deferred_rendering_->SSVOEnabled(0, false);

		light_ = MakeSharedPtr<DirectionalLightSource>();
		light_->Attrib(LightSource::LSA_NoShadow);
		light_->Color(float3(1.0f, 1.0f, 1.0f));
		light_->BindUpdateFunc(LightSourceUpdate());
		light_->AddToSceneManager();

		axis_ = MakeSharedPtr<SceneObjectHelper>(MakeSharedPtr<RenderAxis>(),
			SceneObject::SOA_Cullable | SceneObject::SOA_Moveable | SceneObject::SOA_NotCastShadow);
		axis_->AddToSceneManager();

		grid_ = MakeSharedPtr<SceneObjectHelper>(MakeSharedPtr<RenderGrid>(),
			SceneObject::SOA_Cullable | SceneObject::SOA_Moveable | SceneObject::SOA_NotCastShadow);
		grid_->AddToSceneManager();

		Color clear_clr(0.2f, 0.4f, 0.6f, 1);
		if (Context::Instance().Config().graphics_cfg.gamma)
		{
			clear_clr.r() = 0.029f;
			clear_clr.g() = 0.133f;
			clear_clr.b() = 0.325f;
		}
		uint32_t texel;
		ElementFormat fmt;
		if (re.DeviceCaps().texture_format_support(EF_ABGR8))
		{
			fmt = EF_ABGR8;
			texel = clear_clr.ABGR();
		}
		else
		{
			BOOST_ASSERT(re.DeviceCaps().texture_format_support(EF_ARGB8));

			fmt = EF_ARGB8;
			texel = clear_clr.ARGB();
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
		sky_box_->AddToSceneManager();

		selected_bb_ = MakeSharedPtr<SceneObjectHelper>(MakeSharedPtr<RenderableLineBox>(),
			SceneObject::SOA_Moveable | SceneObject::SOA_NotCastShadow);
		selected_bb_->Visible(false);
		selected_bb_->AddToSceneManager();
		checked_pointer_cast<RenderableLineBox>(selected_bb_->GetRenderable())->SetColor(Color(1, 1, 1, 1));

		this->LookAt(float3(-5, 5, -5), float3(0, 1, 0), float3(0.0f, 1.0f, 0.0f));
		this->Proj(0.1f, 100);

		tb_controller_.AttachCamera(this->ActiveCamera());

		selective_fb_ = rf.MakeFrameBuffer();
		selective_fb_->GetViewport()->camera = re.CurFrameBuffer()->GetViewport()->camera;
	}

	void MeshMLViewerCore::OnDestroy()
	{
		selective_fb_.reset();
		selective_tex_.reset();
		selective_cpu_tex_.reset();

		tb_controller_.DetachCamera();
		fps_controller_.DetachCamera();

		model_.reset();
		sky_box_.reset();
		grid_.reset();
		axis_.reset();
		light_.reset();
		selected_bb_.reset();

		font_.reset();
	}

	void MeshMLViewerCore::OnResize(uint32_t width, uint32_t height)
	{
		App3DFramework::OnResize(width, height);

		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		deferred_rendering_->SetupViewport(0, re.CurFrameBuffer(), 0);
	}

	void MeshMLViewerCore::OpenModel(std::string const & name)
	{
		if (!last_file_path_.empty())
		{
			ResLoader::Instance().DelPath(last_file_path_);
		}

		std::string file_name = name;
		last_file_path_ = file_name.substr(0, file_name.find_last_of('\\'));
		ResLoader::Instance().AddPath(last_file_path_);

		if (model_)
		{
			model_->DelFromSceneManager();
		}
		model_ = MakeSharedPtr<ModelObject>(name);
		model_->AddToSceneManager();
		for (size_t i = 0; i < model_->GetRenderable()->NumSubrenderables(); ++ i)
		{
			model_->GetRenderable()->Subrenderable(i)->ObjectID(static_cast<uint32_t>(i + 1));
		}

		shared_ptr<ModelObject> model = checked_pointer_cast<ModelObject>(model_);

		AABBox const & bb = model_->GetRenderable()->PosBound();
		float3 center = bb.Center();
		float3 half_size = bb.HalfSize();
		this->LookAt(center + float3(half_size.x() * 2, half_size.y() * 2.5f, half_size.z() * 3), float3(0, center.y(), 0), float3(0.0f, 1.0f, 0.0f));
		float far_plane = std::max(200.0f, MathLib::length(half_size) * 5);
		this->Proj(0.1f, far_plane);
		deferred_rendering_->LightDistance(far_plane);

		if (is_fps_camera_)
		{
			fps_controller_.AttachCamera(this->ActiveCamera());
		}
		else
		{
			tb_controller_.AttachCamera(this->ActiveCamera());
		}
		tb_controller_.Scalers(0.01f, MathLib::length(half_size) * 0.001f);
		fps_controller_.Scalers(0.01f, 0.2f);

		update_selective_buffer_ = true;
		selected_obj_ = 0;
		this->UpdateSelectedMesh();
	}

	void MeshMLViewerCore::SaveAsModel(std::string const & name)
	{
		SaveModel(checked_pointer_cast<DetailedSkinnedModel>(model_->GetRenderable()), name);
	}

	void MeshMLViewerCore::DoUpdateOverlay()
	{
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();

		std::wostringstream stream;
		stream.precision(2);
		stream << std::fixed << this->FPS() << " FPS";

		font_->RenderText(0, 0, Color(1, 1, 0, 1), re.ScreenFrameBuffer()->Description(), 16);
		font_->RenderText(0, 18, Color(1, 1, 0, 1), stream.str(), 16);
	}

	uint32_t MeshMLViewerCore::DoUpdate(uint32_t pass)
	{
		if (0 == pass)
		{
			Camera const & camera = this->ActiveCamera();

			if (model_)
			{
				AABBox bb = MathLib::transform_aabb(model_->GetRenderable()->PosBound(), camera.ViewMatrix())
					| MathLib::transform_aabb(grid_->GetRenderable()->PosBound(), camera.ViewMatrix());
				float near_plane = std::max(0.01f, bb.Min().z() * 0.8f);
				float far_plane = std::max(near_plane + 0.1f, bb.Max().z() * 1.2f);
				this->Proj(near_plane, far_plane);
			}

			RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
			uint32_t width = re.CurFrameBuffer()->Width();
			uint32_t height = re.CurFrameBuffer()->Height();
			float x = (width - 70.5f) / width * 2 - 1;
			float y = 1 - (height - 70.5f) / height * 2;
			float3 origin = MathLib::transform_coord(float3(x, y, 0.1f), camera.InverseViewProjMatrix());

			float x1 = (width - 20.5f) / width * 2 - 1;
			float3 x_dir = MathLib::transform_coord(float3(x1, y, 0.1f), camera.InverseViewProjMatrix());
			float len = MathLib::length(x_dir - origin);

			float4x4 scaling = MathLib::scaling(len, len, len);
			float4x4 trans = MathLib::translation(origin);
			axis_->ModelMatrix(scaling * trans);
		}

		uint32_t deferrd_pass_start;
		if (model_ && update_selective_buffer_)
		{
			deferrd_pass_start = 2;

			RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
			if (0 == pass)
			{
				axis_->Visible(false);
				grid_->Visible(false);
				sky_box_->Visible(false);
				selected_bb_->Visible(false);
				for (uint32_t i = 0; i < model_->GetRenderable()->NumSubrenderables(); ++ i)
				{
					model_->GetRenderable()->Subrenderable(i)->SelectMode(true);
				}

				re.BindFrameBuffer(selective_fb_);
				re.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth | FrameBuffer::CBM_Stencil,
					Color(0.0f, 0.0f, 0.0f, 1.0f), 1, 0);
				return App3DFramework::URV_NeedFlush;
			}
			else if (1 == pass)
			{
				axis_->Visible(true);
				grid_->Visible(true);
				sky_box_->Visible(true);
				selected_bb_->Visible(selected_obj_ > 0);
				for (uint32_t i = 0; i < model_->GetRenderable()->NumSubrenderables(); ++ i)
				{
					model_->GetRenderable()->Subrenderable(i)->SelectMode(false);
				}

				re.BindFrameBuffer(FrameBufferPtr());
				return 0;
			}
		}
		else
		{
			deferrd_pass_start = 0;
		}

		if (pass >= deferrd_pass_start)
		{
			uint32_t deferred_pass = pass - deferrd_pass_start;
			uint32_t urv = deferred_rendering_->Update(deferred_pass);
			if (urv & App3DFramework::URV_Finished)
			{
				selective_tex_->CopyToTexture(*selective_cpu_tex_);
				update_selective_buffer_ = false;
			}

			return urv;
		}
		else
		{
			BOOST_ASSERT(false);

			return 0;
		}
	}

	uint32_t MeshMLViewerCore::NumFrames() const
	{
		if (model_)
		{
			return checked_pointer_cast<ModelObject>(model_)->NumFrames();
		}
		else
		{
			return 0;
		}
	}

	void MeshMLViewerCore::CurrFrame(float frame)
	{
		if (skinning_)
		{
			checked_pointer_cast<ModelObject>(model_)->SetFrame(frame);
			curr_frame_ = frame;

			this->UpdateSelectedMesh();
		}
	}

	float MeshMLViewerCore::ModelFrameRate() const
	{
		return static_cast<float>(checked_pointer_cast<ModelObject>(model_)->FrameRate());
	}

	uint32_t MeshMLViewerCore::NumMeshes() const
	{
		return model_->GetRenderable()->NumSubrenderables();
	}

	std::wstring const & MeshMLViewerCore::MeshName(uint32_t index) const
	{
		return model_->GetRenderable()->Subrenderable(index)->Name();
	}

	uint32_t MeshMLViewerCore::NumVertexStreams(uint32_t mesh_id) const
	{
		Renderable const & mesh = *model_->GetRenderable()->Subrenderable(mesh_id);
		RenderLayout const & rl = mesh.GetRenderLayout();
		return rl.NumVertexStreams();
	}

	uint32_t MeshMLViewerCore::NumVertexStreamUsages(uint32_t mesh_id, uint32_t stream_index) const
	{
		Renderable const & mesh = *model_->GetRenderable()->Subrenderable(mesh_id);
		RenderLayout const & rl = mesh.GetRenderLayout();
		return static_cast<uint32_t>(rl.VertexStreamFormat(stream_index).size());
	}

	uint32_t MeshMLViewerCore::VertexStreamUsage(uint32_t mesh_id, uint32_t stream_index, uint32_t usage_index) const
	{
		Renderable const & mesh = *model_->GetRenderable()->Subrenderable(mesh_id);
		RenderLayout const & rl = mesh.GetRenderLayout();
		return (rl.VertexStreamFormat(stream_index)[usage_index].usage << 16)
			| (rl.VertexStreamFormat(stream_index)[usage_index].usage_index);
	}

	uint32_t MeshMLViewerCore::MaterialID(uint32_t mesh_id) const
	{
		StaticMeshPtr mesh = checked_pointer_cast<StaticMesh>(model_->GetRenderable()->Subrenderable(mesh_id - 1));
		return mesh->MaterialID();
	}

	float3 const & MeshMLViewerCore::AmbientMaterial(uint32_t mtl_id) const
	{
		RenderModelPtr model = checked_pointer_cast<RenderModel>(model_->GetRenderable());
		return model->GetMaterial(mtl_id)->ambient;
	}

	float3 const & MeshMLViewerCore::DiffuseMaterial(uint32_t mtl_id) const
	{
		RenderModelPtr model = checked_pointer_cast<RenderModel>(model_->GetRenderable());
		return model->GetMaterial(mtl_id)->diffuse;
	}

	float3 const & MeshMLViewerCore::SpecularMaterial(uint32_t mtl_id) const
	{
		RenderModelPtr model = checked_pointer_cast<RenderModel>(model_->GetRenderable());
		return model->GetMaterial(mtl_id)->specular;
	}

	float MeshMLViewerCore::ShininessMaterial(uint32_t mtl_id) const
	{
		RenderModelPtr model = checked_pointer_cast<RenderModel>(model_->GetRenderable());
		return model->GetMaterial(mtl_id)->shininess;
	}

	float3 const & MeshMLViewerCore::EmitMaterial(uint32_t mtl_id) const
	{
		RenderModelPtr model = checked_pointer_cast<RenderModel>(model_->GetRenderable());
		return model->GetMaterial(mtl_id)->emit;
	}

	float MeshMLViewerCore::OpacityMaterial(uint32_t mtl_id) const
	{
		RenderModelPtr model = checked_pointer_cast<RenderModel>(model_->GetRenderable());
		return model->GetMaterial(mtl_id)->opacity;
	}

	std::string const & MeshMLViewerCore::DiffuseTexture(uint32_t mtl_id) const
	{
		RenderModelPtr model = checked_pointer_cast<RenderModel>(model_->GetRenderable());
		for (auto const & slot : model->GetMaterial(mtl_id)->texture_slots)
		{
			size_t const slot_type_hash = RT_HASH(slot.first.c_str());
			if ((CT_HASH("Color") == slot_type_hash) || (CT_HASH("Diffuse Color") == slot_type_hash)
				|| (CT_HASH("Diffuse Color Map") == slot_type_hash))
			{
				return slot.second;
			}
		}
		static std::string const empty;
		return empty;
	}

	std::string const & MeshMLViewerCore::SpecularTexture(uint32_t mtl_id) const
	{
		RenderModelPtr model = checked_pointer_cast<RenderModel>(model_->GetRenderable());
		for (auto const & slot : model->GetMaterial(mtl_id)->texture_slots)
		{
			size_t const slot_type_hash = RT_HASH(slot.first.c_str());
			if ((CT_HASH("Specular Level") == slot_type_hash) || (CT_HASH("Specular Color") == slot_type_hash))
			{
				return slot.second;
			}
		}
		static std::string const empty;
		return empty;
	}

	std::string const & MeshMLViewerCore::ShininessTexture(uint32_t mtl_id) const
	{
		RenderModelPtr model = checked_pointer_cast<RenderModel>(model_->GetRenderable());
		for (auto const & slot : model->GetMaterial(mtl_id)->texture_slots)
		{
			size_t const slot_type_hash = RT_HASH(slot.first.c_str());
			if ((CT_HASH("Glossiness") == slot_type_hash) || (CT_HASH("Reflection Glossiness Map") == slot_type_hash))
			{
				return slot.second;
			}
		}
		static std::string const empty;
		return empty;
	}

	std::string const & MeshMLViewerCore::NormalTexture(uint32_t mtl_id) const
	{
		RenderModelPtr model = checked_pointer_cast<RenderModel>(model_->GetRenderable());
		for (auto const & slot : model->GetMaterial(mtl_id)->texture_slots)
		{
			size_t const slot_type_hash = RT_HASH(slot.first.c_str());
			if ((CT_HASH("Bump") == slot_type_hash) || (CT_HASH("Bump Map") == slot_type_hash))
			{
				return slot.second;
			}
		}
		static std::string const empty;
		return empty;
	}

	std::string const & MeshMLViewerCore::HeightTexture(uint32_t mtl_id) const
	{
		RenderModelPtr model = checked_pointer_cast<RenderModel>(model_->GetRenderable());
		for (auto const & slot : model->GetMaterial(mtl_id)->texture_slots)
		{
			size_t const slot_type_hash = RT_HASH(slot.first.c_str());
			if ((CT_HASH("Height") == slot_type_hash) || (CT_HASH("Height Map") == slot_type_hash))
			{
				return slot.second;
			}
		}
		static std::string const empty;
		return empty;
	}

	std::string const & MeshMLViewerCore::EmitTexture(uint32_t mtl_id) const
	{
		RenderModelPtr model = checked_pointer_cast<RenderModel>(model_->GetRenderable());
		for (auto const & slot : model->GetMaterial(mtl_id)->texture_slots)
		{
			size_t const slot_type_hash = RT_HASH(slot.first.c_str());
			if (CT_HASH("Self-Illumination") == slot_type_hash)
			{
				return slot.second;
			}
		}
		static std::string const empty;
		return empty;
	}

	std::string const &MeshMLViewerCore::OpacityTexture(uint32_t mtl_id) const
	{
		RenderModelPtr model = checked_pointer_cast<RenderModel>(model_->GetRenderable());
		for (auto const & slot : model->GetMaterial(mtl_id)->texture_slots)
		{
			size_t const slot_type_hash = RT_HASH(slot.first.c_str());
			if (CT_HASH("Opacity") == slot_type_hash)
			{
				return slot.second;
			}
		}
		static std::string const empty;
		return empty;
	}

	uint32_t MeshMLViewerCore::SelectedMesh() const
	{
		return selected_obj_;
	}

	void MeshMLViewerCore::SelectMesh(uint32_t mesh_id)
	{
		selected_obj_ = mesh_id;
		this->UpdateSelectedMesh();
	}

	void MeshMLViewerCore::SkinningOn(bool on)
	{
		skinning_ = on;
		if (skinning_)
		{
			checked_pointer_cast<ModelObject>(model_)->RebindJoints();
		}
		else
		{
			checked_pointer_cast<ModelObject>(model_)->UnbindJoints();
		}
	}

	void MeshMLViewerCore::SmoothMeshOn(bool on)
	{
		checked_pointer_cast<ModelObject>(model_)->SmoothMesh(on);
	}

	void MeshMLViewerCore::FPSCameraOn(bool on)
	{
		is_fps_camera_ = on;
		if (on)
		{
			tb_controller_.DetachCamera();
			fps_controller_.AttachCamera(this->ActiveCamera());
		}
		else
		{
			fps_controller_.DetachCamera();
			tb_controller_.AttachCamera(this->ActiveCamera());
		}
	}

	void MeshMLViewerCore::LineModeOn(bool on)
	{
		deferred_rendering_->ForceLineMode(on);
	}

	void MeshMLViewerCore::Visualize(int index)
	{
		if (model_)
		{
			RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();

			if (0 == index)
			{
				checked_pointer_cast<ModelObject>(model_)->VisualizeLighting();

				deferred_rendering_->SSVOEnabled(0, false);
				re.HDREnabled(true);
				re.PPAAEnabled(1);
				re.ColorGradingEnabled(true);
			}
			else
			{
				if (index < 10)
				{
					VertexElementUsage veu = static_cast<VertexElementUsage>(index - 1);
					checked_pointer_cast<ModelObject>(model_)->VisualizeVertex(veu, 0);
				}
				else
				{
					int slot = index - 10;
					checked_pointer_cast<ModelObject>(model_)->VisualizeTexture(slot);
				}

				deferred_rendering_->SSVOEnabled(0, false);
				re.HDREnabled(false);
				re.PPAAEnabled(0);
				re.ColorGradingEnabled(false);
			}
		}
	}

	void MeshMLViewerCore::MouseMove(int x, int y, uint32_t button)
	{
		if (mouse_down_in_wnd_)
		{
			mouse_tracking_mode_ = true;

			int2 pt(x, y);

			if (button & (MB_Left | MB_Middle | MB_Right))
			{
				float2 move_vec = pt - last_mouse_pt_;
				if (button & MB_Left)
				{
					if (is_fps_camera_)
					{
						fps_controller_.RotateRel(move_vec.x(), move_vec.y(), 0);
						update_selective_buffer_ = true;
					}
					else
					{
						tb_controller_.Rotate(move_vec.x(), move_vec.y());
						update_selective_buffer_ = true;
					}
				}
				else if (button & MB_Middle)
				{
					if (!is_fps_camera_)
					{
						tb_controller_.Move(move_vec.x(), move_vec.y());
						update_selective_buffer_ = true;
					}
				}
				else if (button & MB_Right)
				{
					if (!is_fps_camera_)
					{
						tb_controller_.Zoom(move_vec.x(), move_vec.y());
						update_selective_buffer_ = true;
					}
				}
			}

			last_mouse_pt_ = pt;
		}
	}

	void MeshMLViewerCore::MouseUp(int x, int y, uint32_t button)
	{
		if (mouse_down_in_wnd_)
		{
			mouse_down_in_wnd_ = false;

			if (mouse_tracking_mode_)
			{
				mouse_tracking_mode_ = false;
			}
			else
			{
				if (button & MB_Left)
				{
					uint32_t entity_id;
					Texture::Mapper mapper(*selective_cpu_tex_, 0, 0, TMA_Read_Only,
						x, y, 1, 1);
					uint8_t* p = mapper.Pointer<uint8_t>();
					if (0 == p[3])
					{
						if (EF_ABGR8 == selective_cpu_tex_->Format())
						{
							entity_id = p[0] | (p[1] << 8) | (p[2] << 16);
						}
						else
						{
							BOOST_ASSERT(EF_ARGB8 == selective_cpu_tex_->Format());

							entity_id = p[2] | (p[1] << 8) | (p[0] << 16);
						}
					}
					else
					{
						entity_id = 0;
					}

					selected_obj_ = entity_id;
					this->UpdateSelectedMesh();
				}
			}
		}
	}

	void MeshMLViewerCore::MouseDown(int x, int y, uint32_t button)
	{
		KFL_UNUSED(button);

		mouse_down_in_wnd_ = true;
		last_mouse_pt_ = int2(x, y);
	}

	void MeshMLViewerCore::KeyPress(int key)
	{
		switch (key)
		{
		case 'W':
			if (is_fps_camera_)
			{
				fps_controller_.Move(0, 0, 1);
				update_selective_buffer_ = true;
			}
			break;

		case 'S':
			if (is_fps_camera_)
			{
				fps_controller_.Move(0, 0, -1);
				update_selective_buffer_ = true;
			}
			break;

		case 'A':
			if (is_fps_camera_)
			{
				fps_controller_.Move(-1, 0, 0);
				update_selective_buffer_ = true;
			}
			break;

		case 'D':
			if (is_fps_camera_)
			{
				fps_controller_.Move(1, 0, 0);
				update_selective_buffer_ = true;
			}
			break;
		}
	}

	void MeshMLViewerCore::UpdateSelectedMesh()
	{
		if (selected_obj_ > 0)
		{
			RenderablePtr const & mesh = model_->GetRenderable()->Subrenderable(selected_obj_ - 1);
			OBBox obb;
			if ((this->NumFrames() > 0) && skinning_)
			{
				obb = MathLib::convert_to_obbox(
					checked_pointer_cast<SkinnedMesh>(mesh)->FramePosBound(static_cast<uint32_t>(curr_frame_ + 0.5f)));
			}
			else
			{
				obb = MathLib::convert_to_obbox(mesh->PosBound());
			}
			checked_pointer_cast<RenderableLineBox>(selected_bb_->GetRenderable())->SetBox(obb);
			selected_bb_->ModelMatrix(model_->ModelMatrix());
			selected_bb_->Visible(true);
		}
		else
		{
			selected_bb_->Visible(false);
		}
	}
}
