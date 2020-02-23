SET(SOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Tools/src/VectorTexGen/VectorTexGen.cpp
)

SETUP_TOOL(VectorTexGen)

target_include_directories(VectorTexGen
	PRIVATE
		${KLAYGE_PROJECT_DIR}/../External/nanosvg/src
)
