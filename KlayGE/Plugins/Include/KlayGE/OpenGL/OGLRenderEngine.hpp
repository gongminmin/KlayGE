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

		void SetLight(U32 index, Light const & lt);
		void LightEnable(U32 index, bool enable);

		void ActiveRenderTarget(RenderTargetListIterator iter);

		void StartRendering();

		void BeginFrame();
		void Render(RenderBuffer const & vb);
		void EndFrame();

		void DepthBufferDepthTest(bool depthTest);
		void DepthBufferDepthWrite(bool depthWrite);
		void DepthBufferFunction(CompareFunction depthFunction);
		void DepthBias(U16 bias);

		void Fog(FogMode mode = Fog_None,
			Color const & color = Color(1, 1, 1, 1),
			float expDensity = 1, float linearStart = 0, float linearEnd = 1);

		void SetTexture(U32 stage, TexturePtr const & texture);

		void TextureCoordSet(U32 stage, int index);

		U32 MaxTextureStages();
		void DisableTextureStage(U32 stage);

		void TextureCoordCalculation(U32 stage, TexCoordCalcMethod m);
		void TextureAddressingMode(U32 stage, TexAddressingMode tam);
		void TextureMatrix(U32 stage, Matrix4 const & mat);
		void TextureFiltering(U32 stage, TexFiltering texFiltering);
		void TextureAnisotropy(U32 stage, U32 maxAnisotropy);

		void StencilCheckEnabled(bool enabled);
		bool HasHardwareStencil();

		U16 StencilBufferBitDepth();

		void StencilBufferFunction(CompareFunction func);
		void StencilBufferReferenceValue(U32 refValue);
		void StencilBufferMask(U32 mask);
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
