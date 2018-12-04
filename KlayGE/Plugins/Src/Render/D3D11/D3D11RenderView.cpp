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
	D3D11RenderTargetView::D3D11RenderTargetView(TexturePtr const & texture, ElementFormat pf, int first_array_index, int array_size,
		int level)
		: rt_src_(texture.get()), rt_first_subres_(first_array_index * texture->NumMipMaps() + level), rt_num_subres_(1)
	{
		D3D11RenderEngine& renderEngine(*checked_cast<D3D11RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance()));
		d3d_device_ = renderEngine.D3DDevice();
		d3d_imm_ctx_ = renderEngine.D3DDeviceImmContext();
		d3d_imm_ctx_1_ = renderEngine.D3DDeviceImmContext1();

		tex_ = texture;
		width_ = texture->Width(level);
		height_ = texture->Height(level);
		pf_ = pf == EF_Unknown ? texture->Format() : pf;
		sample_count_ = texture->SampleCount();
		sample_quality_ = texture->SampleQuality();

		first_array_index_ = first_array_index;
		array_size_ = array_size;
		level_ = level;
		first_slice_ = 0;
		num_slices_ = texture->Depth(0);
		first_face_ = Texture::CF_Positive_X;
		num_faces_ = 1;
		first_elem_ = 0;
		num_elems_ = 0;

		d3d_rt_view_ = checked_cast<D3D11Texture*>(texture.get())->CreateD3DRenderTargetView(pf_, first_array_index_, array_size_, level_);

		this->BindDiscardFunc();
	}

	D3D11RenderTargetView::D3D11RenderTargetView(TexturePtr const & texture_3d, ElementFormat pf, int array_index, uint32_t first_slice,
		uint32_t num_slices, int level)
		: rt_src_(texture_3d.get()), rt_first_subres_((array_index * texture_3d->Depth(level) + first_slice) * texture_3d->NumMipMaps() + level), rt_num_subres_(num_slices * texture_3d->NumMipMaps() + level)
	{
		D3D11RenderEngine& renderEngine(*checked_cast<D3D11RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance()));
		d3d_device_ = renderEngine.D3DDevice();
		d3d_imm_ctx_ = renderEngine.D3DDeviceImmContext();
		d3d_imm_ctx_1_ = renderEngine.D3DDeviceImmContext1();

		tex_ = texture_3d;
		width_ = texture_3d->Width(level);
		height_ = texture_3d->Height(level);
		pf_ = pf == EF_Unknown ? texture_3d->Format() : pf;
		sample_count_ = texture_3d->SampleCount();
		sample_quality_ = texture_3d->SampleQuality();

		first_array_index_ = array_index;
		array_size_ = 1;
		level_ = level;
		first_slice_ = first_slice;
		num_slices_ = num_slices;
		first_face_ = Texture::CF_Positive_X;
		num_faces_ = 1;
		first_elem_ = 0;
		num_elems_ = 0;

		d3d_rt_view_ = checked_cast<D3D11Texture*>(texture_3d.get())->CreateD3DRenderTargetView(pf_, first_array_index_, first_slice_,
			num_slices_, level_);

		this->BindDiscardFunc();
	}

	D3D11RenderTargetView::D3D11RenderTargetView(TexturePtr const & texture_cube, ElementFormat pf, int array_index,
		Texture::CubeFaces face, int level)
		: rt_src_(texture_cube.get()), rt_first_subres_((array_index * 6 + face) * texture_cube->NumMipMaps() + level), rt_num_subres_(1)
	{
		D3D11RenderEngine& renderEngine(*checked_cast<D3D11RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance()));
		d3d_device_ = renderEngine.D3DDevice();
		d3d_imm_ctx_ = renderEngine.D3DDeviceImmContext();
		d3d_imm_ctx_1_ = renderEngine.D3DDeviceImmContext1();

		tex_ = texture_cube;
		width_ = texture_cube->Width(level);
		height_ = texture_cube->Width(level);
		pf_ = pf == EF_Unknown ? texture_cube->Format() : pf;
		sample_count_ = texture_cube->SampleCount();
		sample_quality_ = texture_cube->SampleQuality();

		first_array_index_ = array_index;
		array_size_ = 1;
		level_ = level;
		first_slice_ = 0;
		num_slices_ = texture_cube->Depth(0);
		first_face_ = face;
		num_faces_ = 1;
		first_elem_ = 0;
		num_elems_ = 0;

		d3d_rt_view_ = checked_cast<D3D11Texture*>(texture_cube.get())->CreateD3DRenderTargetView(pf_, first_array_index_, first_face_,
			level_);

		this->BindDiscardFunc();
	}

	D3D11RenderTargetView::D3D11RenderTargetView(GraphicsBufferPtr const & gb, ElementFormat pf, uint32_t first_elem, uint32_t num_elems)
		: rt_src_(gb.get()), rt_first_subres_(0), rt_num_subres_(1)
	{
		BOOST_ASSERT(gb->AccessHint() & EAH_GPU_Write);

		D3D11RenderEngine& renderEngine(*checked_cast<D3D11RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance()));
		d3d_device_ = renderEngine.D3DDevice();
		d3d_imm_ctx_ = renderEngine.D3DDeviceImmContext();
		d3d_imm_ctx_1_ = renderEngine.D3DDeviceImmContext1();

		buff_ = gb;
		width_ = num_elems;
		height_ = 1;
		pf_ = pf;
		sample_count_ = 1;
		sample_quality_ = 0;

		first_array_index_ = 0;
		array_size_ = 0;
		level_ = 0;
		first_slice_ = 0;
		num_slices_ = 0;
		first_face_ = Texture::CF_Positive_X;
		num_faces_ = 1;
		first_elem_ = first_elem;
		num_elems_ = num_elems;

		d3d_rt_view_ = checked_cast<D3D11GraphicsBuffer*>(gb.get())->CreateD3DRenderTargetView(pf, first_elem_, num_elems_);

		this->BindDiscardFunc();
	}

	void D3D11RenderTargetView::ClearColor(Color const & clr)
	{
		d3d_imm_ctx_->ClearRenderTargetView(d3d_rt_view_.get(), &clr.r());
	}

	void D3D11RenderTargetView::Discard()
	{
		discard_func_();
	}

	void D3D11RenderTargetView::BindDiscardFunc()
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

	void D3D11RenderTargetView::HWDiscard()
	{
		d3d_imm_ctx_1_->DiscardView(d3d_rt_view_.get());
	}

	void D3D11RenderTargetView::FackDiscard()
	{
		float clr[] = { 0, 0, 0, 0 };
		d3d_imm_ctx_->ClearRenderTargetView(d3d_rt_view_.get(), clr);
	}

	void D3D11RenderTargetView::OnAttached(FrameBuffer& fb, FrameBuffer::Attachment att)
	{
		KFL_UNUSED(fb);
		KFL_UNUSED(att);
	}

	void D3D11RenderTargetView::OnDetached(FrameBuffer& fb, FrameBuffer::Attachment att)
	{
		KFL_UNUSED(fb);
		KFL_UNUSED(att);
	}


	D3D11DepthStencilView::D3D11DepthStencilView(TexturePtr const & texture, ElementFormat pf, int first_array_index, int array_size,
		int level)
		: rt_src_(texture.get()), rt_first_subres_(first_array_index * texture->NumMipMaps() + level), rt_num_subres_(1)
	{
		D3D11RenderEngine& renderEngine(*checked_cast<D3D11RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance()));
		d3d_device_ = renderEngine.D3DDevice();
		d3d_imm_ctx_ = renderEngine.D3DDeviceImmContext();
		d3d_imm_ctx_1_ = renderEngine.D3DDeviceImmContext1();

		tex_ = texture;
		width_ = texture->Width(level);
		height_ = texture->Height(level);
		pf_ = pf == EF_Unknown ? texture->Format() : pf;
		sample_count_ = texture->SampleCount();
		sample_quality_ = texture->SampleQuality();

		first_array_index_ = first_array_index;
		array_size_ = array_size;
		level_ = level;
		first_slice_ = 0;
		num_slices_ = texture->Depth(0);
		first_face_ = Texture::CF_Positive_X;
		num_faces_ = 1;

		d3d_ds_view_ = checked_cast<D3D11Texture*>(tex_.get())->CreateD3DDepthStencilView(pf_, first_array_index_, array_size_, level_);

		this->BindDiscardFunc();
	}

	D3D11DepthStencilView::D3D11DepthStencilView(TexturePtr const & texture_3d, ElementFormat pf, int array_index, uint32_t first_slice,
		uint32_t num_slices, int level)
		: rt_src_(texture_3d.get()), rt_first_subres_((array_index * texture_3d->Depth(level) + first_slice) * texture_3d->NumMipMaps() + level), rt_num_subres_(num_slices * texture_3d->NumMipMaps() + level)
	{
		D3D11RenderEngine& renderEngine(*checked_cast<D3D11RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance()));
		d3d_device_ = renderEngine.D3DDevice();
		d3d_imm_ctx_ = renderEngine.D3DDeviceImmContext();
		d3d_imm_ctx_1_ = renderEngine.D3DDeviceImmContext1();

		tex_ = texture_3d;
		width_ = texture_3d->Width(level);
		height_ = texture_3d->Height(level);
		pf_ = pf == EF_Unknown ? texture_3d->Format() : pf;
		sample_count_ = texture_3d->SampleCount();
		sample_quality_ = texture_3d->SampleQuality();

		first_array_index_ = array_index;
		array_size_ = 1;
		level_ = level;
		first_slice_ = first_slice;
		num_slices_ = num_slices;
		first_face_ = Texture::CF_Positive_X;
		num_faces_ = 1;

		d3d_ds_view_ = checked_cast<D3D11Texture*>(tex_.get())->CreateD3DDepthStencilView(pf_, first_array_index_, first_slice_,
			num_slices_, level_);

		this->BindDiscardFunc();
	}

	D3D11DepthStencilView::D3D11DepthStencilView(TexturePtr const & texture_cube, ElementFormat pf, int array_index,
		Texture::CubeFaces face, int level)
		: rt_src_(texture_cube.get()), rt_first_subres_((array_index * 6 + face) * texture_cube->NumMipMaps() + level), rt_num_subres_(1)
	{
		D3D11RenderEngine& renderEngine(*checked_cast<D3D11RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance()));
		d3d_device_ = renderEngine.D3DDevice();
		d3d_imm_ctx_ = renderEngine.D3DDeviceImmContext();
		d3d_imm_ctx_1_ = renderEngine.D3DDeviceImmContext1();

		tex_ = texture_cube;
		width_ = texture_cube->Width(level);
		height_ = texture_cube->Width(level);
		pf_ = pf == EF_Unknown ? texture_cube->Format() : pf;
		sample_count_ = texture_cube->SampleCount();
		sample_quality_ = texture_cube->SampleQuality();

		first_array_index_ = array_index;
		array_size_ = 1;
		level_ = level;
		first_slice_ = 0;
		num_slices_ = texture_cube->Depth(0);
		first_face_ = face;
		num_faces_ = 1;

		d3d_ds_view_ = checked_cast<D3D11Texture*>(tex_.get())->CreateD3DDepthStencilView(pf_, first_array_index_, first_face_, level_);

		this->BindDiscardFunc();
	}

	D3D11DepthStencilView::D3D11DepthStencilView(uint32_t width, uint32_t height, ElementFormat pf, uint32_t sample_count,
		uint32_t sample_quality)
		: rt_first_subres_(0), rt_num_subres_(1)
	{
		BOOST_ASSERT(IsDepthFormat(pf));

		D3D11RenderEngine& renderEngine(*checked_cast<D3D11RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance()));
		d3d_device_ = renderEngine.D3DDevice();
		d3d_imm_ctx_ = renderEngine.D3DDeviceImmContext();
		d3d_imm_ctx_1_ = renderEngine.D3DDeviceImmContext1();

		auto& rf = Context::Instance().RenderFactoryInstance();
		tex_ = rf.MakeTexture2D(width, height, 1, 1, pf, sample_count, sample_quality, EAH_GPU_Write);
		rt_src_ = tex_.get();

		width_ = width;
		height_ = height;
		pf_ = pf;
		sample_count_ = sample_count;
		sample_quality_ = sample_quality;

		first_array_index_ = 0;
		array_size_ = 1;
		level_ = 0;
		first_slice_ = 0;
		num_slices_ = tex_->Depth(0);
		first_face_ = Texture::CF_Positive_X;
		num_faces_ = 1;

		d3d_ds_view_ = checked_cast<D3D11Texture*>(tex_.get())->CreateD3DDepthStencilView(pf_, first_array_index_, array_size_, level_);

		this->BindDiscardFunc();
	}

	void D3D11DepthStencilView::ClearDepth(float depth)
	{
		d3d_imm_ctx_->ClearDepthStencilView(d3d_ds_view_.get(), D3D11_CLEAR_DEPTH, depth, 0);
	}

	void D3D11DepthStencilView::ClearStencil(int32_t stencil)
	{
		d3d_imm_ctx_->ClearDepthStencilView(d3d_ds_view_.get(), D3D11_CLEAR_STENCIL, 1, static_cast<uint8_t>(stencil));
	}

	void D3D11DepthStencilView::ClearDepthStencil(float depth, int32_t stencil)
	{
		d3d_imm_ctx_->ClearDepthStencilView(d3d_ds_view_.get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, depth,
			static_cast<uint8_t>(stencil));
	}

	void D3D11DepthStencilView::Discard()
	{
		discard_func_();
	}
	
	void D3D11DepthStencilView::BindDiscardFunc()
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

	void D3D11DepthStencilView::HWDiscard()
	{
		d3d_imm_ctx_1_->DiscardView(d3d_ds_view_.get());
	}

	void D3D11DepthStencilView::FackDiscard()
	{
		d3d_imm_ctx_->ClearDepthStencilView(d3d_ds_view_.get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1, 0);
	}

	void D3D11DepthStencilView::OnAttached(FrameBuffer& fb)
	{
		KFL_UNUSED(fb);
	}

	void D3D11DepthStencilView::OnDetached(FrameBuffer& fb)
	{
		KFL_UNUSED(fb);
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

		d3d_ua_view_ = checked_cast<D3D11Texture*>(tex_.get())->CreateD3DUnorderedAccessView(pf_, first_array_index_, array_size_, level_);

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

		d3d_ua_view_ = checked_cast<D3D11Texture*>(tex_.get())->CreateD3DUnorderedAccessView(pf_, first_array_index_, first_slice_,
			num_slices_, level_);

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

		d3d_ua_view_ = checked_cast<D3D11Texture*>(tex_.get())->CreateD3DUnorderedAccessView(pf_, first_array_index_, first_face_, level_);

		this->BindDiscardFunc();
	}

	D3D11UnorderedAccessView::D3D11UnorderedAccessView(GraphicsBufferPtr const & gb, ElementFormat pf, uint32_t first_elem,
		uint32_t num_elems)
		: ua_src_(gb.get()), ua_first_subres_(0), ua_num_subres_(1)
	{
		uint32_t const access_hint = gb->AccessHint();
		BOOST_ASSERT(access_hint & EAH_GPU_Unordered);
		KFL_UNUSED(access_hint);

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

		d3d_ua_view_ = checked_cast<D3D11GraphicsBuffer*>(buff_.get())->CreateD3DUnorderedAccessView(pf_, first_elem_, num_elems_);

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

	void D3D11UnorderedAccessView::OnAttached(FrameBuffer& fb, uint32_t index)
	{
		KFL_UNUSED(fb);
		KFL_UNUSED(index);
	}

	void D3D11UnorderedAccessView::OnDetached(FrameBuffer& fb, uint32_t index)
	{
		KFL_UNUSED(fb);
		KFL_UNUSED(index);
	}
}
