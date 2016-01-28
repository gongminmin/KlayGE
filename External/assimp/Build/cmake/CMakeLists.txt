PROJECT( Assimp )

CMAKE_MINIMUM_REQUIRED(VERSION 2.8.6)

set(LINK_SEARCH_START_STATIC TRUE)

# Define here the needed parameters
set (ASSIMP_VERSION_MAJOR 3)
set (ASSIMP_VERSION_MINOR 2)
set (ASSIMP_VERSION_PATCH 0) # subversion revision?
set (ASSIMP_VERSION ${ASSIMP_VERSION_MAJOR}.${ASSIMP_VERSION_MINOR}.${ASSIMP_VERSION_PATCH})
set (ASSIMP_SOVERSION 3)
set (PROJECT_VERSION "${ASSIMP_VERSION}")

set(ASSIMP_PACKAGE_VERSION "0" CACHE STRING "the package-specific version used for uploading the sources")

SET(ASSIMP_PROJECT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../..")
SET(KLAYGE_ROOT_DIR "${ASSIMP_PROJECT_DIR}/../..")

INCLUDE(${KLAYGE_ROOT_DIR}/cmake/Common.cmake)
INCLUDE(${KLAYGE_ROOT_DIR}/cmake/Platform.cmake)
INCLUDE(${KLAYGE_ROOT_DIR}/cmake/Compiler.cmake)
INCLUDE(${KLAYGE_ROOT_DIR}/External/Build/CMake/ExternalCommon.cmake)

SET_PROPERTY(GLOBAL PROPERTY USE_FOLDERS ON)

SET(ASSIMP_OUTPUT_DIR "${ASSIMP_PROJECT_DIR}/lib/${KLAYGE_PLATFORM_NAME}")
SET(KLAYGE_BIN_DIR "${KLAYGE_ROOT_DIR}/KlayGE/bin/${KLAYGE_PLATFORM_NAME}")
SET(REL_PATH "External/Downloads/assimp-${ASSIMP_VERSION_MAJOR}.${ASSIMP_VERSION_MINOR}.patched.7z")
SET(SEVENZIP_PATH "${KLAYGE_ROOT_DIR}/External/Downloads/${KLAYGE_HOST_PLATFORM_NAME}/7z")
SET(PACKAGE_COMMIT_ID "6416293e007a55770ebe373a6f3ab98bc7d1cb49")
SET(PACKAGE_FILE_ID "fdfe386654f719dfb9f2b579f0950274daa96de1")

DOWNLOAD_PACKAGE("assimp" ${REL_PATH} ${PACKAGE_COMMIT_ID} ${PACKAGE_FILE_ID} "Readme.md")

IF(MSVC)
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /W3 /WX-")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W3 /WX-")
ELSE()
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-error")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-error")
ENDIF()

# Needed for openddl_parser config, no use of c++11 at this moment
add_definitions( -DOPENDDL_NO_USE_CPP11 )

SET(GIT_COMMIT_HASH 0)

configure_file(
  ${ASSIMP_PROJECT_DIR}/revision.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/revision.h
)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(CMAKE_MODULE_PATH       "${ASSIMP_PROJECT_DIR}/cmake-modules" )
set(LIBASSIMP_COMPONENT     "libassimp${ASSIMP_VERSION_MAJOR}.${ASSIMP_VERSION_MINOR}.${ASSIMP_VERSION_PATCH}" )
set(LIBASSIMP-DEV_COMPONENT "libassimp${ASSIMP_VERSION_MAJOR}.${ASSIMP_VERSION_MINOR}.${ASSIMP_VERSION_PATCH}-dev" )
set(CPACK_COMPONENTS_ALL assimp-bin ${LIBASSIMP_COMPONENT} ${LIBASSIMP-DEV_COMPONENT} assimp-dev)
set(ASSIMP_LIBRARY_SUFFIX "" CACHE STRING "Suffix to append to library names")

# Workaround to be able to deal with compiler bug "Too many sections" with mingw.
if( CMAKE_COMPILER_IS_MINGW )
  ADD_DEFINITIONS(-DASSIMP_BUILD_NO_IFC_IMPORTER )
endif()

if((CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX) AND NOT CMAKE_COMPILER_IS_MINGW)
  # hide all not-exported symbols
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden -Wall" )
elseif(MSVC)
  # enable multi-core compilation with MSVC
  add_definitions(/MP)
endif()

INCLUDE_DIRECTORIES( ${ASSIMP_PROJECT_DIR}/include )

INCLUDE (PrecompiledHeader)

