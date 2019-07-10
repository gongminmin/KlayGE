// FFT.cpp
// KlayGE Fast Fourier Transform implement file
// Ver 4.1.0
// Copyright(C) Minmin Gong, 2012
// Homepage: http://www.klayge.org
//
// 4.1.0
// First release (2012.4.11)
//
// CHANGE LIST
//////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/PostProcess.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KFL/Half.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/FrameBuffer.hpp>

#include <tuple>

#include <boost/assert.hpp>

#include <KlayGE/FFT.hpp>

namespace KlayGE
{
	GpuFftPS::GpuFftPS(uint32_t width, uint32_t height, bool forward)
		: width_(width), height_(height), forward_(forward)
	{
		BOOST_ASSERT(0 == (width_ & (width_ - 1)));
		BOOST_ASSERT(0 == (height_ & (height_ - 1)));

		log_x_ = static_cast<uint32_t>(log(static_cast<float>(width_)) / log(2.0f));
		log_y_ = static_cast<uint32_t>(log(static_cast<float>(height_)) / log(2.0f));

		lookup_i_wr_wi_x_tex_.resize(log_x_);
		lookup_i_wr_wi_x_srv_.resize(log_x_);
		lookup_i_wr_wi_y_tex_.resize(log_y_);
		lookup_i_wr_wi_y_srv_.resize(log_y_);

		std::vector<half> lookup_i_wr_wi_x(log_x_ * width_ * 4);
		std::vector<half> lookup_i_wr_wi_y(log_y_ * height_ * 4);

		this->CreateButterflyLookups(lookup_i_wr_wi_x, log_x_, width_);
		this->CreateButterflyLookups(lookup_i_wr_wi_y, log_y_, height_);

		RenderFactory& rf = Context::Instance().RenderFactoryInstance();

		half* ptr = &lookup_i_wr_wi_x[0];
		for (uint32_t i = 0; i < log_x_; ++ i)
		{
			ElementInitData init_data;
			
			init_data.data = ptr;
			init_data.row_pitch = width_ * sizeof(half) * 4;
			init_data.slice_pitch = init_data.row_pitch;
			lookup_i_wr_wi_x_tex_[i] =
				rf.MakeTexture2D(width_, 1, 1, 1, EF_ABGR16F, 1, 0, EAH_GPU_Read | EAH_Immutable, MakeSpan<1>(init_data));
			lookup_i_wr_wi_x_srv_[i] = rf.MakeTextureSrv(lookup_i_wr_wi_x_tex_[i]);
			ptr += width_ * 4;
		}

		ptr = &lookup_i_wr_wi_y[0];
		for (uint32_t i = 0; i < log_y_; ++ i)
		{
			ElementInitData init_data;
			
			init_data.data = ptr;
			init_data.row_pitch = sizeof(half) * 4;
			init_data.slice_pitch = init_data.row_pitch * height_;
			lookup_i_wr_wi_y_tex_[i] =
				rf.MakeTexture2D(1, height_, 1, 1, EF_ABGR16F, 1, 0, EAH_GPU_Read | EAH_Immutable, MakeSpan<1>(init_data));
			lookup_i_wr_wi_y_srv_[i] = rf.MakeTextureSrv(lookup_i_wr_wi_y_tex_[i]);
			ptr += height_ * 4;
		}

		for (uint32_t i = 0; i < 2; ++i)
		{
			tmp_real_tex_[i] = rf.MakeTexture2D(width_, height_, 1, 1, EF_ABGR32F, 1, 0, EAH_GPU_Read | EAH_GPU_Write);
			tmp_real_srv_[i] = rf.MakeTextureSrv(tmp_real_tex_[i]);
			tmp_real_rtv_[i] = rf.Make2DRtv(tmp_real_tex_[i], 0, 1, 0);

			tmp_imag_tex_[i] = rf.MakeTexture2D(width_, height_, 1, 1, EF_ABGR32F, 1, 0, EAH_GPU_Read | EAH_GPU_Write);
			tmp_imag_srv_[i] = rf.MakeTextureSrv(tmp_imag_tex_[i]);
			tmp_imag_rtv_[i] = rf.Make2DRtv(tmp_imag_tex_[i], 0, 1, 0);
		}

		fft_x_pp_ = SyncLoadPostProcess("FFT.ppml", "fft_x");
		fft_y_pp_ = SyncLoadPostProcess("FFT.ppml", "fft_y");
	}

