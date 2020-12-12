SET(BOOST_PROJECT_DIR "${EXTERNAL_PROJECT_DIR}/boost")

UpdateExternalLib("boost/assert" "https://github.com/boostorg/assert.git" "2cd99e24e834d600d51cf3b8cdc3df6ecfd8e9f1")
UpdateExternalLib("boost/container" "https://github.com/boostorg/container.git" "8a2444201ac4856e77b46220cb91beb8e8b3c34d")
UpdateExternalLib("boost/core" "https://github.com/boostorg/core.git" "ca4893063fa244a690b1f799328f16408b794c77")
UpdateExternalLib("boost/type_index" "https://github.com/boostorg/type_index.git" "9862011b6b5e900eee31d32684766c15d3506adf")
UpdateExternalLib("boost/utility" "https://github.com/boostorg/utility.git" "37168a3f4b455588cc63ead1d861d6129c7f63fa")

# Depended by assert, container, container_hash, core, detail, integer, intrusive, move, static_assert, throw_exception, type_index, type_traits, utility
UpdateExternalLib("boost/config" "https://github.com/boostorg/config.git" "fad75493bd2da03524854976f2d80dd7989c90fb")
# Depended by container, intrusive, type_index, utility
UpdateExternalLib("boost/container_hash" "https://github.com/boostorg/container_hash.git" "171c012d4723c5e93cc7cffe42919afdf8b27dfa")
# Depended by container_hash
UpdateExternalLib("boost/detail" "https://github.com/boostorg/detail.git" "5d285a2d5a561308ad6703e1ff284d106eb01032")
# Depended by container_hash
UpdateExternalLib("boost/integer" "https://github.com/boostorg/integer.git" "54dcfa677d8883d0bc7179f32cb806d89d4f8e40")
# Depended by container
UpdateExternalLib("boost/intrusive" "https://github.com/boostorg/intrusive.git" "cb3752dbb1f2e95d28adc541a0e2f4ae72e196a5")
# Depended by container, intrusive
UpdateExternalLib("boost/move" "https://github.com/boostorg/move.git" "4a60a8eb0660ac83fdb4886c1eb60dc6e6a4b2ca")
# Depended by detail, type_index, utility
UpdateExternalLib("boost/preprocessor" "https://github.com/boostorg/preprocessor.git" "5e17804af2496e13ca0cc03d892b5351b637ea43")
# Depended by container, container_hash, detail, integer, intrusive, move, type_index, type_traits, utility
UpdateExternalLib("boost/static_assert" "https://github.com/boostorg/static_assert.git" "073c48260658b799e67d56f117ef4ada09f66e06")
# Depended by integer, type_index, utility
UpdateExternalLib("boost/throw_exception" "https://github.com/boostorg/throw_exception.git" "da6f5420fe7e96f673ee185c2ff37cbaa3f95b84")
# Depended by container, container_hash, detail, type_index, utility
UpdateExternalLib("boost/type_traits" "https://github.com/boostorg/type_traits.git" "c6c7ff164789e7ca1d71745714cb27fae956360b")

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
