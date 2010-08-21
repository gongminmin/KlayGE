// InfTerrain.cpp
// KlayGE Infinite Terrain implement file
// Ver 3.11.0
// Copyright(C) Minmin Gong, 2010
// Homepage: http://www.klayge.org
//
// 3.11.0
// First release (2010.8.21)
//
// CHANGE LIST
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/App3D.hpp>
#include <KlayGE/RenderLayout.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/Camera.hpp>

#include <KlayGE/InfTerrain.hpp>

namespace KlayGE
{
	InfTerrainRenderable::InfTerrainRenderable(std::wstring const & name)
		: RenderableHelper(name)
	{
		int const NX = 256;
		int const NY = 256;

		RenderFactory& rf = Context::Instance().RenderFactoryInstance();

		rl_ = rf.MakeRenderLayout();
		rl_->TopologyType(RenderLayout::TT_TriangleList);

		App3DFramework const & app = Context::Instance().AppInstance();
		Camera const & camera = app.ActiveCamera();

		float angle = atan(tan(camera.FOV() / 2) * camera.Aspect());
		x_dir_ = float2(-sin(angle), cos(angle));
		y_dir_ = float2(-x_dir_.x(), x_dir_.y());

		float2 addr(0, 0);
		float2 increment(1, 1);
		std::vector<float2> vertices;
		for (int y = 0; y < NY - 1; ++ y, addr.y() += increment.y())
		{
			increment.x() = 1;
			addr.x() = 0;
			for (int x = 0; x < NX - 1; ++ x, addr.x() += increment.x())
			{
				float2 p(addr.x() * x_dir_ * 0.5f + addr.y() * y_dir_ * 0.5f);
				vertices.push_back(p);
				increment.x() *= 1.012f;
			}
			{
				float2 p((addr.x() + 1000.0f) * x_dir_ * 0.5f + addr.y() * y_dir_ * 0.5f);
				vertices.push_back(p);
			}

			increment.y() *= 1.012f;
		}
		{
			increment.x() = 1;
			addr.x() = 0;
			for (int x = 0; x < NX - 1; ++ x, addr.x() += increment.x())
			{
				float2 p(addr.x() * x_dir_ * 0.5f + (addr.y() + 1000.0f) * y_dir_ * 0.5f);
				vertices.push_back(p);

				increment.x() *= 1.012f;
			}
			{
				float2 p((addr.x() + 1000.0f) * x_dir_ * 0.5f + (addr.y() + 1000.0f) * y_dir_ * 0.5f);
				vertices.push_back(p);
			}
		}

		ElementInitData init_data;
		init_data.data = &vertices[0];
		init_data.slice_pitch = init_data.row_pitch = static_cast<uint32_t>(vertices.size() * sizeof(vertices[0]));

		GraphicsBufferPtr pos_vb = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read, &init_data);
		rl_->BindVertexStream(pos_vb, boost::make_tuple(vertex_element(VEU_Position, 0, EF_GR32F)));

		std::vector<uint32_t> indices;
		for (uint32_t y = 0; y < NY - 1; ++ y)
		{
			for (uint32_t x = 0; x < NX - 1; ++ x)
			{
				indices.push_back((y + 0) * NX + (x + 0));
				indices.push_back((y + 0) * NX + (x + 1));
				indices.push_back((y + 1) * NX + (x + 0));

				indices.push_back((y + 1) * NX + (x + 0));
				indices.push_back((y + 0) * NX + (x + 1));
				indices.push_back((y + 1) * NX + (x + 1));
			}
		}

		init_data.data = &indices[0];
		init_data.slice_pitch = init_data.row_pitch = static_cast<uint32_t>(indices.size() * sizeof(indices[0]));

