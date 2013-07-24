cmake_minimum_required (VERSION 2.6)
project(TestEnvVariables)

# We need the environment variable names to have a common prefix,
# to enable env_test.cpp.in to successfully identify the variable target set.

set(vp "ENV_TEST_CMAKE_") # common variable prefix

set(f_expected "${TestEnvVariables_BINARY_DIR}/env_result_expected.txt")
set(f_result "env_result.txt")


set(env_test_name "env_test.cpp")
set(env_test_production "${TestEnvVariables_BINARY_DIR}/${env_test_name}")


set(ENV_TEST_ENV_VAR_PREFIX_template "${vp}")
set(ENV_TEST_F_RESULT_template "${f_result}")
configure_file("${env_test_name}.in" "${env_test_name}" @ONLY)

add_executable(env_test "${env_test_production}")


##### Provide a large number of environment variable test cases #####

# Feel free to add whichever nutcase your mind is able to come up with. :)
# Fulfilling requirements of some of these tests
# might be impossible (or very problematic) on some platforms, however.
# If so, skip these via conditional checks (CMAKE_GENERATOR etc.).
# But first verify whether setting (and displaying) such env var data
# in the actual destination environment (i.e., shell)
# would work in the first place.

set(env_var_arg_list "")
set(env_var_res_list "")

# BUG NOTE: SORT will jumble *any* semicolon-containing payload string (element)# within the list. CMake bug! (now filed as #13806).
# The explanation at
# "[sldev] Semicolons and CMake" https://lists.secondlife.com/pipermail/sldev/2009-April/013502.html
# is rather unconvincing...
# The string semicolon non-escaping leads to
# a generally incredibly content-opaque list sorting mechanism
# (provided via list(SORT)) producing *DATA CORRUPTION*,
# and corruption needs to be *fixed*
# (probably via CMake policy mechanism).
# If I was suitably bedeviled (am I not?) I would be leaving this flag at TRUE
# and cause some nice PITA for CMake scrambling to fix this test case result ;)
set(CMAKE_SEMICOLON_NONBUGGY FALSE)

macro(add_test_env ENV_NAME ENV_ARG_VAL )
  if(${ARGC} EQUAL 3)
    set(ENV_RES_VAL ${ARGV2})
  else()
    set(ENV_RES_VAL ${ENV_ARG_VAL})
  endif()
  list(APPEND env_var_arg_list "${vp}${ENV_NAME}=${ENV_ARG_VAL}")
  list(APPEND env_var_res_list "${vp}${ENV_NAME}=${ENV_RES_VAL}")
endmacro()

add_test_env("TESTING" "True")
add_test_env("ANOTHER_VAR" "1")
add_test_env("DQUOTE_TEST" "MYTEST")
add_test_env("SPACE_TEST" "MY TEST")
add_test_env("BS_TEST" "MY\\\\TEST")

# Verify escape handling of the well-known set
# of the five predefined XML entities:
# '"' (&quot;) / '\'' (&apos;) / '<' (&lt;) / '>' (&gt;) / '&' (&amp;)
add_test_env("XML_ENTITY_RAW_QUOT" "XML\\\\\"ENTITY")
add_test_env("XML_ENTITY_RAW_APOS" "XML'ENTITY")
add_test_env("XML_ENTITY_RAW_LT" "XML<ENTITY")
add_test_env("XML_ENTITY_RAW_GT" "XML>ENTITY")
add_test_env("XML_ENTITY_RAW_AMP" "XML&ENTITY")

if(CMAKE_SEMICOLON_NONBUGGY)
  add_test_env("XML_ENTITY_ESC_QUOT" "XML&quot\;ENTITY")
  add_test_env("XML_ENTITY_ESC_APOS" "XML&apos\;ENTITY")
  add_test_env("XML_ENTITY_ESC_LT" "XML&lt\;ENTITY")
  add_test_env("XML_ENTITY_ESC_GT" "XML&gt\;ENTITY")
  add_test_env("XML_ENTITY_ESC_AMP" "XML&amp\;ENTITY")
endif(CMAKE_SEMICOLON_NONBUGGY)

# These two additionally need a local BS-escape for CMake:
add_test_env("CHAR_LF" "CHAR\\\\nLF")
add_test_env("CHAR_CR" "CHAR\\\\rCR")

if(CMAKE_SEMICOLON_NONBUGGY)
  add_test_env("CHAR_SEMICOLON" "\"CHAR\;SEMICOLON\"")

  add_test_env("VS_HEX_LF" "\"VS&#x0A\;HEX\"")
  add_test_env("VS_HEX_CR" "\"VS&#x0D\;HEX\"")
endif(CMAKE_SEMICOLON_NONBUGGY)

# We need to SORT the list, since at least in some environments (Windows)
# output of the environment vars happens in sorted order.
# In some cases (where env vars are being returned in random order)
# this will not be enough, however (file comparison will fail)
# - in such case we'll have to rework the implementation to fully handle it.
# We decide to not do REMOVE_DUPLICATES - if this breaks, then it's
# "programmer error".
list(SORT env_var_res_list)

# Debugging helper (quite important, to verify escape handling on CMake side)
#message(FATAL_ERROR "env_var_list now: ${env_var_list}")

# We'll intentionally skip messing with any (platform-specific) newlines
# in file content, at both CMake side and .cpp side.
# Thus we'll simply write out the CMake list flat,
# and make sure to provide list separators (';') on .cpp side.
file(WRITE "${f_expected}" "${env_var_res_list}")

add_custom_command(
  OUTPUT ${f_result}
  DEPENDS env_test
  COMMAND "${TestEnvVariables_BINARY_DIR}/${CMAKE_CFG_INTDIR}/env_test" "arg1" "arg2"
  ENVIRONMENT ${env_var_arg_list}
  COMMENT "custom command to test setting environment variables"
  VERBATIM
)

add_custom_target(env_result DEPENDS env_result.txt)

set(compare_stamp "${TestEnvVariables_BINARY_DIR}/env_compare.stamp")
add_custom_command(
  OUTPUT "${compare_stamp}"
  COMMAND "${CMAKE_COMMAND}" -E compare_files "${f_result}" "${f_expected}"
  COMMENT "File compare of expected (${f_expected}) with actual (${f_result}) output."
  VERBATIM
)
add_custom_target(env_compare ALL DEPENDS "${compare_stamp}")
add_dependencies(env_compare env_result)
