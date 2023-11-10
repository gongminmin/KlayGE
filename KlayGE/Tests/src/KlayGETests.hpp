#ifndef KLAYGE_TESTS_HPP
#define KLAYGE_TESTS_HPP

#if defined(KLAYGE_COMPILER_MSVC)
#pragma warning(disable : 6326) // Potential comparison of a constant with another constant.
#endif

#if defined(KLAYGE_COMPILER_CLANGCL)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wshift-sign-overflow"
#if (KLAYGE_COMPILER_VERSION >= 160)
#pragma clang diagnostic ignored "-Wdeprecated" // Ignore out-of-line definition of constexpr
#endif
#endif
#include <gtest/gtest.h>
#if defined(KLAYGE_COMPILER_CLANGCL)
#pragma warning(pop)
#endif

#include <KlayGE/GraphicsBuffer.hpp>
#include <KlayGE/Texture.hpp>

namespace KlayGE
{
	bool CompareBuffer(GraphicsBuffer& buff0, uint32_t buff0_offset,
		GraphicsBuffer& buff1, uint32_t buff1_offset,
		uint32_t num_elems, float tolerance);

	bool Compare2D(Texture& tex0, uint32_t tex0_array_index, uint32_t tex0_level, uint32_t tex0_x_offset, uint32_t tex0_y_offset,
		Texture& tex1, uint32_t tex1_array_index, uint32_t tex1_level, uint32_t tex1_x_offset, uint32_t tex1_y_offset,
		uint32_t width, uint32_t height, float tolerance);
}

#endif	// KLAYGE_TESTS_HPP
