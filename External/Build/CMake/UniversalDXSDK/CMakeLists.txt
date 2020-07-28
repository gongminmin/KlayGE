set(UNIVERSALDXSDK_PROJECT_DIR "${EXTERNAL_PROJECT_DIR}/UniversalDXSDK")

UpdateExternalLib("UniversalDXSDK" "https://github.com/gongminmin/UniversalDXSDK.git" "e0741e2a142208a1d499446c0dbd42bca98f5337")

add_subdirectory(${UNIVERSALDXSDK_PROJECT_DIR} "${CMAKE_CURRENT_BINARY_DIR}/UniversalDXSDK" EXCLUDE_FROM_ALL)
