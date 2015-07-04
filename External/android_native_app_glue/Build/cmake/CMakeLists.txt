PROJECT(AndroidNativeAppGlue)

CMAKE_MINIMUM_REQUIRED(VERSION 2.8.6)

IF(COMMAND cmake_policy)
	CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND cmake_policy)

SET(ANAG_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../..")
SET(KLAYGE_ROOT_DIR "${ANAG_DIR}/../..")

INCLUDE(${KLAYGE_ROOT_DIR}/cmake/Common.cmake)
INCLUDE(${KLAYGE_ROOT_DIR}/cmake/Platform.cmake)
INCLUDE(${KLAYGE_ROOT_DIR}/cmake/Compiler.cmake)
INCLUDE(${KLAYGE_ROOT_DIR}/External/Build/CMake/ExternalCommon.cmake)

SET(REL_PATH "External/Downloads/android_native_app_glue.7z")
SET(SEVENZIP_PATH "${KLAYGE_ROOT_DIR}/External/Downloads/${KLAYGE_PLATFORM_NAME}/7z")
SET(PACKAGE_COMMIT "cbda47a1678ce70b6720856736100979d469e159")

DOWNLOAD_PACKAGE("android_native_app_glue" ${REL_PATH} ${PACKAGE_COMMIT})
