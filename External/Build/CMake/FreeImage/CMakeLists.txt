SET(FREEIMAGE_PROJECT_DIR "${EXTERNAL_PROJECT_DIR}/FreeImage")
SET(FREEIMAGE_OUTPUT_DIR "${EXTERNAL_PROJECT_DIR}/lib/FreeImage/${KLAYGE_PLATFORM_NAME}")

UpdateExternalLib("FreeImage" "https://github.com/gongminmin/FreeImage.git" "7c453922d9442cd1f03a751d1f9273236405f9b7" "master" "3.18.0" need_patch)
if(need_patch)
	foreach(patch "0001-Fix-compiling-problems-on-VS2017-15.3-with-std-c-17" "0002-Turn-off-SSE2-for-arm-and-arm64-on-msvc"
		"0003-Fix-some-compiling-problems-on-MinGW-7.x" "0004-Fix-beta-for-GCC8" "0005-Fix-missing-__mingw_uuidof_s-problem-on-gcc-8.3"
		"0006-Fix-compiling-problems-on-VS2019-Update-4" "0007-Fix-for-clang-cl")
		ApplyPatch("FreeImage" "${CMAKE_CURRENT_SOURCE_DIR}/Patches/${patch}.patch")
	endforeach()
endif()

IF(KLAYGE_COMPILER_MSVC)
	STRING(REPLACE "/WX" "/WX:NO" CMAKE_STATIC_LINKER_FLAGS ${CMAKE_STATIC_LINKER_FLAGS})
ELSEIF(KLAYGE_COMPILER_CLANG)
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-register -Wno-c++11-narrowing")
elseif(KLAYGE_COMPILER_CLANGCL)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -w")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-register -w")
endif()
FOREACH(flag_var
		CMAKE_CXX_FLAGS_RELEASE CMAKE_CXX_FLAGS_RELWITHDEBINFO CMAKE_CXX_FLAGS_MINSIZEREL)
	STRING(REPLACE "${NO_RTTI_FLAG}" "${RTTI_FLAG}" ${flag_var} "${${flag_var}}")
ENDFOREACH()

ADD_SUBDIRECTORY(FreeImage)
ADD_SUBDIRECTORY(LibJPEG)
ADD_SUBDIRECTORY(LibJXR)
ADD_SUBDIRECTORY(LibOpenJPEG)
ADD_SUBDIRECTORY(LibPNG)
ADD_SUBDIRECTORY(LibRaw)
ADD_SUBDIRECTORY(LibTIFF4)
ADD_SUBDIRECTORY(LibWebP)
ADD_SUBDIRECTORY(OpenEXR)

foreach(target
	"FreeImage" "LibJPEG" "LibJXR" "LibOpenJPEG" "LibPNG" "LibRaw" "LibTIFF4" "LibWebP" "OpenEXR")
	if(KLAYGE_PLATFORM_WINDOWS)
		target_compile_definitions(${target}
			PRIVATE
				-DWIN32
		)
	else()
		target_compile_definitions(${target}
			PRIVATE
				-D__ANSI__
		)
	endif()

	set_target_properties(${target} PROPERTIES FOLDER "${EXTERNAL_VS_FOLDER_PREFIX}/FreeImage")
endforeach()
