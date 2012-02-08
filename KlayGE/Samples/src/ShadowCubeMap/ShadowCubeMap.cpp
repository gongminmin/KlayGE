#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/GraphicsBuffer.hpp>
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
#include <KlayGE/Mesh.hpp>
#include <KlayGE/Mesh.hpp>
#include <KlayGE/Texture.hpp>
#include <KlayGE/SceneObjectHelper.hpp>
#include <KlayGE/PostProcess.hpp>
#include <KlayGE/Light.hpp>
#include <KlayGE/Camera.hpp>

#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/InputFactory.hpp>

#include <vector>
#include <sstream>
#include <boost/bind.hpp>

#include "ShadowCubeMap.hpp"

using namespace std;
using namespace KlayGE;

namespace
{	
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
		tess_pattern_vbs[0] = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, &init_data);

		std::vector<uint16_t> index;
		index.push_back(0);
		index.push_back(1);
		index.push_back(2);
		init_data.row_pitch = static_cast<uint32_t>(index.size() * sizeof(index[0]));
		init_data.slice_pitch = 0;
		init_data.data = &index[0];
		tess_pattern_ibs[0] = rf.MakeIndexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, &init_data);

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
			tess_pattern_vbs[i] = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, &init_data);

			init_data.row_pitch = static_cast<uint32_t>(index.size() * sizeof(index[0]));
			init_data.slice_pitch = 0;
			init_data.data = &index[0];
			tess_pattern_ibs[i] = rf.MakeIndexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, &init_data);
		}
	}


	uint32_t const SHADOW_MAP_SIZE = 512;

	class ShadowMapped
	{
	public:
		explicit ShadowMapped(uint32_t shadow_map_size)
			: shadow_map_size_(shadow_map_size), pass_index_(0)
		{
		}

		float4x4 LightViewProj() const
		{
			return light_views_[pass_index_] * light_proj_;
		}

		virtual void GenShadowMapPass(bool gen_sm, SM_TYPE sm_type, int pass_index)
		{
			gen_sm_pass_ = gen_sm;
			sm_type_ = sm_type;
			pass_index_ = pass_index;
		}

		void LightSrc(LightSourcePtr const & light_src)
		{
			light_pos_ = light_src->Position();

			float4x4 light_model = MathLib::to_matrix(light_src->Rotation()) * MathLib::translation(light_src->Position());
			inv_light_model_ = MathLib::inverse(light_model);

			App3DFramework const & app = Context::Instance().AppInstance();
			if ((SMT_CubeOne == sm_type_) || (SMT_CubeOneInstance == sm_type_) || (SMT_CubeOneInstanceGS == sm_type_))
			{
				for (int i = 0; i < 6; ++ i)
				{
					light_views_[i] = light_src->SMCamera(i)->ViewMatrix();
				}
			}
			else
			{
				light_views_[pass_index_] = app.ActiveCamera().ViewMatrix();
			}
			light_proj_ = app.ActiveCamera().ProjMatrix();

			light_falloff_ = light_src->Falloff();
		}

		void CubeSMTexture(TexturePtr const & cube_tex)
		{
			sm_cube_tex_ = cube_tex;
		}

		void DPSMTexture(TexturePtr const & dual_tex)
		{
			sm_dual_tex_ = dual_tex;
		}

		void LampTexture(TexturePtr const & tex)
		{
			lamp_tex_ = tex;
		}

	protected:
		void OnRenderBegin(float4x4 const & model, RenderEffectPtr const & effect)
		{
			App3DFramework const & app = Context::Instance().AppInstance();

			*(effect->ParameterByName("model")) = model;
			*(effect->ParameterByName("obj_model_to_light_model")) = model * inv_light_model_;

			if (gen_sm_pass_)
			{
				switch (sm_type_)
				{
				case SMT_DP:
					{
						*(effect->ParameterByName("mvp")) = model * this->LightViewProj();

						float4x4 mv = model * light_views_[pass_index_];
						*(effect->ParameterByName("mv")) = mv;
						*(effect->ParameterByName("far")) = app.ActiveCamera().FarPlane();

						FrameBufferPtr const & cur_fb = Context::Instance().RenderFactoryInstance().RenderEngineInstance().CurFrameBuffer();
						*(effect->ParameterByName("tess_edge_length_scale")) = float2(static_cast<float>(cur_fb->Width()), static_cast<float>(cur_fb->Height())) / 12.0f;
					}
					break;

				case SMT_Cube:
					*(effect->ParameterByName("mvp")) = model * this->LightViewProj();
					break;

				default:
					{
						std::vector<float4x4> mvps(6);
						for (int i = 0; i < 6; ++ i)
						{
							mvps[i] = model * light_views_[i] * light_proj_;
						}
						*(effect->ParameterByName("mvps")) = mvps;
					}
					break;
				}
			}
			else
			{
				float4x4 const & view = app.ActiveCamera().ViewMatrix();
				float4x4 const & proj = app.ActiveCamera().ProjMatrix();

				*(effect->ParameterByName("mvp")) = model * view * proj;
				*(effect->ParameterByName("light_pos")) = light_pos_;

				*(effect->ParameterByName("light_projective_tex")) = lamp_tex_;
				*(effect->ParameterByName("shadow_cube_tex")) = sm_cube_tex_;
				*(effect->ParameterByName("shadow_dual_tex")) = sm_dual_tex_;

				*(effect->ParameterByName("light_falloff")) = light_falloff_;

				*(effect->ParameterByName("dpsm")) = static_cast<int32_t>((SMT_DP == sm_type_) ? 1 : 0);
				if (SMT_DP == sm_type_)
				{
					*(effect->ParameterByName("dpsm")) = static_cast<int32_t>(1);
					*(effect->ParameterByName("obj_model_to_light_view")) = model * light_views_[0];
				}
				else
				{
					*(effect->ParameterByName("dpsm")) = static_cast<int32_t>(0);
				}
			}
		}

	protected:
		uint32_t shadow_map_size_;

		bool gen_sm_pass_;
		SM_TYPE sm_type_;
		int pass_index_;
		TexturePtr sm_cube_tex_;
		TexturePtr sm_dual_tex_;

		float3 light_pos_;
		float4x4 inv_light_model_;
		float4x4 light_views_[6];
		float4x4 light_proj_;
		float3 light_falloff_;

		TexturePtr lamp_tex_;
	};

	class OccluderMesh : public StaticMesh, public ShadowMapped
	{
	public:
		OccluderMesh(RenderModelPtr const & model, std::wstring const & name)
			: StaticMesh(model, name),
				ShadowMapped(SHADOW_MAP_SIZE),
				smooth_mesh_(false), tess_factor_(5)
		{
			effect_ = Context::Instance().RenderFactoryInstance().LoadEffect("ShadowCubeMap.fxml");

			RenderFactory& rf = Context::Instance().RenderFactoryInstance();
			RenderDeviceCaps const & caps = rf.RenderEngineInstance().DeviceCaps();
			if (TM_Instanced == caps.tess_method)
			{
				tess_pattern_rl_ = rf.MakeRenderLayout();
				tess_pattern_rl_->TopologyType(RenderLayout::TT_TriangleList);
			}

			mesh_rl_ = rl_;
		}

		void BuildMeshInfo()
		{
			StaticMesh::BuildMeshInfo();

			*(effect_->ParameterByName("diffuse_tex")) = diffuse_tex_;
			*(effect_->ParameterByName("specular_tex")) = specular_tex_;
			*(effect_->ParameterByName("emit_tex")) = emit_tex_;

			*(effect_->ParameterByName("ambient_clr")) = float4(mtl_->ambient.x(), mtl_->ambient.y(), mtl_->ambient.z(), 1);
			*(effect_->ParameterByName("diffuse_clr")) = float4(mtl_->diffuse.x(), mtl_->diffuse.y(), mtl_->diffuse.z(), bool(diffuse_tex_));
			*(effect_->ParameterByName("specular_clr")) = float4(mtl_->specular.x(), mtl_->specular.y(), mtl_->specular.z(), bool(specular_tex_));
			*(effect_->ParameterByName("emit_clr")) = float4(mtl_->emit.x(), mtl_->emit.y(), mtl_->emit.z(), bool(emit_tex_));

			*(effect_->ParameterByName("specular_level")) = mtl_->specular_level;
			*(effect_->ParameterByName("shininess")) = std::max(1e-6f, mtl_->shininess);

			RenderFactory& rf = Context::Instance().RenderFactoryInstance();
			RenderDeviceCaps const & caps = rf.RenderEngineInstance().DeviceCaps();
			if (TM_Instanced == caps.tess_method)
			{
				{
					GraphicsBufferPtr vb_sysmem = rf.MakeVertexBuffer(BU_Static, EAH_CPU_Read, NULL);
					vb_sysmem->Resize(rl_->GetVertexStream(0)->Size());
					rl_->GetVertexStream(0)->CopyToBuffer(*vb_sysmem);
				
					GraphicsBuffer::Mapper mapper(*vb_sysmem, BA_Read_Only);
					float3* src = mapper.Pointer<float3>() + this->StartVertexLocation();

					std::vector<float4> dst(this->NumVertices());
					for (size_t i = 0; i < dst.size(); ++ i)
					{
						dst[i] = float4(src[i].x(), src[i].y(), src[i].z(), 1);
					}

					ElementInitData init_data;
					init_data.data = &dst[0];
					init_data.row_pitch = this->NumVertices() * sizeof(float4);
					init_data.slice_pitch = init_data.row_pitch;
					skinned_pos_vb_ = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, &init_data, EF_ABGR32F);
				}
				{
					uint32_t const index_size = (EF_R16UI == rl_->IndexStreamFormat()) ? 2 : 4;

					GraphicsBufferPtr ib_sysmem = rf.MakeVertexBuffer(BU_Static, EAH_CPU_Read, NULL);
					ib_sysmem->Resize(rl_->GetIndexStream()->Size());
					rl_->GetIndexStream()->CopyToBuffer(*ib_sysmem);
				
					GraphicsBuffer::Mapper mapper(*ib_sysmem, BA_Read_Only);
					ElementInitData init_data;
					init_data.data = mapper.Pointer<uint8_t>() + this->StartIndexLocation() * index_size;
					init_data.row_pitch = this->NumTriangles() * 3 * index_size;
					init_data.slice_pitch = init_data.row_pitch;
					bindable_ib_ = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, &init_data, rl_->IndexStreamFormat());
				}

				this->SetTessFactor(static_cast<int32_t>(tess_factor_));
			}
		}

		void MinVariance(float min_variance)
		{
			*(effect_->ParameterByName("min_variance")) = min_variance;
		}

		void BleedingReduce(float bleeding_reduce)
		{
			*(effect_->ParameterByName("bleeding_reduce")) = bleeding_reduce;
		}

		void GenShadowMapPass(bool gen_sm, SM_TYPE sm_type, int pass_index)
		{
			ShadowMapped::GenShadowMapPass(gen_sm, sm_type, pass_index);

			if (gen_sm)
			{
				switch (sm_type_)
				{
				case SMT_DP:
					{
						RenderDeviceCaps const & caps = Context::Instance().RenderFactoryInstance().RenderEngineInstance().DeviceCaps();
						if (caps.tess_method != TM_No)
						{
							if (TM_Hardware == caps.tess_method)
							{
								technique_ = effect_->TechniqueByName("GenDPShadowMapTess5Tech");
							}
							else
							{
								technique_ = effect_->TechniqueByName("GenDPShadowMapTess4Tech");
							}
							smooth_mesh_ = true;
						}
						else
						{
							technique_ = effect_->TechniqueByName("GenDPShadowMap");
							smooth_mesh_ = false;
						}
					}
					mesh_rl_->NumInstances(1);
					break;
				
				case SMT_Cube:
					technique_ = effect_->TechniqueByName("GenCubeShadowMap");
					smooth_mesh_ = false;
					mesh_rl_->NumInstances(1);
					break;

				case SMT_CubeOne:
					technique_ = effect_->TechniqueByName("GenCubeOneShadowMap");
					smooth_mesh_ = false;
					mesh_rl_->NumInstances(1);
					break;

				case SMT_CubeOneInstance:
					technique_ = effect_->TechniqueByName("GenCubeOneInstanceShadowMap");
					smooth_mesh_ = false;
					mesh_rl_->NumInstances(6);
					break;

				default:
					technique_ = effect_->TechniqueByName("GenCubeOneInstanceGSShadowMap");
					smooth_mesh_ = false;
					mesh_rl_->NumInstances(1);
					break;
				}
			}
			else
			{
				technique_ = effect_->TechniqueByName("RenderScene");
				smooth_mesh_ = false;
				mesh_rl_->NumInstances(1);
			}
		}

		void OnRenderBegin()
		{
			ShadowMapped::OnRenderBegin(model_mat_, effect_);

			if (smooth_mesh_)
			{
				RenderDeviceCaps const & caps = Context::Instance().RenderFactoryInstance().RenderEngineInstance().DeviceCaps();
				if (caps.tess_method != TM_No)
				{
					*(effect_->ParameterByName("adaptive_tess")) = true;
					*(effect_->ParameterByName("tess_factors")) = float4(tess_factor_, tess_factor_, 1.0f, 32.0f);

					if (TM_Instanced == caps.tess_method)
					{
						*(effect_->ParameterByName("skinned_pos_buf")) = skinned_pos_vb_;
						*(effect_->ParameterByName("index_buf")) = bindable_ib_;
					}
				}
			}
		}
		
		void SetTessFactor(int32_t tess_factor)
		{
			RenderDeviceCaps const & caps = Context::Instance().RenderFactoryInstance().RenderEngineInstance().DeviceCaps();
			if (TM_Instanced == caps.tess_method)
			{
				if (tess_pattern_vbs.empty())
				{
					InitInstancedTessBuffs();
				}

				tess_factor = std::min(tess_factor, static_cast<int32_t>(tess_pattern_vbs.size()));

				tess_pattern_rl_->BindIndexStream(tess_pattern_ibs[tess_factor - 1], EF_R16UI);
				tess_pattern_rl_->BindVertexStream(tess_pattern_vbs[tess_factor - 1], boost::make_tuple(vertex_element(VEU_TextureCoord, 1, EF_GR32F)),
					RenderLayout::ST_Geometry, mesh_rl_->NumIndices() * 3);
			}

			tess_factor_ = static_cast<float>(tess_factor);
		}

		void Render()
		{
			rl_ = mesh_rl_;
			if (smooth_mesh_)
			{
				RenderDeviceCaps const & caps = Context::Instance().RenderFactoryInstance().RenderEngineInstance().DeviceCaps();
				if (TM_Instanced == caps.tess_method)
				{
					rl_ = tess_pattern_rl_;
				}
			}

			StaticMesh::Render();
		}

	private:	
		RenderEffectPtr effect_;

		bool smooth_mesh_;
		float tess_factor_;

		RenderLayoutPtr mesh_rl_;
		RenderLayoutPtr tess_pattern_rl_;
		GraphicsBufferPtr skinned_pos_vb_;
		GraphicsBufferPtr bindable_ib_;
	};

	class OccluderObjectUpdate
	{
	public:
		void operator()(SceneObject& obj)
		{
			obj.SetModelMatrix(MathLib::scaling(5.0f, 5.0f, 5.0f) * MathLib::translation(5.0f, 5.0f, 0.0f) * MathLib::rotation_y(-static_cast<float>(timer_.elapsed()) / 1.5f));
		}

	private:
		Timer timer_;
	};


	class PointLightSourceUpdate
	{
	public:
		void operator()(LightSource& light)
		{
			light.ModelMatrix(MathLib::rotation_z(0.4f)
				* MathLib::rotation_y(static_cast<float>(timer_.elapsed()) / 1.4f)
				* MathLib::translation(2.0f, 12.0f, 4.0f));
		}

	private:
		Timer timer_;
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


int main()
{
	ResLoader::Instance().AddPath("../../Samples/media/Common");

	Context::Instance().LoadCfg("KlayGE.cfg");

	ShadowCubeMap app;
	app.Create();
	app.Run();

	return 0;
}

ShadowCubeMap::ShadowCubeMap()
				: App3DFramework("ShadowCubeMap"),
					sm_type_(SMT_DP)
{
	ResLoader::Instance().AddPath("../../Samples/media/ShadowCubeMap");
}

bool ShadowCubeMap::ConfirmDevice() const
{
	RenderDeviceCaps const & caps = Context::Instance().RenderFactoryInstance().RenderEngineInstance().DeviceCaps();
	if (caps.max_shader_model < 2)
	{
		return false;
	}
	if (!(caps.rendertarget_format_support(EF_D16, 1, 0)
		&& (caps.rendertarget_format_support(EF_GR16F, 1, 0) || caps.rendertarget_format_support(EF_ABGR16F, 1, 0))))
	{
		return false;
	}

	return true;
}

void ShadowCubeMap::InitObjects()
{
	RenderFactory& rf = Context::Instance().RenderFactoryInstance();
	RenderEngine& re = rf.RenderEngineInstance();
	RenderDeviceCaps const & caps = re.DeviceCaps();

	// ½¨Á¢×ÖÌå
	font_ = rf.MakeFont("gkai00mp.kfont");

	boost::function<RenderModelPtr()> model_ml = ASyncLoadModel("ScifiRoom.7z//ScifiRoom.meshml", EAH_GPU_Read | EAH_Immutable, CreateModelFactory<RenderModel>(), CreateMeshFactory<OccluderMesh>());
	boost::function<RenderModelPtr()> teapot_ml = ASyncLoadModel("teapot.meshml", EAH_GPU_Read | EAH_Immutable, CreateModelFactory<RenderModel>(), CreateMeshFactory<OccluderMesh>());

	this->LookAt(float3(0.0f, 10.0f, -25.0f), float3(0, 10.0f, 0));
	this->Proj(0.01f, 500);

	lamp_tex_ = SyncLoadTexture("lamp.dds", EAH_GPU_Read | EAH_Immutable);

	RenderModelPtr scene_model = model_ml();
	scene_objs_.resize(scene_model->NumMeshes() + 1);
	for (size_t i = 0; i < scene_model->NumMeshes(); ++ i)
	{
		scene_objs_[i] = MakeSharedPtr<SceneObjectHelper>(scene_model->Mesh(i), SceneObject::SOA_Cullable);
	}
	scene_objs_.back() = MakeSharedPtr<SceneObjectHelper>(teapot_ml()->Mesh(0), SceneObject::SOA_Cullable | SceneObject::SOA_Moveable);
	scene_objs_.back()->BindUpdateFunc(OccluderObjectUpdate());

	for (size_t i = 0; i < scene_objs_.size(); ++ i)
	{
		scene_objs_[i]->AddToSceneManager();
		checked_pointer_cast<OccluderMesh>(scene_objs_[i]->GetRenderable())->LampTexture(lamp_tex_);
	}

	ElementFormat fmt;
	if (caps.rendertarget_format_support(EF_D24S8, 1, 0))
	{
		fmt = EF_D24S8;
	}
	else
	{
		BOOST_ASSERT(caps.rendertarget_format_support(EF_D16, 1, 0));

		fmt = EF_D16;
	}
	RenderViewPtr depth_view = rf.Make2DDepthStencilRenderView(SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, fmt, 1, 0);
	if (caps.rendertarget_format_support(EF_GR16F, 1, 0))
	{
		fmt = EF_GR16F;
	}
	else
	{
		BOOST_ASSERT(caps.rendertarget_format_support(EF_ABGR16F, 1, 0));

		fmt = EF_ABGR16F;
	}
	shadow_tex_ = rf.MakeTexture2D(SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write, NULL);
	shadow_cube_buffer_ = rf.MakeFrameBuffer();
	shadow_cube_buffer_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*shadow_tex_, 0, 1, 0));
	shadow_cube_buffer_->Attach(FrameBuffer::ATT_DepthStencil, depth_view);

	shadow_cube_tex_ = rf.MakeTextureCube(SHADOW_MAP_SIZE, 1, 1, shadow_tex_->Format(), 1, 0, EAH_GPU_Read | EAH_GPU_Write, NULL);

	if (caps.max_texture_array_length > 1)
	{
		shadow_cube_one_tex_ = rf.MakeTextureCube(SHADOW_MAP_SIZE, 1, 1, shadow_tex_->Format(), 1, 0, EAH_GPU_Read | EAH_GPU_Write, NULL);
		shadow_cube_one_buffer_ = rf.MakeFrameBuffer();
		shadow_cube_one_buffer_->GetViewport().camera->OmniDirectionalMode(true);
		shadow_cube_one_buffer_->GetViewport().camera->ProjParams(PI / 2, 1, 0.1f, 500.0f);
		shadow_cube_one_buffer_->Attach(FrameBuffer::ATT_Color0, rf.MakeCubeRenderView(*shadow_cube_one_tex_, 0, 0));
		TexturePtr shadow_one_depth_tex = rf.MakeTextureCube(SHADOW_MAP_SIZE, 1, 1, EF_D24S8, 1, 0, EAH_GPU_Write, NULL);
		shadow_cube_one_buffer_->Attach(FrameBuffer::ATT_DepthStencil, rf.MakeCubeDepthStencilRenderView(*shadow_one_depth_tex, 0, 0));
	}

	for (int i = 0; i < 2; ++ i)
	{
		shadow_dual_texs_[i] = rf.MakeTexture2D(SHADOW_MAP_SIZE,  SHADOW_MAP_SIZE, 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write, NULL);
		shadow_dual_view_[i] = rf.Make2DRenderView(*shadow_dual_texs_[i], 0, 1, 0);

		shadow_dual_buffers_[i] = rf.MakeFrameBuffer();
		shadow_dual_buffers_[i]->GetViewport().camera->ProjParams(PI, 1, 0.1f, 500.0f);
		shadow_dual_buffers_[i]->Attach(FrameBuffer::ATT_Color0, shadow_dual_view_[i]);
		shadow_dual_buffers_[i]->Attach(FrameBuffer::ATT_DepthStencil, depth_view);
	}
	shadow_dual_tex_ = rf.MakeTexture2D(SHADOW_MAP_SIZE * 2,  SHADOW_MAP_SIZE, 1, 1, fmt, 1, 0, EAH_GPU_Read, NULL);

	for (size_t i = 0; i < scene_objs_.size(); ++ i)
	{
		checked_pointer_cast<OccluderMesh>(scene_objs_[i]->GetRenderable())->CubeSMTexture(shadow_cube_tex_);
		checked_pointer_cast<OccluderMesh>(scene_objs_[i]->GetRenderable())->DPSMTexture(shadow_dual_tex_);
	}

	light_ = MakeSharedPtr<PointLightSource>();
	light_->Attrib(0);
	light_->Color(float3(1, 1, 1));
	light_->Falloff(float3(1, 0.05f, 0));
	light_->ProjectiveTexture(lamp_tex_);
	light_->BindUpdateFunc(PointLightSourceUpdate());
	light_->AddToSceneManager();

	light_proxy_ = MakeSharedPtr<SceneObjectLightSourceProxy>(light_);
	checked_pointer_cast<SceneObjectLightSourceProxy>(light_proxy_)->Scaling(0.5f, 0.5f, 0.5f);
	light_proxy_->AddToSceneManager();

	for (int i = 0; i < 6; ++ i)
	{
		sm_filter_pps_[i] = MakeSharedPtr<BlurPostProcess<SeparableGaussianFilterPostProcess> >(3, 1.0f);
	
		sm_filter_pps_[i]->InputPin(0, shadow_tex_);
		sm_filter_pps_[i]->OutputPin(0, shadow_cube_tex_, 0, 0, i);
		if (!re.RequiresFlipping())
		{
			switch (i)
			{
			case Texture::CF_Positive_Y:
				sm_filter_pps_[i]->OutputPin(0, shadow_cube_tex_, 0, 0, Texture::CF_Negative_Y);
				break;

			case Texture::CF_Negative_Y:
				sm_filter_pps_[i]->OutputPin(0, shadow_cube_tex_, 0, 0, Texture::CF_Positive_Y);
				break;

			default:
				break;
			}
		}
	}

	fpcController_.Scalers(0.05f, 1.0f);

	InputEngine& inputEngine(Context::Instance().InputFactoryInstance().InputEngineInstance());
	InputActionMap actionMap;
	actionMap.AddActions(actions, actions + sizeof(actions) / sizeof(actions[0]));

	action_handler_t input_handler = MakeSharedPtr<input_signal>();
	input_handler->connect(boost::bind(&ShadowCubeMap::InputHandler, this, _1, _2));
	inputEngine.ActionMap(actionMap, input_handler, true);

	UIManager::Instance().Load(ResLoader::Instance().Open("ShadowCubemap.uiml"));
	dialog_ = UIManager::Instance().GetDialogs()[0];

	id_min_variance_static_ = dialog_->IDFromName("MinVarianceStatic");
	id_min_variance_slider_ = dialog_->IDFromName("MinVarianceSlider");
	id_bleeding_reduce_static_ = dialog_->IDFromName("BleedingReduceStatic");
	id_bleeding_reduce_slider_ = dialog_->IDFromName("BleedingReduceSlider");
	id_sm_type_static_ = dialog_->IDFromName("SMStatic");
	id_sm_type_combo_ = dialog_->IDFromName("SMCombo");
	id_ctrl_camera_ = dialog_->IDFromName("CtrlCamera");

	dialog_->Control<UISlider>(id_min_variance_slider_)->OnValueChangedEvent().connect(boost::bind(&ShadowCubeMap::MinVarianceChangedHandler, this, _1));
	dialog_->Control<UISlider>(id_bleeding_reduce_slider_)->OnValueChangedEvent().connect(boost::bind(&ShadowCubeMap::BleedingReduceChangedHandler, this, _1));
	dialog_->Control<UIComboBox>(id_sm_type_combo_)->OnSelectionChangedEvent().connect(boost::bind(&ShadowCubeMap::SMTypeChangedHandler, this, _1));
	dialog_->Control<UICheckBox>(id_ctrl_camera_)->OnChangedEvent().connect(boost::bind(&ShadowCubeMap::CtrlCameraHandler, this, _1));

	this->MinVarianceChangedHandler(*dialog_->Control<UISlider>(id_min_variance_slider_));
	this->BleedingReduceChangedHandler(*dialog_->Control<UISlider>(id_bleeding_reduce_slider_));
	this->SMTypeChangedHandler(*dialog_->Control<UIComboBox>(id_sm_type_combo_));
}

