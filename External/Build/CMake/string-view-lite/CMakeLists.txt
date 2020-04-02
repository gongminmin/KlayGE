set(STRING_VIEW_LITE_PROJECT_DIR "${EXTERNAL_PROJECT_DIR}/string-view-lite")

UpdateExternalLib("string-view-lite" "https://github.com/martinmoene/string-view-lite.git" "5990f16094504866c09cd0c97068c3d9b15ae099" "" "" need_patch)
if(need_patch)
	foreach(patch "0001-Fix-operator-on-wstring_view-and-wchar_t-raw-string")
		ApplyPatch("string-view-lite" "${CMAKE_CURRENT_SOURCE_DIR}/Patches/${patch}.patch")
	endforeach()
endif()

set(STRING_VIEW_LITE_OPT_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(STRING_VIEW_LITE_OPT_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(STRING_VIEW_LITE_OPT_CONV_STD_STRING_CLASS_METHODS OFF CACHE BOOL "" FORCE)
set(STRING_VIEW_LITE_OPT_CONV_STD_STRING_FREE_FUNCTIONS OFF CACHE BOOL "" FORCE)
set(STRING_VIEW_LITE_OPT_SELECT_NONSTD ON CACHE BOOL "" FORCE)
set(STRING_VIEW_LITE_OPT_SELECT_STD OFF CACHE BOOL "" FORCE)

add_subdirectory(${STRING_VIEW_LITE_PROJECT_DIR} "${CMAKE_CURRENT_BINARY_DIR}/string-view-lite" EXCLUDE_FROM_ALL)
