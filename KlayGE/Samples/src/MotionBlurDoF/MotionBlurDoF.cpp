#include <KlayGE/KlayGE.hpp>
#include <KFL/CXX17/iterator.hpp>
#include <KFL/ErrorHandling.hpp>
#include <KFL/Util.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/Renderable.hpp>
#include <KlayGE/RenderableHelper.hpp>
#include <KlayGE/FrameBuffer.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/SceneManager.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/RenderSettings.hpp>
#include <KlayGE/Mesh.hpp>
#include <KlayGE/SceneObjectHelper.hpp>
#include <KlayGE/PostProcess.hpp>
#include <KlayGE/SATPostProcess.hpp>
#include <KlayGE/Script.hpp>
#include <KlayGE/Camera.hpp>
#include <KlayGE/MotionBlur.hpp>

#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/InputFactory.hpp>
#include <KlayGE/ScriptFactory.hpp>

#include <sstream>
#include <random>

#include "SampleCommon.hpp"
#include "MotionBlurDoF.hpp"

using namespace std;
using namespace KlayGE;

namespace
{
	int32_t const NUM_LINE = 10;
	int32_t const NUM_INSTANCE = 400;

	class MotionBlurRenderMesh : public StaticMesh
	{
	public:
		MotionBlurRenderMesh(RenderModelPtr const & model, std::wstring const & name)
			: StaticMesh(model, name),
				half_exposure_(1)
		{
		}

		virtual void VelocityPass(bool velocity) = 0;

		void BlurRadius(uint32_t blur_radius)
		{
			*(effect_->ParameterByName("blur_radius")) = static_cast<float>(blur_radius);
		}

		void Exposure(float exposure)
		{
			half_exposure_ = 0.5f * exposure;
		}

	protected:
		float half_exposure_;
	};

	class RenderInstanceMesh : public MotionBlurRenderMesh
	{
	public:
		RenderInstanceMesh(RenderModelPtr const & model, std::wstring const & /*name*/)
			: MotionBlurRenderMesh(model, L"InstancedMesh")
		{
			effect_ = SyncLoadRenderEffect("MotionBlurDoF.fxml");
			technique_ = effect_->TechniqueByName("ColorDepthInstanced");
		}

		virtual void DoBuildMeshInfo() override
		{
			AABBox const & bb = this->PosBound();
			*(effect_->ParameterByName("pos_center")) = bb.Center();
			*(effect_->ParameterByName("pos_extent")) = bb.HalfSize();
		}

		void OnRenderBegin()
		{
			App3DFramework const & app = Context::Instance().AppInstance();
			Camera const & camera = app.ActiveCamera();

			float4x4 const & curr_view = camera.ViewMatrix();
			float4x4 const & curr_proj = camera.ProjMatrix();
			float4x4 const & prev_view = camera.PrevViewMatrix();
			float4x4 const & prev_proj = camera.PrevProjMatrix();

			*(effect_->ParameterByName("eye_in_world")) = camera.EyePos();
			*(effect_->ParameterByName("view")) = curr_view;
			*(effect_->ParameterByName("proj")) = curr_proj;

			*(effect_->ParameterByName("prev_view")) = prev_view;
			*(effect_->ParameterByName("prev_proj")) = prev_proj;
			*(effect_->ParameterByName("half_exposure_x_framerate")) = half_exposure_ / app.FrameTime();
		}

		void VelocityPass(bool velocity) override
		{
			if (velocity)
			{
				technique_ = effect_->TechniqueByName("VelocityInstanced");
			}
			else
			{
				technique_ = effect_->TechniqueByName("ColorDepthInstanced");
			}
		}
	};

	class RenderNonInstancedMesh : public MotionBlurRenderMesh
	{
	private:
		struct InstData
		{
			float4 mat[3];
			float4 last_mat[3];
			uint32_t clr;
		};

	public:
		RenderNonInstancedMesh(RenderModelPtr const & model, std::wstring const & /*name*/)
			: MotionBlurRenderMesh(model, L"NonInstancedMesh")
		{
			effect_ = SyncLoadRenderEffect("MotionBlurDoF.fxml");
			technique_ = effect_->TechniqueByName("ColorDepthNonInstanced");
		}

		virtual void DoBuildMeshInfo() override
		{
			AABBox const & bb = this->PosBound();
			*(effect_->ParameterByName("pos_center")) = bb.Center();
			*(effect_->ParameterByName("pos_extent")) = bb.HalfSize();
		}

		void OnRenderBegin()
		{
			App3DFramework const & app = Context::Instance().AppInstance();
			Camera const & camera = app.ActiveCamera();

			float4x4 const & curr_view = camera.ViewMatrix();
			float4x4 const & curr_proj = camera.ProjMatrix();
			float4x4 const & prev_view = camera.PrevViewMatrix();
			float4x4 const & prev_proj = camera.PrevProjMatrix();

			*(effect_->ParameterByName("eye_in_world")) = camera.EyePos();
			*(effect_->ParameterByName("view")) = curr_view;
			*(effect_->ParameterByName("proj")) = curr_proj;
			*(effect_->ParameterByName("prev_view")) = prev_view;
			*(effect_->ParameterByName("prev_proj")) = prev_proj;
			*(effect_->ParameterByName("elapsed_time")) = app.FrameTime();

			*(effect_->ParameterByName("half_exposure_x_framerate")) = half_exposure_ / app.FrameTime();
		}

		void OnInstanceBegin(uint32_t id)
		{
			InstData const * data = static_cast<InstData const *>(instances_[id]->InstanceData());

			float4x4 model;
			model.Col(0, data->mat[0]);
			model.Col(1, data->mat[1]);
			model.Col(2, data->mat[2]);
			model.Col(3, float4(0, 0, 0, 1));

			float4x4 last_model;
			last_model.Col(0, data->last_mat[0]);
			last_model.Col(1, data->last_mat[1]);
			last_model.Col(2, data->last_mat[2]);
			last_model.Col(3, float4(0, 0, 0, 1));

			*(effect_->ParameterByName("modelmat")) = model;
			*(effect_->ParameterByName("last_modelmat")) = last_model;
			Color clr(data->clr);
			*(effect_->ParameterByName("color")) = float4(clr.b(), clr.g(), clr.r(), clr.a());	// swap b and r
		}

