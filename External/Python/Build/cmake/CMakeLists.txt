cmake_minimum_required(VERSION 2.8.6)

set(PY_VERSION_MAJOR 2 CACHE STRING "Python major version")
set(PY_VERSION_MINOR 7 CACHE STRING "Python minor version")
set(PY_VERSION_PATCH 11 CACHE STRING "Python patch version")
set(PY_VERSION "${PY_VERSION_MAJOR}.${PY_VERSION_MINOR}.${PY_VERSION_PATCH}")

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_CONFIGURATION_TYPES Release CACHE STRING "Release configuration" FORCE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
    # Set the possible values of build type for cmake-gui
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Release")
endif()

if(APPLE)
  set(_msg "Checking which MACOSX_DEPLOYMENT_TARGET to use")
  message(STATUS "${_msg}")
  include(cmake/PythonAutoDetectOSX.cmake)
  set(MACOSX_DEPLOYMENT_TARGET ${CMAKE_OSX_DEPLOYMENT_TARGET})
  message(STATUS "${_msg} - ${MACOSX_DEPLOYMENT_TARGET}")
endif()

project(Python C)

if(POLICY CMP0042)
    cmake_policy(SET CMP0042 OLD)
endif()

SET(PYTHON_PROJECT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../..")
SET(KLAYGE_ROOT_DIR "${PYTHON_PROJECT_DIR}/../..")

INCLUDE(${KLAYGE_ROOT_DIR}/cmake/Common.cmake)
INCLUDE(${KLAYGE_ROOT_DIR}/cmake/Platform.cmake)
INCLUDE(${KLAYGE_ROOT_DIR}/cmake/Compiler.cmake)
INCLUDE(${KLAYGE_ROOT_DIR}/External/Build/CMake/ExternalCommon.cmake)

SET(REL_PATH "External/Downloads/Python-${PY_VERSION}.patched.7z")
SET(SEVENZIP_PATH "${KLAYGE_ROOT_DIR}/External/Downloads/${KLAYGE_HOST_PLATFORM_NAME}/7z")
SET(PACKAGE_COMMIT_ID "b6f2df28f7a6c28b3ed53c3b409f38c84d42edf7")
SET(PACKAGE_FILE_ID "d59eb9d3bcd4357af48e5af7c641c864e8d71d70")

DOWNLOAD_PACKAGE("Python" ${REL_PATH} ${PACKAGE_COMMIT_ID} ${PACKAGE_FILE_ID} "README")

SET(SRC_DIR ${PYTHON_PROJECT_DIR})

IF(MSVC)
	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /W3 /WX- /wd4273 /we4013 /we4133")
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W3 /WX- /wd4273 /we4013 /we4133")
	IF(MSVC_VERSION GREATER 1800)
		ADD_DEFINITIONS(-D_CRT_NON_CONFORMING_WCSTOK)
	ENDIF()
ELSE()
	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-error")
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-error")
ENDIF()

if(ANDROID OR IOS)
  set(UNIX OFF)
  set(LINUX OFF)
  set(BUILD_PYTHON_EXECUTABLE OFF)
  set(BUILD_PYTHON_DEVELOPMENT OFF)
  set(BUILD_FREEZE_IMPORTLIB OFF)
  set(BUILD_PGEN OFF)
else()
  option(BUILD_PYTHON_EXECUTABLE "Build 'python' executable" ON)
  option(BUILD_PYTHON_DEVELOPMENT "Build files required to develop C extensions" ON)
  option(BUILD_FREEZE_IMPORTLIB "Build '_freeze_importlib' executable" ON)
  option(BUILD_PGEN "Build 'pgen' executable" ON)
endif()

if(CMAKE_CROSSCOMPILING)
    cmake_minimum_required(VERSION 3.3) # Version introducing CROSSCOMPILING_EMULATOR
endif()

# Include helper functions
include(cmake/CMakeChecks.cmake)
include(cmake/Extensions.cmake)
include(CMakeDependentOption)
include(CMakeParseArguments)
if(BUILD_PYTHON_EXECUTABLE)
  include(CTest)
endif()

# Options
option(DOWNLOAD_SOURCES "Automatically download the Python sources" ON)
set(_build_libpython_shared_default  0)
set(_build_libpython_shared_desc "Build libpython as a shared library")

if(DEFINED BUILD_SHARED OR DEFINED BUILD_STATIC)
  message(AUTHOR_WARNING "Deprecated options `BUILD_SHARED` or `BUILD_STATIC` are set, "
                         "please change configure options to use `BUILD_LIBPYTHON_SHARED` instead.")
  if(BUILD_SHARED AND BUILD_STATIC)
    message(AUTHOR_WARNING "Both BUILD_SHARED and BUILD_STATIC options are set to ON. Forcing BUILD_SHARED to OFF")
  endif()
  if(BUILD_STATIC)
    set(_build_libpython_shared_default  0)
    set(_build_libpython_shared_desc "Intialized from `BUILD_STATIC` (deprecated)")
  elseif(BUILD_SHARED)
    set(_build_libpython_shared_default  1)
    set(_build_libpython_shared_desc "Intialized from `BUILD_SHARED` (deprecated)")
  endif()
  message(AUTHOR_WARNING "Unsetting cache variables BUILD_SHARED and BUILD_STATIC")
  unset(BUILD_SHARED CACHE)
  unset(BUILD_STATIC CACHE)
endif()

option(BUILD_LIBPYTHON_SHARED ${_build_libpython_shared_desc} ${_build_libpython_shared_default})
option(BUILD_EXTENSIONS_AS_BUILTIN "Default all modules as builtin to libpython" OFF)
option(USE_LIB64 "Search for dependencies and install to prefix/lib64 instead of prefix/lib" OFF)
if(WIN32)
  option(INSTALL_WINDOWS_TRADITIONAL "Install the executable and extensions in the traditional location" OFF)
  set(_build_wininst_default ON)
  if(CMAKE_CROSSCOMPILING)
    set(_build_wininst_default OFF)
  endif()
  option(BUILD_WININST "Build 'Windows Installer' program for distutils if not already provided in the source tree." ${_build_wininst_default})
  cmake_dependent_option(BUILD_WININST_ALWAYS "Always build 'Windows installer' program even if provided in the source tree." OFF "BUILD_WININST" OFF)
else()
  set(INSTALL_WINDOWS_TRADITIONAL OFF)
  set(BUILD_WININST OFF)
  set(BUILD_WININST_ALWAYS OFF)
endif()
option(INSTALL_DEVELOPMENT "Install files required to develop C extensions" ON)
option(INSTALL_MANUAL "Install man files" ON)
option(INSTALL_TEST "Install test files" ON)

# Append an item to a string if it does not already contain it.  Any additional
# arguments are treated as extra "set" arguments
macro(append_if_absent VARNAME VALUE)
    if(NOT ${VARNAME} MATCHES "(^| )${VALUE}($| )")
        if(${VARNAME})
            set(${VARNAME} "${${VARNAME}} ${VALUE}" ${ARGN})
        else()
            set(${VARNAME} "${VALUE}" ${ARGN})
        endif()
    endif()
endmacro()

option(USE_SYSTEM_LIBRARIES "Use system libraries" ON)

# Set platform dependent defaults
set(_use_builtin_zlib_default OFF)
set(_use_system_zlib_default ON)
if(WIN32)
  set(_use_builtin_zlib_default ON)
  set(_use_system_zlib_default OFF)
endif()

cmake_dependent_option(USE_SYSTEM_BZip2 "Use system BZip2" ON "USE_SYSTEM_LIBRARIES" OFF)
cmake_dependent_option(USE_SYSTEM_Curses "Use system Curses" ON "USE_SYSTEM_LIBRARIES" OFF)
cmake_dependent_option(USE_SYSTEM_EXPAT "Use system EXPAT" ON "USE_SYSTEM_LIBRARIES" OFF)
cmake_dependent_option(USE_SYSTEM_OpenSSL "Use system OpenSSL" ON "USE_SYSTEM_LIBRARIES" OFF)
cmake_dependent_option(USE_SYSTEM_TCL "Use system TCL" ON "USE_SYSTEM_LIBRARIES" OFF)
cmake_dependent_option(USE_SYSTEM_ZLIB "Use system ZLIB" ${_use_system_zlib_default} "USE_SYSTEM_LIBRARIES" OFF)
cmake_dependent_option(USE_SYSTEM_DB "Use system DB" ON "USE_SYSTEM_LIBRARIES" OFF)
cmake_dependent_option(USE_SYSTEM_GDBM "Use system GDBM" ON "USE_SYSTEM_LIBRARIES" OFF)
cmake_dependent_option(USE_SYSTEM_READLINE "Use system READLINE" ON "USE_SYSTEM_LIBRARIES" OFF)
cmake_dependent_option(USE_SYSTEM_SQLITE3 "Use system SQLITE3" ON "USE_SYSTEM_LIBRARIES" OFF)
if(IS_PY3)
  cmake_dependent_option(USE_SYSTEM_LIBMPDEC "Use system LIBMPDEC" ON "USE_SYSTEM_LIBRARIES" OFF)
endif()

cmake_dependent_option(USE_BUILTIN_ZLIB "Use builtin ZLIB" ${_use_builtin_zlib_default} "NOT USE_SYSTEM_ZLIB" OFF)

cmake_dependent_option(USE_LIBEDIT "Use libedit instead of readline" OFF
                       "USE_SYSTEM_READLINE" OFF)

option(WITH_TSC "profile with the Pentium timestamp counter" OFF)
option(ENABLE_IPV6 "Enable IP v6" ON)
option(WITH_DOC_STRINGS "Enable if you want documentation strings in extension modules" ON)
set(Py_DEBUG OFF) # TODO - Build debugging interpreter
option(WITH_PYMALLOC "Define if you want to compile in Python-specific mallocs" ON)
option(WITH_THREAD "Compile in rudimentary thread support" ON)
if(UNIX OR ANDROID OR IOS)
    if (ANDROID OR IOS)
        set(WITH_STATIC_DEPENDENCIES 1)
    else()
        option(WITH_STATIC_DEPENDENCIES "Restrict extensions and external dependencies to static libraries" OFF)
    endif()
    if(WITH_STATIC_DEPENDENCIES)
        if(NOT POLICY CMP0065)
            set(_reason " because option supported only with CMake >= 3.4. Current CMAKE_VERSION [${CMAKE_VERSION}]")
            set(WITH_STATIC_DEPENDENCIES OFF CACHE BOOL "Forced to OFF${_reason}" FORCE)
            message(WARNING "Forcing WITH_STATIC_DEPENDENCIES to OFF${_reason}")
        else()
            cmake_policy(SET CMP0065 NEW)

            set(_reason " because of WITH_STATIC_DEPENDENCIES=ON")

            set(CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_STATIC_LIBRARY_SUFFIX})
            message(STATUS "Setting CMAKE_FIND_LIBRARY_SUFFIXES to \"${CMAKE_STATIC_LIBRARY_SUFFIX}\"${_reason}")

            set(BUILD_EXTENSIONS_AS_BUILTIN ON CACHE BOOL "Forced to ON${_reason}" FORCE)
            message(STATUS "Setting BUILD_EXTENSIONS_AS_BUILTIN to ON${_reason}")

            set(BUILD_LIBPYTHON_SHARED OFF CACHE BOOL "Forced to OFF${_reason}" FORCE)
            message(STATUS "Setting BUILD_LIBPYTHON_SHARED to OFF${_reason}")
        endif()
    endif()
