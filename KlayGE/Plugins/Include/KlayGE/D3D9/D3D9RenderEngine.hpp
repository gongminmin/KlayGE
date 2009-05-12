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

#pragma once

#include <KlayGE/Vector.hpp>

#include <KlayGE/D3D9/D3D9AdapterList.hpp>
#include <KlayGE/D3D9/D3D9MinGWDefs.hpp>

#include <d3d9.h>
#include <d3dx9.h>
#include <dxerr9.h>

#include <vector>
#include <boost/array.hpp>
#include <boost/function.hpp>
#include <boost/smart_ptr.hpp>

#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/ShaderObject.hpp>
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
		void D3DDevice(ID3D9DevicePtr const & device);

		void CreateRenderWindow(std::string const & name, RenderSettings const & settings);

		void StartRendering();

		void BeginFrame();
		void EndFrame();
		void BeginPass();
		void EndPass();

		uint16_t StencilBufferBitDepth();

		void ScissorRect(uint32_t x, uint32_t y, uint32_t width, uint32_t height);

		// Directly mapping texels to pixels by offset by 0.5
		// For more info see the doc page titled "Directly Mapping Texels to Pixels"
		float4 TexelToPixelOffset() const
		{
			return float4(-0.5f, 0.5f, 0, 0);
		}

		void Resize(uint32_t width, uint32_t height);
		bool FullScreen() const;
		void FullScreen(bool fs);

		void ConditionalRender(bool cr);

		void OnLostDevice();
		void OnResetDevice();

		void SetRenderState(D3DRENDERSTATETYPE state, uint32_t value);
		void SetTexture(uint32_t sampler, IDirect3DBaseTexture9* texture);
		void SetSamplerState(uint32_t sampler, D3DSAMPLERSTATETYPE type, uint32_t value);
		void SetVertexShader(IDirect3DVertexShader9* shader);
		void SetPixelShader(IDirect3DPixelShader9* shader);
		void SetVertexShaderConstantB(uint32_t start_register, BOOL const * constant_data, uint32_t register_count);
		void SetPixelShaderConstantB(uint32_t start_register, BOOL const * constant_data, uint32_t register_count);
		void SetVertexShaderConstantI(uint32_t start_register, int const * constant_data, uint32_t register_count);
		void SetPixelShaderConstantI(uint32_t start_register, int const * constant_data, uint32_t register_count);
		void SetVertexShaderConstantF(uint32_t start_register, float const * constant_data, uint32_t register_count);
		void SetPixelShaderConstantF(uint32_t start_register, float const * constant_data, uint32_t register_count);

	private:
		void DoBindFrameBuffer(FrameBufferPtr const & fb);
		void DoRender(RenderTechnique const & tech, RenderLayout const & rl);

		void FillRenderDeviceCaps();
		void InitRenderStates();

		void DoRenderSWInstance(RenderTechnique const & tech, RenderLayout const & rl);
		void DoRenderHWInstance(RenderTechnique const & tech, RenderLayout const & rl);
		void RenderRLSWInstance(RenderTechnique const & tech, RenderLayout const & rl);
		void RenderRL(RenderTechnique const & tech, RenderLayout const & rl);

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

		bool conditional_render_;

		boost::function<void (RenderTechnique const &, RenderLayout const &)> RenderInstance;

		boost::array<uint32_t, D3DRS_BLENDOPALPHA + 1> render_states_cache_;
		boost::array<std::vector<std::pair<IDirect3DBaseTexture9*, boost::array<uint32_t, D3DSAMP_SRGBTEXTURE + 1> > >, ShaderObject::ST_NumShaderTypes> samplers_cache_;
		IDirect3DVertexShader9* vertex_shader_cache_;
		IDirect3DPixelShader9* pixel_shader_cache_;
	};

	typedef boost::shared_ptr<D3D9RenderEngine> D3D9RenderEnginePtr;
}

#endif			// _D3D9RENDERENGINE_HPP
