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

#include "DeferredShadingLayer.hpp"

namespace KlayGE
{
	int const SM_SIZE = 512;

	DeferredLightSource::DeferredLightSource(LightType type)
		: type_(type), attrib_(0), enabled_(true), 
			color_(0, 0, 0)
	{
	}

	DeferredLightSource::~DeferredLightSource()
	{
	}

	LightType DeferredLightSource::Type() const
	{
		return type_;
	}

	int32_t DeferredLightSource::Attrib() const
	{
		return attrib_;
	}

	void DeferredLightSource::Attrib(int32_t attrib)
	{
		attrib_ = attrib;
	}

	bool DeferredLightSource::Enabled() const
	{
		return enabled_;
	}

	void DeferredLightSource::Enabled(bool enabled)
	{
		enabled_ = enabled;
	}

	float3 const & DeferredLightSource::Color() const
	{
		return color_;
	}

	void DeferredLightSource::Color(float3 const & clr)
	{
		color_ = clr;
	}

	float3 const & DeferredLightSource::Position() const
	{
		return float3::Zero();
	}

	float3 const & DeferredLightSource::Direction() const
	{
		return float3::Zero();
	}

	float3 const & DeferredLightSource::Falloff() const
	{
		return float3::Zero();
	}

	float DeferredLightSource::CosInnerAngle() const
	{
		return 0;
	}

	float DeferredLightSource::CosOuterAngle() const
	{
		return 0;
	}

	float4 const & DeferredLightSource::CosOuterInner() const
	{
		return float4::Zero();
	}


	DeferredAmbientLightSource::DeferredAmbientLightSource()
		: DeferredLightSource(LT_Ambient)
	{
		attrib_ = LSA_NoShadow;
		color_ = float3(0, 0, 0);
	}

	DeferredAmbientLightSource::~DeferredAmbientLightSource()
	{
	}


	DeferredPointLightSource::DeferredPointLightSource()
		: DeferredLightSource(LT_Point)
	{
	}

	DeferredPointLightSource::~DeferredPointLightSource()
	{
	}

	float3 const & DeferredPointLightSource::Position() const
	{
		return pos_;
	}

	void DeferredPointLightSource::Position(float3 const & pos)
	{
		pos_ = pos;
	}

	float3 const & DeferredPointLightSource::Falloff() const
	{
		return falloff_;
	}

	void DeferredPointLightSource::Falloff(float3 const & falloff)
	{
		falloff_ = falloff;
	}


	DeferredSpotLightSource::DeferredSpotLightSource()
		: DeferredLightSource(LT_Spot)
	{
	}

	DeferredSpotLightSource::~DeferredSpotLightSource()
	{
	}

	float3 const & DeferredSpotLightSource::Position() const
	{
		return pos_;
	}

	void DeferredSpotLightSource::Position(float3 const & pos)
	{
		pos_ = pos;
	}

	float3 const & DeferredSpotLightSource::Direction() const
	{
		return dir_;
	}

	void DeferredSpotLightSource::Direction(float3 const & dir)
	{
		dir_ = dir;
	}

	float3 const & DeferredSpotLightSource::Falloff() const
	{
		return falloff_;
	}

	void DeferredSpotLightSource::Falloff(float3 const & falloff)
	{
		falloff_ = falloff;
	}

	float DeferredSpotLightSource::CosInnerAngle() const
	{
		return cos_outer_inner_.y();
	}

	void DeferredSpotLightSource::InnerAngle(float angle)
	{
		cos_outer_inner_.y() = cos(angle);
	}

	float DeferredSpotLightSource::CosOuterAngle() const
	{
		return cos_outer_inner_.x();
	}

	void DeferredSpotLightSource::OuterAngle(float angle)
	{
		cos_outer_inner_.x() = cos(angle);
		cos_outer_inner_.z() = angle * 2;
		cos_outer_inner_.w() = tan(angle);
	}

	float4 const & DeferredSpotLightSource::CosOuterInner() const
	{
		return cos_outer_inner_;
	}


	DeferredDirectionalLightSource::DeferredDirectionalLightSource()
		: DeferredLightSource(LT_Directional)
	{
		attrib_ = LSA_NoShadow;
	}

	DeferredDirectionalLightSource::~DeferredDirectionalLightSource()
	{
	}

