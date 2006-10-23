// D3D9RenderEngine.hpp
// KlayGE D3D9渲染引擎类 头文件
// Ver 3.4.0
// 版权所有(C) 龚敏敏, 2003-2006
// Homepage: http://klayge.sourceforge.net
//
// 3.4.0
// 增加了TexelToPixelOffset (2006.8.27)
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

#define KLAYGE_LIB_NAME KlayGE_RenderEngine_D3D9
#include <KlayGE/config/auto_link.hpp>

#include <boost/smart_ptr.hpp>
#include <KlayGE/D3D9/D3D9AdapterList.hpp>

#include <d3d9.h>
#include <d3dx9.h>
#include <dxerr9.h>

#include <vector>
#include <boost/function.hpp>

#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/D3D9/D3D9Typedefs.hpp>
#include <KlayGE/D3D9/D3D9GraphicsBuffer.hpp>

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

		ID3D9Ptr const & D3DObject() const;
		ID3D9DevicePtr const & D3DDevice() const;

		void Clear(uint32_t masks, Color const & clr, float depth, int32_t stencil);

		RenderWindowPtr CreateRenderWindow(std::string const & name, RenderSettings const & settings);

		void StartRendering();

		void BeginFrame();
		void EndFrame();

		void SetSampler(uint32_t stage, SamplerPtr const & sampler);
		void DisableSampler(uint32_t stage);

		uint16_t StencilBufferBitDepth();

		void ScissorRect(uint32_t x, uint32_t y, uint32_t width, uint32_t height);

		// Directly mapping texels to pixels by offset by 0.5
		// For more info see the doc page titled "Directly Mapping Texels to Pixels"
		float4 TexelToPixelOffset() const
		{
			return float4(-0.5f, 0.5f, 0, 0);
		}

		void OnLostDevice();
		void OnResetDevice();

	private:
		void DoBindRenderTarget(RenderTargetPtr rt);
		void DoRender(RenderLayout const & rl);
		void DoFlushRenderStates();

		void FillRenderDeviceCaps();

		void DoRenderSWInstance(RenderLayout const & rl);
		void DoRenderHWInstance(RenderLayout const & rl);
		void RenderRLSWInstance(RenderLayout const & rl);
		void RenderRL(RenderLayout const & rl);

	private:
		D3D9AdapterList const & D3DAdapters() const;
		D3D9Adapter const & ActiveAdapter() const;

		// Direct3D rendering device
		// Only created after top-level window created
		ID3D9Ptr		d3d_;
		ID3D9DevicePtr	d3dDevice_;

		// List of D3D drivers installed (video cards)
		// Enumerates itself
		D3D9AdapterList adapterList_;

		uint32_t last_num_vertex_stream_;

		boost::function<void (RenderLayout const &)> RenderInstance;
	};

	typedef boost::shared_ptr<D3D9RenderEngine> D3D9RenderEnginePtr;
}

#endif			// _D3D9RENDERENGINE_HPP
