SET(nano_rtti_project_dir "${EXTERNAL_PROJECT_DIR}/NanoRtti")

UpdateExternalLib("NanoRtti" "https://github.com/gongminmin/NanoRtti.git" "869432bfcbc95e2b7b8dd1a61e997e6a9db85d17")

set(NANO_RTTI_BUILD_TESTS OFF CACHE BOOL "" FORCE)

add_subdirectory(${nano_rtti_project_dir} "${CMAKE_CURRENT_BINARY_DIR}/NanoRtti" EXCLUDE_FROM_ALL)

set_target_properties(NanoRtti PROPERTIES
	FOLDER "External/NanoRtti"
)
