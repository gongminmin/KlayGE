#include <KlayGE/KlayGE.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/PostProcess.hpp>
#include <KlayGE/Camera.hpp>
#include <KFL/Half.hpp>
#include <KlayGE/App3D.hpp>
#include <KlayGE/DeferredRenderingLayer.hpp>

#include "ProceduralTerrain.hpp"

namespace KlayGE
{
	uint32_t const COARSE_HEIGHT_MAP_SIZE = 1024;

	ProceduralTerrain::ProceduralTerrain()
		: HQTerrainRenderable(SyncLoadRenderEffect("ProceduralTerrain.fxml"))
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		RenderEngine& re = rf.RenderEngineInstance();
		RenderDeviceCaps const & caps = re.DeviceCaps();

		auto const height_fmt = caps.BestMatchTextureRenderTargetFormat(caps.pack_to_rgba_required ? MakeSpan({EF_ABGR8, EF_ARGB8})
			: MakeSpan({EF_R16F, EF_R32F}), 1, 0);
		BOOST_ASSERT(height_fmt != EF_Unknown);
		height_map_tex_ = rf.MakeTexture2D(COARSE_HEIGHT_MAP_SIZE, COARSE_HEIGHT_MAP_SIZE, 1, 1, height_fmt,
			1, 0, EAH_GPU_Read | EAH_GPU_Write);
		height_map_srv_ = rf.MakeTextureSrv(height_map_tex_);
		height_map_rtv_ = rf.Make2DRtv(height_map_tex_, 0, 1, 0);
		height_map_cpu_tex_ = rf.MakeTexture2D(height_map_tex_->Width(0), height_map_tex_->Height(0),
			1, 1, height_map_tex_->Format(), 1, 0, EAH_CPU_Read);

		ElementFormat gradient_fmt;
		if (EF_R16F == height_fmt)
		{
			gradient_fmt = EF_GR16F;
		}
		else if (EF_R32F == height_fmt)
		{
			gradient_fmt = EF_GR32F;
		}
		else
		{
			gradient_fmt = height_fmt;
		}
		gradient_map_tex_ = rf.MakeTexture2D(COARSE_HEIGHT_MAP_SIZE, COARSE_HEIGHT_MAP_SIZE, 1, 1, gradient_fmt,
			1, 0, EAH_GPU_Read | EAH_GPU_Write);
		gradient_map_srv_ = rf.MakeTextureSrv(gradient_map_tex_);
		gradient_map_rtv_ = rf.Make2DRtv(gradient_map_tex_, 0, 1, 0);
		gradient_map_cpu_tex_ = rf.MakeTexture2D(gradient_map_tex_->Width(0), gradient_map_tex_->Height(0),
			1, 1, gradient_map_tex_->Format(), 1, 0, EAH_CPU_Read);

		auto const mask_fmt = re.DeviceCaps().BestMatchTextureRenderTargetFormat(MakeSpan({EF_ABGR8, EF_ARGB8}), 1, 0);
		BOOST_ASSERT(mask_fmt != EF_Unknown);
		mask_map_tex_ = rf.MakeTexture2D(COARSE_HEIGHT_MAP_SIZE, COARSE_HEIGHT_MAP_SIZE, 1, 1, mask_fmt,
			1, 0, EAH_GPU_Read | EAH_GPU_Write);
		mask_map_srv_ = rf.MakeTextureSrv(mask_map_tex_);
		mask_map_rtv_ = rf.Make2DRtv(mask_map_tex_, 0, 1, 0);
		mask_map_cpu_tex_ = rf.MakeTexture2D(mask_map_tex_->Width(0), mask_map_tex_->Height(0),
			1, 1, mask_map_tex_->Format(), 1, 0, EAH_CPU_Read);

		height_pp_ = SyncLoadPostProcess("ProceduralTerrain.ppml", "height");
		gradient_pp_ = SyncLoadPostProcess("ProceduralTerrain.ppml", "gradient");
		mask_pp_ = SyncLoadPostProcess("ProceduralTerrain.ppml", "mask");

		height_pp_->OutputPin(0, height_map_rtv_);

		gradient_pp_->InputPin(0, height_map_srv_);
		gradient_pp_->OutputPin(0, gradient_map_rtv_);

		mask_pp_->InputPin(0, height_map_srv_);
		mask_pp_->InputPin(1, gradient_map_srv_);
		mask_pp_->OutputPin(0, mask_map_rtv_);

		mvp_wo_oblique_param_ = effect_->ParameterByName("mvp_wo_oblique");
	}

	void ProceduralTerrain::FlushTerrainData()
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		RenderEngine& re = rf.RenderEngineInstance();

		int3 octaves(ridge_octaves_, fBm_octaves_, tex_twist_octaves_);
		height_pp_->SetParam(0, octaves);
		height_pp_->SetParam(1, texture_world_offset_);
		height_pp_->SetParam(2, float2(static_cast<float>(world_uv_repeats_), 1.0f / world_uv_repeats_));

		mask_pp_->SetParam(0, texture_world_offset_);
		mask_pp_->SetParam(1, world_scale_ * tile_rings_.back()->OuterWidth() / COARSE_HEIGHT_MAP_SIZE);
		mask_pp_->SetParam(2, float2(static_cast<float>(world_uv_repeats_), 1.0f / world_uv_repeats_));
		mask_pp_->SetParam(3, float2(vertical_scale_, 1.0f / vertical_scale_));

		FrameBufferPtr fb = re.CurFrameBuffer();

		height_pp_->Apply();
		gradient_pp_->Apply();
		//mask_pp_->Apply();

		re.BindFrameBuffer(fb);

		height_map_tex_->CopyToTexture(*height_map_cpu_tex_, TextureFilter::Point);

		//SaveTexture(height_map_cpu_tex_, "height_map.dds");
		//SaveTexture(gradient_map_tex_, "gradient_map.dds");
		//SaveTexture(mask_map_tex_, "mask_map.dds");
	}

	void ProceduralTerrain::ReflectionPlane(Plane const & plane)
	{
		reflection_plane_ = plane;
	}

	void ProceduralTerrain::OnRenderBegin()
	{
		HQTerrainRenderable::OnRenderBegin();

		auto const& pccb = Context::Instance().RenderFactoryInstance().RenderEngineInstance().PredefinedCameraCBufferInstance();

		*mvp_wo_oblique_param_ = MathLib::transpose(pccb.Camera(*camera_cbuffer_, 0).mvp);

		auto drl = Context::Instance().DeferredRenderingLayerInstance();
		if (drl && (drl->ActiveViewport() == 0))
		{
			App3DFramework const & app = Context::Instance().AppInstance();
			Camera const & camera = app.ActiveCamera();
			float4x4 const & view = camera.ViewMatrix();
			float4x4 proj = camera.ProjMatrixWOAdjust();

			MathLib::oblique_clipping(proj,
				MathLib::mul(reflection_plane_, MathLib::transpose(camera.InverseViewMatrix())));
			Context::Instance().RenderFactoryInstance().RenderEngineInstance().AdjustProjectionMatrix(proj);

			float4x4 mvp = model_mat_ * view * proj;

			int32_t cas_index = drl->CurrCascadeIndex();
			if (cas_index >= 0)
			{
				mvp *= drl->GetCascadedShadowLayer().CascadeCropMatrix(cas_index);
			}

			pccb.Camera(*camera_cbuffer_, 0).mvp = MathLib::transpose(mvp);
			camera_cbuffer_->Dirty(true);
		}
	}
}
