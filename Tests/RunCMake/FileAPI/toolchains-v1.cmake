# If testing with a specific CMAKE_CXX_COMPILER_ARG1 value is requested, skip
# any checks that try to actually compile anything, because the compiler
# probably wouldn't understand these arguments or lack thereof.
if(DEFINED TOOLCHAINSV1_COMPILERARGS)
  if(TOOLCHAINSV1_COMPILERARGS EQUAL 1)
    set(CMAKE_CXX_COMPILER_ARG1 "")
  elseif(TOOLCHAINSV1_COMPILERARGS EQUAL 2)
    set(CMAKE_CXX_COMPILER_ARG1 "--hello world --something=other")
  endif()
  set(CMAKE_CXX_COMPILER_WORKS 1)
  set(CMAKE_CXX_ABI_COMPILED 1)
else()
  set(TOOLCHAINSV1_COMPILERARGS 0)
endif()

enable_language(CXX)

set(variable_suffixes
  COMPILER COMPILER_ARG1 COMPILER_ID COMPILER_VERSION COMPILER_TARGET
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
string(JSON json SET "${json}" "TOOLCHAINSV1_COMPILERARGS" "${TOOLCHAINSV1_COMPILERARGS}")

file(WRITE ${CMAKE_BINARY_DIR}/toolchain_variables.json "${json}")

if(FAIL)
  message(FATAL_ERROR "Intentionally fail to configure")
endif()
