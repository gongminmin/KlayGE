#ifndef _OGLRENDERENGINE_HPP
#define _OGLRENDERENGINE_HPP

#include <KlayGE/alloc.hpp>

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

		const WString& Name() const;

		void ClearColor(const Color& clr);

		void ShadingType(ShadeOptions so);

		void EnableLighting(bool enabled);
		void AmbientLight(const Color& col);

		RenderWindowPtr CreateRenderWindow(const String &name, const RenderSettings& settings);

		void CullingMode(CullMode mode);

		void SetMaterial(const Material& mat);

		void SetLight(U32 index, const Light& lt);
		void LightEnable(U32 index, bool enable);

		void ActiveRenderTarget(RenderTargetListIterator iter);

		void StartRendering();

		void BeginFrame();
		void Render(const RenderBuffer& vb);
		void EndFrame();

		void DepthBufferDepthTest(bool depthTest);
		void DepthBufferDepthWrite(bool depthWrite);
		void DepthBufferFunction(CompareFunction depthFunction);
		void DepthBias(U16 bias);

		void Fog(FogMode mode = Fog_None,
			const Color& color = Color(1, 1, 1, 1),
			float expDensity = 1, float linearStart = 0, float linearEnd = 1);

		void SetTexture(U32 stage, const Texture& texture);

		void TextureCoordSet(U32 stage, int index);

		U32 MaxTextureStages();
		void DisableTextureStage(U32 stage);

		void TextureCoordCalculation(U32 stage, TexCoordCalcMethod m);
		void TextureAddressingMode(U32 stage, TexAddressingMode tam);
		void TextureMatrix(U32 stage, const Matrix4& mat);
		void TextureFiltering(U32 stage, TexFiltering texFiltering);
		void TextureAnisotropy(U32 stage, U32 maxAnisotropy);

		U32 MaxVertexBlendMatrices();


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
		void DoWorldMatrix(const Matrix4& mat);
		void DoViewMatrix(const Matrix4& mat);
		void DoProjectionMatrix(const Matrix4& mat);

	private:
		CullMode cullingMode_;
	};

	typedef SharedPtr<OGLRenderEngine> OGLRenderEnginePtr;
}

#endif			// _OGLRENDERENGINE_HPP
