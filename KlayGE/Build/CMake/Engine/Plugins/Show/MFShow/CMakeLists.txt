if (NOT KLAYGE_PLATFORM_WINDOWS)
	return()
endif()

SET(LIB_NAME KlayGE_ShowEngine_MFShow)

SET(MFSHOW_SE_SOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Plugins/Src/Show/MFShow/MFShowEngine.cpp
	${KLAYGE_PROJECT_DIR}/Plugins/Src/Show/MFShow/MFShowFactory.cpp
)

SET(MFSHOW_SE_HEADER_FILES
	${KLAYGE_PROJECT_DIR}/Plugins/Include/KlayGE/MFShow/MFShow.hpp
	${KLAYGE_PROJECT_DIR}/Plugins/Src/Show/MFShow/TinyMFMediaEngine.hpp
)

SOURCE_GROUP("Source Files" FILES ${MFSHOW_SE_SOURCE_FILES})
SOURCE_GROUP("Header Files" FILES ${MFSHOW_SE_HEADER_FILES})

ADD_LIBRARY(${LIB_NAME} SHARED
	${MFSHOW_SE_SOURCE_FILES} ${MFSHOW_SE_HEADER_FILES}
)

if(KLAYGE_COMPILER_GCC)
	# GCC warnings emitted by preprocessor can't be suppressed with pragma GCC diagnostic ignored
	target_compile_options(${LIB_NAME}
		PRIVATE
			-Wno-unknown-pragmas
	)
endif()

target_include_directories(${LIB_NAME}
	PRIVATE
		${KLAYGE_PROJECT_DIR}/Plugins/Include
)

SET_TARGET_PROPERTIES(${LIB_NAME} PROPERTIES
	PROJECT_LABEL ${LIB_NAME}
	OUTPUT_NAME ${LIB_NAME}${KLAYGE_OUTPUT_SUFFIX}
	FOLDER "KlayGE/Engine/Plugins/Show"
)
if(KLAYGE_PREFERRED_LIB_TYPE STREQUAL "SHARED")
	set_target_properties(${LIB_NAME} PROPERTIES
		CXX_VISIBILITY_PRESET hidden
		VISIBILITY_INLINES_HIDDEN ON
	)
endif()

KLAYGE_ADD_PRECOMPILED_HEADER(${LIB_NAME} "${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/KlayGE.hpp")

target_link_libraries(${LIB_NAME}
	PRIVATE
		${KLAYGE_CORELIB_NAME}
		dxsdk
)

ADD_DEPENDENCIES(AllInEngine ${LIB_NAME})
