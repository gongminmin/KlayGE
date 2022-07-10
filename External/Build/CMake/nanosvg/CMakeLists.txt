set(NANOSVG_PROJECT_DIR "${EXTERNAL_PROJECT_DIR}/nanosvg")

UpdateExternalLib("nanosvg" "https://github.com/memononen/nanosvg.git" "3e403ec72a9145cbbcc6c63d94a4caf079aafec2")

add_library(nanosvg INTERFACE)
target_include_directories(nanosvg
	INTERFACE
		${NANOSVG_PROJECT_DIR}/src
)
