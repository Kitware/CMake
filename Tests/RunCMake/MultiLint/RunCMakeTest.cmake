include(RunCMake)

set(RunCMake_TEST_OPTIONS
  "-DPSEUDO_CPPCHECK=${PSEUDO_CPPCHECK}"
  "-DPSEUDO_CPPLINT=${PSEUDO_CPPLINT}"
  "-DPSEUDO_IWYU=${PSEUDO_IWYU}"
  "-DPSEUDO_TIDY=${PSEUDO_TIDY}"
  "-DPSEUDO_PVS=${PSEUDO_PVS}"
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

function(run_skip_linting test_name prop_sf prop_tgt)
  set(RunCMake_TEST_VARIANT_DESCRIPTION " (prop_sf=${prop_sf}, prop_tgt=${prop_tgt})")
  list(APPEND RunCMake_TEST_OPTIONS "-Dprop_sf=${prop_sf}" "-Dprop_tgt=${prop_tgt}")

  set(RunCMake_TEST_BINARY_DIR "${RunCMake_BINARY_DIR}/${test_name}-build")
  set(RunCMake_TEST_NO_CLEAN 1)

  run_cmake(${test_name})
  set(RunCMake_TEST_OUTPUT_MERGE 1)
  run_cmake_command(${test_name}-Build ${CMAKE_COMMAND} --build .)
endfunction()

# There are a few `SKIP_LINTING` source/target propertiy combinations
# that affect the final result:
#
#  prop_sf   prop_tgt   result
#  ---------------------------
#    -         -         OFF
#    OFF       -         OFF
#    ON        -         ON
#    -         OFF       OFF
#    OFF       OFF       OFF
#    ON        OFF       ON
#    -         ON        ON
#    OFF       ON        OFF
#    ON        ON        ON
#
# where `-` means unset property.
#
# Here's the same table for convenience sorted by `result`:
#
#  prop_sf   prop_tgt   result
#  ---------------------------
#    -         -         OFF
#    OFF       -         OFF
#    -         OFF       OFF
#    OFF       OFF       OFF
#    OFF       ON        OFF
#    ON        -         ON
#    ON        OFF       ON
#    -         ON        ON
#    ON        ON        ON

foreach(lang IN ITEMS C CXX)
  # Testing `SKIP_LINTING=OFF` (first half of the table above)
  set(prop_sf_OFF_variants "-" OFF "-" OFF OFF)
  set(prop_tgt_OFF_variants "-" "-" OFF OFF ON)
  foreach(prop_fs prop_tgt IN ZIP_LISTS prop_sf_OFF_variants prop_tgt_OFF_variants)
    run_skip_linting(${lang}_skip_linting_OFF "${prop_fs}" "${prop_tgt}")
  endforeach()

  # Testing `SKIP_LINTING=ON` (second half of the table above)
  set(prop_sf_ON_variants ON ON "-" ON)
  set(prop_tgt_ON_variants "-" OFF ON ON)
  foreach(prop_fs prop_tgt IN ZIP_LISTS prop_sf_ON_variants prop_tgt_ON_variants)
    run_skip_linting(${lang}_skip_linting_ON "${prop_fs}" "${prop_tgt}")
    if(NOT RunCMake_GENERATOR STREQUAL "Watcom WMake")
      run_skip_linting(${lang}-launch_skip_linting_ON "${prop_fs}" "${prop_tgt}")
    endif()
  endforeach()
endforeach()
