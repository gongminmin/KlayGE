PROJECT(rapidxml)

CMAKE_MINIMUM_REQUIRED(VERSION 2.8.6)

IF(COMMAND cmake_policy)
	CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND cmake_policy)

SET(RAPIDXML_DIR "${CMAKE_HOME_DIRECTORY}/../../")
SET(KLAYGE_ROOT_DIR "${RAPIDXML_DIR}/../../")

INCLUDE(${KLAYGE_ROOT_DIR}/cmake/Common.cmake)
INCLUDE(${KLAYGE_ROOT_DIR}/cmake/Platform.cmake)
INCLUDE(${KLAYGE_ROOT_DIR}/cmake/Compiler.cmake)

SET(KLAYGE_BIN_DIR "${RAPIDXML_DIR}/../../KlayGE/bin/${KLAYGE_PLATFORM_NAME}")
SET(RAPIDXML_VER "1.13")
SET(REL_PATH "External/Downloads/rapidxml-${RAPIDXML_VER}.zip")
SET(DST_PATH "${KLAYGE_ROOT_DIR}/External/rapidxml/rapidxml.hpp")
SET(SEVENZIP_PATH "${KLAYGE_ROOT_DIR}/External/Downloads/${KLAYGE_PLATFORM_NAME}/7z")
IF(KLAYGE_PLATFORM_WINDOWS)
	SET(PATCH_PATH "${KLAYGE_ROOT_DIR}/External/Downloads/${KLAYGE_PLATFORM_NAME}/patc")
ELSE()
	SET(PATCH_PATH "patch")
ENDIF()

IF(KLAYGE_PLATFORM_WINDOWS)
	DOWNLOAD_FILE("External/Downloads/${KLAYGE_PLATFORM_NAME}/7z.exe")
	DOWNLOAD_FILE("External/Downloads/${KLAYGE_PLATFORM_NAME}/7z.dll")
	DOWNLOAD_FILE("External/Downloads/${KLAYGE_PLATFORM_NAME}/patc.exe")
ELSE()
	DOWNLOAD_FILE("External/Downloads/${KLAYGE_PLATFORM_NAME}/7z")
	DOWNLOAD_FILE("External/Downloads/${KLAYGE_PLATFORM_NAME}/7z.so")
ENDIF()
DOWNLOAD_FILE_URL(${REL_PATH} "http://downloads.sourceforge.net/project/rapidxml/rapidxml/rapidxml%201.13/rapidxml-1.13.zip")

ADD_CUSTOM_COMMAND(OUTPUT ${DST_PATH}
	COMMAND ${SEVENZIP_PATH} x -y ${KLAYGE_ROOT_DIR}/${REL_PATH}
	COMMAND ${CMAKE_COMMAND} -E copy_directory rapidxml-${RAPIDXML_VER} .
	COMMAND ${CMAKE_COMMAND} -E remove_directory rapidxml-${RAPIDXML_VER}
	COMMAND ${PATCH_PATH} -i Patches/ForGCC.patch
	WORKING_DIRECTORY ${KLAYGE_ROOT_DIR}/External/rapidxml)
ADD_CUSTOM_TARGET(rapidxml ALL DEPENDS ${DST_PATH})
