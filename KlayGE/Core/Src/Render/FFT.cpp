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
#include <KlayGE/half.hpp>

#include <boost/assert.hpp>

#include <KlayGE/FFT.hpp>

namespace KlayGE
{
	GPUFFT::GPUFFT(uint32_t width, uint32_t height, bool forward)
		: width_(width), height_(height), forward_(forward)
	{
		BOOST_ASSERT(0 == (width_ & (width_ - 1)));
		BOOST_ASSERT(0 == (height_ & (height_ - 1)));

		log_x_ = static_cast<uint32_t>(log(static_cast<float>(width_)) / log(2.0f));
		log_y_ = static_cast<uint32_t>(log(static_cast<float>(height_)) / log(2.0f));

		lookup_i_wr_wi_x_tex_.resize(log_x_);
		lookup_i_wr_wi_y_tex_.resize(log_y_);

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
			lookup_i_wr_wi_x_tex_[i] = rf.MakeTexture2D(width_, 1, 1, 1, EF_ABGR16F, 1, 0, EAH_GPU_Read | EAH_Immutable, &init_data);
			ptr += width_ * 4;
		}

		ptr = &lookup_i_wr_wi_y[0];
		for (uint32_t i = 0; i < log_y_; ++ i)
		{
			ElementInitData init_data;
			
			init_data.data = ptr;
			init_data.row_pitch = sizeof(half) * 4;
			init_data.slice_pitch = init_data.row_pitch * height_;
			lookup_i_wr_wi_y_tex_[i] = rf.MakeTexture2D(1, height_, 1, 1, EF_ABGR16F, 1, 0, EAH_GPU_Read | EAH_Immutable, &init_data);
			ptr += height_ * 4;
		}

		tmp_real_tex_[0] = rf.MakeTexture2D(width_, height_, 1, 1, EF_ABGR16F, 1, 0, EAH_GPU_Read | EAH_GPU_Write, NULL);
		tmp_real_tex_[1] = rf.MakeTexture2D(width_, height_, 1, 1, EF_ABGR16F, 1, 0, EAH_GPU_Read | EAH_GPU_Write, NULL);
		tmp_imag_tex_[0] = rf.MakeTexture2D(width_, height_, 1, 1, EF_ABGR16F, 1, 0, EAH_GPU_Read | EAH_GPU_Write, NULL);
		tmp_imag_tex_[1] = rf.MakeTexture2D(width_, height_, 1, 1, EF_ABGR16F, 1, 0, EAH_GPU_Read | EAH_GPU_Write, NULL);

		fft_x_pp_ = LoadPostProcess(ResLoader::Instance().Open("FFT.ppml"), "fft_x");
		fft_y_pp_ = LoadPostProcess(ResLoader::Instance().Open("FFT.ppml"), "fft_y");
	}

	void GPUFFT::Execute(TexturePtr const & out_real, TexturePtr const & out_imag,
			TexturePtr const & in_real, TexturePtr const & in_imag)
	{
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
				fft_x_pp_->InputPin(0, tmp_real_tex_[active]);
				fft_x_pp_->InputPin(1, tmp_imag_tex_[active]);
			}
			fft_x_pp_->InputPin(2, lookup_i_wr_wi_x_tex_[i]);
			fft_x_pp_->OutputPin(0, tmp_real_tex_[!active]);
			fft_x_pp_->OutputPin(1, tmp_imag_tex_[!active]);
			fft_x_pp_->Apply();

			active = !active;
		}

		for (uint32_t i = 0; i < log_y_; ++ i)
		{
			fft_y_pp_->InputPin(0, tmp_real_tex_[active]);
			fft_y_pp_->InputPin(1, tmp_imag_tex_[active]);
			fft_y_pp_->InputPin(2, lookup_i_wr_wi_y_tex_[i]);
			if (log_y_ - 1 == i)
			{
				if (!forward_)
				{
					fft_y_pp_->SetParam(0, 1.0f / (width_ * height_));
				}
				else
				{
					fft_y_pp_->SetParam(0, -1.0f);
				}
				fft_y_pp_->OutputPin(0, out_real);
				fft_y_pp_->OutputPin(1, out_imag);
			}
			else
			{
				fft_y_pp_->SetParam(0, -1.0f);
				fft_y_pp_->OutputPin(0, tmp_real_tex_[!active]);
				fft_y_pp_->OutputPin(1, tmp_imag_tex_[!active]);
			}
			fft_y_pp_->Apply();

			active = !active;
		}
	}
	
	int GPUFFT::BitReverse(int i, int n)
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

	void GPUFFT::ComputeWeight(float& wr, float& wi, int n, int k)
	{
		wr = +cosf(2 * PI * k / n);
		wi = -sinf(2 * PI * k / n);

		wi = forward_ ? wi : -wi;
	}

	void GPUFFT::CreateButterflyLookups(std::vector<half>& lookup_i_wr_wi, int log_n, int n)
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
}
