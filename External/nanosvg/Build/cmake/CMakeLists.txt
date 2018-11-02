PROJECT(nanosvg NONE)

CMAKE_MINIMUM_REQUIRED(VERSION 2.8.6)

SET(NANOSVG_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../..")
SET(KLAYGE_ROOT_DIR "${NANOSVG_DIR}/../..")

INCLUDE(${KLAYGE_ROOT_DIR}/cmake/Common.cmake)
INCLUDE(${KLAYGE_ROOT_DIR}/cmake/Platform.cmake)
INCLUDE(${KLAYGE_ROOT_DIR}/External/Build/CMake/ExternalCommon.cmake)

UpdateExternalLib("nanosvg" "nanosvg" "git://github.com/memononen/nanosvg.git" "2b08deeb553c723d151f908d786c64136d26d576")
