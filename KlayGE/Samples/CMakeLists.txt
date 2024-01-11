option(KLAYGE_BUILD_SAMPLES "Build samples" ON)
if(NOT KLAYGE_BUILD_SAMPLES)
	return()
endif()

add_subdirectory(src)
