ADD_DEFINITIONS(-DUNICODE -D_UNICODE)

if(MSVC)
	set(CMAKE_CXX_FLAGS "/Wall /WX /EHsc /MP /bigobj /Zc:strictStrings /Zc:rvalueCast /Gw")

	if(CMAKE_C_COMPILER_ID MATCHES Clang)
		set(KLAYGE_COMPILER_NAME "clangcl")
		set(KLAYGE_COMPILER_CLANGCL TRUE)

		execute_process(COMMAND ${CMAKE_C_COMPILER} --version OUTPUT_VARIABLE CLANG_VERSION)
		string(REGEX MATCHALL "[0-9]+" CLANG_VERSION_COMPONENTS ${CLANG_VERSION})
		list(GET CLANG_VERSION_COMPONENTS 0 CLANG_MAJOR)
		list(GET CLANG_VERSION_COMPONENTS 1 CLANG_MINOR)
		set(KLAYGE_COMPILER_VERSION ${CLANG_MAJOR}${CLANG_MINOR})
		if(KLAYGE_COMPILER_VERSION LESS "90")
			message(FATAL_ERROR "Unsupported compiler version. Please install clang-cl 9.0 or up.")
		endif()

		set(CMAKE_CXX_STANDARD 17)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")

		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-c++98-compat") # No need to compatible to C++98
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-c++98-compat-pedantic")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-cast-align")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-covered-switch-default")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-dynamic-exception-spec")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-documentation")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-documentation-unknown-command")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-double-promotion")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-exit-time-destructors")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-extra-semi")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-extra-semi-stmt")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-float-equal")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-global-constructors")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-gnu-anonymous-struct")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-implicit-int-conversion")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-implicit-int-float-conversion")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-implicit-float-conversion")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-inconsistent-missing-destructor-override")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-inconsistent-missing-override")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-language-extension-token")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-reserved-id-macro") # Allow macros with __ prefix
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-microsoft-enum-value")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-microsoft-exception-spec")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-missing-noreturn")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-missing-prototypes")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-missing-variable-declarations")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-nested-anon-types")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-nonportable-system-include-path") # Allow windows.h
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-non-virtual-dtor")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-old-style-cast")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-return-std-move-in-c++11")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-sign-conversion")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-signed-enum-bitfield")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-switch-enum") # NEVER turn it on. A bad designed warning.
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-undef") # Ignore undefined macros
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-undefined-func-template") # clang-cl couldn't understand explicit specialization from other compile units
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-zero-as-null-pointer-constant")
		if(KLAYGE_COMPILER_VERSION GREATER_EQUAL "110")
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-suggest-destructor-override")
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-suggest-override")
		endif()

		set(CMAKE_C_FLAGS "/Wall /WX /bigobj /Gw")
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-covered-switch-default")
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-documentation")
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-documentation-unknown-command")
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-double-promotion")
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-extra-semi-stmt")
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-float-equal")
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-implicit-int-conversion")
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-implicit-float-conversion")
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-language-extension-token")
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-reserved-id-macro") # Allow macros with __ prefix
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-sign-conversion")
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-switch-enum") # NEVER turn it on. A bad designed warning.
	else()
		SET(KLAYGE_COMPILER_NAME "vc")
		SET(KLAYGE_COMPILER_MSVC TRUE)
		if(MSVC_VERSION GREATER_EQUAL 1930)
			set(KLAYGE_COMPILER_VERSION "143")
		elseif(MSVC_VERSION GREATER_EQUAL 1920)
			SET(KLAYGE_COMPILER_VERSION "142")
		elseif(MSVC_VERSION GREATER_EQUAL 1911)
			SET(KLAYGE_COMPILER_VERSION "141")
		else()
			message(FATAL_ERROR "Unsupported compiler version. Please install VS2017 15.3 or up.")
		ENDIF()

		if(MSVC_VERSION GREATER_EQUAL 1929)
			set(CMAKE_CXX_STANDARD 20)
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++20")
		else()
			set(CMAKE_CXX_STANDARD 17)
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")
		endif()

		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:throwingNew /permissive-")
		IF(KLAYGE_PLATFORM_WINDOWS_STORE OR (KLAYGE_ARCH_NAME STREQUAL "arm64"))
			SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:twoPhase-")
		ENDIF()
		IF(MSVC_VERSION GREATER 1912)
			SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:externConstexpr")
		ENDIF()

		IF((MSVC_VERSION GREATER 1912) AND (KLAYGE_ARCH_NAME STREQUAL "x64"))
			#SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Qspectre")
		ENDIF()
		if(MSVC_VERSION GREATER 1915)
			set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /JMC")
		endif()
		
		FOREACH(flag_var
			CMAKE_EXE_LINKER_FLAGS CMAKE_SHARED_LINKER_FLAGS CMAKE_MODULE_LINKER_FLAGS)
			SET(${flag_var} "/WX /pdbcompress")
		ENDFOREACH()
		FOREACH(flag_var
			CMAKE_EXE_LINKER_FLAGS_DEBUG CMAKE_SHARED_LINKER_FLAGS_DEBUG)
			SET(${flag_var} "/DEBUG:FASTLINK")
		ENDFOREACH()
		FOREACH(flag_var
			CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO)
			SET(${flag_var} "/DEBUG:FASTLINK /INCREMENTAL:NO /LTCG:incremental /OPT:REF /OPT:ICF")
		ENDFOREACH()
		FOREACH(flag_var
			CMAKE_EXE_LINKER_FLAGS_MINSIZEREL CMAKE_SHARED_LINKER_FLAGS_MINSIZEREL CMAKE_EXE_LINKER_FLAGS_RELEASE CMAKE_SHARED_LINKER_FLAGS_RELEASE)
			SET(${flag_var} "/INCREMENTAL:NO /LTCG /OPT:REF /OPT:ICF")
		ENDFOREACH()
		FOREACH(flag_var
			CMAKE_MODULE_LINKER_FLAGS_RELEASE CMAKE_MODULE_LINKER_FLAGS_MINSIZEREL)
			SET(${flag_var} "/INCREMENTAL:NO /LTCG")
		ENDFOREACH()
		FOREACH(flag_var
			CMAKE_STATIC_LINKER_FLAGS_RELEASE CMAKE_STATIC_LINKER_FLAGS_MINSIZEREL)
			SET(${flag_var} "${${flag_var}} /LTCG")
		ENDFOREACH()
		SET(CMAKE_STATIC_LINKER_FLAGS_RELWITHDEBINFO "${CMAKE_STATIC_LINKER_FLAGS_RELWITHDEBINFO} /LTCG:incremental")

		IF(KLAYGE_PLATFORM_WINDOWS_STORE)
			FOREACH(flag_var
				CMAKE_EXE_LINKER_FLAGS_DEBUG CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO CMAKE_SHARED_LINKER_FLAGS_DEBUG CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO)
				SET(${flag_var} "${${flag_var}} /INCREMENTAL:NO")
			ENDFOREACH()
		ELSE()
			FOREACH(flag_var
				CMAKE_CXX_FLAGS_RELEASE CMAKE_CXX_FLAGS_RELWITHDEBINFO CMAKE_CXX_FLAGS_MINSIZEREL)
				SET(${flag_var} "${${flag_var}} /GS-")
			ENDFOREACH()

			SET(CMAKE_STATIC_LINKER_FLAGS "/WX")
		ENDIF()

		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4061") # False positive on missing enum in switch case (This is a bad designed warning because there are "default"s to handle un-cased enums)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4255") # Allow func() to func(void) in some Windows SDK
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4365") # Ignore int to size_t
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4464") # Allow .. in include path
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4514") # Allow unused inline function
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4623") # Ignore implicitly deleted default constructor
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4625") # Ignore implicitly deleted copy constructor
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4626") # Ignore implicitly deleted copy operator=
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4668") # Undefined macro as 0
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4710") # Allow function with the inline mark not be inlined
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4711") # Allow function to be inlined without the inline mark
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4820") # Ignore padding
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd5026") # Ignore implicitly deleted move constructor
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd5027") # Ignore implicitly deleted move operator=
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd5039") # Ignore passing a throwing function to C functions
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd5045") # False positive on range check
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd5219") # Ignore int to uint
		if(MSVC_VERSION GREATER_EQUAL 1920)
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd5204") # Ignore non trivial destructor in COM interfaces and ppl
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd5220") # Ignore non trivial constructor in ppl
		else()
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4472") # Native and managed enum in vcruntime
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4571") # Allow catching SEH in STL
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4582") # std::optional constructor is not implicitly called
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4583") # std::optional destructor is not implicitly called
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4623") # Default constructor was implicitly defined as deleted in STL
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4774") # Ignore wrong parameter types of printf in STL
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd5039") # Allow passing throwing function to extern C function
		endif()

		SET(CMAKE_C_FLAGS ${CMAKE_CXX_FLAGS})
	endif()

	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /DKLAYGE_SHIP")
	foreach(flag_var
		CMAKE_CXX_FLAGS_RELEASE CMAKE_CXX_FLAGS_RELWITHDEBINFO CMAKE_CXX_FLAGS_MINSIZEREL)
		set(${flag_var} "${${flag_var}} /fp:fast /Ob2 /GL /Qpar")
	endforeach()
	IF(KLAYGE_ARCH_NAME MATCHES "x86")
		FOREACH(flag_var
			CMAKE_CXX_FLAGS_RELEASE CMAKE_CXX_FLAGS_RELWITHDEBINFO CMAKE_CXX_FLAGS_MINSIZEREL)
			SET(${flag_var} "${${flag_var}} /arch:SSE")
		ENDFOREACH()
		FOREACH(flag_var
			CMAKE_EXE_LINKER_FLAGS CMAKE_SHARED_LINKER_FLAGS)
			SET(${flag_var} "${${flag_var}} /LARGEADDRESSAWARE")
		ENDFOREACH()
	ENDIF()

	ADD_DEFINITIONS(-DWIN32 -D_WINDOWS)
	IF(KLAYGE_ARCH_NAME MATCHES "arm")
		ADD_DEFINITIONS(-D_ARM_WINAPI_PARTITION_DESKTOP_SDK_AVAILABLE=1)

		IF(KLAYGE_PLATFORM_WINDOWS_DESKTOP)
			FOREACH(flag_var
				CMAKE_C_STANDARD_LIBRARIES CMAKE_CXX_STANDARD_LIBRARIES)
				SET(${flag_var} "${${flag_var}} gdi32.lib ole32.lib oleaut32.lib comdlg32.lib advapi32.lib shell32.lib")
			ENDFOREACH()
		ENDIF()
	ENDIF()