void ShadowCubeMap::OnResize(uint32_t width, uint32_t height)
{
	App3DFramework::OnResize(width, height);

	UIManager::Instance().SettleCtrls(width, height);
}

void ShadowCubeMap::InputHandler(InputEngine const & /*sender*/, InputAction const & action)
{
	switch (action.first)
	{
	case Exit:
		this->Quit();
		break;
	}
}

void ShadowCubeMap::MinVarianceChangedHandler(KlayGE::UISlider const & sender)
{
	float min_var = sender.GetValue() / 2000.0f + 0.001f;
	for (size_t i = 0; i < scene_objs_.size(); ++ i)
	{
		checked_pointer_cast<OccluderMesh>(scene_objs_[i]->GetRenderable())->MinVariance(min_var);
	}
}

void ShadowCubeMap::BleedingReduceChangedHandler(KlayGE::UISlider const & sender)
{
	float bleeding = sender.GetValue() / 500.0f + 0.45f;
	for (size_t i = 0; i < scene_objs_.size(); ++ i)
	{
		checked_pointer_cast<OccluderMesh>(scene_objs_[i]->GetRenderable())->BleedingReduce(bleeding);
	}
}

void ShadowCubeMap::SMTypeChangedHandler(KlayGE::UIComboBox const & sender)
{
	sm_type_ = static_cast<SM_TYPE>(sender.GetSelectedIndex());
}

