set(NANOSVG_PROJECT_DIR "${EXTERNAL_PROJECT_DIR}/nanosvg")

UpdateExternalLib("nanosvg" "https://github.com/memononen/nanosvg.git" "2b08deeb553c723d151f908d786c64136d26d576")

add_library(nanosvg INTERFACE)
target_include_directories(nanosvg
	INTERFACE
		${NANOSVG_PROJECT_DIR}/src
)
