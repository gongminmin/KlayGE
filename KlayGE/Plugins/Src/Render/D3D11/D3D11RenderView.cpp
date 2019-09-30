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
	D3D11TextureShaderResourceView::D3D11TextureShaderResourceView(TexturePtr const & texture, ElementFormat pf, uint32_t first_array_index,
		uint32_t array_size, uint32_t first_level, uint32_t num_levels)
	{
		BOOST_ASSERT(texture->AccessHint() & EAH_GPU_Read);

		auto const& re = checked_cast<D3D11RenderEngine const&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		d3d_device_ = re.D3DDevice1();
		d3d_imm_ctx_ = re.D3DDeviceImmContext1();

		tex_ = texture;
		pf_ = pf == EF_Unknown ? texture->Format() : pf;

		first_array_index_ = first_array_index;
		array_size_ = array_size;
		first_level_ = first_level;
		num_levels_ = num_levels;
		first_elem_ = 0;
		num_elems_ = 0;

		sr_src_ = texture.get();
	}

	ID3D11ShaderResourceView* D3D11TextureShaderResourceView::RetrieveD3DShaderResourceView() const
	{
		if (!d3d_sr_view_ && tex_ && tex_->HWResourceReady())
		{
			d3d_sr_view_ = checked_cast<D3D11Texture&>(*tex_).RetrieveD3DShaderResourceView(pf_, first_array_index_, array_size_,
				first_level_, num_levels_);
		}
		return d3d_sr_view_.get();
	}


	D3D11CubeTextureFaceShaderResourceView::D3D11CubeTextureFaceShaderResourceView(TexturePtr const& texture_cube, ElementFormat pf, int array_index,
		Texture::CubeFaces face, uint32_t first_level, uint32_t num_levels)
	{
		BOOST_ASSERT(texture_cube->AccessHint() & EAH_GPU_Read);

		auto const& re = checked_cast<D3D11RenderEngine const&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		d3d_device_ = re.D3DDevice1();
		d3d_imm_ctx_ = re.D3DDeviceImmContext1();

		tex_ = texture_cube;
		pf_ = pf == EF_Unknown ? texture_cube->Format() : pf;

		first_array_index_ = array_index * 6 + face;
		array_size_ = 1;
		first_level_ = first_level;
		num_levels_ = num_levels;
		first_elem_ = 0;
		num_elems_ = 0;

		sr_src_ = texture_cube.get();
	}

	ID3D11ShaderResourceView* D3D11CubeTextureFaceShaderResourceView::RetrieveD3DShaderResourceView() const
	{
		if (!d3d_sr_view_ && tex_ && tex_->HWResourceReady())
		{
			uint32_t const array_index = first_array_index_ / 6;
			Texture::CubeFaces const face = static_cast<Texture::CubeFaces>(first_array_index_ - array_index * 6);
			d3d_sr_view_ = checked_cast<D3D11Texture&>(*tex_).RetrieveD3DShaderResourceView(pf_, array_index, face,
				first_level_, num_levels_);
		}
		return d3d_sr_view_.get();
	}


	D3D11BufferShaderResourceView::D3D11BufferShaderResourceView(GraphicsBufferPtr const & gb, ElementFormat pf, uint32_t first_elem,
		uint32_t num_elems)
	{
		BOOST_ASSERT(gb->AccessHint() & EAH_GPU_Read);

		auto const& re = checked_cast<D3D11RenderEngine const&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		d3d_device_ = re.D3DDevice1();
		d3d_imm_ctx_ = re.D3DDeviceImmContext1();

		buff_ = gb;
		pf_ = pf;

		first_array_index_ = 0;
		array_size_ = 0;
		first_level_ = 0;
		num_levels_ = 0;
		first_elem_ = first_elem;
		num_elems_ = num_elems;

		sr_src_ = gb.get();
	}

	ID3D11ShaderResourceView* D3D11BufferShaderResourceView::RetrieveD3DShaderResourceView() const
	{
		if (!d3d_sr_view_ && buff_ && buff_->HWResourceReady())
		{
			d3d_sr_view_ = checked_cast<D3D11GraphicsBuffer&>(*buff_).RetrieveD3DShaderResourceView(pf_, first_elem_, num_elems_);
		}
		return d3d_sr_view_.get();
	}


	D3D11RenderTargetView::D3D11RenderTargetView(void* src, uint32_t first_subres, uint32_t num_subres)
		: rt_src_(src), rt_first_subres_(first_subres), rt_num_subres_(num_subres)
	{
		auto const& re = checked_cast<D3D11RenderEngine const&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		d3d_device_ = re.D3DDevice1();
		d3d_imm_ctx_ = re.D3DDeviceImmContext1();
	}

	void D3D11RenderTargetView::ClearColor(Color const & clr)
	{
		d3d_imm_ctx_->ClearRenderTargetView(d3d_rt_view_.get(), &clr.r());
	}

	void D3D11RenderTargetView::Discard()
	{
		d3d_imm_ctx_->DiscardView(d3d_rt_view_.get());
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


	D3D11Texture1D2DCubeRenderTargetView::D3D11Texture1D2DCubeRenderTargetView(TexturePtr const & texture, ElementFormat pf,
		int first_array_index, int array_size, int level)
		: D3D11RenderTargetView(texture.get(), first_array_index * texture->NumMipMaps() + level, 1)
	{
		BOOST_ASSERT(texture);

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

		this->RetrieveD3DRenderTargetView();
	}

	ID3D11RenderTargetView* D3D11Texture1D2DCubeRenderTargetView::RetrieveD3DRenderTargetView() const
	{
		if (!d3d_rt_view_ && tex_->HWResourceReady())
		{
			d3d_rt_view_ = checked_cast<D3D11Texture&>(*tex_).RetrieveD3DRenderTargetView(pf_, first_array_index_, array_size_, level_);
		}
		return d3d_rt_view_.get();
	}


	D3D11Texture3DRenderTargetView::D3D11Texture3DRenderTargetView(TexturePtr const & texture_3d, ElementFormat pf, int array_index, uint32_t first_slice,
		uint32_t num_slices, int level)
		: D3D11RenderTargetView(texture_3d.get(),
			(array_index * texture_3d->Depth(level) + first_slice) * texture_3d->NumMipMaps() + level,
			num_slices * texture_3d->NumMipMaps() + level)
	{
		BOOST_ASSERT(texture_3d);

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

		this->RetrieveD3DRenderTargetView();
	}

	ID3D11RenderTargetView* D3D11Texture3DRenderTargetView::RetrieveD3DRenderTargetView() const
	{
		if (!d3d_rt_view_ && tex_->HWResourceReady())
		{
			d3d_rt_view_ = checked_cast<D3D11Texture&>(*tex_).RetrieveD3DRenderTargetView(pf_, first_array_index_, first_slice_,
				num_slices_, level_);
		}
		return d3d_rt_view_.get();
	}


	D3D11TextureCubeFaceRenderTargetView::D3D11TextureCubeFaceRenderTargetView(TexturePtr const & texture_cube, ElementFormat pf, int array_index,
		Texture::CubeFaces face, int level)
		: D3D11RenderTargetView(texture_cube.get(), (array_index * 6 + face) * texture_cube->NumMipMaps() + level, 1)
	{
		BOOST_ASSERT(texture_cube);

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

		this->RetrieveD3DRenderTargetView();
	}

	ID3D11RenderTargetView* D3D11TextureCubeFaceRenderTargetView::RetrieveD3DRenderTargetView() const
	{
		if (!d3d_rt_view_ && tex_->HWResourceReady())
		{
			d3d_rt_view_ = checked_cast<D3D11Texture&>(*tex_).RetrieveD3DRenderTargetView(pf_, first_array_index_, first_face_,
				level_);
		}
		return d3d_rt_view_.get();
	}


	D3D11BufferRenderTargetView::D3D11BufferRenderTargetView(GraphicsBufferPtr const & gb, ElementFormat pf, uint32_t first_elem,
		uint32_t num_elems)
		: D3D11RenderTargetView(gb.get(), 0, 1)
	{
		BOOST_ASSERT(gb);
		BOOST_ASSERT(gb->AccessHint() & EAH_GPU_Write);

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

		this->RetrieveD3DRenderTargetView();
	}

	ID3D11RenderTargetView* D3D11BufferRenderTargetView::RetrieveD3DRenderTargetView() const
	{
		if (!d3d_rt_view_ && buff_->HWResourceReady())
		{
			d3d_rt_view_ = checked_cast<D3D11GraphicsBuffer&>(*buff_).RetrieveD3DRenderTargetView(pf_, first_elem_, num_elems_);
		}
		return d3d_rt_view_.get();
	}


	D3D11DepthStencilView::D3D11DepthStencilView(void* src, uint32_t first_subres, uint32_t num_subres)
		: rt_src_(src), rt_first_subres_(first_subres), rt_num_subres_(num_subres)
	{
		auto const& re = checked_cast<D3D11RenderEngine const&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		d3d_device_ = re.D3DDevice1();
		d3d_imm_ctx_ = re.D3DDeviceImmContext1();
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
		d3d_imm_ctx_->DiscardView(d3d_ds_view_.get());
	}

	void D3D11DepthStencilView::OnAttached(FrameBuffer& fb)
	{
		KFL_UNUSED(fb);
	}

	void D3D11DepthStencilView::OnDetached(FrameBuffer& fb)
	{
		KFL_UNUSED(fb);
	}


	D3D11Texture1D2DCubeDepthStencilView::D3D11Texture1D2DCubeDepthStencilView(TexturePtr const & texture, ElementFormat pf,
		int first_array_index, int array_size, int level)
		: D3D11DepthStencilView(texture.get(), first_array_index * texture->NumMipMaps() + level, 1)
	{
		BOOST_ASSERT(texture);

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

		this->RetrieveD3DDepthStencilView();
	}
	
	D3D11Texture1D2DCubeDepthStencilView::D3D11Texture1D2DCubeDepthStencilView(uint32_t width, uint32_t height, ElementFormat pf,
		uint32_t sample_count, uint32_t sample_quality)
		: D3D11DepthStencilView(nullptr, 0, 1)
	{
		BOOST_ASSERT(IsDepthFormat(pf));

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

		this->RetrieveD3DDepthStencilView();
	}

	ID3D11DepthStencilView* D3D11Texture1D2DCubeDepthStencilView::RetrieveD3DDepthStencilView() const
	{
		if (!d3d_ds_view_ && tex_->HWResourceReady())
		{
			d3d_ds_view_ = checked_cast<D3D11Texture&>(*tex_).RetrieveD3DDepthStencilView(pf_, first_array_index_, array_size_,
				level_);
		}
		return d3d_ds_view_.get();
	}


	D3D11Texture3DDepthStencilView::D3D11Texture3DDepthStencilView(TexturePtr const & texture_3d, ElementFormat pf, int array_index,
		uint32_t first_slice, uint32_t num_slices, int level)
		: D3D11DepthStencilView(texture_3d.get(), (array_index * texture_3d->Depth(level) + first_slice) * texture_3d->NumMipMaps() + level,
			num_slices * texture_3d->NumMipMaps() + level)
	{
		BOOST_ASSERT(texture_3d);

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

		this->RetrieveD3DDepthStencilView();
	}

	ID3D11DepthStencilView* D3D11Texture3DDepthStencilView::RetrieveD3DDepthStencilView() const
	{
		if (!d3d_ds_view_ && tex_->HWResourceReady())
		{
			d3d_ds_view_ = checked_cast<D3D11Texture&>(*tex_).RetrieveD3DDepthStencilView(pf_, first_array_index_, first_slice_,
				num_slices_, level_);
		}
		return d3d_ds_view_.get();
	}


	D3D11TextureCubeFaceDepthStencilView::D3D11TextureCubeFaceDepthStencilView(TexturePtr const & texture_cube, ElementFormat pf,
		int array_index, Texture::CubeFaces face, int level)
		: D3D11DepthStencilView(texture_cube.get(), (array_index * 6 + face) * texture_cube->NumMipMaps() + level, 1)
	{
		BOOST_ASSERT(texture_cube);

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

		this->RetrieveD3DDepthStencilView();
	}

	ID3D11DepthStencilView* D3D11TextureCubeFaceDepthStencilView::RetrieveD3DDepthStencilView() const
	{
		if (!d3d_ds_view_ && tex_->HWResourceReady())
		{
			d3d_ds_view_ = checked_cast<D3D11Texture&>(*tex_).RetrieveD3DDepthStencilView(pf_, first_array_index_, first_face_, level_);
		}
		return d3d_ds_view_.get();
	}


	D3D11UnorderedAccessView::D3D11UnorderedAccessView(void* src, uint32_t first_subres, uint32_t num_subres)
		: ua_src_(src), ua_first_subres_(first_subres), ua_num_subres_(num_subres)
	{
		auto const& re = checked_cast<D3D11RenderEngine const&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		d3d_device_ = re.D3DDevice1();
		d3d_imm_ctx_ = re.D3DDeviceImmContext1();
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
		d3d_imm_ctx_->DiscardView(d3d_ua_view_.get());
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


	D3D11Texture1D2DCubeUnorderedAccessView::D3D11Texture1D2DCubeUnorderedAccessView(TexturePtr const & texture, ElementFormat pf,
		int first_array_index, int array_size, int level)
		: D3D11UnorderedAccessView(texture.get(), first_array_index * texture->NumMipMaps() + level, 1)
	{
		BOOST_ASSERT(texture);

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

		this->RetrieveD3DUnorderedAccessView();
	}

	ID3D11UnorderedAccessView* D3D11Texture1D2DCubeUnorderedAccessView::RetrieveD3DUnorderedAccessView() const
	{
		if (!d3d_ua_view_ && tex_->HWResourceReady())
		{
			d3d_ua_view_ = checked_cast<D3D11Texture&>(*tex_).RetrieveD3DUnorderedAccessView(pf_, first_array_index_, array_size_,
				level_);
		}
		return d3d_ua_view_.get();
	}


	D3D11Texture3DUnorderedAccessView::D3D11Texture3DUnorderedAccessView(TexturePtr const & texture_3d, ElementFormat pf, int array_index,
		uint32_t first_slice, uint32_t num_slices, int level)
		: D3D11UnorderedAccessView(texture_3d.get(),
			(array_index * texture_3d->Depth(level) + first_slice) * texture_3d->NumMipMaps() + level,
			num_slices * texture_3d->NumMipMaps() + level)
	{
		BOOST_ASSERT(texture_3d);
		BOOST_ASSERT(array_index == 0);

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

		this->RetrieveD3DUnorderedAccessView();
	}

	ID3D11UnorderedAccessView* D3D11Texture3DUnorderedAccessView::RetrieveD3DUnorderedAccessView() const
	{
		if (!d3d_ua_view_ && tex_->HWResourceReady())
		{
			d3d_ua_view_ = checked_cast<D3D11Texture&>(*tex_).RetrieveD3DUnorderedAccessView(pf_, first_array_index_, first_slice_,
				num_slices_, level_);
		}
		return d3d_ua_view_.get();
	}


	D3D11TextureCubeFaceUnorderedAccessView::D3D11TextureCubeFaceUnorderedAccessView(TexturePtr const & texture_cube, ElementFormat pf,
		int array_index, Texture::CubeFaces face, int level)
		: D3D11UnorderedAccessView(texture_cube.get(), (array_index * 6 + face) * texture_cube->NumMipMaps() + level, 1)
	{
		BOOST_ASSERT(texture_cube);

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

		this->RetrieveD3DUnorderedAccessView();
	}

	ID3D11UnorderedAccessView* D3D11TextureCubeFaceUnorderedAccessView::RetrieveD3DUnorderedAccessView() const
	{
		if (!d3d_ua_view_ && tex_->HWResourceReady())
		{
			d3d_ua_view_ = checked_cast<D3D11Texture&>(*tex_).RetrieveD3DUnorderedAccessView(pf_, first_array_index_, first_face_,
				level_);
		}
		return d3d_ua_view_.get();
	}


	D3D11BufferUnorderedAccessView::D3D11BufferUnorderedAccessView(GraphicsBufferPtr const & gb, ElementFormat pf, uint32_t first_elem,
		uint32_t num_elems)
		: D3D11UnorderedAccessView(gb.get(), 0, 1)
	{
		BOOST_ASSERT(gb);
		BOOST_ASSERT(gb->AccessHint() & EAH_GPU_Unordered);

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

		this->RetrieveD3DUnorderedAccessView();
	}

	ID3D11UnorderedAccessView* D3D11BufferUnorderedAccessView::RetrieveD3DUnorderedAccessView() const
	{
		if (!d3d_ua_view_ && buff_->HWResourceReady())
		{
			d3d_ua_view_ = checked_cast<D3D11GraphicsBuffer&>(*buff_).RetrieveD3DUnorderedAccessView(pf_, first_elem_, num_elems_);
		}
		return d3d_ua_view_.get();
	}
}