ELSE()
	IF(CMAKE_C_COMPILER_ID MATCHES Clang)
		SET(KLAYGE_COMPILER_NAME "clang")
		SET(KLAYGE_COMPILER_CLANG TRUE)
	ELSEIF(MINGW)
		SET(KLAYGE_COMPILER_NAME "mgw")
		SET(KLAYGE_COMPILER_GCC TRUE)
	ELSE()
		SET(KLAYGE_COMPILER_NAME "gcc")
		SET(KLAYGE_COMPILER_GCC TRUE)
	ENDIF()
	IF(KLAYGE_PLATFORM_WINDOWS)
		ADD_DEFINITIONS(-D_WIN32_WINNT=0x0601 -DWINVER=_WIN32_WINNT)
	ENDIF()

	IF(KLAYGE_COMPILER_CLANG)
		EXECUTE_PROCESS(COMMAND ${CMAKE_C_COMPILER} --version OUTPUT_VARIABLE CLANG_VERSION)
		set(CLANG_VERSION_TOKENS ${CLANG_VERSION})
		separate_arguments(CLANG_VERSION_TOKENS)
		list(LENGTH CLANG_VERSION_TOKENS len)
		list(FIND CLANG_VERSION_TOKENS "version" pos)
		math(EXPR pos "${pos}+1")
		list(GET CLANG_VERSION_TOKENS ${pos} CLANG_VERSION_COMPONENTS)
		string(REPLACE "." " " CLANG_VERSION_COMPONENTS ${CLANG_VERSION_COMPONENTS})
		separate_arguments(CLANG_VERSION_COMPONENTS)
		LIST(GET CLANG_VERSION_COMPONENTS 0 CLANG_MAJOR)
		LIST(GET CLANG_VERSION_COMPONENTS 1 CLANG_MINOR)
		SET(KLAYGE_COMPILER_VERSION ${CLANG_MAJOR}${CLANG_MINOR})
		if(KLAYGE_PLATFORM_DARWIN OR KLAYGE_PLATFORM_IOS)
			if(KLAYGE_COMPILER_VERSION LESS "110")
				message(FATAL_ERROR "Unsupported compiler version. Please install Apple clang++ 11 or up.")
			endif()
		elseif(KLAYGE_PLATFORM_ANDROID)
			if(KLAYGE_COMPILER_VERSION LESS "60")
				message(FATAL_ERROR "Unsupported compiler version. Please install clang++ 6.0 (NDK 17) or up.")
			endif()
		elseif(KLAYGE_PLATFORM_LINUX)
			if(KLAYGE_COMPILER_VERSION LESS "100")
				message(FATAL_ERROR "Unsupported compiler version. Please install clang++ 10.0 or up.")
			endif()
		endif()
	ELSE()
		EXECUTE_PROCESS(COMMAND ${CMAKE_C_COMPILER} -dumpfullversion OUTPUT_VARIABLE GCC_VERSION)
		STRING(REGEX MATCHALL "[0-9]+" GCC_VERSION_COMPONENTS ${GCC_VERSION})
		LIST(GET GCC_VERSION_COMPONENTS 0 GCC_MAJOR)
		LIST(GET GCC_VERSION_COMPONENTS 1 GCC_MINOR)
		SET(KLAYGE_COMPILER_VERSION ${GCC_MAJOR}${GCC_MINOR})
		if(KLAYGE_COMPILER_VERSION LESS "90")
			message(FATAL_ERROR "Unsupported compiler version. Please install g++ 9.0 or up.")
		endif()
	ENDIF()

	FOREACH(flag_var
		CMAKE_C_FLAGS CMAKE_CXX_FLAGS)
		SET(${flag_var} "${${flag_var}} -W -Wall -Werror -fpic")
		IF(NOT (ANDROID OR IOS))
			SET(${flag_var} "${${flag_var}} -march=core2 -msse2")
		ENDIF()
		IF(KLAYGE_COMPILER_CLANG AND (KLAYGE_PLATFORM_DARWIN OR KLAYGE_PLATFORM_IOS))
			SET(${flag_var} "${${flag_var}} -fno-asm-blocks")
		ENDIF()
	ENDFOREACH()
	set(CMAKE_C_STANDARD 11)
	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11")
	if(KLAYGE_PLATFORM_ANDROID AND (KLAYGE_COMPILER_VERSION LESS 100))
		set(CMAKE_CXX_STANDARD 17)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1z")
	else()
		set(CMAKE_CXX_STANDARD 20)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++2a")
	endif()
	IF(KLAYGE_COMPILER_CLANG)
		IF(KLAYGE_PLATFORM_LINUX)
			SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
			set(CMAKE_LINKER "lld")
			foreach(flag_var
				CMAKE_SHARED_LINKER_FLAGS CMAKE_MODULE_LINKER_FLAGS CMAKE_EXE_LINKER_FLAGS)
				SET(${flag_var} "${${flag_var}} -lc++abi")
			endforeach()
		ENDIF()
	ELSEIF(MINGW)
		FOREACH(flag_var
			CMAKE_C_FLAGS CMAKE_CXX_FLAGS)
			SET(${flag_var} "${${flag_var}} -Wa,-mbig-obj")
		ENDFOREACH()
	ENDIF()
	SET(CMAKE_CXX_FLAGS_DEBUG "-DDEBUG -g -O0")
	SET(CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG -O2 -DKLAYGE_SHIP")
	SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-DNDEBUG -g -O2")
	SET(CMAKE_CXX_FLAGS_MINSIZEREL "-DNDEBUG -Os")
	IF(KLAYGE_ARCH_NAME STREQUAL "x86")
		FOREACH(flag_var
			CMAKE_C_FLAGS CMAKE_CXX_FLAGS)
			SET(${flag_var} "${${flag_var}} -m32")
		ENDFOREACH()
		FOREACH(flag_var
			CMAKE_SHARED_LINKER_FLAGS CMAKE_MODULE_LINKER_FLAGS CMAKE_EXE_LINKER_FLAGS)
			SET(${flag_var} "${${flag_var}} -m32")
			IF(KLAYGE_PLATFORM_WINDOWS)
				SET(${flag_var} "${${flag_var}} -Wl,--large-address-aware")
			ENDIF()
		ENDFOREACH()
		IF(KLAYGE_PLATFORM_WINDOWS)
			SET(CMAKE_RC_FLAGS "${CMAKE_RC_FLAGS} --target=pe-i386")
		ELSE()
			SET(CMAKE_RC_FLAGS "${CMAKE_RC_FLAGS} --target=elf32-i386")
		ENDIF()
	ELSEIF((KLAYGE_ARCH_NAME STREQUAL "x64") OR (KLAYGE_ARCH_NAME STREQUAL "x86_64"))
		FOREACH(flag_var
			CMAKE_C_FLAGS CMAKE_CXX_FLAGS)
			SET(${flag_var} "${${flag_var}} -m64")
		ENDFOREACH()
		FOREACH(flag_var
			CMAKE_SHARED_LINKER_FLAGS CMAKE_MODULE_LINKER_FLAGS CMAKE_EXE_LINKER_FLAGS)
			SET(${flag_var} "${${flag_var}} -m64")
		ENDFOREACH()
		IF(KLAYGE_PLATFORM_WINDOWS)
			SET(CMAKE_RC_FLAGS "${CMAKE_RC_FLAGS} --target=pe-x86-64")
		ELSE()
			SET(CMAKE_RC_FLAGS "${CMAKE_RC_FLAGS} --target=elf64-x86-64")
		ENDIF()
	ENDIF()
	IF(NOT KLAYGE_HOST_PLATFORM_DARWIN)
		FOREACH(flag_var
			CMAKE_SHARED_LINKER_FLAGS_RELEASE CMAKE_SHARED_LINKER_FLAGS_MINSIZEREL
			CMAKE_MODULE_LINKER_FLAGS_RELEASE CMAKE_MODULE_LINKER_FLAGS_MINSIZEREL
			CMAKE_EXE_LINKER_FLAGS_RELEASE CMAKE_EXE_LINKER_FLAGS_MINSIZEREL)
			SET(${flag_var} "-s")
		ENDFOREACH()
	ENDIF()
ENDIF()

SET(CMAKE_C_FLAGS_DEBUG ${CMAKE_CXX_FLAGS_DEBUG})
SET(CMAKE_C_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_RELEASE})
SET(CMAKE_C_FLAGS_RELWITHDEBINFO ${CMAKE_CXX_FLAGS_RELWITHDEBINFO})
SET(CMAKE_C_FLAGS_MINSIZEREL ${CMAKE_CXX_FLAGS_MINSIZEREL})
IF(KLAYGE_COMPILER_MSVC OR KLAYGE_COMPILER_CLANGCL)
	SET(RTTI_FLAG "/GR")
	SET(NO_RTTI_FLAG "/GR-")
