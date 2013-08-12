SET(SOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Tools/src/HDRCompressor/HDRCompressor.cpp
)

IF(MSVC)
	SET(EXTRA_LINKED_LIBRARIES "")
ELSE()
	SET(EXTRA_LINKED_LIBRARIES 
		debug boost_filesystem-${KLAYGE_COMPILER_NAME}${GCC_VER}-mt-d-${BOOST_VER} optimized boost_filesystem-${KLAYGE_COMPILER_NAME}${GCC_VER}-mt-${BOOST_VER})
ENDIF()
			
SETUP_TOOL(HDRCompressor)
