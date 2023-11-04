SET(KFL_PROJECT_DIR "${KLAYGE_ROOT_DIR}/KFL")

SET(KFL_OUTPUT_DIR "${KFL_PROJECT_DIR}/lib/${KLAYGE_PLATFORM_NAME}")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${KFL_OUTPUT_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${KFL_OUTPUT_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${KFL_OUTPUT_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELWITHDEBINFO ${KFL_OUTPUT_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_MINSIZEREL ${KFL_OUTPUT_DIR})

SET(LIB_NAME KFL)

SET(BASE_CXX20_HEADER_FILES
	${KFL_PROJECT_DIR}/include/KFL/CXX20/bit.hpp
	${KFL_PROJECT_DIR}/include/KFL/CXX20/format.hpp
	${KFL_PROJECT_DIR}/include/KFL/CXX20/span.hpp
)
set(BASE_CXX23_HEADER_FILES
	${KFL_PROJECT_DIR}/include/KFL/CXX23/utility.hpp
)
SET(BASE_HEADER_FILES
	${KFL_PROJECT_DIR}/include/KFL/AlignedAllocator.hpp
	${KFL_PROJECT_DIR}/include/KFL/Architecture.hpp
	${KFL_PROJECT_DIR}/include/KFL/com_ptr.hpp
	${KFL_PROJECT_DIR}/include/KFL/Compiler.hpp
	${KFL_PROJECT_DIR}/include/KFL/Config.hpp.in
	${CMAKE_CURRENT_BINARY_DIR}/include/KFL/Config.hpp
	${KFL_PROJECT_DIR}/include/KFL/CpuInfo.hpp
	${KFL_PROJECT_DIR}/include/KFL/CustomizedStreamBuf.hpp
	${KFL_PROJECT_DIR}/include/KFL/DllLoader.hpp
	${KFL_PROJECT_DIR}/include/KFL/ErrorHandling.hpp
	${KFL_PROJECT_DIR}/include/KFL/Hash.hpp
	${KFL_PROJECT_DIR}/include/KFL/JsonDom.hpp
	${KFL_PROJECT_DIR}/include/KFL/KFL.hpp
	${KFL_PROJECT_DIR}/include/KFL/Log.hpp
	${KFL_PROJECT_DIR}/include/KFL/Noncopyable.hpp
	${KFL_PROJECT_DIR}/include/KFL/Operators.hpp
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
	${KFL_PROJECT_DIR}/include/KFL/Uuid.hpp
	${KFL_PROJECT_DIR}/include/KFL/XMLDom.hpp
)
SET(BASE_SOURCE_FILES
	${KFL_PROJECT_DIR}/src/Base/CpuInfo.cpp
	${KFL_PROJECT_DIR}/src/Base/CustomizedStreamBuf.cpp
	${KFL_PROJECT_DIR}/src/Base/DllLoader.cpp
	${KFL_PROJECT_DIR}/src/Base/ErrorHandling.cpp
	${KFL_PROJECT_DIR}/src/Base/JsonDom.cpp
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
SOURCE_GROUP("Base\\Header Files\\CXX20" FILES ${BASE_CXX20_HEADER_FILES})
SOURCE_GROUP("Base\\Header Files\\CXX23" FILES ${BASE_CXX23_HEADER_FILES})
SOURCE_GROUP("Math\\Source Files" FILES ${MATH_SOURCE_FILES})
SOURCE_GROUP("Math\\Header Files" FILES ${MATH_HEADER_FILES})

ADD_LIBRARY(${LIB_NAME} STATIC
	${BASE_SOURCE_FILES} ${BASE_HEADER_FILES}
	${BASE_CXX20_HEADER_FILES} ${BASE_CXX23_HEADER_FILES}
	${MATH_SOURCE_FILES} ${MATH_HEADER_FILES}
)

target_include_directories(${LIB_NAME}
	PUBLIC
		${CMAKE_CURRENT_BINARY_DIR}/include
		${KFL_PROJECT_DIR}/include
)

SET(LIB_OUTPUT_NAME ${LIB_NAME}${KLAYGE_OUTPUT_SUFFIX})

SET_TARGET_PROPERTIES(${LIB_NAME} PROPERTIES
	PROJECT_LABEL ${LIB_NAME}
	OUTPUT_NAME ${LIB_OUTPUT_NAME}
	FOLDER "KFL"
)
if(KLAYGE_PLATFORM_DARWIN OR KLAYGE_PLATFORM_IOS)
	set_target_properties(${LIB_NAME} PROPERTIES
		CXX_VISIBILITY_PRESET hidden
		VISIBILITY_INLINES_HIDDEN ON
	)
endif()

target_precompile_headers(${LIB_NAME}
	PRIVATE
		include/KFL/KFL.hpp
)

target_link_libraries(${LIB_NAME}
	PUBLIC
		Boost::assert
		Boost::core
		scope-lite

	PRIVATE
		rapidjson
		rapidxml
)
if(TARGET fmt)
	target_link_libraries(${LIB_NAME}
		PUBLIC
			fmt
	)
endif()

if(KLAYGE_COMPILER_MSVC OR KLAYGE_COMPILER_CLANGCL)
	if(KLAYGE_PLATFORM_WINDOWS_STORE)
		target_link_libraries(${LIB_NAME}
			PUBLIC
				kernel32
		)
	endif()
else()
	if(KLAYGE_COMPILER_GCC)
		target_link_libraries(${LIB_NAME}
			PUBLIC
				stdc++fs
		)
	endif()
endif()
