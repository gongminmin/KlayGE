SET(KFL_PROJECT_DIR "${KLAYGE_ROOT_DIR}/KFL")

SET(KFL_OUTPUT_DIR "${KFL_PROJECT_DIR}/lib/${KLAYGE_PLATFORM_NAME}")

SET(LIB_NAME KFL)

SET(BASE_DETAIL_HEADER_FILES
	${KFL_PROJECT_DIR}/include/KFL/Detail/AutoLink.hpp
)
SET(BASE_CXX17_HEADER_FILES
	${KFL_PROJECT_DIR}/include/KFL/CXX17/any.hpp
	${KFL_PROJECT_DIR}/include/KFL/CXX17/filesystem.hpp
	${KFL_PROJECT_DIR}/include/KFL/CXX17/optional.hpp
	${KFL_PROJECT_DIR}/include/KFL/CXX17/string_view.hpp
)
SET(BASE_CXX2a_HEADER_FILES
	${KFL_PROJECT_DIR}/include/KFL/CXX2a/endian.hpp
	${KFL_PROJECT_DIR}/include/KFL/CXX2a/format.hpp
	${KFL_PROJECT_DIR}/include/KFL/CXX2a/span.hpp
)
SET(BASE_HEADER_FILES
	${KFL_PROJECT_DIR}/include/KFL/AlignedAllocator.hpp
	${KFL_PROJECT_DIR}/include/KFL/Architecture.hpp
	${KFL_PROJECT_DIR}/include/KFL/com_ptr.hpp
	${KFL_PROJECT_DIR}/include/KFL/Compiler.hpp
	${KFL_PROJECT_DIR}/include/KFL/Config.hpp
	${KFL_PROJECT_DIR}/include/KFL/CpuInfo.hpp
	${KFL_PROJECT_DIR}/include/KFL/CustomizedStreamBuf.hpp
	${KFL_PROJECT_DIR}/include/KFL/CXX17.hpp
	${KFL_PROJECT_DIR}/include/KFL/DllLoader.hpp
	${KFL_PROJECT_DIR}/include/KFL/ErrorHandling.hpp
	${KFL_PROJECT_DIR}/include/KFL/Hash.hpp
	${KFL_PROJECT_DIR}/include/KFL/KFL.hpp
	${KFL_PROJECT_DIR}/include/KFL/Log.hpp
	${KFL_PROJECT_DIR}/include/KFL/Platform.hpp
	${KFL_PROJECT_DIR}/include/KFL/PreDeclare.hpp
	${KFL_PROJECT_DIR}/include/KFL/ResIdentifier.hpp
	${KFL_PROJECT_DIR}/include/KFL/SmartPtrHelper.hpp
	${KFL_PROJECT_DIR}/include/KFL/StringUtil.hpp
	${KFL_PROJECT_DIR}/include/KFL/Thread.hpp
	${KFL_PROJECT_DIR}/include/KFL/Timer.hpp
	${KFL_PROJECT_DIR}/include/KFL/Trace.hpp
	${KFL_PROJECT_DIR}/include/KFL/Types.hpp
	${KFL_PROJECT_DIR}/include/KFL/Util.hpp
	${KFL_PROJECT_DIR}/include/KFL/XMLDom.hpp
)
SET(BASE_SOURCE_FILES
	${KFL_PROJECT_DIR}/src/Base/CpuInfo.cpp
	${KFL_PROJECT_DIR}/src/Base/CustomizedStreamBuf.cpp
	${KFL_PROJECT_DIR}/src/Base/DllLoader.cpp
	${KFL_PROJECT_DIR}/src/Base/ErrorHandling.cpp
	${KFL_PROJECT_DIR}/src/Base/Log.cpp
	${KFL_PROJECT_DIR}/src/Base/Thread.cpp
	${KFL_PROJECT_DIR}/src/Base/Timer.cpp
	${KFL_PROJECT_DIR}/src/Base/Util.cpp
	${KFL_PROJECT_DIR}/src/Base/XMLDom.cpp
)

SET(MATH_HEADER_FILES
	${KFL_PROJECT_DIR}/include/KFL/Detail/MathHelper.hpp
	${KFL_PROJECT_DIR}/include/KFL/AABBox.hpp
	${KFL_PROJECT_DIR}/include/KFL/Bound.hpp
	${KFL_PROJECT_DIR}/include/KFL/Color.hpp
	${KFL_PROJECT_DIR}/include/KFL/Frustum.hpp
	${KFL_PROJECT_DIR}/include/KFL/Half.hpp
	${KFL_PROJECT_DIR}/include/KFL/Math.hpp
	${KFL_PROJECT_DIR}/include/KFL/Matrix.hpp
	${KFL_PROJECT_DIR}/include/KFL/Noise.hpp
	${KFL_PROJECT_DIR}/include/KFL/OBBox.hpp
	${KFL_PROJECT_DIR}/include/KFL/Plane.hpp
	${KFL_PROJECT_DIR}/include/KFL/Quaternion.hpp
	${KFL_PROJECT_DIR}/include/KFL/Rect.hpp
	${KFL_PROJECT_DIR}/include/KFL/SIMDMath.hpp
	${KFL_PROJECT_DIR}/include/KFL/SIMDMatrix.hpp
	${KFL_PROJECT_DIR}/include/KFL/SIMDVector.hpp
	${KFL_PROJECT_DIR}/include/KFL/Size.hpp
	${KFL_PROJECT_DIR}/include/KFL/Sphere.hpp
	${KFL_PROJECT_DIR}/include/KFL/Vector.hpp
)
SET(MATH_SOURCE_FILES
	${KFL_PROJECT_DIR}/src/Math/AABBox.cpp
	${KFL_PROJECT_DIR}/src/Math/Color.cpp
	${KFL_PROJECT_DIR}/src/Math/Frustum.cpp
	${KFL_PROJECT_DIR}/src/Math/Half.cpp
	${KFL_PROJECT_DIR}/src/Math/Math.cpp
	${KFL_PROJECT_DIR}/src/Math/Matrix.cpp
	${KFL_PROJECT_DIR}/src/Math/Noise.cpp
	${KFL_PROJECT_DIR}/src/Math/OBBox.cpp
	${KFL_PROJECT_DIR}/src/Math/Plane.cpp
	${KFL_PROJECT_DIR}/src/Math/Quaternion.cpp
	${KFL_PROJECT_DIR}/src/Math/Rect.cpp
	${KFL_PROJECT_DIR}/src/Math/SIMDMath.cpp
	${KFL_PROJECT_DIR}/src/Math/SIMDMatrix.cpp
	${KFL_PROJECT_DIR}/src/Math/SIMDVector.cpp
	${KFL_PROJECT_DIR}/src/Math/Size.cpp
	${KFL_PROJECT_DIR}/src/Math/Sphere.cpp
)

