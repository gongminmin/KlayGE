SET(SOURCE_FILES
	${DXBC2GLSL_PROJECT_DIR}/Tools/DXBC2GLSLCmd/DXBC2GLSLCmd.cpp
)

SOURCE_GROUP("Source Files" FILES ${SOURCE_FILES})

ADD_EXECUTABLE(DXBC2GLSLCmd ${SOURCE_FILES})

SET_TARGET_PROPERTIES(DXBC2GLSLCmd PROPERTIES
	DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX}
	CXX_VISIBILITY_PRESET hidden
	VISIBILITY_INLINES_HIDDEN ON
	OUTPUT_NAME DXBC2GLSLCmd
	FOLDER "DXBC2GLSL"
)

target_link_libraries(DXBC2GLSLCmd
	PRIVATE
		DXBC2GLSLLib
		KFL
)
