#ifndef _FFT_HPP
#define _FFT_HPP

#include <KlayGE/PreDeclare.hpp>

namespace KlayGE
{
	//Memory access coherency (in threads)
	#define COHERENCY_GRANULARITY 128


	///////////////////////////////////////////////////////////////////////////////
	// Common types
	///////////////////////////////////////////////////////////////////////////////

	struct CSFFT_Plan
	{
		int width;
		int height;

		RenderEffectPtr fft_effect;
		RenderTechniquePtr radix008a_tech;
		RenderTechniquePtr radix008a_tech2;

		// More than one array can be transformed at same time
		uint32_t slices;

		// Temporary buffers
		GraphicsBufferPtr tmp_buffer;
	};

	////////////////////////////////////////////////////////////////////////////////
	// Common constants
	////////////////////////////////////////////////////////////////////////////////
	float const TWO_PI = 6.283185307179586476925286766559f;

	#define FFT_DIMENSIONS 3U
	#define FFT_PLAN_SIZE_LIMIT (1U << 27)

	#define FFT_FORWARD -1
	#define FFT_INVERSE 1


	void fft_create_plan(CSFFT_Plan* plan, uint32_t width, uint32_t height, uint32_t slices);
	void fft_destroy_plan(CSFFT_Plan* plan);

	void fft_c2c(CSFFT_Plan* fft_plan, 
						 GraphicsBufferPtr const & dst,
						 GraphicsBufferPtr const & src);
}

#endif		//  _FFT_HPP
