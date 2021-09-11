SET(BOOST_PROJECT_DIR "${EXTERNAL_PROJECT_DIR}/boost")

UpdateExternalLib("boost/assert" "https://github.com/boostorg/assert.git" "6aabfebae6d4acf996fe711de4e9b444ad88c17d")
UpdateExternalLib("boost/core" "https://github.com/boostorg/core.git" "f4b3d5dba6f86caaf96e45901655a954b2ff68b4")
UpdateExternalLib("boost/type_index" "https://github.com/boostorg/type_index.git" "a3c6a957eeaf1612eb34b013ec13e67d4c279a41")
UpdateExternalLib("boost/utility" "https://github.com/boostorg/utility.git" "375382e1e6c677c7849e894675251db353ab9186")

# Depended by assert, container_hash, core, detail, integer, static_assert, throw_exception, type_index, type_traits, utility
UpdateExternalLib("boost/config" "https://github.com/boostorg/config.git" "088b79a0ca751932010f82d3f95457c8b483fb9b")
# Depended by type_index, utility
UpdateExternalLib("boost/container_hash" "https://github.com/boostorg/container_hash.git" "e69c4c830e8999602a966a3405487814d7a50f3d")
# Depended by container_hash
UpdateExternalLib("boost/detail" "https://github.com/boostorg/detail.git" "a01fe6d57b906edf0400daebfb5ea88bb4582f44")
# Depended by container_hash
UpdateExternalLib("boost/integer" "https://github.com/boostorg/integer.git" "8fd622545f3e303f467526442a8765290e735778")
# Depended by detail, type_index, utility
UpdateExternalLib("boost/preprocessor" "https://github.com/boostorg/preprocessor.git" "d4e82d7d3f2e2adbe280966ac51ce8f9372f5a44")
# Depended by container_hash, detail, integer, type_index, type_traits, utility
UpdateExternalLib("boost/static_assert" "https://github.com/boostorg/static_assert.git" "392199f6b14ee64260afde27a1c3f876c4bd4843")
# Depended by integer, type_index, utility
UpdateExternalLib("boost/throw_exception" "https://github.com/boostorg/throw_exception.git" "95e02ea52b8525ecf34dbf1e7fae34af09986b8a")
# Depended by container_hash, detail, type_index, utility
UpdateExternalLib("boost/type_traits" "https://github.com/boostorg/type_traits.git" "bfce306637e2a58af4b7bbc881a919dafb7d195b")

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
