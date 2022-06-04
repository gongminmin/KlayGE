SET(SEVENZIP_PROJECT_DIR "${EXTERNAL_PROJECT_DIR}/7z")
SET(SEVENZIP_OUTPUT_DIR "${EXTERNAL_PROJECT_DIR}/lib/7z/${KLAYGE_PLATFORM_NAME}")

UpdateExternalLib("7z" "https://github.com/kornelski/7z" "bafac7b19488de73ca8e840f80142bd6679f8c2b" "" "" need_patch)
if(need_patch)
	foreach(patch "0001-Support-UWP" "0002-Fix-compiling-problems-for-Android" "0003-Fix-for-clang-cl")
		ApplyPatch("7z" "${CMAKE_CURRENT_SOURCE_DIR}/Patches/${patch}.patch")
	endforeach()
endif()

ADD_SUBDIRECTORY(7zxa)
ADD_SUBDIRECTORY(LZMA)