		void VelocityPass(bool velocity) override
		{
			if (velocity)
			{
				technique_ = effect_->TechniqueByName("VelocityNonInstanced");
			}
			else
			{
				technique_ = effect_->TechniqueByName("ColorDepthNonInstanced");
			}
		}

	private:
		void UpdateInstanceStream()
		{
		}
	};

	class Teapot : public SceneObjectHelper
	{
	private:
		struct InstData
		{
			float4 mat[3];
			float4 last_mat[3];
			uint32_t clr;
		};

	public:
		Teapot()
			: SceneObjectHelper(SOA_Moveable | SOA_Cullable)
		{
			instance_format_.push_back(VertexElement(VEU_TextureCoord, 1, EF_ABGR32F));
			instance_format_.push_back(VertexElement(VEU_TextureCoord, 2, EF_ABGR32F));
			instance_format_.push_back(VertexElement(VEU_TextureCoord, 3, EF_ABGR32F));
			instance_format_.push_back(VertexElement(VEU_TextureCoord, 4, EF_ABGR32F));
			instance_format_.push_back(VertexElement(VEU_TextureCoord, 5, EF_ABGR32F));
			instance_format_.push_back(VertexElement(VEU_TextureCoord, 6, EF_ABGR32F));
			instance_format_.push_back(VertexElement(VEU_Diffuse, 0, EF_ABGR8));
		}

		void Instance(float4x4 const & mat, Color const & clr)
		{
			model_ = mat;
			inst_.clr = clr.ABGR();
		}

		void const * InstanceData() const
		{
			return &inst_;
		}

		void SetRenderable(RenderablePtr const & ra)
		{
			renderable_ = ra;
		}

		void SubThreadUpdate(float app_time, float elapsed_time) override
		{
			KFL_UNUSED(app_time);

			last_model_ = model_;

			float4x4 mat_t = MathLib::transpose(last_model_);
			inst_.last_mat[0] = mat_t.Row(0);
			inst_.last_mat[1] = mat_t.Row(1);
			inst_.last_mat[2] = mat_t.Row(2);

			float e = elapsed_time * 0.3f * -model_(3, 1);
			model_ *= MathLib::rotation_y(e);

			mat_t = MathLib::transpose(model_);
			inst_.mat[0] = mat_t.Row(0);
			inst_.mat[1] = mat_t.Row(1);
			inst_.mat[2] = mat_t.Row(2);
		}

		void VelocityPass(bool velocity)
		{
			checked_pointer_cast<MotionBlurRenderMesh>(renderable_)->VelocityPass(velocity);
		}

		void BlurRadius(uint32_t blur_radius)
		{
			checked_pointer_cast<MotionBlurRenderMesh>(renderable_)->BlurRadius(blur_radius);
		}

		void Exposure(float exposure)
		{
			checked_pointer_cast<MotionBlurRenderMesh>(renderable_)->Exposure(exposure);
		}

	private:
		InstData inst_;
		float4x4 last_model_;
	};


	class DepthOfField : public PostProcess
	{
	public:
		DepthOfField()
			: PostProcess(L"DepthOfField", false,
				{},
				{ "color_tex", "depth_tex" },
				{ "output" },
				RenderEffectPtr(), nullptr),
				max_radius_(8), show_blur_factor_(false)
		{
			RenderDeviceCaps const & caps = Context::Instance().RenderFactoryInstance().RenderEngineInstance().DeviceCaps();
			cs_support_ = caps.cs_support && (caps.max_shader_model >= ShaderModel(5, 0));

			RenderEffectPtr effect = SyncLoadRenderEffect("DepthOfFieldPP.fxml");
			this->Technique(effect, effect->TechniqueByName("DepthOfFieldNormalization"));

			RenderFactory& rf = Context::Instance().RenderFactoryInstance();
			spread_fb_ = rf.MakeFrameBuffer();

			if (cs_support_)
			{
				spreading_pp_ = SyncLoadPostProcess("DepthOfField.ppml", "spreading_cs");
			}
			else
			{
				spreading_pp_ = SyncLoadPostProcess("DepthOfField.ppml", "spreading");
			}
			spreading_pp_->SetParam(1, static_cast<float>(max_radius_));

			if (cs_support_)
			{
				sat_pp_ = MakeSharedPtr<SATPostProcessCS>();
			}
			else
			{
				sat_pp_ = MakeSharedPtr<SATPostProcess>();
			}

			normalization_rl_ = rf.MakeRenderLayout();
			normalization_rl_->TopologyType(RenderLayout::TT_TriangleStrip);
		}

		void FocusPlane(float focus_plane)
		{
			focus_plane_ = focus_plane;
		}
		float FocusPlane() const
		{
			return focus_plane_;
		}

		void FocusRange(float focus_range)
		{
			focus_range_ = focus_range;
		}
		float FocusRange() const
		{
			return focus_range_;
		}

		void ShowBlurFactor(bool show)
		{
			show_blur_factor_ = show;
			if (show_blur_factor_)
			{
				technique_ = effect_->TechniqueByName("DepthOfFieldBlurFactor");
			}
			else
			{
				technique_ = effect_->TechniqueByName("DepthOfFieldNormalization");
			}
		}
		bool ShowBlurFactor() const
		{
			return show_blur_factor_;
		}

		void InputPin(uint32_t index, TexturePtr const & tex)
		{
			PostProcess::InputPin(index, tex);

			if (0 == index)
			{
				uint32_t const width = tex->Width(0) + max_radius_ * 4 + 1;
				uint32_t const height = tex->Height(0) + max_radius_ * 4 + 1;

				RenderDeviceCaps const & caps = Context::Instance().RenderFactoryInstance().RenderEngineInstance().DeviceCaps();
				ElementFormat fmt;
				if (caps.texture_format_support(EF_ABGR32F))
				{
					fmt = EF_ABGR32F;
				}
				else
				{
					BOOST_ASSERT(caps.texture_format_support(EF_ABGR16F));
					fmt = EF_ABGR16F;
				}

				RenderFactory& rf = Context::Instance().RenderFactoryInstance();
				spread_tex_ = rf.MakeTexture2D(width, height, 1, 1, fmt, 1, 0,
					EAH_GPU_Read | EAH_GPU_Write | (cs_support_ ? EAH_GPU_Unordered : 0));
				spread_fb_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*spread_tex_, 0, 1, 0));