	float3 const & DeferredDirectionalLightSource::Direction() const
	{
		return dir_;
	}

	void DeferredDirectionalLightSource::Direction(float3 const & dir)
	{
		dir_ = dir;
	}

	float3 const & DeferredDirectionalLightSource::Falloff() const
	{
		return falloff_;
	}

	void DeferredDirectionalLightSource::Falloff(float3 const & falloff)
	{
		falloff_ = falloff;
	}


	DeferredRenderable::DeferredRenderable(RenderEffectPtr const & effect)
		: effect_(effect)
	{
		if (effect_)
		{
			gbuffer_technique_ = effect_->TechniqueByName("GBufferTech");
			gen_sm_technique_ = effect_->TechniqueByName("GenShadowMap");
			shading_technique_ = effect_->TechniqueByName("Shading");
		}
	}

	RenderTechniquePtr const & DeferredRenderable::Pass(PassType type) const
	{
		switch (type)
		{
		case PT_GBuffer:
			return gbuffer_technique_;

		case PT_GenShadowMap:
			return gen_sm_technique_;

		case PT_Shading:
			return shading_technique_;

		default:
			BOOST_ASSERT(false);
			return gbuffer_technique_;
		}
	}

	void DeferredRenderable::LightingTex(TexturePtr const & tex)
	{
		*(effect_->ParameterByName("lighting_tex")) = tex;
	}

	void DeferredRenderable::SSAOTex(TexturePtr const & tex)
	{
		*(effect_->ParameterByName("ssao_tex")) = tex;
	}

	void DeferredRenderable::SSAOEnabled(bool ssao)
	{
		*(effect_->ParameterByName("ssao_enabled")) = ssao;
	}


	DeferredShadingLayer::DeferredShadingLayer()
			: RenderableHelper(L"DeferredShadingLayer")
	{
		pass_scaned_.push_back(static_cast<uint32_t>((PT_GBuffer << 28) + 0));
		pass_scaned_.push_back(static_cast<uint32_t>((PT_GBuffer << 28) + 1));
		
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();

		lights_.push_back(MakeSharedPtr<DeferredAmbientLightSource>());
		light_crs_.push_back(std::vector<ConditionalRenderPtr>());

		g_buffer_ = rf.MakeFrameBuffer();
		lighting_buffer_ = rf.MakeFrameBuffer();
		shading_buffer_ = rf.MakeFrameBuffer();

		box_ = Box(float3(-1, -1, -1), float3(1, 1, 1));

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

		RenderEffectPtr effect = rf.LoadEffect("DeferredShading.fxml");

		technique_lights_[LT_Ambient] = effect->TechniqueByName("DeferredShadingAmbient");
		technique_lights_[LT_Directional] = effect->TechniqueByName("DeferredShadingDirectional");
		technique_lights_[LT_Point] = effect->TechniqueByName("DeferredShadingPoint");
		technique_lights_[LT_Spot] = effect->TechniqueByName("DeferredShadingSpot");
		technique_light_depth_only_ = effect->TechniqueByName("DeferredShadingLightDepthOnly");
		technique_light_stencil_eiv_ = effect->TechniqueByName("DeferredShadingLightStencilEIV");
		technique_light_stencil_eov_ = effect->TechniqueByName("DeferredShadingLightStencilEOV");

		technique_ = technique_light_depth_only_;

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
		sm_cube_tex_ = rf.MakeTextureCube(SM_SIZE, 1, 1, sm_tex_->Format(), 1, 0, EAH_GPU_Read | EAH_GPU_Write, NULL);

		for (int i = 0; i < 7; ++ i)
		{
			sm_filter_pps_[i] = MakeSharedPtr<BlurPostProcess<SeparableGaussianFilterPostProcess> >(3, 1.0f);
		}
		sm_filter_pps_[0]->InputPin(0, sm_tex_);
		sm_filter_pps_[0]->OutputPin(0, blur_sm_tex_);
		for (int i = 1; i < 7; ++ i)
		{
			sm_filter_pps_[i]->InputPin(0, sm_tex_);
			sm_filter_pps_[i]->OutputPin(0, sm_cube_tex_, 0, 0, i - 1);
		}

		*(technique_->Effect().ParameterByName("shadow_map_tex")) = blur_sm_tex_;
		*(technique_->Effect().ParameterByName("shadow_map_cube_tex")) = sm_cube_tex_;

		texel_to_pixel_offset_param_ = technique_->Effect().ParameterByName("texel_to_pixel_offset");
		depth_near_far_invfar_param_ = technique_->Effect().ParameterByName("depth_near_far_invfar");
		upper_left_param_ = technique_->Effect().ParameterByName("upper_left");
		upper_right_param_ = technique_->Effect().ParameterByName("upper_right");
		lower_left_param_ = technique_->Effect().ParameterByName("lower_left");
		lower_right_param_ = technique_->Effect().ParameterByName("lower_right");
		inv_view_param_ = technique_->Effect().ParameterByName("inv_view");
		light_attrib_param_ = technique_->Effect().ParameterByName("light_attrib");
		light_color_param_ = technique_->Effect().ParameterByName("light_color");
		light_falloff_param_ = technique_->Effect().ParameterByName("light_falloff");
		light_view_proj_param_ = technique_->Effect().ParameterByName("light_view_proj");
		light_volume_mvp_param_ = technique_->Effect().ParameterByName("light_volume_mvp");
		light_pos_es_param_ = technique_->Effect().ParameterByName("light_pos_es");
		light_dir_es_param_ = technique_->Effect().ParameterByName("light_dir_es");
	}

