// OGLRenderEngine.cpp
// KlayGE OpenGL渲染引擎类 实现文件
// Ver 3.0.0
// 版权所有(C) 龚敏敏, 2004-2005
// Homepage: http://klayge.sourceforge.net
//
// 3.0.0
// 去掉了固定流水线 (2005.8.18)
//
// 2.8.0
// 增加了RenderDeviceCaps (2005.7.17)
// 简化了StencilBuffer相关操作 (2005.7.20)
// 只支持vbo (2005.7.31)
// 只支持OpenGL 1.5及以上 (2005.8.12)
//
// 2.7.0
// 支持vertex_buffer_object (2005.6.19)
// 支持OpenGL 1.3多纹理 (2005.6.26)
// 去掉了TextureCoordSet (2005.6.26)
// TextureAddressingMode, TextureFiltering和TextureAnisotropy移到Texture中 (2005.6.27)
//
// 2.4.0
// 增加了PolygonMode (2005.3.20)
//
// 2.0.1
// 初次建立 (2003.10.11)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/Viewport.hpp>
#include <KlayGE/VertexBuffer.hpp>
#include <KlayGE/RenderTarget.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/RenderSettings.hpp>
#include <KlayGE/Util.hpp>

#include <glloader/glloader.h>

#include <algorithm>
#include <cstring>
#include <boost/assert.hpp>

#include <KlayGE/OpenGL/OGLMapping.hpp>
#include <KlayGE/OpenGL/OGLRenderWindow.hpp>
#include <KlayGE/OpenGL/OGLTexture.hpp>
#include <KlayGE/OpenGL/OGLVertexStream.hpp>
#include <KlayGE/OpenGL/OGLIndexStream.hpp>
#include <KlayGE/OpenGL/OGLRenderEngine.hpp>

#ifdef KLAYGE_DEBUG
	#pragma comment(lib, "glloader_d.lib")
#else
	#pragma comment(lib, "glloader.lib")
#endif
#pragma comment(lib, "glu32.lib")

namespace KlayGE
{
	// 构造函数
	/////////////////////////////////////////////////////////////////////////////////
	OGLRenderEngine::OGLRenderEngine()
		: cullingMode_(RenderEngine::CM_None)
	{
	}

	// 析构函数
	/////////////////////////////////////////////////////////////////////////////////
	OGLRenderEngine::~OGLRenderEngine()
	{
	}

	// 返回渲染系统的名字
	/////////////////////////////////////////////////////////////////////////////////
	std::wstring const & OGLRenderEngine::Name() const
	{
		static const std::wstring name(L"OpenGL Render Engine");
		return name;
	}

	// 开始渲染
	/////////////////////////////////////////////////////////////////////////////////
	void OGLRenderEngine::StartRendering()
	{
		bool gotMsg;
		MSG  msg;

		::PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE);

		RenderTarget& renderTarget = *this->ActiveRenderTarget(0);
		while (WM_QUIT != msg.message)
		{
			// 如果窗口是激活的，用 PeekMessage()以便我们可以用空闲时间渲染场景
			// 不然, 用 GetMessage() 减少 CPU 占用率
			if (renderTarget.Active())
			{
				gotMsg = ::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) ? true : false;
			}
			else
			{
				gotMsg = ::GetMessage(&msg, NULL, 0, 0) ? true : false;
			}

