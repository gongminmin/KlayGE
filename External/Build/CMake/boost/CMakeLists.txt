SET(BOOST_PROJECT_DIR "${EXTERNAL_PROJECT_DIR}/boost")

UpdateExternalLib("boost/assert" "https://github.com/boostorg/assert.git" "db964b368723d3007986b93c1d02e3f35f225324")
UpdateExternalLib("boost/core" "https://github.com/boostorg/core.git" "ad20fadde7daead18e882722e095e4137b4f417d")
UpdateExternalLib("boost/type_index" "https://github.com/boostorg/type_index.git" "cca370a91834d331e3143ac4d023fb0f178e512b")
UpdateExternalLib("boost/utility" "https://github.com/boostorg/utility.git" "3aa25c9dd789c89f08f92632898693d9997d8a70")

# Depended by assert, container_hash, core, detail, integer, static_assert, throw_exception, type_index, type_traits, utility
UpdateExternalLib("boost/config" "https://github.com/boostorg/config.git" "a7276f03975df55243924b4d277f4b1ae32bcfa2")
# Depended by type_index, utility
UpdateExternalLib("boost/container_hash" "https://github.com/boostorg/container_hash.git" "bd5b7a359c285a65836674d963e1137b067983ac")
# Depended by container_hash
UpdateExternalLib("boost/detail" "https://github.com/boostorg/detail.git" "c6dac76110869b7cd22e48a146170d14af994240")
# Depended by container_hash
UpdateExternalLib("boost/describe" "https://github.com/boostorg/describe.git" "311cab7565398f106f783f904f7bda036493cce4")
# Depended by container_hash
UpdateExternalLib("boost/integer" "https://github.com/boostorg/integer.git" "8ce66ab37e246d1df51a8d7b37cfa0049057ca8f")
# Depended by container_hash, describe
UpdateExternalLib("boost/mp11" "https://github.com/boostorg/mp11.git" "f6133a9f1f965d89676a33c4a39b3df09373b929")
# Depended by detail, type_index, utility
UpdateExternalLib("boost/preprocessor" "https://github.com/boostorg/preprocessor.git" "667e87b3392db338a919cbe0213979713aca52e3")
# Depended by container_hash, detail, integer, type_index, type_traits, utility
UpdateExternalLib("boost/static_assert" "https://github.com/boostorg/static_assert.git" "ba72d3340f3dc6e773868107f35902292f84b07e")
# Depended by integer, type_index, utility
UpdateExternalLib("boost/throw_exception" "https://github.com/boostorg/throw_exception.git" "d307a2d4a75efb0999f97f559f2df83cb9841b8d")
# Depended by container_hash, detail, type_index, utility
UpdateExternalLib("boost/type_traits" "https://github.com/boostorg/type_traits.git" "74995c5d84a367a9243a1392c614c5488b6b62f6")

add_subdirectory(${BOOST_PROJECT_DIR}/assert "${CMAKE_CURRENT_BINARY_DIR}/boost/assert" EXCLUDE_FROM_ALL)
add_subdirectory(${BOOST_PROJECT_DIR}/core "${CMAKE_CURRENT_BINARY_DIR}/boost/core" EXCLUDE_FROM_ALL)
add_subdirectory(${BOOST_PROJECT_DIR}/type_index "${CMAKE_CURRENT_BINARY_DIR}/boost/type_index" EXCLUDE_FROM_ALL)
add_subdirectory(${BOOST_PROJECT_DIR}/utility "${CMAKE_CURRENT_BINARY_DIR}/boost/utility" EXCLUDE_FROM_ALL)

add_subdirectory(${BOOST_PROJECT_DIR}/config "${CMAKE_CURRENT_BINARY_DIR}/boost/config" EXCLUDE_FROM_ALL)
add_subdirectory(${BOOST_PROJECT_DIR}/container_hash "${CMAKE_CURRENT_BINARY_DIR}/boost/container_hash" EXCLUDE_FROM_ALL)
add_subdirectory(${BOOST_PROJECT_DIR}/detail "${CMAKE_CURRENT_BINARY_DIR}/boost/detail" EXCLUDE_FROM_ALL)
add_subdirectory(${BOOST_PROJECT_DIR}/describe "${CMAKE_CURRENT_BINARY_DIR}/boost/describe" EXCLUDE_FROM_ALL)
add_subdirectory(${BOOST_PROJECT_DIR}/integer "${CMAKE_CURRENT_BINARY_DIR}/boost/integer" EXCLUDE_FROM_ALL)
add_subdirectory(${BOOST_PROJECT_DIR}/mp11 "${CMAKE_CURRENT_BINARY_DIR}/boost/mp11" EXCLUDE_FROM_ALL)
add_subdirectory(${BOOST_PROJECT_DIR}/preprocessor "${CMAKE_CURRENT_BINARY_DIR}/boost/preprocessor" EXCLUDE_FROM_ALL)
add_subdirectory(${BOOST_PROJECT_DIR}/static_assert "${CMAKE_CURRENT_BINARY_DIR}/boost/static_assert" EXCLUDE_FROM_ALL)
add_subdirectory(${BOOST_PROJECT_DIR}/throw_exception "${CMAKE_CURRENT_BINARY_DIR}/boost/throw_exception" EXCLUDE_FROM_ALL)
add_subdirectory(${BOOST_PROJECT_DIR}/type_traits "${CMAKE_CURRENT_BINARY_DIR}/boost/type_traits" EXCLUDE_FROM_ALL)
