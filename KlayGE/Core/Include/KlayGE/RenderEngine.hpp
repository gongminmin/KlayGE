// RenderEngine.hpp
// KlayGE 渲染引擎类 实现文件
// Ver 3.11.0
// 版权所有(C) 龚敏敏, 2003-2010
// Homepage: http://www.klayge.org
//
// 3.11.0
// Add ForceFlush (2010.7.20)
// Remove TexelToPixelOffset (2010.9.26)
// Add AdjustPerspectiveMatrix (2010.11.2)
//
// 3.10.0
// 增加了Dispatch (2009.12.22)
// 增加了NumMotionFrames (2010.2.22)
// 支持Stereo (2010.3.20)
//
// 3.9.0
// 增加了BeginPass/EndPass (2009.4.9)
// 支持Stream Output (2009.5.14)
//
// 3.6.0
// 去掉了RenderTarget，直接使用FrameBuffer (2007.6.20)
//
// 3.5.0
// 支持Alpha to Coverage (2006.9.24)
//
// 3.4.0
// 增加了TexelToPixelOffset (2006.8.27)
// 去掉了ClearColor (2006.8.31)
//
// 3.3.0
// 统一了RenderState (2006.5.21)
//
// 3.2.0
// 暴露出了Clear (2005.12.31)
//
// 3.0.0
// 去掉了固定流水线 (2005.8.18)
// 支持point sprite (2005.9.28)
// 支持scissor (2005.10.20)
//
// 2.8.0
// 简化了StencilBuffer相关操作 (2005.7.20)
// 简化了RenderTarget，支持MRT (2005.7.25)
// 去掉了纹理坐标生成 (2005.7.30)
//
// 2.7.1
// ViewMatrix和ProjectionMatrix改为const (2005.7.10)
//
// 2.7.0
// 去掉了TextureCoordSet (2005.6.26)
// TextureAddressingMode, TextureFiltering和TextureAnisotropy移到Texture中 (2005.6.27)
//
// 2.4.0
// 增加了NumFacesJustRendered和NumVerticesJustRendered (2005.3.21)
// 增加了PolygonMode (2005.3.20)
//
// 2.0.4
// 去掉了WorldMatrices (2004.4.3)
// 保存了三个矩阵 (2004.4.7)
//
// 2.0.3
// 去掉了SoftwareBlend (2004.3.10)
//
// 2.0.1
// 去掉了TexBlendMode (2003.10.16)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#ifndef _RENDERENGINE_HPP
#define _RENDERENGINE_HPP

#pragma once

#include <KFL/Noncopyable.hpp>
#include <KlayGE/RenderDeviceCaps.hpp>
#include <KlayGE/RenderSettings.hpp>
#include <KlayGE/Mipmapper.hpp>
#include <KFL/Color.hpp>

#include <memory>
#include <string_view>
#include <vector>

#include <KlayGE/FrameBuffer.hpp>
#include <KlayGE/Mesh.hpp>
#include <KlayGE/PerfProfiler.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/RenderMaterial.hpp>
#include <KlayGE/RenderStateObject.hpp>
#include <KlayGE/RenderView.hpp>

namespace KlayGE
{
	class PostProcess;
	using PostProcessPtr = std::shared_ptr<PostProcess>;
	class SceneNode;
	using SceneNodePtr = std::shared_ptr<SceneNode>;

	class KLAYGE_CORE_API RenderEngine
	{
		KLAYGE_NONCOPYABLE(RenderEngine);

	public:
		RenderEngine();
		virtual ~RenderEngine() noexcept;

		void Suspend();
		void Resume();

		virtual std::wstring const & Name() const = 0;

		virtual bool RequiresFlipping() const = 0;

