SET(SOURCE_FILES 
	${KLAYGE_PROJECT_DIR}/Tools/src/MtlEditor/MtlEditorCoreWrapper/AssemblyInfo.cpp
	${KLAYGE_PROJECT_DIR}/Tools/src/MtlEditor/MtlEditorCoreWrapper/MtlEditorCoreWrapper.cpp
)

SET(HEADER_FILES 
	${KLAYGE_PROJECT_DIR}/Tools/src/MtlEditor/MtlEditorCoreWrapper/MtlEditorCoreWrapper.hpp
)

SOURCE_GROUP("Source Files" FILES ${SOURCE_FILES})
SOURCE_GROUP("Header Files" FILES ${HEADER_FILES})

STRING(REPLACE "/RTC1" "" CMAKE_CXX_FLAGS_DEBUG ${CMAKE_CXX_FLAGS_DEBUG})
STRING(REPLACE "/DEBUG:FASTLINK" "/DEBUG:FULL" CMAKE_SHARED_LINKER_FLAGS_DEBUG ${CMAKE_SHARED_LINKER_FLAGS_DEBUG})
STRING(REPLACE "/DEBUG:FASTLINK" "/DEBUG:FULL" CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO ${CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO})
string(REPLACE "/JMC" " " CMAKE_CXX_FLAGS_DEBUG ${CMAKE_CXX_FLAGS_DEBUG})
string(REPLACE "/JMC" " " CMAKE_CXX_FLAGS_RELWITHDEBINFO ${CMAKE_CXX_FLAGS_RELWITHDEBINFO})
if(MSVC_VERSION LESS 1930)
	set(CMAKE_CXX_STANDARD 17)
	string(REPLACE "/std:c++20" "/std:c++17" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
endif()
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:twoPhase- /EHa")
SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /GR")
SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /GR")
SET(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} /GR")
ADD_LIBRARY(MtlEditorCoreWrapper SHARED ${SOURCE_FILES} ${HEADER_FILES})

if(KLAYGE_COMPILER_MSVC)
	target_compile_options(MtlEditorCoreWrapper
		PRIVATE
			/wd4339 # Ignore classes without CLR meta-data
	)
endif()

SET_TARGET_PROPERTIES(MtlEditorCoreWrapper PROPERTIES
	OUTPUT_NAME MtlEditorCoreWrapper
	COMMON_LANGUAGE_RUNTIME ""
	VS_GLOBAL_ManagedAssembly "true"
	VS_DOTNET_TARGET_FRAMEWORK_VERSION "v4.8"
	VS_DOTNET_REFERENCES "System;System.Windows;PresentationCore"
	FOLDER "KlayGE/Tools/MtlEditor"
)
IF(ADD_COMMON_LANGUAGE_RUNTIME)
	SET_TARGET_PROPERTIES(MtlEditorCoreWrapper PROPERTIES
		COMMON_LANGUAGE_RUNTIME "")
ENDIF()

target_link_libraries(MtlEditorCoreWrapper
	PRIVATE
		MtlEditorCore
)