				spreading_pp_->SetParam(0, float4(static_cast<float>(width),
					static_cast<float>(height), 1.0f / width, 1.0f / height));
				spreading_pp_->OutputPin(0, spread_tex_);

				{
					float4 pos[] =
					{
						float4(-1, +1, 0 + (max_radius_ * 2 + 0.0f) / width, 0 + (max_radius_ * 2 + 0.0f) / height),
						float4(+1, +1, 1 - (max_radius_ * 2 + 1.0f) / width, 0 + (max_radius_ * 2 + 0.0f) / height),
						float4(-1, -1, 0 + (max_radius_ * 2 + 0.0f) / width, 1 - (max_radius_ * 2 + 1.0f) / height),
						float4(+1, -1, 1 - (max_radius_ * 2 + 1.0f) / width, 1 - (max_radius_ * 2 + 1.0f) / height)
					};
				
					GraphicsBufferPtr pos_vb = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, sizeof(pos), &pos[0]);
					normalization_rl_->BindVertexStream(pos_vb, VertexElement(VEU_Position, 0, EF_ABGR32F));
				}

				sat_pp_->InputPin(0, spread_tex_);
				sat_pp_->OutputPin(0, spread_tex_);
			}
		}

		using PostProcess::InputPin;

		void Apply()
		{
			if (show_blur_factor_)
			{
				*(effect_->ParameterByName("focus_plane_inv_range")) = float2(-focus_plane_ / focus_range_, 1.0f / focus_range_);
				*(effect_->ParameterByName("depth_tex")) = this->InputPin(1);
				PostProcess::Apply();
			}
			else
			{
				RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
				spreading_pp_->SetParam(2, float2(-focus_plane_ / focus_range_, 1.0f / focus_range_));
				spreading_pp_->InputPin(0, this->InputPin(0));
				spreading_pp_->InputPin(1, this->InputPin(1));
				spreading_pp_->Apply();

				sat_pp_->Apply();

				*(effect_->ParameterByName("src_tex")) = spread_tex_;

				re.BindFrameBuffer(frame_buffer_);
				re.Render(*effect_, *technique_, *normalization_rl_);
			}
		}

	private:
		PostProcessPtr sat_pp_;

		bool cs_support_;

		int max_radius_;

		float focus_plane_;
		float focus_range_;
		bool show_blur_factor_;

		TexturePtr spread_tex_;
		FrameBufferPtr spread_fb_;

		PostProcessPtr spreading_pp_;

		RenderLayoutPtr normalization_rl_;
	};

	class BokehFilter : public PostProcess
	{
	public:
		BokehFilter()
			: PostProcess(L"BokehFilter", false,
				{},
				{ "color_tex", "depth_tex" },
				{ "output" },
				RenderEffectPtr(), nullptr),
				max_radius_(8)
		{
			RenderDeviceCaps const & caps = Context::Instance().RenderFactoryInstance().RenderEngineInstance().DeviceCaps();
			gs_support_ = caps.gs_support;

			RenderEffectPtr effect = SyncLoadRenderEffect("DepthOfFieldPP.fxml");
			if (gs_support_)
			{
				this->Technique(effect, effect->TechniqueByName("SeparateBokeh4"));
			}
			else
			{
				this->Technique(effect, effect->TechniqueByName("SeparateBokeh"));
			}

			*(effect->ParameterByName("max_radius")) = static_cast<float>(max_radius_);

			RenderFactory& rf = Context::Instance().RenderFactoryInstance();
			bokeh_fb_ = rf.MakeFrameBuffer();

			bokeh_rl_ = rf.MakeRenderLayout();

			merge_bokeh_pp_ = SyncLoadPostProcess("DepthOfField.ppml", "merge_bokeh");
			merge_bokeh_pp_->SetParam(2, static_cast<float>(max_radius_));
		}

		void FocusPlane(float focus_plane)
		{
			focus_plane_ = focus_plane;
		}
		float FocusPlane() const
		{
			return focus_plane_;
		}

		void FocusRange(float focus_range)
		{
			focus_range_ = focus_range;
		}
		float FocusRange() const
		{
			return focus_range_;
		}

		void InputPin(uint32_t index, TexturePtr const & tex)
		{
			PostProcess::InputPin(index, tex);

			if (0 == index)
			{
				uint32_t const in_width = tex->Width(0) / 2;
				uint32_t const in_height = tex->Height(0) / 2;
				uint32_t const out_width = in_width * 2 + max_radius_ * 4;
				uint32_t const out_height = in_height;

				*(effect_->ParameterByName("in_width_height")) = float4(static_cast<float>(in_width),
					static_cast<float>(in_height), 1.0f / in_width, 1.0f / in_height);
				*(effect_->ParameterByName("bokeh_width_height")) = float4(static_cast<float>(out_width),
					static_cast<float>(out_height), 1.0f / out_width, 1.0f / out_height);
				*(effect_->ParameterByName("background_offset")) = static_cast<float>(in_width + max_radius_ * 4);

				RenderFactory& rf = Context::Instance().RenderFactoryInstance();
				RenderDeviceCaps const & caps = rf.RenderEngineInstance().DeviceCaps();
				bokeh_tex_ = rf.MakeTexture2D(out_width, out_height, 1, 1, tex->Format(), 1, 0, EAH_GPU_Read | EAH_GPU_Write);
				bokeh_fb_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*bokeh_tex_, 0, 1, 0));

				if (gs_support_)
				{
					bokeh_rl_->TopologyType(RenderLayout::TT_PointList);

					std::vector<float2> points;
					for (uint32_t y = 0; y < in_height; y += 2)
					{
						for (uint32_t x = 0; x < in_width; x += 2)
						{
							points.push_back(float2((x + 0.5f) / in_width, (y + 0.5f) / in_height));
						}
					}

					GraphicsBufferPtr pos_vb = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable,
						static_cast<uint32_t>(points.size() * sizeof(points[0])), &points[0]);
					bokeh_rl_->BindVertexStream(pos_vb, VertexElement(VEU_Position, 0, EF_GR32F));
				}
				else
				{
					RenderEngine& re = rf.RenderEngineInstance();
					float const flipping = re.RequiresFlipping() ? -1.0f : +1.0f;

					std::vector<float4> points;
					for (uint32_t y = 0; y < in_height; y += 2)
					{
						for (uint32_t x = 0; x < in_width; x += 2)
						{
							points.push_back(float4((x + 0.5f) / in_width, (y + 0.5f) / in_height, -1, +1 * flipping));
							points.push_back(float4((x + 0.5f) / in_width, (y + 0.5f) / in_height, +1, +1 * flipping));
							points.push_back(float4((x + 0.5f) / in_width, (y + 0.5f) / in_height, -1, -1 * flipping));
							points.push_back(float4((x + 0.5f) / in_width, (y + 0.5f) / in_height, +1, -1 * flipping));
						}
					}

					std::vector<uint32_t> indices;
					uint32_t base = 0;
					if (caps.primitive_restart_support)
					{
						bokeh_rl_->TopologyType(RenderLayout::TT_TriangleStrip);

						for (uint32_t y = 0; y < in_height; y += 2)
						{
							for (uint32_t x = 0; x < in_width; x += 2)
							{
								indices.push_back(base + 0);
								indices.push_back(base + 1);
								indices.push_back(base + 2);
								indices.push_back(base + 3);
								indices.push_back(0xFFFFFFFF);

								base += 4;
							}
						}
					}
					else
					{
						bokeh_rl_->TopologyType(RenderLayout::TT_TriangleList);

						for (uint32_t y = 0; y < in_height; y += 2)
						{
							for (uint32_t x = 0; x < in_width; x += 2)
							{
								indices.push_back(base + 0);
								indices.push_back(base + 1);
								indices.push_back(base + 2);

								indices.push_back(base + 2);
								indices.push_back(base + 1);
								indices.push_back(base + 3);

								base += 4;
							}
						}
					}

					GraphicsBufferPtr pos_vb = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable,
						static_cast<uint32_t>(points.size() * sizeof(points[0])), &points[0]);
					bokeh_rl_->BindVertexStream(pos_vb, VertexElement(VEU_Position, 0, EF_ABGR32F));

					GraphicsBufferPtr pos_ib = rf.MakeIndexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable,
						static_cast<uint32_t>(indices.size() * sizeof(indices[0])), &indices[0]);
					bokeh_rl_->BindIndexStream(pos_ib, EF_R32UI);
				}

				merge_bokeh_pp_->SetParam(0, float4(static_cast<float>(in_width),
					static_cast<float>(in_height), 1.0f / in_width, 1.0f / in_height));
				merge_bokeh_pp_->SetParam(1, float4(static_cast<float>(out_width),
					static_cast<float>(out_height), 1.0f / out_width, 1.0f / out_height));
				merge_bokeh_pp_->SetParam(5, static_cast<float>(in_width + max_radius_ * 4));
				merge_bokeh_pp_->InputPin(0, bokeh_tex_);
			}
			else
			{
				merge_bokeh_pp_->InputPin(index, tex);
			}
		}

		using PostProcess::InputPin;

		void OutputPin(uint32_t index, TexturePtr const & tex, int level, int array_index, int face)
		{
			merge_bokeh_pp_->OutputPin(index, tex, level, array_index, face);
		}

		using PostProcess::OutputPin;

		void Apply()
		{
			RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
			*(effect_->ParameterByName("focus_plane_inv_range")) = float2(-focus_plane_ / focus_range_, 1.0f / focus_range_);
			*(effect_->ParameterByName("focus_plane")) = focus_plane_;
			*(effect_->ParameterByName("color_tex")) = this->InputPin(0);
			*(effect_->ParameterByName("depth_tex")) = this->InputPin(1);

			re.BindFrameBuffer(bokeh_fb_);
			bokeh_fb_->Clear(FrameBuffer::CBM_Color, Color(0, 0, 0, 0), 1, 0);
			re.Render(*effect_, *technique_, *bokeh_rl_);

			merge_bokeh_pp_->SetParam(3, float2(-focus_plane_ / focus_range_, 1.0f / focus_range_));
			merge_bokeh_pp_->SetParam(4, focus_plane_);
			merge_bokeh_pp_->Apply();
		}

	private:
		bool gs_support_;

		int max_radius_;

		float focus_plane_;
		float focus_range_;

		TexturePtr bokeh_tex_;
		FrameBufferPtr bokeh_fb_;
		RenderLayoutPtr bokeh_rl_;

		PostProcessPtr merge_bokeh_pp_;
	};


	enum
	{
		CtrlCamera,
		Exit,
	};

	InputActionDefine actions[] =
	{
		InputActionDefine(CtrlCamera, KS_LeftCtrl),
		InputActionDefine(CtrlCamera, KS_RightCtrl),
		InputActionDefine(Exit, KS_Escape),
	};
}

