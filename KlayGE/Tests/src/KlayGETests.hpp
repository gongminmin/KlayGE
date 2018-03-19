#if defined(KLAYGE_COMPILER_CLANGC2)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations" // Ignore POSIX function declaration
#endif
#include <gtest/gtest.h>
#if defined(KLAYGE_COMPILER_CLANGC2)
#pragma clang diagnostic pop
#endif

namespace KlayGE
{
	bool CompareBuffer(GraphicsBuffer& buff0, uint32_t buff0_offset,
		GraphicsBuffer& buff1, uint32_t buff1_offset,
		uint32_t num_elems, float tolerance);

	bool Compare2D(Texture& tex0, uint32_t tex0_array_index, uint32_t tex0_level, uint32_t tex0_x_offset, uint32_t tex0_y_offset,
		Texture& tex1, uint32_t tex1_array_index, uint32_t tex1_level, uint32_t tex1_x_offset, uint32_t tex1_y_offset,
		uint32_t width, uint32_t height, float tolerance);
}