ELSE()
	SET(RTTI_FLAG "-frtti")
	SET(NO_RTTI_FLAG "-fno-rtti")
ENDIF()
SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${RTTI_FLAG}")
FOREACH(flag_var
	CMAKE_CXX_FLAGS_RELEASE CMAKE_CXX_FLAGS_RELWITHDEBINFO CMAKE_CXX_FLAGS_MINSIZEREL)
	SET(${flag_var} "${${flag_var}} ${NO_RTTI_FLAG}")
ENDFOREACH()

SET(KLAYGE_OUTPUT_SUFFIX _${KLAYGE_COMPILER_NAME}${KLAYGE_COMPILER_VERSION})

set(CMAKE_CXX_STANDARD_REQUIRED ON)

IF(MSVC)
	# create vcproj.user file for Visual Studio to set debug working directory
	FUNCTION(CREATE_VCPROJ_USERFILE TARGETNAME)
		SET(SYSTEM_NAME $ENV{USERDOMAIN})
		SET(USER_NAME $ENV{USERNAME})

		CONFIGURE_FILE(
			${KLAYGE_CMAKE_MODULE_DIR}/VisualStudio2010UserFile.vcxproj.user.in
			${CMAKE_CURRENT_BINARY_DIR}/${TARGETNAME}.vcxproj.user
			@ONLY
		)
	ENDFUNCTION()
