// D3D9RenderEngine.hpp
// KlayGE D3D9渲染引擎类 头文件
// Ver 3.0.0
// 版权所有(C) 龚敏敏, 2003-2005
// Homepage: http://klayge.sourceforge.net
//
// 3.0.0
// 去掉了固定流水线 (2005.8.18)
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

#ifndef _D3D9RENDERENGINE_HPP
#define _D3D9RENDERENGINE_HPP

#define NOMINMAX

#include <boost/smart_ptr.hpp>
#include <KlayGE/D3D9/D3D9AdapterList.hpp>

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

		RenderWindowPtr CreateRenderWindow(std::string const & name, RenderSettings const & settings);

		void CullingMode(CullMode mode);
		void PolygonMode(FillMode mode);

		void StartRendering();

		void BeginFrame();
		void EndFrame();

		void AlphaBlend(bool enabled);
		void AlphaBlendFunction(AlphaBlendFactor src_factor, AlphaBlendFactor dst_factor);

		void DepthBufferDepthTest(bool depthTest);
		void DepthBufferDepthWrite(bool depthWrite);
		void DepthBufferFunction(CompareFunction depthFunction);
		void DepthBias(uint16_t bias);

		void AlphaTest(bool enabled);
		void AlphaFunction(CompareFunction alphaFunction, float refValue);

		void SetSampler(uint32_t stage, SamplerPtr const & sampler);
		void DisableSampler(uint32_t stage);

		void StencilCheckEnabled(bool enabled);
		bool HasHardwareStencil();

		uint16_t StencilBufferBitDepth();

		void StencilBufferFunction(CompareFunction func, uint32_t refValue, uint32_t mask);
		void StencilBufferOperation(StencilOperation fail, StencilOperation depth_fail, StencilOperation pass);

		void PointSpriteEnable(bool enable);
		void PointDistanceAttenuation(float quadratic0, float quadratic1, float quadratic2);
		void PointSize(float size);
		void PointMinMaxSize(float min_size, float max_size);

		void ScissorTest(bool enabled);
		void ScissorRect(uint32_t x, uint32_t y, uint32_t width, uint32_t height);

		void OnLostDevice();
		void OnResetDevice();

	private:
		void DoActiveRenderTarget(uint32_t n, RenderTargetPtr renderTarget);

		void DoRender(VertexBuffer const & vb);

		void FillRenderDeviceCaps();

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

		CullMode cullingMode_;
		D3DCOLOR clearClr_;
		uint32_t clearFlags_;

		size_t last_num_vertex_stream_;
	};

	typedef boost::shared_ptr<D3D9RenderEngine> D3D9RenderEnginePtr;
}

#endif			// _D3D9RENDERENGINE_HPP
