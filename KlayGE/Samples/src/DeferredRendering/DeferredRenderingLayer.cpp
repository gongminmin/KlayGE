#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/ResLoader.hpp>
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

#include "DeferredRenderingLayer.hpp"

namespace KlayGE
{
	int const SM_SIZE = 512;

	DeferredRenderable::DeferredRenderable(RenderEffectPtr const & effect)
		: effect_(effect)
	{
		if (effect_)
		{
			gbuffer_tech_ = effect_->TechniqueByName("GBufferTech");
			gbuffer_alpha_tech_ = effect_->TechniqueByName("GBufferAlphaTech");
			gen_sm_tech_ = effect_->TechniqueByName("GenShadowMap");
			gen_sm_alpha_tech_ = effect_->TechniqueByName("GenShadowMapAlpha");
			shading_tech_ = effect_->TechniqueByName("Shading");

			lighting_tex_param_ = effect_->ParameterByName("lighting_tex");
			ssao_tex_param_ = effect_->ParameterByName("ssao_tex");
			ssao_enabled_param_ = effect_->ParameterByName("ssao_enabled");
		}
	}

	RenderTechniquePtr const & DeferredRenderable::Pass(PassType type, bool alpha) const
	{
		switch (type)
		{
		case PT_GBuffer:
			if (alpha)
			{
				return gbuffer_alpha_tech_;
			}
			else
			{
				return gbuffer_tech_;
			}

		case PT_GenShadowMap:
			if (alpha)
			{
				return gen_sm_alpha_tech_;
			}
			else
			{
				return gen_sm_tech_;
			}

		case PT_Shading:
			return shading_tech_;

		default:
			BOOST_ASSERT(false);
			return gbuffer_tech_;
		}
	}

	void DeferredRenderable::LightingTex(TexturePtr const & tex)
	{
		*lighting_tex_param_ = tex;
	}

	void DeferredRenderable::SSAOTex(TexturePtr const & tex)
	{
		*ssao_tex_param_ = tex;
	}

	void DeferredRenderable::SSAOEnabled(bool ssao)
	{
		*ssao_enabled_param_ = static_cast<int32_t>(ssao);
	}


	void DeferredSceneObject::AttachRenderable(DeferredRenderable* dr)
	{
		dr_ = dr;
	}

	void DeferredSceneObject::LightingTex(TexturePtr const & tex)
	{
		dr_->LightingTex(tex);
	}

	void DeferredSceneObject::SSAOTex(TexturePtr const & tex)
	{
		dr_->SSAOTex(tex);
	}

	void DeferredSceneObject::SSAOEnabled(bool ssao)
	{
		dr_->SSAOEnabled(ssao);
	}


