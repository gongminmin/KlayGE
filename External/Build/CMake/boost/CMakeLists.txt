SET(BOOST_PROJECT_DIR "${EXTERNAL_PROJECT_DIR}/boost")

UpdateExternalLib("boost/assert" "https://github.com/boostorg/assert.git" "02256c84fd0cd58a139d9dc1b25b5019ca976ada")
UpdateExternalLib("boost/core" "https://github.com/boostorg/core.git" "c7134904e27237dbbb81ba28f0f23fc365b079dd")
UpdateExternalLib("boost/type_index" "https://github.com/boostorg/type_index.git" "7bde4f36af001dd8533e2a4f580d279f0c8468df")

# Depended by assert, container_hash, core, static_assert, throw_exception, type_index, type_traits
UpdateExternalLib("boost/config" "https://github.com/boostorg/config.git" "a1cf5d531405e62927b0257b5cbecc66a545b508")
# Depended by type_index
UpdateExternalLib("boost/container_hash" "https://github.com/boostorg/container_hash.git" "226eb066e949adbf37b220e993d64ecefeeaae99")
# Depended by container_hash
UpdateExternalLib("boost/describe" "https://github.com/boostorg/describe.git" "a0eafb08100eb15a57b6dae6d270c0012a56aa21")
# Depended by container_hash, describe
UpdateExternalLib("boost/mp11" "https://github.com/boostorg/mp11.git" "ef7608b463298b881bc82eae4f45a4385ed74fca")
# Depended by type_index
UpdateExternalLib("boost/preprocessor" "https://github.com/boostorg/preprocessor.git" "667e87b3392db338a919cbe0213979713aca52e3")
# Depended by core, type_index, type_traits
UpdateExternalLib("boost/static_assert" "https://github.com/boostorg/static_assert.git" "ba72d3340f3dc6e773868107f35902292f84b07e")
# Depended by core, type_index
UpdateExternalLib("boost/throw_exception" "https://github.com/boostorg/throw_exception.git" "23dd41e920ecd91237500ac6428f7d392a7a875c")
# Depended by container_hash, type_index
UpdateExternalLib("boost/type_traits" "https://github.com/boostorg/type_traits.git" "89f5011b4a79d91e42735670e39f72cb25c86c72")

add_subdirectory(${BOOST_PROJECT_DIR}/assert "${CMAKE_CURRENT_BINARY_DIR}/boost/assert" EXCLUDE_FROM_ALL)
add_subdirectory(${BOOST_PROJECT_DIR}/core "${CMAKE_CURRENT_BINARY_DIR}/boost/core" EXCLUDE_FROM_ALL)
add_subdirectory(${BOOST_PROJECT_DIR}/type_index "${CMAKE_CURRENT_BINARY_DIR}/boost/type_index" EXCLUDE_FROM_ALL)

add_subdirectory(${BOOST_PROJECT_DIR}/config "${CMAKE_CURRENT_BINARY_DIR}/boost/config" EXCLUDE_FROM_ALL)
add_subdirectory(${BOOST_PROJECT_DIR}/container_hash "${CMAKE_CURRENT_BINARY_DIR}/boost/container_hash" EXCLUDE_FROM_ALL)
add_subdirectory(${BOOST_PROJECT_DIR}/describe "${CMAKE_CURRENT_BINARY_DIR}/boost/describe" EXCLUDE_FROM_ALL)
add_subdirectory(${BOOST_PROJECT_DIR}/mp11 "${CMAKE_CURRENT_BINARY_DIR}/boost/mp11" EXCLUDE_FROM_ALL)
add_subdirectory(${BOOST_PROJECT_DIR}/preprocessor "${CMAKE_CURRENT_BINARY_DIR}/boost/preprocessor" EXCLUDE_FROM_ALL)
add_subdirectory(${BOOST_PROJECT_DIR}/static_assert "${CMAKE_CURRENT_BINARY_DIR}/boost/static_assert" EXCLUDE_FROM_ALL)
add_subdirectory(${BOOST_PROJECT_DIR}/throw_exception "${CMAKE_CURRENT_BINARY_DIR}/boost/throw_exception" EXCLUDE_FROM_ALL)
add_subdirectory(${BOOST_PROJECT_DIR}/type_traits "${CMAKE_CURRENT_BINARY_DIR}/boost/type_traits" EXCLUDE_FROM_ALL)

set_target_properties(boost_assert boost_core PROPERTIES
	FOLDER "${EXTERNAL_VS_FOLDER_PREFIX}/boost"
)