SET( CMAKE_LIBRARY_OUTPUT_DIRECTORY "${ASSIMP_OUTPUT_DIR}" )
SET( CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${ASSIMP_OUTPUT_DIR}" )
SET( CMAKE_RUNTIME_OUTPUT_DIRECTORY "${ASSIMP_PROJECT_DIR}/bin" )

# Cache these to allow the user to override them manually.
SET( ASSIMP_LIB_INSTALL_DIR "lib" CACHE PATH
  "Path the built library files are installed to." )
SET( ASSIMP_INCLUDE_INSTALL_DIR "include" CACHE PATH
  "Path the header files are installed to." )
SET( ASSIMP_BIN_INSTALL_DIR "bin" CACHE PATH
  "Path the tool executables are installed to." )

# Globally enable Boost resp. the Boost workaround ¨C it is also needed by the
# tools which include the Assimp headers.
option ( ASSIMP_ENABLE_BOOST_WORKAROUND
  "If a simple implementation of the used Boost functions is used. Slightly reduces functionality, but enables builds without Boost available."
  ON
)
IF ( ASSIMP_ENABLE_BOOST_WORKAROUND )
  INCLUDE_DIRECTORIES( ${ASSIMP_PROJECT_DIR}/code/BoostWorkaround )
  ADD_DEFINITIONS( -DASSIMP_BUILD_BOOST_WORKAROUND )
  MESSAGE( STATUS "Building a non-boost version of Assimp." )
ELSE ( ASSIMP_ENABLE_BOOST_WORKAROUND )
  SET( Boost_DETAILED_FAILURE_MSG ON )
  IF ( NOT Boost_ADDITIONAL_VERSIONS )
    SET( Boost_ADDITIONAL_VERSIONS "1.47" "1.47.0" "1.48.0" "1.48" "1.49" "1.49.0" "1.50" "1.50.0" "1.51" "1.51.0" "1.52.0" "1.53.0" "1.54.0" "1.55" "1.55.0" "1.56" "1.56.0" "1.57" "1.57.0" "1.58" "1.58.0" "1.59" "1.59.0")
  ENDIF ( NOT Boost_ADDITIONAL_VERSIONS )
  FIND_PACKAGE( Boost )
  IF ( NOT Boost_FOUND )
    MESSAGE( FATAL_ERROR
      "Boost libraries (http://www.boost.org/) not found. "
      "You can build a non-boost version of Assimp with slightly reduced "
      "functionality by specifying -DASSIMP_ENABLE_BOOST_WORKAROUND=ON."
    )
  ENDIF ( NOT Boost_FOUND )

  INCLUDE_DIRECTORIES( ${Boost_INCLUDE_DIRS} )
ENDIF ( ASSIMP_ENABLE_BOOST_WORKAROUND )

# cmake configuration files
configure_file("${ASSIMP_PROJECT_DIR}/assimp-config.cmake.in"         "${CMAKE_CURRENT_BINARY_DIR}/assimp-config.cmake" @ONLY IMMEDIATE)
configure_file("${ASSIMP_PROJECT_DIR}/assimp-config-version.cmake.in" "${CMAKE_CURRENT_BINARY_DIR}/assimp-config-version.cmake" @ONLY IMMEDIATE)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/assimp-config.cmake"       "${CMAKE_CURRENT_BINARY_DIR}/assimp-config-version.cmake" DESTINATION "${ASSIMP_LIB_INSTALL_DIR}/cmake/assimp-${ASSIMP_VERSION_MAJOR}.${ASSIMP_VERSION_MINOR}" COMPONENT ${LIBASSIMP-DEV_COMPONENT})

if( CMAKE_COMPILER_IS_GNUCXX )
  set(LIBSTDC++_LIBRARIES -lstdc++)
endif( CMAKE_COMPILER_IS_GNUCXX )

# Search for external dependencies, and build them from source if not found
# Search for zlib
find_package(ZLIB)
if( NOT ZLIB_FOUND )
  message(STATUS "compiling zlib from souces")
  include(CheckIncludeFile)
  include(CheckTypeSize)
  include(CheckFunctionExists)
  # compile from sources
  add_subdirectory(${ASSIMP_PROJECT_DIR}/contrib/zlib "${CMAKE_CURRENT_BINARY_DIR}/contrib/zlib")
  set(ZLIB_FOUND 1)
  set(ZLIB_LIBRARIES zlibstatic)
  set(ZLIB_INCLUDE_DIR ${ASSIMP_PROJECT_DIR}/contrib/zlib ${CMAKE_CURRENT_BINARY_DIR}/contrib/zlib)
else(NOT ZLIB_FOUND)
  ADD_DEFINITIONS(-DASSIMP_BUILD_NO_OWN_ZLIB)
  set(ZLIB_LIBRARIES_LINKED -lz)
endif(NOT ZLIB_FOUND)
INCLUDE_DIRECTORIES(${ZLIB_INCLUDE_DIR})

SET ( ASSIMP_BUILD_ARCHITECTURE "" CACHE STRING
  "describe the current architecture."
)
IF    ( ASSIMP_BUILD_ARCHITECTURE STREQUAL "")
ELSE  ( ASSIMP_BUILD_ARCHITECTURE STREQUAL "")
  ADD_DEFINITIONS ( -D'ASSIMP_BUILD_ARCHITECTURE="${ASSIMP_BUILD_ARCHITECTURE}"' )
ENDIF ( ASSIMP_BUILD_ARCHITECTURE STREQUAL "")

# ${CMAKE_GENERATOR}
SET ( ASSIMP_BUILD_COMPILER "" CACHE STRING
  "describe the current compiler."
)
IF    ( ASSIMP_BUILD_COMPILER STREQUAL "")
ELSE  ( ASSIMP_BUILD_COMPILER STREQUAL "")
  ADD_DEFINITIONS ( -D'ASSIMP_BUILD_COMPILER="${ASSIMP_BUILD_COMPILER}"' )
ENDIF ( ASSIMP_BUILD_COMPILER STREQUAL "")

MARK_AS_ADVANCED ( ASSIMP_BUILD_ARCHITECTURE ASSIMP_BUILD_COMPILER )


SET ( ASSIMP_BUILD_NONFREE_C4D_IMPORTER OFF CACHE BOOL
  "Build the C4D importer, which relies on the non-free Melange SDK." 
)

IF (ASSIMP_BUILD_NONFREE_C4D_IMPORTER)
  IF ( MSVC )
    SET(C4D_INCLUDES "${ASSIMP_PROJECT_DIR}/contrib/Melange/_melange/includes")

    # pick the correct prebuilt library
    IF(MSVC11)
      SET(C4D_LIB_POSTFIX "_2012md")
    ELSEIF(MSVC10)
      SET(C4D_LIB_POSTFIX "_2010md")
    ELSEIF(MSVC90)
      SET(C4D_LIB_POSTFIX "_2008md")
    ELSE()
      MESSAGE( FATAL_ERROR 
        "C4D is currently only supported with MSVC 9, 10, 11"
      )
    ENDIF()
      
    IF(CMAKE_CL_64)
      SET(C4D_LIB_ARCH_POSTFIX "_x64")
    ELSE()
      SET(C4D_LIB_ARCH_POSTFIX "")
    ENDIF()
    
    SET(C4D_LIB_BASE_PATH "${ASSIMP_PROJECT_DIR}/contrib/Melange/_melange/lib/WIN")
    
    SET(C4D_DEBUG_LIBRARY "${C4D_LIB_BASE_PATH}/debug/_melange_lib${C4D_LIB_ARCH_POSTFIX}${C4D_LIB_POSTFIX}.lib")
    SET(C4D_RELEASE_LIBRARY "${C4D_LIB_BASE_PATH}/release/_melange_lib${C4D_LIB_ARCH_POSTFIX}${C4D_LIB_POSTFIX}.lib")
    
    # winsock and winmm are necessary dependencies of melange (this is undocumented, but true.)
    SET(C4D_EXTRA_LIBRARIES WSock32.lib Winmm.lib)
  ELSE ()
    MESSAGE( FATAL_ERROR 
      "C4D is currently only available on Windows with melange SDK installed in contrib/Melange"
    )
  ENDIF ( MSVC )
else (ASSIMP_BUILD_NONFREE_C4D_IMPORTER)
  ADD_DEFINITIONS( -DASSIMP_BUILD_NO_C4D_IMPORTER )
ENDIF (ASSIMP_BUILD_NONFREE_C4D_IMPORTER)



ADD_SUBDIRECTORY( ${ASSIMP_PROJECT_DIR}/code/ "${CMAKE_CURRENT_BINARY_DIR}/code/" )

# Generate a pkg-config .pc for the Assimp library.
CONFIGURE_FILE( "${ASSIMP_PROJECT_DIR}/assimp.pc.in" "${PROJECT_BINARY_DIR}/assimp.pc" @ONLY )
INSTALL( FILES "${PROJECT_BINARY_DIR}/assimp.pc" DESTINATION ${ASSIMP_LIB_INSTALL_DIR}/pkgconfig/ COMPONENT ${LIBASSIMP-DEV_COMPONENT})
