SET(fmt_project_dir "${EXTERNAL_PROJECT_DIR}/fmt")

UpdateExternalLib("fmt" "https://github.com/fmtlib/fmt.git" "f5e54359df4c26b6230fc61d38aa294581393084")

set(CPACK_BINARY_NSIS OFF CACHE BOOL "" FORCE)
set(FMT_DOC OFF CACHE BOOL "" FORCE)
set(FMT_INSTALL OFF CACHE BOOL "" FORCE)
set(FMT_OS OFF CACHE BOOL "" FORCE)
set(FMT_TEST OFF CACHE BOOL "" FORCE)
set(FMT_DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX} CACHE STRING "" FORCE)

add_subdirectory(${fmt_project_dir} "${CMAKE_CURRENT_BINARY_DIR}/fmt" EXCLUDE_FROM_ALL)

set_target_properties(fmt PROPERTIES
	FOLDER "External/fmt"
)
