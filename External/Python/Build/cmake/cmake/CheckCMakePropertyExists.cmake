
macro(check_cmake_property_exists propertyname)
  string(TOUPPER ${propertyname} propertyname_upper)
  set(_varname HAVE_${propertyname_upper})
  if(NOT DEFINED ${_varname})
    message(STATUS "Looking for CMake property ${propertyname}")
    execute_process(
      COMMAND ${CMAKE_COMMAND} --help-property ${propertyname_upper}
      OUTPUT_QUIET
      ERROR_QUIET
      RESULT_VARIABLE _result
      )
    if(_result EQUAL 0)
      set(${_varname} TRUE CACHE INTERNAL "Have CMake property ${propertyname}")
      message(STATUS "Looking for CMake property ${propertyname} - found")
    else()
      set(${_varname} FALSE CACHE INTERNAL "Have CMake property ${propertyname}")
      message(STATUS "Looking for CMake property ${propertyname} - not found")
    endif()
  endif()
endmacro()
