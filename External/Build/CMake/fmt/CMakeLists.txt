if(HAVE_CXX20_LIBRARY_FORMAT)
	return()
endif()

SET(fmt_project_dir "${EXTERNAL_PROJECT_DIR}/fmt")

UpdateExternalLib("fmt" "https://github.com/fmtlib/fmt.git" "7bdf0628b1276379886c7f6dda2cef2b3b374f0b")

set(CPACK_BINARY_NSIS OFF CACHE BOOL "" FORCE)
set(FMT_DOC OFF CACHE BOOL "" FORCE)
set(FMT_INSTALL OFF CACHE BOOL "" FORCE)
set(FMT_TEST OFF CACHE BOOL "" FORCE)
set(FMT_DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX} CACHE STRING "" FORCE)

add_subdirectory(${fmt_project_dir} "${CMAKE_CURRENT_BINARY_DIR}/fmt")

set_target_properties(fmt PROPERTIES
	OUTPUT_NAME fmt${KLAYGE_OUTPUT_SUFFIX}
	FOLDER "${EXTERNAL_VS_FOLDER_PREFIX}/fmt"
)