else()
    set(WITH_STATIC_DEPENDENCIES 0)
endif()

# Detect source directory
set(_landmark "pyconfig.h.in") # CMake will look for this file.
if(NOT (SRC_DIR AND EXISTS ${SRC_DIR}/${_landmark}))
    foreach(dirname
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/cpython-${PY_VERSION}
        ${CMAKE_CURRENT_SOURCE_DIR}/Python-${PY_VERSION}
        ${CMAKE_CURRENT_BINARY_DIR}/../cpython-${PY_VERSION}
        ${CMAKE_CURRENT_BINARY_DIR}/../Python-${PY_VERSION})
        set(SRC_DIR ${dirname})
        if(EXISTS ${SRC_DIR}/${_landmark})
            break()
        endif()
    endforeach()
endif()
get_filename_component(SRC_DIR "${SRC_DIR}" ABSOLUTE)

# Download sources
get_filename_component(_parent_dir ${CMAKE_CURRENT_BINARY_DIR} PATH)
string(REGEX REPLACE "rc[1-9]$" "" _py_version_patch_no_rc ${PY_VERSION_PATCH})
set(_py_version_no_rc "${PY_VERSION_MAJOR}.${PY_VERSION_MINOR}.${_py_version_patch_no_rc}")
set(_download_link "http://www.python.org/ftp/python/${_py_version_no_rc}/Python-${PY_VERSION}.tgz")
# Variable below represent the set of supported python version.
set(_download_2.7.3_md5 "2cf641732ac23b18d139be077bd906cd")
set(_download_2.7.4_md5 "592603cfaf4490a980e93ecb92bde44a")
set(_download_2.7.5_md5 "b4f01a1d0ba0b46b05c73b2ac909b1df")
set(_download_2.7.6_md5 "1d8728eb0dfcac72a0fd99c17ec7f386")
set(_download_2.7.7_md5 "cf842800b67841d64e7fb3cd8acb5663")
set(_download_2.7.8_md5 "d4bca0159acb0b44a781292b5231936f")
set(_download_2.7.9_md5 "5eebcaa0030dc4061156d3429657fb83")
set(_download_2.7.10_md5 "d7547558fd673bd9d38e2108c6b42521")
set(_download_2.7.11_md5 "6b6076ec9e93f05dd63e47eb9c15728b")
set(_download_3.5.1_md5 "be78e48cdfc1a7ad90efff146dce6cfe")
set(_extracted_dir "Python-${PY_VERSION}")

