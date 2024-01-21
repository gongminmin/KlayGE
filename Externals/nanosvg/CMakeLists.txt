UpdateExternalLib("nanosvg" "https://github.com/memononen/nanosvg.git" "93ce879dc4c04a3ef1758428ec80083c38610b1f")

add_library(nanosvg INTERFACE)
target_include_directories(nanosvg
	INTERFACE
		${CMAKE_CURRENT_SOURCE_DIR}/nanosvg/src
)
