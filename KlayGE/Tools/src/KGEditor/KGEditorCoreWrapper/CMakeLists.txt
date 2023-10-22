IF(MSVC_VERSION GREATER 1910)
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:twoPhase-")
ENDIF()

SET(SOURCE_FILES 
	${KLAYGE_PROJECT_DIR}/Tools/src/KGEditor/KGEditorCoreWrapper/AssemblyInfo.cpp
	${KLAYGE_PROJECT_DIR}/Tools/src/KGEditor/KGEditorCoreWrapper/KGEditorCoreWrapper.cpp
)

SET(HEADER_FILES 
	${KLAYGE_PROJECT_DIR}/Tools/src/KGEditor/KGEditorCoreWrapper/KGEditorCoreWrapper.hpp
)

SOURCE_GROUP("Source Files" FILES ${SOURCE_FILES})
SOURCE_GROUP("Header Files" FILES ${HEADER_FILES})

STRING(REPLACE "/RTC1" "" CMAKE_CXX_FLAGS_DEBUG ${CMAKE_CXX_FLAGS_DEBUG})
STRING(REPLACE "/DEBUG:FASTLINK" "/DEBUG:FULL" CMAKE_SHARED_LINKER_FLAGS_DEBUG ${CMAKE_SHARED_LINKER_FLAGS_DEBUG})
STRING(REPLACE "/DEBUG:FASTLINK" "/DEBUG:FULL" CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO ${CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO})
IF(CMAKE_VERSION VERSION_LESS "3.12")
	SET(ADD_COMMON_LANGUAGE_RUNTIME FALSE)
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /clr")
ELSE()
	SET(ADD_COMMON_LANGUAGE_RUNTIME TRUE)
ENDIF()
if(MSVC_VERSION GREATER 1915)
	string(REPLACE "/JMC" " " CMAKE_CXX_FLAGS_DEBUG ${CMAKE_CXX_FLAGS_DEBUG})
	string(REPLACE "/JMC" " " CMAKE_CXX_FLAGS_RELWITHDEBINFO ${CMAKE_CXX_FLAGS_RELWITHDEBINFO})
endif()
if(MSVC_VERSION GREATER_EQUAL 1929)
	set(CMAKE_CXX_STANDARD 17)
	string(REPLACE "/std:c++20" "/std:c++17" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
endif()
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHa")
SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /GR /Qpar-")
SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /GR /Qpar-")
SET(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} /GR /Qpar-")
SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /ignore:4248")
ADD_LIBRARY(KGEditorCoreWrapper SHARED ${SOURCE_FILES} ${HEADER_FILES})

if(KLAYGE_COMPILER_MSVC)
	target_compile_options(KGEditorCoreWrapper
		PRIVATE
			/wd4339 # Ignore classes without CLR meta-data
	)
endif()

SET_TARGET_PROPERTIES(KGEditorCoreWrapper PROPERTIES
	PROJECT_LABEL KGEditorCoreWrapper
	OUTPUT_NAME KGEditorCoreWrapper
	VS_GLOBAL_ManagedAssembly "true"
	VS_DOTNET_TARGET_FRAMEWORK_VERSION "v4.8"
	VS_DOTNET_REFERENCES "System;System.Windows;PresentationCore"
	FOLDER "KlayGE/Tools/KGEditor"
)
IF(ADD_COMMON_LANGUAGE_RUNTIME)
	SET_TARGET_PROPERTIES(KGEditorCoreWrapper PROPERTIES
		COMMON_LANGUAGE_RUNTIME "")
ENDIF()

target_link_libraries(KGEditorCoreWrapper
	PRIVATE
		KGEditorCore
)