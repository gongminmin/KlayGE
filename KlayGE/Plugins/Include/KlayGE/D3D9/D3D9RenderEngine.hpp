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

#ifdef KLAYGE_DEBUG
	#pragma comment(lib, "KlayGE_RenderEngine_D3D9_d.lib")
#else
	#pragma comment(lib, "KlayGE_RenderEngine_D3D9.lib")
#endif

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

		void SetLight(uint32_t index, Light const & lt);
		void LightEnable(uint32_t index, bool enable);

		void ActiveRenderTarget(RenderTargetListIterator iter);

		void StartRendering();

		void BeginFrame();
		void Render(VertexBuffer const & rb);
		void EndFrame();

		void DepthBufferDepthTest(bool depthTest);
		void DepthBufferDepthWrite(bool depthWrite);
		void DepthBufferFunction(CompareFunction depthFunction);
		void DepthBias(uint16_t bias);

		void Fog(FogMode mode = Fog_None,
			Color const & color = Color(1, 1, 1, 1),
			float expDensity = 1, float linearStart = 0, float linearEnd = 1);

		void SetTexture(uint32_t stage, TexturePtr const & texture);

		void TextureCoordSet(uint32_t stage, int index);

		uint32_t MaxTextureStages();
		void DisableTextureStage(uint32_t stage);

		void TextureCoordCalculation(uint32_t stage, TexCoordCalcMethod m);
		void TextureAddressingMode(uint32_t stage, TexAddressingMode tam);
		void TextureMatrix(uint32_t stage, Matrix4 const & mat);
		void TextureFiltering(uint32_t stage, TexFiltering texFiltering);
		void TextureAnisotropy(uint32_t stage, uint32_t maxAnisotropy);

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
		uint32_t clearFlags_;

		typedef std::vector<D3DVERTEXELEMENT9> VertexDeclType;
		VertexDeclType currentDecl_;
		boost::shared_ptr<IDirect3DVertexDeclaration9> currentVertexDecl_;
	};

	typedef boost::shared_ptr<D3D9RenderEngine> D3D9RenderEnginePtr;
}

#endif			// _D3D9RENDERENGINE_HPP
