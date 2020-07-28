SET(SOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Tools/src/VectorTexGen/VectorTexGen.cpp
)

SETUP_TOOL(VectorTexGen)

target_link_libraries(VectorTexGen
	PRIVATE
		nanosvg
)
