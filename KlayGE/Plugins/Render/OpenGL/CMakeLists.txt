if (KLAYGE_PLATFORM_WINDOWS_STORE OR KLAYGE_PLATFORM_ANDROID OR KLAYGE_PLATFORM_IOS)
	return()
endif()

SET(LIB_NAME KlayGE_RenderEngine_OpenGL)

ADD_LIBRARY(${LIB_NAME} SHARED
	Source/OGLFence.cpp
	Source/OGLFence.hpp
	Source/OGLFrameBuffer.cpp
	Source/OGLFrameBuffer.hpp
	Source/OGLGraphicsBuffer.cpp
	Source/OGLGraphicsBuffer.hpp
	Source/OGLQuery.cpp
	Source/OGLQuery.hpp
	Source/OGLRenderEngine.cpp
	Source/OGLRenderEngine.hpp
	Source/OGLRenderFactory.cpp
	Source/OGLRenderFactory.hpp
	Source/OGLRenderLayout.cpp
	Source/OGLRenderLayout.hpp
	Source/OGLRenderStateObject.cpp
	Source/OGLRenderStateObject.hpp
	Source/OGLRenderView.cpp
	Source/OGLRenderView.hpp
	Source/OGLRenderWindow.cpp
	Source/OGLRenderWindow.hpp
	Source/OGLShaderObject.cpp
	Source/OGLShaderObject.hpp
	Source/OGLTexture.cpp
	Source/OGLTexture.hpp
	Source/OGLTexture1D.cpp
	Source/OGLTexture2D.cpp
	Source/OGLTexture3D.cpp
	Source/OGLTextureCube.cpp
	Source/OGLUtil.cpp
	Source/OGLUtil.hpp
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
if(KLAYGE_IS_DEV_PLATFORM)
	target_link_libraries(${LIB_NAME}
		PRIVATE
			DXBC2GLSLLib
	)
endif()

ADD_DEPENDENCIES(AllInEngine ${LIB_NAME})
