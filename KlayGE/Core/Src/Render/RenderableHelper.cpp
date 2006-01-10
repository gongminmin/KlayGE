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
#include <KlayGE/GraphicsBuffer.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/Sampler.hpp>
#include <KlayGE/App3D.hpp>

#include <boost/tuple/tuple.hpp>

#include <KlayGE/RenderableHelper.hpp>

namespace KlayGE
{
	RenderableHelper::RenderableHelper(std::wstring const & name)
		: name_(name)
	{
	}

	RenderEffectPtr RenderableHelper::GetRenderEffect() const
	{
		return effect_;
	}

	RenderLayoutPtr RenderableHelper::GetRenderLayout() const
	{
		return rl_;
	}

	Box RenderableHelper::GetBound() const
	{
		return box_;
	}

	std::wstring const & RenderableHelper::Name() const
	{
		return name_;
	}


	RenderablePoint::RenderablePoint(Vector3 const & v, Color const & clr)
		: RenderableHelper(L"Point"),
			clr_(clr.r(), clr.g(), clr.b(), clr.a())
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();

		effect_ = rf.LoadEffect("RenderableHelper.fx");
		effect_->ActiveTechnique("PointTec");

		rl_ = rf.MakeRenderLayout(RenderLayout::BT_PointList);

		GraphicsBufferPtr vb = rf.MakeVertexBuffer(BU_Static);
		vb->Resize(sizeof(v));
		{
			GraphicsBuffer::Mapper mapper(*vb, BA_Write_Only);
			std::copy(&v, &v + 1, mapper.Pointer<Vector3>());
		}
		rl_->BindVertexStream(vb, boost::make_tuple(vertex_element(VEU_Position, 0, sizeof(float), 3)));

