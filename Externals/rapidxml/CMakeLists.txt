UpdateExternalLib("rapidxml" "https://github.com/valnoel/rapidxml.git" "34ad52ab3f066ed598acb675a315709c4082ffb5")

add_library(rapidxml INTERFACE)
target_include_directories(rapidxml
	INTERFACE
		${CMAKE_CURRENT_SOURCE_DIR}/rapidxml
)
