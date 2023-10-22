if (KLAYGE_PLATFORM_WINDOWS_STORE)
	return()
endif()

FIND_PACKAGE(GLES)

IF(NOT KLAYGE_GLES_FOUND)
	return()
ENDIF()

SET(LIB_NAME KlayGE_RenderEngine_OpenGLES)

ADD_LIBRARY(${LIB_NAME} ${KLAYGE_PREFERRED_LIB_TYPE}
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

target_compile_definitions(${LIB_NAME}
	PRIVATE
		-DGLLOADER_GLES_SUPPORT
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
		glloader_es
)
if(KLAYGE_IS_DEV_PLATFORM)
	target_link_libraries(${LIB_NAME}
		PRIVATE
			DXBC2GLSLLib
	)
endif()

ADD_DEPENDENCIES(AllInEngine ${LIB_NAME})
