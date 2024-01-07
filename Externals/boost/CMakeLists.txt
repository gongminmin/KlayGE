UpdateExternalLib("assert" "https://github.com/boostorg/assert.git" "9067f0987bb952d8ff266529d03e39aa1af3ffee")
UpdateExternalLib("core" "https://github.com/boostorg/core.git" "7cbbb08e7be8114d4285e15634fc6b5ae23c17f6")

# Depended by assert, core, static_assert, throw_exception
UpdateExternalLib("config" "https://github.com/boostorg/config.git" "cbeca533d2113efbcffa84326111ad15a6a1f57a")
# Depended by core
UpdateExternalLib("static_assert" "https://github.com/boostorg/static_assert.git" "ba72d3340f3dc6e773868107f35902292f84b07e")
# Depended by core
UpdateExternalLib("throw_exception" "https://github.com/boostorg/throw_exception.git" "152b36e86fd1e24fd5ced47697522d5947d69020")

add_subdirectory(assert)
add_subdirectory(core)

add_subdirectory(config)
add_subdirectory(static_assert)
add_subdirectory(throw_exception)

set_target_properties(boost_assert boost_core PROPERTIES
	FOLDER "Externals/boost"
)