SOURCE_GROUP("Base\\Source Files" FILES ${BASE_SOURCE_FILES})
SOURCE_GROUP("Base\\Header Files" FILES ${BASE_HEADER_FILES})
SOURCE_GROUP("Base\\Header Files\\Detail" FILES ${BASE_DETAIL_HEADER_FILES})
SOURCE_GROUP("Base\\Header Files\\CXX17" FILES ${BASE_CXX17_HEADER_FILES})
SOURCE_GROUP("Base\\Header Files\\CXX2a" FILES ${BASE_CXX2a_HEADER_FILES})
SOURCE_GROUP("Math\\Source Files" FILES ${MATH_SOURCE_FILES})
SOURCE_GROUP("Math\\Header Files" FILES ${MATH_HEADER_FILES})

ADD_LIBRARY(${LIB_NAME} STATIC
	${BASE_SOURCE_FILES} ${BASE_HEADER_FILES} ${BASE_DETAIL_HEADER_FILES} ${BASE_CXX17_HEADER_FILES} ${BASE_CXX2a_HEADER_FILES}
	${MATH_SOURCE_FILES} ${MATH_HEADER_FILES}
)

target_compile_definitions(${LIB_NAME}
	PRIVATE
		-DKFL_SOURCE
)
if(MSVC AND (MSVC_VERSION GREATER_EQUAL 1925))
	target_compile_definitions(${LIB_NAME}
		PUBLIC
			-D_SILENCE_CXX17_RESULT_OF_DEPRECATION_WARNING
	)
endif()

target_include_directories(${LIB_NAME}
	PUBLIC
		# Not using target_link_libraries to prevent adding boost_container project
		$<TARGET_PROPERTY:Boost::container,INTERFACE_INCLUDE_DIRECTORIES>
		# Not using target_link_libraries to prevent depending on Boost::io
		$<TARGET_PROPERTY:Boost::utility,INTERFACE_INCLUDE_DIRECTORIES>
		${KFL_PROJECT_DIR}/include
)

add_dependencies(${LIB_NAME} fmt)

SET(LIB_OUTPUT_NAME ${LIB_NAME}${KLAYGE_OUTPUT_SUFFIX})

SET_TARGET_PROPERTIES(${LIB_NAME} PROPERTIES
	ARCHIVE_OUTPUT_DIRECTORY ${KFL_OUTPUT_DIR}
	ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${KFL_OUTPUT_DIR}
	ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${KFL_OUTPUT_DIR}
	ARCHIVE_OUTPUT_DIRECTORY_RELWITHDEBINFO ${KFL_OUTPUT_DIR}
	ARCHIVE_OUTPUT_DIRECTORY_MINSIZEREL ${KFL_OUTPUT_DIR}
	PROJECT_LABEL ${LIB_NAME}
	DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX}
	OUTPUT_NAME ${LIB_OUTPUT_NAME}
	FOLDER "KFL"
)

KLAYGE_ADD_PRECOMPILED_HEADER(${LIB_NAME} "${KFL_PROJECT_DIR}/include/KFL/KFL.hpp")

target_link_libraries(${LIB_NAME}
	PUBLIC
		Boost::assert
		Boost::core
		fmt
		optional-lite
		string-view-lite
		GSL

	PRIVATE
		rapidxml
)

if(KLAYGE_WITH_GHC_FILESYSTEM)
	target_link_libraries(${LIB_NAME}
		PUBLIC
			ghc_filesystem
	)

	add_dependencies(${LIB_NAME} ghc_filesystem)
endif()

if(KLAYGE_COMPILER_MSVC OR KLAYGE_COMPILER_CLANGCL)
	if(KLAYGE_PLATFORM_WINDOWS_STORE)
		target_link_libraries(${LIB_NAME}
			PUBLIC
				kernel32
		)
	endif()
else()
	if(KLAYGE_COMPILER_GCC AND (NOT KLAYGE_PLATFORM_WINDOWS OR NOT (KLAYGE_COMPILER_VERSION EQUAL 81)))
		target_link_libraries(${LIB_NAME}
			PUBLIC
				stdc++fs
		)
	elseif(KLAYGE_COMPILER_CLANG AND KLAYGE_PLATFORM_LINUX)
		if(KLAYGE_COMPILER_VERSION LESS 70)
			target_link_libraries(${LIB_NAME}
				PUBLIC
					c++experimental
			)
		elseif(KLAYGE_COMPILER_VERSION LESS 90)
			target_link_libraries(${LIB_NAME}
				PUBLIC
					c++fs
			)
		endif()
	endif()
endif()
