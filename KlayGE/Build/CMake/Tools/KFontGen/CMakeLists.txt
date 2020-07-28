SET(SOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Tools/src/KFontGen/KFontGen.cpp
)

SETUP_TOOL(KFontGen)

target_link_libraries(KFontGen
	PRIVATE
		freetype
		kfont
)

add_dependencies(KFontGen freetype)
