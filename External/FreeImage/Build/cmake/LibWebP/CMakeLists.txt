SET(LIB_NAME LibWebP)

SET(LIBWEBP_DEC_SOURCE_FILES
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dec/dec.alpha.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dec/dec.buffer.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dec/dec.frame.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dec/dec.idec.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dec/dec.io.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dec/dec.quant.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dec/dec.tree.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dec/dec.vp8.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dec/dec.vp8l.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dec/dec.webp.c
)
SET(LIBWEBP_DEMUX_SOURCE_FILES
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/demux/demux.demux.c
)
SET(LIBWEBP_DSP_SOURCE_FILES
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dsp/dsp.alpha_processing.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dsp/dsp.alpha_processing_mips_dsp_r2.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dsp/dsp.alpha_processing_sse2.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dsp/dsp.argb.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dsp/dsp.argb_mips_dsp_r2.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dsp/dsp.argb_sse2.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dsp/dsp.cost.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dsp/dsp.cost_mips_dsp_r2.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dsp/dsp.cost_mips32.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dsp/dsp.cost_sse2.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dsp/dsp.cpu.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dsp/dsp.dec.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dsp/dsp.dec_clip_tables.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dsp/dsp.dec_mips_dsp_r2.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dsp/dsp.dec_mips32.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dsp/dsp.dec_neon.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dsp/dsp.dec_sse2.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dsp/dsp.enc.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dsp/dsp.enc_avx2.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dsp/dsp.enc_mips_dsp_r2.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dsp/dsp.enc_mips32.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dsp/dsp.enc_neon.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dsp/dsp.enc_sse2.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dsp/dsp.filters.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dsp/dsp.filters_mips_dsp_r2.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dsp/dsp.filters_sse2.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dsp/dsp.lossless.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dsp/dsp.lossless_mips_dsp_r2.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dsp/dsp.lossless_mips32.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dsp/dsp.lossless_neon.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dsp/dsp.lossless_sse2.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dsp/dsp.rescaler.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dsp/dsp.rescaler_mips_dsp_r2.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dsp/dsp.rescaler_mips32.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dsp/dsp.upsampling.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dsp/dsp.upsampling_mips_dsp_r2.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dsp/dsp.upsampling_neon.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dsp/dsp.upsampling_sse2.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dsp/dsp.yuv.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dsp/dsp.yuv_mips_dsp_r2.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dsp/dsp.yuv_mips32.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dsp/dsp.yuv_sse2.c
)
SET(LIBWEBP_ENC_SOURCE_FILES
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/enc/enc.alpha.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/enc/enc.analysis.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/enc/enc.backward_references.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/enc/enc.config.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/enc/enc.cost.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/enc/enc.filter.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/enc/enc.frame.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/enc/enc.histogram.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/enc/enc.iterator.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/enc/enc.near_lossless.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/enc/enc.picture.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/enc/enc.picture_csp.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/enc/enc.picture_psnr.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/enc/enc.picture_rescale.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/enc/enc.picture_tools.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/enc/enc.quant.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/enc/enc.syntax.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/enc/enc.token.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/enc/enc.tree.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/enc/enc.vp8l.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/enc/enc.webpenc.c
)
SET(LIBWEBP_MUX_SOURCE_FILES
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/mux/mux.anim_encode.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/mux/mux.muxedit.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/mux/mux.muxinternal.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/mux/mux.muxread.c
)
SET(LIBWEBP_UTILS_SOURCE_FILES
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/utils/utils.bit_reader.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/utils/utils.bit_writer.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/utils/utils.color_cache.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/utils/utils.filters.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/utils/utils.huffman.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/utils/utils.huffman_encode.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/utils/utils.quant_levels.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/utils/utils.quant_levels_dec.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/utils/utils.random.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/utils/utils.rescaler.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/utils/utils.thread.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/utils/utils.utils.c
)
SET(LIBWEBP_DEC_HEADER_FILES
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dec/alphai.h
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dec/decode_vp8.h
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dec/vp8i.h
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dec/vp8li.h
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dec/webpi.h
)
SET(LIBWEBP_DSP_HEADER_FILES
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dsp/dsp.h
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dsp/lossless.h
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dsp/mips_macro.h
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dsp/neon.h
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dsp/yuv.h
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dsp/yuv_tables_sse2.h
)
SET(LIBWEBP_ENC_HEADER_FILES
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/enc/backward_references.h
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/enc/cost.h
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/enc/histogram.h
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/enc/vp8enci.h
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/enc/vp8li.h
)
SET(LIBWEBP_MUX_HEADER_FILES
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/mux/muxi.h
)
SET(LIBWEBP_UTILS_HEADER_FILES
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/utils/bit_reader.h
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/utils/bit_reader_inl.h
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/utils/bit_writer.h
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/utils/color_cache.h
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/utils/endian_inl.h
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/utils/filters.h
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/utils/huffman.h
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/utils/huffman_encode.h
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/utils/quant_levels.h
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/utils/quant_levels_dec.h
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/utils/random.h
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/utils/rescaler.h
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/utils/thread.h
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/utils/utils.h
)
SET(LIBWEBP_WEBP_HEADER_FILES
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/webp/decode.h
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/webp/demux.h
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/webp/encode.h
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/webp/format_constants.h
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/webp/mux.h
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/webp/mux_types.h
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/webp/types.h
)