		virtual void BeginFrame();
		virtual void BeginPass();
		void Render(RenderEffect const & effect, RenderTechnique const & tech, RenderLayout const & rl);
		void Dispatch(RenderEffect const & effect, RenderTechnique const & tech, uint32_t tgx, uint32_t tgy, uint32_t tgz);
		void DispatchIndirect(RenderEffect const & effect, RenderTechnique const & tech,
			GraphicsBufferPtr const & buff_args, uint32_t offset);
		virtual void EndPass();
		virtual void EndFrame();

		// Just for debug or profile propose
		virtual void ForceFlush() = 0;

		uint32_t NumPrimitivesJustRendered();
		uint32_t NumVerticesJustRendered();
		uint32_t NumDrawsJustCalled();
		uint32_t NumDispatchesJustCalled();

		void CreateRenderWindow(std::string const & name, RenderSettings& settings);
		void DestroyRenderWindow();

		void SetStateObject(RenderStateObjectPtr const & rs_obj);

		void BindFrameBuffer(FrameBufferPtr const & fb);
		FrameBufferPtr const & CurFrameBuffer() const;
		FrameBufferPtr const & DefaultFrameBuffer() const;
		FrameBufferPtr const & ScreenFrameBuffer() const;
		FrameBufferPtr const & OverlayFrameBuffer() const;

		virtual TexturePtr const & ScreenDepthStencilTexture() const = 0;

		void BindSOBuffers(RenderLayoutPtr const & rl);

		// Get render device capabilities
		RenderDeviceCaps const & DeviceCaps() const;

