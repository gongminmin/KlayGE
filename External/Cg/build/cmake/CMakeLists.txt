PROJECT(Cg)

CMAKE_MINIMUM_REQUIRED(VERSION 2.8.6)

IF(COMMAND cmake_policy)
	CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND cmake_policy)

SET(CG_DIR "${CMAKE_HOME_DIRECTORY}/../../")

INCLUDE(${CG_DIR}/../../cmake/Common.cmake)
INCLUDE(${CG_DIR}/../../cmake/Platform.cmake)
INCLUDE(${CG_DIR}/../../cmake/Compiler.cmake)

SET(KLAYGE_BIN_DIR "${CG_DIR}/../../KlayGE/bin/${KLAYGE_PLATFORM_NAME}")
IF(WIN32)
	SET(CG_NAME "cg.dll")
ELSE()
	SET(CG_NAME "cg.so")
ENDIF()
SET(SRC_PATH "${CG_DIR}/bin/${KLAYGE_PLATFORM_NAME}/${CG_NAME}")
SET(DST_PATH "${KLAYGE_BIN_DIR}/${CG_NAME}")

IF(NOT EXISTS ${SRC_PATH})
	SET(URL "http://www.klayge.org/download/KlayGE/External/Cg/bin/${KLAYGE_PLATFORM_NAME}/${CG_NAME}")
	MESSAGE(STATUS "Downloading ${URL}...")
	FILE(DOWNLOAD ${URL} ${SRC_PATH})
ENDIF()

ADD_CUSTOM_COMMAND(OUTPUT ${DST_PATH}
	COMMAND ${CMAKE_COMMAND} -E copy_if_different ${SRC_PATH} ${DST_PATH})
ADD_CUSTOM_TARGET(Redist ALL DEPENDS ${DST_PATH})
