SET(SOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Tools/src/ImageConv/ImageConv.cpp
)

SETUP_TOOL(ImageConv)

target_link_libraries(ImageConv
	PRIVATE
		ToolCommon
)
