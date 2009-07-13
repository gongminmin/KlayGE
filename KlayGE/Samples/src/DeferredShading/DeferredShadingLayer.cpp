#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/Renderable.hpp>
#include <KlayGE/RenderableHelper.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/FrameBuffer.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/SceneObjectHelper.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/App3D.hpp>

#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable: 4702)
#endif
#include <boost/lexical_cast.hpp>
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(pop)
#endif

#include "DeferredShadingLayer.hpp"

namespace KlayGE
{
	int const SM_SIZE = 512;

	DeferredShadingLayer::DeferredShadingLayer()
			: RenderableHelper(L"DeferredShadingLayer"),
				buffer_type_(0)
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();

		rl_ = rf.MakeRenderLayout();
		rl_->TopologyType(RenderLayout::TT_TriangleList);

		if (rf.RenderEngineInstance().DeviceCaps().max_shader_model < 4)
		{
			max_num_lights_a_batch_ = 1;
		}
		else
		{
			max_num_lights_a_batch_ = 1;
		}

		box_ = Box(float3(-1, -1, -1), float3(1, 1, 1));

		light_mask_vb_ = rf.MakeVertexBuffer(BU_Dynamic, EAH_GPU_Read | EAH_CPU_Write, NULL);
		rl_->BindVertexStream(light_mask_vb_,
			boost::make_tuple(vertex_element(VEU_Position, 0, EF_ABGR32F)));
		light_id_vb_ = rf.MakeVertexBuffer(BU_Dynamic, EAH_GPU_Read | EAH_CPU_Write, NULL);
		rl_->BindVertexStream(light_id_vb_,
			boost::make_tuple(vertex_element(VEU_TextureCoord, 0, EF_R32F)));

		light_mask_ib_ = rf.MakeIndexBuffer(BU_Dynamic, EAH_GPU_Read | EAH_CPU_Write, NULL);
		rl_->BindIndexStream(light_mask_ib_, EF_R16UI);

		std::pair<std::string, std::string> macros[] = { std::make_pair("MAX_NUM_LIGHTS", ""), std::make_pair("", "") };
		macros[0].second = boost::lexical_cast<std::string>(max_num_lights_a_batch_);
		technique_ = rf.LoadEffect("DeferredShading.fxml", macros)->TechniqueByName("DeferredShading");

