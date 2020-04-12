SET(BOOST_PROJECT_DIR "${EXTERNAL_PROJECT_DIR}/boost")

UpdateExternalLib("boost/assert" "https://github.com/boostorg/assert.git" "35c31e7778cd5e02cdcdea7bedce4c7f8d767803")
UpdateExternalLib("boost/container" "https://github.com/boostorg/container.git" "f73fb1442a0d714e97506b6fa86f5a1f16da7ddc")
UpdateExternalLib("boost/core" "https://github.com/boostorg/core.git" "690514e87cde26f656cdd75b96aba03c8abcb398")
UpdateExternalLib("boost/type_index" "https://github.com/boostorg/type_index.git" "f2e395a7f3fc3527b82487e7f728d6cb33328f8a")
UpdateExternalLib("boost/utility" "https://github.com/boostorg/utility.git" "86e7caefea0ed6557a25823a547c01b51ace1a99")

# Depended by assert, container_hash, core, integer, intrusive, move, static_assert, throw_exception, type_index
UpdateExternalLib("boost/config" "https://github.com/boostorg/config.git" "b9eac83cb30c5f0707d44ee1dddfe179973995f2")
# Depended by container, type_index, utility
UpdateExternalLib("boost/container_hash" "https://github.com/boostorg/container_hash.git" "8a7fc581ed987fde8cae4e7145378d23b9b6b351")
# Depended by container_hash
UpdateExternalLib("boost/detail" "https://github.com/boostorg/detail.git" "8dbbfe372b83f7ff32d545bad1c779348bfe3eea")
# Depended by container, container_hash
UpdateExternalLib("boost/integer" "https://github.com/boostorg/integer.git" "bee5818e18cf33f06f776556fdacec694c0259d1")
# Depended by container
UpdateExternalLib("boost/intrusive" "https://github.com/boostorg/intrusive.git" "d8761780b0386c53ab144173a5fdc536ee2e1da8")
# Depended by container, intrusive
UpdateExternalLib("boost/move" "https://github.com/boostorg/move.git" "48243ffc035c09794f7a3c3f0588b3c1ca043817")
# Depended by detail, type_index, type_traits, utility
UpdateExternalLib("boost/preprocessor" "https://github.com/boostorg/preprocessor.git" "9cf66925d40bd4a5ac618c21418e430f76761864")
# Depended by container, container_hash, detail, integer, intrusive, move, type_index, type_traits, utility
UpdateExternalLib("boost/static_assert" "https://github.com/boostorg/static_assert.git" "9102d749337c96e9dd62de1132d45d900e5f184a")
# Depended by integer, type_index, utility
UpdateExternalLib("boost/throw_exception" "https://github.com/boostorg/throw_exception.git" "548084bd4c868325dbf3d1c0bd8947083fd08a02")
# Depended by container, container_hash, detail, type_index, utility
UpdateExternalLib("boost/type_traits" "https://github.com/boostorg/type_traits.git" "7ee9910b2f13611ffb9b7fe95410e7ad763e9a62")

add_subdirectory(${BOOST_PROJECT_DIR}/assert "${CMAKE_CURRENT_BINARY_DIR}/boost/assert" EXCLUDE_FROM_ALL)
add_subdirectory(${BOOST_PROJECT_DIR}/container "${CMAKE_CURRENT_BINARY_DIR}/boost/container" EXCLUDE_FROM_ALL)
add_subdirectory(${BOOST_PROJECT_DIR}/core "${CMAKE_CURRENT_BINARY_DIR}/boost/core" EXCLUDE_FROM_ALL)
add_subdirectory(${BOOST_PROJECT_DIR}/type_index "${CMAKE_CURRENT_BINARY_DIR}/boost/type_index" EXCLUDE_FROM_ALL)
add_subdirectory(${BOOST_PROJECT_DIR}/utility "${CMAKE_CURRENT_BINARY_DIR}/boost/utility" EXCLUDE_FROM_ALL)

add_subdirectory(${BOOST_PROJECT_DIR}/config "${CMAKE_CURRENT_BINARY_DIR}/boost/config" EXCLUDE_FROM_ALL)
add_subdirectory(${BOOST_PROJECT_DIR}/container_hash "${CMAKE_CURRENT_BINARY_DIR}/boost/container_hash" EXCLUDE_FROM_ALL)
add_subdirectory(${BOOST_PROJECT_DIR}/detail "${CMAKE_CURRENT_BINARY_DIR}/boost/detail" EXCLUDE_FROM_ALL)
add_subdirectory(${BOOST_PROJECT_DIR}/integer "${CMAKE_CURRENT_BINARY_DIR}/boost/integer" EXCLUDE_FROM_ALL)
add_subdirectory(${BOOST_PROJECT_DIR}/intrusive "${CMAKE_CURRENT_BINARY_DIR}/boost/intrusive" EXCLUDE_FROM_ALL)
add_subdirectory(${BOOST_PROJECT_DIR}/move "${CMAKE_CURRENT_BINARY_DIR}/boost/move" EXCLUDE_FROM_ALL)
add_subdirectory(${BOOST_PROJECT_DIR}/preprocessor "${CMAKE_CURRENT_BINARY_DIR}/boost/preprocessor" EXCLUDE_FROM_ALL)
add_subdirectory(${BOOST_PROJECT_DIR}/static_assert "${CMAKE_CURRENT_BINARY_DIR}/boost/static_assert" EXCLUDE_FROM_ALL)
add_subdirectory(${BOOST_PROJECT_DIR}/throw_exception "${CMAKE_CURRENT_BINARY_DIR}/boost/throw_exception" EXCLUDE_FROM_ALL)
add_subdirectory(${BOOST_PROJECT_DIR}/type_traits "${CMAKE_CURRENT_BINARY_DIR}/boost/type_traits" EXCLUDE_FROM_ALL)
