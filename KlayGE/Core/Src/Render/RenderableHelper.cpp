// RenderableHelper.cpp
// KlayGE 一些常用的可渲染对象 实现文件
// Ver 3.10.0
// 版权所有(C) 龚敏敏, 2005-2010
// Homepage: http://www.klayge.org
//
// 3.10.0
// RenderableSkyBox和RenderableHDRSkyBox增加了Technique() (2010.1.4)
//
// 3.9.0
// 增加了RenderableHDRSkyBox (2009.5.4)
//
// 2.7.1
// 增加了RenderableHelper基类 (2005.7.10)
//
// 2.6.0
// 增加了RenderableSkyBox (2005.5.26)
//
// 2.5.0
// 增加了RenderablePoint，RenderableLine和RenderableTriangle (2005.4.13)
//
// 2.4.0
// 初次建立 (2005.3.22)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/GraphicsBuffer.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/FrameBuffer.hpp>
#include <KlayGE/App3D.hpp>
#include <KlayGE/RenderLayout.hpp>
#include <KlayGE/Camera.hpp>

#include <cstring>
#include <boost/tuple/tuple.hpp>

#include <KlayGE/RenderableHelper.hpp>

namespace KlayGE
{
	RenderableHelper::RenderableHelper(std::wstring const & name)
		: name_(name)
	{
	}

	RenderTechniquePtr const & RenderableHelper::GetRenderTechnique() const
	{
		return technique_;
	}

	RenderLayoutPtr const & RenderableHelper::GetRenderLayout() const
	{
		return rl_;
	}

	AABBox const & RenderableHelper::Bound() const
	{
		return aabb_;
	}

	std::wstring const & RenderableHelper::Name() const
	{
		return name_;
	}


	RenderablePoint::RenderablePoint(float3 const & v, Color const & clr)
		: RenderableHelper(L"Point")
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();

		technique_ = rf.LoadEffect("RenderableHelper.fxml")->TechniqueByName("PointTec");
		color_ep_ = technique_->Effect().ParameterByName("color");
		matViewProj_ep_ = technique_->Effect().ParameterByName("matViewProj");
		*color_ep_ = float4(clr.r(), clr.g(), clr.b(), clr.a());

		rl_ = rf.MakeRenderLayout();
		rl_->TopologyType(RenderLayout::TT_PointList);

		ElementInitData init_data;
		init_data.row_pitch = sizeof(v);
		init_data.slice_pitch = 0;
		init_data.data = &v;

