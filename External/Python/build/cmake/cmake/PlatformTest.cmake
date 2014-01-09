if(CMAKE_VERSION VERSION_GREATER 2.8.7)
  include(CMakeExpandImportedTargets)
else()
  include(${CMAKE_SOURCE_DIR}/cmake/CMakeExpandImportedTargets.cmake)
endif()

macro(python_platform_test var description srcfile invert)
  IF("${var}_COMPILED" MATCHES "^${var}_COMPILED$")
    message(STATUS "${description}")
    
    set(MACRO_CHECK_FUNCTION_DEFINITIONS
      "-D${var} ${CMAKE_REQUIRED_FLAGS}")
    if(CMAKE_REQUIRED_LIBRARIES)
      # this one translates potentially used imported library targets to their files on disk
      CMAKE_EXPAND_IMPORTED_TARGETS(_ADJUSTED_CMAKE_REQUIRED_LIBRARIES  LIBRARIES  ${CMAKE_REQUIRED_LIBRARIES} CONFIGURATION "${CMAKE_TRY_COMPILE_CONFIGURATION}")
      set(CHECK_C_SOURCE_COMPILES_ADD_LIBRARIES
        "-DLINK_LIBRARIES:STRING=${_ADJUSTED_CMAKE_REQUIRED_LIBRARIES}")
    else()
      set(CHECK_C_SOURCE_COMPILES_ADD_LIBRARIES)
    endif()
    if(CMAKE_REQUIRED_INCLUDES)
      set(CHECK_C_SOURCE_COMPILES_ADD_INCLUDES
        "-DINCLUDE_DIRECTORIES:STRING=${CMAKE_REQUIRED_INCLUDES}")
    else()
      set(CHECK_C_SOURCE_COMPILES_ADD_INCLUDES)
    endif()
    
    try_compile(${var}_COMPILED
      ${CMAKE_CURRENT_BINARY_DIR}
      ${srcfile}
      COMPILE_DEFINITIONS ${CMAKE_REQUIRED_DEFINITIONS}
      CMAKE_FLAGS -DCOMPILE_DEFINITIONS:STRING=${MACRO_CHECK_FUNCTION_DEFINITIONS}
      "${CHECK_C_SOURCE_COMPILES_ADD_LIBRARIES}"
      "${CHECK_C_SOURCE_COMPILES_ADD_INCLUDES}"
      # OUTPUT_VARIABLE OUTPUT # Do NOT use to make cross-compiling easier
      )
    if(${invert} MATCHES INVERT)
      if(${var}_COMPILED)
        message(STATUS "${description} - no")
      else(${var}_COMPILED)
        message(STATUS "${description} - yes")
      endif(${var}_COMPILED)
    else()
      if(${var}_COMPILED)
        message(STATUS "${description} - yes")
      else()
        message(STATUS "${description} - no")
      endif()
    endif()
  endif()
  if(${invert} MATCHES INVERT)
    if(${var}_COMPILED)
      SET(${var} 0)
    else()
      SET(${var} 1)
    endif()
  else()
    if(${var}_COMPILED)
      SET(${var} 1)
    else()
      SET(${var} 0)
    endif()
  endif()
endmacro()

macro(python_platform_test_run var description srcfile invert)
  if(NOT CMAKE_CROSSCOMPILING)
    if("${var}" MATCHES "^${var}$")
      message(STATUS "${description}")
      
      set(MACRO_CHECK_FUNCTION_DEFINITIONS
        "-D${var} ${CMAKE_REQUIRED_FLAGS}")
      if(CMAKE_REQUIRED_LIBRARIES)
        # this one translates potentially used imported library targets to their files on disk
        CMAKE_EXPAND_IMPORTED_TARGETS(_ADJUSTED_CMAKE_REQUIRED_LIBRARIES  LIBRARIES  ${CMAKE_REQUIRED_LIBRARIES} CONFIGURATION "${CMAKE_TRY_COMPILE_CONFIGURATION}")
        set(CHECK_C_SOURCE_COMPILES_ADD_LIBRARIES
          "-DLINK_LIBRARIES:STRING=${_ADJUSTED_CMAKE_REQUIRED_LIBRARIES}")
      else()
        set(CHECK_C_SOURCE_COMPILES_ADD_LIBRARIES)
      endif()
      if(CMAKE_REQUIRED_INCLUDES)
        set(CHECK_C_SOURCE_COMPILES_ADD_INCLUDES
          "-DINCLUDE_DIRECTORIES:STRING=${CMAKE_REQUIRED_INCLUDES}")
      else()
        set(CHECK_C_SOURCE_COMPILES_ADD_INCLUDES)
      endif()
      
      try_run(${var} ${var}_COMPILED
        ${CMAKE_CURRENT_BINARY_DIR}
        ${srcfile}
        COMPILE_DEFINITIONS ${CMAKE_REQUIRED_DEFINITIONS}
        CMAKE_FLAGS -DCOMPILE_DEFINITIONS:STRING=${MACRO_CHECK_FUNCTION_DEFINITIONS}
        -DCMAKE_SKIP_RPATH:BOOL=${CMAKE_SKIP_RPATH}
        "${CHECK_C_SOURCE_COMPILES_ADD_LIBRARIES}"
        "${CHECK_C_SOURCE_COMPILES_ADD_INCLUDES}"
        # OUTPUT_VARIABLE OUTPUT # Do NOT use to make cross-compiling easier
        )
      # Note that ${var} will be a 0 return value on success.
      if(NOT ${var}_COMPILED)
        set(${var} -1 CACHE INTERNAL "${description} failed to compile.")
      endif()
    
      if(${invert} MATCHES INVERT)
        if(${var}_COMPILED)
          if(${var})
            message(STATUS "${description} - yes")
          else()
            message(STATUS "${description} - no")
          endif()
        else()
          message(STATUS "${description} - failed to compile")
        endif()
      else()
        if(${var}_COMPILED)
          if(${var})
            message(STATUS "${description} - no")
          else()
            message(STATUS "${description} - yes")
          endif(${var})
        else()
          message(STATUS "${description} - failed to compile")
        endif()
      endif()
    endif()
    
    if(${invert} MATCHES INVERT)
      if(${var}_COMPILED)
        if(${var})
          SET(${var} 1)
        else()
          SET(${var} 0)
        endif()
      else()
        SET(${var} 1)
      endif()
    else()
      if(${var}_COMPILED)
        if(${var})
          SET(${var} 0)
        else()
          SET(${var} 1)
        endif()
      else()
        SET(${var} 0)
      endif()
    endif()
  else()
    message(STATUS "${description}")
    message(STATUS "${description} - no")
    SET(${var} 0)
  endif()
endmacro()

