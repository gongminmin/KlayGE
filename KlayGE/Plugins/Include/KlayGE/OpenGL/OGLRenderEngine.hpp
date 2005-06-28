// OGLRenderEngine.hpp
// KlayGE OpenGL渲染引擎类 头文件
// Ver 2.7.0
// 版权所有(C) 龚敏敏, 2003-2005
// Homepage: http://klayge.sourceforge.net
//
// 2.7.0
// 去掉了TextureCoordSet和DisableTextureStage (2005.6.26)
// TextureAddressingMode, extureFiltering和TextureAnisotropy移到Texture中 (2005.6.27)
//
// 2.4.0
// 增加了PolygonMode (2005.3.20)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#ifndef _OGLRENDERENGINE_HPP
#define _OGLRENDERENGINE_HPP

#include <vector>
#include <map>

#define NOMINMAX
#include <windows.h>
#include <glloader/glloader.h>

#include <KlayGE/RenderEngine.hpp>

#ifdef KLAYGE_DEBUG
	#pragma comment(lib, "KlayGE_RenderEngine_OpenGL_d.lib")
#else
	#pragma comment(lib, "KlayGE_RenderEngine_OpenGL.lib")
#endif

namespace KlayGE
{
	class OGLRenderEngine : public RenderEngine
	{
	public:
		OGLRenderEngine();
		~OGLRenderEngine();

		std::wstring const & Name() const;

		void ClearColor(Color const & clr);

		void ShadingType(ShadeOptions so);

		void EnableLighting(bool enabled);
		void AmbientLight(Color const & col);

		RenderWindowPtr CreateRenderWindow(std::string const & name, RenderSettings const & settings);

		void CullingMode(CullMode mode);
		void PolygonMode(FillMode mode);

		void SetMaterial(Material const & mat);

		void SetLight(uint32_t index, Light const & lt);
		void LightEnable(uint32_t index, bool enable);

		void ActiveRenderTarget(RenderTargetListIterator iter);

		void StartRendering();

		void BeginFrame();
		void Render(VertexBuffer const & vb);
		void EndFrame();

		void DepthBufferDepthTest(bool depthTest);
		void DepthBufferDepthWrite(bool depthWrite);
		void DepthBufferFunction(CompareFunction depthFunction);
		void DepthBias(uint16_t bias);

		void Fog(FogMode mode = Fog_None,
			Color const & color = Color(1, 1, 1, 1),
			float expDensity = 1, float linearStart = 0, float linearEnd = 1);

		void SetTexture(uint32_t stage, TexturePtr const & texture);

		uint32_t MaxTextureStages();

		void TextureCoordCalculation(uint32_t stage, TexCoordCalcMethod m);
		void TextureMatrix(uint32_t stage, Matrix4 const & mat);

		void StencilCheckEnabled(bool enabled);
		bool HasHardwareStencil();

		uint16_t StencilBufferBitDepth();

		void StencilBufferFunction(CompareFunction func);
		void StencilBufferReferenceValue(uint32_t refValue);
		void StencilBufferMask(uint32_t mask);
		void StencilBufferFailOperation(StencilOperation op);
		void StencilBufferDepthFailOperation(StencilOperation op);
		void StencilBufferPassOperation(StencilOperation op);

	private:
		void DoWorldMatrix();
		void DoViewMatrix();
		void DoProjectionMatrix();

	private:
		CullMode cullingMode_;

	private:
		glActiveTextureFUNC glActiveTexture_;
		glClientActiveTextureFUNC glClientActiveTexture_;
	};

	typedef boost::shared_ptr<OGLRenderEngine> OGLRenderEnginePtr;
}

#endif			// _OGLRENDERENGINE_HPP
