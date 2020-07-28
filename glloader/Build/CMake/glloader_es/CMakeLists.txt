FIND_PACKAGE(GLES)
IF(NOT KLAYGE_GLES_FOUND)
	return()
ENDIF()

SETUP_GLLOADER(glloader_es)

target_compile_definitions(glloader_es
	PRIVATE
		-DGLLOADER_GLES_SUPPORT
)

target_include_directories(glloader_es
	PUBLIC
		${KLAYGE_GLES_INCLUDE_DIR}
)
if(APPLE AND NOT IOS)
	target_include_directories(glloader_es
		PUBLIC
			/opt/X11/include
	)
endif()
