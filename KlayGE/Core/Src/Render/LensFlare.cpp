// LensFlare.cpp
// KlayGE Lens Flare implement file
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
#include <KlayGE/App3D.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/FrameBuffer.hpp>
#include <KlayGE/Camera.hpp>

#include <KlayGE/LensFlare.hpp>

namespace KlayGE
{
	int const SUN_FLARENUM = 6;

	LensFlareRenderable::LensFlareRenderable()
		: Renderable(L"LensFlare")
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();

		rls_[0] = rf.MakeRenderLayout();
		rls_[0]->TopologyType(RenderLayout::TT_TriangleList);

		std::vector<float3> vertices;
		for (int i = 0; i < SUN_FLARENUM; ++ i)
		{
			vertices.push_back(float3(-1, +1, i + 0.1f));
			vertices.push_back(float3(+1, +1, i + 0.1f));
			vertices.push_back(float3(-1, -1, i + 0.1f));
			vertices.push_back(float3(+1, -1, i + 0.1f));
		}

		GraphicsBufferPtr pos_vb = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable,
			static_cast<uint32_t>(vertices.size() * sizeof(vertices[0])), &vertices[0]);
		rls_[0]->BindVertexStream(pos_vb, VertexElement(VEU_Position, 0, EF_BGR32F));

		std::vector<uint32_t> indices;
		for (int i = 0; i < SUN_FLARENUM; ++ i)
		{
			indices.push_back(i * 4 + 2);
			indices.push_back(i * 4 + 0);
			indices.push_back(i * 4 + 1);

			indices.push_back(i * 4 + 1);
			indices.push_back(i * 4 + 3);
			indices.push_back(i * 4 + 2);
		}

		GraphicsBufferPtr ib = rf.MakeIndexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable,
			static_cast<uint32_t>(indices.size() * sizeof(indices[0])), &indices[0]);
		rls_[0]->BindIndexStream(ib, EF_R32UI);

		effect_ = SyncLoadRenderEffect("LensFlare.fxml");
		simple_forward_tech_ = effect_->TechniqueByName("LensFlare");
		technique_ = simple_forward_tech_;

		effect_attrs_ |= EA_SimpleForward;
	}

	void LensFlareRenderable::FlareParam(std::vector<float3> const & param, float alpha_fac)
	{
		*(effect_->ParameterByName("flare_param")) = param;
		*(effect_->ParameterByName("alpha_fac")) = alpha_fac;
	}

	void LensFlareRenderable::OnRenderBegin()
	{
		App3DFramework const & app = Context::Instance().AppInstance();
		Camera const & camera = app.ActiveCamera();
			
		*(effect_->ParameterByName("eye_pos")) = camera.EyePos();

		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		*(effect_->ParameterByName("scale")) = static_cast<float>(re.CurFrameBuffer()->Width()) / re.CurFrameBuffer()->Height();
	}


	LensFlareRenderableComponent::LensFlareRenderableComponent()
		: RenderableComponent(MakeSharedPtr<LensFlareRenderable>())
	{
		this->OnMainThreadUpdate().Connect([this](SceneComponent& component, float app_time, float elapsed_time)
		{
			KFL_UNUSED(component);
			KFL_UNUSED(app_time);
			KFL_UNUSED(elapsed_time);

			float const FLARE_RENDERANGLE = 0.9f;
			float const FLARE_SCALEAMOUNT = 0.2f;

			App3DFramework const& app = Context::Instance().AppInstance();
			Camera const& camera = app.ActiveCamera();

			float4x4 const& view = camera.ViewMatrix();
			float4x4 const& proj = camera.ProjMatrix();

			float4x4 const& mat = this->BoundSceneNode()->TransformToWorld();
			float3 const dir = -*reinterpret_cast<float3 const*>(&mat.Row(2));

			float3 sun_vec = MathLib::normalize(dir);
			float3 const& view_vec = camera.ForwardVec();

			float angle = MathLib::dot(view_vec, sun_vec);

			// update flare
			if (angle > FLARE_RENDERANGLE)
			{
				this->Enabled(true);

				// get angle amount by current angle
				float angle_amount = 1 - (1 - angle) / (1 - FLARE_RENDERANGLE); // convert angle to percent
				float inv_angle_amount = std::max(0.85f, (1 - angle) / (1 - FLARE_RENDERANGLE));

				float alpha_fac;
				if (angle_amount < 0.5f)
				{
					alpha_fac = angle_amount;
				}
				else
				{
					alpha_fac = 1 - angle_amount;
				}

				// calculate flare pos
				float2 center_pos(0, 0);
				float3 sun_vec_es = MathLib::transform_normal(dir, view);
				float3 sun_pos_es = camera.FarPlane() / sun_vec_es.z() * sun_vec_es;
				float2 axis_vec = MathLib::transform_coord(sun_pos_es, proj);

				// update flare pos and scale matrix by pos and angle amount
				std::vector<float3> flare_param(SUN_FLARENUM);
				for (int flare = 0; flare < SUN_FLARENUM; ++flare)
				{
					float2 flare_pos = center_pos + (flare - SUN_FLARENUM * 0.2f) / ((SUN_FLARENUM - 1.0f) * 1.5f) * axis_vec;
					float scale_fac = FLARE_SCALEAMOUNT * inv_angle_amount * ((SUN_FLARENUM - flare) / (SUN_FLARENUM - 1.0f));

					flare_param[flare] = float3(flare_pos.x(), flare_pos.y(), scale_fac);
				}

				this->BoundRenderableOfType<LensFlareRenderable>().FlareParam(flare_param, alpha_fac);
			}
			else
			{
				this->Enabled(false);
			}
		});
	}
}
