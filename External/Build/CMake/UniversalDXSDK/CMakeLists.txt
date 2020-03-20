set(UNIVERSALDXSDK_PROJECT_DIR "${EXTERNAL_PROJECT_DIR}/UniversalDXSDK")

UpdateExternalLib("UniversalDXSDK" "https://github.com/gongminmin/UniversalDXSDK.git" "244dfd4e3929f0a3148e07bce71dbd3edd8608be")

add_subdirectory(${UNIVERSALDXSDK_PROJECT_DIR} "${CMAKE_CURRENT_BINARY_DIR}/UniversalDXSDK" EXCLUDE_FROM_ALL)
