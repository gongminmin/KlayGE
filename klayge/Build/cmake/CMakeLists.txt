PROJECT(KlayGE)

SET(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/CMakeModules/")
CMAKE_MINIMUM_REQUIRED(VERSION 2.4.7)

IF(COMMAND cmake_policy)
  CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND cmake_policy)

SET(KLAYGE_MAJOR_VERSION 3)
SET(KLAYGE_MINOR_VERSION 11)
SET(KLAYGE_PATCH_VERSION 0)
SET(KLAYGE_VERSION ${KLAYGE_MAJOR_VERSION}.${KLAYGE_MINOR_VERSION}.${KLAYGE_PATCH_VERSION})

IF(WIN32)
    IF(MSVC)
        OPTION(WIN32_USE_MP "Set to ON to build KlayGE with the /MP option (Visual Studio 2005 and above)." OFF)
        MARK_AS_ADVANCED(WIN32_USE_MP)
        IF(WIN32_USE_MP)
            SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
        ENDIF(WIN32_USE_MP)

        # More MSVC specific compilation flags
        ADD_DEFINITIONS(-D_SCL_SECURE_NO_WARNINGS)
        ADD_DEFINITIONS(-D_CRT_SECURE_NO_DEPRECATE)
        
        SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /wd4996 /wd4355 /wd4244 /wd4985 /wd4267 /wd4619")
        SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4996 /wd4355 /wd4244 /wd4985 /wd4267 /wd4619")
    ENDIF(MSVC)
ENDIF(WIN32)

IF(NOT WIN32)
    SET(CMAKE_CXX_FLAGS "-W -Wall -Wno-unused")
ENDIF(NOT WIN32)

# Set postfix
SET(CMAKE_DEBUG_POSTFIX "_d" CACHE STRING "add a postfix, usually d on windows")
SET(CMAKE_RELEASE_POSTFIX "" CACHE STRING "add a postfix, usually empty on windows")
SET(CMAKE_RELWITHDEBINFO_POSTFIX "_rd" CACHE STRING "add a postfix, usually empty on windows")
SET(CMAKE_MINSIZEREL_POSTFIX "_s" CACHE STRING "add a postfix, usually empty on windows")


# On CMake 2.4.x use EXECUTABLE_OUTPUT_PATH and LIBRARY_OUTPUT_PATH and later
# we work around the DLL placement by use of the PREFIX target property hack
#
# On CMake 2.6.x use the newly minted CMAKE_LIBRARY_OUTPUT_DIRECTORY,
# CMAKE_ARCHIVE_OUTPUT_DIRECTORY & CMAKE_RUNTIME_OUTPUT_DIRECTORY

IF(CMAKE_MAJOR_VERSION EQUAL 2 AND CMAKE_MINOR_VERSION LESS 5)
    SET(EXECUTABLE_OUTPUT_PATH ${OUTPUT_BINDIR})
    SET(LIBRARY_OUTPUT_PATH    ${OUTPUT_LIBDIR})
ELSE()
    # If CMake >= 2.6.0
    SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_LIBDIR})
    SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_BINDIR})
    IF(WIN32)
        SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_BINDIR})
    ELSE(WIN32)
        SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_LIBDIR})
    ENDIF(WIN32)
ENDIF()


# Main settings and dependencies
SET(KLAYGE_CORELIB_NAME KlayGE_Core)  # Dependency of all other targets
FIND_PATH(KLAYGE_PROJECT_DIR klayge_logo.ico PATHS ../../)

SET(3RDPARTY_DIR "$ENV{KLAYGE_HOME}" CACHE PATH "Location of 3rdparty dependencies")
FIND_PACKAGE(OpenGL)
FIND_PACKAGE(Boost 1.40.0 COMPONENTS filesystem system thread signals)
FIND_PACKAGE(PythonLibs)
FIND_PACKAGE(DirectX)
FIND_PACKAGE(DirectShow)
FIND_PACKAGE(Cg)
FIND_PACKAGE(OpenAL)
FIND_PACKAGE(OggVorbis)
FIND_PACKAGE(glloader)

FIND_PATH(RAPIDXML_INCLUDE_DIR rapidxml/rapidxml.hpp
          PATHS ${3RDPARTY_DIR}/include /usr/include /usr/local/include)
GET_FILENAME_COMPONENT( GLLOADER_LIBRARY_DIR "${GLLOADER_LIBRARY}" PATH )
GET_FILENAME_COMPONENT( CG_LIBRARY_DIR "${CG_LIBRARY}" PATH )

INCLUDE_DIRECTORIES(${CMAKE_INCLUDE_PATH} ${RAPIDXML_INCLUDE_DIR})
LINK_DIRECTORIES(${CMAKE_LIBRARY_PATH})

# Target name postfix
SET(KLAYGE_NAME_POSTFIX "")
IF(WIN32)
    IF(MSVC)
        SET(KLAYGE_NAME_POSTFIX "${KLAYGE_NAME_POSTFIX}_vc")
    ELSEIF(CYGWIN)
        SET(KLAYGE_NAME_POSTFIX "${KLAYGE_NAME_POSTFIX}_cyg")
    ELSE()
        SET(KLAYGE_NAME_POSTFIX "${KLAYGE_NAME_POSTFIX}_mgw")  # So, will it surely be mingw now?
    ENDIF()
ELSE(WIN32)
    SET(KLAYGE_NAME_POSTFIX "${KLAYGE_NAME_POSTFIX}_gcc")