if(NOT EXISTS ${SRC_DIR}/${_landmark} AND DOWNLOAD_SOURCES)
    get_filename_component(_filename ${_download_link} NAME)
    set(_archive_filepath ${CMAKE_CURRENT_BINARY_DIR}/../${_filename})
    if(EXISTS "${_archive_filepath}")
        message(STATUS "${_filename} already downloaded")
    else()
        message(STATUS "Downloading ${_download_link}")
        file(
          DOWNLOAD ${_download_link} ${_archive_filepath}
          EXPECTED_MD5 ${_download_${PY_VERSION}_md5}
          SHOW_PROGRESS
          )
    endif()

    message(STATUS "Extracting ${_filename}")
    execute_process(COMMAND ${CMAKE_COMMAND} -E tar xfz ${_archive_filepath}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/..
        RESULT_VARIABLE rv)
    if(NOT rv EQUAL 0)
        message(FATAL_ERROR "error: extraction of '${_filename}' failed")
    endif()
    set(SRC_DIR ${CMAKE_CURRENT_BINARY_DIR}/../${_extracted_dir})
endif()

if(NOT EXISTS ${SRC_DIR}/${_landmark})
    message(FATAL_ERROR "Failed to locate python source.
The searched locations were:
   <CMAKE_CURRENT_SOURCE_DIR>
   <CMAKE_CURRENT_SOURCE_DIR>/cpython-${PY_VERSION}
   <CMAKE_CURRENT_SOURCE_DIR>/Python-${PY_VERSION}
   <CMAKE_CURRENT_BINARY_DIR>/../cpython-${PY_VERSION}
   <CMAKE_CURRENT_BINARY_DIR>/../Python-${PY_VERSION}
   <SRC_DIR>
You could try to:
  1) download ${_download_link}
  2) extract the archive in folder: ${_parent_dir}
  3) Check that file \"${_parent_dir}/${_extracted_dir}/${_landmark}\" exists.
  4) re-configure.
