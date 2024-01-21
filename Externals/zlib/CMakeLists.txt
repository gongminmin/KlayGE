UpdateExternalLib("zlib" "https://github.com/madler/zlib.git" "09155eaa2f9270dc4ed1fa13e2b4b2613e6e4851" "" "" need_patch)
if(need_patch)
	foreach(patch "0001-Fix-.rc-compiling-issues-in-clang-cl")
		ApplyPatch("zlib" "${CMAKE_CURRENT_SOURCE_DIR}/Patches/${patch}.patch")
	endforeach()
endif()

set(SKIP_INSTALL_ALL ON CACHE BOOL "" FORCE)

add_subdirectory(zlib)

if(KLAYGE_PREFERRED_LIB_TYPE STREQUAL "STATIC")
	set(KLAYGE_ZLIB_NAME "zlibstatic" CACHE INTERNAL "" FORCE)
else()
	set(KLAYGE_ZLIB_NAME "zlib" CACHE INTERNAL "" FORCE)
endif()

set(CMAKE_DISABLE_FIND_PACKAGE_ZLIB TRUE CACHE BOOL "" FORCE)
set(ZLIB_INCLUDE_DIR
	${CMAKE_CURRENT_SOURCE_DIR}/zlib
	${PROJECT_BINARY_DIR}/Externals/zlib/zlib
	CACHE INTERNAL "" FORCE
)
set(ZLIB_INCLUDE_DIRS ${ZLIB_INCLUDE_DIR} CACHE INTERNAL "" FORCE)
set(ZLIB_LIBRARY ${KLAYGE_ZLIB_NAME} CACHE INTERNAL "" FORCE)
set(ZLIB_LIBRARIES ${ZLIB_LIBRARY} CACHE INTERNAL "" FORCE)
set(ZLIB_FOUND TRUE CACHE INTERNAL "" FORCE)

set_target_properties(${KLAYGE_ZLIB_NAME} PROPERTIES
	DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX}
	OUTPUT_NAME ${KLAYGE_ZLIB_NAME}${KLAYGE_OUTPUT_SUFFIX}
)

set_target_properties("example" "minigzip" "zlib" "zlibstatic"
	PROPERTIES FOLDER "Externals/zlib"
)
