SET(SEVENZIP_PROJECT_DIR "${EXTERNAL_PROJECT_DIR}/7z")

UpdateExternalLib("7z" "https://github.com/kornelski/7z" "c3529a41f527101f05e9e834a19205ee33a3b097" "" "" need_patch)
if(need_patch)
	foreach(patch "0001-Support-UWP" "0002-Fix-for-clang-cl" "0003-Fix-for-Android-build")
		ApplyPatch("7z" "${CMAKE_CURRENT_SOURCE_DIR}/Patches/${patch}.patch")
	endforeach()
endif()

ADD_SUBDIRECTORY(7zxa)
ADD_SUBDIRECTORY(LZMA)