If you already downloaded the source, you could try to re-configure this project passing -DSRC_DIR:PATH=/path/to/Python-{PY_VERSION} using cmake or adding an PATH entry named SRC_DIR from cmake-gui.")
endif()
message(STATUS "SRC_DIR: ${SRC_DIR}")

# Extract version from python source (Copied from FindPythonLibs.cmake)
file(STRINGS "${SRC_DIR}/Include/patchlevel.h" python_version_str
    REGEX "^#define[ \t]+PY_VERSION[ \t]+\"[^\"]+\"")
string(REGEX REPLACE "^#define[ \t]+PY_VERSION[ \t]+\"([^\"]+)\".*" "\\1"
    _version "${python_version_str}")
string(REGEX REPLACE "([0-9])\\..+" "\\1"
    PY_VERSION_MAJOR ${_version})
string(REGEX REPLACE "[0-9]\\.([0-9]+)\\..+" "\\1"
    PY_VERSION_MINOR ${_version})
string(REGEX REPLACE "[0-9]\\.[0-9]+\\.([0-9]+)" "\\1"
    PY_VERSION_PATCH ${_version})
set(PY_VERSION "${PY_VERSION_MAJOR}.${PY_VERSION_MINOR}.${PY_VERSION_PATCH}")
message(STATUS "PY_VERSION: ${PY_VERSION}")
if(NOT DEFINED _download_${PY_VERSION}_md5)
    message(FATAL_ERROR "error: unknown python version '${PY_VERSION}'. Valid version should match '2.7.[3-10]'")
endif()

# Convenience boolean variables to easily test python version
set(IS_PY3 0)
set(IS_PY2 1)
if(PY_VERSION_MAJOR VERSION_GREATER 2)
    set(IS_PY3 1)
    set(IS_PY2 0)
endif()

# Convenience boolean variables to easily test the type of python library being build
set(BUILD_STATIC 1)
set(BUILD_SHARED 0)
if(BUILD_LIBPYTHON_SHARED)
  set(BUILD_STATIC 0)
  set(BUILD_SHARED 1)
endif()

# Options depending of the python version
if(IS_PY2)
  option(Py_USING_UNICODE "Enable unicode support" ON)
else()
  if(DEFINED Py_USING_UNICODE)
    message(AUTHOR_WARNING "Py_USING_UNICODE option is *NOT* supported with Python 3. Current version is ${PY_VERSION}")
  endif()
endif()

if(PY_VERSION_MAJOR VERSION_GREATER 2.7.10)
# See "Improving performance in Python 2.7" - http://lwn.net/Articles/646888/
option(WITH_COMPUTED_GOTOS "Improve performance enabling the computed goto based dispatch" OFF)
set(USE_COMPUTED_GOTOS ${WITH_COMPUTED_GOTOS})
endif()

if(IS_PY3)
set(WITH_HASH_ALGORITHM "default" CACHE STRING "Define hash algorithm for str, bytes and memoryview.")
set_property(CACHE WITH_HASH_ALGORITHM PROPERTY STRINGS "default" "siphash24" "fnv")
endif()

