SET(SOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Tools/src/KFontGen/KFontGen.cpp
)

SETUP_TOOL(KFontGen)

target_include_directories(KFontGen
	PRIVATE
		${PROJECT_BINARY_DIR}/External/freetype/freetype/include
		${KLAYGE_PROJECT_DIR}/../External/freetype/include
		${KLAYGE_PROJECT_DIR}/../kfont/include
)

target_link_libraries(KFontGen
	PRIVATE
		kfont
		${KLAYGE_FILESYSTEM_LIBRARY}
		freetype
		${ZLIB_LIBRARIES}
)

add_dependencies(KFontGen freetype)
