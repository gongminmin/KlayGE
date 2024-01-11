option(KLAYGE_BUILD_TUTORIALS "Build tutorials" ON)
if(NOT KLAYGE_BUILD_TUTORIALS)
	return()
endif()

add_subdirectory(src)