# This is the major version number of Python
set(LIBPYTHON_VERSION ${PY_VERSION_MAJOR}.${PY_VERSION_MINOR})
if(WIN32 AND (NOT MINGW))
    set(LIBPYTHON_VERSION ${PY_VERSION_MAJOR}${PY_VERSION_MINOR})
endif()
set(LIBPYTHON python${LIBPYTHON_VERSION})

# Proceed to the configure checks
include(cmake/ConfigureChecks.cmake)

if(NOT BUILD_SHARED AND NOT BUILD_STATIC)
    message(FATAL_ERROR "One or both of BUILD_SHARED or BUILD_STATIC must be set")
endif()

# Set PYTHONHOME
set(LIBDIR "Lib") # See Lib/distutils/sysconfig.py - function 'get_python_lib'
if(UNIX)
    set(LIBDIR "lib")
endif()
if(USE_LIB64)
    set_property(GLOBAL PROPERTY FIND_LIBRARY_USE_LIB64_PATHS ON)
    set(LIBDIR "lib64")
endif()
set(PYTHONHOME "${LIBDIR}")
if(UNIX)
    set(PYTHONHOME "${PYTHONHOME}/${LIBPYTHON}")
endif()

# Install tree directory
set(BIN_INSTALL_DIR bin)                         # Contains the python executable
if(INSTALL_WINDOWS_TRADITIONAL)
    set(BIN_INSTALL_DIR .)                       # Contains the python executable
endif()
set(CONFIG_INSTALL_DIR share/${LIBPYTHON})
set(EXTENSION_INSTALL_DIR ${PYTHONHOME}/lib-dynload)
if(INSTALL_WINDOWS_TRADITIONAL)
    set(EXTENSION_INSTALL_DIR DLLs)
endif()
set(INCLUDE_INSTALL_DIR include/${LIBPYTHON})
if(MSVC)
    set(INCLUDE_INSTALL_DIR include)
endif()
# Build tree directory
set(BIN_BUILD_DIR ${CMAKE_BINARY_DIR}/bin)
set(CONFIG_BUILD_DIR ${CMAKE_BINARY_DIR}/${CONFIG_INSTALL_DIR})
set(EXTENSION_BUILD_DIR ${CMAKE_BINARY_DIR}/${PYTHONHOME}/lib-dynload)
set(INCLUDE_BUILD_DIR ${SRC_DIR}/Include)

set(ARCHIVEDIR "libs")      # Contains the static (*.a) and import libraries (*.lib)

# Directories specific to 'libpython'
set(LIBPYTHON_LIBDIR ${LIBDIR})
set(LIBPYTHON_ARCHIVEDIR ${LIBDIR})
#set(LIBPYTHON_STATIC_ARCHIVEDIR ${LIBDIR})
set(LIBPYTHON_STATIC_ARCHIVEDIR "../../${ARCHIVEDIR}/${KLAYGE_PLATFORM_NAME}")
#if(WIN32)
#    set(LIBPYTHON_LIBDIR ${BIN_INSTALL_DIR})
#    set(LIBPYTHON_ARCHIVEDIR ${ARCHIVEDIR})
#    set(LIBPYTHON_STATIC_ARCHIVEDIR static-${ARCHIVEDIR})
#endif()

set(EXTRA_PYTHONPATH "" CACHE STRING
    "A colon (:) separated list of extra paths to add to the PYTHONPATH")

# Configure 'pyconfig.h'
if(UNIX)
    set(PYCONFIG_BUILD_DIR ${BIN_BUILD_DIR})
    configure_file(cmake/config-unix/pyconfig.h.in
                   ${PYCONFIG_BUILD_DIR}/pyconfig.h)
elseif(WIN32)
    set(PYCONFIG_BUILD_DIR ${SRC_DIR}/PC) # In a windows build tree, 'pyconfig.h' is NOT required to
                                          # live along side the python executable.
                                          # See function '_init_posix()' and '_init_non_posix()'
                                          # in 'Lib/sysconfig.py'
    if(MINGW)
      set(PYCONFIG_BUILD_DIR cmake/config-mingw)
    endif()
endif()

# Install 'pyconfig.h'
if(INSTALL_DEVELOPMENT)
    install(FILES ${PYCONFIG_BUILD_DIR}/pyconfig.h
            DESTINATION ${INCLUDE_INSTALL_DIR}/
            COMPONENT Development)
endif()

# Set include directories
include_directories(${INCLUDE_BUILD_DIR})
include_directories(${PYCONFIG_BUILD_DIR})
include_directories(${SRC_DIR}/Python)

