// RenderEngine.hpp
// KlayGE ��Ⱦ������ ʵ���ļ�
// Ver 3.11.0
// ��Ȩ����(C) ������, 2003-2010
// Homepage: http://www.klayge.org
//
// 3.11.0
// Add ForceFlush (2010.7.20)
// Remove TexelToPixelOffset (2010.9.26)
// Add AdjustPerspectiveMatrix (2010.11.2)
//
// 3.10.0
// ������Dispatch (2009.12.22)
// ������NumMotionFrames (2010.2.22)
// ֧��Stereo (2010.3.20)
//
// 3.9.0
// ������BeginPass/EndPass (2009.4.9)
// ֧��Stream Output (2009.5.14)
//
// 3.6.0
// ȥ����RenderTarget��ֱ��ʹ��FrameBuffer (2007.6.20)
//
// 3.5.0
// ֧��Alpha to Coverage (2006.9.24)
//
// 3.4.0
// ������TexelToPixelOffset (2006.8.27)
// ȥ����ClearColor (2006.8.31)
//
// 3.3.0
// ͳһ��RenderState (2006.5.21)
//
// 3.2.0
// ��¶����Clear (2005.12.31)
//
// 3.0.0
// ȥ���˹̶���ˮ�� (2005.8.18)
// ֧��point sprite (2005.9.28)
// ֧��scissor (2005.10.20)
//
// 2.8.0
// ����StencilBuffer��ز��� (2005.7.20)
// ����RenderTarget��֧��MRT (2005.7.25)
// ȥ���������������� (2005.7.30)
//
// 2.7.1
// ViewMatrix��ProjectionMatrix��Ϊconst (2005.7.10)
//
// 2.7.0
// ȥ����TextureCoordSet (2005.6.26)
// TextureAddressingMode, TextureFiltering��TextureAnisotropy�Ƶ�Texture�� (2005.6.27)
//
// 2.4.0
// ������NumFacesJustRendered��NumVerticesJustRendered (2005.3.21)
// ������PolygonMode (2005.3.20)
//
// 2.0.4
// ȥ����WorldMatrices (2004.4.3)
// �������������� (2004.4.7)
//
// 2.0.3
// ȥ����SoftwareBlend (2004.3.10)
//
// 2.0.1
// ȥ����TexBlendMode (2003.10.16)
//
// �޸ļ�¼
//////////////////////////////////////////////////////////////////////////////////

#ifndef _RENDERENGINE_HPP
#define _RENDERENGINE_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/RenderDeviceCaps.hpp>
#include <KlayGE/RenderSettings.hpp>
#include <KFL/Color.hpp>

#include <vector>

namespace KlayGE
{
	class KLAYGE_CORE_API RenderEngine
	{
	public:
		RenderEngine();
		virtual ~RenderEngine();

		void Suspend();
		void Resume();

		virtual std::wstring const & Name() const = 0;

		virtual bool RequiresFlipping() const = 0;

		virtual void BeginFrame();
		virtual void BeginPass();
		void Render(RenderTechnique const & tech, RenderLayout const & rl);
		void Dispatch(RenderTechnique const & tech, uint32_t tgx, uint32_t tgy, uint32_t tgz);
		void DispatchIndirect(RenderTechnique const & tech, GraphicsBufferPtr const & buff_args, uint32_t offset);
		virtual void EndPass();
		virtual void EndFrame();
		virtual void UpdateGPUTimestampsFrequency();

		// Just for debug or profile propose
		virtual void ForceFlush() = 0;

		uint32_t NumPrimitivesJustRendered();
		uint32_t NumVerticesJustRendered();
		uint32_t NumDrawsJustCalled();
		uint32_t NumDispatchesJustCalled();

		void CreateRenderWindow(std::string const & name, RenderSettings& settings);
		void DestroyRenderWindow();

		void SetStateObjects(RasterizerStateObjectPtr const & rs_obj,
			DepthStencilStateObjectPtr const & dss_obj, uint16_t front_stencil_ref, uint16_t back_stencil_ref,
			BlendStateObjectPtr const & bs_obj, Color const & blend_factor, uint32_t sample_mask);

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

		virtual void GetCustomAttrib(std::string const & name, void* value);
		virtual void SetCustomAttrib(std::string const & name, void* value);

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

		// Render a frame when no pending message
		virtual void Refresh();

		virtual void AdjustProjectionMatrix(float4x4& /*proj_mat*/)
		{
		}

		void Stereoscopic();

		uint32_t NumMotionFrames() const
		{
			return motion_frames_;
		}