	DeferredAmbientLightSourcePtr DeferredShadingLayer::AddAmbientLight(float3 const & clr)
	{
		DeferredAmbientLightSourcePtr ambient = checked_pointer_cast<DeferredAmbientLightSource>(lights_[0]);
		ambient->Color(ambient->Color() + clr);
		return ambient;
	}

	DeferredPointLightSourcePtr DeferredShadingLayer::AddPointLight(int32_t attr, float3 const & pos, float3 const & clr, float3 const & falloff)
	{
		DeferredPointLightSourcePtr point = MakeSharedPtr<DeferredPointLightSource>();
		point->Attrib(attr);
		point->Color(clr);
		point->Position(pos);
		point->Falloff(falloff);
		lights_.push_back(point);

		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		light_crs_.push_back(std::vector<ConditionalRenderPtr>());
		for (int i = 0; i < 7; ++ i)
		{
			light_crs_.back().push_back(checked_pointer_cast<ConditionalRender>(rf.MakeConditionalRender()));
		}

		return point;
	}

	DeferredDirectionalLightSourcePtr DeferredShadingLayer::AddDirectionalLight(int32_t attr, float3 const & dir, float3 const & clr, float3 const & falloff)
	{
		DeferredDirectionalLightSourcePtr directional = MakeSharedPtr<DeferredDirectionalLightSource>();
		directional->Attrib(attr);
		directional->Color(clr);
		directional->Direction(MathLib::normalize(dir));
		directional->Falloff(falloff);
		lights_.push_back(directional);
		light_crs_.push_back(std::vector<ConditionalRenderPtr>());
		return directional;
	}

	DeferredSpotLightSourcePtr DeferredShadingLayer::AddSpotLight(int32_t attr, float3 const & pos, float3 const & dir, float outer, float inner, float3 const & clr, float3 const & falloff)
	{
		DeferredSpotLightSourcePtr spot = MakeSharedPtr<DeferredSpotLightSource>();
		spot->Attrib(attr);
		spot->Color(clr);
		spot->Position(pos);
		spot->Direction(MathLib::normalize(dir));
		spot->Falloff(falloff);
		spot->OuterAngle(outer);
		spot->InnerAngle(inner);
		lights_.push_back(spot);

		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		light_crs_.push_back(std::vector<ConditionalRenderPtr>(1,
			checked_pointer_cast<ConditionalRender>(rf.MakeConditionalRender())));

		return spot;
	}

	void DeferredShadingLayer::SSAOTex(TexturePtr const & tex)
	{
		ssao_tex_ = tex;
	}

	void DeferredShadingLayer::SSAOEnabled(bool ssao)
	{
		ssao_enabled_ = ssao;
	}

	void DeferredShadingLayer::OnResize(uint32_t width, uint32_t height)
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();

