SET(HEADER_FILES
	${DXBC2GLSL_PROJECT_DIR}/Include/DXBC2GLSL/DXBC.hpp
	${DXBC2GLSL_PROJECT_DIR}/Include/DXBC2GLSL/DXBC2GLSL.hpp
	${DXBC2GLSL_PROJECT_DIR}/Include/DXBC2GLSL/GLSLGen.hpp
	${DXBC2GLSL_PROJECT_DIR}/Include/DXBC2GLSL/Shader.hpp
	${DXBC2GLSL_PROJECT_DIR}/Include/DXBC2GLSL/ShaderDefs.hpp
	${DXBC2GLSL_PROJECT_DIR}/Include/DXBC2GLSL/Utils.hpp
)
SET(SOURCE_FILES
	${DXBC2GLSL_PROJECT_DIR}/Src/DXBC2GLSL.cpp
	${DXBC2GLSL_PROJECT_DIR}/Src/DXBCParse.cpp
	${DXBC2GLSL_PROJECT_DIR}/Src/GLSLGen.cpp
	${DXBC2GLSL_PROJECT_DIR}/Src/ShaderDefs.cpp
	${DXBC2GLSL_PROJECT_DIR}/Src/ShaderParse.cpp
	${DXBC2GLSL_PROJECT_DIR}/Src/Utils.cpp
)

SOURCE_GROUP("Source Files" FILES ${SOURCE_FILES})
SOURCE_GROUP("Header Files" FILES ${HEADER_FILES})

ADD_LIBRARY(DXBC2GLSLLib STATIC
	${SOURCE_FILES} ${HEADER_FILES}
)

target_include_directories(DXBC2GLSLLib
	PUBLIC
		${DXBC2GLSL_PROJECT_DIR}/Include
)

SET_TARGET_PROPERTIES(DXBC2GLSLLib PROPERTIES
	OUTPUT_NAME DXBC2GLSLLib${KLAYGE_OUTPUT_SUFFIX}
	FOLDER "DXBC2GLSL"
)

target_link_libraries(DXBC2GLSLLib
	PUBLIC
		KFL
)
