// OGLESRenderEngine.hpp
// KlayGE OpenGL ES��Ⱦ������ ͷ�ļ�
// Ver 3.12.0
// ��Ȩ����(C) ������, 2010
// Homepage: http://www.klayge.org
//
// 3.11.0
// Remove TexelToPixelOffset (2010.9.26)
//
// 3.10.0
// ���ν��� (2010.1.22)
//
// �޸ļ�¼
//////////////////////////////////////////////////////////////////////////////////

#ifndef _OGLESRENDERENGINE_HPP
#define _OGLESRENDERENGINE_HPP

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
	class OGLESRenderEngine : public RenderEngine
	{
	public:
		OGLESRenderEngine();
		~OGLESRenderEngine();

		std::wstring const & Name() const;

		bool RequiresFlipping() const
		{
			return false;
		}

		void UpdateGPUTimestampsFrequency() override;

		void ForceFlush();

		virtual TexturePtr const & ScreenDepthStencilTexture() const override;

		void ScissorRect(uint32_t x, uint32_t y, uint32_t width, uint32_t height);

		void GetCustomAttrib(std::string const & name, void* value);

		bool FullScreen() const;
		void FullScreen(bool fs);

		void AdjustProjectionMatrix(float4x4& proj_mat);

		void ActiveTexture(GLenum tex_unit);
		void BindTexture(GLuint index, GLuint target, GLuint texture, bool force = false);
		void BindTextures(GLuint first, GLsizei count, GLuint const * targets, GLuint const * textures, bool force = false);
		void BindBuffer(GLenum target, GLuint buffer, bool force = false);
		void BindBuffersBase(GLenum target, GLuint first, GLsizei count, GLuint const * buffers, bool force = false);
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

		bool HackForTegra() const
		{
			return hack_for_tegra_;
		}
		bool HackForPVR() const
		{
			return hack_for_pvr_;
		}
		bool HackForMali() const
		{
			return hack_for_mali_;
		}
		bool HackForAdreno() const
		{
			return hack_for_adreno_;
		}
		bool HackForAndroidEmulator() const
		{
			return hack_for_android_emulator_;
		}
		bool HackForAngle() const
		{
			return hack_for_angle_;
		}

		bool GPUDisjointOccurred() const
		{
			return gpu_disjoint_occurred_;
		}

	private:
		virtual void DoCreateRenderWindow(std::string const & name, RenderSettings const & settings) override;
		virtual void DoBindFrameBuffer(FrameBufferPtr const & fb) override;
		virtual void DoBindSOBuffers(RenderLayoutPtr const & rl) override;
		virtual void DoRender(RenderTechnique const & tech, RenderLayout const & rl) override;
		virtual void DoDispatch(RenderTechnique const & tech, uint32_t tgx, uint32_t tgy, uint32_t tgz) override;
		virtual void DoDispatchIndirect(RenderTechnique const & tech,
			GraphicsBufferPtr const & buff_args, uint32_t offset) override;
		virtual void DoResize(uint32_t width, uint32_t height) override;
		virtual void DoDestroy() override;
		virtual void DoSuspend() override;
		virtual void DoResume() override;

		void FillRenderDeviceCaps();
		void InitRenderStates();

		bool VertexFormatSupport(ElementFormat elem_fmt);
		bool TextureFormatSupport(ElementFormat elem_fmt);
		bool RenderTargetFormatSupport(ElementFormat elem_fmt, uint32_t sample_count, uint32_t sample_quality);

		virtual void CheckConfig(RenderSettings& settings) override;

	private:
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
		std::vector<std::string> so_vars_;
		std::vector<char const *> so_vars_ptrs_;
		std::vector<GLuint> so_buffs_;

		GLenum active_tex_unit_;
		std::vector<GLuint> binded_targets_;
		std::vector<GLuint> binded_textures_;
		std::map<GLenum, GLuint> binded_buffers_;
		std::map<GLenum, std::vector<GLuint>> binded_buffers_with_binding_points_;

		std::set<ElementFormat> vertex_format_;
		std::set<ElementFormat> texture_format_;
		std::set<ElementFormat> rendertarget_format_;
		uint32_t max_samples_;

		std::map<GLuint, std::map<GLint, int4>> uniformi_cache_;
		std::map<GLuint, std::map<GLint, float4>> uniformf_cache_;

		bool hack_for_tegra_;
		bool hack_for_pvr_;
		bool hack_for_mali_;
		bool hack_for_adreno_;
		bool hack_for_android_emulator_;
		bool hack_for_angle_;

		bool gpu_disjoint_occurred_;
	};
}

#endif			// _OGLESRENDERENGINE_HPP
