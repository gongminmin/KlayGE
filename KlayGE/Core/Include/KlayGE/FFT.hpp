// FFT.hpp
// KlayGE Fast Fourier Transform header file
// Ver 4.1.0
// Copyright(C) Minmin Gong, 2012
// Homepage: http://www.klayge.org
//
// 4.1.0
// First release (2012.4.11)
//
// CHANGE LIST
//////////////////////////////////////////////////////////////////////////////////

#ifndef _FFT_HPP
#define _FFT_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>

namespace KlayGE
{
	class KLAYGE_CORE_API GpuFft : boost::noncopyable
	{
	public:
		virtual ~GpuFft()
		{
		}

		virtual void Execute(TexturePtr const & out_real, TexturePtr const & out_imag,
			TexturePtr const & in_real, TexturePtr const & in_imag) = 0;
	};

	class KLAYGE_CORE_API GpuFftPS : public GpuFft
	{
	public:
		GpuFftPS(uint32_t width, uint32_t height, bool forward);

		void Execute(TexturePtr const & out_real, TexturePtr const & out_imag,
			TexturePtr const & in_real, TexturePtr const & in_imag);

	private:
		int BitReverse(int i, int n);
		void ComputeWeight(float& wr, float& wi, int n, int k);
		void CreateButterflyLookups(std::vector<half>& lookup_i_wr_wi, int log_n, int n);

	private:
		uint32_t width_, height_;
		bool forward_;

		uint32_t log_x_, log_y_;

		std::vector<TexturePtr> lookup_i_wr_wi_x_tex_;
		std::vector<TexturePtr> lookup_i_wr_wi_y_tex_;

		TexturePtr tmp_real_tex_[2];
		TexturePtr tmp_imag_tex_[2];

		PostProcessPtr fft_x_pp_;
		PostProcessPtr fft_y_pp_;
	};

	class KLAYGE_CORE_API GpuFftCS4 : public GpuFft
	{
	public:
		GpuFftCS4(uint32_t width, uint32_t height, bool forward);

		void Execute(TexturePtr const & out_real, TexturePtr const & out_imag,
			TexturePtr const & in_real, TexturePtr const & in_imag);

	private:
		void Radix008A(UnorderedAccessViewPtr const & dst,
					ShaderResourceViewPtr const & src,
					uint32_t thread_count, uint32_t istride, bool first);

	private:
		GraphicsBufferPtr src_;
		UnorderedAccessViewPtr src_uav_;
		ShaderResourceViewPtr src_srv_;
		GraphicsBufferPtr dst_;
		UnorderedAccessViewPtr dst_uav_;
		ShaderResourceViewPtr dst_srv_;
		RenderLayoutPtr quad_layout_;
		FrameBufferPtr tex_fb_;
		GraphicsBufferPtr tmp_buffer_;
		UnorderedAccessViewPtr tmp_buffer_uav_;
		ShaderResourceViewPtr tmp_buffer_srv_;
		
		RenderEffectPtr effect_;
		RenderTechnique* radix008a_tech_;
		RenderTechnique* radix008a_first_tech_;
		RenderTechnique* radix008a_final_tech_;
		RenderTechnique* buf2tex_tech_;
		RenderEffectParameter* real_tex_ep_;
		RenderEffectParameter* imag_tex_ep_;

		uint32_t width_, height_;
		bool forward_;
	};

	class KLAYGE_CORE_API GpuFftCS5 : public GpuFft
	{
	public:
		GpuFftCS5(uint32_t width, uint32_t height, bool forward);

		void Execute(TexturePtr const & out_real, TexturePtr const & out_imag,
			TexturePtr const & in_real, TexturePtr const & in_imag);

	private:
		void Radix008A(TexturePtr const & dst_real_tex, TexturePtr const & dst_imag_tex,
					TexturePtr const & src_real_tex, TexturePtr const & src_imag_tex,
					uint32_t thread_x, uint32_t thread_y, bool final_pass_x, bool final_pass_y);

	private:
		TexturePtr tmp_real_tex_[2];
		TexturePtr tmp_imag_tex_[2];
		
		RenderEffectPtr effect_;
		RenderTechnique* radix008a_tech_;
		RenderTechnique* radix008a_final_x_tech_;
		RenderTechnique* radix008a_final_y_tech_;

		uint32_t width_, height_;
		bool forward_;
	};
}

#endif		// _FFT_HPP
