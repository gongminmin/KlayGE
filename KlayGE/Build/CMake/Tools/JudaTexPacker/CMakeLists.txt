SET(SOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Tools/src/JudaTexPacker/JudaTexPacker.cpp
)

SETUP_TOOL(JudaTexPacker)

target_link_libraries(JudaTexPacker
	PRIVATE
		${KLAYGE_FILESYSTEM_LIBRARY}
)