void ShadowCubeMap::CtrlCameraHandler(KlayGE::UICheckBox const & sender)
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

void ShadowCubeMap::DoUpdateOverlay()
{
	UIManager::Instance().Render();

	std::wostringstream stream;
	stream.precision(2);
	stream << std::fixed << this->FPS() << " FPS";

	font_->RenderText(0, 0, Color(1, 1, 0, 1), L"ShadowCubeMap", 16);
	font_->RenderText(0, 18, Color(1, 1, 0, 1), stream.str(), 16);
}

uint32_t ShadowCubeMap::DoUpdate(uint32_t pass)
{
	RenderEngine& renderEngine = Context::Instance().RenderFactoryInstance().RenderEngineInstance();

	switch (sm_type_)
	{
	case SMT_DP:
		switch (pass)
		{
		case 0:
		case 1:
			{
				float3 pos = light_->Position();
				float3 lookat = light_->Position() + ((0 == pass) ? 1.0f : -1.0f) * light_->Direction();
				shadow_dual_buffers_[pass]->GetViewport().camera->ViewParams(pos, lookat);

				renderEngine.BindFrameBuffer(shadow_dual_buffers_[pass]);
				renderEngine.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, Color(0.0f, 0.0f, 0.0f, 1), 1.0f, 0);

				for (size_t i = 0; i < scene_objs_.size(); ++ i)
				{
					checked_pointer_cast<OccluderMesh>(scene_objs_[i]->GetRenderable())->GenShadowMapPass(true, sm_type_, pass);
					checked_pointer_cast<OccluderMesh>(scene_objs_[i]->GetRenderable())->LightSrc(light_);
				}
			}
			return App3DFramework::URV_Need_Flush;

		default:
			{
				renderEngine.BindFrameBuffer(FrameBufferPtr());
				
				Color clear_clr(0.2f, 0.4f, 0.6f, 1);
				if (Context::Instance().Config().graphics_cfg.gamma)
				{
					clear_clr.r() = 0.029f;
					clear_clr.g() = 0.133f;
					clear_clr.b() = 0.325f;
				}
				renderEngine.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, clear_clr, 1.0f, 0);

				shadow_dual_texs_[0]->CopyToSubTexture2D(*shadow_dual_tex_, 0, 0, 0, 0, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, 0, 0, 0, 0, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);
				shadow_dual_texs_[1]->CopyToSubTexture2D(*shadow_dual_tex_, 0, 0, SHADOW_MAP_SIZE, 0, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, 0, 0, 0, 0, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);

				for (size_t i = 0; i < scene_objs_.size(); ++ i)
				{
					checked_pointer_cast<OccluderMesh>(scene_objs_[i]->GetRenderable())->GenShadowMapPass(false, sm_type_, pass);
				}
			}
			return App3DFramework::URV_Need_Flush | App3DFramework::URV_Finished;
		}
		break;
	
	case SMT_Cube:
		if (pass > 0)
		{
			sm_filter_pps_[pass - 1]->Apply();
		}

		switch (pass)
		{
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
			{
				renderEngine.BindFrameBuffer(shadow_cube_buffer_);
				renderEngine.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, Color(0.0f, 0.0f, 0.0f, 1), 1.0f, 0);

				shadow_cube_buffer_->GetViewport().camera = light_->SMCamera(pass);

				for (size_t i = 0; i < scene_objs_.size(); ++ i)
				{
					checked_pointer_cast<OccluderMesh>(scene_objs_[i]->GetRenderable())->GenShadowMapPass(true, sm_type_, pass);
					checked_pointer_cast<OccluderMesh>(scene_objs_[i]->GetRenderable())->LightSrc(light_);
				}
			}
			return App3DFramework::URV_Need_Flush;

		default:
			{
				renderEngine.BindFrameBuffer(FrameBufferPtr());

				Color clear_clr(0.2f, 0.4f, 0.6f, 1);
				if (Context::Instance().Config().graphics_cfg.gamma)
				{
					clear_clr.r() = 0.029f;
					clear_clr.g() = 0.133f;
					clear_clr.b() = 0.325f;
				}
				renderEngine.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, clear_clr, 1.0f, 0);

				for (size_t i = 0; i < scene_objs_.size(); ++ i)
				{
					checked_pointer_cast<OccluderMesh>(scene_objs_[i]->GetRenderable())->GenShadowMapPass(false, sm_type_, pass);
				}
			}
			return App3DFramework::URV_Need_Flush | App3DFramework::URV_Finished;
		}
		break;

	default:
		if (renderEngine.DeviceCaps().max_texture_array_length > 1)
		{
			switch (pass)
			{
			case 0:
				{
					shadow_cube_one_buffer_->GetViewport().camera->ViewParams(light_->Position(), light_->Position() + light_->Direction());

					renderEngine.BindFrameBuffer(shadow_cube_one_buffer_);
					renderEngine.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, Color(0.0f, 0.0f, 0.0f, 1), 1.0f, 0);

					for (size_t i = 0; i < scene_objs_.size(); ++ i)
					{
						checked_pointer_cast<OccluderMesh>(scene_objs_[i]->GetRenderable())->GenShadowMapPass(true, sm_type_, pass);
						checked_pointer_cast<OccluderMesh>(scene_objs_[i]->GetRenderable())->LightSrc(light_);
					}
				}
				return App3DFramework::URV_Need_Flush;

			default:
				{
					for (int p = 0; p < 6; ++ p)
					{
						shadow_cube_one_tex_->CopyToSubTexture2D(*shadow_tex_, 0, 0, 0, 0, shadow_tex_->Width(0), shadow_tex_->Height(0), 
							p, 0, 0, 0, shadow_cube_one_tex_->Width(0), shadow_cube_one_tex_->Height(0));
						sm_filter_pps_[p]->Apply();
					}

					renderEngine.BindFrameBuffer(FrameBufferPtr());

					Color clear_clr(0.2f, 0.4f, 0.6f, 1);
					if (Context::Instance().Config().graphics_cfg.gamma)
					{
						clear_clr.r() = 0.029f;
						clear_clr.g() = 0.133f;
						clear_clr.b() = 0.325f;
					}
					renderEngine.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, clear_clr, 1.0f, 0);

					for (size_t i = 0; i < scene_objs_.size(); ++ i)
					{
						checked_pointer_cast<OccluderMesh>(scene_objs_[i]->GetRenderable())->GenShadowMapPass(false, sm_type_, pass);
					}
				}
				return App3DFramework::URV_Need_Flush | App3DFramework::URV_Finished;
			}
		}
		else
		{
			return App3DFramework::URV_Finished;
		}
		break;
	}
}
