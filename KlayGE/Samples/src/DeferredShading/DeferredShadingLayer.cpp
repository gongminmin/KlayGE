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
#include <KlayGE/SceneManager.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/App3D.hpp>
#include <KlayGE/PostProcess.hpp>
#include <KlayGE/Query.hpp>

#include <boost/typeof/typeof.hpp>
#include <boost/foreach.hpp>
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

		light_enabled_.push_back(1);
		light_attrib_.push_back(LSA_NoShadow);
		light_clr_type_.push_back(float4(0, 0, 0, LT_Ambient + 0.1f));
		light_pos_.push_back(float4(0, 0, 0, 0));
		light_dir_.push_back(float4(0, 0, 0, 0));
		light_falloff_.push_back(float4(0, 0, 0, 0));
		light_cos_outer_inner_.push_back(float4(0, 0, 0, 0));

		g_buffer_ = rf.MakeFrameBuffer();
		shaded_buffer_ = rf.MakeFrameBuffer();

		box_ = Box(float3(-1, -1, -1), float3(1, 1, 1));

		{
			rl_cone_ = rf.MakeRenderLayout();
			rl_cone_->TopologyType(RenderLayout::TT_TriangleList);

			std::vector<float3> pos;
			std::vector<uint16_t> index;
			CreateConeMesh(pos, index, 0, 100.0f, 100.0f, 12);

			ElementInitData init_data;
			init_data.row_pitch = static_cast<uint32_t>(pos.size() * sizeof(pos[0]));
			init_data.slice_pitch = 0;
			init_data.data = &pos[0];
			rl_cone_->BindVertexStream(rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read, &init_data),
				boost::make_tuple(vertex_element(VEU_Position, 0, EF_BGR32F)));

			init_data.row_pitch = static_cast<uint32_t>(index.size() * sizeof(index[0]));
			init_data.data = &index[0];
			rl_cone_->BindIndexStream(rf.MakeIndexBuffer(BU_Static, EAH_GPU_Read, &init_data), EF_R16UI);
		}
		{
			rl_pyramid_ = rf.MakeRenderLayout();
			rl_pyramid_->TopologyType(RenderLayout::TT_TriangleList);

			std::vector<float3> pos;
			std::vector<uint16_t> index;
			CreatePyramidMesh(pos, index, 0, 100.0f, 100.0f);

			ElementInitData init_data;
			init_data.row_pitch = static_cast<uint32_t>(pos.size() * sizeof(pos[0]));
			init_data.slice_pitch = 0;
			init_data.data = &pos[0];
			rl_pyramid_->BindVertexStream(rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read, &init_data),
				boost::make_tuple(vertex_element(VEU_Position, 0, EF_BGR32F)));

			init_data.row_pitch = static_cast<uint32_t>(index.size() * sizeof(index[0]));
			init_data.data = &index[0];
			rl_pyramid_->BindIndexStream(rf.MakeIndexBuffer(BU_Static, EAH_GPU_Read, &init_data), EF_R16UI);
		}
		{
			rl_quad_ = rf.MakeRenderLayout();
			rl_quad_->TopologyType(RenderLayout::TT_TriangleStrip);

			std::vector<float3> pos;
			std::vector<uint16_t> index;

			pos.push_back(float3(+1, +1, 1));
			pos.push_back(float3(-1, +1, 1));
			pos.push_back(float3(+1, -1, 1));
			pos.push_back(float3(-1, -1, 1));

			ElementInitData init_data;
			init_data.row_pitch = static_cast<uint32_t>(pos.size() * sizeof(pos[0]));
			init_data.slice_pitch = 0;
			init_data.data = &pos[0];
			rl_quad_->BindVertexStream(rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read, &init_data),
				boost::make_tuple(vertex_element(VEU_Position, 0, EF_BGR32F)));
		}

		technique_ = rf.LoadEffect("DeferredShading.fxml")->TechniqueByName("DeferredShadingPoint");

		technique_ambient_ = technique_->Effect().TechniqueByName("DeferredShadingAmbient");
		technique_directional_ = technique_->Effect().TechniqueByName("DeferredShadingDirectional");
		technique_point_ = technique_->Effect().TechniqueByName("DeferredShadingPoint");
		technique_spot_ = technique_->Effect().TechniqueByName("DeferredShadingSpot");
		technique_light_depth_only_ = technique_->Effect().TechniqueByName("DeferredShadingLightDepthOnly");

		conditional_render_ = rf.MakeConditionalRender();

		sm_buffer_ = rf.MakeFrameBuffer();
		sm_buffer_->Attach(FrameBuffer::ATT_DepthStencil, rf.MakeDepthStencilRenderView(SM_SIZE, SM_SIZE, EF_D16, 1, 0));
		try
		{
			sm_tex_ = rf.MakeTexture2D(SM_SIZE, SM_SIZE, 1, 1, EF_GR16F, 1, 0, EAH_GPU_Read | EAH_GPU_Write, NULL);
			sm_buffer_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*sm_tex_, 0, 0));
		}
		catch (...)
		{
			sm_tex_ = rf.MakeTexture2D(SM_SIZE, SM_SIZE, 1, 1, EF_ABGR16F, 1, 0, EAH_GPU_Read | EAH_GPU_Write, NULL);
			sm_buffer_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*sm_tex_, 0, 0));
		}

		blur_sm_tex_ = rf.MakeTexture2D(SM_SIZE, SM_SIZE, 1, 1, sm_tex_->Format(), 1, 0, EAH_GPU_Read | EAH_GPU_Write, NULL);
		blur_sm_buffer_ = rf.MakeFrameBuffer();
		blur_sm_buffer_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*blur_sm_tex_, 0, 0));

		box_filter_pp_ = MakeSharedPtr<BlurPostProcess<SeparableBoxFilterPostProcess> >(3, 1.0f);
		box_filter_pp_->Source(sm_tex_, sm_buffer_->RequiresFlipping());
		box_filter_pp_->Destinate(blur_sm_buffer_);

		*(technique_->Effect().ParameterByName("shadow_map_tex")) = blur_sm_tex_;

		texel_to_pixel_offset_param_ = technique_->Effect().ParameterByName("texel_to_pixel_offset");
		depth_near_far_invfar_param_ = technique_->Effect().ParameterByName("depth_near_far_invfar");
		upper_left_param_ = technique_->Effect().ParameterByName("upper_left");
		upper_right_param_ = technique_->Effect().ParameterByName("upper_right");
		lower_left_param_ = technique_->Effect().ParameterByName("lower_left");
		lower_right_param_ = technique_->Effect().ParameterByName("lower_right");
		inv_view_param_ = technique_->Effect().ParameterByName("inv_view");
		light_attrib_param_ = technique_->Effect().ParameterByName("light_attrib");
		light_clr_type_param_ = technique_->Effect().ParameterByName("light_clr_type");
		light_falloff_param_ = technique_->Effect().ParameterByName("light_falloff");
		light_view_proj_param_ = technique_->Effect().ParameterByName("light_view_proj");
		light_volume_mvp_param_ = technique_->Effect().ParameterByName("light_volume_mvp");
		light_pos_es_param_ = technique_->Effect().ParameterByName("light_pos_es");
		light_dir_es_param_ = technique_->Effect().ParameterByName("light_dir_es");
	}

	int DeferredShadingLayer::AddAmbientLight(float3 const & clr)
	{
		light_clr_type_[0] += float4(clr.x(), clr.y(), clr.z(), 0);
		return 0;
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
		light_cos_outer_inner_.push_back(float4(0, 0, 0, 0));
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
		light_cos_outer_inner_.push_back(float4(0, 0, 0, 0));
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
		light_cos_outer_inner_.push_back(float4(cos(outer), cos(inner), outer * 2, tan(outer)));
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
		light_cos_outer_inner_[index] = float4(cos(outer), cos(inner), outer * 2, tan(outer));
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

	void DeferredShadingLayer::OnResize(uint32_t width, uint32_t height)
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();

		RenderEngine& re = rf.RenderEngineInstance();
		g_buffer_->GetViewport().camera = re.CurFrameBuffer()->GetViewport().camera;
		shaded_buffer_->GetViewport().camera = re.CurFrameBuffer()->GetViewport().camera;

		RenderViewPtr ds_view = rf.MakeDepthStencilRenderView(width, height, EF_D16, 1, 0);

		diffuse_specular_tex_ = rf.MakeTexture2D(width, height, 1, 1, EF_ABGR16F, 1, 0, EAH_GPU_Read | EAH_GPU_Write, NULL);
		normal_depth_tex_ = rf.MakeTexture2D(width, height, 1, 1, EF_ABGR16F, 1, 0, EAH_GPU_Read | EAH_GPU_Write, NULL);
		g_buffer_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*diffuse_specular_tex_, 0, 0));
		g_buffer_->Attach(FrameBuffer::ATT_Color1, rf.Make2DRenderView(*normal_depth_tex_, 0, 0));
		g_buffer_->Attach(FrameBuffer::ATT_DepthStencil, ds_view);

		try
		{
			shaded_tex_ = rf.MakeTexture2D(width, height, 1, 1, EF_B10G11R11F, 1, 0, EAH_GPU_Read | EAH_GPU_Write, NULL);
			shaded_buffer_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*shaded_tex_, 0, 0));
		}
		catch (...)
		{
			shaded_tex_ = rf.MakeTexture2D(width, height, 1, 1, EF_ABGR16F, 1, 0, EAH_GPU_Read | EAH_GPU_Write, NULL);
			shaded_buffer_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*shaded_tex_, 0, 0));
		}
		shaded_buffer_->Attach(FrameBuffer::ATT_DepthStencil, ds_view);

		if (normal_depth_tex_)
		{
			*(technique_->Effect().ParameterByName("inv_width_height")) = float2(1.0f / width, 1.0f / height);
		}

		*(technique_->Effect().ParameterByName("nd_tex")) = normal_depth_tex_;
		*(technique_->Effect().ParameterByName("color_tex")) = diffuse_specular_tex_;
		*(technique_->Effect().ParameterByName("flipping")) = static_cast<int32_t>(g_buffer_->RequiresFlipping() ? -1 : +1);
	}

	uint32_t DeferredShadingLayer::Update(uint32_t pass)
	{
		SceneManager& scene_mgr = Context::Instance().SceneManagerInstance();
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

			float4 texel_to_pixel = re.TexelToPixelOffset();
			texel_to_pixel.x() /= shaded_buffer_->Width() / 2.0f;
			texel_to_pixel.y() /= shaded_buffer_->Height() / 2.0f;
			*texel_to_pixel_offset_param_ = texel_to_pixel;

			this->ScanLightSrc();

			SceneManager::SceneObjectsType& scene_objs = scene_mgr.SceneObjects();
			BOOST_FOREACH(BOOST_TYPEOF(scene_objs)::reference so, scene_objs)
			{
				boost::shared_ptr<DeferredableObject> deo = boost::dynamic_pointer_cast<DeferredableObject>(so);
				if (deo)
				{
					deo->GenShadowMapPass(false);
				}
			}

			re.BindFrameBuffer(g_buffer_);
			re.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, Color(0, 0, 1, 0), 1.0f, 0);
			return App3DFramework::URV_Need_Flush;
		}
		else
		{
			if ((0 == buffer_type_) || (6 == buffer_type_))
			{
				int32_t const PASS_PER_BATCH = 2;

				int32_t batch = (pass - 1) / PASS_PER_BATCH;
				int32_t pass_in_batch = (pass - 1) - batch * PASS_PER_BATCH;

				int32_t num_lights = static_cast<int32_t>(light_scaned_.size());

				if ((batch == num_lights) && (0 == pass_in_batch))
				{
					SceneManager::SceneObjectsType& scene_objs = scene_mgr.SceneObjects();
					non_emit_objs_.resize(0);
					BOOST_FOREACH(BOOST_TYPEOF(scene_objs)::reference so, scene_objs)
					{
						boost::shared_ptr<DeferredableObject> deo = boost::dynamic_pointer_cast<DeferredableObject>(so);
						if (deo)
						{
							if (deo->IsEmit())
							{
								deo->EmitPass(true);
								so->Visible(true);
							}
							else
							{
								non_emit_objs_.push_back(so);
								so->Visible(false);
							}
						}
						else
						{
							non_emit_objs_.push_back(so);
							so->Visible(false);
						}
					}

					return App3DFramework::URV_Need_Flush;
				}
				else if ((batch == num_lights) && (1 == pass_in_batch))
				{
					BOOST_FOREACH(BOOST_TYPEOF(non_emit_objs_)::reference so, non_emit_objs_)
					{
						so->Visible(true);
					}

					return App3DFramework::URV_Finished;
				}
				else
				{
					if ((0 == batch) && (0 == pass_in_batch))
					{
						SceneManager::SceneObjectsType& scene_objs = scene_mgr.SceneObjects();
						BOOST_FOREACH(BOOST_TYPEOF(scene_objs)::reference so, scene_objs)
						{
							boost::shared_ptr<DeferredableObject> deo = boost::dynamic_pointer_cast<DeferredableObject>(so);
							if (deo)
							{
								deo->GenShadowMapPass(true);
							}
						}
					}

					int32_t light_index = batch;
					int32_t org_no = light_scaned_[light_index] >> 16;
					int32_t offset = light_scaned_[light_index] & 0xFFFF;

					int type = static_cast<int>(light_clr_type_[org_no].w());

					if (0 == pass_in_batch)
					{
						if ((0 == (light_attrib_[org_no] & LSA_NoShadow)) && (type != LT_Ambient))
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
							sm_buffer_->GetViewport().camera->ViewParams(p, p + d, u);
							sm_buffer_->GetViewport().camera->ProjParams(fov, 1, 0.1f, 100.0f);

							float3 dir_es = MathLib::transform_normal(d, view_);
							float4 light_dir_es_actived = float4(dir_es.x(), dir_es.y(), dir_es.z(), offset + 0.1f);

							*light_view_proj_param_ = inv_view_ * sm_buffer_->GetViewport().camera->ViewMatrix()
								* sm_buffer_->GetViewport().camera->ProjMatrix();
							if (type != LT_Directional)
							{
								float3 loc_es = MathLib::transform_coord(p, view_);
								float4 light_pos_es_actived = float4(loc_es.x(), loc_es.y(), loc_es.z(), 1);

								if (LT_Spot == type)
								{
									light_pos_es_actived.w() = light_cos_outer_inner_[org_no].x();
									light_dir_es_actived.w() = light_cos_outer_inner_[org_no].y();
								}

								*light_pos_es_param_ = light_pos_es_actived;
							}

							*light_dir_es_param_ = light_dir_es_actived;

							if ((LT_Spot == type) || (LT_Point == type))
							{
								float4x4 mat;
								if (LT_Spot == type)
								{
									float const scale = light_cos_outer_inner_[org_no].w();
									mat = MathLib::scaling(scale, scale, 1.0f);
									rl_ = rl_cone_;
								}
								else //if (LT_Point == type)
								{
									mat = float4x4::Identity();
									rl_ = rl_pyramid_;
								}

								*light_volume_mvp_param_ = mat
									* MathLib::inverse(sm_buffer_->GetViewport().camera->ViewMatrix())
									* view_ * proj_;

								re.BindFrameBuffer(shaded_buffer_);

								conditional_render_->Begin();
								re.Render(*technique_light_depth_only_, *rl_);
								conditional_render_->End();
							}

							// Shadow map generation

							re.BindFrameBuffer(sm_buffer_);
							re.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, Color(0, 0, 0, 0), 1.0f, 0);

							checked_pointer_cast<ConditionalRender>(conditional_render_)->BeginConditionalRender();

							return App3DFramework::URV_Need_Flush;
						}
						else
						{
							return App3DFramework::URV_Flushed;
						}
					}
					else
					{
						if ((0 == (light_attrib_[org_no] & LSA_NoShadow)) && (type != LT_Ambient))
						{
							box_filter_pp_->Apply();
							checked_pointer_cast<ConditionalRender>(conditional_render_)->EndConditionalRender();
						}

						// Lighting

						re.BindFrameBuffer(shaded_buffer_);

						if ((LT_Spot == type) || (LT_Point == type))
						{
							float4x4 mat;
							if (LT_Spot == type)
							{
								technique_ = technique_spot_;
								float const scale = light_cos_outer_inner_[org_no].w();
								mat = MathLib::scaling(scale, scale, 1.0f);
								rl_ = rl_cone_;
							}
							else //if (LT_Point == type)
							{
								technique_ = technique_point_;
								mat = float4x4::Identity();
								rl_ = rl_pyramid_;
							}

							*light_volume_mvp_param_ = mat
								* MathLib::inverse(sm_buffer_->GetViewport().camera->ViewMatrix())
								* view_ * proj_;
						}
						else
						{
							if (LT_Directional == type)
							{
								technique_ = technique_directional_;
							}
							else
							{
								technique_ = technique_ambient_;
							}

							rl_ = rl_quad_;
							*light_volume_mvp_param_ = float4x4::Identity();
						}

						*light_attrib_param_ = light_attrib_[org_no];
						*light_clr_type_param_ = light_clr_type_[org_no];
						*light_falloff_param_ = light_falloff_[org_no];

						re.Render(*technique_, *rl_);

						return App3DFramework::URV_Flushed;
					}
				}
			}
			else
			{
				re.BindFrameBuffer(FrameBufferPtr());
				re.Render(*technique_, *rl_quad_);

				return App3DFramework::URV_Finished;
			}
		}
	}
}
