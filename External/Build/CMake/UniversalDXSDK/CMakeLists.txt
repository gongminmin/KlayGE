set(UNIVERSALDXSDK_PROJECT_DIR "${EXTERNAL_PROJECT_DIR}/UniversalDXSDK")

UpdateExternalLib("UniversalDXSDK" "https://github.com/gongminmin/UniversalDXSDK.git" "d5f4f66afd0c30697c2345642756fd980b4b230a")

add_subdirectory(${UNIVERSALDXSDK_PROJECT_DIR} "${CMAKE_CURRENT_BINARY_DIR}/UniversalDXSDK" EXCLUDE_FROM_ALL)
