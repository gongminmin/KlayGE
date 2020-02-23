SET(SOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Tools/src/ImposterGen/ImposterGen.cpp
)

SETUP_TOOL(ImposterGen)

target_link_libraries(ImposterGen
	PRIVATE
		${KLAYGE_FILESYSTEM_LIBRARY}
)
