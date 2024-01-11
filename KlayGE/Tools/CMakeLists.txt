option(KLAYGE_BUILD_TOOLS "Build tools" ON)
if(NOT KLAYGE_BUILD_TOOLS)
	return()
endif()

add_subdirectory(src)
