# Sanity checks
foreach(varname INPUT_DEF_FILE OUTPUT_DEF_FILE)
  if(NOT DEFINED ${varname})
    message(FATAL_ERROR "Variable '${varname}' is not defined.")
  endif()
endforeach()

file(STRINGS ${INPUT_DEF_FILE} def_lines REGEX "^  (.+)=.+$")
set(stub_def_lines "EXPORTS")
foreach(line IN LISTS def_lines)
  string(REGEX REPLACE "^  (.+)=.+$" "${CMAKE_MATCH_1}" updated_line ${line})
  set(stub_def_lines "${stub_def_lines}${updated_line}\n")
endforeach()
file(WRITE ${OUTPUT_DEF_FILE} "${stub_def_lines}")
