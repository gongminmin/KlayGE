SET(HEADER_FILES
	${KLAYGE_PROJECT_DIR}/Tools/src/FXMLJIT/OfflineD3D11ShaderObject.hpp
	${KLAYGE_PROJECT_DIR}/Tools/src/FXMLJIT/OfflineD3D12ShaderObject.hpp
	${KLAYGE_PROJECT_DIR}/Tools/src/FXMLJIT/OfflineOGLESShaderObject.hpp
	${KLAYGE_PROJECT_DIR}/Tools/src/FXMLJIT/OfflineOGLShaderObject.hpp
	${KLAYGE_PROJECT_DIR}/Tools/src/FXMLJIT/OfflineRenderEffect.hpp
	${KLAYGE_PROJECT_DIR}/Tools/src/FXMLJIT/OfflineShaderObject.hpp
)

SET(SOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Tools/src/FXMLJIT/FXMLJIT.cpp
	${KLAYGE_PROJECT_DIR}/Tools/src/FXMLJIT/OfflineD3D11ShaderObject.cpp
	${KLAYGE_PROJECT_DIR}/Tools/src/FXMLJIT/OfflineD3D12ShaderObject.cpp
	${KLAYGE_PROJECT_DIR}/Tools/src/FXMLJIT/OfflineOGLESShaderObject.cpp
	${KLAYGE_PROJECT_DIR}/Tools/src/FXMLJIT/OfflineOGLShaderObject.cpp
	${KLAYGE_PROJECT_DIR}/Tools/src/FXMLJIT/OfflineRenderEffect.cpp
	${KLAYGE_PROJECT_DIR}/Tools/src/FXMLJIT/OfflineShaderObject.cpp
)

IF(KLAYGE_PLATFORM_DARWIN OR KLAYGE_PLATFORM_LINUX)
	FIND_PATH(WINE_PATH "winegcc" #searching for 'wine' may result in include path
		PATHS /usr/local/bin opt/local/sbin /usr/bin /bin /usr/sbin /sbin /usr/local/bin)

	IF(NOT WINE_PATH)
		MESSAGE(FATAL_ERROR "Cound NOT find wine")
	ENDIF()
	ADD_DEFINITIONS(-DWINE_PATH="\\"${WINE_PATH}/\\"")
ENDIF()

IF(NOT KLAYGE_COMPILER_MSVC)
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unknown-pragmas -Wno-comment -Wno-switch")
ENDIF()

SET(EXTRA_INCLUDE_DIRS ${EXTRA_INCLUDE_DIRS}
	${KLAYGE_PROJECT_DIR}/Plugins/Include
	${KLAYGE_PROJECT_DIR}/../glloader/include
	${KLAYGE_PROJECT_DIR}/../DXBC2GLSL/Include
	${KLAYGE_PROJECT_DIR}/../External/UniversalDXSDK/Include)
SET(EXTRA_LINKED_DIRS ${EXTRA_LINKED_DIRS}
	${KLAYGE_PROJECT_DIR}/../DXBC2GLSL/lib/${KLAYGE_PLATFORM_NAME})
IF(NOT KLAYGE_COMPILER_MSVC)
	SET(FS_LIB ${Boost_FILESYSTEM_LIBRARY})
	IF(KLAYGE_COMPILER_GCC AND (KLAYGE_COMPILER_VERSION STRGREATER "60"))
		SET(FS_LIB "stdc++fs")
	ENDIF()
	SET(EXTRA_LINKED_LIBRARIES ${EXTRA_LINKED_LIBRARIES}
		debug DXBC2GLSLLib${KLAYGE_OUTPUT_SUFFIX}_d optimized DXBC2GLSLLib${KLAYGE_OUTPUT_SUFFIX}
		${FS_LIB})
ENDIF()

SETUP_TOOL(FXMLJIT)
