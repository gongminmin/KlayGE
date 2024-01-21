UpdateExternalLib("vorbis" "https://github.com/xiph/vorbis.git" "0657aee69dec8508a0011f47f3b69d7538e9d262")

SET(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
SET(BUILD_FRAMEWORK OFF CACHE BOOL "" FORCE)
set(INSTALL_CMAKE_PACKAGE_MODULE OFF CACHE BOOL "" FORCE)

# To make libvorbis enable to find libogg
set(CMAKE_DISABLE_FIND_PACKAGE_Ogg FALSE CACHE BOOL "" FORCE)
set(OGG_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../ogg/ogg/include")
set(OGG_LIBRARY ogg)
add_library(Ogg::ogg ALIAS ogg)

add_subdirectory(vorbis EXCLUDE_FROM_ALL)

set_target_properties("vorbis" "vorbisfile" "vorbisenc" PROPERTIES FOLDER "Externals/vorbis")
