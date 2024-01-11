if (KLAYGE_PLATFORM_WINDOWS_STORE)
	return()
endif()

FIND_PACKAGE(GLES)

IF(NOT KLAYGE_GLES_FOUND)
	return()
ENDIF()

option(KLAYGE_BUILD_PLUGIN_OPENGLES_RENDER_ENGINE "Build OpenGLES render engine plugin" ON)
if(NOT KLAYGE_BUILD_PLUGIN_OPENGLES_RENDER_ENGINE)
	return()
endif()

ADD_LIBRARY(KlayGE_RenderEngine_OpenGLES ${KLAYGE_PREFERRED_LIB_TYPE}
	Source/OGLESFence.cpp
	Source/OGLESFence.hpp
	Source/OGLESFrameBuffer.cpp
	Source/OGLESFrameBuffer.hpp
	Source/OGLESGraphicsBuffer.cpp
	Source/OGLESGraphicsBuffer.hpp
	Source/OGLESQuery.cpp
	Source/OGLESQuery.hpp
	Source/OGLESRenderEngine.cpp
	Source/OGLESRenderEngine.hpp
	Source/OGLESRenderFactory.cpp
	Source/OGLESRenderFactory.hpp
	Source/OGLESRenderLayout.cpp
	Source/OGLESRenderLayout.hpp
	Source/OGLESRenderStateObject.cpp
	Source/OGLESRenderStateObject.hpp
	Source/OGLESRenderView.cpp
	Source/OGLESRenderView.hpp
	Source/OGLESRenderWindow.cpp
	Source/OGLESRenderWindow.hpp
	Source/OGLESShaderObject.cpp
	Source/OGLESShaderObject.hpp
	Source/OGLESTexture.cpp
	Source/OGLESTexture.hpp
	Source/OGLESTexture1D.cpp
	Source/OGLESTexture2D.cpp
	Source/OGLESTexture3D.cpp
	Source/OGLESTextureCube.cpp
	Source/OGLESUtil.cpp
	Source/OGLESUtil.hpp
)

target_compile_definitions(KlayGE_RenderEngine_OpenGLES
	PRIVATE
		-DGLLOADER_GLES_SUPPORT
)

SET_TARGET_PROPERTIES(KlayGE_RenderEngine_OpenGLES PROPERTIES
	OUTPUT_NAME KlayGE_RenderEngine_OpenGLES${KLAYGE_OUTPUT_SUFFIX}
	FOLDER "KlayGE/Engine/Plugins/Render"
)
if(KLAYGE_PREFERRED_LIB_TYPE STREQUAL "SHARED")
	set_target_properties(KlayGE_RenderEngine_OpenGLES PROPERTIES
		CXX_VISIBILITY_PRESET hidden
		VISIBILITY_INLINES_HIDDEN ON
	)
endif()

target_precompile_headers(KlayGE_RenderEngine_OpenGLES
	PRIVATE
		"${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/KlayGE.hpp"
)

target_link_libraries(KlayGE_RenderEngine_OpenGLES
	PRIVATE
		KlayGE_Core
		glloader_es
)
if(KLAYGE_IS_DEV_PLATFORM)
	target_link_libraries(KlayGE_RenderEngine_OpenGLES
		PRIVATE
			DXBC2GLSLLib
	)
endif()

ADD_DEPENDENCIES(AllInEngine KlayGE_RenderEngine_OpenGLES)
