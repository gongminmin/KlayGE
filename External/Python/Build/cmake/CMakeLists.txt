cmake_minimum_required(VERSION 2.8.6)

set(PY_VERSION_MAJOR 3)
set(PY_VERSION_MINOR 5)
set(PY_VERSION_PATCH 1)
SET(PY_VERSION ${PY_VERSION_MAJOR}.${PY_VERSION_MINOR}.${PY_VERSION_PATCH})

project(Python C)

SET(PYTHON_PROJECT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../..")
SET(KLAYGE_ROOT_DIR "${PYTHON_PROJECT_DIR}/../..")

INCLUDE(${KLAYGE_ROOT_DIR}/cmake/Common.cmake)
INCLUDE(${KLAYGE_ROOT_DIR}/cmake/Platform.cmake)
INCLUDE(${KLAYGE_ROOT_DIR}/cmake/Compiler.cmake)
INCLUDE(${KLAYGE_ROOT_DIR}/External/Build/CMake/ExternalCommon.cmake)

SET(REL_PATH "External/Downloads/Python-${PY_VERSION}.patched.7z")
SET(SEVENZIP_PATH "${KLAYGE_ROOT_DIR}/External/Downloads/${KLAYGE_HOST_PLATFORM_NAME}/7z")
SET(PACKAGE_COMMIT_ID "f331ae11103a42cc8b26bf789a8f1f7294b4f81a")
SET(PACKAGE_FILE_ID "64739c58cd38b51e518d061666e9978239ad7708")

DOWNLOAD_PACKAGE("Python" ${REL_PATH} ${PACKAGE_COMMIT_ID} ${PACKAGE_FILE_ID} "README")

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
IF(ANDROID OR IOS)
	SET(BUILD_STATIC ON)
	SET(UNIX OFF)
	SET(LINUX OFF)
	SET(HAVE_WORDS_BIGENDIAN ON)
ENDIF()
	
# This is the major version number of Python
set(LIBPYTHON_VERSION ${PY_VERSION_MAJOR}.${PY_VERSION_MINOR})
if(MSVC)
	set(LIBPYTHON_VERSION ${PY_VERSION_MAJOR}${PY_VERSION_MINOR})
endif(MSVC)
set(LIBPYTHON python${LIBPYTHON_VERSION})

# Include helper functions
include(cmake/Assembler.cmake)
include(cmake/Extensions.cmake)
include(CMakeDependentOption)
include(CMakeParseArguments)

# Options
option(BUILD_SHARED "Build a shared libpython library" OFF)
option(BUILD_STATIC "Build a static libpython library" ON)
option(USE_LIB64 "Search for dependencies and install to prefix/lib64 instead of prefix/lib" OFF)

option(USE_SYSTEM_LIBRARIES "Use system libraries" ON)

cmake_dependent_option(USE_SYSTEM_Curses "Use system Curse" ON "USE_SYSTEM_LIBRARIES" OFF)
cmake_dependent_option(USE_SYSTEM_EXPAT "Use system EXPAT" ON "USE_SYSTEM_LIBRARIES" OFF)
cmake_dependent_option(USE_SYSTEM_OpenSSL "Use system OpenSSL" ON "USE_SYSTEM_LIBRARIES" OFF)
cmake_dependent_option(USE_SYSTEM_TCL "Use system TCL" ON "USE_SYSTEM_LIBRARIES" OFF)
cmake_dependent_option(USE_SYSTEM_ZLIB "Use system ZLIB" ON "USE_SYSTEM_LIBRARIES" OFF)
cmake_dependent_option(USE_SYSTEM_DB "Use system DB" ON "USE_SYSTEM_LIBRARIES" OFF)
cmake_dependent_option(USE_SYSTEM_GDBM "Use system GDBM" ON "USE_SYSTEM_LIBRARIES" OFF)
cmake_dependent_option(USE_SYSTEM_READLINE "Use system READLINE" ON "USE_SYSTEM_LIBRARIES" OFF)
cmake_dependent_option(USE_SYSTEM_SQLITE3 "Use system SQLITE3" ON "USE_SYSTEM_LIBRARIES" OFF)

cmake_dependent_option(USE_LIBEDIT "Use libedit instead of readline" OFF
					   "USE_SYSTEM_READLINE" OFF)

option(WITH_TSC "profile with the Pentium timestamp counter" OFF)
option(ENABLE_IPV6 "Enable IP v6" ON)
option(WITH_DOC_STRINGS "Enable if you want documentation strings in extension modules" ON)
option(Py_USING_UNICODE "Enable unicode support" ON)
set(Py_DEBUG OFF) # TODO - Build debugging interpreter
option(WITH_PYMALLOC "Define if you want to compile in Python-specific mallocs" ON)
option(WITH_THREAD "Compile in rudimentary thread support" ON)

# Detect source directory
set(SRC_DIR ${PYTHON_PROJECT_DIR})

# Proceed to the configure checks
include(cmake/ConfigureChecks.cmake)

if(NOT BUILD_SHARED AND NOT BUILD_STATIC)
	message(FATAL_ERROR "One or both of BUILD_SHARED or BUILD_STATIC must be set")
endif(NOT BUILD_SHARED AND NOT BUILD_STATIC)

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
set(CONFIG_INSTALL_DIR share/${LIBPYTHON})
set(EXTENSION_INSTALL_DIR ${PYTHONHOME}/lib-dynload)
set(INCLUDE_INSTALL_DIR include/${LIBPYTHON})
if(MSVC)
	set(INCLUDE_INSTALL_DIR include)
endif(MSVC)
# Build tree directory
set(BIN_BUILD_DIR ${CMAKE_BINARY_DIR}/bin)
set(CONFIG_BUILD_DIR ${CMAKE_BINARY_DIR}/${CONFIG_INSTALL_DIR})
set(EXTENSION_BUILD_DIR ${CMAKE_BINARY_DIR}/${EXTENSION_INSTALL_DIR})
set(INCLUDE_BUILD_DIR ${SRC_DIR}/Include)

set(ARCHIVEDIR "libs")      # Contains the static (*.a) and import libraries (*.lib)

# Directories specific to 'libpython'
set(LIBPYTHON_LIBDIR ${BIN_INSTALL_DIR})
set(LIBPYTHON_ARCHIVEDIR ${ARCHIVEDIR})
set(LIBPYTHON_STATIC_ARCHIVEDIR "../../${ARCHIVEDIR}/${KLAYGE_PLATFORM_NAME}")

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
endif(UNIX)

# Install 'pyconfig.h'
install(FILES ${PYCONFIG_BUILD_DIR}/pyconfig.h
		DESTINATION ${INCLUDE_INSTALL_DIR}/)

# Set include directories
include_directories(cmake)
include_directories(${INCLUDE_BUILD_DIR})
include_directories(${PYCONFIG_BUILD_DIR})
include_directories(${SRC_DIR}/Python)

# Set cflags used by all components
if(UNIX)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-strict-aliasing -fwrapv -Wall -Wstrict-prototypes")
endif(UNIX)
if(MSVC)
	string(REPLACE "/Ob2" "/Ob1" CMAKE_C_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE}) # InlineFunctionExpansion=1
	set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE}" CACHE STRING "Flags used by the compiler during release builds" FORCE)
