include(RunCMake)

set(RunCMake_TEST_OPTIONS
  "-DPSEUDO_CPPCHECK=${PSEUDO_CPPCHECK}"
  "-DPSEUDO_CPPLINT=${PSEUDO_CPPLINT}"
  "-DPSEUDO_IWYU=${PSEUDO_IWYU}"
  "-DPSEUDO_TIDY=${PSEUDO_TIDY}"
  )

function(run_multilint lang)
  # Use a single build tree for tests without cleaning.
  set(RunCMake_TEST_BINARY_DIR "${RunCMake_BINARY_DIR}/${lang}-build")
  set(RunCMake_TEST_NO_CLEAN 1)
  file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")
  run_cmake(${lang})
  set(RunCMake_TEST_OUTPUT_MERGE 1)
  run_cmake_command(${lang}-Build ${CMAKE_COMMAND} --build .)
endfunction()

run_multilint(C)
run_multilint(CXX)

if(NOT RunCMake_GENERATOR STREQUAL "Watcom WMake")
  run_multilint(C-launch)
  run_multilint(CXX-launch)
  run_multilint(genex)
endif()

function(run_skip_linting test_name)
    set(RunCMake_TEST_BINARY_DIR "${RunCMake_BINARY_DIR}/${test_name}-build")
    set(RunCMake_TEST_NO_CLEAN 1)

    run_cmake(${test_name})
    set(RunCMake_TEST_OUTPUT_MERGE 1)
    run_cmake_command(${test_name}-Build ${CMAKE_COMMAND} --build .)
endfunction()

foreach(lang IN ITEMS C CXX)
  # Testing `SKIP_LINTING=ON`
  run_skip_linting(${lang}_skip_linting_ON)
  if(NOT RunCMake_GENERATOR STREQUAL "Watcom WMake")
    run_skip_linting(${lang}-launch_skip_linting_ON)
  endif()

  # Testing `SKIP_LINTING=OFF`
  run_skip_linting(${lang}_skip_linting_OFF)
endforeach()