SOURCE_GROUP("Source Files\\dec" FILES ${LIBWEBP_DEC_SOURCE_FILES})
SOURCE_GROUP("Source Files\\demux" FILES ${LIBWEBP_DEMUX_SOURCE_FILES})
SOURCE_GROUP("Source Files\\dsp" FILES ${LIBWEBP_DSP_SOURCE_FILES})
SOURCE_GROUP("Source Files\\enc" FILES ${LIBWEBP_ENC_SOURCE_FILES})
SOURCE_GROUP("Source Files\\mux" FILES ${LIBWEBP_MUX_SOURCE_FILES})
SOURCE_GROUP("Source Files\\utils" FILES ${LIBWEBP_UTILS_SOURCE_FILES})
SOURCE_GROUP("Header Files\\dec" FILES ${LIBWEBP_DEC_HEADER_FILES})
SOURCE_GROUP("Header Files\\dsp" FILES ${LIBWEBP_DSP_HEADER_FILES})
SOURCE_GROUP("Header Files\\enc" FILES ${LIBWEBP_ENC_HEADER_FILES})
SOURCE_GROUP("Header Files\\mux" FILES ${LIBWEBP_MUX_HEADER_FILES})
SOURCE_GROUP("Header Files\\utils" FILES ${LIBWEBP_UTILS_HEADER_FILES})
SOURCE_GROUP("Header Files\\webp" FILES ${LIBWEBP_WEBP_HEADER_FILES})

IF(KLAYGE_PLATFORM_WINDOWS)
	ADD_DEFINITIONS(-DWIN32_LEAN_AND_MEAN)
ENDIF()
INCLUDE_DIRECTORIES(${FREEIMAGE_PROJECT_DIR}/Source/src)
ADD_LIBRARY(${LIB_NAME} STATIC
	${LIBWEBP_DEC_SOURCE_FILES} ${LIBWEBP_DEMUX_SOURCE_FILES} ${LIBWEBP_DSP_SOURCE_FILES}
	${LIBWEBP_ENC_SOURCE_FILES} ${LIBWEBP_MUX_SOURCE_FILES} ${LIBWEBP_UTILS_SOURCE_FILES}
	${LIBWEBP_DEC_HEADER_FILES} ${LIBWEBP_DSP_HEADER_FILES} ${LIBWEBP_ENC_HEADER_FILES} ${LIBWEBP_MUX_HEADER_FILES}
	${LIBWEBP_UTILS_HEADER_FILES} ${LIBWEBP_WEBP_HEADER_FILES}
)

SET_TARGET_PROPERTIES(${LIB_NAME} PROPERTIES
	ARCHIVE_OUTPUT_DIRECTORY ${FREEIMAGE_OUTPUT_DIR}
	ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${FREEIMAGE_OUTPUT_DIR}
	ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${FREEIMAGE_OUTPUT_DIR}
	ARCHIVE_OUTPUT_DIRECTORY_RELWITHDEBINFO ${FREEIMAGE_OUTPUT_DIR}
	ARCHIVE_OUTPUT_DIRECTORY_MINSIZEREL ${FREEIMAGE_OUTPUT_DIR}
	PROJECT_LABEL ${LIB_NAME}
	DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX}
	OUTPUT_NAME ${LIB_NAME}${KLAYGE_OUTPUT_SUFFIX}
)

TARGET_LINK_LIBRARIES(${LIB_NAME})
