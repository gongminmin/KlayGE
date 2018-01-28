SET(LIB_NAME vorbis)

SET(LIBVORBIS_SOURCE_FILES
	${VORBIS_PROJECT_DIR}/lib/analysis.c
	${VORBIS_PROJECT_DIR}/lib/bitrate.c
	${VORBIS_PROJECT_DIR}/lib/block.c
	${VORBIS_PROJECT_DIR}/lib/codebook.c
	${VORBIS_PROJECT_DIR}/lib/envelope.c
	${VORBIS_PROJECT_DIR}/lib/floor0.c
	${VORBIS_PROJECT_DIR}/lib/floor1.c
	${VORBIS_PROJECT_DIR}/lib/info.c
	${VORBIS_PROJECT_DIR}/lib/lookup.c
	${VORBIS_PROJECT_DIR}/lib/lpc.c
	${VORBIS_PROJECT_DIR}/lib/lsp.c
	${VORBIS_PROJECT_DIR}/lib/mapping0.c
	${VORBIS_PROJECT_DIR}/lib/mdct.c
	${VORBIS_PROJECT_DIR}/lib/psy.c
	${VORBIS_PROJECT_DIR}/lib/registry.c
	${VORBIS_PROJECT_DIR}/lib/res0.c
	${VORBIS_PROJECT_DIR}/lib/sharedbook.c
	${VORBIS_PROJECT_DIR}/lib/smallft.c
	${VORBIS_PROJECT_DIR}/lib/synthesis.c
	${VORBIS_PROJECT_DIR}/lib/vorbisenc.c
	${VORBIS_PROJECT_DIR}/lib/window.c
)
SET(LIBVORBIS_HEADER_FILES
	${VORBIS_PROJECT_DIR}/lib/backends.h
	${VORBIS_PROJECT_DIR}/lib/bitrate.h
	${VORBIS_PROJECT_DIR}/lib/codebook.h
	${VORBIS_PROJECT_DIR}/include/vorbis/codec.h
	${VORBIS_PROJECT_DIR}/lib/codec_internal.h
	${VORBIS_PROJECT_DIR}/lib/envelope.h
	${VORBIS_PROJECT_DIR}/lib/modes/floor_all.h
	${VORBIS_PROJECT_DIR}/lib/books/floor/floor_books.h
	${VORBIS_PROJECT_DIR}/lib/highlevel.h
	${VORBIS_PROJECT_DIR}/lib/lookup.h
	${VORBIS_PROJECT_DIR}/lib/lookup_data.h
	${VORBIS_PROJECT_DIR}/lib/lpc.h
	${VORBIS_PROJECT_DIR}/lib/lsp.h
	${VORBIS_PROJECT_DIR}/lib/masking.h
	${VORBIS_PROJECT_DIR}/lib/mdct.h
	${VORBIS_PROJECT_DIR}/lib/misc.h
	${VORBIS_PROJECT_DIR}/lib/os.h
	${VORBIS_PROJECT_DIR}/lib/psy.h
	${VORBIS_PROJECT_DIR}/lib/modes/psych_11.h
	${VORBIS_PROJECT_DIR}/lib/modes/psych_16.h
	${VORBIS_PROJECT_DIR}/lib/modes/psych_44.h
	${VORBIS_PROJECT_DIR}/lib/modes/psych_8.h
	${VORBIS_PROJECT_DIR}/lib/registry.h
	${VORBIS_PROJECT_DIR}/lib/books/coupled/res_books_stereo.h
	${VORBIS_PROJECT_DIR}/lib/books/uncoupled/res_books_uncoupled.h
	${VORBIS_PROJECT_DIR}/lib/modes/residue_16.h
	${VORBIS_PROJECT_DIR}/lib/modes/residue_44.h
	${VORBIS_PROJECT_DIR}/lib/modes/residue_44u.h
	${VORBIS_PROJECT_DIR}/lib/modes/residue_8.h
	${VORBIS_PROJECT_DIR}/lib/scales.h
	${VORBIS_PROJECT_DIR}/lib/modes/setup_11.h
	${VORBIS_PROJECT_DIR}/lib/modes/setup_16.h
	${VORBIS_PROJECT_DIR}/lib/modes/setup_22.h
	${VORBIS_PROJECT_DIR}/lib/modes/setup_32.h
	${VORBIS_PROJECT_DIR}/lib/modes/setup_44.h
	${VORBIS_PROJECT_DIR}/lib/modes/setup_44u.h
	${VORBIS_PROJECT_DIR}/lib/modes/setup_8.h
	${VORBIS_PROJECT_DIR}/lib/modes/setup_X.h
	${VORBIS_PROJECT_DIR}/lib/smallft.h
	${VORBIS_PROJECT_DIR}/include/vorbis/vorbisenc.h
	${VORBIS_PROJECT_DIR}/include/vorbis/vorbisfile.h
	${VORBIS_PROJECT_DIR}/lib/window.h
)
SOURCE_GROUP("Source Files" FILES ${LIBVORBIS_SOURCE_FILES})
SOURCE_GROUP("Header Files" FILES ${LIBVORBIS_HEADER_FILES})

ADD_DEFINITIONS(-DLIBVORBIS_EXPORTS)
IF(KLAYGE_PLATFORM_WINDOWS_STORE)
	ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS)
ENDIF()

INCLUDE_DIRECTORIES(${VORBIS_PROJECT_DIR}/include)
INCLUDE_DIRECTORIES(${VORBIS_PROJECT_DIR}/lib)
INCLUDE_DIRECTORIES(${VORBIS_PROJECT_DIR}/../libogg/include)
ADD_LIBRARY(${LIB_NAME} STATIC
	${LIBVORBIS_SOURCE_FILES} ${LIBVORBIS_HEADER_FILES}
)

SET(LIB_OUTPUT_NAME ${LIB_NAME}_static)

SET_TARGET_PROPERTIES(${LIB_NAME} PROPERTIES
	ARCHIVE_OUTPUT_DIRECTORY ${VORBIS_OUTPUT_DIR}
	ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${VORBIS_OUTPUT_DIR}
	ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${VORBIS_OUTPUT_DIR}
	ARCHIVE_OUTPUT_DIRECTORY_RELWITHDEBINFO ${VORBIS_OUTPUT_DIR}
	ARCHIVE_OUTPUT_DIRECTORY_MINSIZEREL ${VORBIS_OUTPUT_DIR}
	PROJECT_LABEL ${LIB_NAME}
	DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX}
	OUTPUT_NAME ${LIB_OUTPUT_NAME}${KLAYGE_OUTPUT_SUFFIX}
)

TARGET_LINK_LIBRARIES(${LIB_NAME})