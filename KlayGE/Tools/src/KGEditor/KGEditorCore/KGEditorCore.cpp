#include <KlayGE/KlayGE.hpp>
#include <KFL/CXX17/iterator.hpp>
#include <KFL/CustomizedStreamBuf.hpp>
#include <KFL/ErrorHandling.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/FrameBuffer.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/RenderableHelper.hpp>
#include <KlayGE/Camera.hpp>
#include <KlayGE/SceneNode.hpp>
#include <KlayGE/SkyBox.hpp>
#include <KlayGE/DeferredRenderingLayer.hpp>
#include <KlayGE/UI.hpp>
#include <KlayGE/Mesh.hpp>
#include <KlayGE/Window.hpp>
#include <KlayGE/SceneManager.hpp>
#include <KlayGE/Input.hpp>
#include <KlayGE/InputFactory.hpp>
#include <KFL/XMLDom.hpp>

#include <sstream>
#include <fstream>

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>

#include "KGEditorCore.hpp"

using namespace std;
using namespace KlayGE;

namespace
{
	class RenderAxis : public Renderable
	{
	public:
		explicit RenderAxis()
			: Renderable(L"Axis")
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();

			effect_ = SyncLoadRenderEffect("MVUtil.fxml");
			simple_forward_tech_ = effect_->TechniqueByName("SimpleAxisTech");

			float4 xyzs[] =
			{
				float4(0, 0, 0, 0),
				float4(1, 0, 0, 0),
				float4(0, 0, 0, 1),
				float4(0, 1, 0, 1),
				float4(0, 0, 0, 2),
				float4(0, 0, 1, 2),
			};

			rls_[0] = rf.MakeRenderLayout();
			rls_[0]->TopologyType(RenderLayout::TT_LineList);

			GraphicsBufferPtr pos_vb = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, sizeof(xyzs), xyzs);

			rls_[0]->BindVertexStream(pos_vb, VertexElement(VEU_Position, 0, EF_ABGR32F));

			pos_aabb_ = MathLib::compute_aabbox(&xyzs[0], &xyzs[0] + std::size(xyzs));
			tc_aabb_ = AABBox(float3(0, 0, 0), float3(0, 0, 0));

