SET(SOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Tools/src/Tex2JTML/Tex2JTML.cpp
)

SETUP_TOOL(Tex2JTML)

target_link_libraries(Tex2JTML
	PRIVATE
		ToolCommon
)
