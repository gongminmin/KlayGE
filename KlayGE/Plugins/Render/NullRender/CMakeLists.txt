if (NOT KLAYGE_IS_DEV_PLATFORM)
	return()
endif()

option(KLAYGE_BUILD_PLUGIN_NULL_RENDER_ENGINE "Build Null render engine plugin" ON)
if(NOT KLAYGE_BUILD_PLUGIN_NULL_RENDER_ENGINE)
	return()
endif()

ADD_LIBRARY(KlayGE_RenderEngine_NullRender SHARED
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
	target_compile_options(KlayGE_RenderEngine_NullRender
		PRIVATE
			-Wno-unknown-pragmas
	)
endif()

SET_TARGET_PROPERTIES(KlayGE_RenderEngine_NullRender PROPERTIES
	OUTPUT_NAME KlayGE_RenderEngine_NullRender${KLAYGE_OUTPUT_SUFFIX}
	FOLDER "KlayGE/Engine/Plugins/Render"
)
if(KLAYGE_PREFERRED_LIB_TYPE STREQUAL "SHARED")
	set_target_properties(KlayGE_RenderEngine_NullRender PROPERTIES
		CXX_VISIBILITY_PRESET hidden
		VISIBILITY_INLINES_HIDDEN ON
	)
endif()

target_precompile_headers(KlayGE_RenderEngine_NullRender
	PRIVATE
		"${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/KlayGE.hpp"
)

target_link_libraries(KlayGE_RenderEngine_NullRender
	PRIVATE
		KlayGE_Core
		glloader
)
if(KLAYGE_PLATFORM_WINDOWS)
	target_link_libraries(KlayGE_RenderEngine_NullRender
		PRIVATE
			dxsdk
	)
endif()
if(KLAYGE_IS_DEV_PLATFORM)
	target_link_libraries(KlayGE_RenderEngine_NullRender
		PRIVATE
			DXBC2GLSLLib
	)
endif()

ADD_DEPENDENCIES(AllInEngine KlayGE_RenderEngine_NullRender)