			effect_attrs_ |= EA_SimpleForward;
		}
	};

	class RenderableTranslationAxis : public Renderable
	{
	public:
		RenderableTranslationAxis()
			: Renderable(L"TranslationAxis")
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();

			effect_ = SyncLoadRenderEffect("MVUtil.fxml");
			simple_forward_tech_ = effect_->TechniqueByName("AxisTech");

			hl_axis_ep_ = effect_->ParameterByName("hl_axis");
			*hl_axis_ep_ = int3(0, 0, 0);

			float const RADIUS = 0.015f;
			int const SEGMENTS = 12;
			std::vector<float3> positions((3 * SEGMENTS + 2) * 3);
			std::vector<float> axises(positions.size());
			std::vector<float3> normals(positions.size());
			for (int i = 0; i < SEGMENTS; ++ i)
			{
				positions[0 * SEGMENTS + i] = float3(0, sin(i * 2 * PI / SEGMENTS) * RADIUS,
					cos(i * 2 * PI / SEGMENTS) * RADIUS);
				axises[0 * SEGMENTS + i] = 0;
			}
			for (int i = 0; i < SEGMENTS; ++ i)
			{
				positions[1 * SEGMENTS + i] = float3(0.8f, positions[0 * SEGMENTS + i].y(),
					positions[0 * SEGMENTS + i].z());
				axises[1 * SEGMENTS + i] = 0;
			}
			for (int i = 0; i < SEGMENTS; ++ i)
			{
				positions[2 * SEGMENTS + i] = float3(0.8f, positions[0 * SEGMENTS + i].y() * 5,
					positions[0 * SEGMENTS + i].z() * 5);
				axises[2 * SEGMENTS + i] = 0;
			}
			{
				positions[3 * SEGMENTS + 0] = float3(0.8f, 0, 0);
				axises[3 * SEGMENTS + 0] = 0;

				positions[3 * SEGMENTS + 1] = float3(1, 0, 0);
				axises[3 * SEGMENTS + 1] = 0;
			}

			for (int i = 0; i < 3 * SEGMENTS + 2; ++ i)
			{
				positions[(3 * SEGMENTS + 2) * 1 + i] = float3(-positions[i].y(), positions[i].x(), positions[i].z());
				axises[(3 * SEGMENTS + 2) * 1 + i] = 1;
				positions[(3 * SEGMENTS + 2) * 2 + i] = float3(-positions[i].z(), positions[i].y(), positions[i].x());
				axises[(3 * SEGMENTS + 2) * 2 + i] = 2;
			}

			std::vector<uint16_t> indices((4 * SEGMENTS * 3) * 3);
			for (uint16_t i = 0; i < SEGMENTS; ++ i)
			{
				indices[0 * SEGMENTS * 3 + i * 6 + 0] = 0 * SEGMENTS + i;
				indices[0 * SEGMENTS * 3 + i * 6 + 1] = 1 * SEGMENTS + i;
				indices[0 * SEGMENTS * 3 + i * 6 + 2] = 1 * SEGMENTS + ((i + 1) % SEGMENTS);

				indices[0 * SEGMENTS * 3 + i * 6 + 3] = 1 * SEGMENTS + ((i + 1) % SEGMENTS);
				indices[0 * SEGMENTS * 3 + i * 6 + 4] = 0 * SEGMENTS + ((i + 1) % SEGMENTS);
				indices[0 * SEGMENTS * 3 + i * 6 + 5] = 0 * SEGMENTS + i;
			}
			for (uint16_t i = 0; i < SEGMENTS; ++ i)
			{
				indices[2 * SEGMENTS * 3 + i * 3 + 0] = 3 * SEGMENTS + 0;
				indices[2 * SEGMENTS * 3 + i * 3 + 1] = 2 * SEGMENTS + i;
				indices[2 * SEGMENTS * 3 + i * 3 + 2] = 2 * SEGMENTS + ((i + 1) % SEGMENTS);
			}
			for (uint16_t i = 0; i < SEGMENTS; ++ i)
			{
				indices[3 * SEGMENTS * 3 + i * 3 + 0] = 2 * SEGMENTS + i;
				indices[3 * SEGMENTS * 3 + i * 3 + 1] = 3 * SEGMENTS + 1;
				indices[3 * SEGMENTS * 3 + i * 3 + 2] = 2 * SEGMENTS + ((i + 1) % SEGMENTS);
			}

			for (uint16_t i = 0; i < 4 * SEGMENTS * 3; ++ i)
			{
				indices[4 * SEGMENTS * 3 * 1 + i] = (3 * SEGMENTS + 2) * 1 + indices[i];
				indices[4 * SEGMENTS * 3 * 2 + i] = (3 * SEGMENTS + 2) * 2 + indices[i];
			}

			MathLib::compute_normal(normals.begin(), indices.begin(), indices.end(), positions.begin(), positions.end());

			rls_[0] = rf.MakeRenderLayout();
			rls_[0]->TopologyType(RenderLayout::TT_TriangleList);

			rls_[0]->BindVertexStream(rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable,
				static_cast<uint32_t>(positions.size() * sizeof(positions[0])), &positions[0]),
				VertexElement(VEU_Position, 0, EF_BGR32F));
			rls_[0]->BindVertexStream(rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable,
				static_cast<uint32_t>(axises.size() * sizeof(axises[0])), &axises[0]),
				VertexElement(VEU_TextureCoord, 0, EF_R32F));
			rls_[0]->BindVertexStream(rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable,
				static_cast<uint32_t>(normals.size() * sizeof(normals[0])), &normals[0]),
				VertexElement(VEU_Normal, 0, EF_BGR32F));

			rls_[0]->BindIndexStream(rf.MakeIndexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable,
				static_cast<uint32_t>(indices.size() * sizeof(indices[0])), &indices[0]), EF_R16UI);

			pos_aabb_ = MathLib::compute_aabbox(positions.begin(), positions.end());
			tc_aabb_ = AABBox(float3(0, 0, 0), float3(0, 0, 0));

			effect_attrs_ |= EA_SimpleForward;
		}

		void HighlightAxis(uint32_t axis)
		{
			*hl_axis_ep_ = int3(axis & 1UL, axis & 2UL, axis & 4UL);
		}

	private:
		RenderEffectParameter* hl_axis_ep_;
	};

	class RenderableRotationAxis : public Renderable
	{
	public:
		RenderableRotationAxis()
			: Renderable(L"RotationAxis")
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();

			effect_ = SyncLoadRenderEffect("MVUtil.fxml");
			simple_forward_tech_ = effect_->TechniqueByName("AxisTech");

			hl_axis_ep_ = effect_->ParameterByName("hl_axis");
			*hl_axis_ep_ = int3(0, 0, 0);

			float const RADIUS = 0.015f;
			int const SEGMENTS = 24;
			int const SEGMENTS2 = 12;

			std::vector<float3> positions(SEGMENTS * SEGMENTS2 * 3);
			std::vector<float> axises(positions.size());
			std::vector<float3> normals(positions.size());
			for (int i = 0; i < SEGMENTS; ++ i)
			{
				float const sx0 = sin(i * 2 * PI / SEGMENTS);
				float const cx0 = cos(i * 2 * PI / SEGMENTS);
				float3 center(0, sx0, cx0);
				for (int j = 0; j < SEGMENTS2; ++ j)
				{
					float const sx1 = sin(j * 2 * PI / SEGMENTS2);
					float const cx1 = cos(j * 2 * PI / SEGMENTS2);
					positions[i * SEGMENTS2 + j] = center + float3(cx1 * RADIUS,
						(sx1 * sx0 - cx0) * RADIUS, (sx1 * cx0 + sx0) * RADIUS);
					axises[i * SEGMENTS2 + j] = 0;
				}
			}
			for (int i = 0; i < SEGMENTS * SEGMENTS2; ++ i)
			{
				positions[SEGMENTS * SEGMENTS2 * 1 + i] = float3(-positions[i].y(), positions[i].x(), positions[i].z());
				axises[SEGMENTS * SEGMENTS2 * 1 + i] = 1;
				positions[SEGMENTS * SEGMENTS2 * 2 + i] = float3(-positions[i].z(), positions[i].y(), positions[i].x());
				axises[SEGMENTS * SEGMENTS2 * 2 + i] = 2;
			}

			std::vector<uint16_t> indices((SEGMENTS * SEGMENTS2 * 6) * 3);
			for (uint16_t i = 0; i < SEGMENTS; ++ i)
			{
				for (uint16_t j = 0; j < SEGMENTS2; ++ j)
				{
					indices[(i * SEGMENTS2 + j) * 6 + 0] = i * SEGMENTS2 + j;
					indices[(i * SEGMENTS2 + j) * 6 + 1] = ((i + 1) % SEGMENTS) * SEGMENTS2 + j;
					indices[(i * SEGMENTS2 + j) * 6 + 2] = ((i + 1) % SEGMENTS) * SEGMENTS2 + ((j + 1) % SEGMENTS2);

					indices[(i * SEGMENTS2 + j) * 6 + 3] = ((i + 1) % SEGMENTS) * SEGMENTS2 + ((j + 1) % SEGMENTS2);
					indices[(i * SEGMENTS2 + j) * 6 + 4] = i * SEGMENTS2 + ((j + 1) % SEGMENTS2);
					indices[(i * SEGMENTS2 + j) * 6 + 5] = i * SEGMENTS2 + j;
				}
			}
			for (uint16_t i = 0; i < SEGMENTS * SEGMENTS2 * 6; ++ i)
			{
				indices[SEGMENTS * SEGMENTS2 * 6 * 1 + i] = SEGMENTS * SEGMENTS2 * 1 + indices[i];
				indices[SEGMENTS * SEGMENTS2 * 6 * 2 + i] = SEGMENTS * SEGMENTS2 * 2 + indices[i];
			}

			MathLib::compute_normal(normals.begin(), indices.begin(), indices.end(), positions.begin(), positions.end());

			rls_[0] = rf.MakeRenderLayout();
			rls_[0]->TopologyType(RenderLayout::TT_TriangleList);

			rls_[0]->BindVertexStream(rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable,
				static_cast<uint32_t>(positions.size() * sizeof(positions[0])), &positions[0]),
				VertexElement(VEU_Position, 0, EF_BGR32F));
			rls_[0]->BindVertexStream(rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable,
				static_cast<uint32_t>(axises.size() * sizeof(axises[0])), &axises[0]),
				VertexElement(VEU_TextureCoord, 0, EF_R32F));
			rls_[0]->BindVertexStream(rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable,
				static_cast<uint32_t>(normals.size() * sizeof(normals[0])), &normals[0]),
				VertexElement(VEU_Normal, 0, EF_BGR32F));

			rls_[0]->BindIndexStream(rf.MakeIndexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable,
				static_cast<uint32_t>(indices.size() * sizeof(indices[0])), &indices[0]), EF_R16UI);

			pos_aabb_ = MathLib::compute_aabbox(positions.begin(), positions.end());
			tc_aabb_ = AABBox(float3(0, 0, 0), float3(0, 0, 0));

			effect_attrs_ |= EA_SimpleForward;
		}

		void HighlightAxis(uint32_t axis)
		{
			*hl_axis_ep_ = int3(axis & 1UL, axis & 2UL, axis & 4UL);
		}

	private:
		RenderEffectParameter* hl_axis_ep_;
	};

	class RenderableScalingAxis : public Renderable
	{
	public:
		RenderableScalingAxis()
			: Renderable(L"ScalingAxis")
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();

			effect_ = SyncLoadRenderEffect("MVUtil.fxml");
			simple_forward_tech_ = effect_->TechniqueByName("AxisTech");

			hl_axis_ep_ = effect_->ParameterByName("hl_axis");
			*hl_axis_ep_ = int3(0, 0, 0);

			float const RADIUS = 0.015f;
			int const SEGMENTS = 12;
			std::vector<float3> positions((2 * SEGMENTS + 8) * 3);
			std::vector<float> axises(positions.size());
			std::vector<float3> normals(positions.size());
			for (int i = 0; i < SEGMENTS; ++ i)
			{
				positions[0 * SEGMENTS + i] = float3(0, sin(i * 2 * PI / SEGMENTS) * RADIUS,
					cos(i * 2 * PI / SEGMENTS) * RADIUS);
				axises[0 * SEGMENTS + i] = 0;
			}
			for (int i = 0; i < SEGMENTS; ++ i)
			{
				positions[1 * SEGMENTS + i] = float3(1 - RADIUS * 10, positions[0 * SEGMENTS + i].y(),
					positions[0 * SEGMENTS + i].z());
				axises[1 * SEGMENTS + i] = 0;
			}
			{
				positions[2 * SEGMENTS + 0] = float3(1 - RADIUS * 10, +RADIUS * 5, +RADIUS * 5);
				positions[2 * SEGMENTS + 1] = float3(1 - RADIUS * 10, +RADIUS * 5, -RADIUS * 5);
				positions[2 * SEGMENTS + 2] = float3(1 - RADIUS * 10, -RADIUS * 5, +RADIUS * 5);
				positions[2 * SEGMENTS + 3] = float3(1 - RADIUS * 10, -RADIUS * 5, -RADIUS * 5);
				axises[2 * SEGMENTS + 0] = 0;
				axises[2 * SEGMENTS + 1] = 0;
				axises[2 * SEGMENTS + 2] = 0;
				axises[2 * SEGMENTS + 3] = 0;

				positions[2 * SEGMENTS + 4] = float4(1, +RADIUS * 5, +RADIUS * 5, 0);
				positions[2 * SEGMENTS + 5] = float4(1, +RADIUS * 5, -RADIUS * 5, 0);
				positions[2 * SEGMENTS + 6] = float4(1, -RADIUS * 5, +RADIUS * 5, 0);
				positions[2 * SEGMENTS + 7] = float4(1, -RADIUS * 5, -RADIUS * 5, 0);
				axises[2 * SEGMENTS + 4] = 0;
				axises[2 * SEGMENTS + 5] = 0;
				axises[2 * SEGMENTS + 6] = 0;
				axises[2 * SEGMENTS + 7] = 0;
			}

			for (int i = 0; i < 2 * SEGMENTS + 8; ++ i)
			{
				positions[(2 * SEGMENTS + 8) * 1 + i] = float3(-positions[i].y(), positions[i].x(), positions[i].z());
				axises[(2 * SEGMENTS + 8) * 1 + i] = 1;
				positions[(2 * SEGMENTS + 8) * 2 + i] = float3(-positions[i].z(), positions[i].y(), positions[i].x());
				axises[(2 * SEGMENTS + 8) * 2 + i] = 2;
			}

			std::vector<uint16_t> indices(((2 * SEGMENTS + 12) * 3) * 3);
			for (uint16_t i = 0; i < SEGMENTS; ++ i)
			{
				indices[0 * SEGMENTS * 3 + i * 6 + 0] = 0 * SEGMENTS + i;
				indices[0 * SEGMENTS * 3 + i * 6 + 1] = 1 * SEGMENTS + i;
				indices[0 * SEGMENTS * 3 + i * 6 + 2] = 1 * SEGMENTS + ((i + 1) % SEGMENTS);

				indices[0 * SEGMENTS * 3 + i * 6 + 3] = 1 * SEGMENTS + ((i + 1) % SEGMENTS);
				indices[0 * SEGMENTS * 3 + i * 6 + 4] = 0 * SEGMENTS + ((i + 1) % SEGMENTS);
				indices[0 * SEGMENTS * 3 + i * 6 + 5] = 0 * SEGMENTS + i;
			}
			{
				indices[2 * SEGMENTS * 3 + 0 * 3 + 0] = 2 * SEGMENTS + 0;
				indices[2 * SEGMENTS * 3 + 0 * 3 + 1] = 2 * SEGMENTS + 1;
				indices[2 * SEGMENTS * 3 + 0 * 3 + 2] = 2 * SEGMENTS + 3;
				indices[2 * SEGMENTS * 3 + 1 * 3 + 0] = 2 * SEGMENTS + 3;
				indices[2 * SEGMENTS * 3 + 1 * 3 + 1] = 2 * SEGMENTS + 2;
				indices[2 * SEGMENTS * 3 + 1 * 3 + 2] = 2 * SEGMENTS + 0;

				indices[2 * SEGMENTS * 3 + 2 * 3 + 0] = 2 * SEGMENTS + 1;
				indices[2 * SEGMENTS * 3 + 2 * 3 + 1] = 2 * SEGMENTS + 5;
				indices[2 * SEGMENTS * 3 + 2 * 3 + 2] = 2 * SEGMENTS + 7;
				indices[2 * SEGMENTS * 3 + 3 * 3 + 0] = 2 * SEGMENTS + 7;
				indices[2 * SEGMENTS * 3 + 3 * 3 + 1] = 2 * SEGMENTS + 3;
				indices[2 * SEGMENTS * 3 + 3 * 3 + 2] = 2 * SEGMENTS + 1;

				indices[2 * SEGMENTS * 3 + 4 * 3 + 0] = 2 * SEGMENTS + 5;
				indices[2 * SEGMENTS * 3 + 4 * 3 + 1] = 2 * SEGMENTS + 4;
				indices[2 * SEGMENTS * 3 + 4 * 3 + 2] = 2 * SEGMENTS + 6;
				indices[2 * SEGMENTS * 3 + 5 * 3 + 0] = 2 * SEGMENTS + 6;
				indices[2 * SEGMENTS * 3 + 5 * 3 + 1] = 2 * SEGMENTS + 7;
				indices[2 * SEGMENTS * 3 + 5 * 3 + 2] = 2 * SEGMENTS + 5;

				indices[2 * SEGMENTS * 3 + 6 * 3 + 0] = 2 * SEGMENTS + 4;
				indices[2 * SEGMENTS * 3 + 6 * 3 + 1] = 2 * SEGMENTS + 0;
				indices[2 * SEGMENTS * 3 + 6 * 3 + 2] = 2 * SEGMENTS + 2;
				indices[2 * SEGMENTS * 3 + 7 * 3 + 0] = 2 * SEGMENTS + 2;
				indices[2 * SEGMENTS * 3 + 7 * 3 + 1] = 2 * SEGMENTS + 6;
				indices[2 * SEGMENTS * 3 + 7 * 3 + 2] = 2 * SEGMENTS + 4;

				indices[2 * SEGMENTS * 3 + 8 * 3 + 0] = 2 * SEGMENTS + 4;
				indices[2 * SEGMENTS * 3 + 8 * 3 + 1] = 2 * SEGMENTS + 5;
				indices[2 * SEGMENTS * 3 + 8 * 3 + 2] = 2 * SEGMENTS + 1;
				indices[2 * SEGMENTS * 3 + 9 * 3 + 0] = 2 * SEGMENTS + 1;
				indices[2 * SEGMENTS * 3 + 9 * 3 + 1] = 2 * SEGMENTS + 0;
				indices[2 * SEGMENTS * 3 + 9 * 3 + 2] = 2 * SEGMENTS + 4;

				indices[2 * SEGMENTS * 3 + 10 * 3 + 0] = 2 * SEGMENTS + 2;
				indices[2 * SEGMENTS * 3 + 10 * 3 + 1] = 2 * SEGMENTS + 3;
				indices[2 * SEGMENTS * 3 + 10 * 3 + 2] = 2 * SEGMENTS + 7;
				indices[2 * SEGMENTS * 3 + 11 * 3 + 0] = 2 * SEGMENTS + 7;
				indices[2 * SEGMENTS * 3 + 11 * 3 + 1] = 2 * SEGMENTS + 6;
				indices[2 * SEGMENTS * 3 + 11 * 3 + 2] = 2 * SEGMENTS + 2;
			}

			for (uint16_t i = 0; i < (2 * SEGMENTS + 12) * 3; ++ i)
			{
				indices[(2 * SEGMENTS + 12) * 3 * 1 + i] = (2 * SEGMENTS + 8) * 1 + indices[i];
				indices[(2 * SEGMENTS + 12) * 3 * 2 + i] = (2 * SEGMENTS + 8) * 2 + indices[i];
			}

			MathLib::compute_normal(normals.begin(), indices.begin(), indices.end(), positions.begin(), positions.end());

			rls_[0] = rf.MakeRenderLayout();
			rls_[0]->TopologyType(RenderLayout::TT_TriangleList);

			rls_[0]->BindVertexStream(rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable,
				static_cast<uint32_t>(positions.size() * sizeof(positions[0])), &positions[0]),
				VertexElement(VEU_Position, 0, EF_BGR32F));
			rls_[0]->BindVertexStream(rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable,
				static_cast<uint32_t>(axises.size() * sizeof(axises[0])), &axises[0]),
				VertexElement(VEU_TextureCoord, 0, EF_R32F));
			rls_[0]->BindVertexStream(rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable,
				static_cast<uint32_t>(normals.size() * sizeof(normals[0])), &normals[0]),
				VertexElement(VEU_Normal, 0, EF_BGR32F));

			rls_[0]->BindIndexStream(rf.MakeIndexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable,
				static_cast<uint32_t>(indices.size() * sizeof(indices[0])), &indices[0]), EF_R16UI);

			pos_aabb_ = MathLib::compute_aabbox(positions.begin(), positions.end());
			tc_aabb_ = AABBox(float3(0, 0, 0), float3(0, 0, 0));

			effect_attrs_ |= EA_SimpleForward;
		}

		void HighlightAxis(uint32_t axis)
		{
			*hl_axis_ep_ = int3(axis & 1UL, axis & 2UL, axis & 4UL);
		}

	private:
		RenderEffectParameter* hl_axis_ep_;
	};

	class RenderGrid : public Renderable
	{
	public:
		RenderGrid()
			: Renderable(L"Grid")
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();

			effect_ = SyncLoadRenderEffect("MVUtil.fxml");
			simple_forward_tech_ = effect_->TechniqueByName("GridTech");

			int const GRID_SIZE = 50;

			float3 xyzs[(2 * GRID_SIZE + 1 + 2 * GRID_SIZE + 1) * 2];
			for (int i = 0; i < 2 * GRID_SIZE + 1; ++ i)
			{
				xyzs[i * 2 + 0] = float3(static_cast<float>(-GRID_SIZE + i), 0, static_cast<float>(-GRID_SIZE));
				xyzs[i * 2 + 1] = float3(static_cast<float>(-GRID_SIZE + i), 0, static_cast<float>(+GRID_SIZE));

				xyzs[(i + 2 * GRID_SIZE + 1) * 2 + 0] = float3(static_cast<float>(-GRID_SIZE), 0, static_cast<float>(-GRID_SIZE + i));
				xyzs[(i + 2 * GRID_SIZE + 1) * 2 + 1] = float3(static_cast<float>(+GRID_SIZE), 0, static_cast<float>(-GRID_SIZE + i));
			}

			rls_[0] = rf.MakeRenderLayout();
			rls_[0]->TopologyType(RenderLayout::TT_LineList);

			GraphicsBufferPtr pos_vb = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, sizeof(xyzs), xyzs);
			rls_[0]->BindVertexStream(pos_vb, VertexElement(VEU_Position, 0, EF_BGR32F));

			pos_aabb_ = MathLib::compute_aabbox(&xyzs[0], &xyzs[0] + std::size(xyzs));
			tc_aabb_ = AABBox(float3(0, 0, 0), float3(0, 0, 0));

			effect_attrs_ |= EA_SimpleForward;
		}
	};
}

