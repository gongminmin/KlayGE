UpdateExternalLib("NanoRtti" "https://github.com/gongminmin/NanoRtti.git" "de403e1698bf5e958269facc04a6e0f88053bf36")

set(NANO_RTTI_BUILD_TESTS OFF CACHE BOOL "" FORCE)

add_subdirectory(NanoRtti)

set_target_properties(NanoRtti PROPERTIES
	FOLDER "Externals/NanoRtti"
)