int SampleMain()
{
	MotionBlurDoFApp app;
	app.Create();
	app.Run();

	return 0;
}

MotionBlurDoFApp::MotionBlurDoFApp()
					: App3DFramework("MotionBlurDoF"),
						use_instance_(true), exposure_(2), blur_radius_(2),
						dof_on_(true), bokeh_on_(true), mb_on_(true),
						num_objs_rendered_(0), num_renderables_rendered_(0),
						num_primitives_rendered_(0), num_vertices_rendered_(0)
{
	ResLoader::Instance().AddPath("../../Samples/media/MotionBlurDoF");
}

void MotionBlurDoFApp::OnCreate()
{
	loading_percentage_ = 0;
	model_instance_ = ASyncLoadModel("teapot.meshml", EAH_GPU_Read | EAH_Immutable,
		CreateModelFactory<RenderModel>(), CreateMeshFactory<RenderInstanceMesh>());
	model_mesh_ = ASyncLoadModel("teapot.meshml", EAH_GPU_Read | EAH_Immutable,
		CreateModelFactory<RenderModel>(), CreateMeshFactory<RenderNonInstancedMesh>());

	RenderFactory& rf = Context::Instance().RenderFactoryInstance();

	font_ = SyncLoadFont("gkai00mp.kfont");

	ScriptEngine& scriptEngine = Context::Instance().ScriptFactoryInstance().ScriptEngineInstance();
	script_module_ = scriptEngine.CreateModule("MotionBlurDoF_init");

	this->LookAt(float3(-1.8f, 1.9f, -1.8f), float3(0, 0, 0));
	this->Proj(0.1f, 100);

	RenderEngine& re = rf.RenderEngineInstance();
	RenderDeviceCaps const & caps = re.DeviceCaps();
	clr_depth_fb_ = rf.MakeFrameBuffer();
	velocity_fb_ = rf.MakeFrameBuffer();
	clr_depth_fb_->GetViewport()->camera = re.CurFrameBuffer()->GetViewport()->camera;
	velocity_fb_->GetViewport()->camera = re.CurFrameBuffer()->GetViewport()->camera;

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

	if (caps.fp_color_support && caps.texture_format_support(EF_ABGR32F)
		&& caps.rendertarget_format_support(EF_ABGR32F, 1, 0))
	{
		depth_of_field_ = MakeSharedPtr<DepthOfField>();
		bokeh_filter_ = MakeSharedPtr<BokehFilter>();
	}
	depth_of_field_copy_pp_ = SyncLoadPostProcess("Copy.ppml", "copy");
	
	motion_blur_ = MakeSharedPtr<MotionBlurPostProcess>();
	motion_blur_copy_pp_ = SyncLoadPostProcess("Copy.ppml", "copy");

	UIManager::Instance().Load(ResLoader::Instance().Open("MotionBlurDoF.uiml"));
	dof_dialog_ = UIManager::Instance().GetDialogs()[0];
	mb_dialog_ = UIManager::Instance().GetDialogs()[1];
	app_dialog_ = UIManager::Instance().GetDialogs()[2];

	id_dof_on_ = dof_dialog_->IDFromName("DoFOn");
	id_bokeh_on_ = dof_dialog_->IDFromName("BokehOn");
	id_dof_focus_plane_static_ = dof_dialog_->IDFromName("FocusPlaneStatic");
	id_dof_focus_plane_slider_ = dof_dialog_->IDFromName("FocusPlaneSlider");
	id_dof_focus_range_static_ = dof_dialog_->IDFromName("FocusRangeStatic");
	id_dof_focus_range_slider_ = dof_dialog_->IDFromName("FocusRangeSlider");
	id_dof_blur_factor_ = dof_dialog_->IDFromName("BlurFactor");
	id_mb_on_ = mb_dialog_->IDFromName("MBOn");
	id_mb_exposure_static_ = mb_dialog_->IDFromName("ExposureStatic");
	id_mb_exposure_slider_ = mb_dialog_->IDFromName("ExposureSlider");
	id_mb_blur_radius_static_ = mb_dialog_->IDFromName("BlurRadiusStatic");
	id_mb_blur_radius_slider_ = mb_dialog_->IDFromName("BlurRadiusSlider");
	id_mb_reconstruction_samples_static_ = mb_dialog_->IDFromName("SamplesStatic");
	id_mb_reconstruction_samples_slider_ = mb_dialog_->IDFromName("SamplesSlider");
	id_motion_blur_type_ = mb_dialog_->IDFromName("MotionBlurTypeCombo");
	id_use_instancing_ = app_dialog_->IDFromName("UseInstancing");
	id_ctrl_camera_ = app_dialog_->IDFromName("CtrlCamera");

	if (depth_of_field_)
	{
		dof_dialog_->Control<UICheckBox>(id_dof_on_)->OnChangedEvent().connect(
			[this](UICheckBox const & sender)
			{
				this->DoFOnHandler(sender);
			});
		this->DoFOnHandler(*dof_dialog_->Control<UICheckBox>(id_dof_on_));
		dof_dialog_->Control<UICheckBox>(id_bokeh_on_)->OnChangedEvent().connect(
			[this](UICheckBox const & sender)
			{
				this->BokehOnHandler(sender);
			});
		this->BokehOnHandler(*dof_dialog_->Control<UICheckBox>(id_bokeh_on_));
		dof_dialog_->Control<UISlider>(id_dof_focus_plane_slider_)->OnValueChangedEvent().connect(
			[this](UISlider const & sender)
			{
				this->DoFFocusPlaneChangedHandler(sender);
			});
		this->DoFFocusPlaneChangedHandler(*dof_dialog_->Control<UISlider>(id_dof_focus_plane_slider_));
		dof_dialog_->Control<UISlider>(id_dof_focus_range_slider_)->OnValueChangedEvent().connect(
			[this](UISlider const & sender)
			{
				this->DoFFocusRangeChangedHandler(sender);
			});
		this->DoFFocusRangeChangedHandler(*dof_dialog_->Control<UISlider>(id_dof_focus_range_slider_));
		dof_dialog_->Control<UICheckBox>(id_dof_blur_factor_)->OnChangedEvent().connect(
			[this](UICheckBox const & sender)
			{
				this->DoFBlurFactorHandler(sender);
			});
		this->DoFBlurFactorHandler(*dof_dialog_->Control<UICheckBox>(id_dof_blur_factor_));
	}
	else
	{
		dof_dialog_->Control<UICheckBox>(id_dof_on_)->SetEnabled(false);
		dof_dialog_->Control<UICheckBox>(id_bokeh_on_)->SetEnabled(false);
		dof_dialog_->Control<UISlider>(id_dof_focus_plane_slider_)->SetEnabled(false);
		dof_dialog_->Control<UISlider>(id_dof_focus_range_slider_)->SetEnabled(false);
		dof_dialog_->Control<UICheckBox>(id_dof_blur_factor_)->SetEnabled(false);
		dof_on_ = false;
	}

	mb_dialog_->Control<UICheckBox>(id_mb_on_)->OnChangedEvent().connect(
		[this](UICheckBox const & sender)
		{
			this->MBOnHandler(sender);
		});
	mb_dialog_->Control<UISlider>(id_mb_exposure_slider_)->OnValueChangedEvent().connect(
		[this](UISlider const & sender)
		{
			this->MBExposureChangedHandler(sender);
		});
	this->MBExposureChangedHandler(*mb_dialog_->Control<UISlider>(id_mb_exposure_slider_));
	mb_dialog_->Control<UISlider>(id_mb_blur_radius_slider_)->OnValueChangedEvent().connect(
		[this](UISlider const & sender)
		{
			this->MBBlurRadiusChangedHandler(sender);
		});
	this->MBBlurRadiusChangedHandler(*mb_dialog_->Control<UISlider>(id_mb_blur_radius_slider_));
	mb_dialog_->Control<UISlider>(id_mb_reconstruction_samples_slider_)->OnValueChangedEvent().connect(
		[this](UISlider const & sender)
		{
			this->MBReconstructionSamplesChangedHandler(sender);
		});
	this->MBReconstructionSamplesChangedHandler(*mb_dialog_->Control<UISlider>(id_mb_reconstruction_samples_slider_));
	mb_dialog_->Control<UIComboBox>(id_motion_blur_type_)->OnSelectionChangedEvent().connect(
		[this](UIComboBox const & sender)
		{
			this->MotionBlurChangedHandler(sender);
		});

	app_dialog_->Control<UICheckBox>(id_ctrl_camera_)->OnChangedEvent().connect(
		[this](UICheckBox const & sender)
		{
			this->CtrlCameraHandler(sender);
		});

	app_dialog_->Control<UICheckBox>(id_use_instancing_)->OnChangedEvent().connect(
		[this](UICheckBox const & sender)
		{
			this->UseInstancingHandler(sender);
		});
}