		GraphicsBufferPtr ib = rf.MakeIndexBuffer(BU_Static, EAH_GPU_Read, &init_data);
		rl_->BindIndexStream(ib, EF_R32UI);
	}

	InfTerrainRenderable::~InfTerrainRenderable()
	{
	}

	void InfTerrainRenderable::SetStretch(float stretch)
	{
		*(technique_->Effect().ParameterByName("stretch")) = stretch;
	}

	void InfTerrainRenderable::SetBaseLevel(float base_level)
	{
		*(technique_->Effect().ParameterByName("base_level")) = base_level;
	}

	void InfTerrainRenderable::OffsetY(float y)
	{
		*(technique_->Effect().ParameterByName("offset_y")) = y;
	}

	void InfTerrainRenderable::OnRenderBegin()
	{
		App3DFramework const & app = Context::Instance().AppInstance();
		Camera const & camera = app.ActiveCamera();

		float4x4 const & view = camera.ViewMatrix();
		float4x4 const & proj = camera.ProjMatrix();

		float3 look_at_vec = float3(camera.LookAt().x() - camera.EyePos().x(), 0, camera.LookAt().z() - camera.EyePos().z());
		if (MathLib::dot(look_at_vec, look_at_vec) < 1e-6f)
		{
			look_at_vec = float3(0, 0, 1);
		}
		float4x4 virtual_view = MathLib::look_at_lh(camera.EyePos(), camera.EyePos() + look_at_vec);
		float4x4 inv_virtual_view = MathLib::inverse(virtual_view);

		float4x4 vp = view * proj;
		*(technique_->Effect().ParameterByName("mvp")) = vp;
		*(technique_->Effect().ParameterByName("inv_virtual_view")) = inv_virtual_view;
		*(technique_->Effect().ParameterByName("eye_pos")) = camera.EyePos();
	}


	InfTerrainSceneObject::InfTerrainSceneObject()
		: SceneObjectHelper(SOA_Moveable)
	{
	}

	InfTerrainSceneObject::~InfTerrainSceneObject()
	{
	}

	void InfTerrainSceneObject::Update()
	{
		App3DFramework const & app = Context::Instance().AppInstance();
		Camera const & camera = app.ActiveCamera();

		float3 look_at_vec = float3(camera.LookAt().x() - camera.EyePos().x(), 0, camera.LookAt().z() - camera.EyePos().z());
		if (MathLib::dot(look_at_vec, look_at_vec) < 1e-6f)
		{
			look_at_vec = float3(0, 0, 1);
		}
		float4x4 virtual_view = MathLib::look_at_lh(camera.EyePos(), camera.EyePos() + look_at_vec);
		float4x4 inv_virtual_view = MathLib::inverse(virtual_view);

		float4x4 const & view = camera.ViewMatrix();
		float4x4 const & proj = camera.ProjMatrix();
		float4x4 proj_to_virtual_view = MathLib::inverse(view * proj) * virtual_view;

		float2 const & x_dir_2d = checked_pointer_cast<InfTerrainRenderable>(renderable_)->XDir();
		float2 const & y_dir_2d = checked_pointer_cast<InfTerrainRenderable>(renderable_)->YDir();
		float3 x_dir(x_dir_2d.x(), -camera.EyePos().y(), x_dir_2d.y());
		float3 y_dir(y_dir_2d.x(), -camera.EyePos().y(), y_dir_2d.y());

		float3 const frustum[8] = 
		{
			MathLib::transform_coord(float3(-1, +1, 1), proj_to_virtual_view),
			MathLib::transform_coord(float3(+1, +1, 1), proj_to_virtual_view),
			MathLib::transform_coord(float3(-1, -1, 1), proj_to_virtual_view),
			MathLib::transform_coord(float3(+1, -1, 1), proj_to_virtual_view),
			MathLib::transform_coord(float3(-1, +1, 0), proj_to_virtual_view),
			MathLib::transform_coord(float3(+1, +1, 0), proj_to_virtual_view),
			MathLib::transform_coord(float3(-1, -1, 0), proj_to_virtual_view),
			MathLib::transform_coord(float3(+1, -1, 0), proj_to_virtual_view)
		};

		int const view_cube[24] =
		{
			0, 1, 1, 3, 3, 2, 2, 0,
			4, 5, 5, 7, 7, 6, 6, 4,
			0, 4, 1, 5, 3, 7, 2, 6
		};

		Plane const lower_bound = MathLib::from_point_normal(float3(0, base_level_ - camera.EyePos().y() - strength_, 0), float3(0, 1, 0));

		bool intersect = false;
		float sy = 0;
		for (int i = 0; i < 12; ++ i)
		{
			int src = view_cube[i * 2 + 0];
			int dst = view_cube[i * 2 + 1];
			if (MathLib::dot_coord(lower_bound, frustum[src]) / MathLib::dot_coord(lower_bound, frustum[dst]) < 0)
			{
				float t = MathLib::intersect_ray(lower_bound, frustum[src], frustum[dst] - frustum[src]);
				float3 p = MathLib::lerp(frustum[src], frustum[dst], t);
				sy = std::max(sy, std::max((x_dir.z() * p.x() - x_dir.x() * p.z()) / x_dir.x(),
					(y_dir.z() * p.x() - y_dir.x() * p.z()) / y_dir.x()));
				intersect = true;
			}
		}
		checked_pointer_cast<InfTerrainRenderable>(renderable_)->OffsetY(sy);

		this->Visible(intersect);
	}
}