endif(MSVC)

if(NOT WIN32)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
endif(NOT WIN32)

# Useful additional variables that extensions can use.
if(UNIX AND NOT APPLE)
	set(LINUX ON)
else(UNIX AND NOT APPLE)
	set(LINUX OFF)
endif(UNIX AND NOT APPLE)

# Clear PythonTargets.cmake
file(WRITE ${CONFIG_BUILD_DIR}/PythonTargets.cmake "")

# Add extension modules
set(builtin_extensions "" CACHE INTERNAL "" FORCE)
set(builtin_source "" CACHE INTERNAL "" FORCE)
set(builtin_link_libraries "" CACHE INTERNAL "" FORCE)
set(builtin_includedirs "" CACHE INTERNAL "" FORCE)
set(builtin_definitions "" CACHE INTERNAL "" FORCE)
set(extensions_enabled "" CACHE INTERNAL "" FORCE)
set(extensions_disabled "" CACHE INTERNAL "" FORCE)
add_subdirectory(cmake/extensions CMakeBuild/extensions)

# Add the other subdirectories
add_subdirectory(cmake/libpython CMakeBuild/libpython)
add_subdirectory(cmake/include CMakeBuild/include)

show_extension_summary()

if(UNIX)

	# python.pc
	configure_file(cmake/python.pc.in
				  ${CMAKE_BINARY_DIR}/Misc/python-${LIBPYTHON_VERSION}.pc @ONLY)
	configure_file(cmake/python.pc.in
				  ${CMAKE_BINARY_DIR}/Misc/python-${PY_VERSION_MAJOR}.pc @ONLY)
	configure_file(cmake/python.pc.in
				  ${CMAKE_BINARY_DIR}/Misc/python.pc @ONLY)
	install(FILES
				${CMAKE_BINARY_DIR}/Misc/python-${LIBPYTHON_VERSION}.pc
				${CMAKE_BINARY_DIR}/Misc/python-${PY_VERSION_MAJOR}.pc
				${CMAKE_BINARY_DIR}/Misc/python.pc
			DESTINATION lib/pkgconfig)

	# Makefile
	set(MAKEFILE_LDSHARED_FLAGS "-shared")
	if(APPLE)
		set(MAKEFILE_LDSHARED_FLAGS "-dynamiclib -headerpad_max_install_names -undefined dynamic_lookup")
	endif(APPLE)
	configure_file(cmake/makefile-variables.in
				   ${BIN_BUILD_DIR}/Makefile @ONLY)
	install(FILES ${BIN_BUILD_DIR}/Makefile
			DESTINATION ${PYTHONHOME}/config/
			RENAME Makefile)

	# Utility scripts
	install(FILES ${SRC_DIR}/install-sh ${SRC_DIR}/Modules/makesetup
			DESTINATION ${PYTHONHOME}/config/)

