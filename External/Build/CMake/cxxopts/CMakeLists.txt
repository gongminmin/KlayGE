set(CXXOPTS_PROJECT_DIR "${EXTERNAL_PROJECT_DIR}/cxxopts")

UpdateExternalLib("cxxopts" "https://github.com/jarro2783/cxxopts.git" "302302b30839505703d37fb82f536c53cf9172fa")

set(CXXOPTS_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(CXXOPTS_BUILD_TESTS OFF CACHE BOOL "" FORCE)

add_subdirectory(${CXXOPTS_PROJECT_DIR} "${CMAKE_CURRENT_BINARY_DIR}/cxxopts" EXCLUDE_FROM_ALL)
