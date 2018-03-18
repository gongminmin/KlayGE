// D3D11RenderView.cpp
// KlayGE D3D11渲染视图类 实现文件
// Ver 3.8.0
// 版权所有(C) 龚敏敏, 2009
// Homepage: http://www.klayge.org
//
// 3.8.0
// 初次建立 (2009.1.30)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KFL/ErrorHandling.hpp>
#include <KFL/Util.hpp>
#include <KFL/COMPtr.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/FrameBuffer.hpp>

#include <boost/assert.hpp>

#include <KlayGE/D3D11/D3D11RenderEngine.hpp>
#include <KlayGE/D3D11/D3D11Mapping.hpp>
#include <KlayGE/D3D11/D3D11Texture.hpp>
#include <KlayGE/D3D11/D3D11RenderView.hpp>

namespace KlayGE
{
	D3D11RenderView::D3D11RenderView()
	{
		D3D11RenderEngine& renderEngine(*checked_cast<D3D11RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance()));
		d3d_device_ = renderEngine.D3DDevice();
		d3d_imm_ctx_ = renderEngine.D3DDeviceImmContext();
		d3d_imm_ctx_1_ = renderEngine.D3DDeviceImmContext1();
	}

	D3D11RenderView::~D3D11RenderView()
	{
	}


	D3D11RenderTargetRenderView::D3D11RenderTargetRenderView(Texture& texture, int first_array_index, int array_size, int level)
		: rt_src_(&texture), rt_first_subres_(first_array_index * texture.NumMipMaps() + level), rt_num_subres_(1)
	{
		rt_view_ = checked_cast<D3D11Texture*>(&texture)->RetriveD3DRenderTargetView(first_array_index, array_size, level);

		width_ = texture.Width(level);
		height_ = texture.Height(level);
		pf_ = texture.Format();
		sample_count_ = texture.SampleCount();
		sample_quality_ = texture.SampleQuality();

		this->BindDiscardFunc();
	}

	D3D11RenderTargetRenderView::D3D11RenderTargetRenderView(Texture& texture_3d, int array_index, uint32_t first_slice, uint32_t num_slices, int level)
		: rt_src_(&texture_3d), rt_first_subres_((array_index * texture_3d.Depth(level) + first_slice) * texture_3d.NumMipMaps() + level), rt_num_subres_(num_slices * texture_3d.NumMipMaps() + level)
	{
		rt_view_ = checked_cast<D3D11Texture*>(&texture_3d)->RetriveD3DRenderTargetView(array_index, first_slice, num_slices, level);

		width_ = texture_3d.Width(level);
		height_ = texture_3d.Height(level);
		pf_ = texture_3d.Format();
		sample_count_ = texture_3d.SampleCount();
		sample_quality_ = texture_3d.SampleQuality();

		this->BindDiscardFunc();
	}

	D3D11RenderTargetRenderView::D3D11RenderTargetRenderView(Texture& texture_cube, int array_index, Texture::CubeFaces face, int level)
		: rt_src_(&texture_cube), rt_first_subres_((array_index * 6 + face) * texture_cube.NumMipMaps() + level), rt_num_subres_(1)
	{
		rt_view_ = checked_cast<D3D11Texture*>(&texture_cube)->RetriveD3DRenderTargetView(array_index, face, level);

		width_ = texture_cube.Width(level);
		height_ = texture_cube.Width(level);
		pf_ = texture_cube.Format();
		sample_count_ = texture_cube.SampleCount();
		sample_quality_ = texture_cube.SampleQuality();

		this->BindDiscardFunc();
	}

	D3D11RenderTargetRenderView::D3D11RenderTargetRenderView(GraphicsBuffer& gb, uint32_t width, uint32_t height, ElementFormat pf)
		: rt_src_(&gb), rt_first_subres_(0), rt_num_subres_(1)
	{
		BOOST_ASSERT(gb.AccessHint() & EAH_GPU_Write);

		rt_view_ = checked_cast<D3D11GraphicsBuffer*>(&gb)->D3DRenderTargetView();

		width_ = width * height;
		height_ = 1;
		pf_ = pf;
		sample_count_ = 1;
		sample_quality_ = 0;

		this->BindDiscardFunc();
	}

	void D3D11RenderTargetRenderView::ClearColor(Color const & clr)
	{
		d3d_imm_ctx_->ClearRenderTargetView(rt_view_.get(), &clr.r());
	}

	void D3D11RenderTargetRenderView::ClearDepth(float /*depth*/)
	{
		KFL_UNREACHABLE("Can't be called");
	}

	void D3D11RenderTargetRenderView::ClearStencil(int32_t /*stencil*/)
	{
		KFL_UNREACHABLE("Can't be called");
	}

	void D3D11RenderTargetRenderView::ClearDepthStencil(float /*depth*/, int32_t /*stencil*/)
	{
		KFL_UNREACHABLE("Can't be called");
	}

	void D3D11RenderTargetRenderView::Discard()
	{
		discard_func_();
	}

	void D3D11RenderTargetRenderView::BindDiscardFunc()
	{
		D3D11RenderEngine& re = *checked_cast<D3D11RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		if (re.D3D11RuntimeSubVer() >= 1)
		{
			discard_func_ = [this] { this->HWDiscard(); };
		}
		else
		{
			discard_func_ = [this] { this->FackDiscard(); };
		}
	}

	void D3D11RenderTargetRenderView::HWDiscard()
	{
		d3d_imm_ctx_1_->DiscardView(rt_view_.get());
	}

	void D3D11RenderTargetRenderView::FackDiscard()
	{
		float clr[] = { 0, 0, 0, 0 };
		d3d_imm_ctx_->ClearRenderTargetView(rt_view_.get(), clr);
	}

	void D3D11RenderTargetRenderView::OnAttached(FrameBuffer& /*fb*/, uint32_t /*att*/)
	{
	}

	void D3D11RenderTargetRenderView::OnDetached(FrameBuffer& /*fb*/, uint32_t /*att*/)
	{
	}


	D3D11DepthStencilRenderView::D3D11DepthStencilRenderView(Texture& texture, int first_array_index, int array_size, int level)
		: rt_src_(&texture), rt_first_subres_(first_array_index * texture.NumMipMaps() + level), rt_num_subres_(1)
	{
		ds_view_ = checked_cast<D3D11Texture*>(&texture)->RetriveD3DDepthStencilView(first_array_index, array_size, level);

		width_ = texture.Width(level);
		height_ = texture.Height(level);
		pf_ = texture.Format();
		sample_count_ = texture.SampleCount();
		sample_quality_ = texture.SampleQuality();

		this->BindDiscardFunc();
	}

	D3D11DepthStencilRenderView::D3D11DepthStencilRenderView(Texture& texture_3d, int array_index, uint32_t first_slice, uint32_t num_slices, int level)
		: rt_src_(&texture_3d), rt_first_subres_((array_index * texture_3d.Depth(level) + first_slice) * texture_3d.NumMipMaps() + level), rt_num_subres_(num_slices * texture_3d.NumMipMaps() + level)
	{
		ds_view_ = checked_cast<D3D11Texture*>(&texture_3d)->RetriveD3DDepthStencilView(array_index, first_slice, num_slices, level);

		width_ = texture_3d.Width(level);
		height_ = texture_3d.Height(level);
		pf_ = texture_3d.Format();
		sample_count_ = texture_3d.SampleCount();
		sample_quality_ = texture_3d.SampleQuality();

		this->BindDiscardFunc();
	}

	D3D11DepthStencilRenderView::D3D11DepthStencilRenderView(Texture& texture_cube, int array_index, Texture::CubeFaces face, int level)
		: rt_src_(&texture_cube), rt_first_subres_((array_index * 6 + face) * texture_cube.NumMipMaps() + level), rt_num_subres_(1)
	{
		ds_view_ = checked_cast<D3D11Texture*>(&texture_cube)->RetriveD3DDepthStencilView(array_index, face, level);

		width_ = texture_cube.Width(level);
		height_ = texture_cube.Width(level);
		pf_ = texture_cube.Format();
		sample_count_ = texture_cube.SampleCount();
		sample_quality_ = texture_cube.SampleQuality();

		this->BindDiscardFunc();
	}

	D3D11DepthStencilRenderView::D3D11DepthStencilRenderView(uint32_t width, uint32_t height,
											ElementFormat pf, uint32_t sample_count, uint32_t sample_quality)
		: rt_src_(nullptr), rt_first_subres_(0), rt_num_subres_(1)
	{
		BOOST_ASSERT(IsDepthFormat(pf));

		auto& rf = Context::Instance().RenderFactoryInstance();
		TexturePtr ds_tex = rf.MakeTexture2D(width, height, 1, 1, pf, sample_count, sample_quality, EAH_GPU_Write);
		ds_view_ = checked_cast<D3D11Texture*>(ds_tex.get())->RetriveD3DDepthStencilView(0, 1, 0);

		width_ = width;
		height_ = height;
		pf_ = pf;
		sample_count_ = sample_count;
		sample_quality_ = sample_quality;

		this->BindDiscardFunc();
	}

	void D3D11DepthStencilRenderView::ClearColor(Color const & /*clr*/)
	{
		KFL_UNREACHABLE("Can't be called");
	}

	void D3D11DepthStencilRenderView::ClearDepth(float depth)
	{
		d3d_imm_ctx_->ClearDepthStencilView(ds_view_.get(), D3D11_CLEAR_DEPTH, depth, 0);
	}

	void D3D11DepthStencilRenderView::ClearStencil(int32_t stencil)
	{
		d3d_imm_ctx_->ClearDepthStencilView(ds_view_.get(), D3D11_CLEAR_STENCIL, 1, static_cast<uint8_t>(stencil));
	}

	void D3D11DepthStencilRenderView::ClearDepthStencil(float depth, int32_t stencil)
	{
		d3d_imm_ctx_->ClearDepthStencilView(ds_view_.get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, depth, static_cast<uint8_t>(stencil));
	}

	void D3D11DepthStencilRenderView::Discard()
	{
		discard_func_();
	}
	
	void D3D11DepthStencilRenderView::BindDiscardFunc()
	{
		D3D11RenderEngine& re = *checked_cast<D3D11RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		if (re.D3D11RuntimeSubVer() >= 1)
		{
			discard_func_ = [this] { this->HWDiscard(); };
		}
		else
		{
			discard_func_ = [this] { this->FackDiscard(); };
		}
	}

	void D3D11DepthStencilRenderView::HWDiscard()
	{
		d3d_imm_ctx_1_->DiscardView(ds_view_.get());
	}

	void D3D11DepthStencilRenderView::FackDiscard()
	{
		d3d_imm_ctx_->ClearDepthStencilView(ds_view_.get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1, 0);
	}

	void D3D11DepthStencilRenderView::OnAttached(FrameBuffer& /*fb*/, uint32_t att)
	{
		KFL_UNUSED(att);

		BOOST_ASSERT(FrameBuffer::ATT_DepthStencil == att);
	}

	void D3D11DepthStencilRenderView::OnDetached(FrameBuffer& /*fb*/, uint32_t att)
	{
		KFL_UNUSED(att);

		BOOST_ASSERT(FrameBuffer::ATT_DepthStencil == att);
	}


	D3D11UnorderedAccessView::D3D11UnorderedAccessView(Texture& texture, int first_array_index, int array_size, int level)
		: ua_src_(&texture), ua_first_subres_(first_array_index * texture.NumMipMaps() + level), ua_num_subres_(1)
	{
		D3D11RenderEngine& renderEngine(*checked_cast<D3D11RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance()));
		d3d_device_ = renderEngine.D3DDevice();
		d3d_imm_ctx_ = renderEngine.D3DDeviceImmContext();

		ua_view_ = checked_cast<D3D11Texture*>(&texture)->RetriveD3DUnorderedAccessView(first_array_index, array_size, level);

		width_ = texture.Width(level);
		height_ = texture.Height(level);
		pf_ = texture.Format();

		this->BindDiscardFunc();
	}

	D3D11UnorderedAccessView::D3D11UnorderedAccessView(Texture& texture_3d, int array_index, uint32_t first_slice, uint32_t num_slices, int level)
		: ua_src_(&texture_3d), ua_first_subres_((array_index * texture_3d.Depth(level) + first_slice) * texture_3d.NumMipMaps() + level), ua_num_subres_(num_slices * texture_3d.NumMipMaps() + level)
	{
		D3D11RenderEngine& renderEngine(*checked_cast<D3D11RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance()));
		d3d_device_ = renderEngine.D3DDevice();
		d3d_imm_ctx_ = renderEngine.D3DDeviceImmContext();

		ua_view_ = checked_cast<D3D11Texture*>(&texture_3d)->RetriveD3DUnorderedAccessView(array_index, first_slice, num_slices, level);

		width_ = texture_3d.Width(level);
		height_ = texture_3d.Height(level);
		pf_ = texture_3d.Format();

		this->BindDiscardFunc();
	}

	D3D11UnorderedAccessView::D3D11UnorderedAccessView(Texture& texture_cube, int array_index, Texture::CubeFaces face, int level)
		: ua_src_(&texture_cube), ua_first_subres_((array_index * 6 + face) * texture_cube.NumMipMaps() + level), ua_num_subres_(1)
	{
		D3D11RenderEngine& renderEngine(*checked_cast<D3D11RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance()));
		d3d_device_ = renderEngine.D3DDevice();
		d3d_imm_ctx_ = renderEngine.D3DDeviceImmContext();

		ua_view_ = checked_cast<D3D11Texture*>(&texture_cube)->RetriveD3DUnorderedAccessView(array_index, face, level);

		width_ = texture_cube.Width(level);
		height_ = texture_cube.Width(level);
		pf_ = texture_cube.Format();

		this->BindDiscardFunc();
	}

	D3D11UnorderedAccessView::D3D11UnorderedAccessView(GraphicsBuffer& gb, ElementFormat pf)
		: ua_src_(&gb), ua_first_subres_(0), ua_num_subres_(1)
	{
		BOOST_ASSERT(gb.AccessHint() & EAH_GPU_Write);

		D3D11RenderEngine& renderEngine(*checked_cast<D3D11RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance()));
		d3d_device_ = renderEngine.D3DDevice();
		d3d_imm_ctx_ = renderEngine.D3DDeviceImmContext();
		d3d_imm_ctx_1_ = renderEngine.D3DDeviceImmContext1();

		ua_view_ = checked_cast<D3D11GraphicsBuffer*>(&gb)->D3DUnorderedAccessView();

		width_ = gb.Size() / NumFormatBytes(pf);
		height_ = 1;
		pf_ = pf;

		this->BindDiscardFunc();
	}
	
	D3D11UnorderedAccessView::~D3D11UnorderedAccessView()
	{
	}

	void D3D11UnorderedAccessView::Clear(float4 const & val)
	{
		d3d_imm_ctx_->ClearUnorderedAccessViewFloat(ua_view_.get(), &val.x());
	}

	void D3D11UnorderedAccessView::Clear(uint4 const & val)
	{
		d3d_imm_ctx_->ClearUnorderedAccessViewUint(ua_view_.get(), &val.x());
	}

	void D3D11UnorderedAccessView::Discard()
	{
		discard_func_();
	}

	void D3D11UnorderedAccessView::BindDiscardFunc()
	{
		D3D11RenderEngine& re = *checked_cast<D3D11RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		if (re.D3D11RuntimeSubVer() >= 1)
		{
			discard_func_ = [this] { this->HWDiscard(); };
		}
		else
		{
			discard_func_ = [this] { this->FackDiscard(); };
		}
	}

	void D3D11UnorderedAccessView::HWDiscard()
	{
		d3d_imm_ctx_1_->DiscardView(ua_view_.get());
	}

	void D3D11UnorderedAccessView::FackDiscard()
	{
		float clr[] = { 0, 0, 0, 0 };
		d3d_imm_ctx_->ClearUnorderedAccessViewFloat(ua_view_.get(), clr);
	}

	void D3D11UnorderedAccessView::OnAttached(FrameBuffer& /*fb*/, uint32_t /*att*/)
	{
	}

	void D3D11UnorderedAccessView::OnDetached(FrameBuffer& /*fb*/, uint32_t /*att*/)
	{
	}
}