namespace KlayGE
{
	KGEditorCore::KGEditorCore(void* native_wnd)
				: App3DFramework("Editor", native_wnd),
					selected_entity_(0), display_scaling_for_axis_(1, 1, 1), active_camera_id_(0),
					ctrl_mode_(CM_EntitySelection), selected_axis_(SA_None),
					mouse_down_in_wnd_(false), mouse_tracking_mode_(false),
					tb_controller_(false), update_selective_buffer_(false),
					last_entity_id_(0)
	{
		ResLoader::Instance().AddPath("../../Tools/media/KGEditor");
	}

	void KGEditorCore::Resize(uint32_t width, uint32_t height)
	{
		Context::Instance().RenderFactoryInstance().RenderEngineInstance().Resize(width, height);

		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		RenderEngine& re = rf.RenderEngineInstance();
		deferred_rendering_->SetupViewport(0, re.CurFrameBuffer(), 0);

		auto const fmt = re.DeviceCaps().BestMatchRenderTargetFormat(MakeSpan({EF_ABGR8, EF_ABGR8}), 1, 0);
		BOOST_ASSERT(fmt != EF_Unknown);

		selective_tex_ = rf.MakeTexture2D(width, height, 1, 1, fmt, 1, 0, EAH_GPU_Write);
		selective_cpu_tex_ = rf.MakeTexture2D(width, height, 1, 1, fmt, 1, 0, EAH_CPU_Read);
		selective_fb_->Attach(FrameBuffer::Attachment::Color0, rf.Make2DRtv(selective_tex_, 0, 1, 0));
		selective_fb_->Attach(rf.Make2DDsv(width, height, EF_D24S8, 1, 0));

		update_selective_buffer_ = true;
	}

	void KGEditorCore::OnCreate()
	{
		scene_name_ = "Scene";
		skybox_name_ = "";

		font_ = SyncLoadFont("gkai00mp.kfont");

		this->LookAt(float3(5, 5, 5), float3(0, 1, 0));
		this->Proj(0.01f, 100);

		deferred_rendering_ = Context::Instance().DeferredRenderingLayerInstance();

		axis_ = MakeSharedPtr<SceneNode>(MakeSharedPtr<RenderableComponent>(MakeSharedPtr<RenderAxis>()),
			SceneNode::SOA_Cullable | SceneNode::SOA_Moveable | SceneNode::SOA_NotCastShadow);
		Context::Instance().SceneManagerInstance().SceneRootNode().AddChild(axis_);

		grid_ = MakeSharedPtr<SceneNode>(MakeSharedPtr<RenderableComponent>(MakeSharedPtr<RenderGrid>()),
			SceneNode::SOA_Cullable | SceneNode::SOA_Moveable | SceneNode::SOA_NotCastShadow);
		Context::Instance().SceneManagerInstance().SceneRootNode().AddChild(grid_);

		auto skybox_renderable = MakeSharedPtr<RenderableSkyBox>();
		skybox_ = MakeSharedPtr<SceneNode>(MakeSharedPtr<RenderableComponent>(skybox_renderable), SceneNode::SOA_NotCastShadow);
		skybox_renderable->CompressedCubeMap(SyncLoadTexture("default_bg_y.dds", EAH_GPU_Read | EAH_Immutable),
			SyncLoadTexture("default_bg_c.dds", EAH_GPU_Read | EAH_Immutable));
		Context::Instance().SceneManagerInstance().SceneRootNode().AddChild(skybox_);

		auto line_box_renderable = MakeSharedPtr<RenderableLineBox>();
		selected_bb_ = MakeSharedPtr<SceneNode>(MakeSharedPtr<RenderableComponent>(line_box_renderable),
			SceneNode::SOA_Moveable | SceneNode::SOA_NotCastShadow);
		selected_bb_->Visible(false);
		Context::Instance().SceneManagerInstance().SceneRootNode().AddChild(selected_bb_);
		line_box_renderable->SetColor(Color(1, 1, 1, 1));

		translation_axis_ = MakeSharedPtr<SceneNode>(MakeSharedPtr<RenderableComponent>(MakeSharedPtr<RenderableTranslationAxis>()),
			SceneNode::SOA_Moveable | SceneNode::SOA_NotCastShadow);
		translation_axis_->Visible(false);
		Context::Instance().SceneManagerInstance().SceneRootNode().AddChild(translation_axis_);

		rotation_axis_ = MakeSharedPtr<SceneNode>(MakeSharedPtr<RenderableComponent>(MakeSharedPtr<RenderableRotationAxis>()),
			SceneNode::SOA_Moveable | SceneNode::SOA_NotCastShadow);
		rotation_axis_->Visible(false);
		Context::Instance().SceneManagerInstance().SceneRootNode().AddChild(rotation_axis_);

		scaling_axis_ = MakeSharedPtr<SceneNode>(MakeSharedPtr<RenderableComponent>(MakeSharedPtr<RenderableScalingAxis>()),
			SceneNode::SOA_Moveable | SceneNode::SOA_NotCastShadow);
		scaling_axis_->Visible(false);
		Context::Instance().SceneManagerInstance().SceneRootNode().AddChild(scaling_axis_);

		tb_controller_.AttachCamera(this->ActiveCamera());
		tb_controller_.Scalers(0.01f, 0.05f);

		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		RenderEngine& re = rf.RenderEngineInstance();
		system_camera_ = re.CurFrameBuffer()->Viewport()->Camera();
		selective_fb_ = rf.MakeFrameBuffer();
		selective_fb_->Viewport()->Camera(system_camera_);

		this->UpdateSceneAABB();
	}

	void KGEditorCore::OnDestroy()
	{
		font_.reset();

		deferred_rendering_ = nullptr;

		axis_.reset();
		grid_.reset();
		skybox_.reset();
		selected_bb_.reset();
		translation_axis_.reset();
		rotation_axis_.reset();
		scaling_axis_.reset();
		system_camera_.reset();

		tb_controller_.DetachCamera();

		selective_fb_.reset();
		selective_tex_.reset();
		selective_cpu_tex_.reset();
	}

	void KGEditorCore::OnResize(uint32_t width, uint32_t height)
	{
		App3DFramework::OnResize(width, height);

		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		deferred_rendering_->SetupViewport(0, re.CurFrameBuffer(), 0);
	}

	void KGEditorCore::DoUpdateOverlay()
	{
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();

		std::wostringstream stream;
		stream.precision(2);
		stream << std::fixed << this->FPS() << " FPS";

		font_->RenderText(0, 0, Color(1, 1, 0, 1), re.ScreenFrameBuffer()->Description(), 16);
		font_->RenderText(0, 18, Color(1, 1, 0, 1), stream.str(), 16);
	}

