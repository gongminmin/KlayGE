set(RAPIDJSON_PROJECT_DIR "${EXTERNAL_PROJECT_DIR}/rapidjson")

UpdateExternalLib("rapidjson" "https://github.com/Tencent/rapidjson.git" "d87b698d0fcc10a5f632ecbc80a9cb2a8fa094a5")

add_library(rapidjson INTERFACE)
target_include_directories(rapidjson
	INTERFACE
		${RAPIDJSON_PROJECT_DIR}/include
)
