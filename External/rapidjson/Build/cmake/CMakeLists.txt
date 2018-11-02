PROJECT(rapidjson NONE)

CMAKE_MINIMUM_REQUIRED(VERSION 2.8.6)

SET(RAPIDJSON_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../..")
SET(KLAYGE_ROOT_DIR "${RAPIDJSON_DIR}/../..")

INCLUDE(${KLAYGE_ROOT_DIR}/cmake/Common.cmake)
INCLUDE(${KLAYGE_ROOT_DIR}/cmake/Platform.cmake)
INCLUDE(${KLAYGE_ROOT_DIR}/External/Build/CMake/ExternalCommon.cmake)

UpdateExternalLib("rapidjson" "rapidjson" "git://github.com/Tencent/rapidjson.git" "67fac85e96220f69076121d569abd15471abb6bf")