# Set cflags used by all components
if(CMAKE_C_COMPILER_ID MATCHES GNU)
    append_if_absent(CMAKE_C_FLAGS "-Wall")
    append_if_absent(CMAKE_C_FLAGS "-Wstrict-prototypes")
    append_if_absent(CMAKE_C_FLAGS "-fno-strict-aliasing")
    append_if_absent(CMAKE_C_FLAGS "-fwrapv")
    append_if_absent(CMAKE_C_FLAGS "-g")
elseif(CMAKE_C_COMPILER_ID MATCHES Clang)
    append_if_absent(CMAKE_C_FLAGS "-Wall")
    append_if_absent(CMAKE_C_FLAGS "-g")
elseif(CMAKE_C_COMPILER_ID MATCHES Intel)
    append_if_absent(CMAKE_C_FLAGS "-Wall")
    append_if_absent(CMAKE_C_FLAGS "-no-ansi-alias")
elseif(CMAKE_C_COMPILER_ID MATCHES PGI)
    append_if_absent(CMAKE_C_FLAGS "-alias=traditional")
endif()

if(MSVC)
    append_if_absent(CMAKE_C_FLAGS "/Zm200")
    string(REPLACE "/Ob2" "/Ob1" CMAKE_C_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE}) # InlineFunctionExpansion=1
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE}" CACHE STRING "Flags used by the compiler during release builds" FORCE)
endif()

if(NOT HAVE_POSITION_INDEPENDENT_CODE AND NOT WIN32)
    append_if_absent(CMAKE_C_FLAGS "-fPIC")
endif()

# Useful additional variables that extensions can use.
if(CMAKE_SYSTEM MATCHES Linux)
    set(LINUX ON)
else()
    set(LINUX OFF)
endif()

# Clear PythonTargets.cmake
file(WRITE ${CONFIG_BUILD_DIR}/PythonTargets.cmake "")

add_definitions(-DPy_BUILD_CORE)

# Define python executable wrapper command to ensure
# python executable resolves the expected python library.
set(PYTHON_WRAPPER_COMMAND )
if(BUILD_SHARED AND UNIX)
    set(_envvar LD_LIBRARY_PATH)
    if(APPLE)
        set(_envvar DYLD_LIBRARY_PATH)
    endif()
    set(PYTHON_WRAPPER_COMMAND env ${_envvar}=${CMAKE_BINARY_DIR}/${LIBPYTHON_LIBDIR})
endif()

# Add extension modules
set(extensions_enabled "" CACHE INTERNAL "" FORCE)
set(extensions_disabled "" CACHE INTERNAL "" FORCE)
add_subdirectory(cmake/extensions CMakeBuild/extensions)

# Add the other subdirectories
add_subdirectory(cmake/libpython CMakeBuild/libpython)
if(BUILD_PYTHON_EXECUTABLE)
    add_subdirectory(cmake/python CMakeBuild/python)
endif()
if(BUILD_PYTHON_DEVELOPMENT)
    add_subdirectory(cmake/include CMakeBuild/include)
    add_subdirectory(cmake/lib CMakeBuild/lib)
endif()
if(BUILD_WININST)
    add_subdirectory(cmake/PC/bdist_wininst CMakeBuild/bdist_wininst)
endif()

if(BUILD_PYTHON_EXECUTABLE)
    # Add target to run "Argument Clinic" over all source files
    if(IS_PY3)
    add_custom_target(clinic
        COMMAND ${CMAKE_CROSSCOMPILING_EMULATOR} $<TARGET_FILE:python> ${SRC_DIR}/Tools/clinic/clinic.py --make
        DEPENDS python
        WORKING_DIRECTORY ${SRC_DIR}
        COMMENT "Running 'Argument Clinic' over all source files"
        VERBATIM
    )
    endif()
endif()

if(BUILD_PGEN)
    # Add target to generate 'Include/graminit.h' and 'Python/graminit.c'
    add_custom_target(generate_graminit
        COMMAND ${CMAKE_CROSSCOMPILING_EMULATOR} $<TARGET_FILE:pgen>
            ${SRC_DIR}/Grammar/Grammar
            ${PROJECT_BINARY_DIR}/CMakeFiles/graminit.h
            ${PROJECT_BINARY_DIR}/CMakeFiles/graminit.c
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${PROJECT_BINARY_DIR}/CMakeFiles/graminit.h
            ${SRC_DIR}/Include/graminit.h
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${PROJECT_BINARY_DIR}/CMakeFiles/graminit.c
            ${SRC_DIR}/Python/graminit.c
        DEPENDS pgen
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Generating '${SRC_DIR}/Include/graminit.h' and '${SRC_DIR}/Python/graminit.c'."
        VERBATIM
    )
endif()

