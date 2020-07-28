SET(SOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Tools/src/PlatformDeployer/PlatformDeployer.cpp
)

SETUP_TOOL(PlatformDeployer)

target_link_libraries(PlatformDeployer
	PRIVATE
		ToolCommon
)
