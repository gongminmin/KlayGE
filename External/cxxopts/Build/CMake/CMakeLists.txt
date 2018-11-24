PROJECT(cxxopts NONE)

CMAKE_MINIMUM_REQUIRED(VERSION 2.8.6)

SET(CXXOPTS_PROJECT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../..")
SET(KLAYGE_ROOT_DIR "${CXXOPTS_PROJECT_DIR}/../..")

INCLUDE(${KLAYGE_ROOT_DIR}/CMake/Common.cmake)
INCLUDE(${KLAYGE_ROOT_DIR}/CMake/Platform.cmake)
INCLUDE(${KLAYGE_ROOT_DIR}/External/Build/CMake/ExternalCommon.cmake)

UpdateExternalLib("cxxopts" "cxxopts" "git://github.com/jarro2783/cxxopts.git" "3f2d70530219e09fe7e563f86126b0d3b228a60d")
