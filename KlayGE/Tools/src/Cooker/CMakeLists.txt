SET(SOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Tools/src/Cooker/Cooker.cpp
)

SETUP_TOOL(Cooker)

target_link_libraries(Cooker
	PRIVATE
		ToolCommon
)
