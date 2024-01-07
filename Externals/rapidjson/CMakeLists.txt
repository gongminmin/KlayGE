UpdateExternalLib("rapidjson" "https://github.com/Tencent/rapidjson.git" "0ccdbf364c577803e2a751f5aededce935314313")

add_library(rapidjson INTERFACE)
target_include_directories(rapidjson
	INTERFACE
		${CMAKE_CURRENT_SOURCE_DIR}/rapidjson/include
)
