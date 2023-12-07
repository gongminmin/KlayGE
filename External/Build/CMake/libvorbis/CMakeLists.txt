SET(VORBIS_PROJECT_DIR "${EXTERNAL_PROJECT_DIR}/libvorbis")

UpdateExternalLib("libvorbis" "https://github.com/xiph/vorbis.git" "3c177d011684fce294edce5ea93f27d3453e6830")

SET(BUILD_SHARED_LIBS OFF)
SET(BUILD_FRAMEWORK OFF)
if(KLAYGE_COMPILER_MSVC)
	string(REPLACE "/WX" "/WX:NO" CMAKE_STATIC_LINKER_FLAGS "${CMAKE_STATIC_LINKER_FLAGS}")
endif()

# To make libvorbis enable to find libogg
add_library(Ogg::ogg ALIAS ogg)
export(EXPORT OggTargets NAMESPACE Ogg:: FILE OggTargets.cmake)
SET(CMAKE_DISABLE_FIND_PACKAGE_Ogg TRUE CACHE BOOL "" FORCE)

ADD_SUBDIRECTORY(${VORBIS_PROJECT_DIR} "${CMAKE_CURRENT_BINARY_DIR}/vorbis")

SET_TARGET_PROPERTIES(vorbis PROPERTIES
	OUTPUT_NAME vorbis_static${KLAYGE_OUTPUT_SUFFIX}
)
SET_TARGET_PROPERTIES(vorbisfile PROPERTIES
	OUTPUT_NAME vorbisfile_static${KLAYGE_OUTPUT_SUFFIX}
)

set_target_properties("vorbis" "vorbisfile" "vorbisenc" PROPERTIES FOLDER "External/libvorbis")
