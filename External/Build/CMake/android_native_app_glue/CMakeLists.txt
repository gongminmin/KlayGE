SET(ANAG_PROJECT_DIR "${EXTERNAL_PROJECT_DIR}/android_native_app_glue")
SET(ANAG_OUTPUT_DIR "${EXTERNAL_PROJECT_DIR}/lib/android_native_app_glue/${KLAYGE_PLATFORM_NAME}")

UpdateExternalLib("android_native_app_glue" "https://github.com/gongminmin/android_native_app_glue.git" "7fafa2cccb91e9ebfa53bf750bd33f84aa426e4d")

SET(ANAG_SOURCES
	${ANAG_PROJECT_DIR}/android_native_app_glue.c
	${ANAG_PROJECT_DIR}/android_native_app_glue.h
)

ADD_LIBRARY(android_native_app_glue ${ANAG_SOURCES})

target_include_directories(android_native_app_glue
	PUBLIC
		${ANAG_PROJECT_DIR}
)

SET_TARGET_PROPERTIES(android_native_app_glue PROPERTIES
	ARCHIVE_OUTPUT_DIRECTORY ${ANAG_OUTPUT_DIR}
	ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${ANAG_OUTPUT_DIR}
	ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${ANAG_OUTPUT_DIR}
	ARCHIVE_OUTPUT_DIRECTORY_RELWITHDEBINFO ${ANAG_OUTPUT_DIR}
	ARCHIVE_OUTPUT_DIRECTORY_MINSIZEREL ${ANAG_OUTPUT_DIR}
	PROJECT_LABEL android_native_app_glue
	DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX}
	OUTPUT_NAME android_native_app_glue${KLAYGE_OUTPUT_SUFFIX}
	FOLDER "${EXTERNAL_VS_FOLDER_PREFIX}/android_native_app_glue"
)
