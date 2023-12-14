SET(BOOST_PROJECT_DIR "${EXTERNAL_PROJECT_DIR}/boost")

UpdateExternalLib("boost/assert" "https://github.com/boostorg/assert.git" "9067f0987bb952d8ff266529d03e39aa1af3ffee")
UpdateExternalLib("boost/core" "https://github.com/boostorg/core.git" "7cbbb08e7be8114d4285e15634fc6b5ae23c17f6")
UpdateExternalLib("boost/type_index" "https://github.com/boostorg/type_index.git" "29ab3258a73215c67598383696c927887fc0cfeb")

# Depended by assert, container_hash, core, static_assert, throw_exception, type_index
UpdateExternalLib("boost/config" "https://github.com/boostorg/config.git" "cbeca533d2113efbcffa84326111ad15a6a1f57a")
# Depended by type_index
UpdateExternalLib("boost/container_hash" "https://github.com/boostorg/container_hash.git" "48a306dcf236ae460d9ba55648d449ed7bea1dee")
# Depended by container_hash
UpdateExternalLib("boost/describe" "https://github.com/boostorg/describe.git" "c89e4dd3db81eb4f2867b2bc965d161f51cc316c")
# Depended by container_hash, describe
UpdateExternalLib("boost/mp11" "https://github.com/boostorg/mp11.git" "391e23ae716aec4d59f6c7272e49e1dd8c01dcdb")
# Depended by core
UpdateExternalLib("boost/static_assert" "https://github.com/boostorg/static_assert.git" "ba72d3340f3dc6e773868107f35902292f84b07e")
# Depended by core, type_index
UpdateExternalLib("boost/throw_exception" "https://github.com/boostorg/throw_exception.git" "152b36e86fd1e24fd5ced47697522d5947d69020")

add_subdirectory(${BOOST_PROJECT_DIR}/assert "${CMAKE_CURRENT_BINARY_DIR}/boost/assert" EXCLUDE_FROM_ALL)
add_subdirectory(${BOOST_PROJECT_DIR}/core "${CMAKE_CURRENT_BINARY_DIR}/boost/core" EXCLUDE_FROM_ALL)
add_subdirectory(${BOOST_PROJECT_DIR}/type_index "${CMAKE_CURRENT_BINARY_DIR}/boost/type_index" EXCLUDE_FROM_ALL)

add_subdirectory(${BOOST_PROJECT_DIR}/config "${CMAKE_CURRENT_BINARY_DIR}/boost/config" EXCLUDE_FROM_ALL)
add_subdirectory(${BOOST_PROJECT_DIR}/container_hash "${CMAKE_CURRENT_BINARY_DIR}/boost/container_hash" EXCLUDE_FROM_ALL)
add_subdirectory(${BOOST_PROJECT_DIR}/describe "${CMAKE_CURRENT_BINARY_DIR}/boost/describe" EXCLUDE_FROM_ALL)
add_subdirectory(${BOOST_PROJECT_DIR}/mp11 "${CMAKE_CURRENT_BINARY_DIR}/boost/mp11" EXCLUDE_FROM_ALL)
add_subdirectory(${BOOST_PROJECT_DIR}/static_assert "${CMAKE_CURRENT_BINARY_DIR}/boost/static_assert" EXCLUDE_FROM_ALL)
add_subdirectory(${BOOST_PROJECT_DIR}/throw_exception "${CMAKE_CURRENT_BINARY_DIR}/boost/throw_exception" EXCLUDE_FROM_ALL)

set_target_properties(boost_assert boost_core PROPERTIES
	FOLDER "External/boost"
)
