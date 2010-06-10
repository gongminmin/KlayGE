// OGLES2RenderEngine.hpp
// KlayGE OpenGL ES 2渲染引擎类 头文件
// Ver 3.10.0
// 版权所有(C) 龚敏敏, 2010
// Homepage: http://www.klayge.org
//
// 3.10.0
// 初次建立 (2010.1.22)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#ifndef _OGLES2RENDERENGINE_HPP
#define _OGLES2RENDERENGINE_HPP

#pragma once

#include <KlayGE/Vector.hpp>

#include <vector>
#include <map>
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable: 6385)
#endif
#include <boost/array.hpp>
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(pop)
#endif

#include <glloader/glloader.h>

#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/ShaderObject.hpp>

namespace KlayGE
{
	class OGLES2RenderEngine : public RenderEngine
	{
	public:
		OGLES2RenderEngine();
		~OGLES2RenderEngine();

		std::wstring const & Name() const;

		void StartRendering();

		void BeginFrame();
		void EndFrame();
		void BeginPass();
		void EndPass();

		uint16_t StencilBufferBitDepth();

		void ScissorRect(uint32_t x, uint32_t y, uint32_t width, uint32_t height);

		float4 TexelToPixelOffset() const
		{
			return float4(0, 0, 0, 0);
		}

		bool FullScreen() const;
		void FullScreen(bool fs);

		void TexParameter(GLuint tex, GLenum target, GLenum pname, GLint param);
		void TexParameterf(GLuint tex, GLenum target, GLenum pname, GLfloat param);
		void ClearColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a);
		void ClearDepth(GLfloat depth);
		void ClearStencil(GLuint stencil);

		void BindFramebuffer(GLuint fbo, bool force = false);
		GLuint BindFramebuffer() const
		{
			return cur_fbo_;
		}

	private:
		void DoCreateRenderWindow(std::string const & name, RenderSettings const & settings);
		void DoBindFrameBuffer(FrameBufferPtr const & fb);
		void DoBindSOBuffers(RenderLayoutPtr const & rl);
		void DoRender(RenderTechnique const & tech, RenderLayout const & rl);
		void DoDispatch(RenderTechnique const & tech, uint32_t tgx, uint32_t tgy, uint32_t tgz);
		void DoResize(uint32_t width, uint32_t height);

		void FillRenderDeviceCaps();
		void InitRenderStates();

	private:
		boost::array<GLfloat, 4> clear_clr_;
		GLfloat clear_depth_;
		GLuint clear_stencil_;

		GLint vp_x_, vp_y_;
		GLsizei vp_width_, vp_height_;

		GLuint cur_fbo_;
	};

	typedef boost::shared_ptr<OGLES2RenderEngine> OGLES2RenderEnginePtr;
}

#endif			// _OGLES2RENDERENGINE_HPP
