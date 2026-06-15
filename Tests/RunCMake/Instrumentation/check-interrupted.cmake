include(${CMAKE_CURRENT_LIST_DIR}/json.cmake)

# After an interrupted `cmake --build`, exactly one cmakeBuild snippet should be
# present, recording the interrupting signal.  Any cmakeBuild snippet from the
# earlier (uninterrupted) helper build was collated and removed by its
# postCMakeBuild hook.
file(GLOB cmakeBuildSnippets LIST_DIRECTORIES false ${v1}/data/cmakeBuild-*.json)
list(LENGTH cmakeBuildSnippets numCmakeBuild)
if (NOT numCmakeBuild EQUAL 1)
  add_error("Expected exactly one cmakeBuild snippet, found ${numCmakeBuild}: ${cmakeBuildSnippets}")
else()
  read_json("${cmakeBuildSnippets}" contents)

  string(JSON interruptSignal ERROR_VARIABLE noSignal GET "${contents}" interruptSignal)
  if (noSignal OR NOT interruptSignal MATCHES "^[1-9][0-9]*$")
    add_error("cmakeBuild snippet is not marked interrupted:\n${contents}")
  endif()

  string(JSON version_minor GET "${contents}" version minor)
  if (NOT version_minor EQUAL 2)
    add_error("cmakeBuild snippet version minor expected 2, got: ${version_minor}")
  endif()
endif()

# The postCMakeBuild hook must be skipped entirely on interrupt, so its callback
# must not run.  The callback (hook.cmake) writes a postCMakeBuild.hook file
# whenever it runs; the helper build's copy was removed before the interrupted
# build, so its presence here would mean the hook wrongly ran on interrupt.
if (EXISTS ${v1}/postCMakeBuild.hook)
  add_error("postCMakeBuild hook should be skipped on interrupt, but it ran")
endif()

if (DEFINED RunCMake_TEST_FAILED)
  set(RunCMake_TEST_FAILED "${RunCMake_TEST_FAILED}" PARENT_SCOPE)
endif()
