PROJECT(AndroidNativeAppGlue NONE)

CMAKE_MINIMUM_REQUIRED(VERSION 2.8.6)

SET(NDK_VERSION 16b)

SET(ANAG_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../..")
SET(KLAYGE_ROOT_DIR "${ANAG_DIR}/../..")

INCLUDE(${KLAYGE_ROOT_DIR}/cmake/Common.cmake)
INCLUDE(${KLAYGE_ROOT_DIR}/cmake/Platform.cmake)
INCLUDE(${KLAYGE_ROOT_DIR}/External/Build/CMake/ExternalCommon.cmake)

SET(REL_PATH "External/Downloads/android_native_app_glue_${NDK_VERSION}.patched.7z")
SET(SEVENZIP_PATH "${KLAYGE_ROOT_DIR}/External/Downloads/${KLAYGE_HOST_PLATFORM_NAME}/7z")
SET(PACKAGE_COMMIT_ID "fb70edd93469bc6ac7b5b43aab4360277c5cd456")
SET(PACKAGE_FILE_ID "28212add9f8f9bcf2540bb4a2333729bec7a0970")

DOWNLOAD_PACKAGE("android_native_app_glue" ${REL_PATH} ${PACKAGE_COMMIT_ID} ${PACKAGE_FILE_ID} "android_native_app_glue.c")
