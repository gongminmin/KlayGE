PROJECT(AndroidNativeAppGlue)

CMAKE_MINIMUM_REQUIRED(VERSION 2.8.6)

IF(COMMAND cmake_policy)
	CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND cmake_policy)

SET(ANAG_DIR "${CMAKE_HOME_DIRECTORY}/../..")
SET(KLAYGE_ROOT_DIR "${ANAG_DIR}/../..")

INCLUDE(${KLAYGE_ROOT_DIR}/cmake/Common.cmake)
INCLUDE(${KLAYGE_ROOT_DIR}/cmake/Platform.cmake)
INCLUDE(${KLAYGE_ROOT_DIR}/cmake/Compiler.cmake)

SET(REL_PATH "External/Downloads/android_native_app_glue.7z")
SET(DST_PATH "${KLAYGE_ROOT_DIR}/External/android_native_app_glue/android_native_app_glue.h")
SET(SEVENZIP_PATH "${KLAYGE_ROOT_DIR}/External/Downloads/${KLAYGE_PLATFORM_NAME}/7z")
SET(PATCH_PATH "${KLAYGE_ROOT_DIR}/External/Downloads/patch.py")

SET(REGEN FALSE)
IF(NOT EXISTS ${DST_PATH})
	SET(REGEN TRUE)
ENDIF()

IF(REGEN)
	IF(KLAYGE_PLATFORM_WINDOWS)
		DOWNLOAD_FILE("External/Downloads/${KLAYGE_PLATFORM_NAME}/7z.exe")
		DOWNLOAD_FILE("External/Downloads/${KLAYGE_PLATFORM_NAME}/7z.dll")
	ELSE()
		DOWNLOAD_FILE("External/Downloads/${KLAYGE_PLATFORM_NAME}/7z")
		DOWNLOAD_FILE("External/Downloads/${KLAYGE_PLATFORM_NAME}/7z.so")
		FILE(COPY ${SEVENZIP_PATH} DESTINATION "${KLAYGE_ROOT_DIR}/External/Downloads")
		FILE(COPY "${KLAYGE_ROOT_DIR}/External/Downloads/7z" DESTINATION "${KLAYGE_ROOT_DIR}/External/Downloads/${KLAYGE_PLATFORM_NAME}/" FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE)
	ENDIF()
	DOWNLOAD_FILE("External/Downloads/patch.py")
	DOWNLOAD_FILE(${REL_PATH})

	EXECUTE_PROCESS(COMMAND "${SEVENZIP_PATH}" "x" "-y" "${KLAYGE_ROOT_DIR}/${REL_PATH}" WORKING_DIRECTORY "${KLAYGE_ROOT_DIR}/External/android_native_app_glue")
ENDIF()
