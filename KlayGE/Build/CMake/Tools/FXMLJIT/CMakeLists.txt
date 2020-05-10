SET(SOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Tools/src/FXMLJIT/FXMLJIT.cpp
)

SETUP_TOOL(FXMLJIT)

target_link_libraries(FXMLJIT
	PRIVATE
		ToolCommon
)