if(BUILD_PYTHON_EXECUTABLE)
    # Add target to generate 'opcode.h' header file
    add_custom_target(generate_opcode_h
        COMMAND ${CMAKE_CROSSCOMPILING_EMULATOR} $<TARGET_FILE:python> ${SRC_DIR}/Tools/scripts/generate_opcode_h.py
            ${SRC_DIR}/Lib/opcode.py
            ${PROJECT_BINARY_DIR}/CMakeFiles/opcode.h
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${PROJECT_BINARY_DIR}/CMakeFiles/opcode.h
            ${SRC_DIR}/Include/opcode.h
        DEPENDS python
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Generating '${SRC_DIR}/Include/opcode.h'."
        VERBATIM
    )

    # Add target to generate 'Include/Python-ast.h' from 'Python.asdl'
    add_custom_target(generate_python_ast_h
        COMMAND ${CMAKE_CROSSCOMPILING_EMULATOR} $<TARGET_FILE:python> ${SRC_DIR}/Parser/asdl_c.py
            -h ${SRC_DIR}/Include
            ${SRC_DIR}/Parser/Python.asdl
        DEPENDS python
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Generating '${SRC_DIR}/Include/Python-ast.h'"
        VERBATIM
    )

    # Add target to generate 'Python/Python-ast.c' from 'Python.asdl'
    add_custom_target(generate_python_ast_c
        COMMAND ${CMAKE_CROSSCOMPILING_EMULATOR} $<TARGET_FILE:python> ${SRC_DIR}/Parser/asdl_c.py
            -c ${SRC_DIR}/Python
            ${SRC_DIR}/Parser/Python.asdl
        DEPENDS python
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Generating '${SRC_DIR}/Python/Python-ast.c'"
        VERBATIM
    )
endif()

# If needed, copy 'asdl.py' and 'Python.asdl' into build directory so that 'test_asdl_parser' passes.
set(_asdl_subdir ${LIBDIR})
if(WIN32)
  set(_asdl_subdir )
endif()
file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/${LIBDIR}/Parser)
configure_file(
    ${SRC_DIR}/Parser/Python.asdl
    ${PROJECT_BINARY_DIR}/${_asdl_subdir}/Parser/Python.asdl
    COPYONLY
)
configure_file(
    ${SRC_DIR}/Parser/asdl.py
    ${PROJECT_BINARY_DIR}/${_asdl_subdir}/Parser/asdl.py
    COPYONLY
)

show_extension_summary()

if(UNIX)

    # python.pc
    configure_file(cmake/python.pc.in
                  ${CMAKE_BINARY_DIR}/Misc/python-${LIBPYTHON_VERSION}.pc @ONLY)
    configure_file(cmake/python.pc.in
                  ${CMAKE_BINARY_DIR}/Misc/python-${PY_VERSION_MAJOR}.pc @ONLY)
    configure_file(cmake/python.pc.in
                  ${CMAKE_BINARY_DIR}/Misc/python.pc @ONLY)
    if(INSTALL_DEVELOPMENT)
        install(FILES
                    ${CMAKE_BINARY_DIR}/Misc/python-${LIBPYTHON_VERSION}.pc
                    ${CMAKE_BINARY_DIR}/Misc/python-${PY_VERSION_MAJOR}.pc
                    ${CMAKE_BINARY_DIR}/Misc/python.pc
                DESTINATION lib/pkgconfig
                COMPONENT Development)
    endif()

    # Makefile
    set(MAKEFILE_LDSHARED_FLAGS "-shared")
    if(APPLE)
        set(MAKEFILE_LDSHARED_FLAGS "-dynamiclib -headerpad_max_install_names -undefined dynamic_lookup")
    endif()
    configure_file(cmake/makefile-variables.in
                   ${BIN_BUILD_DIR}/Makefile @ONLY)
    if(INSTALL_DEVELOPMENT)
        install(FILES ${BIN_BUILD_DIR}/Makefile
                DESTINATION ${PYTHONHOME}/config/
                RENAME Makefile
                COMPONENT Development)
    endif()

    # Utility scripts
    if(INSTALL_DEVELOPMENT)
        install(FILES ${SRC_DIR}/install-sh ${SRC_DIR}/Modules/makesetup
                DESTINATION ${PYTHONHOME}/config/
                COMPONENT Development)
    endif()

    # Install manual
    if(INSTALL_MANUAL)
        set(_install_man FILES ${SRC_DIR}/Misc/python.man DESTINATION share/man/man1 COMPONENT Runtime)
        install(${_install_man} RENAME python${LIBPYTHON_VERSION}.1)
        install(${_install_man} RENAME python${PY_VERSION_MAJOR}.1)
    endif()

endif()

# Create an empty lib-dynload folder, even if we don't have any extensions
# to go in there.  bin/python uses this to auto-determine the exec_prefix
# and properly generate the _sysconfigdata.py
file(MAKE_DIRECTORY "${EXTENSION_BUILD_DIR}")
install(CODE "file(MAKE_DIRECTORY \"\${CMAKE_INSTALL_PREFIX}/${EXTENSION_INSTALL_DIR}\")")

