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
		width_ = texture.Width(level);
		height_ = texture.Height(level);
		pf_ = texture.Format();
		sample_count_ = texture.SampleCount();
		sample_quality_ = texture.SampleQuality();

		rt_view_ = checked_cast<D3D11Texture*>(&texture)->RetrieveD3DRenderTargetView(pf_, first_array_index, array_size, level);

		this->BindDiscardFunc();
	}

	D3D11RenderTargetRenderView::D3D11RenderTargetRenderView(Texture& texture_3d, int array_index, uint32_t first_slice, uint32_t num_slices, int level)
		: rt_src_(&texture_3d), rt_first_subres_((array_index * texture_3d.Depth(level) + first_slice) * texture_3d.NumMipMaps() + level), rt_num_subres_(num_slices * texture_3d.NumMipMaps() + level)
	{
		width_ = texture_3d.Width(level);
		height_ = texture_3d.Height(level);
		pf_ = texture_3d.Format();
		sample_count_ = texture_3d.SampleCount();
		sample_quality_ = texture_3d.SampleQuality();

		rt_view_ = checked_cast<D3D11Texture*>(&texture_3d)->RetrieveD3DRenderTargetView(pf_, array_index, first_slice, num_slices, level);

		this->BindDiscardFunc();
	}

	D3D11RenderTargetRenderView::D3D11RenderTargetRenderView(Texture& texture_cube, int array_index, Texture::CubeFaces face, int level)
		: rt_src_(&texture_cube), rt_first_subres_((array_index * 6 + face) * texture_cube.NumMipMaps() + level), rt_num_subres_(1)
	{
		width_ = texture_cube.Width(level);
		height_ = texture_cube.Width(level);
		pf_ = texture_cube.Format();
		sample_count_ = texture_cube.SampleCount();
		sample_quality_ = texture_cube.SampleQuality();

		rt_view_ = checked_cast<D3D11Texture*>(&texture_cube)->RetrieveD3DRenderTargetView(pf_, array_index, face, level);

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
		width_ = texture.Width(level);
		height_ = texture.Height(level);
		pf_ = texture.Format();
		sample_count_ = texture.SampleCount();
		sample_quality_ = texture.SampleQuality();

		ds_view_ = checked_cast<D3D11Texture*>(&texture)->RetrieveD3DDepthStencilView(pf_, first_array_index, array_size, level);

		this->BindDiscardFunc();
	}

	D3D11DepthStencilRenderView::D3D11DepthStencilRenderView(Texture& texture_3d, int array_index, uint32_t first_slice, uint32_t num_slices, int level)
		: rt_src_(&texture_3d), rt_first_subres_((array_index * texture_3d.Depth(level) + first_slice) * texture_3d.NumMipMaps() + level), rt_num_subres_(num_slices * texture_3d.NumMipMaps() + level)
	{
		width_ = texture_3d.Width(level);
		height_ = texture_3d.Height(level);
		pf_ = texture_3d.Format();
		sample_count_ = texture_3d.SampleCount();
		sample_quality_ = texture_3d.SampleQuality();

		ds_view_ = checked_cast<D3D11Texture*>(&texture_3d)->RetrieveD3DDepthStencilView(pf_, array_index, first_slice, num_slices, level);

		this->BindDiscardFunc();
	}

	D3D11DepthStencilRenderView::D3D11DepthStencilRenderView(Texture& texture_cube, int array_index, Texture::CubeFaces face, int level)
		: rt_src_(&texture_cube), rt_first_subres_((array_index * 6 + face) * texture_cube.NumMipMaps() + level), rt_num_subres_(1)
	{
		width_ = texture_cube.Width(level);
		height_ = texture_cube.Width(level);
		pf_ = texture_cube.Format();
		sample_count_ = texture_cube.SampleCount();
		sample_quality_ = texture_cube.SampleQuality();

		ds_view_ = checked_cast<D3D11Texture*>(&texture_cube)->RetrieveD3DDepthStencilView(pf_, array_index, face, level);

		this->BindDiscardFunc();
	}

	D3D11DepthStencilRenderView::D3D11DepthStencilRenderView(uint32_t width, uint32_t height,
											ElementFormat pf, uint32_t sample_count, uint32_t sample_quality)
		: rt_src_(nullptr), rt_first_subres_(0), rt_num_subres_(1)
	{
		BOOST_ASSERT(IsDepthFormat(pf));

		auto& rf = Context::Instance().RenderFactoryInstance();
		TexturePtr ds_tex = rf.MakeTexture2D(width, height, 1, 1, pf, sample_count, sample_quality, EAH_GPU_Write);
		ds_view_ = checked_cast<D3D11Texture*>(ds_tex.get())->RetrieveD3DDepthStencilView(pf, 0, 1, 0);

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


	D3D11UnorderedAccessView::D3D11UnorderedAccessView(TexturePtr const & texture, ElementFormat pf, int first_array_index, int array_size,
		int level)
		: ua_src_(texture.get()), ua_first_subres_(first_array_index * texture->NumMipMaps() + level), ua_num_subres_(1)
	{
		D3D11RenderEngine& renderEngine(*checked_cast<D3D11RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance()));
		d3d_device_ = renderEngine.D3DDevice();
		d3d_imm_ctx_ = renderEngine.D3DDeviceImmContext();

		tex_ = texture;
		pf_ = pf == EF_Unknown ? texture->Format() : pf;

		first_array_index_ = first_array_index;
		array_size_ = array_size;
		level_ = level;
		first_slice_ = 0;
		num_slices_ = texture->Depth(0);
		first_face_ = Texture::CF_Positive_X;
		num_faces_ = 1;
		first_elem_ = 0;
		num_elems_ = 0;

		D3D11_UNORDERED_ACCESS_VIEW_DESC desc;
		desc.Format = D3D11Mapping::MappingFormat(pf_);
		switch (texture->Type())
		{
		case Texture::TT_1D:
			if (array_size_ > 1)
			{
				desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE1DARRAY;
				desc.Texture1DArray.MipSlice = level_;
				desc.Texture1DArray.FirstArraySlice = first_array_index_;
				desc.Texture1DArray.ArraySize = array_size_;
			}
			else
			{
				desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE1D;
				desc.Texture1D.MipSlice = level_;
			}
			break;

		case Texture::TT_2D:
			if (array_size_ > 1)
			{
				desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2DARRAY;
				desc.Texture2DArray.MipSlice = level_;
				desc.Texture2DArray.FirstArraySlice = first_array_index_;
				desc.Texture2DArray.ArraySize = array_size_;
			}
			else
			{
				desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
				desc.Texture2D.MipSlice = level_;
			}
			break;

		case Texture::TT_3D:
			desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE3D;
			desc.Texture3D.MipSlice = level_;
			desc.Texture3D.FirstWSlice = first_slice_;
			desc.Texture3D.WSize = num_slices_;
			break;

		case Texture::TT_Cube:
			desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2DARRAY;
			desc.Texture2DArray.MipSlice = level;
			desc.Texture2DArray.FirstArraySlice = first_array_index_ * 6 + first_face_;
			desc.Texture2DArray.ArraySize = array_size_ * 6 + num_faces_;
			break;
		}

		ID3D11UnorderedAccessView* d3d_ua_view;
		d3d_device_->CreateUnorderedAccessView(checked_cast<D3D11Texture*>(texture.get())->D3DResource(), &desc, &d3d_ua_view);
		d3d_ua_view_ = MakeCOMPtr(d3d_ua_view);

		this->BindDiscardFunc();
	}

	D3D11UnorderedAccessView::D3D11UnorderedAccessView(TexturePtr const & texture_3d, ElementFormat pf, int array_index,
		uint32_t first_slice, uint32_t num_slices, int level)
		: ua_src_(texture_3d.get()), ua_first_subres_((array_index * texture_3d->Depth(level) + first_slice) * texture_3d->NumMipMaps() + level),
			ua_num_subres_(num_slices * texture_3d->NumMipMaps() + level)
	{
		BOOST_ASSERT(array_index == 0);

		D3D11RenderEngine& renderEngine(*checked_cast<D3D11RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance()));
		d3d_device_ = renderEngine.D3DDevice();
		d3d_imm_ctx_ = renderEngine.D3DDeviceImmContext();

		tex_ = texture_3d;
		pf_ = pf == EF_Unknown ? texture_3d->Format() : pf;

		first_array_index_ = array_index;
		array_size_ = 1;
		level_ = level;
		first_slice_ = first_slice;
		num_slices_ = num_slices;
		first_face_ = Texture::CF_Positive_X;
		num_faces_ = 1;
		first_elem_ = 0;
		num_elems_ = 0;

		D3D11_UNORDERED_ACCESS_VIEW_DESC desc;
		desc.Format = D3D11Mapping::MappingFormat(pf_);
		desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE3D;
		desc.Texture3D.MipSlice = level_;
		desc.Texture3D.FirstWSlice = first_slice_;
		desc.Texture3D.WSize = num_slices_;

		ID3D11UnorderedAccessView* d3d_ua_view;
		d3d_device_->CreateUnorderedAccessView(checked_cast<D3D11Texture*>(texture_3d.get())->D3DResource(), &desc, &d3d_ua_view);
		d3d_ua_view_ = MakeCOMPtr(d3d_ua_view);

		this->BindDiscardFunc();
	}

	D3D11UnorderedAccessView::D3D11UnorderedAccessView(TexturePtr const & texture_cube, ElementFormat pf, int array_index,
		Texture::CubeFaces face, int level)
		: ua_src_(texture_cube.get()), ua_first_subres_((array_index * 6 + face) * texture_cube->NumMipMaps() + level), ua_num_subres_(1)
	{
		D3D11RenderEngine& renderEngine(*checked_cast<D3D11RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance()));
		d3d_device_ = renderEngine.D3DDevice();
		d3d_imm_ctx_ = renderEngine.D3DDeviceImmContext();

		tex_ = texture_cube;
		pf_ = pf == EF_Unknown ? texture_cube->Format() : pf;

		first_array_index_ = array_index;
		array_size_ = 1;
		level_ = level;
		first_slice_ = 0;
		num_slices_ = texture_cube->Depth(0);
		first_face_ = face;
		num_faces_ = 1;
		first_elem_ = 0;
		num_elems_ = 0;

		D3D11_UNORDERED_ACCESS_VIEW_DESC desc;
		desc.Format = D3D11Mapping::MappingFormat(pf_);
		desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2DARRAY;
		desc.Texture2DArray.MipSlice = level_;
		desc.Texture2DArray.FirstArraySlice = first_array_index_ * 6 + first_face_;
		desc.Texture2DArray.ArraySize = array_size_ * 6 + num_faces_;

		ID3D11UnorderedAccessView* d3d_ua_view;
		d3d_device_->CreateUnorderedAccessView(checked_cast<D3D11Texture*>(texture_cube.get())->D3DResource(), &desc, &d3d_ua_view);
		d3d_ua_view_ = MakeCOMPtr(d3d_ua_view);

		this->BindDiscardFunc();
	}

	D3D11UnorderedAccessView::D3D11UnorderedAccessView(GraphicsBufferPtr const & gb, ElementFormat pf, uint32_t first_elem,
		uint32_t num_elems)
		: ua_src_(gb.get()), ua_first_subres_(0), ua_num_subres_(1)
	{
		uint32_t const access_hint = gb->AccessHint();

		BOOST_ASSERT(access_hint & EAH_GPU_Unordered);

		D3D11RenderEngine& renderEngine(*checked_cast<D3D11RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance()));
		d3d_device_ = renderEngine.D3DDevice();
		d3d_imm_ctx_ = renderEngine.D3DDeviceImmContext();
		d3d_imm_ctx_1_ = renderEngine.D3DDeviceImmContext1();

		buff_ = gb;
		pf_ = pf;

		first_array_index_ = 0;
		array_size_ = 0;
		level_ = 0;
		first_slice_ = 0;
		num_slices_ = 0;
		first_face_ = Texture::CF_Positive_X;
		num_faces_ = 1;
		first_elem_ = first_elem;
		num_elems_ = num_elems;

		D3D11_UNORDERED_ACCESS_VIEW_DESC uav_desc;
		if (access_hint & EAH_Raw)
		{
			uav_desc.Format = DXGI_FORMAT_R32_TYPELESS;
		}
		else if (access_hint & EAH_GPU_Structured)
		{
			uav_desc.Format = DXGI_FORMAT_UNKNOWN;
		}
		else
		{
			uav_desc.Format = D3D11Mapping::MappingFormat(pf);
		}
		uav_desc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
		uav_desc.Buffer.FirstElement = first_elem;
		uav_desc.Buffer.NumElements = num_elems;
		uav_desc.Buffer.Flags = 0;
		if (access_hint & EAH_Raw)
		{
			uav_desc.Buffer.Flags |= D3D11_BUFFER_UAV_FLAG_RAW;
		}
		if (access_hint & EAH_Append)
		{
			uav_desc.Buffer.Flags |= D3D11_BUFFER_UAV_FLAG_APPEND;
		}
		if (access_hint & EAH_Counter)
		{
			uav_desc.Buffer.Flags |= D3D11_BUFFER_UAV_FLAG_COUNTER;
		}

		ID3D11UnorderedAccessView* d3d_ua_view;
		TIFHR(d3d_device_->CreateUnorderedAccessView(checked_cast<D3D11GraphicsBuffer*>(gb.get())->D3DBuffer(), &uav_desc, &d3d_ua_view));
		d3d_ua_view_ = MakeCOMPtr(d3d_ua_view);

		this->BindDiscardFunc();
	}
	
	D3D11UnorderedAccessView::~D3D11UnorderedAccessView()
	{
	}

	void D3D11UnorderedAccessView::Clear(float4 const & val)
	{
		d3d_imm_ctx_->ClearUnorderedAccessViewFloat(d3d_ua_view_.get(), &val.x());
	}

	void D3D11UnorderedAccessView::Clear(uint4 const & val)
	{
		d3d_imm_ctx_->ClearUnorderedAccessViewUint(d3d_ua_view_.get(), &val.x());
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
		d3d_imm_ctx_1_->DiscardView(d3d_ua_view_.get());
	}

	void D3D11UnorderedAccessView::FackDiscard()
	{
		float clr[] = { 0, 0, 0, 0 };
		d3d_imm_ctx_->ClearUnorderedAccessViewFloat(d3d_ua_view_.get(), clr);
	}

	void D3D11UnorderedAccessView::OnAttached(FrameBuffer& /*fb*/, uint32_t /*att*/)
	{
	}

	void D3D11UnorderedAccessView::OnDetached(FrameBuffer& /*fb*/, uint32_t /*att*/)
	{
	}
}