		RasterizerStateObjectPtr const & CurRSObj() const
		{
			return cur_rs_obj_;
		}
		DepthStencilStateObjectPtr const & CurDSSObj() const
		{
			return cur_dss_obj_;
		}
		uint16_t CurFrontStencilRef() const
		{
			return cur_front_stencil_ref_;
		}
		uint16_t CurBackStencilRef() const
		{
			return cur_back_stencil_ref_;
		}
		BlendStateObjectPtr const & CurBSObj() const
		{
			return cur_bs_obj_;
		}
		Color const & CurBlendFactor() const
		{
			return cur_blend_factor_;
		}
		uint32_t CurSampleMask() const
		{
			return cur_sample_mask_;
		}

		RenderLayoutPtr const & PostProcessRenderLayout() const
		{
			return pp_rl_;
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
		
		// For debug only
		void ForceLineMode(bool line);
		bool ForceLineMode() const
		{
			return force_line_mode_;
		}

	protected:
		void Destroy();

	private:
		virtual void CheckConfig(RenderSettings& settings);
		virtual void StereoscopicForLCDShutter(int32_t eye);
		void AssemblePostProcessChain();

		virtual void DoCreateRenderWindow(std::string const & name, RenderSettings const & settings) = 0;
		virtual void DoBindFrameBuffer(FrameBufferPtr const & fb) = 0;
		virtual void DoBindSOBuffers(RenderLayoutPtr const & rl) = 0;
		virtual void DoRender(RenderTechnique const & tech, RenderLayout const & rl) = 0;
		virtual void DoDispatch(RenderTechnique const & tech, uint32_t tgx, uint32_t tgy, uint32_t tgz) = 0;
		virtual void DoDispatchIndirect(RenderTechnique const & tech,
			GraphicsBufferPtr const & buff_args, uint32_t offset) = 0;
		virtual void DoResize(uint32_t width, uint32_t height) = 0;
		virtual void DoDestroy() = 0;

		virtual void DoSuspend() = 0;
		virtual void DoResume() = 0;

	protected:
		FrameBufferPtr cur_frame_buffer_;
		FrameBufferPtr screen_frame_buffer_;
		TexturePtr ds_tex_;
		FrameBufferPtr hdr_frame_buffer_;
		TexturePtr hdr_tex_;
		FrameBufferPtr ldr_frame_buffer_;
		TexturePtr ldr_tex_;
		FrameBufferPtr resize_frame_buffer_;
		TexturePtr resize_tex_;
		FrameBufferPtr mono_frame_buffer_;
		TexturePtr mono_tex_;
		FrameBufferPtr default_frame_buffers_[4];

		FrameBufferPtr overlay_frame_buffer_;
		TexturePtr overlay_tex_;

		RenderLayoutPtr so_buffers_;

		uint32_t num_primitives_just_rendered_;
		uint32_t num_vertices_just_rendered_;
		uint32_t num_draws_just_called_;
		uint32_t num_dispatches_just_called_;

		RenderDeviceCaps caps_;

		RasterizerStateObjectPtr cur_rs_obj_;
		RasterizerStateObjectPtr cur_line_rs_obj_;
		DepthStencilStateObjectPtr cur_dss_obj_;
		uint16_t cur_front_stencil_ref_;
		uint16_t cur_back_stencil_ref_;
		BlendStateObjectPtr cur_bs_obj_;
		Color cur_blend_factor_;
		uint32_t cur_sample_mask_;

		float default_fov_;
		float default_render_width_scale_;
		float default_render_height_scale_;

		uint32_t motion_frames_;

		StereoMethod stereo_method_;
		float stereo_separation_;
		// Oculus VR
		float4 ovr_hmd_warp_param_;
		float4 ovr_chrom_ab_param_;
		float ovr_x_center_offset_;
		float ovr_scale_;

		RenderLayoutPtr pp_rl_;

		PostProcessPtr hdr_pp_;
		PostProcessPtr skip_hdr_pp_;
		bool hdr_enabled_;
		PostProcessPtr ldr_pp_;
		int ppaa_enabled_;
		bool gamma_enabled_;
		bool color_grading_enabled_;
		PostProcessPtr resize_pps_[2];
		PostProcessPtr stereoscopic_pp_;
		int fb_stage_;
		bool pp_chain_dirty_;

		PostProcessPtr ldr_pps_[12];

		bool force_line_mode_;

		uint32_t native_shader_fourcc_;
		uint32_t native_shader_version_;

#ifndef KLAYGE_SHIP
		PerfRangePtr hdr_pp_perf_;
		PerfRangePtr ldr_pp_perf_;
		PerfRangePtr resize_pp_perf_;
		PerfRangePtr stereoscopic_pp_perf_;
#endif
	};
}

#endif			// _RENDERENGINE_HPP
