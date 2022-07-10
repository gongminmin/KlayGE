set(RAPIDXML_PROJECT_DIR "${EXTERNAL_PROJECT_DIR}/rapidxml")

UpdateExternalLib("rapidxml" "https://github.com/valnoel/rapidxml.git" "34ad52ab3f066ed598acb675a315709c4082ffb5")

add_library(rapidxml INTERFACE)
target_include_directories(rapidxml
	INTERFACE
		${RAPIDXML_PROJECT_DIR}
)
