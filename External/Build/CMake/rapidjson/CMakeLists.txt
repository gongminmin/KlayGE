set(RAPIDJSON_PROJECT_DIR "${EXTERNAL_PROJECT_DIR}/rapidjson")

UpdateExternalLib("rapidjson" "https://github.com/Tencent/rapidjson.git" "0ccdbf364c577803e2a751f5aededce935314313")

add_library(rapidjson INTERFACE)
target_include_directories(rapidjson
	INTERFACE
		${RAPIDJSON_PROJECT_DIR}/include
)