		GraphicsBufferPtr vb = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, &init_data);
		rl_->BindVertexStream(vb, boost::make_tuple(vertex_element(VEU_Position, 0, EF_BGR32F)));

		aabb_ = MathLib::compute_aabbox(&v, &v + 1);
	}

	void RenderablePoint::OnRenderBegin()
	{
		App3DFramework const & app = Context::Instance().AppInstance();
		Camera const & camera = app.ActiveCamera();

		float4x4 view_proj = camera.ViewMatrix() * camera.ProjMatrix();
		*matViewProj_ep_ = view_proj;
	}


	RenderableLine::RenderableLine(float3 const & v0, float3 const & v1, Color const & clr)
		: RenderableHelper(L"Line")
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();

		technique_ = rf.LoadEffect("RenderableHelper.fxml")->TechniqueByName("LineTec");
		color_ep_ = technique_->Effect().ParameterByName("color");
		matViewProj_ep_ = technique_->Effect().ParameterByName("matViewProj");
		*color_ep_ = float4(clr.r(), clr.g(), clr.b(), clr.a());

		float3 xyzs[] =
		{
			v0, v1
		};

		ElementInitData init_data;
		init_data.row_pitch = sizeof(xyzs);
		init_data.slice_pitch = 0;
		init_data.data = xyzs;

		rl_ = rf.MakeRenderLayout();
		rl_->TopologyType(RenderLayout::TT_LineList);

		GraphicsBufferPtr vb = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, &init_data);
		rl_->BindVertexStream(vb, boost::make_tuple(vertex_element(VEU_Position, 0, EF_BGR32F)));

		aabb_ = MathLib::compute_aabbox(&xyzs[0], &xyzs[0] + sizeof(xyzs) / sizeof(xyzs[0]));
	}

	void RenderableLine::OnRenderBegin()
	{
		App3DFramework const & app = Context::Instance().AppInstance();
		Camera const & camera = app.ActiveCamera();

		float4x4 view_proj = camera.ViewMatrix() * camera.ProjMatrix();
		*matViewProj_ep_ = view_proj;
	}


	RenderableTriangle::RenderableTriangle(float3 const & v0, float3 const & v1, float3 const & v2, Color const & clr)
		: RenderableHelper(L"Triangle")
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();

		technique_ = rf.LoadEffect("RenderableHelper.fxml")->TechniqueByName("TriangleTec");
		color_ep_ = technique_->Effect().ParameterByName("color");
		matViewProj_ep_ = technique_->Effect().ParameterByName("matViewProj");
		*color_ep_ = float4(clr.r(), clr.g(), clr.b(), clr.a());

		float3 xyzs[] =
		{
			v0, v1, v2
		};

		ElementInitData init_data;
		init_data.row_pitch = sizeof(xyzs);
		init_data.slice_pitch = 0;
		init_data.data = xyzs;

		rl_ = rf.MakeRenderLayout();
		rl_->TopologyType(RenderLayout::TT_TriangleList);

		GraphicsBufferPtr vb = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, &init_data);
		rl_->BindVertexStream(vb, boost::make_tuple(vertex_element(VEU_Position, 0, EF_BGR32F)));

		aabb_ = MathLib::compute_aabbox(&xyzs[0], &xyzs[0] + sizeof(xyzs) / sizeof(xyzs[0]));
	}

	void RenderableTriangle::OnRenderBegin()
	{
		App3DFramework const & app = Context::Instance().AppInstance();
		Camera const & camera = app.ActiveCamera();

		float4x4 view_proj = camera.ViewMatrix() * camera.ProjMatrix();
		*matViewProj_ep_ = view_proj;
	}


	RenderableTriBox::RenderableTriBox(AABBox const & aabb, Color const & clr)
		: RenderableHelper(L"TriBox")
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();

		aabb_ = aabb;

		technique_ = rf.LoadEffect("RenderableHelper.fxml")->TechniqueByName("BoxTec");
		color_ep_ = technique_->Effect().ParameterByName("color");
		matViewProj_ep_ = technique_->Effect().ParameterByName("matViewProj");
		*color_ep_ = float4(clr.r(), clr.g(), clr.b(), clr.a());

		float3 xyzs[] =
		{
			aabb[0], aabb[1], aabb[2], aabb[3], aabb[4], aabb[5], aabb[6], aabb[7]
		};

		uint16_t indices[] =
		{
			0, 2, 3, 3, 1, 0,
			5, 7, 6, 6, 4, 5,
			4, 0, 1, 1, 5, 4,
			4, 6, 2, 2, 0, 4,
			2, 6, 7, 7, 3, 2,
			1, 3, 7, 7, 5, 1
		};

		rl_ = rf.MakeRenderLayout();
		rl_->TopologyType(RenderLayout::TT_TriangleList);

		ElementInitData init_data;
		init_data.row_pitch = sizeof(xyzs);
		init_data.slice_pitch = 0;
		init_data.data = xyzs;

		GraphicsBufferPtr vb = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, &init_data);
		rl_->BindVertexStream(vb, boost::make_tuple(vertex_element(VEU_Position, 0, EF_BGR32F)));

		init_data.row_pitch = sizeof(indices);
		init_data.slice_pitch = 0;
		init_data.data = indices;

		GraphicsBufferPtr ib = rf.MakeIndexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, &init_data);
		rl_->BindIndexStream(ib, EF_R16UI);
	}

	void RenderableTriBox::OnRenderBegin()
	{
		App3DFramework const & app = Context::Instance().AppInstance();
		Camera const & camera = app.ActiveCamera();

		float4x4 view_proj = camera.ViewMatrix() * camera.ProjMatrix();
		*matViewProj_ep_ = view_proj;
	}


	RenderableLineBox::RenderableLineBox(AABBox const & aabb, Color const & clr)
		: RenderableHelper(L"LineBox")
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();

		aabb_ = aabb;

		technique_ = rf.LoadEffect("RenderableHelper.fxml")->TechniqueByName("BoxTec");
		color_ep_ = technique_->Effect().ParameterByName("color");
		matViewProj_ep_ = technique_->Effect().ParameterByName("matViewProj");
		*color_ep_ = float4(clr.r(), clr.g(), clr.b(), clr.a());

		float3 xyzs[] =
		{
			aabb[0], aabb[1], aabb[2], aabb[3], aabb[4], aabb[5], aabb[6], aabb[7]
		};

		uint16_t indices[] =
		{
			0, 1, 1, 3, 3, 2, 2, 0,
			4, 5, 5, 7, 7, 6, 6, 4,
			0, 4, 1, 5, 2, 6, 3, 7
		};

		rl_ = rf.MakeRenderLayout();
		rl_->TopologyType(RenderLayout::TT_LineList);

		ElementInitData init_data;
		init_data.row_pitch = sizeof(xyzs);
		init_data.slice_pitch = 0;
		init_data.data = xyzs;

		GraphicsBufferPtr vb = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, &init_data);
		rl_->BindVertexStream(vb, boost::make_tuple(vertex_element(VEU_Position, 0, EF_BGR32F)));

		init_data.row_pitch = sizeof(indices);
		init_data.slice_pitch = 0;
		init_data.data = indices;

		GraphicsBufferPtr ib = rf.MakeIndexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, &init_data);
		rl_->BindIndexStream(ib, EF_R16UI);
	}

	void RenderableLineBox::OnRenderBegin()
	{
		App3DFramework const & app = Context::Instance().AppInstance();
		Camera const & camera = app.ActiveCamera();

		float4x4 view_proj = camera.ViewMatrix() * camera.ProjMatrix();
		*matViewProj_ep_ = view_proj;
	}


	RenderableSkyBox::RenderableSkyBox()
		: RenderableHelper(L"SkyBox")
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();

		if (deferred_effect_)
		{
			depth_tech_ = deferred_effect_->TechniqueByName("DepthSkyBoxTech");
			gbuffer_rt0_tech_ = deferred_effect_->TechniqueByName("GBufferSkyBoxRT0Tech");
			gbuffer_rt1_tech_ = deferred_effect_->TechniqueByName("GBufferLDRSkyBoxRT1Tech");
			gbuffer_mrt_tech_ = deferred_effect_->TechniqueByName("GBufferLDRSkyBoxMRTTech");
			this->Technique(gbuffer_rt0_tech_);

			skybox_cube_tex_ep_ = deferred_effect_->ParameterByName("skybox_tex");
			depth_far_ep_ = deferred_effect_->ParameterByName("depth_far");
			inv_mvp_ep_ = deferred_effect_->ParameterByName("inv_mvp");
		}
		else
		{
			this->Technique(rf.LoadEffect("RenderableHelper.fxml")->TechniqueByName("SkyBoxTec"));
		}

		float3 xyzs[] =
		{
			float3(1.0f, 1.0f, 1.0f),
			float3(1.0f, -1.0f, 1.0f),
			float3(-1.0f, 1.0f, 1.0f),
			float3(-1.0f, -1.0f, 1.0f),
		};

		ElementInitData init_data;
		init_data.row_pitch = sizeof(xyzs);
		init_data.slice_pitch = 0;
		init_data.data = xyzs;

		rl_ = rf.MakeRenderLayout();
		rl_->TopologyType(RenderLayout::TT_TriangleStrip);

		GraphicsBufferPtr vb = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, &init_data);
		rl_->BindVertexStream(vb, boost::make_tuple(vertex_element(VEU_Position, 0, EF_BGR32F)));

		aabb_ = MathLib::compute_aabbox(&xyzs[0], &xyzs[4]);
	}

	void RenderableSkyBox::Technique(RenderTechniquePtr const & tech)
	{
		technique_ = tech;
		skybox_cube_tex_ep_ = technique_->Effect().ParameterByName("skybox_tex");
		depth_far_ep_ = technique_->Effect().ParameterByName("depth_far");
		inv_mvp_ep_ = technique_->Effect().ParameterByName("inv_mvp");
	}

	void RenderableSkyBox::CubeMap(TexturePtr const & cube)
	{
		*skybox_cube_tex_ep_ = cube;
	}

	void RenderableSkyBox::Pass(PassType type)
	{
		switch (type)
		{
		case PT_OpaqueDepth:
			technique_ = depth_tech_;
			break;

		case PT_OpaqueGBufferRT0:
			technique_ = gbuffer_rt0_tech_;
			break;

		case PT_OpaqueGBufferRT1:
			technique_ = gbuffer_rt1_tech_;
			break;

		case PT_OpaqueGBufferMRT:
			technique_ = gbuffer_mrt_tech_;
			break;

		case PT_OpaqueSpecialShading:
			technique_ = special_shading_tech_;
			break;

		default:
			break;
		}
	}

	void RenderableSkyBox::OnRenderBegin()
	{
		App3DFramework const & app = Context::Instance().AppInstance();
		Camera const & camera = app.ActiveCamera();

		if (depth_far_ep_)
		{
			*depth_far_ep_ = camera.FarPlane();
		}

		float4x4 rot_view = camera.ViewMatrix();
		rot_view(3, 0) = 0;
		rot_view(3, 1) = 0;
		rot_view(3, 2) = 0;
		*inv_mvp_ep_ = MathLib::inverse(rot_view * camera.ProjMatrix());
	}


	RenderableHDRSkyBox::RenderableHDRSkyBox()
	{
		if (deferred_effect_)
		{
			gbuffer_rt1_tech_ = deferred_effect_->TechniqueByName("GBufferSkyBoxRT1Tech");
			gbuffer_mrt_tech_ = deferred_effect_->TechniqueByName("GBufferSkyBoxMRTTech");

			skybox_Ccube_tex_ep_ = deferred_effect_->ParameterByName("skybox_C_tex");
		}
		else
		{
			this->Technique(technique_->Effect().TechniqueByName("HDRSkyBoxTec"));
		}
	}

	void RenderableHDRSkyBox::Technique(RenderTechniquePtr const & tech)
	{
		RenderableSkyBox::Technique(tech);
		skybox_Ccube_tex_ep_ = technique_->Effect().ParameterByName("skybox_C_tex");
	}

	void RenderableHDRSkyBox::CompressedCubeMap(TexturePtr const & y_cube, TexturePtr const & c_cube)
	{
		*skybox_cube_tex_ep_ = y_cube;
		*skybox_Ccube_tex_ep_ = c_cube;
	}


	RenderablePlane::RenderablePlane(float length, float width,
				int length_segs, int width_segs, bool has_tex_coord)
			: RenderableHelper(L"RenderablePlane")
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();

		rl_ = rf.MakeRenderLayout();
		rl_->TopologyType(RenderLayout::TT_TriangleList);

		std::vector<float3> pos;
		for (int y = 0; y < width_segs + 1; ++ y)
		{
			for (int x = 0; x < length_segs + 1; ++ x)
			{
				pos.push_back(float3(x * (length / length_segs) - length / 2,
					-y * (width / width_segs) + width / 2, 0.0f));
			}
		}

		ElementInitData init_data;
		init_data.row_pitch = static_cast<uint32_t>(pos.size() * sizeof(pos[0]));
		init_data.slice_pitch = 0;
		init_data.data = &pos[0];

		GraphicsBufferPtr pos_vb = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, &init_data);
		rl_->BindVertexStream(pos_vb, boost::make_tuple(vertex_element(VEU_Position, 0, EF_BGR32F)));

		if (has_tex_coord)
		{
			std::vector<float2> tex;
			for (int y = 0; y < width_segs + 1; ++ y)
			{
				for (int x = 0; x < length_segs + 1; ++ x)
				{
					tex.push_back(float2(static_cast<float>(x) / length_segs,
						static_cast<float>(y) / width_segs));
				}
			}

			init_data.row_pitch = static_cast<uint32_t>(tex.size() * sizeof(tex[0]));
			init_data.slice_pitch = 0;
			init_data.data = &tex[0];

			GraphicsBufferPtr tex_vb = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, &init_data);
			rl_->BindVertexStream(tex_vb, boost::make_tuple(vertex_element(VEU_TextureCoord, 0, EF_GR32F)));
		}

		std::vector<uint16_t> index;
		for (int y = 0; y < width_segs; ++ y)
		{
			for (int x = 0; x < length_segs; ++ x)
			{
				index.push_back(static_cast<uint16_t>((y + 0) * (length_segs + 1) + (x + 0)));
				index.push_back(static_cast<uint16_t>((y + 0) * (length_segs + 1) + (x + 1)));
				index.push_back(static_cast<uint16_t>((y + 1) * (length_segs + 1) + (x + 1)));

				index.push_back(static_cast<uint16_t>((y + 1) * (length_segs + 1) + (x + 1)));
				index.push_back(static_cast<uint16_t>((y + 1) * (length_segs + 1) + (x + 0)));
				index.push_back(static_cast<uint16_t>((y + 0) * (length_segs + 1) + (x + 0)));
			}
		}

		init_data.row_pitch = static_cast<uint32_t>(index.size() * sizeof(index[0]));
		init_data.slice_pitch = 0;
		init_data.data = &index[0];

		GraphicsBufferPtr ib = rf.MakeIndexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, &init_data);
		rl_->BindIndexStream(ib, EF_R16UI);

		aabb_ = MathLib::compute_aabbox(pos.begin(), pos.end());
	}


	RenderDecal::RenderDecal(TexturePtr const & normal_tex, TexturePtr const & diffuse_tex, float3 const & diffuse_clr,
			TexturePtr const & specular_tex, float3 const & specular_level, float shininess)
		: RenderableHelper(L"Decal")
	{
		BOOST_ASSERT(deferred_effect_);

		gbuffer_alpha_test_mrt_tech_ = deferred_effect_->TechniqueByName("DecalGBufferAlphaTestMRTTech");
		technique_ = gbuffer_alpha_test_mrt_tech_;

		aabb_ = AABBox(float3(-1, -1, -1), float3(1, 1, 1));
		float3 xyzs[] =
		{
			aabb_[0], aabb_[1], aabb_[2], aabb_[3], aabb_[4], aabb_[5], aabb_[6], aabb_[7]
		};

		uint16_t indices[] =
		{
			0, 2, 3, 3, 1, 0,
			5, 7, 6, 6, 4, 5,
			4, 0, 1, 1, 5, 4,
			4, 6, 2, 2, 0, 4,
			2, 6, 7, 7, 3, 2,
			1, 3, 7, 7, 5, 1
		};

		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		rl_ = rf.MakeRenderLayout();
		rl_->TopologyType(RenderLayout::TT_TriangleList);

		ElementInitData init_data;
		init_data.row_pitch = sizeof(xyzs);
		init_data.slice_pitch = 0;
		init_data.data = xyzs;

		GraphicsBufferPtr vb = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, &init_data);
		rl_->BindVertexStream(vb, boost::make_tuple(vertex_element(VEU_Position, 0, EF_BGR32F)));

		init_data.row_pitch = sizeof(indices);
		init_data.slice_pitch = 0;
		init_data.data = indices;

		GraphicsBufferPtr ib = rf.MakeIndexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, &init_data);
		rl_->BindIndexStream(ib, EF_R16UI);

		model_mat_ = float4x4::Identity();
		effect_attrs_ |= EA_AlphaTest;

		inv_mv_ep_ = technique_->Effect().ParameterByName("inv_mv");

		normal_tex_ = normal_tex;
		diffuse_tex_ = diffuse_tex;
		diffuse_clr_ = diffuse_clr;
		specular_tex_ = specular_tex;
		specular_level_ = specular_level.x();
		shininess_ = shininess;
	}

	void RenderDecal::OnRenderBegin()
	{
		RenderableHelper::OnRenderBegin();

		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		Camera const & camera = *re.CurFrameBuffer()->GetViewport()->camera;

		float4x4 const & view_to_decal = MathLib::inverse(model_mat_ * camera.ViewMatrix());
					
		switch (type_)
		{
		case PT_OpaqueGBufferMRT:
		case PT_TransparencyBackGBufferMRT:
		case PT_TransparencyFrontGBufferMRT:
			*diffuse_clr_param_ = float4(diffuse_clr_.x(), diffuse_clr_.y(), diffuse_clr_.z(), static_cast<float>(!!diffuse_tex_));
			*specular_level_param_ = float4(specular_level_, 0, 0, static_cast<float>(!!specular_tex_));
			*shininess_param_ = MathLib::clamp(shininess_ / 256.0f, 1e-6f, 0.999f);
			*inv_mv_ep_ = view_to_decal;
			break;
		}
	}
}
