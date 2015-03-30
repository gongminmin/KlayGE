SET(LIB_NAME regex)

SET(SOURCE_FILES
	${BOOST_PROJECT_DIR}/libs/regex/src/c_regex_traits.cpp
	${BOOST_PROJECT_DIR}/libs/regex/src/cpp_regex_traits.cpp
	${BOOST_PROJECT_DIR}/libs/regex/src/cregex.cpp
	${BOOST_PROJECT_DIR}/libs/regex/src/fileiter.cpp
	${BOOST_PROJECT_DIR}/libs/regex/src/icu.cpp
	${BOOST_PROJECT_DIR}/libs/regex/src/instances.cpp
	${BOOST_PROJECT_DIR}/libs/regex/src/posix_api.cpp
	${BOOST_PROJECT_DIR}/libs/regex/src/regex.cpp
	${BOOST_PROJECT_DIR}/libs/regex/src/regex_debug.cpp
	${BOOST_PROJECT_DIR}/libs/regex/src/regex_raw_buffer.cpp
	${BOOST_PROJECT_DIR}/libs/regex/src/regex_traits_defaults.cpp
	${BOOST_PROJECT_DIR}/libs/regex/src/static_mutex.cpp
	${BOOST_PROJECT_DIR}/libs/regex/src/w32_regex_traits.cpp
	${BOOST_PROJECT_DIR}/libs/regex/src/wc_regex_traits.cpp
	${BOOST_PROJECT_DIR}/libs/regex/src/wide_posix_api.cpp
	${BOOST_PROJECT_DIR}/libs/regex/src/winstances.cpp
	${BOOST_PROJECT_DIR}/libs/regex/src/usinstances.cpp
)

IF(KLAYGE_PREFERRED_LIB_TYPE STREQUAL "SHARED")
	ADD_DEFINITIONS(-DBOOST_REGEX_DYN_LINK=1)
ENDIF()

SETUP_BOOST_LIBRARY(${LIB_NAME})