endif(UNIX)

if(BUILD_TESTING)
	set(EXTRATESTOPTS -v)
	set(TESTOPTS -l ${EXTRATESTOPTS})
	set(TESTPROG ${CMAKE_BINARY_DIR}/${PYTHONHOME}/test/regrtest.py)
	set(TESTPYTHONOPTS )
	set(TESTPYTHON $<TARGET_FILE:python> -Wd -3 -E -tt ${TESTPYTHONOPTS})

	include(cmake/UnitTests.cmake)
	foreach(unittest ${unittests})
	  add_test(NAME ${unittest} COMMAND ${TESTPYTHON} ${TESTPROG} ${TESTOPTS} ${unittest})
	endforeach(unittest)
endif(BUILD_TESTING)


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
	VERSION ${PY_VERSION_MAJOR}.${PY_VERSION_MINOR}.${PY_VERSION_PATCH}
	COMPATIBILITY SameMajorVersion
)

# Install 'PythonTargets.cmake', 'PythonConfig.cmake' and 'PythonConfigVersion.cmake
install(EXPORT PythonTargets FILE PythonTargets.cmake DESTINATION ${CONFIG_INSTALL_DIR})

install(
	FILES ${python_install_config} ${python_config_version}
	DESTINATION ${CONFIG_INSTALL_DIR} COMPONENT Development
)

endif(HAVE_CONFIGURE_PACKAGE_CONFIG_FILE AND HAVE_WRITE_BASIC_PACKAGE_VERSION_FILE)