void MotionBlurDoFApp::OnResize(uint32_t width, uint32_t height)
{
	App3DFramework::OnResize(width, height);

	RenderFactory& rf = Context::Instance().RenderFactoryInstance();
	RenderEngine& re = rf.RenderEngineInstance();
	RenderDeviceCaps const & caps = re.DeviceCaps();

	depth_texture_support_ = caps.depth_texture_support;

	RenderViewPtr ds_view;
	if (depth_texture_support_)
	{
		ds_tex_ = rf.MakeTexture2D(width, height, 1, 1, EF_D16, 1, 0, EAH_GPU_Read | EAH_GPU_Write);
		ds_view = rf.Make2DDepthStencilRenderView(*ds_tex_, 0, 1, 0);
	}
	else
	{
		ds_view = rf.Make2DDepthStencilRenderView(width, height, EF_D16, 1, 0);
	}

	ElementFormat depth_fmt;
	if (caps.pack_to_rgba_required)
	{
		if (caps.rendertarget_format_support(EF_ABGR8, 1, 0))
		{
			depth_fmt = EF_ABGR8;
		}
		else
		{
			BOOST_ASSERT(caps.rendertarget_format_support(EF_ARGB8, 1, 0));
			depth_fmt = EF_ARGB8;
		}
	}
	else
	{
		if (caps.rendertarget_format_support(EF_R16F, 1, 0))
		{
			depth_fmt = EF_R16F;
		}
		else
		{
			BOOST_ASSERT(caps.rendertarget_format_support(EF_R32F, 1, 0));
			depth_fmt = EF_R32F;
		}
	}
	depth_tex_ = rf.MakeTexture2D(width, height, 2, 1, depth_fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write | EAH_Generate_Mips);

	if (depth_texture_support_)
	{
		depth_to_linear_pp_ = SyncLoadPostProcess("Depth.ppml", "DepthToLinear");
		depth_to_linear_pp_->InputPin(0, ds_tex_);
		depth_to_linear_pp_->OutputPin(0, depth_tex_);
	}

	ElementFormat color_fmt;
	if (caps.fp_color_support)
	{
		if (caps.rendertarget_format_support(EF_B10G11R11F, 1, 0))
		{
			color_fmt = EF_B10G11R11F;
		}
		else
		{
			BOOST_ASSERT(caps.rendertarget_format_support(EF_ABGR16F, 1, 0));
			color_fmt = EF_ABGR16F;
		}
	}
	else
	{
		if (caps.rendertarget_format_support(EF_ABGR8, 1, 0))
		{
			color_fmt = EF_ABGR8;
		}
		else
		{
			BOOST_ASSERT(caps.rendertarget_format_support(EF_ARGB8, 1, 0));
			color_fmt = EF_ARGB8;
		}
	}

	color_tex_ = rf.MakeTexture2D(width, height, 2, 1, color_fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write | EAH_Generate_Mips);
	clr_depth_fb_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*color_tex_, 0, 1, 0));
	clr_depth_fb_->Attach(FrameBuffer::ATT_DepthStencil, ds_view);

	ElementFormat motion_fmt;
	if (caps.rendertarget_format_support(EF_GR8, 1, 0))
	{
		motion_fmt = EF_GR8;
	}
	else
	{
		if (caps.rendertarget_format_support(EF_ABGR8, 1, 0))
		{
			motion_fmt = EF_ABGR8;
		}
		else
		{
			BOOST_ASSERT(caps.rendertarget_format_support(EF_ARGB8, 1, 0));

			motion_fmt = EF_ARGB8;
		}
	}

	velocity_tex_ = rf.MakeTexture2D(width, height, 1, 1, motion_fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write);
	velocity_fb_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*velocity_tex_, 0, 1, 0));
	velocity_fb_->Attach(FrameBuffer::ATT_DepthStencil, ds_view);

	dof_tex_ = rf.MakeTexture2D(width, height, 1, 1, color_fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write);

	if (depth_of_field_)
	{
		depth_of_field_->InputPin(0, color_tex_);
		depth_of_field_->InputPin(1, depth_tex_);
		depth_of_field_->OutputPin(0, dof_tex_);
	}
	depth_of_field_copy_pp_->InputPin(0, color_tex_);
	depth_of_field_copy_pp_->OutputPin(0, dof_tex_);

	if (bokeh_filter_)
	{
		bokeh_filter_->InputPin(0, color_tex_);
		bokeh_filter_->InputPin(1, depth_tex_);
		bokeh_filter_->OutputPin(0, dof_tex_);
	}

	motion_blur_->InputPin(0, dof_tex_);
	motion_blur_->InputPin(1, depth_tex_);
	motion_blur_->InputPin(2, velocity_tex_);
	motion_blur_copy_pp_->InputPin(0, dof_tex_);

	UIManager::Instance().SettleCtrls();
}