	uint32_t KGEditorCore::DoUpdate(uint32_t pass)
	{
		if (0 == pass)
		{
			Camera const & camera = this->ActiveCamera();

			AABBox bb = MathLib::transform_aabb(scene_aabb_, camera.ViewMatrix());
			float near_plane = std::max(0.01f, bb.Min().z() * 0.8f);
			float far_plane = std::max(near_plane + 0.1f, bb.Max().z() * 1.2f);
			this->Proj(near_plane, far_plane);

			this->UpdateProxyScaling();
		}

		uint32_t deferrd_pass_start;
		if (update_selective_buffer_)
		{
			deferrd_pass_start = 2;

			RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
			if (0 == pass)
			{
				axis_->Visible(false);
				grid_->Visible(false);
				skybox_->Visible(false);
				selected_bb_->Visible(false);
				translation_axis_->Visible(false);
				rotation_axis_->Visible(false);
				scaling_axis_->Visible(false);
				for (auto iter = entities_.begin(); iter != entities_.end(); ++ iter)
				{
					auto const & model = *iter->second.model;
					for (uint32_t i = 0; i < model.NumMeshes(); ++ i)
					{
						model.Mesh(i)->SelectMode(true);
					}
				}

				re.BindFrameBuffer(selective_fb_);
				re.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth | FrameBuffer::CBM_Stencil,
					Color(0.0f, 0.0f, 0.0f, 1.0f), 1, 0);
				return App3DFramework::URV_NeedFlush;
			}
			else if (1 == pass)
			{
				axis_->Visible(true);
				grid_->Visible(true);
				skybox_->Visible(true);
				this->UpdateHelperObjs();
				for (auto iter = entities_.begin(); iter != entities_.end(); ++ iter)
				{
					auto const & model = *iter->second.model;
					for (uint32_t i = 0; i < model.NumMeshes(); ++ i)
					{
						model.Mesh(i)->SelectMode(false);
					}
				}

				re.BindFrameBuffer(FrameBufferPtr());
				return 0;
			}
		}
		else
		{
			deferrd_pass_start = 0;
		}

