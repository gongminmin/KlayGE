SET(SOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Tools/src/HDRCompressor/HDRCompressor.cpp
)

SETUP_TOOL(HDRCompressor)

target_link_libraries(HDRCompressor
	PRIVATE
		${KLAYGE_FILESYSTEM_LIBRARY}
)