if(BUILD_TESTING)
    set(TESTOPTS -l)
    set(TESTPROG ${CMAKE_BINARY_DIR}/${PYTHONHOME}/test/regrtest.py)
    set(TESTPYTHONOPTS )
    if(IS_PY2)
      set(TESTPYTHON $<TARGET_FILE:python> -Wd -3 -E -tt ${TESTPYTHONOPTS})
    else()
      set(TESTPYTHON $<TARGET_FILE:python> ${TESTPYTHONOPTS})
    endif()
    set(TESTPYTHON ${CMAKE_CROSSCOMPILING_EMULATOR} ${TESTPYTHON})
    include(cmake/UnitTests.cmake)
    foreach(unittest ${unittests})
      set(EXTRATESTOPTS -v)
      # XXX Running 'test_doctest' in verbose mode always return 1
      if(unittest STREQUAL "test_doctest")
        set(EXTRATESTOPTS )
      endif()
      add_test(NAME ${unittest} COMMAND ${PYTHON_WRAPPER_COMMAND} ${TESTPYTHON} ${TESTPROG} ${TESTOPTS} ${EXTRATESTOPTS} ${unittest})
    endforeach()
endif()


if(HAVE_CONFIGURE_PACKAGE_CONFIG_FILE AND HAVE_WRITE_BASIC_PACKAGE_VERSION_FILE)

# Configure 'PythonConfig.cmake' for a build tree
set(CONFIG_DIR_CONFIG ${CONFIG_BUILD_DIR})
set(INCLUDE_DIR_CONFIG ${INCLUDE_BUILD_DIR})
set(PYTHON_CONFIG_CODE "####### Expanded from \@PYTHON_CONFIG_CODE\@ #######\n")
set(PYTHON_CONFIG_CODE "${PYTHON_CONFIG_CODE}list(APPEND PYTHON_INCLUDE_DIR \"${PYCONFIG_BUILD_DIR}\")\n")
set(PYTHON_CONFIG_CODE "${PYTHON_CONFIG_CODE}##################################################")
set(python_config ${CONFIG_BUILD_DIR}/PythonConfig.cmake)
configure_package_config_file(
    cmake/PythonConfig.cmake.in
    ${python_config}
    INSTALL_DESTINATION ${CMAKE_BINARY_DIR}
    PATH_VARS CONFIG_DIR_CONFIG INCLUDE_DIR_CONFIG
    NO_CHECK_REQUIRED_COMPONENTS_MACRO
)

# Configure 'PythonConfig.cmake' for an install tree
set(CONFIG_DIR_CONFIG ${CONFIG_INSTALL_DIR})
set(INCLUDE_DIR_CONFIG ${INCLUDE_INSTALL_DIR})
set(PYTHON_CONFIG_CODE "")
set(python_install_config ${CMAKE_BINARY_DIR}/CMakeFiles/PythonConfig.cmake)
configure_package_config_file(
    cmake/PythonConfig.cmake.in
    ${python_install_config}
    INSTALL_DESTINATION ${CMAKE_INSTALL_PREFIX}/${CONFIG_INSTALL_DIR}
    PATH_VARS CONFIG_DIR_CONFIG INCLUDE_DIR_CONFIG
    NO_CHECK_REQUIRED_COMPONENTS_MACRO
)

# Configure 'PythonTargets.cmake' and 'PythonConfigVersion.cmake
get_property(PYTHON_TARGETS GLOBAL PROPERTY PYTHON_TARGETS)
export(TARGETS ${PYTHON_TARGETS} APPEND FILE ${CONFIG_BUILD_DIR}/PythonTargets.cmake)

set(python_config_version ${CONFIG_BUILD_DIR}/PythonConfigVersion.cmake)
write_basic_package_version_file(
    ${python_config_version}
    VERSION ${PY_VERSION}
    COMPATIBILITY SameMajorVersion
)

if(INSTALL_DEVELOPMENT)
    # Install 'PythonTargets.cmake', 'PythonConfig.cmake' and 'PythonConfigVersion.cmake
    install(EXPORT PythonTargets
        FILE PythonTargets.cmake
        DESTINATION ${CONFIG_INSTALL_DIR} COMPONENT Development)

    install(
        FILES ${python_install_config} ${python_config_version}
        DESTINATION ${CONFIG_INSTALL_DIR} COMPONENT Development
    )
endif()

endif()

# Install License
set(license_destination ${PYTHONHOME})
if(INSTALL_WINDOWS_TRADITIONAL)
  set(license_destination .)
endif()
install(FILES ${SRC_DIR}/LICENSE DESTINATION ${license_destination} COMPONENT Runtime RENAME LICENSE.txt)