ENDIF(WIN32)

IF(CMAKE_CL_64)
    SET(KLAYGE_NAME_POSTFIX "${KLAYGE_NAME_POSTFIX}_x64")
ELSE(CMAKE_CL_64)
    SET(KLAYGE_NAME_POSTFIX "${KLAYGE_NAME_POSTFIX}_x86")
ENDIF(CMAKE_CL_64)


# Output directories
SET(KLAYGE_INSTALL_INCDIR include)
SET(KLAYGE_INSTALL_BINDIR bin)
SET(KLAYGE_INSTALL_LIB_POSTFIX "")
IF(WIN32)
    SET(KLAYGE_INSTALL_LIBDIR bin)
    SET(KLAYGE_INSTALL_ARCHIVEDIR lib)
ELSE(WIN32)
    SET(KLAYGE_INSTALL_LIBDIR lib${KLAYGE_INSTALL_LIB_POSTFIX})
    SET(KLAYGE_INSTALL_ARCHIVEDIR lib${KLAYGE_INSTALL_LIB_POSTFIX})
ENDIF(WIN32)

# Projects
ADD_SUBDIRECTORY(Core)

OPTION(BUILD_WITH_OPENGL "Build with OpenGL rendering support. " ON)
IF(CG_FOUND AND GLLOADER_FOUND AND BUILD_WITH_OPENGL)
    ADD_SUBDIRECTORY(Plugins/Render/OpenGL)
ELSEIF(BUILD_WITH_OPENGL)
    MESSAGE("Please setup Cg and glloader dependencies for OpenGL support")
ENDIF()

OPTION(BUILD_WITH_OPENGL_ES2 "Build with OpenGL ES2 rendering support. " OFF)
IF(BUILD_WITH_OPENGL_ES2)
    # todo: add es2 support
    # ADD_SUBDIRECTORY(Plugins/Render/OpenGLES2)
ENDIF(BUILD_WITH_OPENGL_ES2)

OPTION(BUILD_WITH_OPENAL "Build with OpenAL audio support. " ON)
IF(OPENAL_FOUND AND BUILD_WITH_OPENAL)
    ADD_SUBDIRECTORY(Plugins/Audio/OpenAL)
ELSEIF(BUILD_WITH_OPENAL)
    MESSAGE("Please setup OpenAL dependencies for OpenAL support")
ENDIF()

OPTION(BUILD_WITH_OGGVORBIS "Build with OggVorbis audio datasource support. " ON)
IF(LIBOGG_FOUND AND VORBIS_FOUND AND BUILD_WITH_OGGVORBIS)
    ADD_SUBDIRECTORY(Plugins/Audio/OggVorbis)
ELSEIF(BUILD_WITH_OGGVORBIS)
    MESSAGE("Please setup libogg and vorbis dependencies for OggVorbis support")
ENDIF()

OPTION(BUILD_WITH_OCTREE "Build with OCTree scene management support. " ON)
IF(BUILD_WITH_OCTREE)
    ADD_SUBDIRECTORY(Plugins/Scene/OCTree)
ENDIF(BUILD_WITH_OCTREE)

IF(WIN32)

    OPTION(BUILD_WITH_DSOUND "Build with DirectSound audio support. " ON)
    IF(DSOUND_FOUND AND BUILD_WITH_DSOUND)
        ADD_SUBDIRECTORY(Plugins/Audio/DSound)
    ELSEIF(BUILD_WITH_DSOUND)
        MESSAGE("Please setup DSound dependencies for DirectSound support")
    ENDIF()
    
    OPTION(BUILD_WITH_DINPUT "Build with DirectInput input support. " ON)
    IF(DINPUT_FOUND AND BUILD_WITH_DINPUT)
        ADD_SUBDIRECTORY(Plugins/Input/DInput)
    ELSEIF(BUILD_WITH_DINPUT)
        MESSAGE("Please setup DInput dependencies for DirectInput support")
    ENDIF()
    
    OPTION(BUILD_WITH_DSHOW "Build with DirectShow showing support. " ON)
    IF(DIRECTSHOW_FOUND AND BUILD_WITH_DSHOW)
        ADD_SUBDIRECTORY(Plugins/Show/DShow)
    ELSEIF(BUILD_WITH_DSHOW)
        MESSAGE("Please setup DShow dependencies for DirectShow support")
    ENDIF()
    
    OPTION(BUILD_WITH_D3D11 "Build with DirectX 11 rendering support. " ON)
    IF(D3D11_FOUND AND BUILD_WITH_D3D11)
        ADD_SUBDIRECTORY(Plugins/Render/D3D11)
    ELSEIF(BUILD_WITH_D3D11)
        MESSAGE("Please setup DirectX 11 dependencies for D3D11 support")
    ENDIF()
    
    OPTION(BUILD_WITH_KCONFIG "Build with KGEConfig support. " ON)
    IF(BUILD_WITH_KCONFIG)
        ADD_SUBDIRECTORY(KGEConfig)
    ENDIF(BUILD_WITH_KCONFIG)

ENDIF(WIN32)

OPTION(BUILD_SAMPLES "Build KlayGE samples. " ON)
IF(BUILD_SAMPLES)
    ADD_SUBDIRECTORY(Samples)
ENDIF(BUILD_SAMPLES)


# Uninstalling configuration
CONFIGURE_FILE(
  "${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules/cmake_uninstall.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
  IMMEDIATE @ONLY)
ADD_CUSTOM_TARGET(uninstall
  "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")