		if (pass >= deferrd_pass_start)
		{
			uint32_t deferred_pass = pass - deferrd_pass_start;
			if (0 == deferred_pass)
			{
				Camera const & camera = this->ActiveCamera();

				RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
				uint32_t width = re.CurFrameBuffer()->Width();
				uint32_t height = re.CurFrameBuffer()->Height();
				float x = (width - 70.5f) / width * 2 - 1;
				float y = 1 - (height - 70.5f) / height * 2;
				float3 origin = MathLib::transform_coord(float3(x, y, 0.1f), camera.InverseViewProjMatrix());

				float x1 = (width - 20.5f) / width * 2 - 1;
				float3 x_dir = MathLib::transform_coord(float3(x1, y, 0.1f), camera.InverseViewProjMatrix());
				float len = MathLib::length(x_dir - origin);

				float4x4 scaling = MathLib::scaling(len, len, len);
				float4x4 trans = MathLib::translation(origin);
				axis_->TransformToParent(scaling * trans);
			}

			uint32_t urv = deferred_rendering_->Update(deferred_pass);
			if (urv & App3DFramework::URV_Finished)
			{
				selective_tex_->CopyToTexture(*selective_cpu_tex_);
				update_selective_buffer_ = false;
			}

			return urv;
		}
		else
		{
			KFL_UNREACHABLE("Can't be here");
		}
	}

	std::string const & KGEditorCore::SceneName() const
	{
		return scene_name_;
	}

	void KGEditorCore::SceneName(std::string const & name)
	{
		scene_name_ = name;
	}

	std::string const & KGEditorCore::SkyboxName() const
	{
		return skybox_name_;
	}

	void KGEditorCore::SkyboxName(std::string const & name)
	{
		skybox_name_ = name;

		auto& skybox_renderable = skybox_->FirstComponentOfType<RenderableComponent>()->BoundRenderableOfType<RenderableSkyBox>();
		if (!skybox_name_.empty())
		{
			TexturePtr y_tex = SyncLoadTexture(name, EAH_GPU_Read | EAH_Immutable);
			TexturePtr c_tex;

			std::string::size_type pos = name.find_last_of('.');
			if ((pos > 0) && ('_' == name[pos - 2]))
			{
				if ('y' == name[pos - 1])
				{
					std::string c_name = name;
					c_name[pos - 1] = 'c';
					c_tex = SyncLoadTexture(c_name, EAH_GPU_Read | EAH_Immutable);
				}
				else if ('c' == name[pos - 1])
				{
					c_tex = y_tex;

					std::string y_name = name;
					y_name[pos - 1] = 'y';
					y_tex = SyncLoadTexture(y_name, EAH_GPU_Read | EAH_Immutable);
				}
			}

			if (c_tex)
			{
				skybox_renderable.CompressedCubeMap(y_tex, c_tex);
				if (ambient_light_)
				{
					ambient_light_->SkylightTex(y_tex, c_tex);
				}
			}
			else
			{
				skybox_renderable.CubeMap(y_tex);
				if (ambient_light_)
				{
					ambient_light_->SkylightTex(y_tex);
				}
			}
		}
		else
		{
			TexturePtr y_cube = SyncLoadTexture("default_bg_y.dds", EAH_GPU_Read | EAH_Immutable);
			TexturePtr c_cube = SyncLoadTexture("default_bg_y.dds", EAH_GPU_Read | EAH_Immutable);
			skybox_renderable.CompressedCubeMap(y_cube, c_cube);
			if (ambient_light_)
			{
				ambient_light_->SkylightTex(y_cube, c_cube);
			}
		}
	}

	void KGEditorCore::DisplaySSVO(bool ssvo)
	{
		deferred_rendering_->SSVOEnabled(0, ssvo);
	}

	void KGEditorCore::DisplayHDR(bool hdr)
	{
		Context::Instance().RenderFactoryInstance().RenderEngineInstance().HDREnabled(hdr);
	}

	void KGEditorCore::DisplayAA(bool aa)
	{
		Context::Instance().RenderFactoryInstance().RenderEngineInstance().PPAAEnabled(aa);
	}

	void KGEditorCore::DisplayGamma(bool gamma)
	{
		Context::Instance().RenderFactoryInstance().RenderEngineInstance().GammaEnabled(gamma);
	}

	void KGEditorCore::DisplayColorGrading(bool cg)
	{
		Context::Instance().RenderFactoryInstance().RenderEngineInstance().ColorGradingEnabled(cg);
	}

	void KGEditorCore::SetControlMode(KGEditorCore::ControlMode mode)
	{
		ctrl_mode_ = mode;

		this->UpdateHelperObjs();
	}

	uint32_t KGEditorCore::AddModel(std::string const & meshml_name)
	{
		uint32_t const entity_id = last_entity_id_ + 1;
		last_entity_id_ = entity_id;

		ResLoader::Instance().AddPath(meshml_name.substr(0, meshml_name.find_last_of('\\')));

		auto model = SyncLoadModel(meshml_name, EAH_GPU_Read | EAH_Immutable,
			SceneNode::SOA_Cullable | SceneNode::SOA_Moveable, AddToSceneRootHelper);
		auto scene_node = model->RootNode();
		for (size_t i = 0; i < model->NumMeshes(); ++ i)
		{
			model->Mesh(i)->ObjectID(entity_id);
		}

		EntityInfo mi;
		std::string::size_type begin = meshml_name.rfind('\\') + 1;
		std::string::size_type end = meshml_name.rfind('.') - begin;
		mi.name = meshml_name.substr(begin, end);
		mi.type = ET_Model;
		mi.model = model;
		mi.meshml_name = meshml_name;
		mi.obb = MathLib::convert_to_obbox(model->RootNode()->PosBoundOS());
		mi.trf_pivot = mi.obb.Center();
		mi.trf_pos = float3(0, 0, 0);
		mi.trf_scale = float3(1, 1, 1);
		mi.trf_rotate = Quaternion::Identity();
		mi.scene_node = scene_node;
		entities_.insert(std::make_pair(entity_id, mi));

		update_selective_buffer_ = true;
		this->UpdateSceneAABB();

		return entity_id;
	}

	void KGEditorCore::ClearModels()
	{
		for (auto iter = entities_.begin(); iter != entities_.end();)
		{
			if (ET_Model == iter->second.type)
			{
				Context::Instance().SceneManagerInstance().SceneRootNode().RemoveChild(iter->second.scene_node);
				entities_.erase(iter ++);
			}
			else
			{
				++ iter;
			}
		}

		update_selective_buffer_ = true;
		this->UpdateSceneAABB();
	}

	uint32_t KGEditorCore::AddLight(LightSource::LightType type, std::string const & name)
	{
		auto light_node = MakeSharedPtr<SceneNode>(SceneNode::SOA_Cullable | SceneNode::SOA_Moveable | SceneNode::SOA_NotCastShadow);

		LightSourcePtr light;
		switch (type)
		{
		case LightSource::LT_Ambient:
			light = MakeSharedPtr<AmbientLightSource>();

			ambient_light_ = light;
			if (skybox_c_cube_)
			{
				light->SkylightTex(skybox_y_cube_, skybox_c_cube_);
			}
			else
			{
				light->SkylightTex(skybox_y_cube_);
			}
			break;

		case LightSource::LT_Directional:
			light = MakeSharedPtr<DirectionalLightSource>();
			break;

		case LightSource::LT_Point:
			light = MakeSharedPtr<PointLightSource>();
			break;

		case LightSource::LT_Spot:
			light = MakeSharedPtr<SpotLightSource>();
			light->OuterAngle(PI / 4);
			light->InnerAngle(PI / 6);
			break;

		default:
			KFL_UNREACHABLE("Invalid light type");
			break;
		}
		light->Attrib(0);
		light->Color(float3(1, 1, 1));
		light->Falloff(float3(1, 0, 1));

		auto light_proxy = LoadLightSourceProxyModel(light);
		auto light_proxy_node = MakeSharedPtr<SceneNode>(SceneNode::SOA_Cullable | SceneNode::SOA_Moveable | SceneNode::SOA_NotCastShadow);
		light_proxy_node->AddChild(light_proxy->RootNode());
		light_node->AddChild(light_proxy_node);
		light_node->AddComponent(light);
		Context::Instance().SceneManagerInstance().SceneRootNode().AddChild(light_node);

		uint32_t const entity_id = last_entity_id_ + 1;
		last_entity_id_ = entity_id;
		for (size_t i = 0; i < light_proxy->NumMeshes(); ++ i)
		{
			light_proxy->Mesh(i)->ObjectID(entity_id);
		}

		EntityInfo li;
		li.name = name;
		li.type = ET_Light;
		li.model = light_proxy;
		li.light = light;
		li.obb = MathLib::convert_to_obbox(light_proxy->RootNode()->PosBoundOS());
		li.trf_pivot = float3(0, 0, 0);
		li.trf_pos = float3(0, 0, 0);
		li.trf_scale = float3(1, 1, 1);
		li.trf_rotate = Quaternion::Identity();
		li.scene_node = light_node;
		entities_.insert(std::make_pair(entity_id, li));

		update_selective_buffer_ = true;
		this->UpdateSceneAABB();

		return entity_id;
	}

	void KGEditorCore::ClearLights()
	{
		for (auto iter = entities_.begin(); iter != entities_.end();)
		{
			if (ET_Light == iter->second.type)
			{
				Context::Instance().SceneManagerInstance().SceneRootNode().RemoveChild(iter->second.scene_node);
				entities_.erase(iter ++);
			}
			else
			{
				++ iter;
			}
		}

		update_selective_buffer_ = true;
		this->UpdateSceneAABB();
	}

	uint32_t KGEditorCore::AddCamera(std::string const & name)
	{
		auto camera_node = MakeSharedPtr<SceneNode>(SceneNode::SOA_Cullable | SceneNode::SOA_Moveable | SceneNode::SOA_NotCastShadow);

		CameraPtr camera = MakeSharedPtr<Camera>();

		auto camera_proxy = LoadCameraProxyModel(camera);
		auto camera_proxy_node = MakeSharedPtr<SceneNode>(SceneNode::SOA_Cullable | SceneNode::SOA_Moveable | SceneNode::SOA_NotCastShadow);
		camera_proxy_node->AddChild(camera_proxy->RootNode());
		camera_node->AddChild(camera_proxy_node);
		camera_node->AddComponent(camera);
		Context::Instance().SceneManagerInstance().SceneRootNode().AddChild(camera_node);

		uint32_t const entity_id = last_entity_id_ + 1;
		last_entity_id_ = entity_id;
		for (size_t i = 0; i < camera_proxy->NumMeshes(); ++ i)
		{
			camera_proxy->Mesh(i)->ObjectID(entity_id);
		}

		EntityInfo ci;
		ci.name = name;
		ci.type = ET_Camera;
		ci.model = camera_proxy;
		ci.camera = camera;
		ci.obb = MathLib::convert_to_obbox(camera_proxy->RootNode()->PosBoundOS());
		ci.trf_pivot = float3(0, 0, 0);
		ci.trf_pos = float3(0, 0, 0);
		ci.trf_scale = float3(1, 1, 1);
		ci.trf_rotate = Quaternion::Identity();
		ci.scene_node = camera_node;
		entities_.insert(std::make_pair(entity_id, ci));

		update_selective_buffer_ = true;
		this->UpdateSceneAABB();

		return entity_id;
	}

	void KGEditorCore::ClearCameras()
	{
		for (auto iter = entities_.begin(); iter != entities_.end();)
		{
			if (ET_Camera == iter->second.type)
			{
				Context::Instance().SceneManagerInstance().SceneRootNode().RemoveChild(iter->second.scene_node);
				entities_.erase(iter ++);
			}
			else
			{
				++ iter;
			}
		}

		update_selective_buffer_ = true;
		this->UpdateSceneAABB();

		update_property_event_();
	}

	void KGEditorCore::RemoveEntity(uint32_t entity_id)
	{
		auto iter = entities_.find(entity_id);
		if (iter != entities_.end())
		{
			Context::Instance().SceneManagerInstance().SceneRootNode().RemoveChild(iter->second.scene_node);
			entities_.erase(iter);
		}

		update_selective_buffer_ = true;
		this->UpdateSceneAABB();

		update_property_event_();
	}

	void KGEditorCore::SelectEntity(uint32_t entity_id)
	{
		selected_entity_ = 0;
		auto iter = entities_.find(entity_id);
		if (iter != entities_.end())
		{
			selected_entity_ = entity_id;
		}

		this->UpdateHelperObjs();

		if (selected_entity_ > 0)
		{
			EntityInfo const & ei = entities_[selected_entity_];
			auto& line_box_renderable =
				selected_bb_->FirstComponentOfType<RenderableComponent>()->BoundRenderableOfType<RenderableLineBox>();
			line_box_renderable.SetBox(ei.obb);

			float3 proxy_scaling;
			float4x4 mat;
			switch (ei.type)
			{
			case ET_Light:
				mat = this->CalcAdaptiveScaling(ei, 25, proxy_scaling);
				break;

			case ET_Camera:
				mat = this->CalcAdaptiveScaling(ei, 75, proxy_scaling);
				break;

			default:
				mat = MathLib::transformation<float>(&ei.trf_pivot, nullptr, &ei.trf_scale,
					&ei.trf_pivot, &ei.trf_rotate, &ei.trf_pos);
				break;
			}
			selected_bb_->TransformToParent(mat);
		}

		update_property_event_();
	}

	std::string const & KGEditorCore::EntityName(uint32_t id)
	{
		return this->GetEntityInfo(id).name;
	}

	void KGEditorCore::EntityName(uint32_t id, std::string const & name)
	{
		this->GetEntityInfo(id).name = name;
	}

	bool KGEditorCore::EntityVisible(uint32_t id)
	{
		return this->GetEntityInfo(id).scene_node->Visible();
	}

	void KGEditorCore::EntityVisible(uint32_t id, bool visible)
	{
		auto& entity = this->GetEntityInfo(id);
		entity.scene_node->Visible(visible);
		if (entity.type == ET_Light)
		{
			entity.light->Enabled(visible);
		}

		update_selective_buffer_ = true;
		this->UpdateSceneAABB();
	}

	LightSourcePtr const & KGEditorCore::GetLight(uint32_t id) const
	{
		auto const & entity_info = this->GetEntityInfo(id);
		BOOST_ASSERT(ET_Light == entity_info.type);

		return entity_info.light;
	}

	std::string const & KGEditorCore::LightProjectiveTexName(uint32_t id) const
	{
		auto const & entity_info = this->GetEntityInfo(id);
		BOOST_ASSERT(ET_Light == entity_info.type);

		return entity_info.projective_tex_name;
	}

	void KGEditorCore::LightProjectiveTexName(uint32_t id, std::string const & name)
	{
		auto& entity_info = this->GetEntityInfo(id);
		BOOST_ASSERT(ET_Light == entity_info.type);

		entity_info.projective_tex_name = name;
		if (name.empty())
		{
			entity_info.light->ProjectiveTexture(TexturePtr());
		}
		else
		{
			entity_info.light->ProjectiveTexture(ASyncLoadTexture(name, EAH_GPU_Read | EAH_Immutable));
		}
	}

	CameraPtr const & KGEditorCore::GetCamera(uint32_t id) const
	{
		auto const & entity_info = this->GetEntityInfo(id);
		BOOST_ASSERT(ET_Camera == entity_info.type);

		return entity_info.camera;
	}

	float3 const & KGEditorCore::EntityScale(uint32_t id) const
	{
		return this->GetEntityInfo(id).trf_scale;
	}

	void KGEditorCore::EntityScale(uint32_t id, float3 const & s)
	{
		this->GetEntityInfo(id).trf_scale = s;
		this->UpdateSelectedEntity(false);
	}

	Quaternion const & KGEditorCore::EntityRotation(uint32_t id) const
	{
		return this->GetEntityInfo(id).trf_rotate;
	}

	void KGEditorCore::EntityRotation(uint32_t id, Quaternion const & r)
	{
		this->GetEntityInfo(id).trf_rotate = r;
		this->UpdateSelectedEntity(false);
	}

	float3 const & KGEditorCore::EntityPosition(uint32_t id) const
	{
		return this->GetEntityInfo(id).trf_pos;
	}

	void KGEditorCore::EntityPosition(uint32_t id, float3 const & t)
	{
		this->GetEntityInfo(id).trf_pos = t;
		this->UpdateSelectedEntity(false);
	}

	float3 const & KGEditorCore::EntityPivot(uint32_t id) const
	{
		return this->GetEntityInfo(id).trf_pivot;
	}

	void KGEditorCore::EntityPivot(uint32_t id, float3 const & t)
	{
		this->GetEntityInfo(id).trf_pivot = t;
		this->UpdateSelectedEntity(false);
	}

	uint32_t KGEditorCore::ActiveCameraId() const
	{
		return active_camera_id_;
	}

	void KGEditorCore::ActiveCameraId(uint32_t id)
	{
		active_camera_id_ = id;

		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		CameraPtr camera = (0 == id) ? system_camera_ : this->GetCamera(id);

		re.CurFrameBuffer()->Viewport()->Camera(camera);
		re.DefaultFrameBuffer()->Viewport()->Camera(camera);
		selective_fb_->Viewport()->Camera(camera);

		tb_controller_.AttachCamera(this->ActiveCamera());
	}

	void KGEditorCore::UpdateSelectedEntity(bool callback)
	{
		EntityInfo& ei = entities_[selected_entity_];
		float4x4 model_mat = MathLib::transformation<float>(&ei.trf_pivot, nullptr, &ei.trf_scale,
			&ei.trf_pivot, &ei.trf_rotate, &ei.trf_pos);

		float3 proxy_scaling;
		float4x4 mat;

		switch (ei.type)
		{
		case ET_Light:
			mat = this->CalcAdaptiveScaling(ei, 25, proxy_scaling);
			ei.scene_node->TransformToParent(mat);
			ei.scene_node->Children()[0]->TransformToParent(MathLib::scaling(proxy_scaling * ei.trf_scale));
			break;

		case ET_Camera:
			mat = this->CalcAdaptiveScaling(ei, 75, proxy_scaling);
			ei.camera->LookAtDist(MathLib::length(MathLib::transform_normal(float3(0, 0, 1), mat)));
			ei.camera->BoundSceneNode()->TransformToWorld(
				MathLib::inverse(MathLib::look_at_lh(ei.trf_pos, ei.trf_pos + MathLib::transform_normal(float3(0, 0, 1), mat))));
			ei.scene_node->Children()[0]->TransformToParent(MathLib::scaling(proxy_scaling * ei.trf_scale));
			break;

		default:
			ei.scene_node->TransformToParent(model_mat);
			break;
		}
		this->UpdateEntityAxis();
		this->UpdateSceneAABB();

		update_selective_buffer_ = true;

		if (callback)
		{
			update_property_event_();
		}
	}

	void KGEditorCore::UpdateEntityAxis()
	{
		if (selected_entity_ > 0)
		{
			EntityInfo const & oi = entities_[selected_entity_];

			float3 proxy_scaling;
			float4x4 mat = this->CalcAdaptiveScaling(oi, 100, proxy_scaling);

			translation_axis_->TransformToParent(mat);
			rotation_axis_->TransformToParent(mat);
			scaling_axis_->TransformToParent(mat);

			switch (oi.type)
			{
			case ET_Light:
				mat = this->CalcAdaptiveScaling(oi, 25, proxy_scaling);
				break;

			case ET_Camera:
				mat = this->CalcAdaptiveScaling(oi, 75, proxy_scaling);
				break;

			default:
				mat = MathLib::transformation<float>(&oi.trf_pivot, nullptr, &oi.trf_scale,
					&oi.trf_pivot, &oi.trf_rotate, &oi.trf_pos);
				break;
			}
			selected_bb_->TransformToParent(mat);
		}
	}

	void KGEditorCore::UpdateHelperObjs()
	{
		selected_bb_->Visible(false);
		translation_axis_->Visible(false);
		rotation_axis_->Visible(false);
		scaling_axis_->Visible(false);

		if (selected_entity_ > 0)
		{
			selected_bb_->Visible(true);
			switch (ctrl_mode_)
			{
			case CM_EntityTranslation:
				translation_axis_->Visible(true);
				this->UpdateEntityAxis();
				break;

			case CM_EntityRotation:
				rotation_axis_->Visible(true);
				this->UpdateEntityAxis();
				break;

			case CM_EntityScale:
				scaling_axis_->Visible(true);
				this->UpdateEntityAxis();
				break;

			default:
				break;
			}
		}
	}

	void KGEditorCore::UpdateSceneAABB()
	{
		scene_aabb_ = grid_->FirstComponentOfType<RenderableComponent>()->BoundRenderable().PosBound();

		for (auto const & entity : entities_)
		{
			EntityInfo const & ei = entity.second;
			if (ei.scene_node->Visible())
			{
				float4x4 model_mat = MathLib::transformation<float>(&ei.trf_pivot, nullptr, &ei.trf_scale,
					&ei.trf_pivot, &ei.trf_rotate, &ei.trf_pos);
				scene_aabb_ |= MathLib::convert_to_aabbox(MathLib::transform_obb(ei.obb, model_mat));
			}
		}
	}

	void KGEditorCore::UpdateProxyScaling()
	{
		for (auto const & entity : entities_)
		{
			EntityInfo const & ei = entity.second;

			if ((ET_Light == ei.type) || (ET_Camera == ei.type))
			{
				float3 proxy_scaling;
				float4x4 mat;

				if (ET_Light == ei.type)
				{
					mat = this->CalcAdaptiveScaling(ei, 25, proxy_scaling);
				}
				else
				{
					mat = this->CalcAdaptiveScaling(ei, 75, proxy_scaling);
				}
				ei.scene_node->Children()[0]->TransformToParent(MathLib::scaling(proxy_scaling * ei.trf_scale));

				if (selected_entity_ == entity.first)
				{
					selected_bb_->TransformToParent(mat);
				}
			}
		}
	}

	float4x4 KGEditorCore::CalcAdaptiveScaling(EntityInfo const & ei, uint32_t pixels, float3& scaling)
	{
		Camera const & camera = this->ActiveCamera();
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		uint32_t width = re.CurFrameBuffer()->Width();
		uint32_t height = re.CurFrameBuffer()->Height();

		int vp[4] = { 0, 0, static_cast<int>(width), static_cast<int>(height) };

		float4x4 model_mat_wo_scale = MathLib::transformation<float>(&ei.trf_pivot, nullptr, nullptr,
			&ei.trf_pivot, &ei.trf_rotate, &ei.trf_pos);

		float3 pivot_ss = MathLib::project(ei.trf_pivot, model_mat_wo_scale, camera.ViewMatrix(), camera.ProjMatrix(),
			vp, camera.NearPlane(), camera.FarPlane());
		float3 pivot_os = MathLib::transform_coord(ei.trf_pivot, model_mat_wo_scale);
		float3 pivot_ps = MathLib::transform_coord(ei.trf_pivot, model_mat_wo_scale * camera.ViewProjMatrix());

		float x = (pivot_ss.x() + pixels) / width * 2 - 1;
		float3 x_dir = MathLib::transform_coord(float3(x, pivot_ps.y(), pivot_ps.z()), camera.InverseViewProjMatrix());
		float len = MathLib::length(x_dir - pivot_os);
		scaling = float3(len, len, len);

		return MathLib::scaling(scaling) * MathLib::translation(ei.trf_pivot) * model_mat_wo_scale;
	}

	void KGEditorCore::MouseMove(int x, int y, uint32_t button)
	{
		int2 pt(x, y);

		if (0 == button)
		{
			if (selected_entity_ > 0)
			{
				RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
				FrameBufferPtr const & fb = re.CurFrameBuffer();
				uint32_t const width = fb->Width();
				uint32_t const height = fb->Height();

				float4x4 const & inv_mvp = MathLib::inverse(rotation_axis_->TransformToWorld() * this->ActiveCamera().ViewProjMatrix());
				float3 ray_target = MathLib::transform_coord(float3((pt.x() + 0.5f) / width * 2 - 1,
					1 - (pt.y() + 0.5f) / height * 2, 1), inv_mvp);
				float3 ray_origin = MathLib::transform_coord(float3(0, 0, 0),
					MathLib::inverse(rotation_axis_->TransformToWorld() * this->ActiveCamera().ViewMatrix()));
				float3 ray_dir = ray_target - ray_origin;

				float3 intersect_pt[3] =
				{
					ray_origin + MathLib::intersect_ray(Plane(1, 0, 0, 0), ray_origin, ray_dir) * ray_dir, // yoz
					ray_origin + MathLib::intersect_ray(Plane(0, 1, 0, 0), ray_origin, ray_dir) * ray_dir, // xoz
					ray_origin + MathLib::intersect_ray(Plane(0, 0, 1, 0), ray_origin, ray_dir) * ray_dir  // xoy
				};

				selected_axis_ = SA_None;

				switch (ctrl_mode_)
				{
				case CM_EntityTranslation:
					{
						float const DIST_THRESHOLD = 0.1f;

						{
							float min_dist = 1e10f;
							for (int i = 0; i < 3; ++ i)
							{
								int index0 = (i + 1) % 3;
								int index1 = (i + 2) % 3;
								float dist = MathLib::sqr(intersect_pt[index0][index1])
									+ MathLib::sqr(intersect_pt[index1][index0]);
								if ((intersect_pt[index0][i] > 0.4f) && (intersect_pt[index0][i] < 1.2f)
									&& (dist < DIST_THRESHOLD) && (dist < min_dist))
								{
									min_dist = dist;
									selected_axis_ = static_cast<SelectedAxis>(1UL << i);
								}
							}
						}
						if (SA_None == selected_axis_)
						{
							float min_dist = 1e10f;
							for (int i = 0; i < 3; ++ i)
							{
								int index0 = (i + 1) % 3;
								int index1 = (i + 2) % 3;
								float dist = MathLib::sqr(intersect_pt[i][index0])
									+ MathLib::sqr(intersect_pt[i][index1]);
								if ((intersect_pt[i][index0] < 0.4f) && (intersect_pt[i][index0] > 0)
									&& (intersect_pt[i][index1] < 0.4f) && (intersect_pt[i][index1] > 0)
									&& (dist < min_dist))
								{
									min_dist = dist;
									selected_axis_ = static_cast<SelectedAxis>((1UL << index0) | (1UL << index1));
								}
							}
						}

						translation_axis_->FirstComponentOfType<RenderableComponent>()
							->BoundRenderableOfType<RenderableTranslationAxis>()
							.HighlightAxis(selected_axis_);
					}
					break;

				case CM_EntityRotation:
					{
						float const DIST_THRESHOLD = 0.2f;

						float min_dist = 1e10f;
						for (int i = 0; i < 3; ++ i)
						{
							float dist = abs(1 - MathLib::length(intersect_pt[i]));
							if ((dist < DIST_THRESHOLD) && (dist < min_dist))
							{
								min_dist = dist;
								selected_axis_ = static_cast<SelectedAxis>(1UL << i);
							}
						}

						rotation_axis_->FirstComponentOfType<RenderableComponent>()
							->BoundRenderableOfType<RenderableRotationAxis>()
							.HighlightAxis(selected_axis_);
					}
					break;

				case CM_EntityScale:
					{
						float const DIST_THRESHOLD = 0.1f;

						{
							float min_dist = 1e10f;
							for (int i = 0; i < 3; ++ i)
							{
								int index0 = (i + 1) % 3;
								int index1 = (i + 2) % 3;
								float dist = MathLib::sqr(intersect_pt[index0][index1])
									+ MathLib::sqr(intersect_pt[index1][index0]);
								if ((intersect_pt[index0][i] > 0.4f) && (intersect_pt[index0][i] < 1.2f)
									&& (dist < DIST_THRESHOLD) && (dist < min_dist))
								{
									min_dist = dist;
									selected_axis_ = static_cast<SelectedAxis>(1UL << i);
								}
							}
						}
						if (SA_None == selected_axis_)
						{
							float min_dist = 1e10f;
							for (int i = 0; i < 3; ++ i)
							{
								int index0 = (i + 1) % 3;
								int index1 = (i + 2) % 3;
								float dist = MathLib::sqr(intersect_pt[i][index0])
									+ MathLib::sqr(intersect_pt[i][index1]);
								if ((intersect_pt[i][index0] < 0.4f) && (intersect_pt[i][index0] > 0)
									&& (intersect_pt[i][index1] < 0.4f) && (intersect_pt[i][index1] > 0)
									&& (dist < min_dist))
								{
									min_dist = dist;
									selected_axis_ = SA_XYZ;
								}
							}
						}

						scaling_axis_->FirstComponentOfType<RenderableComponent>()
							->BoundRenderableOfType<RenderableScalingAxis>()
							.HighlightAxis(selected_axis_);
					}
					break;

				default:
					break;
				}
			}
		}
		else if (mouse_down_in_wnd_)
		{
			mouse_tracking_mode_ = true;

			if ((selected_entity_ > 0) && (selected_axis_ != SA_None))
			{
				if (button & MB_Left)
				{
					RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
					FrameBufferPtr const & fb = re.CurFrameBuffer();
					uint32_t const width = fb->Width();
					uint32_t const height = fb->Height();

					float4x4 const & inv_mvp = MathLib::inverse(rotation_axis_->TransformToWorld()
						* this->ActiveCamera().ViewProjMatrix());
					float3 last_ray_target = MathLib::transform_coord(float3((last_mouse_pt_.x() + 0.5f) / width * 2 - 1,
						1 - (last_mouse_pt_.y() + 0.5f) / height * 2, 1), inv_mvp);
					float3 this_ray_target = MathLib::transform_coord(float3((pt.x() + 0.5f) / width * 2 - 1,
						1 - (pt.y() + 0.5f) / height * 2, 1), inv_mvp);
					float3 ray_origin = MathLib::transform_coord(float3(0, 0, 0),
						MathLib::inverse(rotation_axis_->TransformToWorld() * this->ActiveCamera().ViewMatrix()));
					float3 last_ray_dir = last_ray_target - ray_origin;
					float3 this_ray_dir = this_ray_target - ray_origin;

					float3 last_intersect_pt[3] =
					{
						ray_origin + MathLib::intersect_ray(Plane(1, 0, 0, 0), ray_origin, last_ray_dir) * last_ray_dir,
						ray_origin + MathLib::intersect_ray(Plane(0, 1, 0, 0), ray_origin, last_ray_dir) * last_ray_dir,
						ray_origin + MathLib::intersect_ray(Plane(0, 0, 1, 0), ray_origin, last_ray_dir) * last_ray_dir
					};
					float3 this_intersect_pt[3] =
					{
						ray_origin + MathLib::intersect_ray(Plane(1, 0, 0, 0), ray_origin, this_ray_dir) * this_ray_dir,
						ray_origin + MathLib::intersect_ray(Plane(0, 1, 0, 0), ray_origin, this_ray_dir) * this_ray_dir,
						ray_origin + MathLib::intersect_ray(Plane(0, 0, 1, 0), ray_origin, this_ray_dir) * this_ray_dir
					};

					int intersect_index[3] = { 0, 0, 0 };
					switch (selected_axis_)
					{
					case SA_X:
						intersect_index[0] = 1;
						break;

					case SA_Y:
						intersect_index[1] = 2;
						break;

					case SA_Z:
						intersect_index[2] = 0;
						break;

					case SA_YZ:
						intersect_index[1] = 0;
						intersect_index[2] = 0;
						break;

					case SA_XZ:
						intersect_index[0] = 1;
						intersect_index[2] = 1;
						break;

					case SA_XY:
						intersect_index[0] = 2;
						intersect_index[1] = 2;
						break;

					case SA_XYZ:
						break;

					default:
						KFL_UNREACHABLE("Invalid axis type");
						break;
					}

					EntityInfo& ei = entities_[selected_entity_];
					bool dirty = false;
					switch (ctrl_mode_)
					{
					case CM_EntityTranslation:
						{
							float3 translation(0, 0, 0);
							for (int i = 0; i < 3; ++ i)
							{
								if (selected_axis_ & (1UL << i))
								{
									translation[i] = this_intersect_pt[intersect_index[i]][i]
										- last_intersect_pt[intersect_index[i]][i];
								}
							}
							ei.trf_pos = MathLib::transform_quat(translation * ei.trf_scale, ei.trf_rotate) + ei.trf_pos;
							dirty = true;
						}
						break;

					case CM_EntityRotation:
						{
							for (int i = 0; i < 3; ++ i)
							{
								if (selected_axis_ & (1UL << i))
								{
									Quaternion quat = MathLib::axis_to_axis(last_intersect_pt[i], this_intersect_pt[i]);
									ei.trf_rotate = quat * ei.trf_rotate;
									dirty = true;

									break;
								}
							}
						}
						break;

					case CM_EntityScale:
						if ((ei.type != ET_Light) && (ei.type != ET_Camera))
						{
							float3 scaling(1, 1, 1);
							if (selected_axis_ != SA_XYZ)
							{
								for (int i = 0; i < 3; ++ i)
								{
									if (selected_axis_ & (1UL << i))
									{
										scaling[i] *= 1 + (this_intersect_pt[intersect_index[i]][i]
											- last_intersect_pt[intersect_index[i]][i]);

										if (ei.trf_scale[i] * scaling[i] < 0.01f)
										{
											scaling[i] = 0.01f / ei.trf_scale[i];
										}
									}
								}
							}
							else
							{
								float whole_scaling = 1 + (last_mouse_pt_.x() - pt.x()) * 0.5f / height;
								float min_s = MathLib::min3(scaling.x(), scaling.y(), scaling.z());
								if (min_s * whole_scaling < 0.01f)
								{
									whole_scaling = 0.01f / min_s;
								}
								scaling.x() = scaling.y() = scaling.z() = whole_scaling;
							}
							ei.trf_scale *= scaling;
							display_scaling_for_axis_ *= scaling;
							dirty = true;
						}
						break;

					default:
						break;
					}

					if (dirty)
					{
						this->UpdateSelectedEntity(true);
					}
				}
			}
			else if (button & (MB_Left | MB_Middle | MB_Right))
			{
				float2 move_vec = pt - last_mouse_pt_;
				if (button & MB_Left)
				{
					tb_controller_.Rotate(move_vec.x(), move_vec.y());
				}
				else if (button & MB_Middle)
				{
					tb_controller_.Move(move_vec.x(), move_vec.y());
				}
				else if (button & MB_Right)
				{
					tb_controller_.Zoom(move_vec.x(), move_vec.y());
				}

				update_selective_buffer_ = true;
			}

			last_mouse_pt_ = pt;
		}
	}

	void KGEditorCore::MouseUp(int x, int y, uint32_t button)
	{
		if (mouse_down_in_wnd_)
		{
			mouse_down_in_wnd_ = false;

			if (mouse_tracking_mode_)
			{
				mouse_tracking_mode_ = false;
				if (selected_entity_ > 0)
				{
					if (CM_EntityScale == ctrl_mode_)
					{
						display_scaling_for_axis_ = float3(1, 1, 1);
						this->UpdateEntityAxis();
					}
				}
			}
			else
			{
				if (button & MB_Left)
				{
					if (SA_None == selected_axis_)
					{
						uint32_t entity_id;
						Texture::Mapper mapper(*selective_cpu_tex_, 0, 0, TMA_Read_Only,
							x, y, 1, 1);
						uint8_t* p = mapper.Pointer<uint8_t>();
						if (0 == p[3])
						{
							if (EF_ABGR8 == selective_cpu_tex_->Format())
							{
								entity_id = p[0] | (p[1] << 8) | (p[2] << 16);
							}
							else
							{
								BOOST_ASSERT(EF_ARGB8 == selective_cpu_tex_->Format());

								entity_id = p[2] | (p[1] << 8) | (p[0] << 16);
							}
						}
						else
						{
							entity_id = 0;
						}

						update_select_entity_event_(entity_id);
					}
				}
			}
		}
	}

	void KGEditorCore::MouseDown(int x, int y, uint32_t button)
	{
		KFL_UNUSED(button);

		mouse_down_in_wnd_ = true;
		last_mouse_pt_ = int2(x, y);
	}

	void KGEditorCore::LoadScene(std::string const & file_name)
	{
		this->CloseScene();

		ResIdentifierPtr file = ResLoader::Instance().Open(file_name);
		if (file)
		{
			XMLDocument kges_doc;
			XMLNodePtr kges_root = kges_doc.Parse(file);
			this->SceneName(std::string(kges_root->Attrib("name")->ValueString()));
			{
				XMLAttributePtr attr = kges_root->Attrib("skybox");
				if (attr)
				{
					this->SkyboxName(std::string(attr->ValueString()));
				}
			}

			for (XMLNodePtr node = kges_root->FirstNode(); node; node = node->NextSibling())
			{
				if ("model" == node->Name())
				{
					add_model_event_(std::string(node->Attrib("meshml")->ValueString()).c_str());

					EntityInfo& oi = entities_[last_entity_id_];
					oi.name = std::string(node->Attrib("name")->ValueString());

					this->LoadTransformNodes(node, oi);

					float4x4 model_mat = MathLib::transformation<float>(&oi.trf_pivot, nullptr, &oi.trf_scale,
						&oi.trf_pivot, &oi.trf_rotate, &oi.trf_pos);
					oi.scene_node->TransformToParent(model_mat);
				}
				else if ("light" == node->Name())
				{
					std::string_view const lt_str = node->Attrib("type")->ValueString();
					if ("ambient" == lt_str)
					{
						add_light_event_(LightSource::LT_Ambient);
					}
					else if ("directional" == lt_str)
					{
						add_light_event_(LightSource::LT_Directional);
					}
					else if ("point" == lt_str)
					{
						add_light_event_(LightSource::LT_Point);
					}
					else
					{
						BOOST_ASSERT("spot" == lt_str);
						add_light_event_(LightSource::LT_Spot);
					}

					EntityInfo& oi = entities_[last_entity_id_];
					LightSourcePtr light = oi.light;

					oi.name = std::string(node->Attrib("name")->ValueString());

					int32_t light_attr = 0;
					XMLNodePtr attr_node = node->FirstNode("attr");
					if (attr_node)
					{
						std::string_view const attr_str = attr_node->Attrib("value")->ValueString();
						std::vector<std::string> tokens;
						boost::algorithm::split(tokens, attr_str, boost::is_any_of(" \t|"));
						for (auto& token : tokens)
						{
							boost::algorithm::trim(token);

							if ("no_shadow" == token)
							{
								light_attr |= LightSource::LSA_NoShadow;
							}
							else if ("no_diffuse" == token)
							{
								light_attr |= LightSource::LSA_NoDiffuse;
							}
							else if ("no_specular" == token)
							{
								light_attr |= LightSource::LSA_NoSpecular;
							}
							else if ("indirect" == token)
							{
								light_attr |= LightSource::LSA_IndirectLighting;
							}
						}
					}
					light->Attrib(light_attr);

					float3 dir(0, 0, 1);
					float3 pos(0, 0, 0);
					XMLNodePtr color_node = node->FirstNode("color");
					if (color_node)
					{
						float3 color;
						auto v = color_node->Attrib("v")->ValueString();
						MemInputStreamBuf stream_buff(v.data(), v.size());
						std::istream(&stream_buff) >> color.x() >> color.y() >> color.z();
						light->Color(color);
					}
					if (light->Type() != LightSource::LT_Ambient)
					{
						XMLNodePtr dir_node = node->FirstNode("dir");
						if (dir_node)
						{
							auto v = dir_node->Attrib("v")->ValueString();
							MemInputStreamBuf stream_buff(v.data(), v.size());
							std::istream(&stream_buff) >> dir.x() >> dir.y() >> dir.z();
						}
					}
					if ((LightSource::LT_Point == light->Type()) || (LightSource::LT_Spot == light->Type())
						|| (LightSource::LT_SphereArea == light->Type()) || (LightSource::LT_TubeArea == light->Type()))
					{
						XMLNodePtr pos_node = node->FirstNode("pos");
						if (pos_node)
						{
							auto v = pos_node->Attrib("v")->ValueString();
							MemInputStreamBuf stream_buff(v.data(), v.size());
							std::istream(&stream_buff) >> pos.x() >> pos.y() >> pos.z();
						}

						XMLNodePtr fall_off_node = node->FirstNode("fall_off");
						if (fall_off_node)
						{
							float3 fall_off;
							auto v = fall_off_node->Attrib("v")->ValueString();
							MemInputStreamBuf stream_buff(v.data(), v.size());
							std::istream(&stream_buff) >> fall_off.x() >> fall_off.y() >> fall_off.z();
							light->Falloff(fall_off);
						}

						if ((LightSource::LT_Point == light->Type()) || (LightSource::LT_Spot == light->Type()))
						{
							XMLNodePtr projective_node = node->FirstNode("projective");
							if (projective_node)
							{
								XMLAttributePtr attr = projective_node->Attrib("name");
								if (attr)
								{
									TexturePtr projective = ASyncLoadTexture(std::string(attr->ValueString()),
										EAH_GPU_Read | EAH_Immutable);
									light->ProjectiveTexture(projective);
								}
							}

							if (LightSource::LT_Spot == light->Type())
							{
								XMLNodePtr angle_node = node->FirstNode("angle");
								if (angle_node)
								{
									light->InnerAngle(angle_node->Attrib("inner")->ValueFloat());
									light->OuterAngle(angle_node->Attrib("outer")->ValueFloat());
								}
							}
						}

						// TODO: sphere area light and tube area light
					}

					light->BoundSceneNode()->TransformToParent(MathLib::inverse(MathLib::look_at_lh(pos, pos + dir)));

					this->LoadTransformNodes(node, oi);
				}
				else
				{
					BOOST_ASSERT("camera" == node->Name());

					add_camera_event_();

					EntityInfo& oi = entities_[last_entity_id_];
					CameraPtr camera = oi.camera;

					oi.name = std::string(node->Attrib("name")->ValueString());

					float3 eye_pos(0, 0, -1);
					float3 look_at(0, 0, 0);
					float3 up(0, 1, 0);

					XMLNodePtr eye_pos_node = node->FirstNode("eye_pos");
					if (eye_pos_node)
					{
						auto v = eye_pos_node->Attrib("v")->ValueString();
						MemInputStreamBuf stream_buff(v.data(), v.size());
						std::istream(&stream_buff) >> eye_pos.x() >> eye_pos.y() >> eye_pos.z();
					}
					XMLNodePtr look_at_node = node->FirstNode("look_at");
					if (look_at_node)
					{
						auto v = look_at_node->Attrib("v")->ValueString();
						MemInputStreamBuf stream_buff(v.data(), v.size());
						std::istream(&stream_buff) >> look_at.x() >> look_at.y() >> look_at.z();
					}
					XMLNodePtr up_node = node->FirstNode("up");
					if (up_node)
					{
						auto v = up_node->Attrib("v")->ValueString();
						MemInputStreamBuf stream_buff(v.data(), v.size());
						std::istream(&stream_buff) >> up.x() >> up.y() >> up.z();
					}
					camera->LookAtDist(MathLib::length(look_at - eye_pos));
					camera->BoundSceneNode()->TransformToWorld(MathLib::inverse(MathLib::look_at_lh(eye_pos, look_at, up)));

					float fov = PI / 4;
					float aspect = 1;
					float near_plane = 1;
					float far_plane = 1000;

					XMLNodePtr fov_node = node->FirstNode("fov");
					if (fov_node)
					{
						fov = fov_node->Attrib("s")->ValueFloat();
					}
					XMLNodePtr aspect_node = node->FirstNode("aspect");
					if (aspect_node)
					{
						aspect = aspect_node->Attrib("s")->ValueFloat();
					}
					XMLNodePtr near_plane_node = node->FirstNode("near");
					if (near_plane_node)
					{
						near_plane = near_plane_node->Attrib("s")->ValueFloat();
					}
					XMLNodePtr far_plane_node = node->FirstNode("far");
					if (far_plane_node)
					{
						far_plane = far_plane_node->Attrib("s")->ValueFloat();
					}
					camera->ProjParams(fov, aspect, near_plane, far_plane);

					this->LoadTransformNodes(node, oi);
				}
			}

			selected_entity_ = 0;
			update_selective_buffer_ = true;
			this->UpdateSceneAABB();

			update_property_event_();
		}
	}

	void KGEditorCore::SaveScene(std::string const & file_name)
	{
		std::ofstream ofs(file_name);

		ofs << "<?xml version='1.0'?>" << endl << endl;
		ofs << "<scene version=\"1\" name=\"" << scene_name_ << "\" skybox=\"" << skybox_name_ << "\">" << endl;

		for (auto iter = entities_.begin(); iter != entities_.end(); ++ iter)
		{
			switch (iter->second.type)
			{
			case ET_Model:
			{
				ofs << "\t<model name=\"" << iter->second.name << "\" meshml=\"" << iter->second.meshml_name << "\">" << endl;
				this->SaveTransformNodes(ofs, iter->second);
				ofs << "\t</model>" << endl;
			}
			break;

			case ET_Light:
			{
				LightSourcePtr const & light = iter->second.light;

				ofs << "\t<light type=\"";
				switch (light->Type())
				{
				case LightSource::LT_Ambient:
					ofs << "ambient";
					break;

				case LightSource::LT_Directional:
					ofs << "directional";
					break;

				case LightSource::LT_Point:
					ofs << "point";
					break;

				case LightSource::LT_Spot:
					ofs << "spot";
					break;

				default:
					KFL_UNREACHABLE("Invalid light type");
					break;
				}
				ofs << "\" name=\"" << iter->second.name << "\">" << endl;

				uint32_t attr = light->Attrib();
				if (attr != 0)
				{
					std::vector<std::string> attrs;
					if ((LightSource::LT_Point == light->Type()) || (LightSource::LT_Spot == light->Type()))
					{
						if (attr & LightSource::LSA_NoShadow)
						{
							attrs.push_back("no_shadow");
						}
					}
					if (attr & LightSource::LSA_NoDiffuse)
					{
						attrs.push_back("no_diffuse");
					}
					if (attr & LightSource::LSA_NoSpecular)
					{
						attrs.push_back("no_specular");
					}
					if (LightSource::LT_Spot == light->Type())
					{
						if (attr & LightSource::LSA_IndirectLighting)
						{
							attrs.push_back("indirect");
						}
					}
					if (!attrs.empty())
					{
						ofs << "\t\t<attr value=\"";
						for (size_t i = 0; i < attrs.size(); ++ i)
						{
							ofs << attrs[i];
							if (i != attrs.size() - 1)
							{
								ofs << ' ';
							}
						}
						ofs << "\"/>" << endl;
					}
				}

				ofs << "\t\t<pos v=\"" << light->Position().x() << ' '
					<< light->Position().y() << ' ' << light->Position().z() << "\"/>" << endl;
				ofs << "\t\t<dir v=\"" << light->Direction().x() << ' '
					<< light->Direction().y() << ' ' << light->Direction().z() << "\"/>" << endl;
				ofs << "\t\t<color v=\"" << light->Color().x() << ' '
					<< light->Color().y() << ' ' << light->Color().z() << "\"/>" << endl;
				ofs << "\t\t<fall_off v=\"" << light->Falloff().x() << ' '
					<< light->Falloff().y() << ' ' << light->Falloff().z() << "\"/>" << endl;
				if (LightSource::LT_Spot == light->Type())
				{
					ofs << "\t\t<angle outer=\"" << acos(light->CosOuterAngle())
						<< "\" inner=\"" << acos(light->CosInnerAngle()) << "\"/>" << endl;
				}
				this->SaveTransformNodes(ofs, iter->second);
				ofs << "\t</light>" << endl;
			}
			break;

			case ET_Camera:
			{
				Camera const & camera = *iter->second.camera;

				ofs << "\t<camera name=\"" << iter->second.name << "\">" << endl;

				ofs << "\t\t<eye_pos v=\"" << camera.EyePos().x() << ' '
					<< camera.EyePos().y() << ' ' << camera.EyePos().z() << "\"/>" << endl;
				ofs << "\t\t<look_at v=\"" << camera.LookAt().x() << ' '
					<< camera.LookAt().y() << ' ' << camera.LookAt().z() << "\"/>" << endl;
				ofs << "\t\t<up v=\"" << camera.UpVec().x() << ' '
					<< camera.UpVec().y() << ' ' << camera.UpVec().z() << "\"/>" << endl;
				ofs << "\t\t<fov s=\"" << camera.FOV() << "\"/>" << endl;
				ofs << "\t\t<aspect s=\"" << camera.Aspect() << "\"/>" << endl;
				ofs << "\t\t<near s=\"" << camera.NearPlane() << "\"/>" << endl;
				ofs << "\t\t<far s=\"" << camera.FarPlane() << "\"/>" << endl;
				this->SaveTransformNodes(ofs, iter->second);
				ofs << "\t</camera>" << endl;
			}
			break;
			}
		}

		ofs << "</scene>" << endl;
	}

	void KGEditorCore::CloseScene()
	{
		this->ClearLights();
		this->ClearCameras();
		this->ClearModels();
	}

	void KGEditorCore::LoadTransformNodes(XMLNodePtr const & node, EntityInfo& oi)
	{
		XMLNodePtr pivot_node = node->FirstNode("pivot");
		if (!!pivot_node)
		{
			auto v = pivot_node->Attrib("v")->ValueString();
			MemInputStreamBuf stream_buff(v.data(), v.size());
			std::istream(&stream_buff) >> oi.trf_pivot.x() >> oi.trf_pivot.y() >> oi.trf_pivot.z();
		}
		else
		{
			oi.trf_pivot = oi.obb.Center();
		}

		XMLNodePtr scale_node = node->FirstNode("scale");
		if (!!scale_node)
		{
			auto v = scale_node->Attrib("v")->ValueString();
			MemInputStreamBuf stream_buff(v.data(), v.size());
			std::istream(&stream_buff) >> oi.trf_scale.x() >> oi.trf_scale.y() >> oi.trf_scale.z();
		}
		else
		{
			oi.trf_scale = float3(1, 1, 1);
		}

		XMLNodePtr rotate_node = node->FirstNode("rotate");
		if (!!rotate_node)
		{
			auto v = rotate_node->Attrib("v")->ValueString();
			MemInputStreamBuf stream_buff(v.data(), v.size());
			std::istream(&stream_buff) >> oi.trf_rotate.x() >> oi.trf_rotate.y() >> oi.trf_rotate.z() >> oi.trf_rotate.w();
		}
		else
		{
			oi.trf_rotate = Quaternion::Identity();
		}

		XMLNodePtr translate_node = node->FirstNode("translate");
		if (!!translate_node)
		{
			auto v = translate_node->Attrib("v")->ValueString();
			MemInputStreamBuf stream_buff(v.data(), v.size());
			std::istream(&stream_buff) >> oi.trf_pos.x() >> oi.trf_pos.y() >> oi.trf_pos.z();
		}
		else
		{
			oi.trf_pos = float3(0, 0, 0);
		}
	}

	void KGEditorCore::SaveTransformNodes(std::ostream& os, EntityInfo const & oi)
	{
		os << "\t\t<pivot v=\"" << oi.trf_pivot.x() << ' ' << oi.trf_pivot.y()
			<< ' ' << oi.trf_pivot.z() << "\"/>" << endl;
		os << "\t\t<scale v=\"" << oi.trf_scale.x() << ' ' << oi.trf_scale.y()
			<< ' ' << oi.trf_scale.z() << "\"/>" << endl;
		os << "\t\t<rotate v=\"" << oi.trf_rotate.x() << ' ' << oi.trf_rotate.y()
			<< ' ' << oi.trf_rotate.z() << ' ' << oi.trf_rotate.w() << "\"/>" << endl;
		os << "\t\t<translate v=\"" << oi.trf_pos.x() << ' ' << oi.trf_pos.y()
			<< ' ' << oi.trf_pos.z() << "\"/>" << endl;
	}

	KGEditorCore::EntityInfo& KGEditorCore::GetEntityInfo(uint32_t id)
	{
		auto iter = entities_.find(id);
		BOOST_ASSERT(iter != entities_.end());

		return iter->second;
	}

	KGEditorCore::EntityInfo const & KGEditorCore::GetEntityInfo(uint32_t id) const
	{
		auto iter = entities_.find(id);
		BOOST_ASSERT(iter != entities_.end());

		return iter->second;
	}
}
