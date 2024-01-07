UpdateExternalLib("googletest" "https://github.com/google/googletest.git" "e649993a402d96afe25fbf3413749adf0f2947f6")

set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(gtest_disable_pthreads ON CACHE BOOL "" FORCE)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

ADD_SUBDIRECTORY(googletest)

SET_TARGET_PROPERTIES(gtest PROPERTIES
	FOLDER "Externals/googletest"
)
set_property(TARGET gtest PROPERTY PDB_OUTPUT_DIRECTORY)