	void GpuFftPS::Execute(TexturePtr const& out_real, TexturePtr const& out_imag,
		ShaderResourceViewPtr const& in_real, ShaderResourceViewPtr const& in_imag)
	{
		auto& rf = Context::Instance().RenderFactoryInstance();

		int active = 0;

		for (uint32_t i = 0; i < log_x_; ++ i)
		{
			if (0 == i)
			{
				fft_x_pp_->InputPin(0, in_real);
				fft_x_pp_->InputPin(1, in_imag);
			}
			else
			{
				fft_x_pp_->InputPin(0, tmp_real_srv_[active]);
				fft_x_pp_->InputPin(1, tmp_imag_srv_[active]);
			}
			fft_x_pp_->InputPin(2, lookup_i_wr_wi_x_srv_[i]);
			fft_x_pp_->OutputPin(0, tmp_real_rtv_[!active]);
			fft_x_pp_->OutputPin(1, tmp_imag_rtv_[!active]);
			fft_x_pp_->Apply();

			active = !active;
		}

		for (uint32_t i = 0; i < log_y_; ++ i)
		{
			fft_y_pp_->InputPin(0, tmp_real_srv_[active]);
			fft_y_pp_->InputPin(1, tmp_imag_srv_[active]);
			fft_y_pp_->InputPin(2, lookup_i_wr_wi_y_srv_[i]);
			if (log_y_ - 1 == i)
			{
				if (forward_)
				{
					fft_y_pp_->SetParam(0, -1.0f);
				}
				else
				{
					fft_y_pp_->SetParam(0, 1.0f / (width_ * height_));
				}
				fft_y_pp_->OutputPin(0, rf.Make2DRtv(out_real, 0, 1, 0));
				fft_y_pp_->OutputPin(1, rf.Make2DRtv(out_imag, 0, 1, 0));
			}
			else
			{
				fft_y_pp_->SetParam(0, -1.0f);
				fft_y_pp_->OutputPin(0, tmp_real_rtv_[!active]);
				fft_y_pp_->OutputPin(1, tmp_imag_rtv_[!active]);
			}
			fft_y_pp_->Apply();

			active = !active;
		}
	}
	
	int GpuFftPS::BitReverse(int i, int n)
	{
		int j = i;

		int m = n;
		int sum = 0;
		int w = 1;
		m /= 2;
		while (m != 0)
		{
			j = (i & m) > m - 1;
			sum += j * w;
			w *= 2;
			m /= 2;
		}
		return sum;
	}

	void GpuFftPS::ComputeWeight(float& wr, float& wi, int n, int k)
	{
		float phase = 2 * PI * k / n;
		MathLib::sincos(phase, wi, wr);
		wi = forward_ ? -wi : wi;
	}

	void GpuFftPS::CreateButterflyLookups(std::vector<half>& lookup_i_wr_wi, int log_n, int n)
	{
		half* ptr = &lookup_i_wr_wi[0];

		for (int i = 0; i < log_n; ++ i)
		{
			int const blocks = 1UL << (log_n - 1 - i);
			int const hinputs = 1UL << i;
			for (int j = 0; j < blocks; ++ j)
			{
				for (int k = 0; k < hinputs; ++ k)
				{
					int i1, i2, j1, j2;
					i1 = j * hinputs * 2 + k;
					i2 = j * hinputs * 2 + hinputs + k;
					if (0 == i)
					{
						j1 = this->BitReverse(i1, n);
						j2 = this->BitReverse(i2, n);
					}
					else
					{
						j1 = i1;
						j2 = i2;
					}

					float wr, wi;
					this->ComputeWeight(wr, wi, n, k * blocks);

					ptr[i1 * 4 + 0] = half((j1 + 0.5f) / n);
					ptr[i1 * 4 + 1] = half((j2 + 0.5f) / n);
					ptr[i1 * 4 + 2] = half(+wr);
					ptr[i1 * 4 + 3] = half(+wi);

					ptr[i2 * 4 + 0] = half((j1 + 0.5f) / n);
					ptr[i2 * 4 + 1] = half((j2 + 0.5f) / n);
					ptr[i2 * 4 + 2] = half(-wr);
					ptr[i2 * 4 + 3] = half(-wi);
				}
			}

			ptr += n * 4;
		}
	}
	

	int const COHERENCY_GRANULARITY = 128;
	int const BLOCK_SIZE_X = 16;
	int const BLOCK_SIZE_Y = 16;

	GpuFftCS4::GpuFftCS4(uint32_t width, uint32_t height, bool forward)
			: width_(width), height_(height), forward_(forward)
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();

