include(${CMAKE_CURRENT_LIST_DIR}/verify-snippet.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/json.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/check-query-dir.cmake)

file(GLOB snippets LIST_DIRECTORIES false ${v1}/data/*)
if (NOT snippets)
  add_error("No snippet files generated")
endif()

set(FOUND_SNIPPETS "")
foreach(snippet IN LISTS snippets)
  get_filename_component(filename "${snippet}" NAME)

  read_json("${snippet}" contents)

  # Verify snippet file is valid
  verify_snippet_file("${snippet}" "${contents}")

  # Append to list of collected snippet roles
  if (NOT role IN_LIST FOUND_SNIPPETS AND NOT role STREQUAL build)
    list(APPEND FOUND_SNIPPETS ${role})
  endif()

  # Verify target
  string(JSON target ERROR_VARIABLE noTarget GET "${contents}" target)
  if (NOT target MATCHES NOTFOUND)
    set(targets "main;lib;customTarget;TARGET_NAME")
    if (ARGS_FAIL)
      list(APPEND targets "dummy")
    endif()
    if (NOT ${target} IN_LIST targets)
      json_error("${snippet}" "Unexpected target: ${target}")
    endif()
  endif()

  # Verify contents of compile-* Snippets
  if (filename MATCHES "^compile-")
    string(JSON target GET "${contents}" target)
    string(JSON source GET "${contents}" source)
    string(JSON language GET "${contents}" language)
    string(JSON result GET "${contents}" result)
    if (NOT language MATCHES "C\\+\\+")
      json_error("${snippet}" "Expected C++ compile language")
    endif()
    if (NOT source MATCHES "${target}.cxx$")
      json_error("${snippet}" "Unexpected source file")
    endif()
    if (ARGS_FAIL)
      if (source MATCHES "dummy.cxx" AND result EQUAL 0)
        json_error("${snippet}"
          "Expected nonzero exit code for compile command, got: ${result}"
        )
      elseif (NOT source MATCHES "dummy.cxx" AND NOT result EQUAL 0)
        json_error("${snippet}"
          "Expected zero exit code for compile command, got: ${result}"
        )
      endif()
    else()
      if (NOT result EQUAL 0)
        json_error("${snippet}"
          "Expected zero exit code for compile command, got: ${result}"
        )
      endif()
    endif()
  endif()

  # Verify contents of link-* Snippets
  if (filename MATCHES "^link-")
    string(JSON target GET "${contents}" target)
    if (NOT target MATCHES "main|lib")
      json_error("${snippet}" "Unexpected link target: ${target}")
    endif()
  endif()

  # Verify contents of custom-* Snippets
  if (filename MATCHES "^custom-")
    string(JSON outputs GET "${contents}" outputs)
    # if "outputs" is CMakeFiles/customTarget, should not have a "target"
    if (outputs MATCHES "customTarget")
      json_missing_key("${snippet}" "${contents}" target)
    # if "outputs" is empty list, should have "target" main
    elseif (outputs MATCHES "\\[\\]")
      json_assert_key("${snippet}" "${contents}" target main)
    # if "outputs" is includes output1, should also include output2, and no target
    elseif (outputs MATCHES "output1")
      if (NOT outputs MATCHES "output2")
        json_error("${snippet}" "Custom command missing outputs")
      endif()
      json_missing_key("${snippet}" "${contents}" target)
    # unrecognized outputs
    else()
      json_error("${snippet}" "Custom command has unexpected outputs\n${outputs}")
    endif()
  endif()

  # Verify contents of test-* Snippets
  if (filename MATCHES "^test-")
    string(JSON testName GET "${contents}" testName)
    string(JSON result GET "${contents}" result)
    if (ARGS_FAIL)
      if (testName STREQUAL "test" AND NOT result EQUAL 0)
        json_error("${snippet}" "Expected zero exit code for test")
      elseif (testName STREQUAL "dummy" AND result EQUAL 0)
        json_error("${snippet}"
          "Expected nonzero exit code for dummy test, got: ${result}"
        )
      elseif (NOT testName MATCHES "test|dummy")
        json_error("${snippet}" "Unexpected test name: ${testName}")
      endif()
    else()
      if (NOT testName STREQUAL "test")
        json_error("${snippet}" "Unexpected test name: ${testName}")
      endif()
      if (NOT result EQUAL 0)
        json_error("${snippet}"
          "Expected zero exit code for test, got: ${result}"
        )
      endif()
    endif()
  endif()

  # Verify the overall result, in addition to the sub-commands above.
  if (filename MATCHES "^cmakeInstall|^cmakeBuild|^ctest")
    string(JSON result GET "${contents}" result)
    if (ARGS_FAIL AND result EQUAL 0)
      json_error("${snippet}"
        "Expected nonzero exit code, got: ${result}"
      )
    elseif (NOT ARGS_FAIL AND NOT result EQUAL 0)
      json_error("${snippet}"
        "Expected zero exit code, got: ${result}"
      )
    endif()
  endif()

  # Verify that Config is Debug
  if (filename MATCHES "^test|^compile|^link|^custom|^install")
    string(JSON config GET "${contents}" config)
    if (NOT config STREQUAL "Debug")
      json_error(${snippet} "Unexpected config: ${config}")
    endif()
  endif()

  # Verify command args were passed
  if (filename MATCHES "^cmakeBuild|^ctest")
    string(JSON command GET "${contents}" command)
    if (NOT command MATCHES "Debug")
      json_error(${snippet} "Command value missing passed arguments")
    endif()
  endif()

endforeach()

# Verify that listed snippets match expected roles
set(EXPECTED_SNIPPETS configure generate)
if (ARGS_BUILD OR ARGS_BUILD_MAKE_PROGRAM)
  list(APPEND EXPECTED_SNIPPETS compile link custom)
  if (ARGS_BUILD)
    list(APPEND EXPECTED_SNIPPETS cmakeBuild)
  endif()
endif()
if (ARGS_TEST)
  list(APPEND EXPECTED_SNIPPETS ctest test)
endif()
if (ARGS_INSTALL)
  list(APPEND EXPECTED_SNIPPETS cmakeInstall)
  if (ARGS_INSTALL_PARALLEL)
    list(APPEND EXPECTED_SNIPPETS install)
  endif()
endif()
foreach(role IN LISTS EXPECTED_SNIPPETS)
  list(FIND FOUND_SNIPPETS "${role}" found)
  if (found EQUAL -1)
    add_error("No snippet files of role \"${role}\" were found in ${v1}")
  endif()
endforeach()
foreach(role IN LISTS FOUND_SNIPPETS)
  list(FIND EXPECTED_SNIPPETS "${role}" found)
  if (${found} EQUAL -1)
    add_error("Found unexpected snippet file of role \"${role}\" in ${v1}")
  endif()
endforeach()

# Verify test/install artifacts
if (ARGS_INSTALL AND NOT EXISTS ${RunCMake_TEST_BINARY_DIR}/install)
  add_error("ctest --instrument launcher failed to install the project")
endif()
if (ARGS_TEST AND NOT EXISTS ${RunCMake_TEST_BINARY_DIR}/Testing)
  add_error("ctest --instrument launcher failed to test the project")
endif()

# Look for build snippet, which may not appear immediately
if (ARGS_BUILD_MAKE_PROGRAM)
  set(NUM_TRIES 30)
  set(DELAY 1)
  set(foundBuildSnippet 0)
  foreach(_ RANGE ${NUM_TRIES})
    file(GLOB snippets LIST_DIRECTORIES false ${v1}/data/build-*)
    if (snippets MATCHES build)
      set(foundBuildSnippet 1)
      break()
    endif()
    execute_process(COMMAND ${CMAKE_COMMAND} -E sleep ${DELAY})
  endforeach()
  if (NOT foundBuildSnippet)
    add_error("No snippet files of role \"build\" were found in ${v1}")
  endif()
endif()
