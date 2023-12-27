SET(BOOST_PROJECT_DIR "${EXTERNAL_PROJECT_DIR}/boost")

UpdateExternalLib("boost/assert" "https://github.com/boostorg/assert.git" "9067f0987bb952d8ff266529d03e39aa1af3ffee")
UpdateExternalLib("boost/core" "https://github.com/boostorg/core.git" "7cbbb08e7be8114d4285e15634fc6b5ae23c17f6")

# Depended by assert, core, static_assert, throw_exception
UpdateExternalLib("boost/config" "https://github.com/boostorg/config.git" "cbeca533d2113efbcffa84326111ad15a6a1f57a")
# Depended by core
UpdateExternalLib("boost/static_assert" "https://github.com/boostorg/static_assert.git" "ba72d3340f3dc6e773868107f35902292f84b07e")
# Depended by core
UpdateExternalLib("boost/throw_exception" "https://github.com/boostorg/throw_exception.git" "152b36e86fd1e24fd5ced47697522d5947d69020")

add_subdirectory(${BOOST_PROJECT_DIR}/assert "${CMAKE_CURRENT_BINARY_DIR}/boost/assert" EXCLUDE_FROM_ALL)
add_subdirectory(${BOOST_PROJECT_DIR}/core "${CMAKE_CURRENT_BINARY_DIR}/boost/core" EXCLUDE_FROM_ALL)

add_subdirectory(${BOOST_PROJECT_DIR}/config "${CMAKE_CURRENT_BINARY_DIR}/boost/config" EXCLUDE_FROM_ALL)
add_subdirectory(${BOOST_PROJECT_DIR}/static_assert "${CMAKE_CURRENT_BINARY_DIR}/boost/static_assert" EXCLUDE_FROM_ALL)
add_subdirectory(${BOOST_PROJECT_DIR}/throw_exception "${CMAKE_CURRENT_BINARY_DIR}/boost/throw_exception" EXCLUDE_FROM_ALL)

set_target_properties(boost_assert boost_core PROPERTIES
	FOLDER "External/boost"
)
