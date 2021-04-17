SET(BOOST_PROJECT_DIR "${EXTERNAL_PROJECT_DIR}/boost")

UpdateExternalLib("boost/assert" "https://github.com/boostorg/assert.git" "793d7580e84baf73bf0dbfdd0eb112a6356075e1")
UpdateExternalLib("boost/core" "https://github.com/boostorg/core.git" "ddbaa242a99f62eda6aa9db1e3abc0c4bc36134f")
UpdateExternalLib("boost/type_index" "https://github.com/boostorg/type_index.git" "62154d3b19116f489caf61fb076ef0f5222fea58")
UpdateExternalLib("boost/utility" "https://github.com/boostorg/utility.git" "7aafdf92a06e4edc98965c92d8355682f0b30e2d")

# Depended by assert, container_hash, core, detail, integer, static_assert, throw_exception, type_index, type_traits, utility
UpdateExternalLib("boost/config" "https://github.com/boostorg/config.git" "aed3dd07270d770f3075dd5384037b7c7b746a2f")
# Depended by type_index, utility
UpdateExternalLib("boost/container_hash" "https://github.com/boostorg/container_hash.git" "171c012d4723c5e93cc7cffe42919afdf8b27dfa")
# Depended by container_hash
UpdateExternalLib("boost/detail" "https://github.com/boostorg/detail.git" "131208d8ccd82ef69afb9cf0bad1a314bd931d88")
# Depended by container_hash
UpdateExternalLib("boost/integer" "https://github.com/boostorg/integer.git" "be13dbce6d74515c4d15a2efbcae87af2e0fbe53")
# Depended by detail, type_index, utility
UpdateExternalLib("boost/preprocessor" "https://github.com/boostorg/preprocessor.git" "229f654b59f725ee18984e2255a51e4489466711")
# Depended by container_hash, detail, integer, type_index, type_traits, utility
UpdateExternalLib("boost/static_assert" "https://github.com/boostorg/static_assert.git" "073c48260658b799e67d56f117ef4ada09f66e06")
# Depended by integer, type_index, utility
UpdateExternalLib("boost/throw_exception" "https://github.com/boostorg/throw_exception.git" "5b675bbc9c2fb49cc9b6459d5776507950b0f9a1")
# Depended by container_hash, detail, type_index, utility
UpdateExternalLib("boost/type_traits" "https://github.com/boostorg/type_traits.git" "30396ed792697b5a5a7c4f02f7e22ae8d6ad95fd")

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