		src_ = rf.MakeVertexBuffer(BU_Dynamic, EAH_GPU_Read | EAH_GPU_Unordered | EAH_GPU_Structured,
			3 * width * height * sizeof(float2), nullptr, sizeof(float2));
		src_srv_ = rf.MakeBufferSrv(src_, EF_GR32F);
		src_uav_ = rf.MakeBufferUav(src_, EF_GR32F);

		dst_ = rf.MakeVertexBuffer(BU_Dynamic, EAH_GPU_Read | EAH_GPU_Unordered | EAH_GPU_Structured,
			3 * width * height * sizeof(float2), nullptr, sizeof(float2));
		src_srv_ = rf.MakeBufferSrv(src_, EF_GR32F);
		dst_uav_ = rf.MakeBufferUav(dst_, EF_GR32F);

		tmp_buffer_ = rf.MakeVertexBuffer(BU_Dynamic, EAH_GPU_Read | EAH_GPU_Unordered | EAH_GPU_Structured,
			3 * width * height * sizeof(float2), nullptr, sizeof(float2));
		tmp_buffer_srv_ = rf.MakeBufferSrv(tmp_buffer_, EF_GR32F);
		tmp_buffer_uav_ = rf.MakeBufferUav(tmp_buffer_, EF_GR32F);

		quad_layout_ = rf.MakeRenderLayout();
		quad_layout_->TopologyType(RenderLayout::TT_TriangleStrip);

