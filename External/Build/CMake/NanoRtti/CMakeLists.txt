SET(nano_rtti_project_dir "${EXTERNAL_PROJECT_DIR}/NanoRtti")

UpdateExternalLib("NanoRtti" "https://github.com/gongminmin/NanoRtti.git" "0372314282fbce688d19f7f87d15c3e311864b83")

set(NANO_RTTI_BUILD_TESTS OFF CACHE BOOL "" FORCE)

add_subdirectory(${nano_rtti_project_dir} "${CMAKE_CURRENT_BINARY_DIR}/NanoRtti" EXCLUDE_FROM_ALL)

set_target_properties(NanoRtti PROPERTIES
	FOLDER "External/NanoRtti"
)
