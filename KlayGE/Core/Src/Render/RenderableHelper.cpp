// RenderableHelper.cpp
// KlayGE 一些常用的可渲染对象 实现文件
// Ver 2.7.1
// 版权所有(C) 龚敏敏, 2005
// Homepage: http://klayge.sourceforge.net
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

	Box const & RenderableHelper::GetBound() const
	{
		return box_;
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

		GraphicsBufferPtr vb = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read, &init_data);
		rl_->BindVertexStream(vb, boost::make_tuple(vertex_element(VEU_Position, 0, EF_BGR32F)));

		box_ = MathLib::compute_bounding_box<float>(&v, &v + 1);
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

		GraphicsBufferPtr vb = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read, &init_data);
		rl_->BindVertexStream(vb, boost::make_tuple(vertex_element(VEU_Position, 0, EF_BGR32F)));

		box_ = MathLib::compute_bounding_box<float>(&xyzs[0], &xyzs[0] + sizeof(xyzs) / sizeof(xyzs[0]));
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

		GraphicsBufferPtr vb = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read, &init_data);
		rl_->BindVertexStream(vb, boost::make_tuple(vertex_element(VEU_Position, 0, EF_BGR32F)));

		box_ = MathLib::compute_bounding_box<float>(&xyzs[0], &xyzs[0] + sizeof(xyzs) / sizeof(xyzs[0]));
	}

	void RenderableTriangle::OnRenderBegin()
	{
		App3DFramework const & app = Context::Instance().AppInstance();
		Camera const & camera = app.ActiveCamera();

		float4x4 view_proj = camera.ViewMatrix() * camera.ProjMatrix();
		*matViewProj_ep_ = view_proj;
	}


	RenderableTriBox::RenderableTriBox(Box const & box, Color const & clr)
		: RenderableHelper(L"TriBox")
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();

		box_ = box;

		technique_ = rf.LoadEffect("RenderableHelper.fxml")->TechniqueByName("BoxTec");
		color_ep_ = technique_->Effect().ParameterByName("color");
		matViewProj_ep_ = technique_->Effect().ParameterByName("matViewProj");
		*color_ep_ = float4(clr.r(), clr.g(), clr.b(), clr.a());

		float3 xyzs[] =
		{
			box[0], box[1], box[2], box[3], box[4], box[5], box[6], box[7]
		};

		uint16_t indices[] =
		{
			0, 2, 3, 3, 1, 0,
			5, 7, 6, 6, 4, 5,
			4, 0, 1, 1, 5, 4,
			4, 6, 2, 2, 0, 4,
			2, 6, 7, 7, 3, 1,
			1, 3, 7, 7, 5, 1,
		};

		rl_ = rf.MakeRenderLayout();
		rl_->TopologyType(RenderLayout::TT_TriangleList);

		ElementInitData init_data;
		init_data.row_pitch = sizeof(xyzs);
		init_data.slice_pitch = 0;
		init_data.data = xyzs;

		GraphicsBufferPtr vb = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read, &init_data);
		rl_->BindVertexStream(vb, boost::make_tuple(vertex_element(VEU_Position, 0, EF_BGR32F)));

		init_data.row_pitch = sizeof(indices);
		init_data.slice_pitch = 0;
		init_data.data = indices;

		GraphicsBufferPtr ib = rf.MakeIndexBuffer(BU_Static, EAH_GPU_Read, &init_data);
		rl_->BindIndexStream(ib, EF_R16UI);
	}

	void RenderableTriBox::OnRenderBegin()
	{
		App3DFramework const & app = Context::Instance().AppInstance();
		Camera const & camera = app.ActiveCamera();

		float4x4 view_proj = camera.ViewMatrix() * camera.ProjMatrix();
		*matViewProj_ep_ = view_proj;
	}


	RenderableLineBox::RenderableLineBox(Box const & box, Color const & clr)
		: RenderableHelper(L"LineBox")
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();

		box_ = box;

		technique_ = rf.LoadEffect("RenderableHelper.fxml")->TechniqueByName("BoxTec");
		color_ep_ = technique_->Effect().ParameterByName("color");
		matViewProj_ep_ = technique_->Effect().ParameterByName("matViewProj");
		*color_ep_ = float4(clr.r(), clr.g(), clr.b(), clr.a());

		float3 xyzs[] =
		{
			box[0], box[1], box[2], box[3], box[4], box[5], box[6], box[7]
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

		GraphicsBufferPtr vb = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read, &init_data);
		rl_->BindVertexStream(vb, boost::make_tuple(vertex_element(VEU_Position, 0, EF_BGR32F)));

		init_data.row_pitch = sizeof(indices);
		init_data.slice_pitch = 0;
		init_data.data = indices;

		GraphicsBufferPtr ib = rf.MakeIndexBuffer(BU_Static, EAH_GPU_Read, &init_data);
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

		technique_ = rf.LoadEffect("RenderableHelper.fxml")->TechniqueByName("SkyBoxTec");

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

		GraphicsBufferPtr vb = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read, &init_data);
		rl_->BindVertexStream(vb, boost::make_tuple(vertex_element(VEU_Position, 0, EF_BGR32F)));

		box_ = MathLib::compute_bounding_box<float>(&xyzs[0], &xyzs[4]);

		skybox_cube_tex_ep_ = technique_->Effect().ParameterByName("skybox_cube_tex");
		inv_mvp_ep_ = technique_->Effect().ParameterByName("inv_mvp");
	}

	void RenderableSkyBox::CubeMap(TexturePtr const & cube)
	{
		*skybox_cube_tex_ep_ = cube;
	}

	void RenderableSkyBox::OnRenderBegin()
	{
		App3DFramework const & app = Context::Instance().AppInstance();
		Camera const & camera = app.ActiveCamera();

		float4x4 rot_view = camera.ViewMatrix();
		rot_view(3, 0) = 0;
		rot_view(3, 1) = 0;
		rot_view(3, 2) = 0;
		*inv_mvp_ep_ = MathLib::inverse(rot_view * camera.ProjMatrix());
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

		GraphicsBufferPtr pos_vb = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read, &init_data);
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

			GraphicsBufferPtr tex_vb = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read, &init_data);
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

		GraphicsBufferPtr ib = rf.MakeIndexBuffer(BU_Static, EAH_GPU_Read, &init_data);
		rl_->BindIndexStream(ib, EF_R16UI);

		box_ = MathLib::compute_bounding_box<float>(pos.begin(), pos.end());
	}
}