	DeferredRenderingLayer::DeferredRenderingLayer()
	{
		pass_scaned_.push_back(static_cast<uint32_t>((PT_GBuffer << 28) + 0));
		pass_scaned_.push_back(static_cast<uint32_t>((PT_GBuffer << 28) + 1));

		RenderFactory& rf = Context::Instance().RenderFactoryInstance();

		lights_.push_back(MakeSharedPtr<AmbientLightSource>());

		g_buffer_ = rf.MakeFrameBuffer();
		shadowing_buffer_ = rf.MakeFrameBuffer();
		lighting_buffer_ = rf.MakeFrameBuffer();
		shading_buffer_ = rf.MakeFrameBuffer();

		{
			rl_cone_ = rf.MakeRenderLayout();
			rl_cone_->TopologyType(RenderLayout::TT_TriangleList);

			std::vector<float3> pos;
			std::vector<uint16_t> index;
			CreateConeMesh(pos, index, 0, 100.0f, 100.0f, 12);
			cone_bbox_ = MathLib::compute_bounding_box<float>(pos.begin(), pos.end());

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
			pyramid_bbox_ = MathLib::compute_bounding_box<float>(pos.begin(), pos.end());

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
			rl_box_ = rf.MakeRenderLayout();
			rl_box_->TopologyType(RenderLayout::TT_TriangleList);

			std::vector<float3> pos;
			std::vector<uint16_t> index;
			CreateBoxMesh(pos, index, 0, 100.0f);
			box_bbox_ = MathLib::compute_bounding_box<float>(pos.begin(), pos.end());

			ElementInitData init_data;
			init_data.row_pitch = static_cast<uint32_t>(pos.size() * sizeof(pos[0]));
			init_data.slice_pitch = 0;
			init_data.data = &pos[0];
			rl_box_->BindVertexStream(rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read, &init_data),
				boost::make_tuple(vertex_element(VEU_Position, 0, EF_BGR32F)));

			init_data.row_pitch = static_cast<uint32_t>(index.size() * sizeof(index[0]));
			init_data.data = &index[0];
			rl_box_->BindIndexStream(rf.MakeIndexBuffer(BU_Static, EAH_GPU_Read, &init_data), EF_R16UI);
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

		effect_ = rf.LoadEffect("DeferredRendering.fxml");

		technique_shadows_[LT_Ambient] = effect_->TechniqueByName("DeferredShadowingAmbient");
		technique_shadows_[LT_Directional] = effect_->TechniqueByName("DeferredShadowingDirectional");
		technique_shadows_[LT_Point] = effect_->TechniqueByName("DeferredShadowingPoint");
		technique_shadows_[LT_Spot] = effect_->TechniqueByName("DeferredShadowingSpot");
		technique_lights_[LT_Ambient] = effect_->TechniqueByName("DeferredRenderingAmbient");
		technique_lights_[LT_Directional] = effect_->TechniqueByName("DeferredRenderingDirectional");
		technique_lights_[LT_Point] = effect_->TechniqueByName("DeferredRenderingPoint");
		technique_lights_[LT_Spot] = effect_->TechniqueByName("DeferredRenderingSpot");
		technique_light_depth_only_ = effect_->TechniqueByName("DeferredRenderingLightDepthOnly");
		technique_light_stencil_eiv_ = effect_->TechniqueByName("DeferredRenderingLightStencilEIV");
		technique_light_stencil_eov_ = effect_->TechniqueByName("DeferredRenderingLightStencilEOV");
		technique_clear_stencil_ = effect_->TechniqueByName("ClearStencil");

		sm_buffer_ = rf.MakeFrameBuffer();
		ElementFormat fmt;
		if (rf.RenderEngineInstance().DeviceCaps().rendertarget_format_support(EF_GR16F, 1, 0))
		{
			fmt = EF_GR16F;
		}
		else
		{
			BOOST_ASSERT(rf.RenderEngineInstance().DeviceCaps().rendertarget_format_support(EF_ABGR16F, 1, 0));

			fmt = EF_ABGR16F;
		}
		sm_tex_ = rf.MakeTexture2D(SM_SIZE, SM_SIZE, 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write, NULL);
		sm_buffer_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*sm_tex_, 0, 0));
		sm_depth_tex_ = rf.MakeTexture2D(SM_SIZE, SM_SIZE, 1, 1, EF_D24S8, 1, 0, EAH_GPU_Read | EAH_GPU_Write, NULL);
		sm_buffer_->Attach(FrameBuffer::ATT_DepthStencil, rf.Make2DDepthStencilRenderView(*sm_depth_tex_, 0, 0));

		blur_sm_tex_ = rf.MakeTexture2D(SM_SIZE, SM_SIZE, 1, 1, sm_tex_->Format(), 1, 0, EAH_GPU_Read | EAH_GPU_Write, NULL);
		sm_cube_tex_ = rf.MakeTextureCube(SM_SIZE, 1, 1, sm_tex_->Format(), 1, 0, EAH_GPU_Read | EAH_GPU_Write, NULL);

		for (int i = 0; i < 7; ++ i)
		{
			sm_filter_pps_[i] = MakeSharedPtr<BlurPostProcess<SeparableGaussianFilterPostProcess> >(8, 1.0f);
		}
		sm_filter_pps_[0]->InputPin(0, sm_tex_);
		sm_filter_pps_[0]->OutputPin(0, blur_sm_tex_);
		for (int i = 1; i < 7; ++ i)
		{
			sm_filter_pps_[i]->InputPin(0, sm_tex_);
			sm_filter_pps_[i]->OutputPin(0, sm_cube_tex_, 0, 0, i - 1);
			if (!sm_buffer_->RequiresFlipping())
			{
				switch (i - 1)
				{
				case Texture::CF_Positive_Y:
					sm_filter_pps_[i]->OutputPin(0, sm_cube_tex_, 0, 0, Texture::CF_Negative_Y);
					break;

				case Texture::CF_Negative_Y:
					sm_filter_pps_[i]->OutputPin(0, sm_cube_tex_, 0, 0, Texture::CF_Positive_Y);
					break;

				default:
					break;
				}
			}
		}
		depth_to_vsm_pp_ = LoadPostProcess(ResLoader::Instance().Load("DepthToVSM.ppml"), "DepthToVSM");
		depth_to_vsm_pp_->InputPin(0, sm_depth_tex_);
		depth_to_vsm_pp_->OutputPin(0, sm_tex_);

		*(effect_->ParameterByName("shadow_map_tex")) = blur_sm_tex_;
		*(effect_->ParameterByName("shadow_map_cube_tex")) = sm_cube_tex_;

		depth_near_far_invfar_param_ = effect_->ParameterByName("depth_near_far_invfar");
		inv_view_param_ = effect_->ParameterByName("inv_view");
		light_attrib_param_ = effect_->ParameterByName("light_attrib");
		light_color_param_ = effect_->ParameterByName("light_color");
		light_falloff_param_ = effect_->ParameterByName("light_falloff");
		light_view_proj_param_ = effect_->ParameterByName("light_view_proj");
		light_volume_mv_param_ = effect_->ParameterByName("light_volume_mv");
		light_volume_mvp_param_ = effect_->ParameterByName("light_volume_mvp");
		light_pos_es_param_ = effect_->ParameterByName("light_pos_es");
		light_dir_es_param_ = effect_->ParameterByName("light_dir_es");
	}

	AmbientLightSourcePtr DeferredRenderingLayer::AddAmbientLight(float3 const & clr)
	{
		AmbientLightSourcePtr ambient = checked_pointer_cast<AmbientLightSource>(lights_[0]);
		ambient->Color(float3(ambient->Color()) + clr);
		return ambient;
	}

	PointLightSourcePtr DeferredRenderingLayer::AddPointLight(int32_t attr, float3 const & pos, float3 const & clr, float3 const & falloff)
	{
		PointLightSourcePtr point = MakeSharedPtr<PointLightSource>();
		point->Attrib(attr);
		point->Color(clr);
		point->Position(pos);
		point->Falloff(falloff);
		lights_.push_back(point);

		return point;
	}

	DirectionalLightSourcePtr DeferredRenderingLayer::AddDirectionalLight(int32_t attr, float3 const & dir, float3 const & clr, float3 const & falloff)
	{
		DirectionalLightSourcePtr directional = MakeSharedPtr<DirectionalLightSource>();
		directional->Attrib(attr);
		directional->Color(clr);
		directional->Direction(MathLib::normalize(dir));
		directional->Falloff(falloff);
		lights_.push_back(directional);
		return directional;
	}

	SpotLightSourcePtr DeferredRenderingLayer::AddSpotLight(int32_t attr, float3 const & pos, float3 const & dir, float outer, float inner, float3 const & clr, float3 const & falloff)
	{
		SpotLightSourcePtr spot = MakeSharedPtr<SpotLightSource>();
		spot->Attrib(attr);
		spot->Color(clr);
		spot->Position(pos);
		spot->Direction(MathLib::normalize(dir));
		spot->Falloff(falloff);
		spot->OuterAngle(outer);
		spot->InnerAngle(inner);
		lights_.push_back(spot);
		return spot;
	}

	void DeferredRenderingLayer::SSAOTex(TexturePtr const & tex)
	{
		ssao_tex_ = tex;
	}

	void DeferredRenderingLayer::SSAOEnabled(bool ssao)
	{
		ssao_enabled_ = ssao;
	}

	void DeferredRenderingLayer::OnResize(uint32_t width, uint32_t height)
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();

		RenderEngine& re = rf.RenderEngineInstance();
		g_buffer_->GetViewport().camera = re.CurFrameBuffer()->GetViewport().camera;
		shadowing_buffer_->GetViewport().camera = re.CurFrameBuffer()->GetViewport().camera;
		lighting_buffer_->GetViewport().camera = re.CurFrameBuffer()->GetViewport().camera;
		shading_buffer_->GetViewport().camera = re.CurFrameBuffer()->GetViewport().camera;

		RenderViewPtr ds_view = rf.Make2DDepthStencilRenderView(width, height, EF_D24S8, 1, 0);

		g_buffer_tex_ = rf.MakeTexture2D(width, height, 2, 1, EF_ABGR16F, 1, 0, EAH_GPU_Read | EAH_GPU_Write | EAH_Generate_Mips, NULL);
		g_buffer_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*g_buffer_tex_, 0, 0));
		g_buffer_->Attach(FrameBuffer::ATT_DepthStencil, ds_view);

		ElementFormat fmt;
		if (rf.RenderEngineInstance().DeviceCaps().rendertarget_format_support(EF_R16F, 1, 0))
		{
			fmt = EF_R16F;
		}
		else
		{
			BOOST_ASSERT(rf.RenderEngineInstance().DeviceCaps().rendertarget_format_support(EF_ABGR16F, 1, 0));

			fmt = EF_ABGR16F;
		}
		shadowing_tex_ = rf.MakeTexture2D(width / 2, height / 2, 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write, NULL);
		shadowing_buffer_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*shadowing_tex_, 0, 0));

		lighting_tex_ = rf.MakeTexture2D(width, height, 1, 1, EF_ABGR16F, 1, 0, EAH_GPU_Read | EAH_GPU_Write, NULL);
		lighting_buffer_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*lighting_tex_, 0, 0));
		lighting_buffer_->Attach(FrameBuffer::ATT_DepthStencil, ds_view);

		if (rf.RenderEngineInstance().DeviceCaps().rendertarget_format_support(EF_B10G11R11F, 1, 0))
		{
			fmt = EF_B10G11R11F;
		}
		else
		{
			BOOST_ASSERT(rf.RenderEngineInstance().DeviceCaps().rendertarget_format_support(EF_ABGR16F, 1, 0));

			fmt = EF_ABGR16F;
		}
		shading_tex_ = rf.MakeTexture2D(width, height, 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write, NULL);
		shading_buffer_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*shading_tex_, 0, 0));
		shading_buffer_->Attach(FrameBuffer::ATT_DepthStencil, ds_view);

		if (g_buffer_tex_)
		{
			*(effect_->ParameterByName("inv_width_height")) = float2(1.0f / width, 1.0f / height);
		}

		*(effect_->ParameterByName("g_buffer_tex")) = g_buffer_tex_;
		*(effect_->ParameterByName("shadowing_tex")) = shadowing_tex_;
		*(effect_->ParameterByName("flipping")) = static_cast<int32_t>(g_buffer_->RequiresFlipping() ? -1 : +1);
	}

	uint32_t DeferredRenderingLayer::Update(uint32_t pass)
	{
		SceneManager& scene_mgr = Context::Instance().SceneManagerInstance();
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();

		int32_t pass_type = pass_scaned_[pass] >> 28;
		int32_t org_no = (pass_scaned_[pass] >> 16) & 0xFFF;
		int32_t index_in_pass = pass_scaned_[pass] & 0xFFFF;

		if (0 == pass)
		{
			deferred_scene_objs_.resize(0);
			SceneManager::SceneObjectsType& scene_objs = scene_mgr.SceneObjects();
			BOOST_FOREACH(BOOST_TYPEOF(scene_objs)::reference so, scene_objs)
			{
				if (so->Attrib() & SOA_Deferred)
				{
					DeferredSceneObject* deo = dynamic_cast<DeferredSceneObject*>(so.get());

					deferred_scene_objs_.push_back(deo);

					deo->LightingTex(lighting_tex_);
					deo->SSAOTex(ssao_tex_);
					deo->SSAOEnabled(ssao_enabled_);
				}
			}
		}

		if (pass_type != PT_Lighting)
		{
			BOOST_FOREACH(BOOST_TYPEOF(deferred_scene_objs_)::reference deo, deferred_scene_objs_)
			{
				deo->Pass(static_cast<PassType>(pass_type));
			}
		}

		if (PT_GBuffer == pass_type)
		{
			if (0 == index_in_pass)
			{
				// Generate G-Buffer

				Camera const & camera = Context::Instance().AppInstance().ActiveCamera();

				view_ = camera.ViewMatrix();
				proj_ = camera.ProjMatrix();
				inv_view_ = MathLib::inverse(view_);
				inv_proj_ = MathLib::inverse(proj_);

				*depth_near_far_invfar_param_ = float3(camera.NearPlane(), camera.FarPlane(), 1 / camera.FarPlane());
				*inv_view_param_ = inv_view_;

				re.BindFrameBuffer(g_buffer_);
				re.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth | FrameBuffer::CBM_Stencil, Color(0, 0, 1, 0), 1.0f, 0);
				return App3DFramework::URV_Need_Flush;
			}
			else
			{
				g_buffer_tex_->BuildMipSubLevels();

				// Light depth only

				float4x4 vp = view_ * proj_;

				re.BindFrameBuffer(lighting_buffer_);

				pass_scaned_.resize(2);
				for (size_t i = 0; i < lights_.size(); ++ i)
				{
					LightSourcePtr const & light = lights_[i];
					if (light->Enabled())
					{
						int type = light->Type();
						int32_t attr = light->Attrib();
						switch (type)
						{
						case LT_Spot:
							{
								float4x4 const & light_view = light->SMCamera(0)->ViewMatrix();

								float const scale = light->CosOuterInner().w();
								float4x4 mat = MathLib::scaling(scale, scale, 1.0f);
								float4x4 light_model = mat * MathLib::inverse(light_view);
								*light_volume_mv_param_ = light_model * view_;
								*light_volume_mvp_param_ = light_model * vp;

								float3 min, max;
								min = max = MathLib::transform_coord(cone_bbox_[0], light_model);
								for (size_t k = 1; k < 8; ++ k)
								{
									float3 vec = MathLib::transform_coord(cone_bbox_[k], light_model);
									min = MathLib::minimize(min, vec);
									max = MathLib::maximize(max, vec);
								}

								//if (scene_mgr.AABBVisible(Box(min, max)))
								{
									if (0 == (attr & LSA_NoShadow))
									{
										pass_scaned_.push_back(static_cast<uint32_t>((PT_GenShadowMap << 28) + (i << 16) + 0));
									}
									pass_scaned_.push_back(static_cast<uint32_t>((PT_Lighting << 28) + (i << 16) + 0));

									light->ConditionalRenderQuery(0)->Begin();
									re.Render(*technique_light_depth_only_, *rl_cone_);
									light->ConditionalRenderQuery(0)->End();
								}
							}
							break;

						case LT_Point:
							{
								float3 const & p = light->Position();
								for (int j = 0; j < 6; ++ j)
								{
									float4x4 const & light_view = light->SMCamera(j)->ViewMatrix();
									float4x4 light_model = MathLib::inverse(light_view);
									*light_volume_mv_param_ = light_model * view_;
									*light_volume_mvp_param_ = light_model * vp;

									float3 min, max;
									min = max = MathLib::transform_coord(pyramid_bbox_[0], light_model);
									for (size_t k = 1; k < 8; ++ k)
									{
										float3 vec = MathLib::transform_coord(pyramid_bbox_[k], light_model);
										min = MathLib::minimize(min, vec);
										max = MathLib::maximize(max, vec);
									}

									//if (scene_mgr.AABBVisible(Box(min, max)))
									{
										if (0 == (attr & LSA_NoShadow))
										{
											pass_scaned_.push_back(static_cast<uint32_t>((PT_GenShadowMap << 28) + (i << 16) + j));
										}

										light->ConditionalRenderQuery(j)->Begin();
										re.Render(*technique_light_depth_only_, *rl_pyramid_);
										light->ConditionalRenderQuery(j)->End();
									}
								}
								{
									float4x4 light_model = MathLib::translation(p);
									*light_volume_mv_param_ = light_model * view_;
									*light_volume_mvp_param_ = light_model * vp;

									float3 min, max;
									min = max = MathLib::transform_coord(box_bbox_[0], light_model);
									for (size_t k = 1; k < 8; ++ k)
									{
										float3 vec = MathLib::transform_coord(box_bbox_[k], light_model);
										min = MathLib::minimize(min, vec);
										max = MathLib::maximize(max, vec);
									}

									//if (scene_mgr.AABBVisible(Box(min, max)))
									{
										pass_scaned_.push_back(static_cast<uint32_t>((PT_Lighting << 28) + (i << 16) + 6));

										light->ConditionalRenderQuery(6)->Begin();
										re.Render(*technique_light_depth_only_, *rl_box_);
										light->ConditionalRenderQuery(6)->End();
									}
								}
							}
							break;

						default:
							if (0 == (attr & LSA_NoShadow))
							{
								pass_scaned_.push_back(static_cast<uint32_t>((PT_GenShadowMap << 28) + (i << 16) + 0));
							}
							pass_scaned_.push_back(static_cast<uint32_t>((PT_Lighting << 28) + (i << 16) + 0));
							break;
						}
					}
				}
				pass_scaned_.push_back(static_cast<uint32_t>((PT_Shading << 28) + 0));
				pass_scaned_.push_back(static_cast<uint32_t>((PT_Shading << 28) + 1));

				return App3DFramework::URV_Flushed;
			}
		}
		else
		{
			if (PT_Shading == pass_type)
			{
				if (0 == index_in_pass)
				{
					re.BindFrameBuffer(shading_buffer_);
					return App3DFramework::URV_Need_Flush;
				}
				else
				{
					return App3DFramework::URV_Finished;
				}
			}
			else
			{
				LightSourcePtr const & light = lights_[org_no];

				LightType type = light->Type();
				int32_t attr = light->Attrib();

				RenderLayoutPtr rl;
				switch (type)
				{
				case LT_Point:
				case LT_Spot:
					{
						CameraPtr sm_camera;
						float3 dir_es;
						if (LT_Spot == type)
						{
							dir_es = MathLib::transform_normal(light->Direction(), view_);
							if (0 == (attr & LSA_NoShadow))
							{
								sm_camera = light->SMCamera(0);
							}
						}
						else
						{
							if (index_in_pass < 6)
							{
								std::pair<float3, float3> ad = CubeMapViewVector<float>(static_cast<Texture::CubeFaces>(index_in_pass));
								dir_es = MathLib::transform_normal(ad.first, view_);
								if (0 == (attr & LSA_NoShadow))
								{
									sm_camera = light->SMCamera(index_in_pass);
								}
							}
						}
						float4 light_dir_es_actived = float4(dir_es.x(), dir_es.y(), dir_es.z(), 0);

						float4x4 mat_v, mat_vp;
						if (sm_camera)
						{
							sm_buffer_->GetViewport().camera = sm_camera;

							*light_view_proj_param_ = inv_view_ * sm_camera->ViewMatrix() * sm_camera->ProjMatrix();

							mat_v = MathLib::inverse(sm_camera->ViewMatrix()) * view_;
							mat_vp = mat_v * proj_;
						}

						if (PT_GenShadowMap == pass_type)
						{
							float4x4 inv_sm_proj = MathLib::inverse(sm_camera->ProjMatrix());
							float q = sm_camera->FarPlane() / (sm_camera->FarPlane() - sm_camera->NearPlane());
							depth_to_vsm_pp_->SetParam(0, float2(sm_camera->NearPlane() * q, q));
							depth_to_vsm_pp_->SetParam(1, inv_sm_proj);
						}

						float3 const & p = light->Position();
						float3 loc_es = MathLib::transform_coord(p, view_);
						float4 light_pos_es_actived = float4(loc_es.x(), loc_es.y(), loc_es.z(), 1);

						switch (type)
						{
						case LT_Spot:
							{
								light_pos_es_actived.w() = light->CosOuterInner().x();
								light_dir_es_actived.w() = light->CosOuterInner().y();

								rl = rl_cone_;
								float const scale = light->CosOuterInner().w();
								float4x4 light_model = MathLib::scaling(scale, scale, 1.0f);
								*light_volume_mv_param_ = light_model * mat_v;
								*light_volume_mvp_param_ = light_model * mat_vp;
							}
							break;

						case LT_Point:
							if (PT_Lighting == pass_type)
							{
								rl = rl_box_;
								float4x4 light_model = MathLib::translation(p);
								*light_volume_mv_param_ = light_model * view_;
								*light_volume_mvp_param_ = light_model * view_ * proj_;
							}
							else
							{
								rl = rl_pyramid_;
								*light_volume_mv_param_ = mat_v;
								*light_volume_mvp_param_ = mat_vp;
							}
							break;

						default:
							BOOST_ASSERT(false);
							break;
						}
					
						*light_pos_es_param_ = light_pos_es_actived;
						*light_dir_es_param_ = light_dir_es_actived;
					}
					break;

				case LT_Directional:
					{
						float3 dir_es = MathLib::transform_normal(light->Direction(), view_);
						*light_dir_es_param_ = float4(dir_es.x(), dir_es.y(), dir_es.z(), 0);
					}
					rl = rl_quad_;
					*light_volume_mv_param_ = inv_proj_;
					*light_volume_mvp_param_ = float4x4::Identity();
					break;

				case LT_Ambient:
					rl = rl_quad_;
					*light_volume_mv_param_ = inv_proj_;
					*light_volume_mvp_param_ = float4x4::Identity();
					break;

				default:
					BOOST_ASSERT(false);
					break;
				}

				if ((index_in_pass > 0) || (PT_Lighting == pass_type))
				{
					if (0 == (attr & LSA_NoShadow))
					{
						depth_to_vsm_pp_->Apply();

						if (LT_Point == type)
						{
							sm_filter_pps_[index_in_pass]->Apply();
							light->ConditionalRenderQuery(index_in_pass - 1)->EndConditionalRender();
						}
						else
						{
							sm_filter_pps_[0]->Apply();
							light->ConditionalRenderQuery(index_in_pass)->EndConditionalRender();
						}
					}
				}

				if (PT_GenShadowMap == pass_type)
				{
					light->ConditionalRenderQuery(index_in_pass)->BeginConditionalRender();

					// Shadow map generation

					re.BindFrameBuffer(sm_buffer_);
					re.CurFrameBuffer()->Attached(FrameBuffer::ATT_DepthStencil)->ClearDepth(1.0f);

					return App3DFramework::URV_Need_Flush;
				}
				else //if (PT_Lighting == pass_type)
				{
					// Shadowing

					re.BindFrameBuffer(shadowing_buffer_);
					re.CurFrameBuffer()->Attached(FrameBuffer::ATT_Color0)->ClearColor(Color(1, 1, 1, 1));

					*light_attrib_param_ = attr;
					*light_color_param_ = light->Color();
					*light_falloff_param_ = light->Falloff();

					if ((type != LT_Ambient) && (type != LT_Directional))
					{
						light->ConditionalRenderQuery(index_in_pass)->BeginConditionalRender();
					}

					re.Render(*technique_shadows_[type], *rl);

					if ((type != LT_Ambient) && (type != LT_Directional))
					{
						light->ConditionalRenderQuery(index_in_pass)->EndConditionalRender();
					}


					// Lighting

					re.BindFrameBuffer(lighting_buffer_);
					// Clear stencil to 0 with write mask
					re.Render(*technique_clear_stencil_, *rl_quad_);

					if ((type != LT_Ambient) && (type != LT_Directional))
					{
						light->ConditionalRenderQuery(index_in_pass)->BeginConditionalRender();
					}

					if ((LT_Point == type) || (LT_Spot == type))
					{
						Camera const & camera = Context::Instance().AppInstance().ActiveCamera();
						float3 eye = camera.EyePos();

						bool eye_in_volume = false;

						float3 const & p = light->Position();
						if (LT_Spot == type)
						{
							float3 v = MathLib::normalize(eye - p);
							float3 const & d = light->Direction();
							float cos_direction = MathLib::dot(v, d);
							if (light->CosOuterInner().x() < cos_direction * 1.01f)
							{
								Plane plane = MathLib::from_point_normal(p, d);
								float dist = MathLib::dot_coord(plane, eye);
								if (dist < 100.0f)
								{
									eye_in_volume = true;
								}
							}
						}
						else
						{
							float3 v = eye - p;
							if ((abs(v.x()) < 100.0f) && (abs(v.y()) < 100.0f) && (abs(v.z()) < 100.0f))
							{
								eye_in_volume = true;
							}
						}

						if (eye_in_volume)
						{
							re.Render(*technique_light_stencil_eiv_, *rl);
						}
						else
						{
							re.Render(*technique_light_stencil_eov_, *rl);
						}
					}

					re.Render(*technique_lights_[type], *rl);

					if ((type != LT_Ambient) && (type != LT_Directional))
					{
						light->ConditionalRenderQuery(index_in_pass)->EndConditionalRender();
					}

					return App3DFramework::URV_Flushed;
				}
			}
		}
	}
}
