SET(LIB_NAME KlayGE_RenderEngine_OpenGL)

SET(OPENGL_RE_SOURCE_FILES
    ${KLAYGE_PROJECT_DIR}/Plugins/Src/Render/OpenGL/OGLFrameBuffer.cpp
    ${KLAYGE_PROJECT_DIR}/Plugins/Src/Render/OpenGL/OGLGraphicsBuffer.cpp
    ${KLAYGE_PROJECT_DIR}/Plugins/Src/Render/OpenGL/OGLMapping.cpp
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
)

SET(OPENGL_RE_HEADER_FILES
    ${KLAYGE_PROJECT_DIR}/Plugins/Include/KlayGE/OpenGL/OGLFrameBuffer.hpp
    ${KLAYGE_PROJECT_DIR}/Plugins/Include/KlayGE/OpenGL/OGLGraphicsBuffer.hpp
    ${KLAYGE_PROJECT_DIR}/Plugins/Include/KlayGE/OpenGL/OGLMapping.hpp
    ${KLAYGE_PROJECT_DIR}/Plugins/Include/KlayGE/OpenGL/OGLQuery.hpp
    ${KLAYGE_PROJECT_DIR}/Plugins/Include/KlayGE/OpenGL/OGLRenderEngine.hpp
    ${KLAYGE_PROJECT_DIR}/Plugins/Include/KlayGE/OpenGL/OGLRenderFactory.hpp
    ${KLAYGE_PROJECT_DIR}/Plugins/Include/KlayGE/OpenGL/OGLRenderFactoryInternal.hpp
    ${KLAYGE_PROJECT_DIR}/Plugins/Include/KlayGE/OpenGL/OGLRenderLayout.hpp
    ${KLAYGE_PROJECT_DIR}/Plugins/Include/KlayGE/OpenGL/OGLRenderStateObject.hpp
    ${KLAYGE_PROJECT_DIR}/Plugins/Include/KlayGE/OpenGL/OGLRenderView.hpp
    ${KLAYGE_PROJECT_DIR}/Plugins/Include/KlayGE/OpenGL/OGLRenderWindow.hpp
    ${KLAYGE_PROJECT_DIR}/Plugins/Include/KlayGE/OpenGL/OGLShaderObject.hpp
    ${KLAYGE_PROJECT_DIR}/Plugins/Include/KlayGE/OpenGL/OGLTexture.hpp
)

SOURCE_GROUP("Source Files" FILES ${OPENGL_RE_SOURCE_FILES})
SOURCE_GROUP("Header Files" FILES ${OPENGL_RE_HEADER_FILES})

ADD_DEFINITIONS(-DKLAYGE_BUILD_DLL -DKLAYGE_OGL_RE_SOURCE)
ADD_DEFINITIONS(${Boost_LIB_DIAGNOSTIC_DEFINITIONS})

INCLUDE_DIRECTORIES(${KLAYGE_PROJECT_DIR}/Core/Include)
INCLUDE_DIRECTORIES(${KLAYGE_PROJECT_DIR}/Plugins/Include
                    ${Boost_INCLUDE_DIRS} ${OPENGL_INCLUDE_DIR}
                    ${CG_INCLUDE_PATH} ${GLLOADER_INCLUDE_DIR})
LINK_DIRECTORIES(${Boost_LIBRARY_DIRS}
                 ${CG_LIBRARY_DIR} ${GLLOADER_LIBRARY_DIR}
                 ${CMAKE_BINARY_DIR}/Core)  # for autolinking under MSVC

ADD_LIBRARY(${LIB_NAME} SHARED
    ${OPENGL_RE_SOURCE_FILES} ${OPENGL_RE_HEADER_FILES}
)
ADD_DEPENDENCIES(${LIB_NAME} ${KLAYGE_CORELIB_NAME})

SET_TARGET_PROPERTIES(${LIB_NAME} PROPERTIES PROJECT_LABEL "${LIB_NAME}")
SET_TARGET_PROPERTIES(${LIB_NAME} PROPERTIES DEBUG_POSTFIX "${CMAKE_DEBUG_POSTFIX}")
SET_TARGET_PROPERTIES(${LIB_NAME} PROPERTIES OUTPUT_NAME "${LIB_NAME}${KLAYGE_NAME_POSTFIX}")

IF(MSVC)
    SET(EXTRA_LINKED_LIBRARIES "")
ELSE(MSVC)
    SET(EXTRA_LINKED_LIBRARIES
        boost_system ${CG_LIBRARY} ${CG_GL_LIBRARY})
ENDIF(MSVC)

TARGET_LINK_LIBRARIES(${LIB_NAME}
    debug ${GLLOADER_LIBRARY_DEBUG} optimized ${GLLOADER_LIBRARY}
    ${OPENGL_LIBRARIES}
    ${EXTRA_LINKED_LIBRARIES}
)

INSTALL(TARGETS ${LIB_NAME}
    RUNTIME DESTINATION ${KLAYGE_INSTALL_BINDIR}/Render
    LIBRARY DESTINATION ${KLAYGE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${KLAYGE_INSTALL_ARCHIVEDIR}
)