void MotionBlurDoFApp::InputHandler(InputEngine const & /*sender*/, InputAction const & action)
{
	switch (action.first)
	{
	case Exit:
		this->Quit();
		break;
	}
}

void MotionBlurDoFApp::DoFOnHandler(KlayGE::UICheckBox const & sender)
{
	dof_on_ = sender.GetChecked();

	dof_dialog_->Control<UICheckBox>(id_bokeh_on_)->SetEnabled(dof_on_);
	dof_dialog_->Control<UIStatic>(id_dof_focus_plane_static_)->SetEnabled(dof_on_);
	dof_dialog_->Control<UISlider>(id_dof_focus_plane_slider_)->SetEnabled(dof_on_);
	dof_dialog_->Control<UIStatic>(id_dof_focus_range_static_)->SetEnabled(dof_on_);
	dof_dialog_->Control<UISlider>(id_dof_focus_range_slider_)->SetEnabled(dof_on_);
	dof_dialog_->Control<UICheckBox>(id_dof_blur_factor_)->SetEnabled(dof_on_);
}

void MotionBlurDoFApp::BokehOnHandler(KlayGE::UICheckBox const & sender)
{
	bokeh_on_ = sender.GetChecked();
}

void MotionBlurDoFApp::DoFFocusPlaneChangedHandler(KlayGE::UISlider const & sender)
{
	float focus_plane = sender.GetValue() / 50.0f;
	checked_pointer_cast<DepthOfField>(depth_of_field_)->FocusPlane(focus_plane);
	if (bokeh_filter_)
	{
		checked_pointer_cast<BokehFilter>(bokeh_filter_)->FocusPlane(focus_plane);
	}

	std::wostringstream stream;
	stream << "Focus Plane: " << focus_plane;
	dof_dialog_->Control<UIStatic>(id_dof_focus_plane_static_)->SetText(stream.str());
}

