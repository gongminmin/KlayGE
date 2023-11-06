set(ZLIB_PROJECT_DIR "${EXTERNAL_PROJECT_DIR}/zlib")

UpdateExternalLib("zlib" "https://github.com/madler/zlib.git" "cacf7f1d4e3d44d871b605da3b647f07d718623f" "" "" need_patch)
if(need_patch)
	foreach(patch "0001-Fix-.rc-compiling-issues-in-clang-cl")
		ApplyPatch("zlib" "${CMAKE_CURRENT_SOURCE_DIR}/Patches/${patch}.patch")
	endforeach()
endif()

set(SKIP_INSTALL_ALL ON CACHE BOOL "" FORCE)
set(AMD64 OFF CACHE BOOL "" FORCE)
set(ASM686 OFF CACHE BOOL "" FORCE)

add_subdirectory(${ZLIB_PROJECT_DIR} "${CMAKE_CURRENT_BINARY_DIR}/zlib")

if(KLAYGE_PREFERRED_LIB_TYPE STREQUAL "STATIC")
	set(KLAYGE_ZLIB_NAME "zlibstatic" CACHE STRING "" FORCE)
else()
	set(KLAYGE_ZLIB_NAME "zlib" CACHE STRING "" FORCE)
endif()

set(CMAKE_DISABLE_FIND_PACKAGE_ZLIB TRUE CACHE BOOL "" FORCE)
set(ZLIB_INCLUDE_DIR
	${EXTERNAL_PROJECT_DIR}/zlib
	${PROJECT_BINARY_DIR}/External/zlib/zlib
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

foreach(target
	"example" "minigzip" "zlib" "zlibstatic")
	set_target_properties(${target} PROPERTIES FOLDER "${EXTERNAL_VS_FOLDER_PREFIX}/zlib")
endforeach()
