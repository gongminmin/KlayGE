set(UNIVERSALDXSDK_PROJECT_DIR "${EXTERNAL_PROJECT_DIR}/UniversalDXSDK")

UpdateExternalLib("UniversalDXSDK" "https://github.com/gongminmin/UniversalDXSDK.git" "810cbdcec04156c9ca3323f2a51b2d7eb84a6f4e")

add_library(UniversalDXSDK INTERFACE)
target_include_directories(UniversalDXSDK
	INTERFACE
		${UNIVERSALDXSDK_PROJECT_DIR}/Include
)
