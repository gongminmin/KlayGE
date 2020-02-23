SET(SOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Tools/src/FXMLJIT/FXMLJIT.cpp
)

SETUP_TOOL(FXMLJIT)

target_include_directories(FXMLJIT
	PRIVATE
		${KLAYGE_PROJECT_DIR}/Plugins/Include
)

target_link_libraries(FXMLJIT
	PRIVATE
		ToolCommon
		${KLAYGE_FILESYSTEM_LIBRARY}
)
