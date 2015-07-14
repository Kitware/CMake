include(RunCTest)

set(CASE_CTEST_BUILD_ARGS "")

function(run_ctest_build CASE_NAME)
  set(CASE_CTEST_BUILD_ARGS "${ARGN}")
  run_ctest(${CASE_NAME})
endfunction()

run_ctest_build(BuildQuiet QUIET)

function(run_BuildFailure)
  set(CASE_CMAKELISTS_SUFFIX_CODE [[
add_custom_target(BuildFailure ALL COMMAND command-does-not-exist)
]])
  set(CASE_TEST_PREFIX_CODE [[
cmake_policy(SET CMP0061 NEW)
]])
  set(CASE_TEST_SUFFIX_CODE [[
if (ctest_build_return_value)
  message("ctest_build returned non-zero")
else()
  message("ctest_build returned zero")
endif()
]])
  run_ctest(BuildFailure)

  if (RunCMake_GENERATOR MATCHES "Makefiles")
    set(CASE_TEST_PREFIX_CODE "")
    run_ctest(BuildFailure-CMP0061-OLD)
  endif()
endfunction()
run_BuildFailure()


function(run_BuildChangeID)
  set(CASE_TEST_PREFIX_CODE [[
    set(CTEST_CHANGE_ID "<>1")
  ]])

  set(CASE_TEST_SUFFIX_CODE [[
file(GLOB_RECURSE build_xml_file
  "${CTEST_BINARY_DIRECTORY}/Testing/Build.xml")
if(build_xml_file)
  file(STRINGS "${build_xml_file}" line
    REGEX "^.*<ChangeID>(.*)</ChangeID>$" LIMIT_COUNT 1)
  if("${line}" MATCHES "<ChangeID>&amp\\\;lt\\\;&amp\\\;gt\\\;1</ChangeID>")
    message("expected ChangeID found")
  endif()
endif()
]])

  run_ctest(BuildChangeID)
endfunction()
run_BuildChangeID()
