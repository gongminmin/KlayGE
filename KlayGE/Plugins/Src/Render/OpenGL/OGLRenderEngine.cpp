// OGLRenderEngine.cpp
// KlayGE OpenGL渲染引擎类 实现文件
// Ver 2.7.0
// 版权所有(C) 龚敏敏, 2004-2005
// Homepage: http://klayge.sourceforge.net
//
// 2.7.0
// 支持vertex_buffer_object (2005.6.19)
// 支持OpenGL 1.3多纹理 (2005.6.26)
// 去掉了TextureCoordSet和DisableTextureStage (2005.6.26)
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
#include <KlayGE/Light.hpp>
#include <KlayGE/Material.hpp>
#include <KlayGE/Viewport.hpp>
#include <KlayGE/VertexBuffer.hpp>
#include <KlayGE/RenderTarget.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/OpenGL/OGLRenderSettings.hpp>
#include <KlayGE/OpenGL/OGLRenderWindow.hpp>
#include <KlayGE/OpenGL/OGLTexture.hpp>
#include <KlayGE/OpenGL/OGLVertexStream.hpp>
#include <KlayGE/OpenGL/OGLIndexStream.hpp>

#include <glloader/glloader.h>

#include <cassert>
#include <algorithm>
#include <cstring>

#include <KlayGE/OpenGL/OGLRenderEngine.hpp>

#ifdef KLAYGE_DEBUG
	#pragma comment(lib, "glloader_d.lib")
#else
	#pragma comment(lib, "glloader.lib")
#endif
#pragma comment(lib, "glu32.lib")

namespace KlayGE
{
	// 从RenderEngine::CompareFunction转换到D3DCMPFUNC
	/////////////////////////////////////////////////////////////////////////////////
	GLint Convert(RenderEngine::CompareFunction func)
	{
		GLint ret = GL_NEVER;

		switch (func)
		{
		case RenderEngine::CF_AlwaysFail:
			ret = GL_NEVER;
			break;

		case RenderEngine::CF_AlwaysPass:
			ret = GL_ALWAYS;
			break;

		case RenderEngine::CF_Less:
			ret = GL_LESS;
			break;

		case RenderEngine::CF_LessEqual:
			ret = GL_LEQUAL;
			break;

		case RenderEngine::CF_Equal:
			ret = GL_EQUAL;
			break;

		case RenderEngine::CF_NotEqual:
			ret = GL_NOTEQUAL;
			break;

		case RenderEngine::CF_GreaterEqual:
			ret = GL_GEQUAL;
			break;

		case RenderEngine::CF_Greater:
			ret = GL_GREATER;
			break;
		};

		return ret;
	}

	// 从RenderEngine::StencilOperation转换到D3DSTENCILOP
	/////////////////////////////////////////////////////////////////////////////////
	GLint Convert(RenderEngine::StencilOperation op)
	{
		GLint ret = GL_KEEP;

		switch (op)
		{
		case RenderEngine::SOP_Keep:
			ret = GL_KEEP;
			break;

		case RenderEngine::SOP_Zero:
			ret = GL_ZERO;
			break;

		case RenderEngine::SOP_Replace:
			ret = GL_REPLACE;
			break;

		case RenderEngine::SOP_Increment:
			ret = GL_INCR;
			break;

		case RenderEngine::SOP_Decrement:
			ret = GL_DECR;
			break;

		case RenderEngine::SOP_Invert:
			ret = GL_INVERT;
			break;
		};

		return ret;
	}
}

namespace KlayGE
{
	// 构造函数
	/////////////////////////////////////////////////////////////////////////////////
	OGLRenderEngine::OGLRenderEngine()
		: cullingMode_(RenderEngine::CM_None)
	{
		if (glloader_is_supported("GL_VERSION_1_3"))
		{
			glActiveTexture_ = glActiveTexture;
			glClientActiveTexture_ = glClientActiveTexture;
		}
		else
		{
			if (glloader_is_supported("GL_ARB_multitexture"))
			{
				glActiveTexture_ = glActiveTextureARB;
				glClientActiveTexture_ = glClientActiveTextureARB;
			}
			else
			{
				glActiveTexture_ = NULL;
				glClientActiveTexture_ = NULL;
			}
		}
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
		static const std::wstring name(L"OpenGL Render System");
		return name;
	}

	// 开始渲染
	/////////////////////////////////////////////////////////////////////////////////
	void OGLRenderEngine::StartRendering()
	{
		bool gotMsg;
		MSG  msg;

		::PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE);

