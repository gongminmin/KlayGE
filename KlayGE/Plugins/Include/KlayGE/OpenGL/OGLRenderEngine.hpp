#ifndef _OGLRENDERENGINE_HPP
#define _OGLRENDERENGINE_HPP

#include <vector>

#define NOMINMAX
#include <windows.h>
#include <gl/gl.h>

#include <KlayGE/RenderEngine.hpp>

#pragma comment(lib, "KlayGE_RenderEngine_OpenGL.lib")

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

		void SetMaterial(Material const & mat);

		void SetLight(uint32 index, Light const & lt);
		void LightEnable(uint32 index, bool enable);

		void ActiveRenderTarget(RenderTargetListIterator iter);

		void StartRendering();

		void BeginFrame();
		void Render(RenderBuffer const & vb);
		void EndFrame();

		void DepthBufferDepthTest(bool depthTest);
		void DepthBufferDepthWrite(bool depthWrite);
		void DepthBufferFunction(CompareFunction depthFunction);
		void DepthBias(uint16 bias);

		void Fog(FogMode mode = Fog_None,
			Color const & color = Color(1, 1, 1, 1),
			float expDensity = 1, float linearStart = 0, float linearEnd = 1);

		void SetTexture(uint32 stage, TexturePtr const & texture);

		void TextureCoordSet(uint32 stage, int index);

		uint32 MaxTextureStages();
		void DisableTextureStage(uint32 stage);

		void TextureCoordCalculation(uint32 stage, TexCoordCalcMethod m);
		void TextureAddressingMode(uint32 stage, TexAddressingMode tam);
		void TextureMatrix(uint32 stage, Matrix4 const & mat);
		void TextureFiltering(uint32 stage, TexFiltering texFiltering);
		void TextureAnisotropy(uint32 stage, uint32 maxAnisotropy);

		void StencilCheckEnabled(bool enabled);
		bool HasHardwareStencil();

		uint16 StencilBufferBitDepth();

		void StencilBufferFunction(CompareFunction func);
		void StencilBufferReferenceValue(uint32 refValue);
		void StencilBufferMask(uint32 mask);
		void StencilBufferFailOperation(StencilOperation op);
		void StencilBufferDepthFailOperation(StencilOperation op);
		void StencilBufferPassOperation(StencilOperation op);

	private:
		void DoWorldMatrix();
		void DoViewMatrix();
		void DoProjectionMatrix();

	private:
		CullMode cullingMode_;
	};

	typedef boost::shared_ptr<OGLRenderEngine> OGLRenderEnginePtr;
}

#endif			// _OGLRENDERENGINE_HPP
