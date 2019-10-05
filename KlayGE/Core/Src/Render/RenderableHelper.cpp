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
#include <KFL/Math.hpp>
#include <KFL/Util.hpp>
#include <KlayGE/GraphicsBuffer.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/FrameBuffer.hpp>
#include <KlayGE/App3D.hpp>
#include <KlayGE/RenderLayout.hpp>
#include <KlayGE/Camera.hpp>
#include <KlayGE/RenderMaterial.hpp>
#include <KlayGE/Renderable.hpp>
#include <KlayGE/DeferredRenderingLayer.hpp>

#include <cstring>
#include <iterator>

#include <KlayGE/RenderableHelper.hpp>

namespace KlayGE
{
	RenderablePoint::RenderablePoint()
		: Renderable(L"Point")
	{
		auto& rf = Context::Instance().RenderFactoryInstance();

		effect_ = SyncLoadRenderEffect("RenderableHelper.fxml");
		technique_ = simple_forward_tech_ = effect_->TechniqueByName("PointTec");
		v0_ep_ = effect_->ParameterByName("v0");
		color_ep_ = effect_->ParameterByName("color");

		rls_[0] = rf.MakeRenderLayout();
		rls_[0]->TopologyType(RenderLayout::TT_PointList);

		float v = 0;
		auto vb = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, sizeof(v), &v);
		rls_[0]->BindVertexStream(vb, VertexElement(VEU_Position, 0, EF_R32F));

		tc_aabb_ = AABBox(float3(0, 0, 0), float3(0, 0, 0));

		*(effect_->ParameterByName("pos_center")) = float3(0, 0, 0);
		*(effect_->ParameterByName("pos_extent")) = float3(1, 1, 1);

		effect_attrs_ |= EA_SimpleForward;