ELSEIF(KLAYGE_PLATFORM_DARWIN OR KLAYGE_PLATFORM_IOS)
	# create .xcscheme file for Xcode to set debug working directory
	FUNCTION(CREATE_XCODE_USERFILE PROJECTNAME TARGETNAME)
		IF(KLAYGE_PLATFORM_DARWIN OR KLAYGE_PLATFORM_IOS)
			SET(SYSTEM_NAME $ENV{USERDOMAIN})
			SET(USER_NAME $ENV{USER})

			CONFIGURE_FILE(
				${KLAYGE_CMAKE_MODULE_DIR}/xcode.xcscheme.in
				${PROJECT_BINARY_DIR}/${PROJECTNAME}.xcodeproj/xcuserdata/${USER_NAME}.xcuserdatad/xcschemes/${TARGETNAME}.xcscheme
				@ONLY
			)
		ENDIF()
	ENDFUNCTION()
ENDIF()
	
FUNCTION(CREATE_PROJECT_USERFILE PROJECTNAME TARGETNAME)
	IF(MSVC)
		CREATE_VCPROJ_USERFILE(${TARGETNAME})
	ELSEIF(KLAYGE_PLATFORM_DARWIN OR KLAYGE_PLATFORM_IOS)
		CREATE_XCODE_USERFILE(${PROJECTNAME} ${TARGETNAME})
	ENDIF()
