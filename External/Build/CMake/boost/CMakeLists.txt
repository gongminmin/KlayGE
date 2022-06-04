SET(BOOST_PROJECT_DIR "${EXTERNAL_PROJECT_DIR}/boost")

UpdateExternalLib("boost/assert" "https://github.com/boostorg/assert.git" "f1a6f9ffd1a0161fc18fad36829033c570cf5ac3")
UpdateExternalLib("boost/core" "https://github.com/boostorg/core.git" "dbefea96316465c501bcb853642cc855b03a462b")
UpdateExternalLib("boost/type_index" "https://github.com/boostorg/type_index.git" "cca370a91834d331e3143ac4d023fb0f178e512b")
UpdateExternalLib("boost/utility" "https://github.com/boostorg/utility.git" "69f6588d7cf7b392ab8e7eac8eb92cdd1a90d77a")

# Depended by assert, container_hash, core, detail, integer, static_assert, throw_exception, type_index, type_traits, utility
UpdateExternalLib("boost/config" "https://github.com/boostorg/config.git" "08dced51e9a688ec3e9c1b7c5f90c99f2b1cdca7")
# Depended by type_index, utility
UpdateExternalLib("boost/container_hash" "https://github.com/boostorg/container_hash.git" "355603c0c2a56013d7e44f3a0952004f4f280b39")
# Depended by container_hash
UpdateExternalLib("boost/detail" "https://github.com/boostorg/detail.git" "8c603a1894ec1d8abc7a0a538297923b0ac98b33")
# Depended by container_hash
UpdateExternalLib("boost/integer" "https://github.com/boostorg/integer.git" "49195acf608190f6a1cf43bce3b472e6fadce0a2")
# Depended by detail, type_index, utility
UpdateExternalLib("boost/preprocessor" "https://github.com/boostorg/preprocessor.git" "d4e82d7d3f2e2adbe280966ac51ce8f9372f5a44")
# Depended by container_hash, detail, integer, type_index, type_traits, utility
UpdateExternalLib("boost/static_assert" "https://github.com/boostorg/static_assert.git" "ba72d3340f3dc6e773868107f35902292f84b07e")
# Depended by integer, type_index, utility
UpdateExternalLib("boost/throw_exception" "https://github.com/boostorg/throw_exception.git" "68d092f4d7aafcc7dfbcc5f408d78bcb9e5289ec")
# Depended by container_hash, detail, type_index, utility
UpdateExternalLib("boost/type_traits" "https://github.com/boostorg/type_traits.git" "d2a4a6bf0a3900e11faaf6904b95183115bac54d")

add_subdirectory(${BOOST_PROJECT_DIR}/assert "${CMAKE_CURRENT_BINARY_DIR}/boost/assert" EXCLUDE_FROM_ALL)
add_subdirectory(${BOOST_PROJECT_DIR}/core "${CMAKE_CURRENT_BINARY_DIR}/boost/core" EXCLUDE_FROM_ALL)
add_subdirectory(${BOOST_PROJECT_DIR}/type_index "${CMAKE_CURRENT_BINARY_DIR}/boost/type_index" EXCLUDE_FROM_ALL)
add_subdirectory(${BOOST_PROJECT_DIR}/utility "${CMAKE_CURRENT_BINARY_DIR}/boost/utility" EXCLUDE_FROM_ALL)

add_subdirectory(${BOOST_PROJECT_DIR}/config "${CMAKE_CURRENT_BINARY_DIR}/boost/config" EXCLUDE_FROM_ALL)
add_subdirectory(${BOOST_PROJECT_DIR}/container_hash "${CMAKE_CURRENT_BINARY_DIR}/boost/container_hash" EXCLUDE_FROM_ALL)
add_subdirectory(${BOOST_PROJECT_DIR}/detail "${CMAKE_CURRENT_BINARY_DIR}/boost/detail" EXCLUDE_FROM_ALL)
add_subdirectory(${BOOST_PROJECT_DIR}/integer "${CMAKE_CURRENT_BINARY_DIR}/boost/integer" EXCLUDE_FROM_ALL)
add_subdirectory(${BOOST_PROJECT_DIR}/preprocessor "${CMAKE_CURRENT_BINARY_DIR}/boost/preprocessor" EXCLUDE_FROM_ALL)
add_subdirectory(${BOOST_PROJECT_DIR}/static_assert "${CMAKE_CURRENT_BINARY_DIR}/boost/static_assert" EXCLUDE_FROM_ALL)
add_subdirectory(${BOOST_PROJECT_DIR}/throw_exception "${CMAKE_CURRENT_BINARY_DIR}/boost/throw_exception" EXCLUDE_FROM_ALL)
add_subdirectory(${BOOST_PROJECT_DIR}/type_traits "${CMAKE_CURRENT_BINARY_DIR}/boost/type_traits" EXCLUDE_FROM_ALL)