		this->UpdateBoundBox();
	}

	RenderablePoint::RenderablePoint(float3 const & v, Color const & clr)
		: RenderablePoint()
	{
		this->SetPoint(v);
		this->SetColor(clr);
	}

	void RenderablePoint::SetPoint(float3 const & v)
	{
		pos_aabb_.Min() = pos_aabb_.Max() = v;
		*v0_ep_ = v;
	}

	void RenderablePoint::SetColor(Color const & clr)
	{
		*color_ep_ = float4(clr.r(), clr.g(), clr.b(), clr.a());
	}


	RenderableLine::RenderableLine()
		: Renderable(L"Line")
	{
		auto& rf = Context::Instance().RenderFactoryInstance();

		effect_ = SyncLoadRenderEffect("RenderableHelper.fxml");
		technique_ = simple_forward_tech_ = effect_->TechniqueByName("LineTec");
		v0_ep_ = effect_->ParameterByName("v0");
		v1_ep_ = effect_->ParameterByName("v1");
		color_ep_ = effect_->ParameterByName("color");

		float vertices[] =
		{
			0, 1
		};

		rls_[0] = rf.MakeRenderLayout();
		rls_[0]->TopologyType(RenderLayout::TT_LineList);

		auto vb = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, sizeof(vertices), vertices);
		rls_[0]->BindVertexStream(vb, VertexElement(VEU_Position, 0, EF_R32F));

		tc_aabb_ = AABBox(float3(0, 0, 0), float3(0, 0, 0));

		*(effect_->ParameterByName("pos_center")) = float3(0, 0, 0);
		*(effect_->ParameterByName("pos_extent")) = float3(1, 1, 1);

		effect_attrs_ |= EA_SimpleForward;

		this->UpdateBoundBox();
	}
	
	RenderableLine::RenderableLine(float3 const & v0, float3 const & v1, Color const & clr)
		: RenderableLine()
	{
		this->SetLine(v0, v1);
		this->SetColor(clr);
	}

	void RenderableLine::SetLine(float3 const & v0, float3 const & v1)
	{
		float3 vs[] = 
		{
			v0, v1
		};
		pos_aabb_ = MathLib::compute_aabbox(&vs[0], &vs[0] + std::size(vs));
		
		*v0_ep_ = v0;
		*v1_ep_ = v1;
	}

	void RenderableLine::SetColor(Color const & clr)
	{
		*color_ep_ = float4(clr.r(), clr.g(), clr.b(), clr.a());
	}


	RenderableTriangle::RenderableTriangle()
		: Renderable(L"Triangle")
	{
		auto& rf = Context::Instance().RenderFactoryInstance();

		effect_ = SyncLoadRenderEffect("RenderableHelper.fxml");
		technique_ = simple_forward_tech_ = effect_->TechniqueByName("LineTec");
		v0_ep_ = effect_->ParameterByName("v0");
		v1_ep_ = effect_->ParameterByName("v1");
		v2_ep_ = effect_->ParameterByName("v2");
		color_ep_ = effect_->ParameterByName("color");

		float vertices[] =
		{
			0, 1, 2
		};

		rls_[0] = rf.MakeRenderLayout();
		rls_[0]->TopologyType(RenderLayout::TT_TriangleList);

		auto vb = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, sizeof(vertices), vertices);
		rls_[0]->BindVertexStream(vb, VertexElement(VEU_Position, 0, EF_R32F));

		tc_aabb_ = AABBox(float3(0, 0, 0), float3(0, 0, 0));

		*(effect_->ParameterByName("pos_center")) = float3(0, 0, 0);
		*(effect_->ParameterByName("pos_extent")) = float3(1, 1, 1);

		effect_attrs_ |= EA_SimpleForward;

		this->UpdateBoundBox();
	}

	RenderableTriangle::RenderableTriangle(float3 const & v0, float3 const & v1, float3 const & v2, Color const & clr)
		: RenderableTriangle()
	{
		this->SetTriangle(v0, v1, v2);
		this->SetColor(clr);
	}

	void RenderableTriangle::SetTriangle(float3 const & v0, float3 const & v1, float3 const & v2)
	{
		float3 vs[] = 
		{
			v0, v1, v2
		};
		pos_aabb_ = MathLib::compute_aabbox(&vs[0], &vs[0] + std::size(vs));
		
		*v0_ep_ = v0;
		*v1_ep_ = v1;
		*v2_ep_ = v2;
	}

	void RenderableTriangle::SetColor(Color const & clr)
	{
		*color_ep_ = float4(clr.r(), clr.g(), clr.b(), clr.a());
	}


	RenderableTriBox::RenderableTriBox()
		: Renderable(L"TriBox")
	{
		auto& rf = Context::Instance().RenderFactoryInstance();

		effect_ = SyncLoadRenderEffect("RenderableHelper.fxml");
		technique_ = simple_forward_tech_ = effect_->TechniqueByName("LineTec");
		v0_ep_ = effect_->ParameterByName("v0");
		v1_ep_ = effect_->ParameterByName("v1");
		v2_ep_ = effect_->ParameterByName("v2");
		v3_ep_ = effect_->ParameterByName("v3");
		v4_ep_ = effect_->ParameterByName("v4");
		v5_ep_ = effect_->ParameterByName("v5");
		v6_ep_ = effect_->ParameterByName("v6");
		v7_ep_ = effect_->ParameterByName("v7");
		color_ep_ = effect_->ParameterByName("color");

		float vertices[] =
		{
			0, 1, 2, 3, 4, 5, 6, 7
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

		rls_[0] = rf.MakeRenderLayout();
		rls_[0]->TopologyType(RenderLayout::TT_TriangleList);

		auto vb = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, sizeof(vertices), vertices);
		rls_[0]->BindVertexStream(vb, VertexElement(VEU_Position, 0, EF_R32F));

		auto ib = rf.MakeIndexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, sizeof(indices), indices);
		rls_[0]->BindIndexStream(ib, EF_R16UI);

		tc_aabb_ = AABBox(float3(0, 0, 0), float3(0, 0, 0));

		*(effect_->ParameterByName("pos_center")) = float3(0, 0, 0);
		*(effect_->ParameterByName("pos_extent")) = float3(1, 1, 1);

		effect_attrs_ |= EA_SimpleForward;

		this->UpdateBoundBox();
	}

	RenderableTriBox::RenderableTriBox(OBBox const & obb, Color const & clr)
		: RenderableTriBox()
	{
		this->SetBox(obb);
		this->SetColor(clr);
	}

	void RenderableTriBox::SetBox(OBBox const & obb)
	{
		pos_aabb_ = MathLib::convert_to_aabbox(obb);

		*v0_ep_ = obb.Corner(0);
		*v1_ep_ = obb.Corner(1);
		*v2_ep_ = obb.Corner(2);
		*v3_ep_ = obb.Corner(3);
		*v4_ep_ = obb.Corner(4);
		*v5_ep_ = obb.Corner(5);
		*v6_ep_ = obb.Corner(6);
		*v7_ep_ = obb.Corner(7);
	}

	void RenderableTriBox::SetColor(Color const & clr)
	{
		*color_ep_ = float4(clr.r(), clr.g(), clr.b(), clr.a());
	}


	RenderableLineBox::RenderableLineBox()
		: Renderable(L"LineBox")
	{
		auto& rf = Context::Instance().RenderFactoryInstance();

		effect_ = SyncLoadRenderEffect("RenderableHelper.fxml");
		technique_ = simple_forward_tech_ = effect_->TechniqueByName("LineTec");
		v0_ep_ = effect_->ParameterByName("v0");
		v1_ep_ = effect_->ParameterByName("v1");
		v2_ep_ = effect_->ParameterByName("v2");
		v3_ep_ = effect_->ParameterByName("v3");
		v4_ep_ = effect_->ParameterByName("v4");
		v5_ep_ = effect_->ParameterByName("v5");
		v6_ep_ = effect_->ParameterByName("v6");
		v7_ep_ = effect_->ParameterByName("v7");
		color_ep_ = effect_->ParameterByName("color");

		float vertices[] =
		{
			0, 1, 2, 3, 4, 5, 6, 7
		};

		uint16_t indices[] =
		{
			0, 1, 1, 3, 3, 2, 2, 0,
			4, 5, 5, 7, 7, 6, 6, 4,
			0, 4, 1, 5, 2, 6, 3, 7
		};

		rls_[0] = rf.MakeRenderLayout();
		rls_[0]->TopologyType(RenderLayout::TT_LineList);

		auto vb = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, sizeof(vertices), vertices);
		rls_[0]->BindVertexStream(vb, VertexElement(VEU_Position, 0, EF_R32F));

		auto ib = rf.MakeIndexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, sizeof(indices), indices);
		rls_[0]->BindIndexStream(ib, EF_R16UI);

		tc_aabb_ = AABBox(float3(0, 0, 0), float3(0, 0, 0));

		*(effect_->ParameterByName("pos_center")) = float3(0, 0, 0);
		*(effect_->ParameterByName("pos_extent")) = float3(1, 1, 1);

		effect_attrs_ |= EA_SimpleForward;

		this->UpdateBoundBox();
	}
	
	RenderableLineBox::RenderableLineBox(OBBox const & obb, Color const & clr)
		: RenderableLineBox()
	{
		this->SetBox(obb);
		this->SetColor(clr);
	}

	void RenderableLineBox::SetBox(OBBox const & obb)
	{
		pos_aabb_ = MathLib::convert_to_aabbox(obb);

		*v0_ep_ = obb.Corner(0);
		*v1_ep_ = obb.Corner(1);
		*v2_ep_ = obb.Corner(2);
		*v3_ep_ = obb.Corner(3);
		*v4_ep_ = obb.Corner(4);
		*v5_ep_ = obb.Corner(5);
		*v6_ep_ = obb.Corner(6);
		*v7_ep_ = obb.Corner(7);
	}

	void RenderableLineBox::SetColor(Color const & clr)
	{
		*color_ep_ = float4(clr.r(), clr.g(), clr.b(), clr.a());
	}


	RenderablePlane::RenderablePlane(float length, float width,
				int length_segs, int width_segs, bool has_tex_coord, bool has_tangent)
			: Renderable(L"RenderablePlane")
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();

		rls_[0] = rf.MakeRenderLayout();
		rls_[0]->TopologyType(RenderLayout::TT_TriangleList);

		std::vector<int16_t> positions;
		for (int y = 0; y < width_segs + 1; ++ y)
		{
			for (int x = 0; x < length_segs + 1; ++ x)
			{
				float3 pos(static_cast<float>(x) / length_segs, 1 - (static_cast<float>(y) / width_segs), 0.5f);
				int16_t s_pos[4] = 
				{
					static_cast<int16_t>(MathLib::clamp<int32_t>(static_cast<int32_t>(pos.x() * 65535 - 32768), -32768, 32767)),
					static_cast<int16_t>(MathLib::clamp<int32_t>(static_cast<int32_t>(pos.y() * 65535 - 32768), -32768, 32767)),
					static_cast<int16_t>(MathLib::clamp<int32_t>(static_cast<int32_t>(pos.z() * 65535 - 32768), -32768, 32767)),
					32767
				};

				positions.push_back(s_pos[0]);
				positions.push_back(s_pos[1]);
				positions.push_back(s_pos[2]);
				positions.push_back(s_pos[3]);
			}
		}

		GraphicsBufferPtr pos_vb = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable,
			static_cast<uint32_t>(positions.size() * sizeof(positions[0])), &positions[0]);
		rls_[0]->BindVertexStream(pos_vb, VertexElement(VEU_Position, 0, EF_SIGNED_ABGR16));

		if (has_tex_coord)
		{
			std::vector<int16_t> tex_coords;
			for (int y = 0; y < width_segs + 1; ++ y)
			{
				for (int x = 0; x < length_segs + 1; ++ x)
				{
					float3 tex_coord(static_cast<float>(x) / length_segs * 0.5f + 0.5f,
						static_cast<float>(y) / width_segs * 0.5f + 0.5f, 0.5f);
					int16_t s_tc[2] = 
					{
						static_cast<int16_t>(MathLib::clamp<int32_t>(static_cast<int32_t>(tex_coord.x() * 65535 - 32768), -32768, 32767)),
						static_cast<int16_t>(MathLib::clamp<int32_t>(static_cast<int32_t>(tex_coord.y() * 65535 - 32768), -32768, 32767)),
					};

					tex_coords.push_back(s_tc[0]);
					tex_coords.push_back(s_tc[1]);
				}
			}

			GraphicsBufferPtr tex_vb = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable,
				static_cast<uint32_t>(tex_coords.size() * sizeof(tex_coords[0])), &tex_coords[0]);
			rls_[0]->BindVertexStream(tex_vb, VertexElement(VEU_TextureCoord, 0, EF_SIGNED_GR16));
		}

		if (has_tangent)
		{
			std::vector<uint32_t> tangent(positions.size() / 4);
			ElementFormat fmt;
			if (rf.RenderEngineInstance().DeviceCaps().VertexFormatSupport(EF_ABGR8))
			{
				fmt = EF_ABGR8;
				tangent.assign(tangent.size(), 0x807F7FFE);
			}
			else
			{
				BOOST_ASSERT(rf.RenderEngineInstance().DeviceCaps().VertexFormatSupport(EF_ARGB8));

				fmt = EF_ARGB8;
				tangent.assign(tangent.size(), 0x80FE7F7F);
			}

			GraphicsBufferPtr tex_vb = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable,
				static_cast<uint32_t>(tangent.size() * sizeof(tangent[0])), &tangent[0]);
			rls_[0]->BindVertexStream(tex_vb, VertexElement(VEU_Tangent, 0, fmt));
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

		GraphicsBufferPtr ib = rf.MakeIndexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable,
			static_cast<uint32_t>(index.size() * sizeof(index[0])), &index[0]);
		rls_[0]->BindIndexStream(ib, EF_R16UI);

		pos_aabb_ = AABBox(float3(-length / 2, -width / 2, 0), float3(+length / 2, +width / 2, 0));
		tc_aabb_ = AABBox(float3(0, 0, 0), float3(1, 1, 0));

		this->UpdateBoundBox();
	}


	RenderDecal::RenderDecal(TexturePtr const & normal_tex, TexturePtr const & albedo_tex,
			float3 const & albedo_clr, float metalness, float glossiness)
		: Renderable(L"Decal")
	{
		this->BindDeferredEffect(SyncLoadRenderEffect("Decal.fxml"));

		gbuffer_tech_ = effect_->TechniqueByName("DecalGBufferAlphaTestTech");
		technique_ = gbuffer_tech_;

		pos_aabb_ = AABBox(float3(-1, -1, -1), float3(1, 1, 1));
		tc_aabb_ = AABBox(float3(0, 0, 0), float3(1, 1, 0));

		float3 xyzs[] =
		{
			pos_aabb_.Corner(0), pos_aabb_.Corner(1), pos_aabb_.Corner(2), pos_aabb_.Corner(3),
			pos_aabb_.Corner(4), pos_aabb_.Corner(5), pos_aabb_.Corner(6), pos_aabb_.Corner(7)
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
		rls_[0] = rf.MakeRenderLayout();
		rls_[0]->TopologyType(RenderLayout::TT_TriangleList);

		GraphicsBufferPtr vb = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, sizeof(xyzs), xyzs);
		rls_[0]->BindVertexStream(vb, VertexElement(VEU_Position, 0, EF_BGR32F));

		GraphicsBufferPtr ib = rf.MakeIndexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, sizeof(indices), indices);
		rls_[0]->BindIndexStream(ib, EF_R16UI);

		this->ModelMatrix(float4x4::Identity());

		mtl_ = MakeSharedPtr<RenderMaterial>();
		mtl_->Albedo(float4(albedo_clr.x(), albedo_clr.y(), albedo_clr.z(), 1));
		mtl_->Metalness(metalness);
		mtl_->Glossiness(glossiness);

		effect_attrs_ |= EA_AlphaTest;

		g_buffer_rt0_tex_param_ = effect_->ParameterByName("g_buffer_rt0_tex");

		mtl_->Texture(RenderMaterial::TS_Normal, rf.MakeTextureSrv(normal_tex));
		mtl_->Texture(RenderMaterial::TS_Albedo, rf.MakeTextureSrv(albedo_tex));

		this->UpdateBoundBox();
	}

	void RenderDecal::OnRenderBegin()
	{
		Renderable::OnRenderBegin();

		auto drl = Context::Instance().DeferredRenderingLayerInstance();

		switch (type_)
		{
		case PT_OpaqueGBuffer:
		case PT_TransparencyBackGBuffer:
		case PT_TransparencyFrontGBuffer:
			*opaque_depth_tex_param_ = drl->ResolvedDepthTex(drl->ActiveViewport());
			*g_buffer_rt0_tex_param_ = drl->GBufferRT0BackupTex(drl->ActiveViewport());
			break;

		default:
			break;
		}
	}
}
