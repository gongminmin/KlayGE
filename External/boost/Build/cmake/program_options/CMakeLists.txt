SET(LIB_NAME program_options)

SET(SOURCE_FILES
	${BOOST_PROJECT_DIR}/libs/program_options/src/cmdline.cpp
	${BOOST_PROJECT_DIR}/libs/program_options/src/config_file.cpp
	${BOOST_PROJECT_DIR}/libs/program_options/src/options_description.cpp
	${BOOST_PROJECT_DIR}/libs/program_options/src/parsers.cpp
	${BOOST_PROJECT_DIR}/libs/program_options/src/variables_map.cpp
	${BOOST_PROJECT_DIR}/libs/program_options/src/value_semantic.cpp
	${BOOST_PROJECT_DIR}/libs/program_options/src/positional_options.cpp
	${BOOST_PROJECT_DIR}/libs/program_options/src/utf8_codecvt_facet.cpp
	${BOOST_PROJECT_DIR}/libs/program_options/src/convert.cpp
	${BOOST_PROJECT_DIR}/libs/program_options/src/winmain.cpp
	${BOOST_PROJECT_DIR}/libs/program_options/src/split.cpp
)

IF(KLAYGE_PREFERRED_LIB_TYPE STREQUAL "SHARED")
	ADD_DEFINITIONS(-DBOOST_PROGRAM_OPTIONS_DYN_LINK=1)
ELSE()
	ADD_DEFINITIONS(-DBOOST_PROGRAM_OPTIONS_STATIC_LINK=1)
ENDIF()

SETUP_BOOST_LIBRARY(${LIB_NAME})