		// Scissor support
		virtual void ScissorRect(uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;

		virtual void GetCustomAttrib(std::string_view name, void* value) const;
		virtual void SetCustomAttrib(std::string_view name, void* value);

		void Resize(uint32_t width, uint32_t height);
		virtual bool FullScreen() const = 0;
		virtual void FullScreen(bool fs) = 0;

		uint32_t NativeShaderFourCC() const
		{
			return native_shader_fourcc_;
		}
		uint32_t NativeShaderVersion() const
		{
			return native_shader_version_;
		}
		std::string_view NativeShaderPlatformName() const
		{
			return native_shader_platform_name_;
		}

		void PostProcess(bool skip);

		void HDREnabled(bool hdr);
		void PPAAEnabled(int aa);
		void GammaEnabled(bool gamma);
		void ColorGradingEnabled(bool cg);

		float DefaultFOV() const
		{
			return default_fov_;
		}
		void DefaultFOV(float fov)
		{
			default_fov_ = fov;
		}
		void DefaultRenderWidthScale(float scale)
		{
			default_render_width_scale_ = scale;
		}
		void DefaultRenderHeightScale(float scale)
		{
			default_render_height_scale_ = scale;
		}

		void NumCameraInstances(uint32_t num)
		{
			num_camera_instances_ = num;
		}
		uint32_t NumCameraInstances() const
		{
			return num_camera_instances_;
		}

		// Render a frame when no pending message
		virtual void Refresh();

		virtual void AdjustProjectionMatrix(float4x4& /*proj_mat*/)
		{
		}

		void ConvertToDisplay();

		RenderStateObjectPtr const & CurRenderStateObject() const
		{
			return cur_rs_obj_;
		}

		RenderLayoutPtr const & PostProcessRenderLayout() const
		{
			return pp_rl_;
		}
		RenderLayoutPtr const & VolumetricPostProcessRenderLayout() const
		{
			return vpp_rl_;
		}

		StereoMethod Stereo() const
		{
			return stereo_method_;
		}
		void Stereo(StereoMethod method);
		void StereoSeparation(float separation)
		{
			stereo_separation_ = separation;
		}
		float StereoSeparation() const
		{
			return stereo_separation_;
		}
		// For Oculus VR
		void OVRHMDWarpParam(float4 const & hmd_warp_param)
		{
			ovr_hmd_warp_param_ = hmd_warp_param;
		}
		void OVRChromAbParam(float4 const & chrom_ab_param)
		{
			ovr_chrom_ab_param_ = chrom_ab_param;
		}
		void OVRXCenterOffset(float x_center_offset)
		{
			ovr_x_center_offset_ = x_center_offset;
		}
		void OVRScale(float scale)
		{
			ovr_scale_ = scale;
		}

		DisplayOutputMethod DisplayOutput() const
		{
			return display_output_method_;
		}
		void DisplayOutput(DisplayOutputMethod method);
		void PaperWhiteNits(uint32_t nits);
		uint32_t PaperWhiteNits() const
		{
			return paper_white_;
		}
		void DisplayMaxLuminanceNits(uint32_t nits);
		uint32_t DisplayMaxLuminanceNits() const
		{
			return display_max_luminance_;
		}
		float HDRRescale() const
		{
			return hdr_rescale_;
		}

		// For debug only
		void ForceLineMode(bool line);
		bool ForceLineMode() const
		{
			return force_line_mode_;
		}

		RenderMaterialPtr const& DefaultMaterial() const;

		PredefinedMaterialCBuffer const& PredefinedMaterialCBufferInstance() const;
		PredefinedMeshCBuffer const& PredefinedMeshCBufferInstance() const;
		PredefinedModelCBuffer const& PredefinedModelCBufferInstance() const;
		PredefinedCameraCBuffer const& PredefinedCameraCBufferInstance() const;

		Mipmapper const& MipmapperInstance() const;

	protected:
		void Destroy();
		uint32_t NumRealizedCameraInstances() const;

	private:
		virtual void CheckConfig(RenderSettings& settings);
		virtual void StereoscopicForLCDShutter(int32_t eye);
		void AssemblePostProcessChain();

		virtual void DoCreateRenderWindow(std::string const & name, RenderSettings const & settings) = 0;
		virtual void DoBindFrameBuffer(FrameBufferPtr const & fb) = 0;
		virtual void DoBindSOBuffers(RenderLayoutPtr const & rl) = 0;
		virtual void DoRender(RenderEffect const & effect, RenderTechnique const & tech, RenderLayout const & rl) = 0;
		virtual void DoDispatch(RenderEffect const & effect, RenderTechnique const & tech, uint32_t tgx, uint32_t tgy, uint32_t tgz) = 0;
		virtual void DoDispatchIndirect(RenderEffect const & effect, RenderTechnique const & tech,
			GraphicsBufferPtr const & buff_args, uint32_t offset) = 0;
		virtual void DoResize(uint32_t width, uint32_t height) = 0;
		virtual void DoDestroy() = 0;

		virtual void DoSuspend() = 0;
		virtual void DoResume() = 0;

		void UpdateHDRRescale();

	protected:
		FrameBufferPtr cur_frame_buffer_;
		FrameBufferPtr screen_frame_buffer_;
		SceneNodePtr screen_frame_buffer_camera_node_;
		TexturePtr ds_tex_;
		ShaderResourceViewPtr ds_srv_;
		FrameBufferPtr hdr_frame_buffer_;
		TexturePtr hdr_tex_;
		ShaderResourceViewPtr hdr_srv_;
		FrameBufferPtr post_tone_mapping_frame_buffer_;
		TexturePtr post_tone_mapping_tex_;
		ShaderResourceViewPtr post_tone_mapping_srv_;
		RenderTargetViewPtr post_tone_mapping_rtv_;
		FrameBufferPtr resize_frame_buffer_;
		TexturePtr resize_tex_;
		ShaderResourceViewPtr resize_srv_;
		RenderTargetViewPtr resize_rtv_;
		FrameBufferPtr mono_frame_buffer_;
		TexturePtr mono_tex_;
		ShaderResourceViewPtr mono_srv_;
		RenderTargetViewPtr mono_rtv_;
		FrameBufferPtr default_frame_buffers_[4];

		FrameBufferPtr overlay_frame_buffer_;
		TexturePtr overlay_tex_;
		ShaderResourceViewPtr overlay_srv_;

		TexturePtr smaa_edges_tex_;
		ShaderResourceViewPtr smaa_edges_srv_;
		RenderTargetViewPtr smaa_edges_rtv_;
		TexturePtr smaa_blend_tex_;
		ShaderResourceViewPtr smaa_blend_srv_;
		RenderTargetViewPtr smaa_blend_rtv_;

		RenderLayoutPtr so_buffers_;

		uint32_t num_primitives_just_rendered_;
		uint32_t num_vertices_just_rendered_;
		uint32_t num_draws_just_called_;
		uint32_t num_dispatches_just_called_;

		RenderDeviceCaps caps_;

		RenderStateObjectPtr cur_rs_obj_;
		RenderStateObjectPtr cur_line_rs_obj_;

		float default_fov_;
		float default_render_width_scale_;
		float default_render_height_scale_;

		StereoMethod stereo_method_;
		float stereo_separation_;
		// Oculus VR
		float4 ovr_hmd_warp_param_;
		float4 ovr_chrom_ab_param_;
		float ovr_x_center_offset_;
		float ovr_scale_;

		DisplayOutputMethod display_output_method_;
		uint32_t paper_white_;
		uint32_t display_max_luminance_;
		float hdr_rescale_;

		RenderLayoutPtr pp_rl_;
		RenderLayoutPtr vpp_rl_;

		PostProcessPtr hdr_pp_;
		PostProcessPtr skip_hdr_pp_;
		bool hdr_enabled_;
		PostProcessPtr smaa_edge_detection_pp_;
		PostProcessPtr smaa_blending_weight_pp_;
		PostProcessPtr post_tone_mapping_pp_;
		int ppaa_enabled_;
		bool gamma_enabled_;
		bool color_grading_enabled_;
		PostProcessPtr resize_pps_[2];
		PostProcessPtr hdr_display_pp_;
		PostProcessPtr stereoscopic_pp_;
		int fb_stage_;
		bool pp_chain_dirty_;

		PostProcessPtr post_tone_mapping_pps_[12];

		bool force_line_mode_;

		uint32_t native_shader_fourcc_;
		uint32_t native_shader_version_;
		std::string_view native_shader_platform_name_;

		uint32_t num_camera_instances_ = 0;

#ifndef KLAYGE_SHIP
		PerfRegion* hdr_pp_perf_;
		PerfRegion* smaa_pp_perf_;
		PerfRegion* post_tone_mapping_pp_perf_;
		PerfRegion* resize_pp_perf_;
		PerfRegion* hdr_display_pp_perf_;
		PerfRegion* stereoscopic_pp_perf_;
#endif

		mutable RenderMaterialPtr default_material_;
		mutable std::unique_ptr<PredefinedMaterialCBuffer> predefined_material_cb_;
		mutable std::unique_ptr<PredefinedMeshCBuffer> predefined_mesh_cb_;
		mutable std::unique_ptr<PredefinedModelCBuffer> predefined_model_cb_;
		mutable std::unique_ptr<PredefinedCameraCBuffer> predefined_camera_cb_;

		mutable std::unique_ptr<Mipmapper> mipmapper_;
	};

	struct DrawIndirectArgs
	{
		uint32_t num_vertices_per_instance;
		uint32_t num_instances;
		uint32_t start_vertex_location;
		uint32_t start_instance_location;
	};
	static_assert(sizeof(DrawIndirectArgs) == 16);

	struct DrawIndirectIndexedArgs
	{
		uint32_t num_indices_per_instance;
		uint32_t num_instances;
		uint32_t start_index_location;
		int32_t base_vertex_location;
		uint32_t start_instance_location;
	};
	static_assert(sizeof(DrawIndirectIndexedArgs) == 20);

	struct DispatchIndirectArgs
	{
		uint32_t tgx;
		uint32_t tgy;
		uint32_t tgz;
	};
	static_assert(sizeof(DispatchIndirectArgs) == 12);
}

#endif			// _RENDERENGINE_HPP