		RenderEngine& re = rf.RenderEngineInstance();
		g_buffer_->GetViewport().camera = re.CurFrameBuffer()->GetViewport().camera;
		lighting_buffer_->GetViewport().camera = re.CurFrameBuffer()->GetViewport().camera;
		shading_buffer_->GetViewport().camera = re.CurFrameBuffer()->GetViewport().camera;

		RenderViewPtr ds_view = rf.MakeDepthStencilRenderView(width, height, EF_D24S8, 1, 0);

		normal_depth_tex_ = rf.MakeTexture2D(width, height, 1, 1, EF_ABGR16F, 1, 0, EAH_GPU_Read | EAH_GPU_Write, NULL);
		g_buffer_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*normal_depth_tex_, 0, 0));
		g_buffer_->Attach(FrameBuffer::ATT_DepthStencil, ds_view);

		lighting_tex_ = rf.MakeTexture2D(width, height, 1, 1, EF_ABGR16F, 1, 0, EAH_GPU_Read | EAH_GPU_Write, NULL);
		lighting_buffer_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*lighting_tex_, 0, 0));
		lighting_buffer_->Attach(FrameBuffer::ATT_DepthStencil, ds_view);

		try
		{
			shading_tex_ = rf.MakeTexture2D(width, height, 1, 1, EF_B10G11R11F, 1, 0, EAH_GPU_Read | EAH_GPU_Write, NULL);
			shading_buffer_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*shading_tex_, 0, 0));
		}
		catch (...)
		{
			shading_tex_ = rf.MakeTexture2D(width, height, 1, 1, EF_ABGR16F, 1, 0, EAH_GPU_Read | EAH_GPU_Write, NULL);
			shading_buffer_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*shading_tex_, 0, 0));
		}
		shading_buffer_->Attach(FrameBuffer::ATT_DepthStencil, ds_view);

		if (normal_depth_tex_)
		{
			*(technique_->Effect().ParameterByName("inv_width_height")) = float2(1.0f / width, 1.0f / height);
		}

		*(technique_->Effect().ParameterByName("nd_tex")) = normal_depth_tex_;
		*(technique_->Effect().ParameterByName("flipping")) = static_cast<int32_t>(g_buffer_->RequiresFlipping() ? -1 : +1);
	}

	uint32_t DeferredShadingLayer::Update(uint32_t pass)
	{
		SceneManager& scene_mgr = Context::Instance().SceneManagerInstance();
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();

		int32_t pass_type = pass_scaned_[pass] >> 28;
		int32_t org_no = (pass_scaned_[pass] >> 16) & 0xFFF;
		int32_t index_in_pass = pass_scaned_[pass] & 0xFFFF;

		SceneManager::SceneObjectsType& scene_objs = scene_mgr.SceneObjects();
		BOOST_FOREACH(BOOST_TYPEOF(scene_objs)::reference so, scene_objs)
		{
			DeferredSceneObjectPtr deo = boost::dynamic_pointer_cast<DeferredSceneObject>(so);
			if (deo)
			{
				if (pass_type != PT_Lighting)
				{
					deo->Pass(static_cast<PassType>(pass_type));
				}
				if (0 == pass)
				{
					deo->LightingTex(lighting_tex_);
					deo->SSAOTex(ssao_tex_);
					deo->SSAOEnabled(ssao_enabled_);
				}
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
				float4x4 const inv_proj = MathLib::inverse(proj_);

				*depth_near_far_invfar_param_ = float3(camera.NearPlane(), camera.FarPlane(), 1 / camera.FarPlane());

				*upper_left_param_ = MathLib::transform_coord(float3(-1, 1, 1), inv_proj);
				*upper_right_param_ = MathLib::transform_coord(float3(1, 1, 1), inv_proj);
				*lower_left_param_ = MathLib::transform_coord(float3(-1, -1, 1), inv_proj);
				*lower_right_param_ = MathLib::transform_coord(float3(1, -1, 1), inv_proj);

				*inv_view_param_ = inv_view_;

				float4 texel_to_pixel = re.TexelToPixelOffset();
				texel_to_pixel.x() /= g_buffer_->Width() / 2.0f;
				texel_to_pixel.y() /= g_buffer_->Height() / 2.0f;
				*texel_to_pixel_offset_param_ = texel_to_pixel;

				re.BindFrameBuffer(g_buffer_);
				re.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth | FrameBuffer::CBM_Stencil, Color(0, 0, 1, 0), 1.0f, 0);
				return App3DFramework::URV_Need_Flush;
			}
			else
			{
				// Light depth only

				float4x4 vp = view_ * proj_;

				re.BindFrameBuffer(lighting_buffer_);

				pass_scaned_.resize(2);
				for (size_t i = 0; i < lights_.size(); ++ i)
				{
					DeferredLightSourcePtr const & light = lights_[i];
					if (light->Enabled())
					{
						int type = light->Type();
						int32_t attr = light->Attrib();
						switch (type)
						{
						case LT_Spot:
							{
								float3 const & d = light->Direction();
								float3 const & u = float3(0, 1, 0);

								float3 const & p = light->Position();
								float4x4 light_view = MathLib::look_at_lh(p, p + d, u);

								float const scale = light->CosOuterInner().w();
								float4x4 mat = MathLib::scaling(scale, scale, 1.0f);
								float4x4 light_model = mat * MathLib::inverse(light_view);
								*light_volume_mvp_param_ = light_model * vp;

								float3 min, max;
								min = max = MathLib::transform_coord(cone_bbox_[0], light_model);
								for (size_t k = 1; k < 8; ++ k)
								{
									float3 vec = MathLib::transform_coord(cone_bbox_[k], light_model);
									min = MathLib::minimize(min, vec);
									max = MathLib::maximize(max, vec);
								}

								if (scene_mgr.AABBVisible(Box(min, max)))
								{
									if (0 == (attr & LSA_NoShadow))
									{
										pass_scaned_.push_back(static_cast<uint32_t>((PT_GenShadowMap << 28) + (i << 16) + 0));
									}
									pass_scaned_.push_back(static_cast<uint32_t>((PT_Lighting << 28) + (i << 16) + 0));

									light_crs_[i][0]->Begin();
									re.Render(*technique_light_depth_only_, *rl_cone_);
									light_crs_[i][0]->End();
								}
							}
							break;
						
						case LT_Point:
							{
								float3 const & p = light->Position();
								for (int j = 0; j < 6; ++ j)
								{
									std::pair<float3, float3> ad = CubeMapViewVector<float>(static_cast<Texture::CubeFaces>(j));
									float3 const & d = ad.first;
									float3 const & u = ad.second;

									float4x4 light_view = MathLib::look_at_lh(p, p + d, u);
									float4x4 light_model = MathLib::inverse(light_view);
									*light_volume_mvp_param_ = light_model * vp;

									float3 min, max;
									min = max = MathLib::transform_coord(pyramid_bbox_[0], light_model);
									for (size_t k = 1; k < 8; ++ k)
									{
										float3 vec = MathLib::transform_coord(pyramid_bbox_[k], light_model);
										min = MathLib::minimize(min, vec);
										max = MathLib::maximize(max, vec);
									}

									if (scene_mgr.AABBVisible(Box(min, max)))
									{
										if (0 == (attr & LSA_NoShadow))
										{
											pass_scaned_.push_back(static_cast<uint32_t>((PT_GenShadowMap << 28) + (i << 16) + j));
										}

										light_crs_[i][j]->Begin();
										re.Render(*technique_light_depth_only_, *rl_pyramid_);
										light_crs_[i][j]->End();
									}
								}
								{
									float4x4 light_model = MathLib::translation(p);
									*light_volume_mvp_param_ = light_model * vp;

									float3 min, max;
									min = max = MathLib::transform_coord(box_bbox_[0], light_model);
									for (size_t k = 1; k < 8; ++ k)
									{
										float3 vec = MathLib::transform_coord(box_bbox_[k], light_model);
										min = MathLib::minimize(min, vec);
										max = MathLib::maximize(max, vec);
									}

									if (scene_mgr.AABBVisible(Box(min, max)))
									{
										pass_scaned_.push_back(static_cast<uint32_t>((PT_Lighting << 28) + (i << 16) + 6));

										light_crs_[i][6]->Begin();
										re.Render(*technique_light_depth_only_, *rl_box_);
										light_crs_[i][6]->End();
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
				DeferredLightSourcePtr const & light = lights_[org_no];

				LightType type = light->Type();
				int32_t attr = light->Attrib();

				float3 d, u;
				if (type != LT_Point)
				{
					d = light->Direction();
					u = float3(0, 1, 0);
				}
				else
				{
					std::pair<float3, float3> ad = CubeMapViewVector<float>(static_cast<Texture::CubeFaces>(index_in_pass));
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
					fov = light->CosOuterInner().z();
				}

				float3 const & p = light->Position();
				sm_buffer_->GetViewport().camera->ViewParams(p, p + d, u);
				sm_buffer_->GetViewport().camera->ProjParams(fov, 1, 0.1f, 100.0f);

				float3 dir_es = MathLib::transform_normal(d, view_);
				float4 light_dir_es_actived = float4(dir_es.x(), dir_es.y(), dir_es.z(), 0);

				*light_view_proj_param_ = inv_view_ * sm_buffer_->GetViewport().camera->ViewMatrix()
					* sm_buffer_->GetViewport().camera->ProjMatrix();
				
				float3 loc_es = MathLib::transform_coord(p, view_);
				float4 light_pos_es_actived = float4(loc_es.x(), loc_es.y(), loc_es.z(), 1);

				float4x4 mat = MathLib::inverse(sm_buffer_->GetViewport().camera->ViewMatrix()) * view_ * proj_;
				switch (type)
				{
				case LT_Spot:
					{
						light_pos_es_actived.w() = light->CosOuterInner().x();
						light_dir_es_actived.w() = light->CosOuterInner().y();

						rl_ = rl_cone_;
						float const scale = light->CosOuterInner().w();
						*light_volume_mvp_param_ = MathLib::scaling(scale, scale, 1.0f) * mat;
					}
					break;

				case LT_Point:
					if (PT_Lighting == pass_type)
					{
						rl_ = rl_box_;
						float4x4 light_model = MathLib::translation(p);
						*light_volume_mvp_param_ = light_model * view_ * proj_;
					}
					else
					{
						rl_ = rl_pyramid_;
						*light_volume_mvp_param_ = mat;
					}
					break;

				default:
					rl_ = rl_quad_;
					*light_volume_mvp_param_ = float4x4::Identity();
					break;
				}

				*light_pos_es_param_ = light_pos_es_actived;
				*light_dir_es_param_ = light_dir_es_actived;

				if ((index_in_pass > 0) || (PT_Lighting == pass_type))
				{
					if (0 == (attr & LSA_NoShadow))
					{
						if (LT_Point == type)
						{
							sm_filter_pps_[index_in_pass]->Apply();
							light_crs_[org_no][index_in_pass - 1]->EndConditionalRender();
						}
						else
						{
							sm_filter_pps_[0]->Apply();
						}
					}
				}

				if (PT_GenShadowMap == pass_type)
				{
					light_crs_[org_no][index_in_pass]->BeginConditionalRender();

					// Shadow map generation

					re.BindFrameBuffer(sm_buffer_);
					re.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, Color(0, 0, 0, 0), 1.0f, 0);

					return App3DFramework::URV_Need_Flush;
				}
				else //if (PT_Lighting == pass_type)
				{
					// Lighting

					re.BindFrameBuffer(lighting_buffer_);
					re.CurFrameBuffer()->Attached(FrameBuffer::ATT_DepthStencil)->Clear(0L);

					technique_ = technique_lights_[type];

					*light_attrib_param_ = attr;
					*light_color_param_ = light->Color();
					*light_falloff_param_ = light->Falloff();

					if ((attr & LSA_NoShadow) && (type != LT_Ambient) && (type != LT_Directional))
					{
						light_crs_[org_no][index_in_pass]->BeginConditionalRender();
					}

					if ((LT_Point == type) || (LT_Spot == type))
					{
						Camera const & camera = Context::Instance().AppInstance().ActiveCamera();
						float3 eye = camera.EyePos();

						bool eye_in_volume = false;
					
						if (LT_Spot == type)
						{
							float3 v = MathLib::normalize(eye - p);
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
							re.Render(*technique_light_stencil_eiv_, *rl_);
						}
						else
						{
							re.Render(*technique_light_stencil_eov_, *rl_);
						}
					}

					re.Render(*technique_, *rl_);

					if ((type != LT_Ambient) && (type != LT_Directional))
					{
						light_crs_[org_no][index_in_pass]->EndConditionalRender();
					}

					return App3DFramework::URV_Flushed;
				}
			}
		}
	}
}
