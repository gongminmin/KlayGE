#ifndef _D3D9RENDERENGINE_HPP
#define _D3D9RENDERENGINE_HPP

#include <boost/smart_ptr.hpp>
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

		std::wstring const & Name() const;

		boost::shared_ptr<IDirect3D9> const & D3D() const;
		boost::shared_ptr<IDirect3DDevice9> const & D3DDevice() const;

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
		void Render(RenderBuffer const & rb);
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
		D3D9AdapterList const & D3DAdapters() const;
		D3D9Adapter const & ActiveAdapter() const;

		// Direct3D rendering device
		// Only created after top-level window created
		boost::shared_ptr<IDirect3D9>		d3d_;
		boost::shared_ptr<IDirect3DDevice9>	d3dDevice_;

		// List of D3D drivers installed (video cards)
		// Enumerates itself
		D3D9AdapterList adapterList_;

		D3DCAPS9 caps_;

		CullMode cullingMode_;
		D3DCOLOR clearClr_;
		uint32 clearFlags_;

		typedef std::vector<D3DVERTEXELEMENT9> VertexDeclType;
		VertexDeclType currentDecl_;
		boost::shared_ptr<IDirect3DVertexDeclaration9> currentVertexDecl_;
	};

	typedef boost::shared_ptr<D3D9RenderEngine> D3D9RenderEnginePtr;
}

#endif			// _D3D9RENDERENGINE_HPP
