if (NOT KLAYGE_IS_DEV_PLATFORM)
	return()
endif()

SET(LIB_NAME KlayGE_RenderEngine_NullRender)

ADD_LIBRARY(${LIB_NAME} SHARED
	Source/NullRenderEngine.cpp
	Source/NullRenderEngine.hpp
	Source/NullRenderFactory.cpp
	Source/NullRenderFactory.hpp
	Source/NullRenderStateObject.cpp
	Source/NullRenderStateObject.hpp
	Source/NullShaderObject.cpp
	Source/NullShaderObject.hpp
	Source/NullTexture.cpp
	Source/NullTexture.hpp
)

if(KLAYGE_PLATFORM_WINDOWS AND KLAYGE_COMPILER_GCC)
	# GCC warnings emitted by preprocessor can't be suppressed with pragma GCC diagnostic ignored
	target_compile_options(${LIB_NAME}
		PRIVATE
			-Wno-unknown-pragmas
	)
endif()

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
