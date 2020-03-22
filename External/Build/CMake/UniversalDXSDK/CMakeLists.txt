set(UNIVERSALDXSDK_PROJECT_DIR "${EXTERNAL_PROJECT_DIR}/UniversalDXSDK")

UpdateExternalLib("UniversalDXSDK" "https://github.com/gongminmin/UniversalDXSDK.git" "fa30d7ba92ef8959be46f040ce418adfcdd9098c")

add_subdirectory(${UNIVERSALDXSDK_PROJECT_DIR} "${CMAKE_CURRENT_BINARY_DIR}/UniversalDXSDK" EXCLUDE_FROM_ALL)
