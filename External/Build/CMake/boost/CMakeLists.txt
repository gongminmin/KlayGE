SET(BOOST_PROJECT_DIR "${EXTERNAL_PROJECT_DIR}/boost")

UpdateExternalLib("boost/assert" "https://github.com/boostorg/assert.git" "35c31e7778cd5e02cdcdea7bedce4c7f8d767803")
UpdateExternalLib("boost/container" "https://github.com/boostorg/container.git" "06417de498fbfd6f6684efe85a99c6adcc0721a5")
UpdateExternalLib("boost/core" "https://github.com/boostorg/core.git" "dcc04c55089b8a74c5a37e3d25694fca697ffb9f")
UpdateExternalLib("boost/type_index" "https://github.com/boostorg/type_index.git" "9862011b6b5e900eee31d32684766c15d3506adf")
UpdateExternalLib("boost/utility" "https://github.com/boostorg/utility.git" "a4feaf4f243b9865dc8c65102232394d0c20473d")

# Depended by assert, container_hash, core, integer, intrusive, move, static_assert, throw_exception, type_index
UpdateExternalLib("boost/config" "https://github.com/boostorg/config.git" "83fd6b3e9d3709d3efdc609a5aceb9d5851212b0")
# Depended by container, type_index, utility
UpdateExternalLib("boost/container_hash" "https://github.com/boostorg/container_hash.git" "8a7fc581ed987fde8cae4e7145378d23b9b6b351")
# Depended by container_hash
UpdateExternalLib("boost/detail" "https://github.com/boostorg/detail.git" "5d285a2d5a561308ad6703e1ff284d106eb01032")
# Depended by container, container_hash
UpdateExternalLib("boost/integer" "https://github.com/boostorg/integer.git" "54dcfa677d8883d0bc7179f32cb806d89d4f8e40")
# Depended by container
UpdateExternalLib("boost/intrusive" "https://github.com/boostorg/intrusive.git" "d8761780b0386c53ab144173a5fdc536ee2e1da8")
# Depended by container, intrusive
UpdateExternalLib("boost/move" "https://github.com/boostorg/move.git" "fa3aa147b4b0a8e02edb0ac516acc21b4b139b78")
# Depended by detail, type_index, type_traits, utility
UpdateExternalLib("boost/preprocessor" "https://github.com/boostorg/preprocessor.git" "e4fb5f4866f69a917b7eeb522a3257d4625e52c1")
# Depended by container, container_hash, detail, integer, intrusive, move, type_index, type_traits, utility
UpdateExternalLib("boost/static_assert" "https://github.com/boostorg/static_assert.git" "ba846a309164183757683b64df4b88f2d72cb0ff")
# Depended by integer, type_index, utility
UpdateExternalLib("boost/throw_exception" "https://github.com/boostorg/throw_exception.git" "da6f5420fe7e96f673ee185c2ff37cbaa3f95b84")
# Depended by container, container_hash, detail, type_index, utility
UpdateExternalLib("boost/type_traits" "https://github.com/boostorg/type_traits.git" "5588bb1ab7ba1809aaa492c4f6c6620dc2aaa927")

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
