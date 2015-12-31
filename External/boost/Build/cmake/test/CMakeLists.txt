SET(LIB_NAME unit_test_framework)

SET(SOURCE_FILES
	${BOOST_PROJECT_DIR}/libs/test/src/compiler_log_formatter.cpp
	${BOOST_PROJECT_DIR}/libs/test/src/debug.cpp
	${BOOST_PROJECT_DIR}/libs/test/src/decorator.cpp
	${BOOST_PROJECT_DIR}/libs/test/src/execution_monitor.cpp
	${BOOST_PROJECT_DIR}/libs/test/src/framework.cpp
	${BOOST_PROJECT_DIR}/libs/test/src/plain_report_formatter.cpp
	${BOOST_PROJECT_DIR}/libs/test/src/progress_monitor.cpp
	${BOOST_PROJECT_DIR}/libs/test/src/results_collector.cpp
	${BOOST_PROJECT_DIR}/libs/test/src/results_reporter.cpp
	${BOOST_PROJECT_DIR}/libs/test/src/test_tools.cpp
	${BOOST_PROJECT_DIR}/libs/test/src/test_tree.cpp
	${BOOST_PROJECT_DIR}/libs/test/src/unit_test_log.cpp
	${BOOST_PROJECT_DIR}/libs/test/src/unit_test_main.cpp
	${BOOST_PROJECT_DIR}/libs/test/src/unit_test_monitor.cpp
	${BOOST_PROJECT_DIR}/libs/test/src/unit_test_parameters.cpp
	${BOOST_PROJECT_DIR}/libs/test/src/xml_log_formatter.cpp
	${BOOST_PROJECT_DIR}/libs/test/src/xml_report_formatter.cpp
)

IF(KLAYGE_PREFERRED_LIB_TYPE STREQUAL "SHARED")
	ADD_DEFINITIONS(-DBOOST_TEST_DYN_LINK=1)
ENDIF()
IF(MSVC)
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4275 /wd4671 /wd4673 /EHa")
ENDIF()

SETUP_BOOST_LIBRARY(${LIB_NAME})
