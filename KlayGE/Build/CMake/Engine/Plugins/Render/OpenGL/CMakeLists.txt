if (KLAYGE_PLATFORM_WINDOWS_STORE OR KLAYGE_PLATFORM_ANDROID OR KLAYGE_PLATFORM_IOS)
	return()
endif()

SET(LIB_NAME KlayGE_RenderEngine_OpenGL)

SET(OPENGL_RE_SOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Plugins/Src/Render/OpenGL/OGLFence.cpp
	${KLAYGE_PROJECT_DIR}/Plugins/Src/Render/OpenGL/OGLFrameBuffer.cpp
	${KLAYGE_PROJECT_DIR}/Plugins/Src/Render/OpenGL/OGLGraphicsBuffer.cpp
	${KLAYGE_PROJECT_DIR}/Plugins/Src/Render/OpenGL/OGLQuery.cpp
	${KLAYGE_PROJECT_DIR}/Plugins/Src/Render/OpenGL/OGLRenderEngine.cpp
	${KLAYGE_PROJECT_DIR}/Plugins/Src/Render/OpenGL/OGLRenderFactory.cpp
	${KLAYGE_PROJECT_DIR}/Plugins/Src/Render/OpenGL/OGLRenderLayout.cpp
	${KLAYGE_PROJECT_DIR}/Plugins/Src/Render/OpenGL/OGLRenderStateObject.cpp
	${KLAYGE_PROJECT_DIR}/Plugins/Src/Render/OpenGL/OGLRenderView.cpp
	${KLAYGE_PROJECT_DIR}/Plugins/Src/Render/OpenGL/OGLRenderWindow.cpp
	${KLAYGE_PROJECT_DIR}/Plugins/Src/Render/OpenGL/OGLShaderObject.cpp
	${KLAYGE_PROJECT_DIR}/Plugins/Src/Render/OpenGL/OGLTexture.cpp
	${KLAYGE_PROJECT_DIR}/Plugins/Src/Render/OpenGL/OGLTexture1D.cpp
	${KLAYGE_PROJECT_DIR}/Plugins/Src/Render/OpenGL/OGLTexture2D.cpp
	${KLAYGE_PROJECT_DIR}/Plugins/Src/Render/OpenGL/OGLTexture3D.cpp
	${KLAYGE_PROJECT_DIR}/Plugins/Src/Render/OpenGL/OGLTextureCube.cpp
	${KLAYGE_PROJECT_DIR}/Plugins/Src/Render/OpenGL/OGLUtil.cpp
)

SET(OPENGL_RE_HEADER_FILES
	${KLAYGE_PROJECT_DIR}/Plugins/Include/KlayGE/OpenGL/OGLFence.hpp
	${KLAYGE_PROJECT_DIR}/Plugins/Include/KlayGE/OpenGL/OGLFrameBuffer.hpp
	${KLAYGE_PROJECT_DIR}/Plugins/Include/KlayGE/OpenGL/OGLGraphicsBuffer.hpp
	${KLAYGE_PROJECT_DIR}/Plugins/Include/KlayGE/OpenGL/OGLQuery.hpp
	${KLAYGE_PROJECT_DIR}/Plugins/Include/KlayGE/OpenGL/OGLRenderEngine.hpp
	${KLAYGE_PROJECT_DIR}/Plugins/Include/KlayGE/OpenGL/OGLRenderFactory.hpp
	${KLAYGE_PROJECT_DIR}/Plugins/Include/KlayGE/OpenGL/OGLRenderLayout.hpp
	${KLAYGE_PROJECT_DIR}/Plugins/Include/KlayGE/OpenGL/OGLRenderStateObject.hpp
	${KLAYGE_PROJECT_DIR}/Plugins/Include/KlayGE/OpenGL/OGLRenderView.hpp
	${KLAYGE_PROJECT_DIR}/Plugins/Include/KlayGE/OpenGL/OGLRenderWindow.hpp
	${KLAYGE_PROJECT_DIR}/Plugins/Include/KlayGE/OpenGL/OGLShaderObject.hpp
	${KLAYGE_PROJECT_DIR}/Plugins/Include/KlayGE/OpenGL/OGLTexture.hpp
	${KLAYGE_PROJECT_DIR}/Plugins/Include/KlayGE/OpenGL/OGLUtil.hpp
)

SOURCE_GROUP("Source Files" FILES ${OPENGL_RE_SOURCE_FILES})
SOURCE_GROUP("Header Files" FILES ${OPENGL_RE_HEADER_FILES})

ADD_LIBRARY(${LIB_NAME} SHARED
	${OPENGL_RE_SOURCE_FILES} ${OPENGL_RE_HEADER_FILES}
)

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
if(KLAYGE_IS_DEV_PLATFORM)
	target_link_libraries(${LIB_NAME}
		PRIVATE
			DXBC2GLSLLib
	)
endif()

ADD_DEPENDENCIES(AllInEngine ${LIB_NAME})
