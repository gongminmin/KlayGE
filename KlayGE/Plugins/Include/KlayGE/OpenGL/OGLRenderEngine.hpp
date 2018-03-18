// OGLRenderEngine.hpp
// KlayGE OpenGL渲染引擎类 头文件
// Ver 3.12.0
// 版权所有(C) 龚敏敏, 2003-2010
// Homepage: http://www.klayge.org
//
// 3.11.0
// Remove TexelToPixelOffset (2010.9.26)
//
// 3.4.0
// 增加了TexelToPixelOffset (2006.8.27)
//
// 3.3.0
// 只支持OpenGL 2.0 (2006.5.21)
//
// 3.0.0
// 去掉了固定流水线 (2005.8.18)
//
// 2.8.0
// 增加了RenderDeviceCaps (2005.7.17)
// 简化了StencilBuffer相关操作 (2005.7.20)
//
// 2.7.0
// 去掉了TextureCoordSet (2005.6.26)
// TextureAddressingMode, TextureFiltering和TextureAnisotropy移到Texture中 (2005.6.27)
//
// 2.4.0
// 增加了PolygonMode (2005.3.20)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#ifndef _OGLRENDERENGINE_HPP
#define _OGLRENDERENGINE_HPP

#pragma once

#include <KFL/Vector.hpp>

#include <vector>
#include <map>
#include <set>

#include <glloader/glloader.h>

#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/ShaderObject.hpp>

namespace KlayGE
{
	class OGLRenderEngine : public RenderEngine
	{
	public:
		OGLRenderEngine();
		~OGLRenderEngine();

		std::wstring const & Name() const;

		bool RequiresFlipping() const
		{
			return false;
		}

		void ForceFlush();

		virtual TexturePtr const & ScreenDepthStencilTexture() const override;

		void ScissorRect(uint32_t x, uint32_t y, uint32_t width, uint32_t height);

		void GetCustomAttrib(std::string_view name, void* value) const override;

		bool FullScreen() const;
		void FullScreen(bool fs);

		void AdjustProjectionMatrix(float4x4& proj_mat);

		void ActiveTexture(GLenum tex_unit);
		void BindTexture(GLuint index, GLuint target, GLuint texture, bool force = false);
		void BindTextures(GLuint first, GLsizei count, GLuint const * targets, GLuint const * textures, bool force = false);
		void BindSampler(GLuint index, GLuint sampler, bool force = false);
		void BindSamplers(GLuint first, GLsizei count, GLuint const * samplers, bool force = false);
		void BindBuffer(GLenum target, GLuint buffer, bool force = false);
		void BindBuffersBase(GLenum target, GLuint first, GLsizei count, GLuint const * buffers, bool force = false);
		void DeleteTextures(GLsizei n, GLuint const * textures);
		void DeleteSamplers(GLsizei n, GLuint const * samplers);
		void DeleteBuffers(GLsizei n, GLuint const * buffers);
		void OverrideBindBufferCache(GLenum target, GLuint buffer);

		void ClearColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a);
		void ClearDepth(GLfloat depth);
		void ClearStencil(GLuint stencil);

		void UseProgram(GLuint program);

		void Uniform1i(GLint location, GLint value);
		void Uniform1ui(GLint location, GLuint value);
		void Uniform1f(GLint location, GLfloat value);
		void Uniform1iv(GLint location, GLsizei count, GLint const * value);
		void Uniform1uiv(GLint location, GLsizei count, GLuint const * value);
		void Uniform1fv(GLint location, GLsizei count, GLfloat const * value);
		void Uniform2iv(GLint location, GLsizei count, GLint const * value);
		void Uniform2uiv(GLint location, GLsizei count, GLuint const * value);
		void Uniform2fv(GLint location, GLsizei count, GLfloat const * value);
		void Uniform3iv(GLint location, GLsizei count, GLint const * value);
		void Uniform3uiv(GLint location, GLsizei count, GLuint const * value);
		void Uniform3fv(GLint location, GLsizei count, GLfloat const * value);
		void Uniform4iv(GLint location, GLsizei count, GLint const * value);
		void Uniform4uiv(GLint location, GLsizei count, GLuint const * value);
		void Uniform4fv(GLint location, GLsizei count, GLfloat const * value);
		void UniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, GLfloat const * value);

		void EnableFramebufferSRGB(bool srgb);

		void GetFBOForBlit(GLuint& src, GLuint& dst) const
		{
			src = fbo_blit_src_;
			dst = fbo_blit_dst_;
		}

		void BindFramebuffer(GLuint fbo, bool force = false);
		GLuint BindFramebuffer() const
		{
			return cur_fbo_;
		}
		void DeleteFramebuffers(GLsizei n, GLuint const * framebuffers);

		void SetPolygonMode(GLenum face, GLenum mode);

		bool HackForNV() const
		{
			return hack_for_nv_;
		}
		bool HackForAMD() const
		{
			return hack_for_amd_;
		}
		bool HackForIntel() const
		{
			return hack_for_intel_;
		}

