SET(SOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Tools/src/Tex2JTML/Tex2JTML.cpp
)

IF(MSVC)
	SET(EXTRA_LINKED_LIBRARIES ${EXTRA_LINKED_LIBRARIES})
ELSEIF(MINGW)
	SET(EXTRA_LINKED_LIBRARIES ${EXTRA_LINKED_LIBRARIES}
		debug boost_program_options-mgw47-mt-d-1_53 optimized boost_program_options-mgw47-mt-1_53
		debug boost_filesystem-mgw47-mt-d-1_53 optimized boost_filesystem-mgw47-mt-1_53)
ELSE()
	SET(EXTRA_LINKED_LIBRARIES ${EXTRA_LINKED_LIBRARIES}
		boost_program_options boost_filesystem)
ENDIF()

SETUP_TOOL(Tex2JTML)
