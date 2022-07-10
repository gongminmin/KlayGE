if (NOT KLAYGE_IS_DEV_PLATFORM)
	return()
endif()

SET(LIB_NAME KlayGE_RenderEngine_NullRender)

SET(NULL_RE_SOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Plugins/Src/Render/NullRender/NullRenderEngine.cpp
	${KLAYGE_PROJECT_DIR}/Plugins/Src/Render/NullRender/NullRenderFactory.cpp
	${KLAYGE_PROJECT_DIR}/Plugins/Src/Render/NullRender/NullRenderStateObject.cpp
	${KLAYGE_PROJECT_DIR}/Plugins/Src/Render/NullRender/NullShaderObject.cpp
	${KLAYGE_PROJECT_DIR}/Plugins/Src/Render/NullRender/NullTexture.cpp
)

SET(NULL_RE_HEADER_FILES
	${KLAYGE_PROJECT_DIR}/Plugins/Include/KlayGE/NullRender/NullRenderEngine.hpp
	${KLAYGE_PROJECT_DIR}/Plugins/Include/KlayGE/NullRender/NullRenderFactory.hpp
	${KLAYGE_PROJECT_DIR}/Plugins/Include/KlayGE/NullRender/NullRenderStateObject.hpp
	${KLAYGE_PROJECT_DIR}/Plugins/Include/KlayGE/NullRender/NullShaderObject.hpp
	${KLAYGE_PROJECT_DIR}/Plugins/Include/KlayGE/NullRender/NullTexture.hpp
)

SOURCE_GROUP("Source Files" FILES ${NULL_RE_SOURCE_FILES})
SOURCE_GROUP("Header Files" FILES ${NULL_RE_HEADER_FILES})

ADD_LIBRARY(${LIB_NAME} SHARED
	${NULL_RE_SOURCE_FILES} ${NULL_RE_HEADER_FILES}
)

if(KLAYGE_PLATFORM_WINDOWS AND KLAYGE_COMPILER_GCC)
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
	FOLDER "KlayGE/Engine/Plugins/Render"
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
		glloader
)
if(KLAYGE_PLATFORM_WINDOWS)
	target_link_libraries(${LIB_NAME}
		PRIVATE
			dxsdk
	)
endif()
if(KLAYGE_IS_DEV_PLATFORM)
	target_link_libraries(${LIB_NAME}
		PRIVATE
			DXBC2GLSLLib
	)
endif()

ADD_DEPENDENCIES(AllInEngine ${LIB_NAME})
