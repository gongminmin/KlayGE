// OGLRenderEngine.hpp
// KlayGE OpenGL渲染引擎类 头文件
// Ver 2.8.0
// 版权所有(C) 龚敏敏, 2003-2005
// Homepage: http://klayge.sourceforge.net
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

		void StartRendering();

		void BeginFrame();
		void EndFrame();

		void AlphaBlend(bool enabled);
		void AlphaBlendFunction(AlphaBlendFactor src_factor, AlphaBlendFactor dst_factor);

		void DepthBufferDepthTest(bool enabled);
		void DepthBufferDepthWrite(bool enabled);
		void DepthBufferFunction(CompareFunction depthFunction);
		void DepthBias(uint16_t bias);

		void AlphaTest(bool enabled);
		void AlphaFunction(CompareFunction alphaFunction, float refValue);

		void Fog(FogMode mode = Fog_None,
			Color const & color = Color(1, 1, 1, 1),
			float expDensity = 1, float linearStart = 0, float linearEnd = 1);

		void SetSampler(uint32_t stage, SamplerPtr const & sampler);
		void DisableSampler(uint32_t stage);

		void StencilCheckEnabled(bool enabled);
		bool HasHardwareStencil();

		uint16_t StencilBufferBitDepth();

		void StencilBufferFunction(CompareFunction func, uint32_t refValue, uint32_t mask);
		void StencilBufferOperation(StencilOperation fail, StencilOperation depth_fail, StencilOperation pass);

	private:
		void DoActiveRenderTarget(uint32_t n, RenderTargetPtr renderTarget);

		void DoWorldMatrix();
		void DoViewMatrix();
		void DoProjectionMatrix();

		void DoRender(VertexBuffer const & vb);

		void FillRenderDeviceCaps();

	private:
		CullMode cullingMode_;

	private:
		glActiveTextureFUNC glActiveTexture_;
		glClientActiveTextureFUNC glClientActiveTexture_;
	};

	typedef boost::shared_ptr<OGLRenderEngine> OGLRenderEnginePtr;
}

#endif			// _OGLRENDERENGINE_HPP