void MotionBlurDoFApp::DoFFocusRangeChangedHandler(KlayGE::UISlider const & sender)
{
	float focus_range = sender.GetValue() / 50.0f;
	checked_pointer_cast<DepthOfField>(depth_of_field_)->FocusRange(focus_range);
	if (bokeh_filter_)
	{
		checked_pointer_cast<BokehFilter>(bokeh_filter_)->FocusRange(focus_range);
	}

	std::wostringstream stream;
	stream << "Focus Range: " << focus_range;
	dof_dialog_->Control<UIStatic>(id_dof_focus_range_static_)->SetText(stream.str());
}

void MotionBlurDoFApp::DoFBlurFactorHandler(KlayGE::UICheckBox const & sender)
{
	checked_pointer_cast<DepthOfField>(depth_of_field_)->ShowBlurFactor(sender.GetChecked());
}

void MotionBlurDoFApp::MBOnHandler(KlayGE::UICheckBox const & sender)
{
	mb_on_ = sender.GetChecked();

	mb_dialog_->Control<UIComboBox>(id_motion_blur_type_)->SetEnabled(mb_on_);
}

void MotionBlurDoFApp::MBExposureChangedHandler(KlayGE::UISlider const & sender)
{
	exposure_ = sender.GetValue() / 10.0f;
	motion_blur_->SetParam(0, exposure_);

	std::wostringstream stream;
	stream << "Exposure: " << exposure_;
	mb_dialog_->Control<UIStatic>(id_mb_exposure_static_)->SetText(stream.str());

	for (size_t i = 0; i < scene_objs_.size(); ++ i)
	{
		checked_pointer_cast<Teapot>(scene_objs_[i])->Exposure(exposure_);
	}
}

void MotionBlurDoFApp::MBBlurRadiusChangedHandler(KlayGE::UISlider const & sender)
{
	blur_radius_ = sender.GetValue();
	motion_blur_->SetParam(1, blur_radius_);

	std::wostringstream stream;
	stream << "Blur Radius: " << blur_radius_;
	mb_dialog_->Control<UIStatic>(id_mb_blur_radius_static_)->SetText(stream.str());

	for (size_t i = 0; i < scene_objs_.size(); ++ i)
	{
		checked_pointer_cast<Teapot>(scene_objs_[i])->BlurRadius(blur_radius_);
	}
}

void MotionBlurDoFApp::MBReconstructionSamplesChangedHandler(KlayGE::UISlider const & sender)
{
	uint32_t reconstruction_samples = sender.GetValue();
	motion_blur_->SetParam(2, reconstruction_samples);

	std::wostringstream stream;
	stream << "Samples: " << reconstruction_samples;
	mb_dialog_->Control<UIStatic>(id_mb_reconstruction_samples_static_)->SetText(stream.str());
}

void MotionBlurDoFApp::MotionBlurChangedHandler(KlayGE::UIComboBox const & sender)
{
	motion_blur_->SetParam(3, static_cast<uint32_t>(sender.GetSelectedIndex()));
}

void MotionBlurDoFApp::UseInstancingHandler(UICheckBox const & /*sender*/)
{
	use_instance_ = app_dialog_->Control<UICheckBox>(id_use_instancing_)->GetChecked();

	if (use_instance_)
	{
		for (size_t i = 0; i < scene_objs_.size(); ++ i)
		{
			checked_pointer_cast<Teapot>(scene_objs_[i])->SetRenderable(renderInstance_);
		}
	}
	else
	{
		for (size_t i = 0; i < scene_objs_.size(); ++ i)
		{
			checked_pointer_cast<Teapot>(scene_objs_[i])->SetRenderable(renderMesh_);
		}
	}
}

void MotionBlurDoFApp::CtrlCameraHandler(KlayGE::UICheckBox const & sender)
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

