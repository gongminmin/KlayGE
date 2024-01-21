UpdateExternalLib("googletest" "https://github.com/google/googletest.git" "f8d7d77c06936315286eb55f8de22cd23c188571")

set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(gtest_disable_pthreads ON CACHE BOOL "" FORCE)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

add_subdirectory(googletest EXCLUDE_FROM_ALL)

SET_TARGET_PROPERTIES(gtest PROPERTIES
	FOLDER "Externals/googletest"
)
set_property(TARGET gtest PROPERTY PDB_OUTPUT_DIRECTORY)
