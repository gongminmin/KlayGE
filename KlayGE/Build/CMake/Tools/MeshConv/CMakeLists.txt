SET(SOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Tools/src/MeshConv/MeshConv.cpp
)

SETUP_TOOL(MeshConv)

target_link_libraries(MeshConv
	PRIVATE
		ToolCommon
)
