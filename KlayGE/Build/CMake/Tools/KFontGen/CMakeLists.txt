SET(SOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Tools/src/KFontGen/KFontGen.cpp
)

SETUP_TOOL(KFontGen)

target_include_directories(KFontGen
	PRIVATE
		$<TARGET_PROPERTY:freetype,INTERFACE_INCLUDE_DIRECTORIES>
		$<TARGET_PROPERTY:kfont,INTERFACE_INCLUDE_DIRECTORIES>
)

target_link_libraries(KFontGen
	PRIVATE
		freetype
		kfont
)

add_dependencies(KFontGen freetype)