		box_ = MathLib::ComputeBoundingBox<float>(&v, &v + 1);
	}

	void RenderablePoint::OnRenderBegin()
	{
		App3DFramework const & app = Context::Instance().AppInstance();
		Camera const & camera = app.ActiveCamera();

		Matrix4 view_proj = camera.ViewMatrix() * camera.ProjMatrix();
		*(effect_->ParameterByName("matViewProj")) = view_proj;

		*(effect_->ParameterByName("color")) = clr_;
	}


	RenderableLine::RenderableLine(Vector3 const & v0, Vector3 const & v1, Color const & clr)
		: RenderableHelper(L"Line"),
			clr_(clr.r(), clr.g(), clr.b(), clr.a())
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();

		effect_ = rf.LoadEffect("RenderableHelper.fx");
		effect_->ActiveTechnique("LineTec");

		Vector3 xyzs[] =
		{
			v0, v1
		};

		rl_ = rf.MakeRenderLayout(RenderLayout::BT_LineList);

		GraphicsBufferPtr vb = rf.MakeVertexBuffer(BU_Static);
		vb->Resize(sizeof(xyzs));
		{
			GraphicsBuffer::Mapper mapper(*vb, BA_Write_Only);
			std::copy(&xyzs[0], &xyzs[0] + sizeof(xyzs) / sizeof(xyzs[0]), mapper.Pointer<Vector3>());
		}
		rl_->BindVertexStream(vb, boost::make_tuple(vertex_element(VEU_Position, 0, sizeof(float), 3)));

		box_ = MathLib::ComputeBoundingBox<float>(&xyzs[0], &xyzs[0] + sizeof(xyzs) / sizeof(xyzs[0]));
	}

	void RenderableLine::OnRenderBegin()
	{
		App3DFramework const & app = Context::Instance().AppInstance();
		Camera const & camera = app.ActiveCamera();

		Matrix4 view_proj = camera.ViewMatrix() * camera.ProjMatrix();
		*(effect_->ParameterByName("matViewProj")) = view_proj;

		*(effect_->ParameterByName("color")) = clr_;
	}


	RenderableTriangle::RenderableTriangle(Vector3 const & v0, Vector3 const & v1, Vector3 const & v2, Color const & clr)
		: RenderableHelper(L"Triangle"),
			clr_(clr.r(), clr.g(), clr.b(), clr.a())
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();

		effect_ = rf.LoadEffect("RenderableHelper.fx");
		effect_->ActiveTechnique("TriangleTec");

		Vector3 xyzs[] =
		{
			v0, v1, v2
		};

		rl_ = rf.MakeRenderLayout(RenderLayout::BT_TriangleList);

		GraphicsBufferPtr vb = rf.MakeVertexBuffer(BU_Static);
		vb->Resize(sizeof(xyzs));
		{
			GraphicsBuffer::Mapper mapper(*vb, BA_Write_Only);
			std::copy(&xyzs[0], &xyzs[0] + sizeof(xyzs) / sizeof(xyzs[0]), mapper.Pointer<Vector3>());
		}
		rl_->BindVertexStream(vb, boost::make_tuple(vertex_element(VEU_Position, 0, sizeof(float), 3)));

		box_ = MathLib::ComputeBoundingBox<float>(&xyzs[0], &xyzs[0] + sizeof(xyzs) / sizeof(xyzs[0]));
	}

	void RenderableTriangle::OnRenderBegin()
	{
		App3DFramework const & app = Context::Instance().AppInstance();
		Camera const & camera = app.ActiveCamera();

		Matrix4 view_proj = camera.ViewMatrix() * camera.ProjMatrix();
		*(effect_->ParameterByName("matViewProj")) = view_proj;

		*(effect_->ParameterByName("color")) = clr_;
	}


	RenderableBox::RenderableBox(Box const & box, Color const & clr)
		: RenderableHelper(L"Box"),
			clr_(clr.r(), clr.g(), clr.b(), clr.a())
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();

		box_ = box;

		effect_ = rf.LoadEffect("RenderableHelper.fx");
		effect_->ActiveTechnique("BoxTec");

		Vector3 xyzs[] =
		{
			box[0], box[1], box[2], box[3], box[4], box[5], box[6], box[7]
		};

		uint16_t indices[] =
		{
			0, 1, 2, 2, 3, 0,
			7, 6, 5, 5, 4, 7,
			4, 0, 3, 3, 7, 4,
			4, 5, 1, 1, 0, 4,
			1, 5, 6, 6, 2, 1,
			3, 2, 6, 6, 7, 3,
		};

		rl_ = rf.MakeRenderLayout(RenderLayout::BT_TriangleList);

		GraphicsBufferPtr vb = rf.MakeVertexBuffer(BU_Static);
		vb->Resize(sizeof(xyzs));
		{
			GraphicsBuffer::Mapper mapper(*vb, BA_Write_Only);
			std::copy(&xyzs[0], &xyzs[0] + sizeof(xyzs) / sizeof(xyzs[0]), mapper.Pointer<Vector3>());
		}
		rl_->BindVertexStream(vb, boost::make_tuple(vertex_element(VEU_Position, 0, sizeof(float), 3)));

		GraphicsBufferPtr ib = rf.MakeIndexBuffer(BU_Static);
		ib->Resize(sizeof(indices));
		{
			GraphicsBuffer::Mapper mapper(*ib, BA_Write_Only);
			std::copy(indices, indices + sizeof(indices) / sizeof(indices[0]), mapper.Pointer<uint16_t>());
		}
		rl_->BindIndexStream(ib, IF_Index16);
	}

	void RenderableBox::OnRenderBegin()
	{
		App3DFramework const & app = Context::Instance().AppInstance();
		Camera const & camera = app.ActiveCamera();

		Matrix4 view_proj = camera.ViewMatrix() * camera.ProjMatrix();
		*(effect_->ParameterByName("matViewProj")) = view_proj;

		*(effect_->ParameterByName("color")) = clr_;
	}


	RenderableSkyBox::RenderableSkyBox()
		: RenderableHelper(L"SkyBox"),
			cube_sampler_(new Sampler)
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();

		effect_ = rf.LoadEffect("RenderableHelper.fx");
		effect_->ActiveTechnique("SkyBoxTec");

		Vector3 xyzs[] =
		{
			Vector3(1.0f, 1.0f, 1.0f),
			Vector3(1.0f, -1.0f, 1.0f),
			Vector3(-1.0f, -1.0f, 1.0f),
			Vector3(-1.0f, 1.0f, 1.0f),
		};

		uint16_t indices[] =
		{
			0, 1, 2, 2, 3, 0,
		};

		rl_ = rf.MakeRenderLayout(RenderLayout::BT_TriangleList);

		GraphicsBufferPtr vb = rf.MakeVertexBuffer(BU_Static);
		vb->Resize(sizeof(xyzs));
		{
			GraphicsBuffer::Mapper mapper(*vb, BA_Write_Only);
			std::copy(&xyzs[0], &xyzs[0] + sizeof(xyzs) / sizeof(xyzs[0]), mapper.Pointer<Vector3>());
		}
		rl_->BindVertexStream(vb, boost::make_tuple(vertex_element(VEU_Position, 0, sizeof(float), 3)));

		GraphicsBufferPtr ib = rf.MakeIndexBuffer(BU_Static);
		ib->Resize(sizeof(indices));
		{
			GraphicsBuffer::Mapper mapper(*ib, BA_Write_Only);
			std::copy(indices, indices + sizeof(indices) / sizeof(indices[0]), mapper.Pointer<uint16_t>());
		}
		rl_->BindIndexStream(ib, IF_Index16);

		box_ = MathLib::ComputeBoundingBox<float>(&xyzs[0], &xyzs[4]);

		cube_sampler_->Filtering(Sampler::TFO_Bilinear);
		cube_sampler_->AddressingMode(Sampler::TAT_Addr_U, Sampler::TAM_Clamp);
		cube_sampler_->AddressingMode(Sampler::TAT_Addr_V, Sampler::TAM_Clamp);
		cube_sampler_->AddressingMode(Sampler::TAT_Addr_W, Sampler::TAM_Clamp);
		*(effect_->ParameterByName("skybox_cubeMapSampler")) = cube_sampler_;
	}

	void RenderableSkyBox::CubeMap(TexturePtr const & cube)
	{
		cube_sampler_->SetTexture(cube);
	}

	void RenderableSkyBox::OnRenderBegin()
	{
		App3DFramework const & app = Context::Instance().AppInstance();
		Camera const & camera = app.ActiveCamera();

		Matrix4 rot_view = camera.ViewMatrix();
		rot_view(3, 0) = 0;
		rot_view(3, 1) = 0;
		rot_view(3, 2) = 0;
		*(effect_->ParameterByName("inv_mvp")) = MathLib::Inverse(rot_view * camera.ProjMatrix());
	}

	RenderablePlane::RenderablePlane(float length, float width,
				int length_segs, int width_segs, bool has_tex_coord)
			: RenderableHelper(L"RenderablePlane")
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();

		rl_ = rf.MakeRenderLayout(RenderLayout::BT_TriangleList);

		std::vector<Vector3> pos;
		for (int y = 0; y < width_segs + 1; ++ y)
		{
			for (int x = 0; x < length_segs + 1; ++ x)
			{
				pos.push_back(Vector3(x * (length / length_segs) - length / 2,
					-y * (width / width_segs) + width / 2, 0.0f));
			}
		}

		GraphicsBufferPtr pos_vb = rf.MakeVertexBuffer(BU_Static);
		pos_vb->Resize(sizeof(pos[0]) * pos.size());
		{
			GraphicsBuffer::Mapper mapper(*pos_vb, BA_Write_Only);
			std::copy(pos.begin(), pos.end(), mapper.Pointer<Vector3>());
		}
		rl_->BindVertexStream(pos_vb, boost::make_tuple(vertex_element(VEU_Position, 0, sizeof(float), 3)));

		if (has_tex_coord)
		{
			std::vector<Vector2> tex;
			for (int y = 0; y < width_segs + 1; ++ y)
			{
				for (int x = 0; x < length_segs + 1; ++ x)
				{
					tex.push_back(Vector2(static_cast<float>(x) / length_segs,
						static_cast<float>(y) / width_segs));
				}
			}

			GraphicsBufferPtr tex_vb = rf.MakeVertexBuffer(BU_Static);
			tex_vb->Resize(sizeof(tex[0]) * tex.size());
			{
				GraphicsBuffer::Mapper mapper(*tex_vb, BA_Write_Only);
				std::copy(tex.begin(), tex.end(), mapper.Pointer<Vector2>());
			}
			rl_->BindVertexStream(tex_vb, boost::make_tuple(vertex_element(VEU_TextureCoord, 0, sizeof(float), 2)));
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

		GraphicsBufferPtr ib = rf.MakeIndexBuffer(BU_Static);
		ib->Resize(static_cast<uint32_t>(index.size() * sizeof(index[0])));
		{
			GraphicsBuffer::Mapper mapper(*ib, BA_Write_Only);
			std::copy(index.begin(), index.end(), mapper.Pointer<uint16_t>());
		}
		rl_->BindIndexStream(ib, IF_Index16);

		box_ = MathLib::ComputeBoundingBox<float>(pos.begin(), pos.end());
	}
}
