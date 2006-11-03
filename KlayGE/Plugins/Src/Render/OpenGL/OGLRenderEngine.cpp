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
#include <KlayGE/GraphicsBuffer.hpp>
#include <KlayGE/RenderLayout.hpp>
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
#include <KlayGE/OpenGL/OGLFrameBuffer.hpp>
#include <KlayGE/OpenGL/OGLTexture.hpp>
#include <KlayGE/OpenGL/OGLGraphicsBuffer.hpp>
#include <KlayGE/OpenGL/OGLRenderLayout.hpp>
#include <KlayGE/OpenGL/OGLRenderEngine.hpp>
#include <KlayGE/OpenGL/OGLShaderObject.hpp>

#ifdef KLAYGE_COMPILER_MSVC
#ifdef KLAYGE_DEBUG
	#pragma comment(lib, "glloader_d.lib")
#else
	#pragma comment(lib, "glloader.lib")
#endif
#pragma comment(lib, "glu32.lib")
#endif

namespace KlayGE
{
	// 构造函数
	/////////////////////////////////////////////////////////////////////////////////
	OGLRenderEngine::OGLRenderEngine()
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

		RenderTarget& renderTarget = *this->CurRenderTarget();
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

	// 清空缓冲区
	/////////////////////////////////////////////////////////////////////////////////
	void OGLRenderEngine::Clear(uint32_t masks, Color const & clr, float depth, int32_t stencil)
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