void MotionBlurDoFApp::DoUpdateOverlay()
{
	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

	UIManager::Instance().Render();

	font_->RenderText(0, 0, Color(1, 1, 0, 1), L"Motion Blur and Depth of field", 16);
	font_->RenderText(0, 18, Color(1, 1, 0, 1), renderEngine.ScreenFrameBuffer()->Description(), 16);

	std::wostringstream stream;
	stream.precision(2);
	stream << std::fixed << this->FPS() << " FPS";
	font_->RenderText(0, 36, Color(1, 1, 0, 1), stream.str(), 16);

	stream.str(L"");
	stream << num_objs_rendered_ << " Scene objects "
		<< num_renderables_rendered_ << " Renderables "
		<< num_primitives_rendered_ << " Primitives "
		<< num_vertices_rendered_ << " Vertices";
	font_->RenderText(0, 54, Color(1, 1, 1, 1), stream.str(), 16);

	if (use_instance_)
	{
		font_->RenderText(0, 72, Color(1, 1, 1, 1), L"Instancing is enabled", 16);
	}
	else
	{
		font_->RenderText(0, 72, Color(1, 1, 1, 1), L"Instancing is disabled", 16);
	}
}

uint32_t MotionBlurDoFApp::DoUpdate(uint32_t pass)
{
	if (0 == pass)
	{
		if (loading_percentage_ < 100)
		{
			if (loading_percentage_ < 80 - NUM_LINE)
			{
				if (model_instance_->HWResourceReady())
				{
					renderInstance_ = model_instance_->Subrenderable(0);
					loading_percentage_ = 80 - NUM_LINE;
				}
			}
			else if (loading_percentage_ < 80)
			{
				int32_t i = loading_percentage_ - (80 - NUM_LINE);
				for (int32_t j = 0; j < NUM_INSTANCE / NUM_LINE; ++ j)
				{
					float3 pos(0, 0, 0);
					Color clr(0, 0, 0, 1);
					try
					{
						std::vector<std::any> scr_pos = std::any_cast<std::vector<std::any>>(script_module_->Call("get_pos",
							{ i, j, NUM_INSTANCE, NUM_LINE }));

						pos.x() = std::any_cast<float>(scr_pos[0]);
						pos.y() = std::any_cast<float>(scr_pos[1]);
						pos.z() = std::any_cast<float>(scr_pos[2]);

						std::vector<std::any> scr_clr = std::any_cast<std::vector<std::any>>(script_module_->Call("get_clr",
							{i, j, NUM_INSTANCE, NUM_LINE }));

						clr.r() = std::any_cast<float>(scr_clr[0]);
						clr.g() = std::any_cast<float>(scr_clr[1]);
						clr.b() = std::any_cast<float>(scr_clr[2]);
						clr.a() = std::any_cast<float>(scr_clr[3]);
					}
					catch (...)
					{
						LogWarn("Wrong callings to script engine");
					}

					SceneObjectPtr so = MakeSharedPtr<Teapot>();
					checked_pointer_cast<Teapot>(so)->Instance(MathLib::translation(pos), clr);

					checked_pointer_cast<Teapot>(so)->SetRenderable(renderInstance_);
					checked_pointer_cast<Teapot>(so)->Exposure(exposure_);
					checked_pointer_cast<Teapot>(so)->BlurRadius(blur_radius_);
					so->AddToSceneManager();
					scene_objs_.push_back(so);

					so->SubThreadUpdate(0, 0);
					so->MainThreadUpdate(0, 0);
				}

				++ loading_percentage_;
			}
			else
			{
				if (model_mesh_)
				{
					renderMesh_ = model_mesh_->Subrenderable(0);
					loading_percentage_ = 100;
				}
			}
		}
	}

	Context& context = Context::Instance();
	App3DFramework& app = context.AppInstance();
	SceneManager& sceneMgr = context.SceneManagerInstance();
	RenderEngine& renderEngine = context.RenderFactoryInstance().RenderEngineInstance();

	switch (pass)
	{
	case 0:
		{
			Camera& camera = this->ActiveCamera();

			camera.Update(app.AppTime(), app.FrameTime());

			if (depth_texture_support_)
			{
				float q = camera.FarPlane() / (camera.FarPlane() - camera.NearPlane());
				float4 near_q_far(camera.NearPlane() * q, q, camera.FarPlane(), 1 / camera.FarPlane());
				depth_to_linear_pp_->SetParam(0, near_q_far);
			}
		}

		renderEngine.BindFrameBuffer(clr_depth_fb_);
		{
			Color clear_clr(0.2f, 0.4f, 0.6f, 1);
			if (Context::Instance().Config().graphics_cfg.gamma)
			{
				clear_clr.r() = 0.029f;
				clear_clr.g() = 0.133f;
				clear_clr.b() = 0.325f;
			}
			renderEngine.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, clear_clr, 1.0f, 0);
		}
		for (size_t i = 0; i < scene_objs_.size(); ++ i)
		{
			checked_pointer_cast<Teapot>(scene_objs_[i])->VelocityPass(false);
		}
		return App3DFramework::URV_NeedFlush;

	case 1:
		if (depth_texture_support_)
		{
			depth_to_linear_pp_->Apply();
		}

		renderEngine.BindFrameBuffer(velocity_fb_);
		renderEngine.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, Color(0.5f, 0.5f, 0.5f, 1), 1.0f, 0);
		for (size_t i = 0; i < scene_objs_.size(); ++ i)
		{
			checked_pointer_cast<Teapot>(scene_objs_[i])->VelocityPass(true);
		}
		return App3DFramework::URV_NeedFlush;

	default:
		num_objs_rendered_ = sceneMgr.NumObjectsRendered();
		num_renderables_rendered_ = sceneMgr.NumRenderablesRendered();
		num_primitives_rendered_ = sceneMgr.NumPrimitivesRendered();
		num_vertices_rendered_ = sceneMgr.NumVerticesRendered();

		color_tex_->BuildMipSubLevels();
		depth_tex_->BuildMipSubLevels();

		if (dof_on_)
		{
			depth_of_field_->Apply();
			if (bokeh_on_ && bokeh_filter_)
			{
				bokeh_filter_->Apply();
			}
		}
		else
		{
			depth_of_field_copy_pp_->Apply();
		}
		if (mb_on_)
		{
			motion_blur_->Apply();
		}
		else
		{
			motion_blur_copy_pp_->Apply();
		}

		renderEngine.BindFrameBuffer(FrameBufferPtr());
		renderEngine.CurFrameBuffer()->Attached(FrameBuffer::ATT_DepthStencil)->ClearDepth(1.0f);
		return App3DFramework::URV_Finished;
	}
}