#if defined KLAYGE_PLATFORM_WINDOWS
		HGLRC wglCreateContext(HDC hdc);
		BOOL wglDeleteContext(HGLRC hglrc);
		BOOL wglMakeCurrent(HDC hdc, HGLRC hglrc);
#endif

	private:
		virtual void DoCreateRenderWindow(std::string const & name, RenderSettings const & settings) override;
		virtual void DoBindFrameBuffer(FrameBufferPtr const & fb) override;
		virtual void DoBindSOBuffers(RenderLayoutPtr const & rl) override;
		virtual void DoRender(RenderEffect const & effect, RenderTechnique const & tech, RenderLayout const & rl) override;
		virtual void DoDispatch(RenderEffect const & effect, RenderTechnique const & tech,
			uint32_t tgx, uint32_t tgy, uint32_t tgz) override;
		virtual void DoDispatchIndirect(RenderEffect const & effect, RenderTechnique const & tech,
			GraphicsBufferPtr const & buff_args, uint32_t offset) override;
		virtual void DoResize(uint32_t width, uint32_t height) override;
		virtual void DoDestroy() override;
		virtual void DoSuspend() override;
		virtual void DoResume() override;

		void FillRenderDeviceCaps();
		void InitRenderStates();

		virtual void StereoscopicForLCDShutter(int32_t eye) override;

		bool VertexFormatSupport(ElementFormat elem_fmt);
		bool TextureFormatSupport(ElementFormat elem_fmt);
		bool RenderTargetFormatSupport(ElementFormat elem_fmt, uint32_t sample_count, uint32_t sample_quality);
		bool UAVFormatSupport(ElementFormat elem_fmt);

		virtual void CheckConfig(RenderSettings& settings) override;

	private:
#if defined KLAYGE_PLATFORM_WINDOWS
		typedef HGLRC (WINAPI *wglCreateContextFUNC)(HDC hdc);
		typedef BOOL (WINAPI *wglDeleteContextFUNC)(HGLRC hglrc);
		typedef BOOL (WINAPI *wglMakeCurrentFUNC)(HDC hdc, HGLRC hglrc);

		HMODULE mod_opengl32_;
		wglCreateContextFUNC DynamicWglCreateContext_;
		wglDeleteContextFUNC DynamicWglDeleteContext_;
		wglMakeCurrentFUNC DynamicWglMakeCurrent_;
#endif

		GLuint fbo_blit_src_;
		GLuint fbo_blit_dst_;

		std::array<GLfloat, 4> clear_clr_;
		GLfloat clear_depth_;
		GLuint clear_stencil_;

		GLuint cur_program_;

		GLint vp_x_, vp_y_;
		GLsizei vp_width_, vp_height_;

		GLuint cur_fbo_;

		RenderLayoutPtr so_rl_;
		GLenum so_primitive_mode_;
		std::vector<GLuint> so_buffs_;

		GLenum active_tex_unit_;
		std::vector<std::pair<GLuint, GLuint>> binded_textures_;
		std::vector<GLuint> binded_samplers_;
		std::map<GLenum, GLuint> binded_buffers_;
		std::map<GLenum, std::vector<GLuint>> binded_buffers_with_binding_points_;

		GLuint restart_index_;

		std::vector<ElementFormat> vertex_format_;
		std::vector<ElementFormat> texture_format_;
		std::vector<ElementFormat> rendertarget_format_;
		uint32_t max_samples_;

		std::map<GLuint, std::map<GLint, int4>> uniformi_cache_;
		std::map<GLuint, std::map<GLint, float4>> uniformf_cache_;
		bool fb_srgb_cache_;

		GLenum polygon_mode_override_;

		bool hack_for_nv_;
		bool hack_for_amd_;
		bool hack_for_intel_;
	};
}

#endif			// _OGLRENDERENGINE_HPP
