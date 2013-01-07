SET(SOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Tools/src/HDRCompressor/HDRCompressor.cpp
)

IF(MSVC)
	SET(EXTRA_LINKED_LIBRARIES "")
ELSEIF(MINGW)
	SET(EXTRA_LINKED_LIBRARIES 
		debug boost_filesystem-mgw47-mt-d-1_51 optimized boost_filesystem-mgw47-mt-1_51)
ELSE()
	SET(EXTRA_LINKED_LIBRARIES
		boost_filesystem)
ENDIF()
			
SETUP_TOOL(HDRCompressor)
