#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/RenderEngine.hpp>

#include <cstdio>
#include <cstdlib>
#include <cassert>

#include "FFT.hpp"

namespace KlayGE
{
	void radix008a(CSFFT_Plan* fft_plan,
				   GraphicsBufferPtr const & dst,
				   GraphicsBufferPtr const & src,
				   uint32_t thread_count,
				   uint32_t istride)
	{
		// Setup execution configuration
		uint32_t grid = thread_count / COHERENCY_GRANULARITY;

		// Buffers
		*(fft_plan->fft_effect->ParameterByName("src_data")) = src;
		*(fft_plan->fft_effect->ParameterByName("dst_data")) = dst;

		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		RenderEngine& re = rf.RenderEngineInstance();

		// Shader
		if (istride > 1)
		{
			re.Dispatch(*fft_plan->radix008a_tech, grid, 1, 1);
		}
		else
		{
			re.Dispatch(*fft_plan->radix008a_tech2, grid, 1, 1);
		}
	}

	void fft_c2c(CSFFT_Plan* fft_plan,
						 GraphicsBufferPtr const & dst,
						 GraphicsBufferPtr const & src)
	{
		uint32_t const thread_count = fft_plan->slices * (fft_plan->width * fft_plan->height) / 8;
		uint32_t ostride = fft_plan->width * fft_plan->height / 8;
		uint32_t istride = ostride;
		uint32_t pstride = fft_plan->width;
		float phase_base = -TWO_PI / (fft_plan->width * fft_plan->height);

		*(fft_plan->fft_effect->ParameterByName("thread_count")) = thread_count;

		// X direction
		
		*(fft_plan->fft_effect->ParameterByName("ostride")) = ostride;
		*(fft_plan->fft_effect->ParameterByName("pstride")) = pstride;

		*(fft_plan->fft_effect->ParameterByName("istride")) = istride;
		*(fft_plan->fft_effect->ParameterByName("phase_base")) = phase_base;
		radix008a(fft_plan, fft_plan->tmp_buffer, src, thread_count, istride);

		GraphicsBufferPtr buf[2] = { dst, fft_plan->tmp_buffer };
		int index = 0;

		uint32_t t = fft_plan->width;
		while (t > 8)
		{
			istride /= 8;
			phase_base *= 8.0f;
			*(fft_plan->fft_effect->ParameterByName("istride")) = istride;
			*(fft_plan->fft_effect->ParameterByName("phase_base")) = phase_base;
			radix008a(fft_plan, buf[index], buf[!index], thread_count, istride);
			index = !index;

			t /= 8;
		}

		// Y direction
		
		ostride = fft_plan->height / 8;
		pstride = 1;
		*(fft_plan->fft_effect->ParameterByName("ostride")) = ostride;
		*(fft_plan->fft_effect->ParameterByName("pstride")) = pstride;
		
		istride /= 8;
		phase_base *= 8.0f;
		*(fft_plan->fft_effect->ParameterByName("istride")) = istride;
		*(fft_plan->fft_effect->ParameterByName("phase_base")) = phase_base;
		radix008a(fft_plan, buf[index], buf[!index], thread_count, istride);
		index = !index;

		t = fft_plan->height;
		while (t > 8)
		{
			istride /= 8;
			phase_base *= 8.0f;
			*(fft_plan->fft_effect->ParameterByName("istride")) = istride;
			*(fft_plan->fft_effect->ParameterByName("phase_base")) = phase_base;
			radix008a(fft_plan, buf[index], buf[!index], thread_count, istride);
			index = !index;

			t /= 8;
		}
	}

	void fft_create_plan(CSFFT_Plan* plan, uint32_t width, uint32_t height, uint32_t slices)
	{
		plan->width = width;
		plan->height = height;
		plan->slices = slices;

		RenderFactory& rf = Context::Instance().RenderFactoryInstance();

		// Compute shaders
		if (!plan->fft_effect)
		{
			plan->fft_effect = rf.LoadEffect("FFT.fxml");
			plan->radix008a_tech = plan->fft_effect->TechniqueByName("Radix008A");
			plan->radix008a_tech2 = plan->fft_effect->TechniqueByName("Radix008A2");
		}

		// Temp buffer
		plan->tmp_buffer = rf.MakeVertexBuffer(BU_Dynamic, EAH_GPU_Read | EAH_GPU_Unordered | EAH_GPU_Structured, NULL, EF_GR32F);
		plan->tmp_buffer->Resize((plan->height * slices) * plan->width * sizeof(float) * 2);
	}

	void fft_destroy_plan(CSFFT_Plan* plan)
	{
		plan->tmp_buffer.reset();
	}
}