			if (gotMsg)
			{
				::TranslateMessage(&msg);
				::DispatchMessage(&msg);
			}
			else
			{
				// 在空余时间渲染帧 (没有等待的消息)
				if (renderTarget.Active())
				{
					renderTarget.Update();
				}
			}
		}
	}

	// 设置清除颜色
	/////////////////////////////////////////////////////////////////////////////////
	void OGLRenderEngine::ClearColor(Color const & col)
	{
		glClearColor(col.r(), col.g(), col.b(), col.a());
	}

	// 清空缓冲区
	/////////////////////////////////////////////////////////////////////////////////
	void OGLRenderEngine::Clear(uint32_t masks)
	{
		uint32_t flags = 0;
		if (masks & CBM_Color)
		{
			flags |= GL_COLOR_BUFFER_BIT;
		}
		if (masks & CBM_Depth)
		{
			flags |= GL_DEPTH_BUFFER_BIT;
		}
		if (masks & CBM_Stencil)
		{
			flags |= GL_STENCIL_BUFFER_BIT;
		}

		glClear(flags);
	}

	// 设置光影类型
	/////////////////////////////////////////////////////////////////////////////////
	void OGLRenderEngine::ShadingType(ShadeOptions so)
	{
		glShadeModel(OGLMapping::Mapping(so));
	}


	// 建立渲染窗口
	/////////////////////////////////////////////////////////////////////////////////
	RenderWindowPtr OGLRenderEngine::CreateRenderWindow(std::string const & name,
		RenderSettings const & settings)
	{
		RenderWindowPtr win(new OGLRenderWindow(name, settings));

		this->FillRenderDeviceCaps();
		renderTargets_.resize(caps_.max_simultaneous_rts);

		this->ActiveRenderTarget(0, win);

		this->DepthBufferDepthTest(settings.depthBuffer);
		this->DepthBufferDepthWrite(settings.depthBuffer);

		if (glloader_GL_VERSION_2_0())
		{
			glPointParameterf_ = glPointParameterf;
			glPointParameterfv_ = glPointParameterfv;
		}
		else
		{
			if (glloader_GL_ARB_point_parameters() && glloader_GL_ARB_point_sprite())
			{
				glPointParameterf_ = glPointParameterfARB;
				glPointParameterfv_ = glPointParameterfvARB;
			}
		}

		return win;
	}

	// 设置剪裁模式
	/////////////////////////////////////////////////////////////////////////////////
	void OGLRenderEngine::CullingMode(CullMode mode)
	{
		cullingMode_ = mode;

		switch (mode)
		{
		case CM_None:
			glDisable(GL_CULL_FACE);
			break;

		case CM_Clockwise:
			glEnable(GL_CULL_FACE);
			glFrontFace(GL_CCW);
			break;

		case CM_AntiClockwise:
			glEnable(GL_CULL_FACE);
			glFrontFace(GL_CW);
			break;
		}
	}

	// 设置多变性填充模式
	/////////////////////////////////////////////////////////////////////////////////
	void OGLRenderEngine::PolygonMode(FillMode mode)
	{
		glPolygonMode(GL_FRONT_AND_BACK, OGLMapping::Mapping(mode));
	}

	// 设置当前渲染目标
	/////////////////////////////////////////////////////////////////////////////////
	void OGLRenderEngine::DoActiveRenderTarget(uint32_t n, RenderTargetPtr renderTarget)
	{
		this->CullingMode(cullingMode_);

		Viewport const & vp(renderTarget->GetViewport());
		glViewport(vp.left, vp.top, vp.width, vp.height);
	}

	// 开始一帧
	/////////////////////////////////////////////////////////////////////////////////
	void OGLRenderEngine::BeginFrame()
	{
	}

	// 渲染
	/////////////////////////////////////////////////////////////////////////////////
	void OGLRenderEngine::DoRender(RenderLayout const & rl)
	{
		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_NORMAL_ARRAY);
		glDisableClientState(GL_COLOR_ARRAY);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);

		uint32_t const num_instance = rl.NumInstance();
		BOOST_ASSERT(num_instance != 0);

		for (uint32_t instance = 0; instance < num_instance; ++ instance)
		{
			if (rl.InstanceStream())
			{
				VertexStream& stream = *rl.InstanceStream();

				uint32_t const instance_size = rl.InstanceSize();
				VertexStream::Mapper mapper(stream, BA_Read_Only);
				uint8_t const * buffer = mapper.Pointer<uint8_t>();

				uint32_t elem_offset = 0;
				for (uint32_t i = 0; i < rl.InstanceStreamFormat().size(); ++ i)
				{
					vertex_element const & vs_elem = rl.InstanceStreamFormat()[i];
					void const * addr = &buffer[instance * instance_size + elem_offset];

					switch (vs_elem.usage)
					{
					// Vertex xyzs
					case VEU_Position:
						switch (vs_elem.num_components)
						{
						case 2:
							glVertex2fv(static_cast<GLfloat const *>(addr));
							break;

						case 3:
							glVertex3fv(static_cast<GLfloat const *>(addr));
							break;

						case 4:
							glVertex4fv(static_cast<GLfloat const *>(addr));
							break;
						}
						break;
					
					case VEU_Normal:
						glNormal3fv(static_cast<GLfloat const *>(addr));
						break;

					case VEU_Diffuse:
						switch (vs_elem.num_components)
						{
						case 3:
							glColor3fv(static_cast<GLfloat const *>(addr));
							break;

						case 4:
							glColor4fv(static_cast<GLfloat const *>(addr));
							break;
						}
						break;

					case VEU_Specular:
						glSecondaryColor3fv(static_cast<GLfloat const *>(addr));
						break;

					case VEU_TextureCoord:
						{
							GLenum target = GL_TEXTURE0 + vs_elem.usage_index;
							switch (vs_elem.num_components)
							{
							case 1:
								glMultiTexCoord1fv(target, static_cast<GLfloat const *>(addr));
								break;

							case 2:
								glMultiTexCoord2fv(target, static_cast<GLfloat const *>(addr));
								break;

							case 3:
								glMultiTexCoord3fv(target, static_cast<GLfloat const *>(addr));
								break;

							case 4:
								glMultiTexCoord4fv(target, static_cast<GLfloat const *>(addr));
								break;
							}
						}
						break;

					default:
						BOOST_ASSERT(false);
						break;
					}

					elem_offset += vs_elem.element_size();
				}
			}

			// Geometry streams
			for (uint32_t i = 0; i < rl.NumVertexStreams(); ++ i)
			{
				OGLVertexStream& stream(static_cast<OGLVertexStream&>(*rl.GetVertexStream(i)));

				uint32_t elem_offset = 0;
				for (uint32_t j = 0; j < rl.VertexStreamFormat(i).size(); ++ j)
				{
					vertex_element const & vs_elem = rl.VertexStreamFormat(i)[j];

					switch (vs_elem.usage)
					{
					// Vertex xyzs
					case VEU_Position:
						glEnableClientState(GL_VERTEX_ARRAY);
						stream.Active();
						glVertexPointer(3, GL_FLOAT, rl.VertexSize(i),
							reinterpret_cast<GLvoid*>(elem_offset));
						break;
				
					case VEU_Normal:
						glEnableClientState(GL_NORMAL_ARRAY);
						stream.Active();
						glNormalPointer(GL_FLOAT, rl.VertexSize(i),
							reinterpret_cast<GLvoid*>(elem_offset));
						break;

					case VEU_Diffuse:
						glEnableClientState(GL_COLOR_ARRAY);
						stream.Active();
						glColorPointer(4, GL_UNSIGNED_BYTE, rl.VertexSize(i),
							reinterpret_cast<GLvoid*>(elem_offset));
						break;

					case VEU_Specular:
						glEnableClientState(GL_SECONDARY_COLOR_ARRAY);
						stream.Active();
						glSecondaryColorPointer(4, GL_UNSIGNED_BYTE, rl.VertexSize(i),
							reinterpret_cast<GLvoid*>(elem_offset));
						break;

					case VEU_TextureCoord:
						glClientActiveTexture(GL_TEXTURE0 + vs_elem.usage_index);
						glEnableClientState(GL_TEXTURE_COORD_ARRAY);
						stream.Active();
						glTexCoordPointer(static_cast<GLint>(vs_elem.num_components),
								GL_FLOAT, rl.VertexSize(i), reinterpret_cast<GLvoid*>(elem_offset));
						break;

					default:
						BOOST_ASSERT(false);
						break;
					}

					elem_offset += vs_elem.element_size();
				}
			}

			size_t const vertexCount = rl.UseIndices() ? rl.NumIndices() : rl.NumVertices();
			GLenum mode;
			uint32_t primCount;
			OGLMapping::Mapping(mode, primCount, rl);

			numPrimitivesJustRendered_ += primCount;
			numVerticesJustRendered_ += vertexCount;

			RenderTechniquePtr tech = renderEffect_->ActiveTechnique();
			uint32_t num_passes = tech->NumPasses();
			if (rl.UseIndices())
			{
				OGLIndexStream& stream(static_cast<OGLIndexStream&>(*rl.GetIndexStream()));
				stream.Active();

				GLenum index_type;
				if (IF_Index16 == rl.IndexStreamFormat())
				{
					index_type = GL_UNSIGNED_SHORT;
				}
				else
				{
					index_type = GL_UNSIGNED_INT;
				}

				for (uint32_t i = 0; i < num_passes; ++ i)
				{
					RenderPassPtr pass = tech->Pass(i);

					pass->Begin();
					glDrawElements(mode, static_cast<GLsizei>(rl.NumIndices()),
						index_type, 0);
					pass->End();
				}
			}
			else
			{
				for (uint32_t i = 0; i < num_passes; ++ i)
				{
					RenderPassPtr pass = tech->Pass(i);

					pass->Begin();
					glDrawArrays(mode, 0, static_cast<GLsizei>(rl.NumVertices()));
					pass->End();
				}
			}
		}
	}

	// 结束一帧
	/////////////////////////////////////////////////////////////////////////////////
	void OGLRenderEngine::EndFrame()
	{
	}

	// 打开/关闭深度测试
	/////////////////////////////////////////////////////////////////////////////////
	void OGLRenderEngine::AlphaBlend(bool enabled)
	{
		if (enabled)
		{
			glEnable(GL_BLEND);
		}
		else
		{
			glDisable(GL_BLEND);
		}
	}

	// 设置Alpha混合因数
	/////////////////////////////////////////////////////////////////////////////////
	void OGLRenderEngine::AlphaBlendFunction(AlphaBlendFactor src_factor, AlphaBlendFactor dst_factor)
	{
		glBlendFunc(OGLMapping::Mapping(src_factor), OGLMapping::Mapping(dst_factor));
	}

	// 打开/关闭深度测试
	/////////////////////////////////////////////////////////////////////////////////
	void OGLRenderEngine::DepthBufferDepthTest(bool enabled)
	{
		if (enabled)
		{
			glClearDepth(1.0f);
			glEnable(GL_DEPTH_TEST);
		}
		else
		{
			glDisable(GL_DEPTH_TEST);
		}
	}

	// 打开/关闭深度缓存
	/////////////////////////////////////////////////////////////////////////////////
	void OGLRenderEngine::DepthBufferDepthWrite(bool enabled)
	{
		glDepthMask(enabled ? GL_TRUE : GL_FALSE);
	}

	// 设置深度比较函数
	/////////////////////////////////////////////////////////////////////////////////
	void OGLRenderEngine::DepthBufferFunction(CompareFunction func)
	{
		glDepthFunc(OGLMapping::Mapping(func));
	}

	// 设置深度偏移
	/////////////////////////////////////////////////////////////////////////////////
	void OGLRenderEngine::DepthBias(uint16_t bias)
	{
		glEnable(GL_POLYGON_OFFSET_FILL);
		glEnable(GL_POLYGON_OFFSET_POINT);
		glEnable(GL_POLYGON_OFFSET_LINE);
		// Bias is in {0, 16}, scale the unit addition appropriately
		glPolygonOffset(1.0f, bias);
	}

	// 打开/关闭Alpha测试
	/////////////////////////////////////////////////////////////////////////////////
	void OGLRenderEngine::AlphaTest(bool enabled)
	{
		if (enabled)
		{
			glEnable(GL_ALPHA_TEST);
		}
		else
		{
			glDisable(GL_ALPHA_TEST);
		}
	}

	// 设置Alpha比较函数和参考值
	/////////////////////////////////////////////////////////////////////////////////
	void OGLRenderEngine::AlphaFunction(CompareFunction alphaFunction, float refValue)
	{
		glAlphaFunc(OGLMapping::Mapping(alphaFunction), refValue);
	}

	// 设置纹理
	/////////////////////////////////////////////////////////////////////////////////
	void OGLRenderEngine::SetSampler(uint32_t stage, SamplerPtr const & sampler)
	{
		TexturePtr texture = sampler->GetTexture();

		glActiveTexture(GL_TEXTURE0 + stage);

		OGLTexture& gl_tex = *checked_cast<OGLTexture*>(texture.get());
		GLenum tex_type = gl_tex.GLType();
		if (!texture)
		{
			glDisable(tex_type);
		}
		else
		{
			glEnable(tex_type);
			gl_tex.GLBindTexture();

			glTexParameteri(tex_type, GL_TEXTURE_WRAP_S, OGLMapping::Mapping(sampler->AddressingMode(Sampler::TAT_Addr_U)));
			glTexParameteri(tex_type, GL_TEXTURE_WRAP_T, OGLMapping::Mapping(sampler->AddressingMode(Sampler::TAT_Addr_V)));
			glTexParameteri(tex_type, GL_TEXTURE_WRAP_R, OGLMapping::Mapping(sampler->AddressingMode(Sampler::TAT_Addr_W)));

			glTexParameterfv(tex_type, GL_TEXTURE_BORDER_COLOR, &sampler->BorderColor().r());

			switch (sampler->Filtering())
			{
			case Sampler::TFO_None:
			case Sampler::TFO_Point:
				glTexParameteri(tex_type, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				glTexParameteri(tex_type, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				break;

			case Sampler::TFO_Bilinear:
				glTexParameteri(tex_type, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(tex_type, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
				break;

			case Sampler::TFO_Trilinear:
				glTexParameteri(tex_type, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(tex_type, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
				break;

			case Sampler::TFO_Anisotropic:
				if (caps_.max_texture_anisotropy != 0)
				{
					uint32_t anisotropy = std::min(caps_.max_texture_anisotropy, sampler->Anisotropy());
					glTexParameteri(tex_type, GL_TEXTURE_MAX_ANISOTROPY_EXT, anisotropy);
				}	
				break;

			default:
				BOOST_ASSERT(false);
				break;
			}

			glTexParameteri(tex_type, GL_TEXTURE_MAX_LEVEL, sampler->MaxMipLevel());

			{
				GLfloat bias;
				glGetFloatv(GL_MAX_TEXTURE_LOD_BIAS, &bias);
				bias = std::min(sampler->MipMapLodBias(), bias);
				glTexEnvf(GL_TEXTURE_FILTER_CONTROL, GL_TEXTURE_LOD_BIAS, bias);
			}

			{
				Matrix4 oglMat(MathLib::LHToRH(sampler->TextureMatrix()));

				glPushAttrib(GL_TRANSFORM_BIT);
				glMatrixMode(GL_TEXTURE);
				glLoadMatrixf(&oglMat(0, 0));
				glPopAttrib();
			}
		}
	}

	// 关闭某个纹理阶段
	/////////////////////////////////////////////////////////////////////////////////
	void OGLRenderEngine::DisableSampler(uint32_t stage)
	{
		glActiveTexture(GL_TEXTURE0 + stage);
		glDisable(GL_TEXTURE_1D);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_TEXTURE_3D);
		glDisable(GL_TEXTURE_CUBE_MAP);
	}

	// 打开模板缓冲区
	/////////////////////////////////////////////////////////////////////////////////
	void OGLRenderEngine::StencilCheckEnabled(bool enabled)
	{
		if (enabled)
		{
			glEnable(GL_STENCIL_TEST);
		}
		else
		{
			glDisable(GL_STENCIL_TEST);
		}
	}

	// 硬件是否支持模板缓冲区
	/////////////////////////////////////////////////////////////////////////////////
	bool OGLRenderEngine::HasHardwareStencil()
	{
		return true;
	}

	// 设置模板位数
	/////////////////////////////////////////////////////////////////////////////////
	uint16_t OGLRenderEngine::StencilBufferBitDepth()
	{
		return 8;
	}

		// 设置模板比较函数，参考值和掩码
	/////////////////////////////////////////////////////////////////////////////////
	void OGLRenderEngine::StencilBufferFunction(CompareFunction func, uint32_t refValue, uint32_t mask)
	{
		glStencilFunc(OGLMapping::Mapping(func), refValue, mask);
	}

	// 设置模板缓冲区模板测试失败，深度测试失败和通过后的操作
	/////////////////////////////////////////////////////////////////////////////////
	void OGLRenderEngine::StencilBufferOperation(StencilOperation fail,
		StencilOperation depth_fail, StencilOperation pass)
	{
		glStencilOp(OGLMapping::Mapping(fail), OGLMapping::Mapping(depth_fail), OGLMapping::Mapping(pass));
	}

	// 打开/关闭点精灵模式
	/////////////////////////////////////////////////////////////////////////////////
	void OGLRenderEngine::PointSpriteEnable(bool enable)
	{
		if (glPointParameterfv_ != NULL)
		{
			if (enable)
			{
				glTexEnvf(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);
				glEnable(GL_POINT_SPRITE);
			}
			else
			{
				glDisable(GL_POINT_SPRITE);
			}
		}
	}

	// 设置点距离参数
	/////////////////////////////////////////////////////////////////////////////////
	void OGLRenderEngine::PointDistanceAttenuation(float quadratic0, float quadratic1, float quadratic2)
	{
		if (glPointParameterfv_ != NULL)
		{
			float quadratic[] = { quadratic0, quadratic1, quadratic2 };
			glPointParameterfv_(GL_POINT_DISTANCE_ATTENUATION, quadratic);
		}
	}

	// 设置点大小
	/////////////////////////////////////////////////////////////////////////////////
	void OGLRenderEngine::PointSize(float size)
	{
		if (glPointParameterfv_ != NULL)
		{
			glPointSize(size);
		}
	}

	// 设置点的最小最大大小
	/////////////////////////////////////////////////////////////////////////////////
	void OGLRenderEngine::PointMinMaxSize(float min_size, float max_size)
	{
		if (glPointParameterf_ != NULL)
		{
			glPointParameterf_(GL_POINT_SIZE_MIN, std::max(min_size, caps_.min_point_size));
			glPointParameterf_(GL_POINT_SIZE_MAX, std::min(max_size, caps_.max_point_size));
		}
	}

	// 打开/关闭剪除测试
	/////////////////////////////////////////////////////////////////////////////////
	void OGLRenderEngine::ScissorTest(bool enabled)
	{
		if (enabled)
		{
			glEnable(GL_SCISSOR_TEST);
		}
		else
		{
			glDisable(GL_SCISSOR_TEST);
		}
	}

	// 设置剪除矩阵
	/////////////////////////////////////////////////////////////////////////////////
	void OGLRenderEngine::ScissorRect(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
	{
		glScissor(x, y, width, height);
	}

	// 填充设备能力
	/////////////////////////////////////////////////////////////////////////////////
	void OGLRenderEngine::FillRenderDeviceCaps()
	{
		GLint temp;

		if (glloader_GL_VERSION_2_0() || glloader_GL_ARB_vertex_shader())
		{
			glGetIntegerv(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, &temp);
			caps_.max_vertex_texture_units = temp;
		}
		else
		{
			caps_.max_vertex_texture_units = 0;
		}

		if (glloader_GL_VERSION_2_0()
			|| (glloader_GL_ARB_vertex_shader() && glloader_GL_ARB_fragment_shader()))
		{
			if (caps_.max_vertex_texture_units != 0)
			{
				caps_.max_shader_model = 3;
			}
			else
			{
				caps_.max_shader_model = 2;
			}
		}
		else
		{
			if (glloader_GL_ARB_vertex_program() && glloader_GL_ARB_fragment_program())
			{
				caps_.max_shader_model = 1;
			}
			else
			{
				caps_.max_shader_model = 0;
			}
		}

		glGetIntegerv(GL_MAX_TEXTURE_SIZE, &temp);
		caps_.max_texture_height = caps_.max_texture_width = temp;
		glGetIntegerv(GL_MAX_3D_TEXTURE_SIZE, &temp);
		caps_.max_texture_depth = temp;
		
		glGetIntegerv(GL_MAX_CUBE_MAP_TEXTURE_SIZE, &temp);
		caps_.max_texture_cube_size = temp;

		glGetIntegerv(GL_MAX_TEXTURE_UNITS, &temp);
		caps_.max_textures_units = temp;

		if (glloader_GL_EXT_texture_filter_anisotropic())
		{
			glGetIntegerv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &temp);
			caps_.max_texture_anisotropy = temp;
		}
		else
		{
			caps_.max_texture_anisotropy = 0;
		}

		caps_.max_user_clip_planes = 6;

		if (glloader_GL_VERSION_2_0() || glloader_GL_ARB_draw_buffers())
		{
			glGetIntegerv(GL_MAX_DRAW_BUFFERS, &temp);
			caps_.max_simultaneous_rts	= temp;
		}
		else
		{
			caps_.max_simultaneous_rts	= 1;
		}

		glGetIntegerv(GL_MAX_ELEMENTS_VERTICES, &temp);
		caps_.max_vertices = temp;
		glGetIntegerv(GL_MAX_ELEMENTS_INDICES, &temp);
		caps_.max_indices = temp;

		caps_.texture_2d_filter_caps = Sampler::TFO_Point | Sampler::TFO_Bilinear | Sampler::TFO_Trilinear | Sampler::TFO_Anisotropic;
		caps_.texture_1d_filter_caps = caps_.texture_2d_filter_caps;
		caps_.texture_3d_filter_caps = caps_.texture_2d_filter_caps;
		caps_.texture_cube_filter_caps = caps_.texture_2d_filter_caps;

		glGetFloatv(GL_POINT_SIZE_MIN, &caps_.min_point_size);
		glGetFloatv(GL_POINT_SIZE_MAX, &caps_.max_point_size);

		caps_.hw_instancing_support = true;
	}
}