		RenderViewPtr ds_view = rf.MakeDepthStencilRenderView(SM_SIZE, SM_SIZE, EF_D16, 1, 0);
		sm_tex_ = rf.MakeTexture2D(SM_SIZE, SM_SIZE, 1, static_cast<uint16_t>(max_num_lights_a_batch_), EF_GR16F, 1, 0, EAH_GPU_Read | EAH_GPU_Write, NULL);
		sm_buffer_.resize(max_num_lights_a_batch_);
		for (int i = 0; i < max_num_lights_a_batch_; ++ i)
		{
			sm_buffer_[i] = rf.MakeFrameBuffer();
			sm_buffer_[i]->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*sm_tex_, i, 0));
			sm_buffer_[i]->Attach(FrameBuffer::ATT_DepthStencil, ds_view);
		}

		*(technique_->Effect().ParameterByName("sm_flipping")) = static_cast<int32_t>(sm_buffer_[0]->RequiresFlipping() ? -1 : 1);
		if (rf.RenderEngineInstance().DeviceCaps().max_shader_model < 4)
		{
			*(technique_->Effect().ParameterByName("shadow_map_tex")) = sm_tex_;
		}
		else
		{
			*(technique_->Effect().ParameterByName("shadow_map_tex_array")) = sm_tex_;
		}

		texel_to_pixel_offset_param_ = technique_->Effect().ParameterByName("texel_to_pixel_offset");
		depth_near_far_invfar_param_ = technique_->Effect().ParameterByName("depth_near_far_invfar");
		upper_left_param_ = technique_->Effect().ParameterByName("upper_left");
		upper_right_param_ = technique_->Effect().ParameterByName("upper_right");
		lower_left_param_ = technique_->Effect().ParameterByName("lower_left");
		lower_right_param_ = technique_->Effect().ParameterByName("lower_right");
		inv_view_param_ = technique_->Effect().ParameterByName("inv_view");
		show_skybox_param_ = technique_->Effect().ParameterByName("show_skybox");
		num_lights_param_ = technique_->Effect().ParameterByName("num_lights");
		light_attrib_param_ = technique_->Effect().ParameterByName("light_attrib");
		light_clr_type_param_ = technique_->Effect().ParameterByName("light_clr_type");
		light_falloff_param_ = technique_->Effect().ParameterByName("light_falloff");
		light_view_proj_param_ = technique_->Effect().ParameterByName("light_view_proj");
		light_pos_es_param_ = technique_->Effect().ParameterByName("light_pos_es");
		light_dir_es_param_ = technique_->Effect().ParameterByName("light_dir_es");

		light_attrib_enabled_.resize(max_num_lights_a_batch_);
		light_clr_type_enabled_.resize(max_num_lights_a_batch_);
		light_falloff_enabled_.resize(max_num_lights_a_batch_);
		light_view_proj_enabled_.resize(max_num_lights_a_batch_);
		light_pos_es_enabled_.resize(max_num_lights_a_batch_);
		light_dir_es_enabled_.resize(max_num_lights_a_batch_);
	}

	int DeferredShadingLayer::AddAmbientLight(int32_t attr, float3 const & clr)
	{
		int id = static_cast<int>(light_clr_type_.size());
		light_enabled_.push_back(1);
		light_attrib_.push_back(attr | LSA_NoShadow);
		light_clr_type_.push_back(float4(clr.x(), clr.y(), clr.z(), LT_Ambient + 0.1f));
		light_pos_.push_back(float4(0, 0, 0, 0));
		light_dir_.push_back(float4(0, 0, 0, 0));
		light_falloff_.push_back(float4(0, 0, 0, 0));
		light_cos_outer_inner_.push_back(float3(0, 0, 0));
		return id;
	}

	int DeferredShadingLayer::AddPointLight(int32_t attr, float3 const & pos, float3 const & clr, float3 const & falloff)
	{
		int id = static_cast<int>(light_clr_type_.size());
		light_enabled_.push_back(1);
		light_attrib_.push_back(attr);
		light_clr_type_.push_back(float4(clr.x(), clr.y(), clr.z(), LT_Point + 0.1f));
		light_pos_.push_back(float4(pos.x(), pos.y(), pos.z(), 1));
		light_dir_.push_back(float4(0, 0, 0, 0));
		light_falloff_.push_back(float4(falloff.x(), falloff.y(), falloff.z(), 0));
		light_cos_outer_inner_.push_back(float3(0, 0, 0));
		return id;
	}

	int DeferredShadingLayer::AddDirectionalLight(int32_t attr, float3 const & dir, float3 const & clr, float3 const & falloff)
	{
		float3 d = MathLib::normalize(dir);
		int id = static_cast<int>(light_clr_type_.size());
		light_enabled_.push_back(1);
		light_attrib_.push_back(attr);
		light_clr_type_.push_back(float4(clr.x(), clr.y(), clr.z(), LT_Directional + 0.1f));
		light_pos_.push_back(float4(0, 0, 0, 0));
		light_dir_.push_back(float4(d.x(), d.y(), d.z(), 0));
		light_falloff_.push_back(float4(falloff.x(), falloff.y(), falloff.z(), 0));
		light_cos_outer_inner_.push_back(float3(0, 0, 0));
		return id;
	}

	int DeferredShadingLayer::AddSpotLight(int32_t attr, float3 const & pos, float3 const & dir, float outer, float inner, float3 const & clr, float3 const & falloff)
	{
		float3 d = MathLib::normalize(dir);
		int id = static_cast<int>(light_clr_type_.size());
		light_enabled_.push_back(1);
		light_attrib_.push_back(attr);
		light_clr_type_.push_back(float4(clr.x(), clr.y(), clr.z(), LT_Spot + 0.1f));
		light_pos_.push_back(float4(pos.x(), pos.y(), pos.z(), 1));
		light_dir_.push_back(float4(d.x(), d.y(), d.z(), 0));
		light_falloff_.push_back(float4(falloff.x(), falloff.y(), falloff.z(), 0));
		light_cos_outer_inner_.push_back(float3(cos(outer), cos(inner), outer * 2));
		return id;
	}

	void DeferredShadingLayer::LightAttrib(int index, uint32_t attr)
	{
		light_attrib_[index] = attr;
	}

	void DeferredShadingLayer::LightColor(int index, float3 const & clr)
	{
		light_clr_type_[index] = float4(clr.x(), clr.y(), clr.z(), light_clr_type_[index].w());
	}

	void DeferredShadingLayer::LightDir(int index, float3 const & dir)
	{
		float3 d = MathLib::normalize(dir);
		light_dir_[index] = float4(d.x(), d.y(), d.z(), 0);
	}

	void DeferredShadingLayer::LightPos(int index, float3 const & pos)
	{
		light_pos_[index] = float4(pos.x(), pos.y(), pos.z(), 1);
	}

	void DeferredShadingLayer::LightFalloff(int index, float3 const & falloff)
	{
		light_falloff_[index] = float4(falloff.x(), falloff.y(), falloff.z(), 0);
	}

	void DeferredShadingLayer::SpotLightAngle(int index, float outer, float inner)
	{
		light_cos_outer_inner_[index] = float3(cos(outer), cos(inner), outer * 2);
	}

	float3 DeferredShadingLayer::LightColor(int index) const
	{
		return *reinterpret_cast<float3 const *>(&light_clr_type_[index]);
	}

	float3 DeferredShadingLayer::LightDir(int index) const
	{
		return *reinterpret_cast<float3 const *>(&light_dir_[index]);
	}

	float3 DeferredShadingLayer::LightPos(int index) const
	{
		return *reinterpret_cast<float3 const *>(&light_pos_[index]);
	}

	float3 DeferredShadingLayer::LightFalloff(int index) const
	{
		return *reinterpret_cast<float3 const *>(&light_falloff_[index]);
	}

	float2 DeferredShadingLayer::SpotLightAngle(int index) const
	{
		return light_cos_outer_inner_[index];
	}

	void DeferredShadingLayer::LightEnabled(int index, bool enable)
	{
		light_enabled_[index] = enable;
	}

	bool DeferredShadingLayer::LightEnabled(int index) const
	{
		return light_enabled_[index] != 0;
	}

	void DeferredShadingLayer::Destinate(FrameBufferPtr const & fb)
	{
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();

		if (fb)
		{
			frame_buffer_ = fb;
		}
		else
		{
			frame_buffer_ = re.DefaultFrameBuffer();
		}
	}

	void DeferredShadingLayer::GBufferTexs(TexturePtr const & nd_tex, TexturePtr const & clr_tex, bool flipping)
	{
		if (nd_tex)
		{
			*(technique_->Effect().ParameterByName("inv_width_height")) = float2(1.0f / nd_tex->Width(0), 1.0f / nd_tex->Height(0));
		}

		*(technique_->Effect().ParameterByName("nd_tex")) = nd_tex;
		*(technique_->Effect().ParameterByName("color_tex")) = clr_tex;
		*(technique_->Effect().ParameterByName("flipping")) = static_cast<int32_t>(flipping ? -1 : +1);
	}

	void DeferredShadingLayer::SSAOTex(TexturePtr const & tex)
	{
		*(technique_->Effect().ParameterByName("ssao_tex")) = tex;
	}

	void DeferredShadingLayer::SSAOEnabled(bool ssao)
	{
		*(technique_->Effect().ParameterByName("ssao_enabled")) = ssao;
	}

	void DeferredShadingLayer::BufferType(int buffer_type)
	{
		buffer_type_ = buffer_type;
		switch (buffer_type_)
		{
		case 0:
			technique_ = technique_->Effect().TechniqueByName("DeferredShading");
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

	void DeferredShadingLayer::ScanLightSrc()
	{
		light_scaned_.resize(0);
		for (size_t i = 0; i < light_clr_type_.size(); ++ i)
		{
			if (light_enabled_[i])
			{
				int type = static_cast<int>(light_clr_type_[i].w());
				if (LT_Point == type)
				{
					for (int j = 0; j < 6; ++ j)
					{
						light_scaned_.push_back(static_cast<uint32_t>((i << 16) + j));
					}
				}
				else
				{
					light_scaned_.push_back(static_cast<uint32_t>((i << 16) + 0));
				}
			}
		}
	}

	uint32_t DeferredShadingLayer::Update(uint32_t pass)
	{
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();

		if (0 == pass)
		{
			Camera const & camera = Context::Instance().AppInstance().ActiveCamera();

			view_ = camera.ViewMatrix();
			proj_ = camera.ProjMatrix();
			inv_view_ = MathLib::inverse(view_);
			float4x4 const inv_proj = MathLib::inverse(proj_);

			*depth_near_far_invfar_param_ = float3(camera.NearPlane(), camera.FarPlane(), 1 / camera.FarPlane());

			*upper_left_param_ = MathLib::transform_coord(float3(-1, 1, 1), inv_proj);
			*upper_right_param_ = MathLib::transform_coord(float3(1, 1, 1), inv_proj);
			*lower_left_param_ = MathLib::transform_coord(float3(-1, -1, 1), inv_proj);
			*lower_right_param_ = MathLib::transform_coord(float3(1, -1, 1), inv_proj);

			*inv_view_param_ = inv_view_;

			RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
			float4 texel_to_pixel = re.TexelToPixelOffset();
			texel_to_pixel.x() /= frame_buffer_->Width() / 2.0f;
			texel_to_pixel.y() /= frame_buffer_->Height() / 2.0f;
			*texel_to_pixel_offset_param_ = texel_to_pixel;

			this->ScanLightSrc();
		}

		if (0 == buffer_type_)
		{
			int32_t batch = pass / (max_num_lights_a_batch_ + 1);
			int32_t pass_in_batch = pass - batch * (max_num_lights_a_batch_ + 1);

			int32_t num_lights = static_cast<int32_t>(light_scaned_.size());
			int32_t start = batch * max_num_lights_a_batch_;
			int32_t n = std::min(num_lights - start, max_num_lights_a_batch_);

			if (pass_in_batch < n)
			{
				int32_t light_index = batch * max_num_lights_a_batch_ + pass_in_batch;

				int32_t org_no = light_scaned_[light_index] >> 16;
				int32_t offset = light_scaned_[light_index] & 0xFFFF;
				light_attrib_enabled_[pass_in_batch] = light_attrib_[org_no];
				light_clr_type_enabled_[pass_in_batch] = light_clr_type_[org_no];

				int type = static_cast<int>(light_clr_type_enabled_[pass_in_batch].w());
				if (type != LT_Ambient)
				{
					float3 d, u;
					if (type != LT_Point)
					{
						d = *reinterpret_cast<float3*>(&light_dir_[org_no]);
						u = float3(0, 1, 0);
					}
					else
					{
						std::pair<float3, float3> ad = CubeMapViewVector<float>(static_cast<Texture::CubeFaces>(offset));
						d = ad.first;
						u = ad.second;
					}

					float fov;
					if (type != LT_Spot)
					{
						fov = PI / 2;
					}
					else
					{
						fov = light_cos_outer_inner_[org_no].z();
					}

					float3 p = *reinterpret_cast<float3*>(&light_pos_[org_no]);
					sm_buffer_[pass_in_batch]->GetViewport().camera->ViewParams(p, p + d, u);
					sm_buffer_[pass_in_batch]->GetViewport().camera->ProjParams(fov, 1, 0.1f, 100.0f);

					float3 dir_es = MathLib::transform_normal(d, view_);
					light_dir_es_enabled_[pass_in_batch] = float4(dir_es.x(), dir_es.y(), dir_es.z(), offset + 0.1f);

					light_view_proj_enabled_[pass_in_batch] = inv_view_ * sm_buffer_[pass_in_batch]->GetViewport().camera->ViewMatrix()
						* sm_buffer_[pass_in_batch]->GetViewport().camera->ProjMatrix();
					if (type != LT_Directional)
					{
						light_falloff_enabled_[pass_in_batch] = light_falloff_[org_no];

						float3 loc_es = MathLib::transform_coord(p, view_);
						light_pos_es_enabled_[pass_in_batch] = float4(loc_es.x(), loc_es.y(), loc_es.z(), 1);

						if (LT_Spot == type)
						{
							light_pos_es_enabled_[pass_in_batch].w() = light_cos_outer_inner_[org_no].x();
							light_dir_es_enabled_[pass_in_batch].w() = light_cos_outer_inner_[org_no].y();
						}
					}
				}

				if (0 == (light_attrib_enabled_[pass_in_batch] & LSA_NoShadow))
				{
					re.BindFrameBuffer(sm_buffer_[pass_in_batch]);
					re.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, Color(0, 0, 0, 0), 1.0f, 0);

					return App3DFramework::URV_Need_Flush;
				}
				else
				{
					return App3DFramework::URV_Flushed;
				}
			}
			else
			{
				re.BindFrameBuffer(frame_buffer_);

				if (0 == batch)
				{
					re.CurFrameBuffer()->Attached(FrameBuffer::CBM_Color)->Clear(Color(0, 0, 0, 0));
					*show_skybox_param_ = true;
				}
				else
				{
					*show_skybox_param_ = false;
				}

				std::vector<float4> pos;
				std::vector<float> lid;
				std::vector<uint16_t> index;
				for (uint16_t i = 0; i < n; ++ i)
				{
					uint16_t vertex_base = static_cast<uint16_t>(pos.size());

					int type = static_cast<int>(light_clr_type_enabled_[i].w());
					if (LT_Spot == type)
					{
						int32_t light_index = batch * max_num_lights_a_batch_ + i;
						int32_t org_no = light_scaned_[light_index] >> 16;

						CreateConeMesh(pos, index, vertex_base, 100 * tan(light_cos_outer_inner_[org_no].z() / 2), 100.0f, 12);

						lid.resize(pos.size());

						float4x4 mat = MathLib::rotation_x(-PI / 2) * MathLib::inverse(sm_buffer_[i]->GetViewport().camera->ViewMatrix()) * view_ * proj_;
						for (size_t j = vertex_base; j < pos.size(); ++ j)
						{
							pos[j].w() = 1;
							pos[j] = MathLib::transform(pos[j], mat);
							lid[j] = i + 0.1f;
						}
					}
					else
					{
						pos.push_back(float4(-1, +1, 0, 1));
						pos.push_back(float4(+1, +1, 0, 1));
						pos.push_back(float4(-1, -1, 0, 1));
						pos.push_back(float4(+1, -1, 0, 1));

						lid.push_back(i + 0.1f);
						lid.push_back(i + 0.1f);
						lid.push_back(i + 0.1f);
						lid.push_back(i + 0.1f);

						index.push_back(vertex_base + 0);
						index.push_back(vertex_base + 1);
						index.push_back(vertex_base + 3);
						index.push_back(vertex_base + 3);
						index.push_back(vertex_base + 2);
						index.push_back(vertex_base + 0);
					}
				};
				light_mask_vb_->Resize(static_cast<uint32_t>(pos.size() * sizeof(pos[0])));
				{
					GraphicsBuffer::Mapper mapper(*light_mask_vb_, BA_Write_Only);
					std::copy(pos.begin(), pos.end(), mapper.Pointer<float4>());
				}
				light_id_vb_->Resize(static_cast<uint32_t>(lid.size() * sizeof(lid[0])));
				{
					GraphicsBuffer::Mapper mapper(*light_id_vb_, BA_Write_Only);
					std::copy(lid.begin(), lid.end(), mapper.Pointer<float>());
				}
				light_mask_ib_->Resize(static_cast<uint32_t>(index.size() * sizeof(index[0])));
				{
					GraphicsBuffer::Mapper mapper(*light_mask_ib_, BA_Write_Only);
					std::copy(index.begin(), index.end(), mapper.Pointer<uint16_t>());
				}

				*num_lights_param_ = n;
				*light_attrib_param_ = light_attrib_enabled_;
				*light_clr_type_param_ = light_clr_type_enabled_;
				*light_falloff_param_ = light_falloff_enabled_;
				*light_view_proj_param_ = light_view_proj_enabled_;
				*light_pos_es_param_ = light_pos_es_enabled_;
				*light_dir_es_param_ = light_dir_es_enabled_;

				RenderableHelper::Render();

				if (start + n >= num_lights)
				{
					return App3DFramework::URV_Finished;
				}
				else
				{
					return App3DFramework::URV_Flushed;
				}
			}
		}
		else
		{
			re.BindFrameBuffer(FrameBufferPtr());
			re.CurFrameBuffer()->Attached(FrameBuffer::CBM_Depth)->Clear(1.0f);
			RenderableHelper::Render();

			return App3DFramework::URV_Finished;
		}
	}
}