		float2 xyzs[] =
		{
			float2(-1, +1),
			float2(+1, +1),
			float2(-1, -1),
			float2(+1, -1)
		};
		GraphicsBufferPtr quad_vb = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, sizeof(xyzs), xyzs);
		quad_layout_->BindVertexStream(quad_vb, VertexElement(VEU_Position, 0, EF_GR32F));

		tex_fb_ = rf.MakeFrameBuffer();

		effect_ = SyncLoadRenderEffect("FFT.fxml");
		buf2tex_tech_ = effect_->TechniqueByName("Buf2Tex");
		radix008a_tech_ = effect_->TechniqueByName("FFTRadix008A4");
		radix008a_first_tech_ = effect_->TechniqueByName("FFTRadix008AFirst4");
		radix008a_final_tech_ = effect_->TechniqueByName("FFTRadix008AFinal4");
		real_tex_ep_ = effect_->ParameterByName("real_tex");
		imag_tex_ep_ = effect_->ParameterByName("imag_tex");

		*(effect_->ParameterByName("input_buf")) = dst_srv_;

		*(effect_->ParameterByName("tex_width_height")) = uint2(width, height);
		uint32_t n = width * height;
		*(effect_->ParameterByName("addr_offset")) = uint3(0 * n, 1 * n, 2 * n);

		*(effect_->ParameterByName("forward")) = static_cast<int32_t>(forward_);
		*(effect_->ParameterByName("scale")) = 1.0f / (width_ * height_);
	}

	void GpuFftCS4::Execute(
		TexturePtr const& out_real, TexturePtr const& out_imag, ShaderResourceViewPtr const& in_real, ShaderResourceViewPtr const& in_imag)
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		RenderEngine& re = rf.RenderEngineInstance();

		tex_fb_->Attach(FrameBuffer::Attachment::Color0, rf.Make2DRtv(out_real, 0, 1, 0));
		tex_fb_->Attach(FrameBuffer::Attachment::Color1, rf.Make2DRtv(out_imag, 0, 1, 0));

		FrameBufferPtr old_fb = re.CurFrameBuffer();
		re.BindFrameBuffer(FrameBufferPtr());

		*real_tex_ep_ = in_real;
		*imag_tex_ep_ = in_imag;
		
		uint32_t const thread_count = 3 * (width_ * height_) / 8;
		uint32_t ostride = width_ * height_ / 8;
		uint32_t istride = ostride;
		uint32_t istride3 = height_ / 8;
		uint32_t pstride = width_;
		float phase_base = -PI2 / (width_ * height_);

		*(effect_->ParameterByName("thread_count")) = thread_count;

		// X direction
		
		*(effect_->ParameterByName("ostride")) = ostride;
		*(effect_->ParameterByName("pstride")) = pstride;

		*(effect_->ParameterByName("istride")) = istride;
		*(effect_->ParameterByName("istride3")) = uint2(0, istride3);
		*(effect_->ParameterByName("phase_base")) = phase_base;
		this->Radix008A(tmp_buffer_uav_, src_srv_, thread_count, istride, true);

		ShaderResourceViewPtr srvs[2] = { dst_srv_, tmp_buffer_srv_ };
		UnorderedAccessViewPtr uavs[2] = { dst_uav_, tmp_buffer_uav_ };
		int index = 0;

		uint32_t t = width_;
		while (t > 8)
		{
			istride /= 8;
			istride3 /= 8;
			phase_base *= 8.0f;
			*(effect_->ParameterByName("istride")) = istride;
			*(effect_->ParameterByName("istride3")) = uint2(0, istride3);
			*(effect_->ParameterByName("phase_base")) = phase_base;
			this->Radix008A(uavs[index], srvs[!index], thread_count, istride, false);
			index = !index;

			t /= 8;
		}

		// Y direction
		
		ostride = height_ / 8;
		pstride = 1;
		*(effect_->ParameterByName("ostride")) = ostride;
		*(effect_->ParameterByName("pstride")) = pstride;

		istride /= 8;
		istride3 /= 8;
		phase_base *= 8.0f;
		*(effect_->ParameterByName("istride")) = istride;
		*(effect_->ParameterByName("istride3")) = uint2(istride3, 0);
		*(effect_->ParameterByName("phase_base")) = phase_base;
		this->Radix008A(uavs[index], srvs[!index], thread_count, istride, false);
		index = !index;

		t = height_;
		while (t > 8)
		{
			istride /= 8;
			istride3 /= 8;
			phase_base *= 8.0f;
			*(effect_->ParameterByName("istride")) = istride;
			*(effect_->ParameterByName("istride3")) = uint2(istride3, 0);
			*(effect_->ParameterByName("phase_base")) = phase_base;
			this->Radix008A(uavs[index], srvs[!index], thread_count, istride, false);
			index = !index;

			t /= 8;
		}


		re.BindFrameBuffer(tex_fb_);

		re.Render(*effect_, *buf2tex_tech_, *quad_layout_);

		re.BindFrameBuffer(old_fb);
	}

	void GpuFftCS4::Radix008A(UnorderedAccessViewPtr const & dst,
					ShaderResourceViewPtr const & src,
					uint32_t thread_count, uint32_t istride, bool first)
	{
		// Setup execution configuration
		uint32_t grid = (thread_count + COHERENCY_GRANULARITY - 1) / COHERENCY_GRANULARITY;

		// Buffers
		*(effect_->ParameterByName("src_data")) = src;
		*(effect_->ParameterByName("dst_data")) = dst;

		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		RenderEngine& re = rf.RenderEngineInstance();

		// Shader
		if (first)
		{
			re.Dispatch(*effect_, *radix008a_first_tech_, grid, 1, 1);
		}
		else
		{
			if (istride > 1)
			{
				re.Dispatch(*effect_, *radix008a_tech_, grid, 1, 1);
			}
			else
			{
				re.Dispatch(*effect_, *radix008a_final_tech_, grid, 1, 1);
			}
		}
	}


	GpuFftCS5::GpuFftCS5(uint32_t width, uint32_t height, bool forward)
			: width_(width), height_(height), forward_(forward)
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();

		for (uint32_t i = 0; i < 2; ++i)
		{
			tmp_real_tex_[i] = rf.MakeTexture2D(width, height, 1, 1, EF_ABGR32F, 1, 0, EAH_GPU_Read | EAH_GPU_Unordered);
			tmp_real_srv_[i] = rf.MakeTextureSrv(tmp_real_tex_[i]);

			tmp_imag_tex_[i] = rf.MakeTexture2D(width, height, 1, 1, EF_ABGR32F, 1, 0, EAH_GPU_Read | EAH_GPU_Unordered);
			tmp_imag_srv_[i] = rf.MakeTextureSrv(tmp_imag_tex_[i]);
		}

		effect_ = SyncLoadRenderEffect("FFT.fxml");
		radix008a_tech_ = effect_->TechniqueByName("FFTRadix008A5");
		radix008a_final_x_tech_ = effect_->TechniqueByName("FFTRadix008AFinalX5");
		radix008a_final_y_tech_ = effect_->TechniqueByName("FFTRadix008AFinalY5");

		*(effect_->ParameterByName("tex_width_height")) = uint2(width - 1, height - 1);

		*(effect_->ParameterByName("forward")) = static_cast<int32_t>(forward_);
		*(effect_->ParameterByName("scale")) = 1.0f / (width_ * height_);
	}

	void GpuFftCS5::Execute(
		TexturePtr const& out_real, TexturePtr const& out_imag, ShaderResourceViewPtr const& in_real, ShaderResourceViewPtr const& in_imag)
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		RenderEngine& re = rf.RenderEngineInstance();

		FrameBufferPtr old_fb = re.CurFrameBuffer();
		re.BindFrameBuffer(FrameBufferPtr());

		int index = 0;

		// X direction

		{
			uint32_t istride = width_ / 8;
			float phase_base = -PI2 / width_;

			*(effect_->ParameterByName("ostride2")) = uint2(width_ / 8, 0);
			*(effect_->ParameterByName("iscale2")) = uint2(8, 1);

			*(effect_->ParameterByName("istride2")) = uint4(istride, 0, istride - 1, static_cast<uint32_t>(-1));
			*(effect_->ParameterByName("phase_base2")) = phase_base;
			this->Radix008A(tmp_real_tex_[index], tmp_imag_tex_[index], in_real, in_imag, width_ / 8, height_, 1 == istride, false);
			index = !index;

			uint32_t t = width_;
			while (t > 8)
			{
				istride /= 8;
				phase_base *= 8;
				*(effect_->ParameterByName("istride2")) = uint4(istride, 0, istride - 1, static_cast<uint32_t>(-1));
				*(effect_->ParameterByName("phase_base2")) = phase_base;
				this->Radix008A(tmp_real_tex_[index], tmp_imag_tex_[index], tmp_real_srv_[!index], tmp_imag_srv_[!index], width_ / 8, height_, 1 == istride, false);
				index = !index;

				t /= 8;
			}
		}

		// Y direction
		
		{
			uint32_t istride = height_ / 8;
			float phase_base = -PI2 / height_;

			*(effect_->ParameterByName("ostride2")) = uint2(0, height_ / 8);
			*(effect_->ParameterByName("iscale2")) = uint2(1, 8);

			*(effect_->ParameterByName("istride2")) = uint4(0, istride, static_cast<uint32_t>(-1), istride - 1);
			*(effect_->ParameterByName("phase_base2")) = phase_base;
			if (1 == istride)
			{
				this->Radix008A(out_real, out_imag, tmp_real_srv_[!index], tmp_imag_srv_[!index], width_, height_ / 8, false, 1 == istride);
			}
			else
			{
				this->Radix008A(tmp_real_tex_[index], tmp_imag_tex_[index], tmp_real_srv_[!index], tmp_imag_srv_[!index], width_, height_ / 8, false, 1 == istride);
			}
			index = !index;

			uint32_t t = height_;
			while (t > 8)
			{
				istride /= 8;
				phase_base *= 8;
				*(effect_->ParameterByName("istride2")) = uint4(0, istride, static_cast<uint32_t>(-1), istride - 1);
				*(effect_->ParameterByName("phase_base2")) = phase_base;
				if (1 == istride)
				{
					this->Radix008A(out_real, out_imag, tmp_real_srv_[!index], tmp_imag_srv_[!index], width_, height_ / 8, false, 1 == istride);
				}
				else
				{
					this->Radix008A(tmp_real_tex_[index], tmp_imag_tex_[index], tmp_real_srv_[!index], tmp_imag_srv_[!index], width_, height_ / 8, false, 1 == istride);
				}
				index = !index;

				t /= 8;
			}
		}

		re.BindFrameBuffer(old_fb);
	}

	void GpuFftCS5::Radix008A(TexturePtr const& dst_real_tex, TexturePtr const& dst_imag_tex, ShaderResourceViewPtr const& src_real_srv,
		ShaderResourceViewPtr const& src_imag_srv, uint32_t thread_x, uint32_t thread_y, bool final_pass_x, bool final_pass_y)
	{
		// Setup execution configuration
		uint32_t grid_x = (thread_x + BLOCK_SIZE_X - 1) / BLOCK_SIZE_X;
		uint32_t grid_y = (thread_y + BLOCK_SIZE_Y - 1) / BLOCK_SIZE_Y;

		// Buffers
		*(effect_->ParameterByName("src_real_tex")) = src_real_srv;
		*(effect_->ParameterByName("src_imag_tex")) = src_imag_srv;
		*(effect_->ParameterByName("dst_real_tex")) = dst_real_tex;
		*(effect_->ParameterByName("dst_imag_tex")) = dst_imag_tex;

		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		RenderEngine& re = rf.RenderEngineInstance();

		// Shader
		RenderTechnique* tech;
		if (final_pass_x)
		{
			tech = radix008a_final_x_tech_;
		}
		else
		{
			if (final_pass_y && !forward_)
			{
				tech = radix008a_final_y_tech_;
			}
			else
			{
				tech = radix008a_tech_;
			}
		}
		re.Dispatch(*effect_, *tech, grid_x, grid_y, 1);
	}
}
