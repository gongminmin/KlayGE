#ifndef _D3D9RENDERENGINE_HPP
#define _D3D9RENDERENGINE_HPP

#include <KlayGE/COMPtr.hpp>
#include <KlayGE/D3D9/D3D9AdapterList.hpp>

#define NOMINMAX

#include <d3d9.h>
#include <d3dx9.h>
#include <dxerr9.h>

#include <vector>

#include <KlayGE/RenderEngine.hpp>

#pragma comment(lib, "KlayGE_RenderEngine_D3D9.lib")

namespace KlayGE
{
	class D3D9AdapterList;
	class D3D9Adapter;

	class D3D9RenderEngine : public RenderEngine
	{
	public:
		D3D9RenderEngine();
		~D3D9RenderEngine();

		const std::wstring& Name() const;

		const COMPtr<IDirect3D9>& D3D() const;
		const COMPtr<IDirect3DDevice9>& D3DDevice() const;

		void ClearColor(const Color& clr);

		void ShadingType(ShadeOptions so);

		void EnableLighting(bool enabled);
		void AmbientLight(const Color& col);

		RenderWindowPtr CreateRenderWindow(const std::string& name, const RenderSettings& settings);

		void CullingMode(CullMode mode);

		void SetMaterial(const Material& mat);

		void SetLight(U32 index, const Light& lt);
		void LightEnable(U32 index, bool enable);

		void ActiveRenderTarget(RenderTargetListIterator iter);

		void StartRendering();

		void BeginFrame();
		void Render(const RenderBuffer& rb);
		void EndFrame();

		void DepthBufferDepthTest(bool depthTest);
		void DepthBufferDepthWrite(bool depthWrite);
		void DepthBufferFunction(CompareFunction depthFunction);
		void DepthBias(U16 bias);

		void Fog(FogMode mode = Fog_None,
			const Color& color = Color(1, 1, 1, 1),
			float expDensity = 1, float linearStart = 0, float linearEnd = 1);

		void SetTexture(U32 stage, const TexturePtr& texture);

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
		void DoWorldMatrix();
		void DoViewMatrix();
		void DoProjectionMatrix();

	private:
		const D3D9AdapterList& D3DAdapters() const;
		const D3D9Adapter& ActiveAdapter() const;

		// Direct3D rendering device
		// Only created after top-level window created
		COMPtr<IDirect3D9>			d3d_;
		COMPtr<IDirect3DDevice9>	d3dDevice_;

		// List of D3D drivers installed (video cards)
		// Enumerates itself
		D3D9AdapterList adapterList_;

		D3DCAPS9 caps_;

		CullMode cullingMode_;
		D3DCOLOR clearClr_;
		U32 clearFlags_;

		typedef std::vector<D3DVERTEXELEMENT9> VertexDeclType;
		VertexDeclType currentDecl_;
		COMPtr<IDirect3DVertexDeclaration9> currentVertexDecl_;
	};

	typedef boost::shared_ptr<D3D9RenderEngine> D3D9RenderEnginePtr;
}

#endif			// _D3D9RENDERENGINE_HPP