ENDFUNCTION()

FUNCTION(KLAYGE_ADD_PRECOMPILED_HEADER TARGET_NAME PRECOMPILED_HEADER)
	IF(KLAYGE_COMPILER_MSVC OR KLAYGE_COMPILER_CLANGCL)
		IF(CMAKE_GENERATOR MATCHES "^Visual Studio")
			KLAYGE_ADD_MSVC_PRECOMPILED_HEADER(${TARGET_NAME} ${PRECOMPILED_HEADER})
		ENDIF()
	ELSEIF(KLAYGE_PLATFORM_DARWIN OR KLAYGE_PLATFORM_IOS)
		KLAYGE_ADD_XCODE_PRECOMPILED_HEADER(${TARGET_NAME} ${PRECOMPILED_HEADER})
	ELSE()
		KLAYGE_ADD_GCC_PRECOMPILED_HEADER(${TARGET_NAME} ${PRECOMPILED_HEADER})
	ENDIF()
ENDFUNCTION()

FUNCTION(KLAYGE_ADD_MSVC_PRECOMPILED_HEADER TARGET_NAME PRECOMPILED_HEADER)
	get_filename_component(pch_base_name ${PRECOMPILED_HEADER} NAME_WE)
	get_filename_component(pch_output "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/${pch_base_name}.pch" ABSOLUTE)

	get_filename_component(pch_header "${PRECOMPILED_HEADER}" ABSOLUTE)

	get_target_property(source_list ${TARGET_NAME} SOURCES)
	set(cpp_source_list "")
	foreach(file_name ${source_list})
		string(TOLOWER ${file_name} lower_file_name)
		string(FIND "${lower_file_name}" ".cpp" is_cpp REVERSE)
		if(is_cpp LESS 0)
			set_source_files_properties(${file_name} PROPERTIES COMPILE_FLAGS "/Y-")
		else()
			list(APPEND cpp_source_list "${file_name}")
			set_source_files_properties(${file_name} PROPERTIES COMPILE_FLAGS "/FI\"${pch_header}\"")
		endif()
	endforeach()
	list(GET cpp_source_list 0 first_cpp_file)
	set_source_files_properties(${first_cpp_file} PROPERTIES COMPILE_FLAGS "/Yc\"${pch_header}\" /FI\"${pch_header}\""
		OBJECT_OUTPUTS "${pch_output}")
	set_target_properties(${TARGET_NAME} PROPERTIES COMPILE_FLAGS "/Yu\"${pch_header}\" /Fp\"${pch_output}\""
		OBJECT_DEPENDS "${pch_output}")