		while (WM_QUIT != msg.message)
		{
			// 如果窗口是激活的，用 PeekMessage()以便我们可以用空闲时间渲染场景
			// 不然, 用 GetMessage() 减少 CPU 占用率
			if ((*RenderEngine::ActiveRenderTarget())->Active())
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
				if ((*RenderEngine::ActiveRenderTarget())->Active())
				{
					(*RenderEngine::ActiveRenderTarget())->Update();
				}
			}
		}
	}

	// 设置环境光
	/////////////////////////////////////////////////////////////////////////////////
	void OGLRenderEngine::AmbientLight(Color const & col)
	{
		GLfloat ambient[] = { col.r(), col.g(), col.b(), 1.0f };
		glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);
	}

	// 设置清除颜色
	/////////////////////////////////////////////////////////////////////////////////
	void OGLRenderEngine::ClearColor(Color const & col)
	{
		glClearColor(col.r(), col.g(), col.b(), col.a());
	}

	// 设置光影类型
	/////////////////////////////////////////////////////////////////////////////////
	void OGLRenderEngine::ShadingType(ShadeOptions so)
	{
		GLenum shadeMode = GL_FLAT;

		switch (so)
		{
		case SO_Flat:
			shadeMode = GL_FLAT;
			break;

		case SO_Gouraud:
			shadeMode = GL_SMOOTH;
			break;

		case SO_Phong:
			shadeMode = GL_SMOOTH;
			break;
		}

		glShadeModel(shadeMode);
	}

	// 打开/关闭光源
	/////////////////////////////////////////////////////////////////////////////////
	void OGLRenderEngine::EnableLighting(bool enabled)
	{
		if (enabled)
		{
			glEnable(GL_LIGHTING);
		}
		else
		{
			glDisable(GL_LIGHTING);
		}
	}

	// 建立渲染窗口
	/////////////////////////////////////////////////////////////////////////////////
	RenderWindowPtr OGLRenderEngine::CreateRenderWindow(std::string const & name,
		RenderSettings const & settings)
	{
		RenderWindowPtr win(new OGLRenderWindow(name,
			static_cast<OGLRenderSettings const &>(settings)));

		this->ActiveRenderTarget(this->AddRenderTarget(win));

		this->DepthBufferDepthTest(settings.depthBuffer);
		this->DepthBufferDepthWrite(settings.depthBuffer);

		this->SetMaterial(Material(Color(1, 1, 1, 1)));

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
		GLenum oglMode = GL_FILL;

		switch (mode)
		{
		case FM_Point:
			oglMode = GL_POINT;
			break;

		case FM_Line:
			oglMode = GL_LINE;
			break;

		case FM_Fill:
			oglMode = GL_FILL;
			break;

		default:
			assert(false);
			break;
		}

		glPolygonMode(GL_FRONT_AND_BACK, oglMode);
	}

	// 设置光源
	/////////////////////////////////////////////////////////////////////////////////
	void OGLRenderEngine::SetLight(uint32_t index, Light const & light)
	{
		GLint lightIndex(GL_LIGHT0 + index);

		switch (light.lightType)
		{
		case Light::LT_Spot:
			glLightf(lightIndex, GL_SPOT_CUTOFF, light.spotOuter);
			break;

		default:
			glLightf(lightIndex, GL_SPOT_CUTOFF, 180.0f);
			break;
		}

		GLfloat ambient[4] = { light.ambient.r(), light.ambient.g(), light.ambient.b(), light.ambient.a() };
		glLightfv(lightIndex, GL_AMBIENT, ambient);

		GLfloat diffuse[4] = { light.diffuse.r(), light.diffuse.g(), light.diffuse.b(), light.diffuse.a() };
		glLightfv(lightIndex, GL_DIFFUSE, diffuse);

		GLfloat specular[4] = { light.specular.r(), light.specular.g(), light.specular.b(), light.specular.a() };
		glLightfv(lightIndex, GL_SPECULAR, specular);

		// Set position / direction
		GLfloat f4vals[4];
		switch (light.lightType)
		{
		case Light::LT_Point:
			f4vals[0] = light.position.x();
			f4vals[1] = light.position.y();
			f4vals[2] = light.position.z();
			f4vals[3] = 1.0f;
			glLightfv(lightIndex, GL_POSITION, f4vals);
			break;

		case Light::LT_Directional:
			f4vals[0] = -light.direction.x(); // GL light directions are in eye coords
			f4vals[1] = -light.direction.y();
			f4vals[2] = -light.direction.z(); // GL light directions are in eye coords
			f4vals[3] = 0.0f; // important!
			// In GL you set direction through position, but the
			//  w value of the vector being 0 indicates which it is
			glLightfv(lightIndex, GL_POSITION, f4vals);
			break;

		case Light::LT_Spot:
			f4vals[0] = light.position.x();
			f4vals[1] = light.position.y();
			f4vals[2] = light.position.z();
			f4vals[3] = 1.0f;
			glLightfv(lightIndex, GL_POSITION, f4vals);

			f4vals[0] = light.direction.x();
			f4vals[1] = light.direction.y();
			f4vals[2] = light.direction.z();
			f4vals[3] = 0.0f; 
			glLightfv(lightIndex, GL_SPOT_DIRECTION, f4vals);
		}

		glLightf(lightIndex, GL_CONSTANT_ATTENUATION, light.attenuationConst);
		glLightf(lightIndex, GL_LINEAR_ATTENUATION, light.attenuationLinear);
		glLightf(lightIndex, GL_QUADRATIC_ATTENUATION, light.attenuationQuad);
	}

	// 打开/关闭某个光源
	/////////////////////////////////////////////////////////////////////////////////
	void OGLRenderEngine::LightEnable(uint32_t index, bool enabled)
	{
		if (enabled)
		{
			glEnable(GL_LIGHT0 + index);
		}
		else
		{
			glDisable(GL_LIGHT0 + index);
		}
	}

	// 实现设置世界矩阵
	/////////////////////////////////////////////////////////////////////////////////
	void OGLRenderEngine::DoWorldMatrix()
	{
		glMatrixMode(GL_MODELVIEW);

		Matrix4 oglViewMat(viewMat_);

		oglViewMat(0, 2) = -oglViewMat(0, 2);
		oglViewMat(1, 2) = -oglViewMat(1, 2);
		oglViewMat(2, 2) = -oglViewMat(2, 2);
		oglViewMat(3, 2) = -oglViewMat(3, 2);

		Matrix4 const oglMat(worldMat_ * oglViewMat);
		glLoadMatrixf(&oglMat(0, 0));
	}

	// 设置观察矩阵
	/////////////////////////////////////////////////////////////////////////////////
	void OGLRenderEngine::DoViewMatrix()
	{
		this->DoWorldMatrix();
	}

	// 实现设置投射矩阵
	/////////////////////////////////////////////////////////////////////////////////
	void OGLRenderEngine::DoProjectionMatrix()
	{
		Matrix4 oglMat(MathLib::LHToRH(projMat_));

		glMatrixMode(GL_PROJECTION);
		glLoadMatrixf(&oglMat(0, 0));
		glMatrixMode(GL_MODELVIEW);
	}

	// 设置材质
	/////////////////////////////////////////////////////////////////////////////////
	void OGLRenderEngine::SetMaterial(Material const & material)
	{
		GLfloat ambient[4] = { material.ambient.r(), material.ambient.g(), material.ambient.b(), material.ambient.a() };
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);

		GLfloat diffuse[4] = { material.diffuse.r(), material.diffuse.g(), material.diffuse.b(), material.diffuse.a() };
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);

		GLfloat specular[4] = { material.specular.r(), material.specular.g(), material.specular.b(), material.specular.a() };
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);

		glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, material.shininess);
	}

	// 设置当前渲染目标，该渲染目标必须已经在列表中
	/////////////////////////////////////////////////////////////////////////////////
	void OGLRenderEngine::ActiveRenderTarget(RenderTargetListIterator iter)
	{
		RenderEngine::ActiveRenderTarget(iter);

		this->CullingMode(cullingMode_);

		Viewport const & vp((*iter)->GetViewport());
		glViewport(vp.left, vp.top, vp.width, vp.height);
	}

	// 开始一帧
	/////////////////////////////////////////////////////////////////////////////////
	void OGLRenderEngine::BeginFrame()
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	// 渲染
	/////////////////////////////////////////////////////////////////////////////////
	void OGLRenderEngine::Render(VertexBuffer const & vb)
	{
		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_NORMAL_ARRAY);
		glDisableClientState(GL_COLOR_ARRAY);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);

		bool use_vbo = false;
		for (VertexBuffer::VertexStreamConstIterator iter = vb.VertexStreamBegin();
			iter != vb.VertexStreamEnd(); ++ iter)
		{
			OGLVertexStream& stream(static_cast<OGLVertexStream&>(*(*iter)));
			if (stream.UseVBO())
			{
				use_vbo = true;
				break;
			}
		}

		for (VertexBuffer::VertexStreamConstIterator iter = vb.VertexStreamBegin();
			iter != vb.VertexStreamEnd(); ++ iter)
		{
			OGLVertexStream& stream(static_cast<OGLVertexStream&>(*(*iter)));
			VertexStreamType type(stream.Type());

			std::vector<uint8_t> const & data(stream.OGLBuffer());

			switch (type)
			{
			// Vertex xyzs
			case VST_Positions:
				glEnableClientState(GL_VERTEX_ARRAY);
				if (use_vbo)
				{
					stream.Active();
					glVertexPointer(3, GL_FLOAT, 0, NULL);
				}
				else
				{
					glVertexPointer(3, GL_FLOAT, 0, &data[0]);
				}
				break;
		
			case VST_Normals:
				glEnableClientState(GL_NORMAL_ARRAY);
				if (use_vbo)
				{
					stream.Active();
					glNormalPointer(GL_FLOAT, 0, NULL);
				}
				else
				{
					glNormalPointer(GL_FLOAT, 0, &data[0]);
				}
				break;

			case VST_Diffuses:
				glEnableClientState(GL_COLOR_ARRAY);
				if (use_vbo)
				{
					stream.Active();
					glColorPointer(4, GL_UNSIGNED_BYTE, 0, NULL);
				}
				else
				{
					glColorPointer(4, GL_UNSIGNED_BYTE, 0, &data[0]);
				}
				break;

			case VST_TextureCoords0:
			case VST_TextureCoords1:
			case VST_TextureCoords2:
			case VST_TextureCoords3:
			case VST_TextureCoords4:
			case VST_TextureCoords5:
			case VST_TextureCoords6:
			case VST_TextureCoords7:
				if (glClientActiveTexture_ != NULL)
				{
					glClientActiveTexture_(GL_TEXTURE0 + type - VST_TextureCoords0);
				}
				glEnableClientState(GL_TEXTURE_COORD_ARRAY);
				if (use_vbo)
				{
					stream.Active();
					glTexCoordPointer(static_cast<GLint>(stream.ElementsPerVertex()),
						GL_FLOAT, 0, NULL);
				}
				else
				{
					glTexCoordPointer(static_cast<GLint>(stream.ElementsPerVertex()),
						GL_FLOAT, 0, &data[0]);
				}
				break;
			}
		}

		size_t const vertexCount = vb.UseIndices() ? vb.NumIndices() : vb.NumVertices();
		GLenum mode = GL_POINTS;
		size_t primCount = vertexCount;
		switch (vb.Type())
		{
		case VertexBuffer::BT_PointList:
			mode = GL_POINTS;
			primCount = vertexCount;
			break;

		case VertexBuffer::BT_LineList:
			mode = GL_LINES;
			primCount = vertexCount / 2;
			break;

		case VertexBuffer::BT_LineStrip:
			mode = GL_LINE_STRIP;
			primCount = vertexCount - 1;
			break;

		case VertexBuffer::BT_TriangleList:
			mode = GL_TRIANGLES;
			primCount = vertexCount / 3;
			break;

		case VertexBuffer::BT_TriangleStrip:
			mode = GL_TRIANGLE_STRIP;
			primCount = vertexCount - 2;
			break;

		case VertexBuffer::BT_TriangleFan:
			mode = GL_TRIANGLE_FAN;
			primCount = vertexCount - 2;
			break;

		default:
			assert(false);
			break;
		}

		numPrimitivesJustRendered_ += primCount;
		numVerticesJustRendered_ += vertexCount;

		if (vb.UseIndices())
		{
			OGLIndexStream& stream(static_cast<OGLIndexStream&>(*vb.GetIndexStream()));

			if (use_vbo)
			{
				stream.Active();
				glDrawElements(mode, static_cast<GLsizei>(vb.NumIndices()),
					GL_UNSIGNED_SHORT, 0);
			}
			else
			{
				std::vector<uint16_t> const & data(stream.OGLBuffer());
				glDrawElements(mode, static_cast<GLsizei>(vb.NumIndices()),
					GL_UNSIGNED_SHORT, &data[0]);
			}
		}
		else
		{
			glDrawArrays(mode, 0, static_cast<GLsizei>(vb.NumVertices()));
		}
	}

	// 结束一帧
	/////////////////////////////////////////////////////////////////////////////////
	void OGLRenderEngine::EndFrame()
	{
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
		glDepthFunc(Convert(func));
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

	// 设置雾化效果
	/////////////////////////////////////////////////////////////////////////////////
	void OGLRenderEngine::Fog(FogMode mode, Color const & color,
		float expDensity, float linearStart, float linearEnd)
	{
		GLint fogMode;
		switch (mode)
		{
		case Fog_Exp:
			fogMode = GL_EXP;
			break;

		case Fog_Exp2:
			fogMode = GL_EXP2;
			break;

		case Fog_Linear:
			fogMode = GL_LINEAR;
			break;

		default:
			// Give up on it
			glDisable(GL_FOG);
			return;
		}

		glEnable(GL_FOG);
		glFogi(GL_FOG_MODE, fogMode);
		GLfloat fogColor[4] = { color.r(), color.g(), color.b(), color.a() };
		glFogfv(GL_FOG_COLOR, fogColor);
		glFogf(GL_FOG_DENSITY, expDensity);
		glFogf(GL_FOG_START, linearStart);
		glFogf(GL_FOG_END, linearEnd);
	}

	// 设置纹理
	/////////////////////////////////////////////////////////////////////////////////
	void OGLRenderEngine::SetTexture(uint32_t stage, TexturePtr const & texture)
	{
		if (glActiveTexture_ != NULL)
		{
			glActiveTexture_(GL_TEXTURE0 + stage);
		}

		if (!texture)
		{
			if (tex_stage_type_.find(stage) != tex_stage_type_.end())
			{
				glDisable(tex_stage_type_[stage]);
				tex_stage_type_.erase(stage);
			}
		}
		else
		{
			switch (texture->Type())
			{
			case Texture::TT_1D:
				tex_stage_type_[stage] = GL_TEXTURE_1D;
				break;

			case Texture::TT_2D:
				tex_stage_type_[stage] = GL_TEXTURE_2D;
				break;

			case Texture::TT_3D:
				tex_stage_type_[stage] = GL_TEXTURE_3D;
				break;

			case Texture::TT_Cube:
				tex_stage_type_[stage] = GL_TEXTURE_CUBE_MAP;
				break;
			}

			glEnable(tex_stage_type_[stage]);
			glBindTexture(tex_stage_type_[stage], OGLTexturePtr(static_cast<OGLTexture*>(texture.get()))->GLTexture());
		}
	}

	// 获取最大纹理阶段数
	/////////////////////////////////////////////////////////////////////////////////
	uint32_t OGLRenderEngine::MaxTextureStages()
	{
		GLint ret;
		glGetIntegerv(GL_MAX_TEXTURE_UNITS, &ret);
		return static_cast<uint32_t>(ret);
	}

	// 计算纹理坐标
	/////////////////////////////////////////////////////////////////////////////////
	void OGLRenderEngine::TextureCoordCalculation(uint32_t /*stage*/, TexCoordCalcMethod /*m*/)
	{
	}

	// 设置纹理寻址模式
	/////////////////////////////////////////////////////////////////////////////////
	void OGLRenderEngine::TextureAddressingMode(uint32_t stage, TexAddressingMode tam)
	{
		if (glActiveTexture_ != NULL)
		{
			glActiveTexture_(GL_TEXTURE0 + stage);
		}

		GLint mode;
		switch (tam)
		{
		case TAM_Wrap:
			mode = GL_REPEAT;
			break;

		case TAM_Mirror:
			mode = GL_MIRRORED_REPEAT;
			break;

		case TAM_Clamp:
			mode = GL_CLAMP;
			break;

		default:
			assert(false);
			break;
		}

		glTexParameteri(tex_stage_type_[stage], GL_TEXTURE_WRAP_S, mode);
		glTexParameteri(tex_stage_type_[stage], GL_TEXTURE_WRAP_T, mode);
		glTexParameteri(tex_stage_type_[stage], GL_TEXTURE_WRAP_R, mode);
	}

	// 设置纹理坐标
	/////////////////////////////////////////////////////////////////////////////////
	void OGLRenderEngine::TextureMatrix(uint32_t stage, Matrix4 const & mat)
	{
		if (glActiveTexture_ != NULL)
		{
			glActiveTexture_(GL_TEXTURE0 + stage);
		}

		Matrix4 oglMat(MathLib::LHToRH(projMat_));

		glMatrixMode(GL_TEXTURE);
		glLoadMatrixf(&oglMat(0, 0));
		glMatrixMode(GL_MODELVIEW);
	}

	// 设置纹理过滤模式
	/////////////////////////////////////////////////////////////////////////////////
	void OGLRenderEngine::TextureFiltering(uint32_t stage, TexFilterType type, TexFilterOp op)
	{
		if (glActiveTexture_ != NULL)
		{
			glActiveTexture_(GL_TEXTURE0 + stage);
		}

		GLint mode;
		if (type != TFT_Min)
		{
			switch (op)
			{
			case TFO_None:
			case TFO_Point:
				mode = GL_NEAREST;
				break;

			case TFO_Bilinear:
			case TFO_Trilinear:
			case TFO_Anisotropic:
				mode = GL_LINEAR;
				break;

			default:
				assert(false);
				break;
			}
		}
		else
		{
			switch (op)
			{
			case TFO_None:
			case TFO_Point:
				switch (tex_stage_mip_filter_[stage])
				{
				case TFO_None:
					// nearest min, no mip
					mode = GL_NEAREST;
					break;

				case TFO_Point:
					// nearest min, nearest mip
					mode = GL_NEAREST_MIPMAP_NEAREST;
					break;

				case TFO_Bilinear:
				case TFO_Trilinear:
				case TFO_Anisotropic:
					// nearest min, linear mip
					mode = GL_NEAREST_MIPMAP_LINEAR;
					break;
				}
				break;

			case TFO_Bilinear:
			case TFO_Trilinear:
			case TFO_Anisotropic:
				switch (tex_stage_mip_filter_[stage])
				{
				case TFO_None:
					// linear min, no mip
					mode = GL_LINEAR;
					break;

				case TFO_Point:
					// linear min, point mip
					mode = GL_LINEAR_MIPMAP_NEAREST;
					break;

				case TFO_Bilinear:
				case TFO_Trilinear:
				case TFO_Anisotropic:
					// linear min, linear mip
					mode = GL_LINEAR_MIPMAP_LINEAR;
					break;
				}
				break;

			default:
				assert(false);
				break;
			}
		}

		switch (type)
		{
		case TFT_Min:
			glTexParameteri(tex_stage_type_[stage], GL_TEXTURE_MIN_FILTER, mode);
			break;

		case TFT_Mag:
			glTexParameteri(tex_stage_type_[stage], GL_TEXTURE_MAG_FILTER, mode);
			break;

		case TFT_Mip:
			tex_stage_mip_filter_[stage] = op;
			glTexParameteri(tex_stage_type_[stage], GL_TEXTURE_MIN_FILTER, mode);
			break;

		default:
			assert(false);
			break;
		}
	}

	// 设置纹理异性过滤
	/////////////////////////////////////////////////////////////////////////////////
	void OGLRenderEngine::TextureAnisotropy(uint32_t /*stage*/, uint32_t /*maxAnisotropy*/)
	{
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

	// 设置模板比较函数
	/////////////////////////////////////////////////////////////////////////////////
	void OGLRenderEngine::StencilBufferFunction(CompareFunction /*func*/)
	{
	}

	// 设置模板缓冲区参考值
	/////////////////////////////////////////////////////////////////////////////////
	void OGLRenderEngine::StencilBufferReferenceValue(uint32_t /*refValue*/)
	{
	}

	// 设置模板缓冲区掩码
	/////////////////////////////////////////////////////////////////////////////////
	void OGLRenderEngine::StencilBufferMask(uint32_t /*mask*/)
	{
	}

	// 设置模板缓冲区测试失败后的操作
	/////////////////////////////////////////////////////////////////////////////////
	void OGLRenderEngine::StencilBufferFailOperation(StencilOperation /*op*/)
	{
	}

	// 设置模板缓冲区深度测试失败后的操作
	/////////////////////////////////////////////////////////////////////////////////
	void OGLRenderEngine::StencilBufferDepthFailOperation(StencilOperation /*op*/)
	{
	}

	// 设置模板缓冲区通过后的操作
	/////////////////////////////////////////////////////////////////////////////////
	void OGLRenderEngine::StencilBufferPassOperation(StencilOperation /*op*/)
	{
	}
}
