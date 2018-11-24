SET(LIB_NAME LibWebP)

SET(LIBWEBP_DEC_SOURCE_FILES
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dec/alpha_dec.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dec/buffer_dec.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dec/frame_dec.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dec/idec_dec.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dec/io_dec.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dec/quant_dec.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dec/tree_dec.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dec/vp8_dec.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dec/vp8l_dec.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dec/webp_dec.c
)
SET(LIBWEBP_DEMUX_SOURCE_FILES
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/demux/anim_decode.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/demux/demux.c
)
SET(LIBWEBP_DSP_SOURCE_FILES
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dsp/alpha_processing.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dsp/alpha_processing_mips_dsp_r2.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dsp/alpha_processing_neon.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dsp/alpha_processing_sse2.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dsp/alpha_processing_sse41.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dsp/cost.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dsp/cost_mips_dsp_r2.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dsp/cost_mips32.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dsp/cost_sse2.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dsp/cpu.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dsp/dec.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dsp/dec_clip_tables.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dsp/dec_mips_dsp_r2.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dsp/dec_mips32.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dsp/dec_msa.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dsp/dec_neon.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dsp/dec_sse2.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dsp/dec_sse41.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dsp/enc.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dsp/enc_avx2.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dsp/enc_mips_dsp_r2.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dsp/enc_mips32.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dsp/enc_msa.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dsp/enc_neon.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dsp/enc_sse2.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dsp/enc_sse41.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dsp/filters.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dsp/filters_mips_dsp_r2.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dsp/filters_msa.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dsp/filters_neon.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dsp/filters_sse2.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dsp/lossless.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dsp/lossless_enc.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dsp/lossless_enc_mips_dsp_r2.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dsp/lossless_enc_mips32.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dsp/lossless_enc_msa.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dsp/lossless_enc_neon.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dsp/lossless_enc_sse2.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dsp/lossless_enc_sse41.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dsp/lossless_mips_dsp_r2.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dsp/lossless_msa.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dsp/lossless_neon.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dsp/lossless_sse2.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dsp/rescaler.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dsp/rescaler_mips_dsp_r2.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dsp/rescaler_mips32.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dsp/rescaler_msa.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dsp/rescaler_neon.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dsp/rescaler_sse2.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dsp/ssim.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dsp/ssim_sse2.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dsp/upsampling.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dsp/upsampling_mips_dsp_r2.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dsp/upsampling_msa.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dsp/upsampling_neon.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dsp/upsampling_sse2.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dsp/upsampling_sse41.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dsp/yuv.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dsp/yuv_mips_dsp_r2.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dsp/yuv_mips32.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dsp/yuv_neon.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dsp/yuv_sse2.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dsp/yuv_sse41.c
)
SET(LIBWEBP_ENC_SOURCE_FILES
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/enc/alpha_enc.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/enc/analysis_enc.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/enc/backward_references_cost_enc.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/enc/backward_references_enc.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/enc/config_enc.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/enc/cost_enc.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/enc/filter_enc.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/enc/frame_enc.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/enc/histogram_enc.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/enc/iterator_enc.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/enc/near_lossless_enc.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/enc/picture_csp_enc.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/enc/picture_enc.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/enc/picture_psnr_enc.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/enc/picture_rescale_enc.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/enc/picture_tools_enc.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/enc/predictor_enc.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/enc/quant_enc.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/enc/syntax_enc.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/enc/token_enc.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/enc/tree_enc.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/enc/vp8l_enc.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/enc/webp_enc.c
)
SET(LIBWEBP_MUX_SOURCE_FILES
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/mux/anim_encode.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/mux/muxedit.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/mux/muxinternal.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/mux/muxread.c
)
SET(LIBWEBP_UTILS_SOURCE_FILES
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/utils/bit_reader_utils.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/utils/bit_writer_utils.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/utils/color_cache_utils.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/utils/filters_utils.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/utils/huffman_encode_utils.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/utils/huffman_utils.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/utils/quant_levels_dec_utils.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/utils/quant_levels_utils.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/utils/random_utils.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/utils/rescaler_utils.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/utils/thread_utils.c
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/utils/utils.c
)
SET(LIBWEBP_DEC_HEADER_FILES
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dec/alphai_dec.h
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dec/common_dec.h
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dec/vp8_dec.h
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dec/vp8i_dec.h
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dec/vp8li_dec.h
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dec/webpi_dec.h
)
SET(LIBWEBP_DSP_HEADER_FILES
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dsp/common_sse2.h
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dsp/common_sse41.h
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dsp/dsp.h
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dsp/lossless.h
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dsp/lossless_common.h
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dsp/mips_macro.h
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dsp/msa_macro.h
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dsp/neon.h
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/dsp/yuv.h
)
SET(LIBWEBP_ENC_HEADER_FILES
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/enc/backward_references_enc.h
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/enc/cost_enc.h
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/enc/histogram_enc.h
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/enc/vp8i_enc.h
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/enc/vp8li_enc.h
)
SET(LIBWEBP_MUX_HEADER_FILES
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/mux/animi.h
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/mux/muxi.h
)
SET(LIBWEBP_UTILS_HEADER_FILES
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/utils/bit_reader_inl_utils.h
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/utils/bit_reader_utils.h
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/utils/bit_writer_utils.h
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/utils/color_cache_utils.h
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/utils/endian_inl_utils.h
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/utils/filters_utils.h
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/utils/huffman_encode_utils.h
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/utils/huffman_utils.h
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/utils/quant_levels_dec_utils.h
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/utils/quant_levels_utils.h
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/utils/random_utils.h
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/utils/rescaler_utils.h
	${FREEIMAGE_PROJECT_DIR}/Source/LibWebP/src/utils/thread_utils.h
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
INCLUDE_DIRECTORIES(${FREEIMAGE_PROJECT_DIR}/Source/LibWebP)
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