		glClearColor(clr.r(), clr.g(), clr.b(), clr.a());
		glClearDepth(depth);
		glClearStencil(stencil);
		glClear(flags);
	}

	// 建立渲染窗口
	/////////////////////////////////////////////////////////////////////////////////
	RenderWindowPtr OGLRenderEngine::CreateRenderWindow(std::string const & name,
		RenderSettings const & settings)
	{
		RenderWindowPtr win(new OGLRenderWindow(name, settings));
		default_render_target_ = win;

		this->FillRenderDeviceCaps();
		this->BindRenderTarget(win);

		return win;
	}

	// 设置当前渲染状态对象
	/////////////////////////////////////////////////////////////////////////////////
	void OGLRenderEngine::SetStateObjects(RenderStateObject const & rs_obj, ShaderObject const & shader_obj)
	{
		for (int i = 0; i < RenderStateObject::RST_NUM_RENDER_STATES; ++ i)
		{
			RenderStateObject::RenderStateType rst = static_cast<RenderStateObject::RenderStateType>(i);
			uint32_t state = rs_obj.GetRenderState(rst);
			if (cur_render_state_obj_.GetRenderState(rst) != state)
			{
				switch (rst)
				{
				case RenderStateObject::RST_PolygonMode:
					glPolygonMode(GL_FRONT_AND_BACK,
						OGLMapping::Mapping(static_cast<RenderStateObject::PolygonMode>(state)));
					break;

				case RenderStateObject::RST_ShadeMode:
					glShadeModel(OGLMapping::Mapping(static_cast<RenderStateObject::ShadeMode>(state)));
					break;

				case RenderStateObject::RST_CullMode:
					switch (state)
					{
					case RenderStateObject::CM_None:
						glDisable(GL_CULL_FACE);
						break;

					case RenderStateObject::CM_Clockwise:
						glEnable(GL_CULL_FACE);
						glFrontFace(GL_CCW);
						break;

					case RenderStateObject::CM_AntiClockwise:
						glEnable(GL_CULL_FACE);
						glFrontFace(GL_CW);
						break;
					}
					break;

				case RenderStateObject::RST_AlphaToCoverageEnable:
					if (state)
					{
						glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);
					}
					else
					{
						glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE);
					}
					break;

				case RenderStateObject::RST_BlendEnable:
					if (state)
					{
						glEnable(GL_BLEND);
					}
					else
					{
						glDisable(GL_BLEND);
					}
					break;

				case RenderStateObject::RST_BlendOp:
				case RenderStateObject::RST_BlendOpAlpha:
					glBlendEquationSeparate(OGLMapping::Mapping(static_cast<RenderStateObject::BlendOperation>(rs_obj.GetRenderState(RenderStateObject::RST_BlendOp))),
						OGLMapping::Mapping(static_cast<RenderStateObject::BlendOperation>(rs_obj.GetRenderState(RenderStateObject::RST_BlendOpAlpha))));
					break;

				case RenderStateObject::RST_SrcBlend:
				case RenderStateObject::RST_DestBlend:
				case RenderStateObject::RST_SrcBlendAlpha:
				case RenderStateObject::RST_DestBlendAlpha:
					glBlendFuncSeparate(OGLMapping::Mapping(static_cast<RenderStateObject::AlphaBlendFactor>(rs_obj.GetRenderState(RenderStateObject::RST_SrcBlend))),
						OGLMapping::Mapping(static_cast<RenderStateObject::AlphaBlendFactor>(rs_obj.GetRenderState(RenderStateObject::RST_DestBlend))),
						OGLMapping::Mapping(static_cast<RenderStateObject::AlphaBlendFactor>(rs_obj.GetRenderState(RenderStateObject::RST_SrcBlendAlpha))),
						OGLMapping::Mapping(static_cast<RenderStateObject::AlphaBlendFactor>(rs_obj.GetRenderState(RenderStateObject::RST_DestBlendAlpha))));
					break;

				case RenderStateObject::RST_DepthEnable:
					if (state)
					{
						glClearDepth(1.0f);
						glEnable(GL_DEPTH_TEST);
					}
					else
					{
						glDisable(GL_DEPTH_TEST);
					}
					break;

				case RenderStateObject::RST_DepthMask:
					glDepthMask(state ? GL_TRUE : GL_FALSE);
					break;

				case RenderStateObject::RST_DepthFunc:
					glDepthFunc(OGLMapping::Mapping(static_cast<RenderStateObject::CompareFunction>(state)));
					break;

				case RenderStateObject::RST_PolygonOffsetFactor:
				case RenderStateObject::RST_PolygonOffsetUnits:
					glEnable(GL_POLYGON_OFFSET_FILL);
					glEnable(GL_POLYGON_OFFSET_POINT);
					glEnable(GL_POLYGON_OFFSET_LINE);
					// Bias is in {0, 16}, scale the unit addition appropriately
					glPolygonOffset(uint32_to_float(rs_obj.GetRenderState(RenderStateObject::RST_PolygonOffsetFactor)),
						uint32_to_float(rs_obj.GetRenderState(RenderStateObject::RST_PolygonOffsetUnits)));
					break;

				case RenderStateObject::RST_FrontStencilFunc:
				case RenderStateObject::RST_FrontStencilRef:
				case RenderStateObject::RST_FrontStencilMask:
					glStencilFuncSeparate(GL_FRONT,
						OGLMapping::Mapping(static_cast<RenderStateObject::CompareFunction>(rs_obj.GetRenderState(RenderStateObject::RST_FrontStencilFunc))),
						rs_obj.GetRenderState(RenderStateObject::RST_FrontStencilRef), rs_obj.GetRenderState(RenderStateObject::RST_FrontStencilMask));
					break;

				case RenderStateObject::RST_FrontStencilFail:
				case RenderStateObject::RST_FrontStencilDepthFail:
				case RenderStateObject::RST_FrontStencilPass:
					glStencilOpSeparate(GL_FRONT,
						OGLMapping::Mapping(static_cast<RenderStateObject::StencilOperation>(rs_obj.GetRenderState(RenderStateObject::RST_FrontStencilFail))),
						OGLMapping::Mapping(static_cast<RenderStateObject::StencilOperation>(rs_obj.GetRenderState(RenderStateObject::RST_FrontStencilDepthFail))),
						OGLMapping::Mapping(static_cast<RenderStateObject::StencilOperation>(rs_obj.GetRenderState(RenderStateObject::RST_FrontStencilPass))));
					break;

				case RenderStateObject::RST_FrontStencilWriteMask:
					glStencilMaskSeparate(GL_FRONT, state);
					break;

				case RenderStateObject::RST_BackStencilFunc:
				case RenderStateObject::RST_BackStencilRef:
				case RenderStateObject::RST_BackStencilMask:
					glStencilFuncSeparate(GL_BACK,
						OGLMapping::Mapping(static_cast<RenderStateObject::CompareFunction>(rs_obj.GetRenderState(RenderStateObject::RST_BackStencilFunc))),
						rs_obj.GetRenderState(RenderStateObject::RST_BackStencilRef), rs_obj.GetRenderState(RenderStateObject::RST_BackStencilMask));
					break;

				case RenderStateObject::RST_BackStencilFail:
				case RenderStateObject::RST_BackStencilDepthFail:
				case RenderStateObject::RST_BackStencilPass:
					glStencilOpSeparate(GL_BACK,
						OGLMapping::Mapping(static_cast<RenderStateObject::StencilOperation>(rs_obj.GetRenderState(RenderStateObject::RST_BackStencilFail))),
						OGLMapping::Mapping(static_cast<RenderStateObject::StencilOperation>(rs_obj.GetRenderState(RenderStateObject::RST_BackStencilDepthFail))),
						OGLMapping::Mapping(static_cast<RenderStateObject::StencilOperation>(rs_obj.GetRenderState(RenderStateObject::RST_BackStencilPass))));
					break;

				case RenderStateObject::RST_BackStencilWriteMask:
					glStencilMaskSeparate(GL_BACK, state);
					break;

				case RenderStateObject::RST_FrontStencilEnable:
				case RenderStateObject::RST_BackStencilEnable:
					if (rs_obj.GetRenderState(RenderStateObject::RST_FrontStencilEnable)
						|| rs_obj.GetRenderState(RenderStateObject::RST_BackStencilEnable))
					{
						glEnable(GL_STENCIL_TEST);
					}
					else
					{
						glDisable(GL_STENCIL_TEST);
					}
					break;

				case RenderStateObject::RST_ScissorEnable:
					if (state)
					{
						glEnable(GL_SCISSOR_TEST);
					}
					else
					{
						glDisable(GL_SCISSOR_TEST);
					}
					break;

				case RenderStateObject::RST_ColorMask0:
				case RenderStateObject::RST_ColorMask1:
				case RenderStateObject::RST_ColorMask2:
				case RenderStateObject::RST_ColorMask3:
					glColorMask((state & RenderStateObject::CMASK_Red) != 0,
						(state & RenderStateObject::CMASK_Green) != 0,
						(state & RenderStateObject::CMASK_Blue) != 0,
						(state & RenderStateObject::CMASK_Alpha) != 0);
					break;
				}

				cur_render_state_obj_.SetRenderState(rst, state);
			}
		}

		OGLShaderObject const & ogl_shader_obj = *checked_cast<OGLShaderObject const *>(&shader_obj);

		cgGLBindProgram(ogl_shader_obj.VertexShader());
		cgGLEnableProfile(ogl_shader_obj.VertexShaderProfile());
		cgGLBindProgram(ogl_shader_obj.PixelShader());
		cgGLEnableProfile(ogl_shader_obj.PixelShaderProfile());

		for (int i = 0; i < ShaderObject::ST_NumShaderTypes; ++ i)
		{
			std::vector<SamplerPtr> const & samplers = ogl_shader_obj.Samplers(static_cast<ShaderObject::ShaderType>(i));

			for (uint32_t stage = 0; stage < samplers.size(); ++ stage)
			{
				glActiveTexture(GL_TEXTURE0 + stage);

				SamplerPtr cur_sampler = cur_samplers_[stage];
				SamplerPtr sampler = samplers[stage];
				if (!sampler || !sampler->GetTexture())
				{
					if (cur_sampler)
					{
						if (cur_sampler->GetTexture() != TexturePtr())
						{
							glBindTexture(GL_TEXTURE_2D, 0);
							cur_sampler->SetTexture(TexturePtr());
						}
					}
				}
				else
				{
					TexturePtr texture = sampler->GetTexture();
					OGLTexture& gl_tex = *checked_pointer_cast<OGLTexture>(texture);
					GLenum tex_type = gl_tex.GLType();

					glEnable(tex_type);

					if (cur_sampler->GetTexture() != sampler->GetTexture())
					{
						gl_tex.GLBindTexture();
						cur_sampler->SetTexture(sampler->GetTexture());
					}

					if (cur_sampler->AddressingMode(Sampler::TAT_Addr_U) != sampler->AddressingMode(Sampler::TAT_Addr_U))
					{
						glTexParameteri(tex_type, GL_TEXTURE_WRAP_S, OGLMapping::Mapping(sampler->AddressingMode(Sampler::TAT_Addr_U)));
						cur_sampler->AddressingMode(Sampler::TAT_Addr_U, sampler->AddressingMode(Sampler::TAT_Addr_U));
					}
					if (cur_sampler->AddressingMode(Sampler::TAT_Addr_V) != sampler->AddressingMode(Sampler::TAT_Addr_V))
					{
						glTexParameteri(tex_type, GL_TEXTURE_WRAP_T, OGLMapping::Mapping(sampler->AddressingMode(Sampler::TAT_Addr_V)));
						cur_sampler->AddressingMode(Sampler::TAT_Addr_V, sampler->AddressingMode(Sampler::TAT_Addr_V));
					}
					if (cur_sampler->AddressingMode(Sampler::TAT_Addr_W) != sampler->AddressingMode(Sampler::TAT_Addr_W))
					{
						glTexParameteri(tex_type, GL_TEXTURE_WRAP_R, OGLMapping::Mapping(sampler->AddressingMode(Sampler::TAT_Addr_W)));
						cur_sampler->AddressingMode(Sampler::TAT_Addr_W, sampler->AddressingMode(Sampler::TAT_Addr_W));
					}

					if (cur_sampler->BorderColor() != sampler->BorderColor())
					{
						glTexParameterfv(tex_type, GL_TEXTURE_BORDER_COLOR, &sampler->BorderColor().r());
						cur_sampler->BorderColor(sampler->BorderColor());
					}

					if (cur_sampler->Filtering() != sampler->Filtering())
					{
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

						cur_sampler->Filtering(sampler->Filtering());
					}

					if (cur_sampler->MaxMipLevel() != sampler->MaxMipLevel())
					{
						glTexParameteri(tex_type, GL_TEXTURE_MAX_LEVEL, sampler->MaxMipLevel());
						cur_sampler->MaxMipLevel(sampler->MaxMipLevel());
					}

					if (cur_sampler->MipMapLodBias() != sampler->MipMapLodBias())
					{
						GLfloat bias;
						glGetFloatv(GL_MAX_TEXTURE_LOD_BIAS, &bias);
						bias = std::min(sampler->MipMapLodBias(), bias);
						glTexEnvf(GL_TEXTURE_FILTER_CONTROL, GL_TEXTURE_LOD_BIAS, bias);
						cur_sampler->MipMapLodBias(sampler->MipMapLodBias());
					}
				}
			}
		}
	}

	// 设置当前渲染目标
	/////////////////////////////////////////////////////////////////////////////////
	void OGLRenderEngine::DoBindRenderTarget(RenderTargetPtr rt)
	{
		BOOST_ASSERT(rt);

		Viewport const & vp(rt->GetViewport());
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
		glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);

		uint32_t const num_instance = rl.NumInstance();
		BOOST_ASSERT(num_instance != 0);

		for (uint32_t instance = 0; instance < num_instance; ++ instance)
		{
			if (rl.InstanceStream())
			{
				GraphicsBuffer& stream = *rl.InstanceStream();

				uint32_t const instance_size = rl.InstanceSize();
				GraphicsBuffer::Mapper mapper(stream, BA_Read_Only);
				uint8_t const * buffer = mapper.Pointer<uint8_t>();

				uint32_t elem_offset = 0;
				for (uint32_t i = 0; i < rl.InstanceStreamFormat().size(); ++ i)
				{
					vertex_element const & vs_elem = rl.InstanceStreamFormat()[i];
					void const * addr = &buffer[instance * instance_size + elem_offset];
					GLfloat const * float_addr = static_cast<GLfloat const *>(addr);
					GLint const num_components = static_cast<GLint>(NumComponents(vs_elem.format));
					BOOST_ASSERT(IsFloatFormat(vs_elem.format));

					switch (vs_elem.usage)
					{
					case VEU_Position:
						switch (num_components)
						{
						case 2:
							glVertex2fv(float_addr);
							break;

						case 3:
							glVertex3fv(float_addr);
							break;

						case 4:
							glVertex4fv(float_addr);
							break;

						default:
							BOOST_ASSERT(false);
							break;
						}
						break;

					case VEU_Normal:
						switch (num_components)
						{
						case 3:
							glNormal3fv(float_addr);
							break;

						default:
							BOOST_ASSERT(false);
							break;
						}
						break;

					case VEU_Diffuse:
						switch (num_components)
						{
						case 3:
							glColor3fv(float_addr);
							break;

						case 4:
							glColor4fv(float_addr);
							break;

						default:
							BOOST_ASSERT(false);
							break;
						}
						break;

					case VEU_Specular:
						switch (num_components)
						{
						case 3:
							glSecondaryColor3fv(float_addr);
							break;

						default:
							BOOST_ASSERT(false);
							break;
						}
						break;

					case VEU_TextureCoord:
						{
							GLenum target = GL_TEXTURE0 + vs_elem.usage_index;
							glActiveTexture(target);

							switch (num_components)
							{
							case 1:
								glMultiTexCoord1fv(target, float_addr);
								break;

							case 2:
								glMultiTexCoord2fv(target, float_addr);
								break;

							case 3:
								glMultiTexCoord3fv(target, float_addr);
								break;

							case 4:
								glMultiTexCoord4fv(target, float_addr);
								break;

							default:
								BOOST_ASSERT(false);
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
				OGLGraphicsBuffer& stream(*checked_pointer_cast<OGLGraphicsBuffer>(rl.GetVertexStream(i)));
				uint32_t const size = rl.VertexSize(i);

				uint32_t elem_offset = 0;
				for (uint32_t j = 0; j < rl.VertexStreamFormat(i).size(); ++ j)
				{
					vertex_element const & vs_elem = rl.VertexStreamFormat(i)[j];
					GLvoid* offset = reinterpret_cast<GLvoid*>(elem_offset);
					GLint const num_components = static_cast<GLint>(NumComponents(vs_elem.format));
					GLenum const type = IsFloatFormat(vs_elem.format) ? GL_FLOAT : GL_UNSIGNED_BYTE;

					switch (vs_elem.usage)
					{
					case VEU_Position:
						glEnableClientState(GL_VERTEX_ARRAY);
						stream.Active();
						glVertexPointer(num_components, type, size, offset);
						break;

					case VEU_Normal:
						glEnableClientState(GL_NORMAL_ARRAY);
						stream.Active();
						glNormalPointer(type, size, offset);
						break;

					case VEU_Diffuse:
						glEnableClientState(GL_COLOR_ARRAY);
						stream.Active();
						glColorPointer(num_components, type, size, offset);
						break;

					case VEU_Specular:
						glEnableClientState(GL_SECONDARY_COLOR_ARRAY);
						stream.Active();
						glSecondaryColorPointer(num_components, type, size, offset);
						break;

					case VEU_TextureCoord:
						glClientActiveTexture(GL_TEXTURE0 + vs_elem.usage_index);
						glEnableClientState(GL_TEXTURE_COORD_ARRAY);
						stream.Active();
						glTexCoordPointer(num_components, type, size, offset);
						break;

					default:
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

			uint32_t num_passes = render_tech_->NumPasses();
			if (rl.UseIndices())
			{
				OGLGraphicsBuffer& stream(*checked_pointer_cast<OGLGraphicsBuffer>(rl.GetIndexStream()));
				stream.Active();

				GLenum index_type;
				if (EF_R16 == rl.IndexStreamFormat())
				{
					index_type = GL_UNSIGNED_SHORT;
				}
				else
				{
					index_type = GL_UNSIGNED_INT;
				}

				for (uint32_t i = 0; i < num_passes; ++ i)
				{
					RenderPassPtr pass = render_tech_->Pass(i);

					pass->Begin();

					this->AttachAttribs(rl, pass);

					glDrawElements(mode, static_cast<GLsizei>(rl.NumIndices()),
						index_type, 0);

					pass->End();
				}
			}
			else
			{
				for (uint32_t i = 0; i < num_passes; ++ i)
				{
					RenderPassPtr pass = render_tech_->Pass(i);

					pass->Begin();

					this->AttachAttribs(rl, pass);

					glDrawArrays(mode, 0, static_cast<GLsizei>(rl.NumVertices()));

					pass->End();
				}
			}
		}

		glPopClientAttrib();
	}

	// 结束一帧
	/////////////////////////////////////////////////////////////////////////////////
	void OGLRenderEngine::EndFrame()
	{
	}

	void OGLRenderEngine::AttachAttribs(RenderLayout const & rl, RenderPassPtr pass)
	{
		OGLShaderObjectPtr ogl_shader_object = checked_pointer_cast<OGLShaderObject>(pass->GetShaderObject());

		// Geometry streams
		for (uint32_t i = 0; i < rl.NumVertexStreams(); ++ i)
		{
			OGLGraphicsBuffer& stream(*checked_pointer_cast<OGLGraphicsBuffer>(rl.GetVertexStream(i)));
			uint32_t const size = rl.VertexSize(i);

			uint32_t elem_offset = 0;
			for (uint32_t j = 0; j < rl.VertexStreamFormat(i).size(); ++ j)
			{
				vertex_element const & vs_elem = rl.VertexStreamFormat(i)[j];
				int32_t const index = ogl_shader_object->AttribIndex(vs_elem.usage, vs_elem.usage_index);
				if (index >= 0)
				{
					GLvoid* offset = reinterpret_cast<GLvoid*>(elem_offset);
					GLint const num_components = static_cast<GLint>(NumComponents(vs_elem.format));
					GLenum const type = IsFloatFormat(vs_elem.format) ? GL_FLOAT : GL_UNSIGNED_BYTE;

					glEnableVertexAttribArray(index);
					stream.Active();
					glVertexAttribPointer(index, num_components, type, GL_FALSE, size, offset);
				}

				elem_offset += vs_elem.element_size();
			}
		}
	}

	// 设置模板位数
	/////////////////////////////////////////////////////////////////////////////////
	uint16_t OGLRenderEngine::StencilBufferBitDepth()
	{
		return 8;
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

		caps_.hw_instancing_support = true;

		for (size_t i = 0; i < caps_.max_textures_units; ++ i)
		{
			cur_samplers_.push_back(SamplerPtr(new Sampler));
		};
	}
}
