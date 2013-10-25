
macro(check_cmake_command_exists commandname)
  message(STATUS "Looking for CMake command ${commandname}")
  string(TOUPPER ${commandname} commandname_upper)
  if(COMMAND ${commandname})
    set(HAVE_${commandname_upper} TRUE)
    message(STATUS "Looking for CMake command ${commandname} - found")
  else()
    set(HAVE_${commandname_upper} FALSE)
    message(STATUS "Looking for CMake command ${commandname} - not found")
  endif()
endmacro()

