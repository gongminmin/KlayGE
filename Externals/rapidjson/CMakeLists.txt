UpdateExternalLib("rapidjson" "https://github.com/Tencent/rapidjson.git" "6089180ecb704cb2b136777798fa1be303618975")

add_library(rapidjson INTERFACE)
target_include_directories(rapidjson
	INTERFACE
		${CMAKE_CURRENT_SOURCE_DIR}/rapidjson/include
)
