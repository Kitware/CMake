# Verify the build system reused the owning target BMI when compatible,
# generated a shared synthetic target BMI for the incompatible importers,
# and ignored preprocessor-only compile options in the BMI compatibility hash.
set(expected_consumers consumer20 consumer23 consumer23flag)
set(shared_synth_target_name "")

if (DEFINED RunCMake_TEST_CONFIG)
  set(config_dir "${RunCMake_TEST_CONFIG}")
else ()
  set(config_dir "Debug")
endif ()

# Get and check linked-target-dirs for each consumer
foreach (consumer IN LISTS expected_consumers)

  if (RunCMake_GENERATOR_IS_MULTI_CONFIG)
    set(output_dir "${consumer}.dir/${config_dir}")
  else ()
    set(output_dir "${consumer}.dir")
  endif ()

  set(depend_info_file "${RunCMake_TEST_BINARY_DIR}/CMakeFiles/${output_dir}/CXXDependInfo.json")
  if (NOT EXISTS "${depend_info_file}")
    list(APPEND RunCMake_TEST_FAILED
      "Could not find CXXDependInfo.json for consumer ${consumer}: checked ${depend_info_file}")
    continue()
  endif ()

  file(READ "${depend_info_file}" depend_info_json)

  # Extract linked-target-dirs.
  string(JSON linked_dirs_len LENGTH "${depend_info_json}" "linked-target-dirs")
  string(JSON linked_dirs GET "${depend_info_json}" "linked-target-dirs")

  if (consumer STREQUAL "consumer20")
    set(expected_linked_tgt_regex "^importable\\.dir$")
    set(expected_linked_tgt_desc "owning target dir for 'importable'")
    set(expected_linked_tgt_has_bmi 0)
  elseif (consumer STREQUAL "consumer23" OR
          consumer STREQUAL "consumer23flag")
    set(expected_linked_tgt_regex "^importable@synth_[A-Za-z0-9_]+\\.dir$")
    set(expected_linked_tgt_desc "shared synthetic target dir for 'importable'")
    set(expected_linked_tgt_has_bmi 1)
  else ()
    list(APPEND RunCMake_TEST_FAILED
      "No linked target expectation defined for consumer '${consumer}'")
    continue()
  endif ()

  if (NOT linked_dirs_len GREATER 0)
    list(APPEND RunCMake_TEST_FAILED
      "Consumer '${consumer}' has no linked-target-dirs but expected ${expected_linked_tgt_desc}")
    continue()
  endif ()

  set(importable_dirs "")
  set(linked_dir "")
  set(linked_tgt_name "")
  foreach (idx RANGE "${linked_dirs_len}")
    if (idx EQUAL linked_dirs_len)
      break ()
    endif ()
    string(JSON candidate GET "${depend_info_json}" "linked-target-dirs" "${idx}")
    if (RunCMake_GENERATOR_IS_MULTI_CONFIG)
      cmake_path(GET candidate PARENT_PATH candidate_root)
    else ()
      set(candidate_root "${candidate}")
    endif ()
    cmake_path(GET candidate_root FILENAME candidate_name)
    if (candidate_name MATCHES "^importable(@synth_[A-Za-z0-9_]+)?\\.dir$")
      list(APPEND importable_dirs "${candidate}")
      set(linked_dir "${candidate}")
      set(linked_tgt_name "${candidate_name}")
    endif ()
  endforeach ()

  list(LENGTH importable_dirs importable_dirs_len)
  if (NOT importable_dirs_len EQUAL 1)
    list(APPEND RunCMake_TEST_FAILED
      "Expected exactly one 'importable' linked-target-dir for '${consumer}' but found ${importable_dirs_len}: ${linked_dirs}")
    continue ()
  endif ()

  if (NOT linked_tgt_name MATCHES "${expected_linked_tgt_regex}")
    list(APPEND RunCMake_TEST_FAILED
      "Consumer '${consumer}' should link to ${expected_linked_tgt_desc} but found ${linked_dir}")
    continue()
  endif ()

  # Verify the linked target dir exists and contains a BMI
  if (NOT EXISTS "${linked_dir}")
    list(APPEND RunCMake_TEST_FAILED
      "Consumer '${consumer}' links to target directory that does not exist: ${linked_dir}")
    continue()
  endif ()

  if (expected_linked_tgt_has_bmi)
    file(GLOB_RECURSE bmi_files "${linked_dir}/*.bmi")
    if (NOT bmi_files)
      list(APPEND RunCMake_TEST_FAILED
        "No BMI files found in target directory: ${linked_dir}")
      continue()
    endif ()
  endif ()

  if (consumer STREQUAL "consumer23" OR consumer STREQUAL "consumer23flag")
    if (shared_synth_target_name STREQUAL "")
      set(shared_synth_target_name "${linked_tgt_name}")
    elseif (NOT linked_tgt_name STREQUAL shared_synth_target_name)
      list(APPEND RunCMake_TEST_FAILED
        "Expected consumer23 and consumer23flag to share one synthetic target dir, but found '${shared_synth_target_name}' and '${linked_tgt_name}'")
    endif ()
  endif ()

endforeach ()

string(REPLACE ";" "\n  " RunCMake_TEST_FAILED "${RunCMake_TEST_FAILED}")
