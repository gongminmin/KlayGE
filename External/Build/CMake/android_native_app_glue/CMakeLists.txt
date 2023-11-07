SET(ANAG_PROJECT_DIR "${EXTERNAL_PROJECT_DIR}/android_native_app_glue")

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
	PROJECT_LABEL android_native_app_glue
	OUTPUT_NAME android_native_app_glue${KLAYGE_OUTPUT_SUFFIX}
	FOLDER "External/android_native_app_glue"
)
