SET(SOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Tools/src/FxmlJit/FxmlJit.cpp
)

SETUP_TOOL(FxmlJit)

target_link_libraries(FxmlJit
	PRIVATE
		ToolCommon
)
