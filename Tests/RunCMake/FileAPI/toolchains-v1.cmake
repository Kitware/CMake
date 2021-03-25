enable_language(CXX)

set(variable_suffixes
  COMPILER COMPILER_ID COMPILER_VERSION COMPILER_TARGET
  IMPLICIT_INCLUDE_DIRECTORIES IMPLICIT_LINK_DIRECTORIES
  IMPLICIT_LINK_FRAMEWORK_DIRECTORIES IMPLICIT_LINK_LIBRARIES
  SOURCE_FILE_EXTENSIONS)
set(language CXX)
set(json "{}")

foreach(variable_suffix ${variable_suffixes})
  set(variable "CMAKE_${language}_${variable_suffix}")
  string(JSON json SET "${json}" "${variable}" "{}")
  if(DEFINED "${variable}")
    string(JSON json SET "${json}" "${variable}" "defined" "true")
    string(JSON json SET "${json}" "${variable}" "value" "\"${${variable}}\"")
  else()
    string(JSON json SET "${json}" "${variable}" "defined" "false")
  endif()
endforeach()

file(WRITE ${CMAKE_BINARY_DIR}/toolchain_variables.json "${json}")
