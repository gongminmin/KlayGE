SET(LIB_NAME date_time)

SET(SOURCE_FILES
	${BOOST_PROJECT_DIR}/libs/date_time/src/gregorian/greg_month.cpp
	${BOOST_PROJECT_DIR}/libs/date_time/src/gregorian/greg_weekday.cpp
	${BOOST_PROJECT_DIR}/libs/date_time/src/gregorian/date_generators.cpp
)

ADD_DEFINITIONS(-DDATE_TIME_INLINE)
IF(KLAYGE_PREFERRED_LIB_TYPE STREQUAL "SHARED")
	ADD_DEFINITIONS(-DBOOST_DATE_TIME_DYN_LINK=1)
ELSE()
	ADD_DEFINITIONS(-DBOOST_DATE_TIME_STATIC_LINK=1)
ENDIF()

SETUP_BOOST_LIBRARY(${LIB_NAME})
