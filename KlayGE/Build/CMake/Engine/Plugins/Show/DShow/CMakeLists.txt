if (NOT KLAYGE_PLATFORM_WINDOWS_DESKTOP)
	return()
endif()

IF(KLAYGE_COMPILER_MSVC)
	SET(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} /INCREMENTAL:NO")
ENDIF()

SET(LIB_NAME KlayGE_ShowEngine_DShow)

SET(DSHOW_SE_SOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Plugins/Src/Show/DShow/DShowEngine.cpp
	${KLAYGE_PROJECT_DIR}/Plugins/Src/Show/DShow/DShowFactory.cpp
	${KLAYGE_PROJECT_DIR}/Plugins/Src/Show/DShow/DShowVMR9Allocator.cpp
)

SET(DSHOW_SE_HEADER_FILES
	${KLAYGE_PROJECT_DIR}/Plugins/Include/KlayGE/DShow/DShow.hpp
	${KLAYGE_PROJECT_DIR}/Plugins/Include/KlayGE/DShow/DShowVMR9Allocator.hpp
)

SOURCE_GROUP("Source Files" FILES ${DSHOW_SE_SOURCE_FILES})
SOURCE_GROUP("Header Files" FILES ${DSHOW_SE_HEADER_FILES})

ADD_LIBRARY(${LIB_NAME} SHARED
	${DSHOW_SE_SOURCE_FILES} ${DSHOW_SE_HEADER_FILES}
)

if(KLAYGE_COMPILER_GCC)
	# GCC warnings emitted by preprocessor can't be suppressed with pragma GCC diagnostic ignored
	target_compile_options(${LIB_NAME}
		PRIVATE
			-Wno-unknown-pragmas
			-Wno-comment
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
		strmiids ole32 uuid
)

ADD_DEPENDENCIES(AllInEngine ${LIB_NAME})