ENDFUNCTION()

FUNCTION(KLAYGE_ADD_GCC_PRECOMPILED_HEADER TARGET_NAME PRECOMPILED_HEADER)
	SET(CXX_COMPILE_FLAGS ${CMAKE_CXX_FLAGS})
	IF(CMAKE_BUILD_TYPE)
		STRING(TOUPPER ${CMAKE_BUILD_TYPE} UPPER_CMAKE_BUILD_TYPE)
		LIST(APPEND CXX_COMPILE_FLAGS ${CMAKE_CXX_FLAGS_${UPPER_CMAKE_BUILD_TYPE}})
	ENDIF()

	GET_FILENAME_COMPONENT(PRECOMPILED_HEADER_NAME ${PRECOMPILED_HEADER} NAME)

	SET(PCH_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${PRECOMPILED_HEADER_NAME}.gch")

	set(include_dirs "$<TARGET_PROPERTY:${TARGET_NAME},INCLUDE_DIRECTORIES>")
	set(comp_defs "$<TARGET_PROPERTY:${TARGET_NAME},COMPILE_DEFINITIONS>")
	set(comp_flags "$<TARGET_PROPERTY:${TARGET_NAME},COMPILE_FLAGS>")
	set(comp_options "$<TARGET_PROPERTY:${TARGET_NAME},COMPILE_OPTIONS>")
	set(include_dirs "$<$<BOOL:${include_dirs}>:-I$<JOIN:${include_dirs},\n-I>\n>")
	set(comp_defs "$<$<BOOL:${comp_defs}>:-D$<JOIN:${comp_defs},\n-D>\n>")
	set(comp_flags "$<$<BOOL:${comp_flags}>:$<JOIN:${comp_flags},\n>\n>")
	set(comp_options "$<$<BOOL:${comp_options}>:$<JOIN:${comp_options},\n>\n>")
	set(pch_flags_file "${CMAKE_CURRENT_BINARY_DIR}/compile_flags.rsp")
	file(GENERATE OUTPUT "${pch_flags_file}" CONTENT "${comp_defs}${include_dirs}${comp_flags}${comp_options}\n")
	set(pch_compile_flags "@${pch_flags_file}")

	if(DEFINED CMAKE_CXX_COMPILER_TARGET)
		list(APPEND CXX_COMPILE_FLAGS "--target=${CMAKE_CXX_COMPILER_TARGET}")
	endif()
	GET_TARGET_PROPERTY(PIC_OPTION ${TARGET_NAME} POSITION_INDEPENDENT_CODE)
	IF(PIC_OPTION AND CMAKE_CXX_COMPILE_OPTIONS_PIC)
		LIST(APPEND CXX_COMPILE_FLAGS "${CMAKE_CXX_COMPILE_OPTIONS_PIC}")
	ENDIF()

	list(APPEND CXX_COMPILE_FLAGS "-Wno-error -x c++-header")
	if(KLAYGE_COMPILER_CLANG)
		list(APPEND CXX_COMPILE_FLAGS "-Wno-pragma-once-outside-header")
	endif()

	SEPARATE_ARGUMENTS(CXX_COMPILE_FLAGS)

	ADD_CUSTOM_COMMAND(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${PRECOMPILED_HEADER_NAME}
		COMMAND ${CMAKE_COMMAND} -E copy ${PRECOMPILED_HEADER} ${CMAKE_CURRENT_BINARY_DIR}/${PRECOMPILED_HEADER_NAME}
	)

	SET(PCHOUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${PRECOMPILED_HEADER_NAME})

	ADD_CUSTOM_COMMAND(OUTPUT ${PCH_OUTPUT}
		COMMAND ${CMAKE_CXX_COMPILER} ${pch_compile_flags} ${CXX_COMPILE_FLAGS} -o ${PCH_OUTPUT} ${PRECOMPILED_HEADER}
		DEPENDS ${PRECOMPILED_HEADER} ${CMAKE_CURRENT_BINARY_DIR}/${PRECOMPILED_HEADER_NAME}
	)
	ADD_CUSTOM_TARGET(${TARGET_NAME}_gch
		DEPENDS ${PCH_OUTPUT}
	)
	ADD_DEPENDENCIES(${TARGET_NAME} ${TARGET_NAME}_gch)

	get_property(source_list TARGET ${TARGET_NAME} PROPERTY SOURCES)
	foreach(file_name ${source_list})
		string(TOLOWER ${file_name} lower_file_name)
		string(FIND "${lower_file_name}" ".cpp" is_cpp REVERSE)
		if(is_cpp GREATER 0)
			set_source_files_properties(${_source} PROPERTIES
				COMPILE_FLAGS "-include ${CMAKE_CURRENT_BINARY_DIR}/${PRECOMPILED_HEADER_NAME} -Winvalid-pch"
				OBJECT_DEPENDS "${PCHOUTPUT}")
		endif()
	endforeach()
ENDFUNCTION()

FUNCTION(KLAYGE_ADD_XCODE_PRECOMPILED_HEADER TARGET_NAME PRECOMPILED_HEADER)
	SET_TARGET_PROPERTIES(${TARGET_NAME} PROPERTIES XCODE_ATTRIBUTE_GCC_PRECOMPILE_PREFIX_HEADER YES)
	SET_TARGET_PROPERTIES(${TARGET_NAME} PROPERTIES XCODE_ATTRIBUTE_GCC_PREFIX_HEADER "${PRECOMPILED_HEADER}")
ENDFUNCTION()
